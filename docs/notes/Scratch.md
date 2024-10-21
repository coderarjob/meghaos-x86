# MEGHA OPERATING SYSTEM V2 - x86
----------------------------------------------------------------------------------------------------

## SCRATCH PAD FOR QUICK NOTES

#### Notation used

| Notation   | Description                                               | Example                |
|------------|-----------------------------------------------------------|------------------------|
| A[X]       | Access item at index X. Each of the items is of type A.   | PDE[768]               |
| [A:B:C] X  | Virtual address X from PDE[A], PTE[B] and offset C.       | [1023:0:0] 0xFFC0_0000 |
| A[X], B[Y] | Nested access item at index Y (of type B) within the item | PDE[768]PTE[0]         |
|            | at index X (of type A).                                   |                        |

------------------------------------

## Application standard library
categories: note, independent
21 Oct 2024

Application here means the programs that runs outside the kernel irrespective if they are running
in Ring 0 or Ring 3. The main purpose of this library is to abstract away the OS level details and
interfaces/system calls from the application programs. The abstraction demands that no OS
structure/interface is exposed directly to the application side. The library would convert
application types to kernel types and vice-versa. This is done to shield OS changes to cause changes
to application programs, though library (not its headers) can change with the OS.

Note that the library has a private side (the .c files) and public side (the app.h). The public side
must which the application programs would depend on should not expose the kernel implementation
details like enums, structures etc. The private side which talks to the kernel however requires to
know the Kernel interfaces and configurations, so its fine for the .c files of the library to
include Kernel headers but not so in the .h files.

I previously thought that it might be possible to reuse some of the implementation from the kernel
in the application library, parts which does not directly on the Kernel like `memcpy`, `strlen` etc.

Though this is possible I am now second guessing.
* The kernel implementation may include logging, timer perf etc which is not applicable in the
  application side. So some change is required to disable the when building app library.
* Having a common source for the Kernel & application side, might create blot since the functions
  used required by a application might not every be required in the Kernel.

I understand the cons of having duplicate code but I think the code which actually will be
common would be small and talking about unit testing, it would be required test the app library
anyways since the library would have functions which are present/required in the kernel.

------------------------------------

## Application and kernel mode standard library
categories: note, independent, obsolete
11 Oct 2024
  
Application here means the programs that runs outside the kernel irrespective if they are running
in Ring 0 or Ring 3. The main purpose of this library is to abstract away the OS level details and
interfaces/system calls from the application programs. Secondarily it provides common data
structures and functions for applications to use.

Regarding the secondary goal mentioned above, it is my observation that few implementations in the
kernel can be useful for the application programs as well. I want to repurpose them for the
application programs. This will reduce the headache of maintaining and testing duplicate
implementations of the same thing. Following are some the Kernel features which can use useful for
applications as well.

* memcpy, strlen, strcpy, memset, memset_pat4
* utils.h
* malloc_internal
* intrusive list & queue
* snprintf, vsnprintf

So the question is how to share the code. I plan to have two libraries one for kernel and another
for applications. The parts that need to change can be under a define.

Reusing kernel code also means both the kernel and application libraries need access to kernel
headers at build time. While not an issue, it may look suspicious about why an application library
requires kernel headers. This is a drawback of kernel code reuse.

The application programs however just need to reference the library and the library headers.

------------------------------------

## Keeping track of Virtual pages using Bitmap
categories: note, x86
_27 April 2024_

A = getPage(region, 50, flags)
setPageAttr(A, 0 , NULL_PAGE)  // To set the page at offset 0 from the start to NULL page
setPageAttr(A, 49 , NULL_PAGE) // To set the page at offset 49 from the start to NULL page

getPage works on the current Page directory. Flags will determine where the virtual page is
allocated from (called regions) - Below, High, Kernel.

These regions are not overlapping and will have a head for each.

At the time of getPage it will start scanning from the head for an AddressSpace with enough free
pages. Once found pages will be marked in the PAB of the AddressSpace. If not found then new
AddressSpace will be created.

AddressSpaces are of fixed size and contain a bitmap to track virutal pages. If the fixed size is
256KiB and we use 2 bits for each Bitmap state, then the bitmap need to be of 16 bytes.

Note that AddressSpace is a way we are using to keep track of Virtual pages and is transparent to
its users.

Now to share pages we need to store the count somewhere. It can be stored in the bitmap itself. In
that case instead of 2 bits we will use 5 bits:
    0-1 -> State (Allocated, Free, Committed, Null)
    2-4 -> Share count. Starts with 0 which means not shared with any other process.

But this required change in the bitmap implementation.

### What changes will be requried in Bitmap

```
/* Bitmap:
* Bitmap can be thought of an array of items, each of size 'b' bits. Out of the 'b' bits, 'm' LSB
* bits represent state and remaining 'b' - 'm' MSB bits are arbitary and can used to store metadata.
* In case there is no metadata then 'm' = 'b'. The states and its metadata only has meaning to the
* user of the bitmap, however the bitmap must know the number of total bits in an bitmap item and
* the number of bits in a 'state' in order to get items and search for items with matching state.
* So the general view of the Bitmap is the following:
*
*     +----+----+----+----+----+----+----+----+----+
*     |   Item 2     |   Item 1     |   Item 0     |
*     +----+----+----+----+----+----+----+----+----+
* MSB |MD 2| State 2 |MD 1| State 1 |MD 0| State 0 | LSB
*     +----+----+----+----+----+----+----+----+----+
*
* Here is an example:
* In the example below, each item is 2 bits no metadata, (which means state is also of 2 bits).
* These states are addresses with their index in the bitmap. Indices start from 0.
*
*    7    6    5    4    3    2    1    0
*     +----+----+----+----+----+----+----+----+
*     | Item 3  | Item 2  | Item 1  | Item 0  |
*     +----+----+----+----+----+----+----+----+
* MSB | State 3 | State 2 | State 1 | State 0 | LSB
*     +----+----+----+----+----+----+----+----+
*/

typedef struct Bitmap {
    BitmapState* bitmap; /* Pointer to the actual Bitmap */
    SIZE size;           /* Number of bytes in the bitmap array. */
    SIZE bitsPerItem;    /* one Item in bitmap requires this many bits. Must be a factor
                            of 8. */
    SIZE bitsPerState;   /* one state in bitmap requires this many bits. Must be a factor
                            of 8 and must be <= bits per item. */
    bool (*allow) (UINT index, BitmapState old,
                   BitmapState new); /* Function returns true if state change can be done. */
} Bitmap;
```

------------------------------------
## Round-robin operation using queue
categories: note, x86
_11 April 2024_

```
Two pointers: back and front

dequeue -> return F, F++, F < B
enqueue -> B < QUEUE_LEN, add item at B, B++

 F       B
 0 1 2 3
[A|B|C|D| | ]

   F       B
 0 1 2 3 4 5
[ |B|C|D|A| ]

     F     B
 0 1 2 3 4 5
[ | |C|D|A|B]

       F     B
 0 1 2 3 4 5 6
[ | | |D|A|B|C]

```

------------------------------------

## Code snippets
categories: note, obsolete, x86
_10 April 2024_

### Temporary map modes

There could  be another mode possible called General, where we scan the page directory/tables for a
free virtual address and assign the physical page to this one. This requires address reservation and
for that reason this can be very slow. Could be used in rare occations but not in places which will
be frequently invoked or in a loop.

```c
// These Temporary map modes determine where physical page is going to be mapped in the address
// space. KERNEL mode causes temporary map to be in Kernel address space, while in PROCESS mode it
// is mapped in the address space of the current process.
typedef enum PagingTemporaryMapModes {
    PG_TEMPO_MAP_MODE_PROCESS,
    PG_TEMPO_MAP_MODE_KERNEL
} PagingTemporaryMapModes;

```

------------------------------------

## System call stack in User and Kernel processes
categories: note, obsolete, x86
_9 April 2024_

NEED TO BE REVISED

### System call - User process

| Register state                         | Where                     |
|----------------------------------------|---------------------------|
| esp: 30fef                             | INT 0x50                  |
| ebp: 30ffb                             |                           |
| Ring 0 stack switched. (Base: 0x22FFF) | At syscall_asm_despatcher |
| 20 bytes of interrupt frame pushed     |                           |
| esp: c0022feb                          |                           |
|----------------------------------------|---------------------------|

* esp: 30fef
* ebp: 30ffb
* INT 0x40
* Ring 0 stack switched. (Base: 0x22FFF)
* 20 bytes of interrupt frame pushed
* esp: c0022feb
* One push eab
* 48 bytes of OS interrupt frame pushed
* esp: c0022fb7
* Push interrupt frame address (argument)
* call sys_dummy_handler
* One push eab
* esp: c0022fab

At the end of the interrupt routine control will reach the instruction after INT 0x40 and at that
time ESP, EBP should be what it was at the start 0x30fef, 0x30ffb.

System call - Kernel process

* esp: c0022f87
* ebp: c0022f93
* INT 0x40
* 12 bytes of interrupt frame pushed
* esp: c0022f7b
* One push eab
* 48 bytes of OS interrupt frame pushed
* esp: c0022fb7
* Push interrupt frame address (argument)
* call sys_dummy_handler
* One push eab
* esp: c0022f3b

At the end of the interrupt routine control will reach the instruction after INT 0x40 and at that
time ESP, EBP should be what it was at the start 0xc0022f87, 0xc0022f93.

------------------------------------

## Different way to switch privilege levels in x86 processors
categories: note, x86
_8 April 2024_

* Call gate - Call gates are used to switch between 16 and 32 bit modes but are not used elsewhere.
* Task gate - The Task gates rely on TSS segments, which are used in bare minimum in MOS.
* Interrupt - We use this kind of gates for both HW and SW interrupts. Other interrupts are not
    processed until the current one exits.
* Trap gates - These are not used. But in general they are same as Interrupt gates, just that they
    allow interrupts to occur while inside an interrupt handler.

------------------------------------

## Strange behaviour because of Stale TLB
categories: note, x86
_3 March 2024_

Cache related errors are strange and sometimes are difficult to debug. Following describes one such
case, though this one was not too difficult to debug. It seemed to be strange to me at first but
then, with a little digging it became apparent what was going on.

##### Setup

* Temporary mapping is done using [768:1023:0] 0xC03FF000.
* PD is at 0x43000 paddress and PDE[768],PTE[0] is 0x44000 paddress.
* TLB invalidation is disabled in paging.c throughout.

##### Observation

1. We get a page aligned physical address. Say that is 0x47000.
2. Map this physical address to the temporary address. That is call `kpg_temporaryMap(0x47000)`.

At this point the memory map looks like the following.

```
0x00000000c0000000-0x00000000c01fffff -> 0x000000000000-0x0000001fffff
0x00000000c0200000-0x00000000c0200fff -> 0x000000046000-0x000000046fff
0x00000000c03ff000-0x00000000c03fffff -> 0x000000047000-0x000000047fff
0x00000000fff00000-0x00000000fff00fff -> 0x000000044000-0x000000044fff
0x00000000fffff000-0x00000000ffffffff -> 0x000000043000-0x000000043fff
```

3. Write something to this address. Say for example `0xF001`

Now at this point if you take a look at the memory map, you see something strange. The correct
behaviour would have been the memory map staying the same as before.

```
0x00000000c0000000-0x00000000c0000fff -> 0x00000000f000-0x00000000ffff
0x00000000c0001000-0x00000000c01fffff -> 0x000000001000-0x0000001fffff
0x00000000c0200000-0x00000000c0200fff -> 0x000000046000-0x000000046fff
0x00000000c03ff000-0x00000000c03fffff -> 0x000000047000-0x000000047fff
0x00000000fff00000-0x00000000fff00fff -> 0x000000044000-0x000000044fff
0x00000000fffff000-0x00000000ffffffff -> 0x000000043000-0x000000043fff
```

We see there there is one more row, this extra one is coming because the 0xC0000000 mapping got
splitted. Where previously 0xC0000000 vaddress mapped to 0x000000000 paddress, it now mapped to
0xF000 paddress.

##### Reason and conclusion

This error which is only seen when all TLB invalidation was disabled, is a TLB/Caching related
error.

For vaddress mapping to change its corresponding PTE must change. 0xC0000000 vaddress, corresponds
to PDE[768],PTE[0] (at 0x44000 paddress) entry. When we wrote to 0xC03FF000 vaddress, 0x44000
paddress got written to instead of 0x47000, which we temporary mapped.

This happened because TLB was not flushed before doing the `kpg_temporaryMap` call and this resulted
in MMU using an old map from TLB. It so happens that there was an older call to temporary map 
0x44000 paddress. Even though we have called `kpg_temporaryUnmap` the TLB was not flushed, and this
this issue.

Note that only when we have a PTE with Present = 1, so we have to flush the TLB. Entires where
Present = 0 need not require flushing.
