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
| Virtual address                       | Physical address | Usage                                |
|---------------------------------------|------------------|--------------------------------------|
| Start            Size           End   | Start            |                                      |
|---------------------------------------|------------------|--------------------------------------|
| 0                4KB            ...   | ...              | Null page                            |
| 4KB              ...            3GB   | ...              | User space                           |
| 3GB              KMEM_LOW_SZ    ...   | 0                | Kernel Stack, GDT, Boot info etc.    |
| 3GB+640KB        ...            1MB   | 640KB            | System reserved, VGA memory etc      |
| KMEM_MOD_ST      1MB            ...   | 1MB              | 11 Mod files (64KB max each) (704KB) |
| KMEM_MOD_ST+1MB  1MB            ...   | KMEM_MOD_END     | Kernel Static Allocations            |
| KMEM_MOD_ST+2MB  KHEAP_SZ       ...   | ...              | Kernel Heap                          |
| FF800000         4MB        FFBFFFFF  | ...              | Temporary PDE/PTE mapping (for 1 PT) |
| FFC00000         4MB        FFFFFFFC  | ...              | Recursive mapping                    |
|                                       |                  | * FFC00000 - FFFFEFFC for PT access  |
|                                       |                  | * FFFFF000 - FFFFFFFC for PD access  |
|---------------------------------------|------------------|--------------------------------------|

KMEM_LOW_SZ  = 271KB (0x43C00) + 8KB (for initial PD & PT) = 279KB (0x45C00)
KMEM_MOD_ST  = 3GB + 1MB
KMEM_MOD_END = 704KB Maximum (11 module files, 64KB each)
KHEAP_SZ     = 512MB (arbitary size limit). Can be max ~1013MB; ending at Temporary mapping start.
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
| physical address range  | virtual address start | Usage                      |
|-------------------------|-----------------------|----------------------------|
| 0x000000 - 0x017FF      | unmapped              |                            |
| 0x001800 - 0x43BFF      | 0xC0001800            | GDT, BootInfo, stack etc.  |
| 0x043C00 - 0x45BFF      | 0xC0043C00            | Initial PD and PT.         |
|                         |                       | * Should be reclamed later |
| 0x100000 - KMEM_MOD_END | 0xC0100000            | Module binaries            |
|-------------------------|-----------------------|----------------------------|
```

2. Let the initial setup be just temporary, it sets up a higher-half kernel and lands us in
   kernel_main. Kernel will re-setup paging as per OS requirements. If we go by this route, the
   initial pageing setup in entry.s can be moved to the boot1 phase.
