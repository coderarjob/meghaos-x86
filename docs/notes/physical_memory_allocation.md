## Megha Operating System V2 - x86
## Physical Memory allocation
------------------------------------------------------------------------------
_29 July 2022__

Here is a dry run of both the freeing and allocation procedures.

First things first - Few basics and system configurations.
```
PAB_SIZE_BYTES               =  4096 bytes/pab

MAX_ADDRESSABLE_PAGE_COUNT   =  4096 bytes/pab × 8 pages/byte
                             => 32768 pages/pab

MAX_ADDRESSABLE_PAGE (index) = MAX_ADDRESSABLE_PAGE_COUNT - 1
                             => 32767 pages/pab

As each page is one bit in PAB. Number of bits in pab = 32768.
PAB_SIZE_BITS                =  32768 bits/pab


MAX_ADDRESSABLE_BYTE_COUNT   =  MAX_ADDRESSABLE_PAGE_COUNT pages/pab × 4096 bytes/page
                             => 134217728 bytes/pab

MAX_ADDRESSABLE_BYTE (index) = MAX_ADDRESSABLE_BYTE_COUNT - 1
                             => 134217727 byte

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

### Freeing memory - Steps.

#### 1. s_markFreeMemory

Free is a coservative process. We will never free at most the input byte count.

**Input:**
* : Byte count: 0x9FC00 bytes. Location/start address = 0.

**Procedure:**
* Page frame count = floor(0x9FC00/4096) pages.
                   => 159 pages.    <-- This is count.

#### 2. kpmm_free

**Input:**
* Start Address = 0, Page Count = 159.

**Procedure:**
* start page frame = floor(0/4096)
                   => 0 page        <-- This is the start page index (into PAB).

#### 3. s_allocateDeallocatePages

**Input:**
* Start page frame = 0, page frame count = 159, allocate = false (not allocating, freeing)

**Procedure:**
* end page frame = 0 + 159 - 1
                 => 158             <-- This is the end page index (into PAB).
* From 0 to 158 (including 158), call `s_freePage`.

#### 4. s_freePage (first call)

**Input:**
* Page frame index = 0

**Procedure:**
* PAB Byte index = 0 / 8
                 => 0.
* PAB but index = 0 % 8
                => 0.

This clears 0th bit in PAB[0].

#### 4. s_freePage (last call)

**Input:**
* Page frame index = 158

**Procedure:**
* PAB Byte index = floor(158 / 8)
                 => 19.
* PAB but index = 158 % 8
                => 6.

This clears 6th bit in PAB[19].

### End result

This clears bit 0:0 (bit 0, of byte 0) till 19:6 (bit 6, of byte 19). That is only the bit 7 of byte
19 will be set. This is exactly what we see here.

```
0xc0105000:     0x00    0x00    0x00    0x00    0x00    0x00    0x00    0x00
0xc0105008:     0x00    0x00    0x00    0x00    0x00    0x00    0x00    0x00
0xc0105010:     0x00    0x00    0x00    0x80    0xff    0xff    0xff    0xff
0xc0105018:     0xff    0xff    0xff    0xff    0xff    0xff    0xff    0xff
0xc0105020:     0x07    0x00    0x00    0x00    0x00    0x00    0x00    0x00
0xc0105028:     0x00    0x00    0x00    0x00    0x00    0x00    0x00    0x00
0xc0105030:     0x00    0x00
```

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
