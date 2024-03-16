/*
 * --------------------------------------------------------------------------------------------------
 * Megha Operating System V2 - x86 Kernel - kmalloc allocator
 *
 * This is one of be basic Heap allocators in the kernel
 * --------------------------------------------------------------------------------------------------
 */
#include <x86/memloc.h>
#include <paging.h>
#include <pmm.h>
#include <memmanage.h>

void* kmalloc_arch_preAllocateMemory()
{
    // Allocate physical pages
    Physical start;
    if (kpmm_alloc (&start, KMALLOC_SIZE_PAGES, PMM_REGION_ANY) == false)
        k_panic ("Memory allocation failed");

    // Allocate virutal pages
    PageDirectory pd = kpg_getcurrentpd();

    // Because we are pre-allocating physical memory, we have to map these pages
    // to virtual pages to be useful.
    for (UINT pageIndex = 0; pageIndex < KMALLOC_SIZE_PAGES; pageIndex++)
    {
        Physical pa = PHYSICAL (start.val + PAGEFRAMES_TO_BYTES (pageIndex));
        PTR va      = KMALLOC_MEM_START + PAGEFRAMES_TO_BYTES (pageIndex);
        if (kpg_map (pd, va, pa,
                     PG_MAP_FLAG_KERNEL | PG_MAP_FLAG_WRITABLE | PG_MAP_FLAG_CACHE_ENABLED) ==
            false)
            k_panic ("Page map failed");
    }

    return (void*)KMALLOC_MEM_START;
}
