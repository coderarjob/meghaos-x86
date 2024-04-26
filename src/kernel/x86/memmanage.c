/*
 * --------------------------------------------------------------------------------------------------
 * Megha Operating System V2 - x86 Kernel - kmalloc allocator
 *
 * This is one of be basic Heap allocators in the kernel
 * --------------------------------------------------------------------------------------------------
 */
#include <kerror.h>
#include <x86/memloc.h>
#include <paging.h>
#include <pmm.h>
#include <memmanage.h>
#include <panic.h>
#include <kdebug.h>

static void s_preAllocateMemory (SIZE sz_pages, PTR va_start)
{
    /* Pre-allocate all memory for salloc */
    Physical pa;
    if (kpmm_alloc (&pa, sz_pages, PMM_REGION_ANY) == false) {
        k_panicOnError();
    }

    INFO ("Physical page allocated at: %px", pa.val);
    INFO ("Allocation size pages : %lu", sz_pages);
    INFO ("Virtual map starts from : %px", va_start);

    PageDirectory pd = kpg_getcurrentpd();

    /* As we are pre-allocating all the physical pages, we have also map the virtual pages. This is
     * required because there is no other way to reserve virtual pages */
    for (UINT pageIndex = 0; pageIndex < sz_pages; pageIndex++) {
        PTR this_va      = va_start + PAGEFRAMES_TO_BYTES (pageIndex);
        Physical this_pa = PHYSICAL (pa.val + PAGEFRAMES_TO_BYTES (pageIndex));
        if (!kpg_map (pd, this_va, this_pa,
                      PG_MAP_FLAG_KERNEL | PG_MAP_FLAG_WRITABLE | PG_MAP_FLAG_CACHE_ENABLED)) {
            k_panicOnError();
        }
    }
}

void* salloc_arch_preAllocateMemory()
{
    FUNC_ENTRY();

    s_preAllocateMemory (SALLOC_MEM_SIZE_PAGES, SALLOC_MEM_START);
    return (void*)SALLOC_MEM_START;
}

void* kmalloc_arch_preAllocateMemory()
{
    FUNC_ENTRY();

    s_preAllocateMemory (KMALLOC_SIZE_PAGES, KMALLOC_MEM_START);
    return (void*)KMALLOC_MEM_START;
}
