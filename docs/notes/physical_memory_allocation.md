## Megha Operating System V2 - x86
## Physical Memory allocation
------------------------------------------------------------------------------
__19 Nov 2022__

With the latest change to the PMM, the physical address 0x0000 is now invalid. Think of it as `NULL`
for PMM.  Making physical addresses 0x0000 an invalid address has several benefits.
* This makes the 0x0000 address perfect for initializing variables.
* Functions can return this address to indicate an error.

Note that only the 0x0000 address is invalid, but the whole 1st page can not not be used
(allocated/freed) by PMM. This is because
* `kpmm_alloc` only returns page aligned addresses, as 0x0000 is invlaid, the first valid address it
  can return is 0x1000 - Start of the 2nd page.
* `kpmm_allocAt` and `kpmm_free` only accepts page aligned addresses, as 0x0000 is invlaid, the
   first address it accepts is 0x1000 - Start of the 2nd page.

### Modifying PAB based on BootInfo items

One sideeffect of making 0x0000 an invalid address, is now we have to handle the case where a
BootInfo item starts at location 0. The case can be seen below.

##### Console log
```
I: Freeing startAddress: 0, byteCount: 9FC00, pageFrames: 159.
I: Freeing 0x9F000 bytes starting physical address 0x0.
I: Freeing startAddress: 100000, byteCount: 3E0000, pageFrames: 992.
I: Freeing 0x3E0000 bytes starting physical address 0x100000.
I: Allocate startAddress: 100000, byteCount: 2CC8, pageFrames: 3.
I: Allocating 0x3000 bytes starting physical address 0x100000.
I: Allocating 0x1000 bytes starting physical address 0x0.
GDT {size = 0x1F, location = 0xC0000800}
GDT {size = 0x2F, location = 0xC0000800}
IDT {limit = 0x7FF, location = 0xC0000000}
```

The first BootInfo item shows a free block of 0x9FC00 bytes, starting at location 0. Now as
location 0x0000 is not valid, calling `kalloc_free (0x0000, 0x9FC000, false)` will cause error.

##### Console log: Before

```
I: Freeing startAddress: 1000, byteCount: 9EC00, pageFrames: 158.
I: Freeing 0x9E000 bytes starting physical address 0x1000.
I: Freeing startAddress: 100000, byteCount: 3E0000, pageFrames: 992.
I: Freeing 0x3E0000 bytes starting physical address 0x100000.
I: Allocate startAddress: 100000, byteCount: 2DA8, pageFrames: 3.
I: Allocating 0x3000 bytes starting physical address 0x100000.
GDT {size = 0x1F, location = 0xC0001800}
GDT {size = 0x2F, location = 0xC0001800}
IDT {limit = 0x7FF, location = 0xC0001000}
```

What I do in this case, is to change the start address to the beginning of the next page, (i.e.
0x1000) and decrease length by the same amount as the increase in the start address.

Calculations are as follows:

```
    Given 'start' and 'length'

    if (start < 4096)
        start_mod = 0x1000
        delta_start = 0x1000 - start
        length_mod = length - delta_start

        // If the whole block lies within the 1st page, then we skip
        if (length_mod <= 0) continue

```

##### Examples

```
                    Start   Length   End
                    ------- -------- -------
BootInfo Item[0]    0x0000  0x9FC00  0x9FC00
Modified            0x1000  0x9EC00  0x9FC00

                    Start   Length   End
                    ------- -------- -------
BootInfo Item[0]    0x0100  0x9FC00  0x9FD00
Modified            0x1000  0x9ED00  0x9FD00

                    Start   Length   End
                    ------- -------- -------
BootInfo Item[0]    0x0100  0x00C00  0x00D00
Modified            0x1000 -0x00300     -       Skipping

                    Start   Length   End
                    ------- -------- -------
BootInfo Item[0]    0x0FFF  0x00001  0x01000
Modified            0x1000  0x00000     -       Skipping
```

##### PAB dump: After

The end effect on the PAB is the same - same pages are marked allocated. Here is a dump of the PAB
and will match exactly the dump from previous (see below).

```
0xc0105000:     0x01    0x00    0x00    0x00    0x00    0x00    0x00    0x00
0xc0105008:     0x00    0x00    0x00    0x00    0x00    0x00    0x00    0x00
0xc0105010:     0x00    0x00    0x00    0x80    0xff    0xff    0xff    0xff
0xc0105018:     0xff    0xff    0xff    0xff    0xff    0xff    0xff    0xff
0xc0105020:     0x07    0x00    0x00    0x00    0x00    0x00    0x00    0x00
```

##### PAB Dump: Before

```
0xc0105000:     0x01    0x00    0x00    0x00    0x00    0x00    0x00    0x00
0xc0105008:     0x00    0x00    0x00    0x00    0x00    0x00    0x00    0x00
0xc0105010:     0x00    0x00    0x00    0x80    0xff    0xff    0xff    0xff
0xc0105018:     0xff    0xff    0xff    0xff    0xff    0xff    0xff    0xff
0xc0105020:     0x07    0x00    0x00    0x00    0x00    0x00    0x00    0x00
```

------------------------------------------------------------------------------
__29 July 2022__

Here is a dry run of both the freeing and allocation procedures.

First things first - Few basics and system configurations.
```
Macro name                     Value(DEC)  Value(Hex)  Unit  Calculation
---------------------------    ----------- ---------- ---- ------------------------------------------
PAB_SIZE_BYTES                 4096        0x1000     byte
MAX_ADDRESSABLE_PAGE_COUNT     32768       0x8000     page PAB_SIZE_BYTES * 8 bytes/page
MAX_ADDRESSABLE_BYTE_COUNT     134217728   0x8000000  byte MAX_ADDRESSABLE_PAGE_COUNT * PAB_SIZE_BYTES
MAX_ADDRESSABLE_BYTE           134217727   0x7FFFFFF  byte MAX_ADDRESSABLE_BYTE_COUNT - 1
MAX_ADDRESSABLE_PAGE           32767       0x7FFF     page MAX_ADDRESSABLE_PAGE_COUNT - 1
MAX_DMA_BYTE_COUNT             16777216    0x1000000  byte Fixed 16 MiB
MAX_DMA_PAGE_COUNT             4096        0x1000     page MAX_DMA_BYTE_COUNT / PAB_SIZE_BYTES
MAX_DMA_ADDRESSABLE_BYTE_COUNT 16777216    0x1000000  byte Min(MAX_DMA_BYTE_COUNT, MAX_ADDRESSABLE_BYTE_COUNT)
MAX_DMA_ADDRESSABLE_PAGE_COUNT 4096        0x1000     page Min(MAX_DMA_PAGE_COUNT, MAX_ADDRESSABLE_PAGE_COUNT)
MAX_DMA_ADDRESSABLE_BYTE       16777215    0xFFFFFF   byte MAX_DMA_ADDRESSABLE_BYTE_COUNT - 1
MAX_DMA_ADDRESSABLE_PAGE       4095        0xFFF      page MAX_DMA_ADDRESSABLE_PAGE_COUNT - 1
```

```
RAM Reported        = 4F0000 bytes  (5056 KiB)
Page count          = 1264

Memory map:

Start    Length  Type
-------- ------- ------
0        9FC00    1
9FC00    400      2
F0000    10000    2
100000   3E0000   1
4E0000   20000    2
FFFC0000 40000    2
```

**TODO:** RAM size calculation is wrong. It should be one below.

```
((9fc00 − 0) + (f0000 − 9fc00) + (100000 − f0000) + (4e0000 − 100000) + 20000)/400/400 = 5 MiB
```

It does not effect our calculation, so lets continue.

Few more basics:
```
PAB_BYTE_INDEX = pageFrameIndex / 8
PAB_BIT_INDEX = pageFrameIndex % 8

Here is the representation of the bits in memory

     |---------------------------------------------------------------|
PAB: |7      |6      |5      |4      |3      |2      |1      |0      |      <-- byte 0
     |---------------------------------------------------------------|
     |page 7 |page 6 |page 5 |page 4 |page 3 |page 2 |page 1 |page 0 |
     |---------------------------------------------------------------|
     |page 15|page 14|page 13|page 12|page 11|page 10|page 9 |page 8 |
     |---------------------------------------------------------------|
PAB: |7      |6      |5      |4      |3      |2      |1      |0      |      <-- byte 1
     |---------------------------------------------------------------|

So Page frame 13 is Bit 5 Byte 1.

```

### Initialization

In the beginning, all the pages are marked allocated (written 1 to every bit in PAB). Then pages are
marked free based on the BIOS memory map (those with status 1).

The pages where the kernel modules are loaded (by boot1), are then marked allocated. As these
modules, including the Kernel image is loaded at 0x100000, allocated pages start from
`pageFrameIndex` = 256 i.e `(0x100000/4096)`. 

Taking an example:
The Kernel today is of size 0x2BE8, so a 3 pages need to be allocated (after rounding up).

```
page frame index = 256
byte index       = 256 / 8 = 0x20
bit index        = 256 % 8 = 0x00

0xc0105000:     0x01    0x00    0x00    0x00    0x00    0x00    0x00    0x00
0xc0105008:     0x00    0x00    0x00    0x00    0x00    0x00    0x00    0x00
0xc0105010:     0x00    0x00    0x00    0x80    0xff    0xff    0xff    0xff
0xc0105018:     0xff    0xff    0xff    0xff    0xff    0xff    0xff    0xff
0xc0105020:    [0x07]   0x00    0x00    0x00    0x00    0x00    0x00    0x00
```

Notice the value 0x07, for the 3 pages allocated.

**TODO:** The initialization up to this point is complete. But now I must allocate for the Kernel 
Stack, VGA Buffer and Boot Info structures.

Note that the very first page of PAB is set, this is to mark allocated memory for the IDT and GDT.
It is only temporary, later on almost every allocations; from Stack, IDT, GDT etc will be done 
dynamically using the Virtual Memory Manager.

### Steps: Page allocation

Use the `kpmm_alloc` to allocate consecutive physical pages. When `PMM_NORMAL` or `PMM_DMA` is
passed, it will search the PAB for the first block of memory large enough for all the pages,
otherwise when `PMM_FIXED` is passed, the physical pages are allocated starting from the page
aligned physical address that was also passed in.

The PAB is always of size `PAB_SIZE_BYTES`, irrespective of the actual amount of physical
RAM in the system. This determines the maximum RAM that MeghaOS can access. With `PAB_SIZE_BYTES` 
of 4096, the maximum RAM that MeghaOS can access is **128 MiB** (This is the
`MAX_ADDRESSABLE_PAGE_COUNT`).

So in systems which less that `MAX_ADDRESSABLE_BYTE_COUNT` bytes of RAM, the rest of the PAB will 
always be set. **MeghaOS must never allocate memory that is not present or it cannot access.**

DMA hardware has a limitation. It may only be able to access `MAX_DMA_BYTE_COUNT` bytes. In x86
systems this is 16 MiB. So in systems which less that `MAX_DMA_BYTE_COUNT` bytes of RAM,
`PMM_NORMAL` and `PMM_DMA` allocate in the same range. 

On success `EXIT_SUCCESS` is returned, and the physical address is returned in the `allocated`
parameter. This address is always page aligned.

On error `EXIT_FAILURE` is returned. The exact cause of the error can found by reading the
`k_errorNumber` global variable.

### Steps: Page de-allocation

Use the `kpmm_free` to deallocate continuous physical pages. The first argument is a page aligned
physical address, the second is number of pages to free.

Here deallocation is means that the bit representing the page in PAB is unset.

On success `EXIT_SUCCESS` is returned, and the physical address is returned in the `allocated`
parameter. This address is always page aligned.

On error `EXIT_FAILURE` is returned. The exact cause of the error can found by reading the
`k_errorNumber` global variable.

------------------------------------------------------------------------------
_20 July 2022__

Notes on the API:

```
enum AllocTypes {
    FIRST_FREE, /* start is ignored. First largest free pages will be allocated. */
    FIXED       /* start must be provided. Must be 4KB aligned address. */
}

PADDR kpmm_alloc (INT size, AllocTypes type, INT start);
UINT kpmm_free (PADDR address, INT size);
```
GDT, IDT, DISP are all responsible to call `kpmm_alloc` to allocate the `FIXED` addresses for them.
Or will they call virtual memory allocator methods??

------------------------------------------------------------------------------
_19 July 2022__

Size of PAB array determines the maximum physical RAM that can be accessed. Each bit in this array
represents 4KB of memory (i.e. one page). This means, one cannot allocate less than 4KB.

Windows 95 required 4MB of RAM at minimum, while 8MB was recommended. There is no way that MeghaOS
should require close to such an amount. 8MB corresponds to a PAB size of just (8 * 1024)/4 = 256
bytes.

As physical allocator can only assign multiples of 4KB, allocating anything that does not end at a
page boundary caused waste. PAB of 256 bytes means (4096 - 256) = 3.75 KB of memory is wasted in
that page. However this waste is not significant, as mentioned below.

Lets explore the RAM requirement of MeghaOS to get a sense of how much RAM may be required.
* The kernel is loaded at 1MB, so that plus the kernel size is the lower limit.
* Given the minimal nature of MeghaOS, I guess the combined size of kernel, kernel modules and
  system tools will not exceed 1MB.
* Only when more advanced/graphical software is added, will the RAM requirement will exceed 2 MB.

This means, that the realistic assumption at this point is to support just 2MB of RAM.

Here is a table with the range PAB size can take.

```
PAB (Minimum)     RAM supported        Comments
--------------    --------------       -------------------------------------------------------
32   Bytes        1   MB               Absolute minimum, but not practical. No space for kernel.
64   Bytes        2   MB               1 MB base RAM + 1 MB for the OS (Realistic estimate)
4096 Bytes        128 MB               The largest RAM supported in 1 page allocation for PAB.
```
The actual size will thus be somewhere in between 32 and 64 bytes in the beginning, then can be
increased as and when required. Note than PAB of size 4096 bytes, will have the minimum waste. It
is ironic, how supporting less RAM causes more waste, and this waste decreased as more RAM support
is provided.

```
PAB     Total Supported     Waste     Waste %
        RAM
------  ----------------    -------   ----------
32      1MB                 4064      0.3
64      2MB                 4032      0.2
4096    12MB                0         0.0

```
But as one can see, the largest waste is just 0.3% of the total supported RAM.

The actual size will be controlled by a MACRO definition.
