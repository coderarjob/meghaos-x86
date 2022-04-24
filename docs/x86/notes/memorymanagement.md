## Megha Operating System V2 - x86
## Memory Mangement
## GDT entries
------------------------------------------------------------------------------

Currently GDT, IDT are all at a fixed memory location. The GDT which is 
initially set by 'boot1', is at physical 0x800 location and IDT is set by the 
kernel at location physical 0x0000.

These fixed addresses, will be moved to new allocations by the memory manager. 
The TSS instance is allocated on the .bss by the linker. This too will be 
moved to an address provided by the MM.

1. Physical page allocation
----------------------------
I partition the physical memory into 4 KByte pages and will use a bitmap 
(Page Allocation Bitmap/PAB) to keep track of free physical memory pages. 
Each bit in this map correspond to a physical memory page. 

The amount of physical memory, which MOS supports, determines the size of PAB.
bitmap_size_byte = (Ram_Bytes/4096)/8
bitmap_size_page = bitmap_size_byte/4096

Below table shows, how PAB size changes with RAM size.
                    |-----------|-------------|-------------|
                    | Ram_Bytes | bitmap_page | bitmap_size |
                    |-----------|-------------|-------------|
                    |     4 GB  |    32       | 128 KB      |
                    |   128 MB  |     1       |   4 KB      |
                    |   256 MB  |     2       |   8 KB      |
                    |    64 MB  |     0.5     |   2 KB      |
                    |    32 MB  |     0.25    |   1 KB      |
                    |-----------|-------------|-------------|

Thus supporting 4 GB of RAM, is not too much of an overhead, just 128 KB of 
PAB. But for a small OS like MOS, at this stage, 4 GB is just overkill. I want 
to start small and grow from there.

The PAB size will thus be determined by the MAX RAM supported by the build.

[!] But how small do I want to start?
32 MB. Why not!

[!] Where should PAB reside? In the bss, allocated by the linker, or 
calculated dynamically and placed say after page0.
