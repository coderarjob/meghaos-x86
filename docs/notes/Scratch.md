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

## GCC definitions for CM and Applications
categories: note, independent
22 Dec 2024

CM is build with the same definitions as the Kernel. This is so done to enable/disable features that
are also present/absent in the Kernel. Applications should be oblivious of these definitions. This
is trivial knowledge, but what is not trivial are CM header files which are common to both CM and
applications. Problem arises when we check for a build option in the CM header files. Say for DEBUG
builds of the kernel, some debug specific function is enabled in the CM library for applications to
use, but the application should not have to have DEBUG definitions set. In this scenario any `ifdef
DEBUG` block in a CM header would not be enabled for the application since DEBUG definitions is not
set for the application.

### Solution 1
Have CMAKE generate a `cm_build.h` header file which would include the CM build definitions. Every
header in the CM library would include this `cm_build.h` file and will be able know the definitions
CM library was build with.

### Solution 2
Do not have any `ifdef` block in the header so that the type/function definition to be available
always, but when the build option is not enabled have a dummy function of the same name. The dummy
function is required because applications cannot know before calling a function if its defined or
not, since the application would not know the condition for the function to be defined and even if
it does, it should not depend on them as the condition may change in future.

I think its obvious which solution to go with - Solution 1.

------------------------------------

## Logging & Debugging a graphical OS
categories: note, independent
16 Nov 2024

The previous note on this topic stated that `kearly_printf` should replace `kdebug_printf` and that
macros like `INFO` should also use `kearly_printf`. It also stated that `kdebug_printf` should print
to VGA Text/VirtualScreen along with port 0xE9. Here are the problems this would create:
* Throwing all the debug logs at the screen (and to VirtualScreen) would just create a wall of text
  and I am not sure how would that be any helpful.
* Since `kearly_printf` will be dumping everything on the screen, the debug programs would not be
  able to show any structured output.

So there is the new proposal. To keep both `kearly_printf` and `kdebug_printf` each having separate
purpose. The former can be used for structured output on the screen (and to VirtualScreen), while
the later be used for debug logging. Both of these will exist only for debug builds, but separating
it this way would allow debug specific programs/tests to use the screen for some sensible output.

Sure this means that `kdebug_printf` is of no use for standalone debugging, but the previous
proposal also would not have been be any better. For standalone debugging it would make sense to
have `kdebug_printf` to print to serial port. But that can come later on.

Here is the new proposal to make the OS Graphical yet would allow text mode/debugging programs to
run and produce output both for non-graphical & graphical builds.

1. `kearly_printf`, `vgatext` function only be defined for DEBUG builds.
2. `vgatext` function only be defined for DEBUG, non-graphical builds.
3. `VirtualScreen` would be defined for DEBUG, graphical builds.
4. `kearly_printf` will write to VGA text if non-graphical, or to VirtualScreen for graphical
   builds. VirtualScreen will display the text in a 'Window'. It would flush out the text (repaint)
   as soon as its written.
5. `kdebug_printf` will also be defined for DEBUG builds with `E9_ENABLED` set. It will (as of now)
   will print to the host terminal using port 0xE9. Whatever is printed using `kearly_printf` will
   also be passed to `kdebug_printf`.
6. `DEBUG_LEVEL` will be removed as `E9_ENABLED` macro makes it redundant.


```
|-------|------------|-----------|---------------|---------------|---------------|---------|
| Debug | E9_ENABLED | Graphical | kearly_printf | kdebug_printf | kdisp_putc    | e9_putc |
|-------|------------|-----------|---------------|---------------|---------------|---------|
| No    | No         | No        | invalid       | invalid       | invalid       | invalid |
| Yes   | No         | No        | kdisp_putc    | No            | Yes           | No      |
| No    | Yes        | No        | invalid       | invalid       | invalid       | invalid |
| Yes   | Yes        | No        | kdisp_putc    | Yes           | Yes           | Yes     |
|-------|------------|-----------|---------------|---------------|---------------|---------|
| Debug | E9_ENABLED | Graphical | kearly_printf | kdebug_printf | VirtualScreen | e9_putc |
|-------|------------|-----------|---------------|---------------|---------------|---------|
| No    | No         | Yes       | No            | No            | No            | No      |
| Yes   | No         | Yes       | VritualScreen | No            | Yes           | No      |
| No    | Yes        | Yes       | invalid       | invalid       | invalid       | invalid |
| Yes   | Yes        | Yes       | VirtualScreen | e9_putc       | Yes           | Yes     |
|-------|------------|-----------|---------------|---------------|---------------|---------|

kdisp_putc = (Debug & !Graphical)
e9_putc    = (Debug & E9_ENABLED)

VirtualScreen = (Debug & Graphical)

kearly_printf = (kdisp_putc | VirtualScreen) = (Debug)
kdebug_printf = (e9_putc) = (Debug & E9_ENABLED)
```

As with `kdisp_importantPrint`, its of no use. `kpanic` will
* NDEBUG & Graphical -> Print panic text directly to vesa framebuffer.
* DEBUG & Graphical  -> + `kdebug_printf`
* DEBUG & Text       -> Print panic text to `kdebug_printf` + `kearly_printf`.

#### Unittests

Also note that these above rules & build options are not for Unittests. It is required to test even
the debug components, so the `ifdef` guards for debug specific components must allow when building
unittests.

#### Applications

Since AppLib is built along with the Kenrel, it can make graphical functions not defined for
non-graphical Kernel and VGA Text function not defined for graphical Kernel.

Applications can ask AppLib to check graphical mode is available and take suitable steps.

------------------------------------

## Logging & Debugging a graphical OS
categories: note, independent, obsolete
13 Nov 2024

Since I want the OS to be primarily Graphical and text mode for only debugging purposes, some design
changes is required.

1. Functions which can only be used in text mode would become debug only.
2. Few functions which were used specific for debug printing becomes redundant & can be removed.

`kearly_printf` function which prints only works in text mode thus now be treated as a DEBUG only
function. `kdebug_printf` thus now becomes redundant. `kdisp_importantPrint` also has no purpose
now.

The `kearly_printf` would now be printing to any of these 3 destinations:
* `kdisp_putc` -> When in debug text mode, prints to VGA text mode buffer
* `kdscreen_putc` -> When in debug graphical mode, prints to Debug Screen (new debug feature)
* `kdebug_qemu_putc` -> When in debug & `E9_ENABLED` mode, prints to host console using port 0xE9.

Note debug mode would work in both text & graphical mode. So there would be two build options:
1. BUILD = (debug|ndebug)
2. UI = (debug/text|graphical)
3. E9_ENABLED = (true|false)

All macros `INFO`, `ERROR` now will use `kearly_printf`.

`k_panic` would use graphical implementation when in graphical mode and also print using
`kearly_printf` in debug mode. For debug text mode `k_panic` would only print using `kearly_printf`.

------------------------------------

## GUI basics in MeghaOS
categories: note, independent
21 Oct 2024

GraphicsSurface (Per application)
|--(??)----------> GraphicsElement (Per UI element)
|--(Compositor)--> Back Buffer (OS)
|--(Video driver)-> Framebuffer (Video driver)

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

## New Git workflow
categories: note, independent
29 Aug 2024

I used to do development in the 'kernel/develop' branch and after a substantial amount of work I
would merge it to the 'master'. However I am rethinking this approach and proposing either all
development be done in the 'master' or in some short lived feature branch.

### Development only in the 'master' branch:

#### Advantages:
1. Visitors to my GitHub page will be able to see the most recent state of the project and not
   how it was months ago.
2. Since I am the only person developing, there is not reason to have a separate long-running 'dev'
   branch. I am not doing a release or merging changes from multiple people.
3. Less disruptive since there is no need to merge with the 'master' later.

#### Disadvantages
1. The idea is that the 'master' must always be in 'good' state. However now since every commit goes
   into the 'master' a bad commit would break the 'master'.
2. Some features which replaces an older implementation and would take a long time to complete,
   risks making either the 'master' branch dirty (with both old and new implementations coexisting,
   or the documentation stale). For example if I make the transition to CMake in the 'master'
   branch then for sometime the branch would have files for both 'build.sh' and CMake build. Worse
   would be when I start deleting the 'build.sh' files and the CMake build not yet fully ready. That
   surely meant that the build system is completely broken.

### Development in the 'master' & 'feature' branches:

#### Advantages:
1. Visitors to my GitHub page will be able to see a relatively recent state of the project and not
   how it was months ago. The state of the 'master' would be the last good.
2. All other benefits from development in the 'master' branch.
3. The disadvantages mentioned about stability is also reduced. I would not merge with the 'master'
   until the feature is ready. But it would also not run more than few weeks.
4. Non-disruptive changes would not require a 'feature' branch and can be done all in the 'master'
   branch. If required they can be controlled using a 'Feature flag'.
   
#### Disadvantages
1. The 'short' lived feature branches could still go on for a long time, causing the same problem it
   tries to solve.
2. Stability of the 'master' branch is reduced but cannot completely be eliminated.

Read more in the stackexchange question: 
https://softwareengineering.stackexchange.com/questions/454794/git-trunk-based-development-for-change-to-build-sytem-and-documentation

------------------------------------

## Theory of mixing platform dependent and independent parts
categories: note, x86
23 July 2024

Here are the scnerarios that are possible. The variables are 'Implementation' and 'Use', each can
have to values 'arch dependent' (AD) or 'arch independent' (AID).

|----------------|-----|-----------------------------|-----------------------|
| Implementation | Use | Interface naming convention | Interface declaration |
|----------------|-----|-----------------------------|-----------------------|
| AD             | AD  | 'X86_' prepended            | AD header file        |
| AD             | AID | 'ARCH_' prepended           | AID header file       |
| AID            | AID | Nothing special             | AID header            |
| AID            | AD  | <not possible>              | -                     |
|----------------|-----|-----------------------------|-----------------------|

Now unittests are always arch independent, because they run in environment different from what they
are testing. Depending on what its testing these two scnerarios can occur.

|----------------|------------------------------------------------|
| Implementation | Unittest convention                            |
|----------------|------------------------------------------------|
| AD             | Either mock the AD part or it cannot be tested |
| AID            | Nothing special is required                    |
|----------------|------------------------------------------------|

------------------------------------
## How to test stack overflow/underflow checks
categories: note, x86
05 August 2024

```c
    // Stack pointer decremented - should hit the stack bottom boundary
    volatile U8 ar;
    volatile U8* a = &ar;
    for (SIZE i = 0;  i < (256 * KB); i++) {
        *a-- = 19;
    }

    // Stack pointer incremented - should hit the stack top boundary
    volatile U8 ar;
    volatile U8* a = &ar;
    for (SIZE i = 0;  i < (256 * KB); i++) {
        *a++ = 19;
    }
```
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

## System call frame
categories: note, x86
_17 April 2024_

```
|------------------|----------|
| Interrupt frame  |          |
|------------------|----------|
| Offset           | Register |
|------------------|----------|
| 40               | SS       |
| 36               | ESP      |
| 32               | EFLAGS   |
| 28               | CS       |
| 24               | EIP      |
|------------------|----------|
| OS Syscall frame |          |
|------------------|----------|
| 20               | EBP      |
| 16               | EDI      |
| 12               | ESI      |
| 08               | EDX      |
| 04               | ECX      |
| 00               | EBX      |
|------------------|----------|

Interrupt Frame Size : 4 * 5 = 20 bytes
OS Syscall Frame Size: 4 * 6 = 24 bytes
Total                :         44 bytes
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
