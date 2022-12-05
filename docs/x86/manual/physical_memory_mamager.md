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

The first step is to mark every page as allocated (by writting 1 to every bit in PAB). Then pages
are marked free based on the BIOS memory map (those with status 1).

At this point PMM is said to be initialized.

Physical address zero is special, access to that location causes panic and processor is halted. So
care is taken when freeing (as part of initialization) a section that starts at physical location 0.

You can skip the below and go straight to to #PAB state after initialization section.

#### System Into (for the demonstration below)

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

#### Handling address 0 when Initializing PAB

The 1st memory map entry is a section which is of size 0x9FC00 bytes and starts at 0.
Now as location 0x0000 is not valid, calling `kalloc_free (0x0000, 0x9FC000, false)` will cause
error.

What I do in this case, is to change the start address to the beginning of the next page, (i.e.
0x1000) and decrease length by the same amount to keep the end address same.

As you see from the memory map above, the 1st map entry

Calculations are as follows:

```
    Given 'start' and 'length'

    if (start < 4096)
        distance = 0x1000 - start

        // If the whole block lies within the 1st page, then we skip
        if (length <= distance) continue

        start_mod = 0x1000
        length_mod = length - delta_start
```

##### Examples

```
        Original    Modified
        ----------  -----------
Start  | 0x0000     0x1000
Length | 0x9FC00    0x9EC00
End    | 0x9FC00    0x9FC00
        ----------  -----------
Start  | 0x0100        -            <-- Skipped as the whole section is within the 1st page.
Length | 0x00C00       -
End    | 0x00D00       -
        ----------  -----------
Start  | 0x0FFF        -            <-- Skipped as the whole section is within the 1st page.
Length | 0x0001        -
End    | 0x1000        -
        ----------  -----------
Start  | 0x0FFF     0x1000
Length | 0x0002     0x0001
End    | 0x1001     0x1001
```

#### PAB state after initialization

As an example, I will take the first memory map (one below).

```
        Original    Modified
        ----------  -----------
Start  | 0x0000     0x1000
Length | 0x9FC00    0x9EC00
End    | 0x9FC00    0x9FC00
```

The calculation for the values in the 'Modified' column is explained above. The

```
0xc0106000:     0x01    0x00    0x00    0x00    0x00    0x00    0x00    0x00
0xc0106008:     0x00    0x00    0x00    0x00    0x00    0x00    0x00    0x00
0xc0106010:     0x00    0x00    0x00    0x80    0xff    0xff    0xff    0xff
0xc0106018:     0xff    0xff    0xff    0xff    0xff    0xff    0xff    0xff
0xc0106020:     0x00    0x00    0x00    0x00    0x00    0x00    0x00    0x00
```

5 MB of memory, which is
