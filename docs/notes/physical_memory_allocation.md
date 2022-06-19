## Megha Operating System V2 - x86
## Physical Memory allocation
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
