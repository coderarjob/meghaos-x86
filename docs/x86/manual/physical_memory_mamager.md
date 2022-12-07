## Megha Operating System V2 - x86
## Physical Memory allocation
------------------------------------------------------------------------------
__04 December 2022__

### Summary:

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

### Basic Idea: Page Allocation Bitmap

The OS need to keep track of which physical/actual memory is in use and which is free. There can be
many ways one can achieve this. In MeghaOS I am using a 4096 byte bitmap - I call this bitmap Page
Allocation Bitmap (PAB).

Each bit in this bitmap is a physical page (each of 4096 bytes) - a 1 bit means the page is in
use/allocated and 0 means it is free.

The physical pages and virtual pages are of the same size, because this will help to map one on one
a physical page to a virtual page. Makes life simpler.

The PMM module has routines to allocate and free these physical pages.

### Size of PAB

Size of PAB determines the maximum physical RAM that can be accessed.

```
PAB            RAM supported     Comments
-----------    --------------    -------------------------------------------------------
32   bytes     1   MB            Absolute minimum, but not practical. No space for kernel.
64   bytes     2   MB            1 MB base RAM + 1 MB for the OS (Realistic estimate)
4096 bytes     128 MB            The largest RAM supported in 1 page allocation for PAB.
```

Lets explore the RAM requirement of MeghaOS to get a sense of how much RAM may be required.
* The kernel is loaded at 1MB, so that plus the kernel size is the lower limit.

* Given the minimal nature of MeghaOS, I guess the combined size of kernel, kernel modules and
  system tools will not exceed 1MB.

* Only when more advanced/graphical software is added, will the RAM requirement will exceed 2 MB.

It is this safe to assume, that the **minimum RAM which is required is 2 MB**. But what is the
maximum and thus the PAB size.

Looking at the table above I think the best bet is to start with a PAB of size 4096 bytes
(this the above mentioned 4096 byte bitmap).

PAB size: **4096 bytes**

Minimum RAM: **2 MB**

Maximum RAM: **128 MB**

### PAB structure

Here is the representation of the bits in memory

```
PAB_BYTE_INDEX = pageFrameIndex / 8
PAB_BIT_INDEX = pageFrameIndex % 8

     |---------------------------------------------------------------|
PAB: |7      |6      |5      |4      |3      |2      |1      |0      |      <-- byte 0
     |---------------------------------------------------------------|
     |page 7 |page 6 |page 5 |page 4 |page 3 |page 2 |page 1 |page 0 |
     |---------------------------------------------------------------|
     |page 15|page 14|page 13|page 12|page 11|page 10|page 9 |page 8 |
     |---------------------------------------------------------------|
PAB: |7      |6      |5      |4      |3      |2      |1      |0      |      <-- byte 1
     |---------------------------------------------------------------|
```

So Page frame 13 is Bit 5 Byte 1.

### PAB location

PAB physical address is calculated dynamically in entry.s. It is placed after kernel, kernel
modules, kernel PD, kernel PT.

In the example below it is stored at physical location 0x106000. Because of higher half mapping the
kernel, the virtual address of PAB will thus be 0xC0106000.

### PAB initialization

**System Info:**

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

#### Procedure

```
    PAB_SIZE_BYTES  = 4096
    PAGE_SIZE_BYTES = 1024

    kpmm_init ->
        memcpy (pab, 0xFF, PAB_SIZE_BYTES)

        for each item in memory_map_items
            if (item.type != Free) then continue

            start  = item.baseAddr
            length = item.length

            if (start < PAGE_SIZE_BYTES) then
                adjust (&start, &length)

            pages = To_Pages (ALIGN_DOWN (length, PAGE_SIZE_BYTES))
            kpmm_free (start, pages, NON_DMA);
```

#### Step 1

Whole PAB is initialized to 0xFF. This marks every physical page as allocated (note that memory the
PAB covers can differ from the amount of RAM the system actually has).

#### Step 2

##### Processing memory map entry #1

Now the memory map entries are consulted and free memory is marked as such in the PAB.

```
memory map entry #0:

Start    Length  Type
-------- ------- ------
0        9FC00    1
```

###### Step 2.1 - Handle byte address zero

Note that the `baseAddr` is `0x0000`, which is the `NULL` address.

Physical address zero is special, access to that location causes panic and processor is halted. So
before this can be passed to `kpmm_free`, the `baseAddr` and `length` are modified - `baseAddr` has
is now `0x1000` (start of the 2nd page) and `length` is decreased by the same amount to keep the end
address same (see below section for more details).

Calculations are as follows:

```
    kpmm_init (start, length) ->
        if (start < 4096)
            distance = 0x1000 - start

            // If the whole block lies within the 1st page, then we skip
            if (length <= distance) continue

            *start  = 0x1000
            *length = length - delta_start
```

Post adjustment (see below section) the base and length values come as follows.

```
memory map entry #0 (adjusted):
    baseAddr:   0x01000
    length  :   0x9EC00
```

###### Step 2.2 - Alignment to page boundary

`kpmm_free` takes two major parameters, one a page aligned start address and number of pages to free
from that address.

* The `baseAddr` is already aligned,
* We need to convert the `length in bytes` to `length in pages`.

The `length` is first `down aligned` to make it aligned to page boundary, then a simple division by
`PAGE_SIZE_BYTES` will yield the number of pages.

```
    start       :   0x01000
    length      :   0x9EC00
    aligned     :   0x9E000   <--- align_down (length, PAGE_SIZE_BYTES)
    num pages   :       158   <--- aligned / PAGE_SIZE_BYTES

    kpmm_free (0x1000, 158, NO_DRM)
```

This will make changes to PAB to indicate that `158 pages` (`0x9E000` bytes) starting at physical
address `0x1000` are free.

###### Step 2.3 - kpmm_free

Each bit in PAB represents one page. So before we can mark these free areas in PAB, the bit which
represents the pages need to be calculated. So here they are.

```
PAB byte   = page / 8
PAB bit    = page % 8
num pages  = length / PAGE_SIZE_BYTES
end addr   = start addr + num pages * PAGE_SIZE_BYTES - 1

start addr = 0x01000
num pages  =     158
end addr   = 0x9EFFF


argument  address   page   PAB byte   PAB bit
                    index  index      index
--------- -------   -----  ---------  --------
start     0x01000      1     0         1 --------> bit 1 (0 indexed) of byte 0 in PAB
end       0x9EFFF    158    19         6 --------> bit 6             of byte 19 in PAB
```

Thus in order to mark free memory starting at address `0x1000` and ending at address `0x9DFFF`, the
PAB is cleared from `Bit 1 of Byte 0` until `Bit 6 of Byte 19`.

* `byte 19` is `0xC0106000 + 19 = 0xC0106013`.

This checks out as memory starting from address `0xC0106000:1` until (and including) `0xC0106013:6`
is set to `0x0`.

Below is the dump of PAB after initialization.

```
0xc0106000:     0x01    0x00    0x00    0x00    0x00    0x00    0x00    0x00
0xc0106008:     0x00    0x00    0x00    0x00    0x00    0x00    0x00    0x00
0xc0106010:     0x00    0x00    0x00    0x80    0xff    0xff    0xff    0xff
0xc0106018:     0xff    0xff    0xff    0xff    0xff    0xff    0xff    0xff
0xc0106020:     0x00    0x00    0x00    0x00    0x00    0x00    0x00    0x00
....
0xc0106090:     0x00    0x00    0x00    0x00    0x00    0x00    0x00    0x00
0xc0106098:     0x00    0x00    0x00    0x00    0xff    0xff    0xff    0xff
0xc01060a0:     0xff    0xff    0xff    0xff    0xff    0xff    0xff    0xff
0xc01060a8:     0xff    0xff    0xff    0xff    0xff    0xff    0xff    0xff
0xc01060b0:     0xff    0xff    0xff    0xff
...
0xc0106ff0:     0xff    0xff    0xff    0xff    0xff    0xff    0xff    0xff
0xc0106ff8:     0xff    0xff    0xff    0xff    0xff    0xff    0xff    0xff
0xc0107000:     0x00    0x00    0x00    0x00    0x00    0x00    0x00    0x00
0xc0107008:     0x00    0x00    0x00    0x00    0x00    0x00    0x00    0x00
```
##### PAB state after processing memory map entry #4

```
Memory map entry[3]:

Start    Length  Type
-------- ------- ------
100000   3E0000   1
```

```
PAB byte   = page / 8
PAB bit    = page % 8
num pages  = length / PAGE_SIZE_BYTES
end addr   = start addr + num pages * PAGE_SIZE_BYTES - 1

start addr = 0x100000
length     = 0x3E0000       <--- already aligned
num pages  =      992
end addr   = 0x4DFFFF

argument  address   page   PAB byte   PAB bit
                    index  index      index
--------- --------  -----  ---------  --------
start     0x100000   256    32        0 --------> bit 0 of byte 32 in PAB
end       0x4DFFFF  1247   155        7 --------> bit 7 of byte 157 in PAB
```

Thus in order to mark free memory starting at address `0x100000` and ending at address `0x4DFFFF`
the PAB is cleared from `Bit 0 of Byte 32` until (and including) `Bit 7 of Byte 155`.

* `byte  32` is `0xC0106000 + 32  = 0xC0106020`.
* `byte 155` is `0xC0106000 + 155 = 0xC010609B`.

This checks out as memory starting from address `0xC0106020:0` until (and including) `0xC010609B:7`
is set to `0x0`.
