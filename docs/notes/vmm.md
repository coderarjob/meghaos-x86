# Megha Operating System V2 - x86
------------------------------------------------------------------------------

## Virtual memory map
categories: feature, x86

Please note that this memory map is for x86 (IBM clone) systems. The assumptions made may not be
true on other systems.

Assumptions:
1. System is x86 PC (IBM clones).
2. **TODO**: I must add the assumptions on the BIOS.
3. 0x00000 - 0x80000 (512KB) of physical memory is free. Around ~640KB (0xA0000) is normally a free
   region.

### Virtual memory map:

```
| Virtual address         | Size  | Physical address    | Usage                                |
|-------------------------|-------|---------------------|--------------------------------------|
| 0x00000000 - 0x00000FFF | 4KB   | ...                 | Null page                            |
| 0x00001000 - 0xBFFFFFFF | 3GB   | ...                 | User space                           |
| 0xC0000000 - 0xC0045FFF | 280KB | 0x000000 - 0x045FFF | Kernel Static/Fixed memory           |
| 0xC0046000 - 0xC009FFFF | 360KB | ...                 | Unused                               |
| 0xC00A0000 - 0xC00FFFFF | 384KB | 0x0A0000 - 0x0FFFFF | System reserved, VGA memory etc      |
| 0xC0100000 - 0xC01AFFFF | 704KB | 0x100000 - 0x1AFFFF | 11 Mod files (64KB max each) (704KB) |
| 0xC0200000 - 0xC02FFFFF | 1MB   | dynamic             | Static Allocations                   |
| 0xC0300000 - 0xE02FFFFF | 512MB | dynamic             | Kernel Heap                          |
| 0xFF800000 - 0xFFBFFFFF |       | ...                 | Temporary PDE/PTE mapping (for 1 PT) |
| 0xFFC00000 - 0xFFFFFFFC |       | ...                 | Recursive mapping                    |
|                         |       |                     | * FFC00000 - FFFFEFFC for PT access  |
|                         |       |                     | * FFFFF000 - FFFFFFFC for PD access  |
|-------------------------|-------|---------------------|--------------------------------------|
```

1. Unlike PMM, VMM does not provide any operation to search and allocate for free virtual addresses.
   VMM only offers map/unmap operations to map a virtual page to a physical one. Physical page
   addresses are either constants or was provided by the PMM, however virtual addresses on the other
   hand always either constants or derived by adding/subtracting a constant from a physical address.
2. Static allocations are for permanent allocations, which need no freeing, for example IDTs.
3. Kernel Heap allocators will allocate from the heap region and will provide kmalloc/kfree like
   operations.
4. Additional allocators may have to be added later.
5. The last two PD entries (1022 and 1023) are special and are used for Temporary mappings and
   recursive mappings respectively. This means top 8MB of virtual address space cannot be used.
   MeghaOS will never come to hitting this limit.

### Implications for the initial paging setup

There are two ways to go about the initial paging setup.

1. Setup the paging just right and have the following mappings:

```
| physical address range | virtual address start | Usage                      |
|------------------------|-----------------------|----------------------------|
| 0x000000 - 0x42FFF     | 0xC0000000            | GDT, BootInfo, stack etc.  |
| 0x043000 - 0x44FFF     | 0xC0043000            | Initial PD and PT.         |
|                        |                       | * Should be reclamed later |
| 0x100000 - ...         | 0xC0100000            | Module binaries            |
|------------------------|-----------------------|----------------------------|
```
2. Let the initial setup be just temporary, it sets up a higher-half kernel and lands us in
   kernel_main. Kernel will re-setup paging as per OS requirements. If we go by this route, the
   initial paging setup in entry.s can be moved to the boot1 phase.

I will go with the 1st option, it makes little sense to create a temporary mapping just to be
replaced by the kernel. The 1st option requires no extra space and little change is required from
the current implementation. The only thing required for this option to work is to fix a physical
region for the initial PD/PT to reside. This fixed region are defined in "[gdt](gdt.md)".
