## Megha Operating System V2 - x86
------------------------------------------------------------------------------

## Physical Memory allocation
categories: feature, x86
_19 July 2023_

### Summary:

```
Macro name                   Value(Hex)   Calculation
---------------------------  ---------- ------------------------------------------
PAB_BITS_PER_STATE           2          Constant
CONFIG_PAB_NUMBER_OF_PAGES   1          Constant
CONFIG_PAGE_FRAME_SIZE_BYTES 0x1000     Constant

PAB_SIZE_BYTES               0x1000     CONFIG_PAB_NUMBER_OF_PAGES * CONFIG_PAGE_FRAME_SIZE_BYTES
MAX_ADDRESSABLE_PAGE_COUNT   0x4000     PAB_SIZE_BYTES * (8 / PAB_BITS_PER_STATE)
MAX_ADDRESSABLE_BYTE_COUNT   0x4000000  MAX_ADDRESSABLE_PAGE_COUNT * PAB_SIZE_BYTES
MAX_ADDRESSABLE_BYTE         0x3FFFFFF  MAX_ADDRESSABLE_BYTE_COUNT - 1
MAX_ADDRESSABLE_PAGE         0x3FFF     MAX_ADDRESSABLE_PAGE_COUNT - 1
```

### Basic Idea: Page Allocation Bitmap

The OS need to keep track of which physical memory addresses are in use, free or reserved. There can
be many ways one can achieve this. In MeghaOS I am using a bitmap (Page Allocation Bitmap or PAB)
and from the name it may be clear that it stores status of actual physical pages in memory.

The size of this bitmap depends on the configuration, and currently it is 4096 bytes in size.

At this point each physical page can be in 4 states: Free, Used, Reserved and Invalid. Each state is
2 bits in the PAB.

```
State      Value Deccription
---------- ----- -------------------------
Free       0     Page is not being used, but can be allocated by the OS.
Used       1     Page is being used, and can be freed by the OS.
Reserved   2     Memory not marked as usable (1) in the BIOS memory map.
Invalid    3     ** not used for anything at this time **
```

The physical pages and virtual pages are of the same size, because this will help to map one on one
a physical page to a virtual page. Makes life simpler.

The PMM module has routines to allocate and free these physical pages.

### Size of PAB

Size of PAB determines the maximum physical RAM that can be accessed.

```
PAB    RAM
bytes  supported  Comments
-----  ---------  -------------------------------------------------------
64     1   MB     Absolute minimum, but not practical. No space for kernel.
128    2   MB     1 MB base RAM + 1 MB for the OS (Realistic estimate)
4096   64  MB     The largest RAM supported in PAB size == 1 page.

** Using 2 Bits per state.
```

Lets explore the RAM requirement of MeghaOS to get a sense of how much RAM may be required.
* The kernel is loaded at 1MB so that, plus the kernel size is the lower limit.

* Given the minimal nature of MeghaOS, I guess the combined size of kernel, kernel modules and
  system tools will not exceed 1MB.

* Only when more advanced/graphical software is added, will the RAM requirement will exceed 2 MB.

It is this safe to assume, that the **minimum RAM which is required is 2 MB**. But what is the
maximum and thus the PAB size.

Looking at the table above I think the best bet is to start with a PAB of size 4096 bytes.

PAB size: **4096 bytes**

Minimum RAM: **2 MB**

Maximum RAM: **64 MB**

### PAB structure

Here is the representation of the bits in memory

```
STATES_PER_BYTE = 8 / PAB_BITS_PER_STATE
PAB_BYTE_INDEX  = pageFrameIndex / STATES_PER_BYTE
PAB_BIT_INDEX   = (pageFrameIndex % STATES_PER_BYTE) * PAB_BITS_PER_STATE

For PAB_BITS_PER_STATE = 2, STATES_PER_BYTE = 4

PAB_BYTE_INDEX = pageFrameIndex / 4
PAB_BIT_INDEX  = (pageFrameIndex % 4) * 2

     <           BYTE 1                 >  <           BYTE 0               >
     |--------|--------|--------|--------||--------|--------|--------|--------|
PAB: | 7 | 6  | 5  | 4 |  3 | 2 |  1 | 0 || 7 | 6  |  5 | 4 |  3 | 2 |  1 | 0 |
     |--------|--------|--------|--------||--------|--------|--------|--------|
     | page 7 | page 6 | page 5 | page 4 || page 3 | page 2 | page 1 | page 0 |
     |--------|--------|--------|--------||--------|--------|--------|--------|
```

So status for Page frame 6 can be found at byte 1, bit 6-7 in the PAB.

### PAB location

PAB physical address is calculated dynamically in entry.s. It is placed after kernel, kernel
modules, kernel PD, kernel PT.

In the example below it is stored at physical location 0x106000. Because of higher half mapping the
kernel, the virtual address of PAB will thus be 0xC0106000.

### PAB initialization

#### Step 1

The Whole PAB is initialized to 0xAA, which indicates that every physical page is reserved. This
sets up the stage for the second step of initialization.

#### Step 2

Now the memory map entries are consulted and free memory is marked as such in the PAB. Memory
size are clipped to ensure that 'Start + length' do not exceed the amount of installed physical
memory.

Memory mappings are skipped if the 'Start' address is completely outside the installed physical
memory, or when length is less than 1 page size (we cannot free/allocate memory less than one page
size).

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
100000   165000   1
262000   20000    2
FFFC0000 40000    2
```

#### Step 3

Similar to step 2, here the Bootloader module entries are consulted and memory used by module files
are marked USED in the PAB. This step is done separately by the Kernel and not strictly part of PMM
initialization.

In the end the after step 3 PAB looks something like this:

```
Installed RAM bytes: 0x272000 bytes
Installed RAM Pages: 626
Free RAM bytes: 0x1FC000 bytes

C0106000:       0:0     0:0     0:0     0:0     0:0     0:0     0:0     0:0
C0106010:       0:0     0:0     0:0     0:0     0:0     0:0     0:0     0:0
C0106020:       0:0     0:0     0:0     80:0    AA:AA   AA:AA   AA:AA   AA:AA
C0106030:       AA:AA   AA:AA   AA:AA   AA:AA   AA:AA   AA:AA   AA:AA   AA:AA
C0106040:       0:55    0:0     0:0     0:0     0:0     0:0     0:0     0:0
C0106050:       0:0     0:0     0:0     0:0     0:0     0:0     0:0     0:0
C0106060:       0:0     0:0     0:0     0:0     0:0     0:0     0:0     0:0
C0106070:       0:0     0:0     0:0     0:0     0:0     0:0     0:0     0:0
C0106080:       0:0     0:0     0:0     0:0     0:0     0:0     0:0     0:0
C0106090:       0:0     0:0     0:0     0:0     AA:A0   AA:AA   AA:AA   AA:AA
C01060A0:       AA:AA   AA:AA   AA:AA   AA:AA   AA:AA   AA:AA   AA:AA   AA:AA
C01060B0:       AA:AA   AA:AA   AA:AA   AA:AA   AA:AA   AA:AA   AA:AA   AA:AA
C01060C0:       AA:AA   AA:AA   AA:AA   AA:AA   AA:AA   AA:AA   AA:AA   AA:AA
C01060D0:       AA:AA   AA:AA   AA:AA   AA:AA   AA:AA   AA:AA   AA:AA   AA:AA
C01060E0:       AA:AA   AA:AA   AA:AA   AA:AA   AA:AA   AA:AA   AA:AA   AA:AA
```

Kernel binary of size `0x3D68` was loaded at `0x100000`, so we can see that 4 pages are marked used
at offset `0x40` in the PAB (This is the `0x55` at `0xC0106040` address).
