/*
 * --------------------------------------------------------------------------------------------------
 * Megha Operating System V2 - Cross Platform Kernel - Static memory allocation
 *
 * Static memories are for permanent allocations.
 * --------------------------------------------------------------------------------------------------
 */
#include <kdebug.h>
#include <assert.h>
#include <config.h>
#include <kerror.h>
#include <stdbool.h>
#include <utils.h>
#include <types.h>
#include <config.h>
#include <pmm.h>
#include <panic.h>
#include <paging.h>
#include <kstdlib.h>
#include <memmanage.h>
#if defined(__i386__)
    #include <x86/memloc.h>
#endif

#define SALLOC_MEM_END (SALLOC_MEM_START + (SALLOC_MEM_SIZE_PAGES * CONFIG_PAGE_FRAME_SIZE_BYTES))

void *next = NULL; // Points to the start of next allocation.

void salloc_init()
{
    FUNC_ENTRY();

    /* Pre-allocate all memory for salloc */
    Physical pa;
    if (kpmm_alloc (&pa, SALLOC_MEM_SIZE_PAGES, PMM_REGION_ANY) == false)
        k_panic ("Memory allocaiton failed");

    PageDirectory pd = kpg_getcurrentpd();

    /* As we are pre-allocating all the physical pages, we have also map the virtual pages. This is
     * required because there is no other way to reserve virtual pages */
    for (UINT pageIndex = 0; pageIndex < SALLOC_MEM_SIZE_PAGES; pageIndex++)
    {
        PTR this_va = SALLOC_MEM_START + (pageIndex * CONFIG_PAGE_FRAME_SIZE_BYTES);
        Physical this_pa = PHYSICAL(pa.val + (pageIndex * CONFIG_PAGE_FRAME_SIZE_BYTES));
        if (kpg_map (pd, this_va, this_pa, PG_MAP_FLAG_KERNEL) == false)
            k_panic ("Page map failed");
    }

    next = (void*)SALLOC_MEM_START;
}

// TODO: salloc should take flags for alignment requirements to meet various placement requirements.
// For example: Page boundary must not ocour within TSS.
void *salloc (UINT bytes)
{
    FUNC_ENTRY("Bytes: 0x%px", bytes);

    UINT allocSize = ALIGN_UP (bytes, SALLOC_GRANUALITY);
    INFO("Size after aligning: 0x%px", allocSize);

    if ((PTR)next >= SALLOC_MEM_END)
    {
        RETURN_ERROR (ERR_OUT_OF_MEM, NULL);
    }

    next = (void *)((PTR)next + allocSize);
    return (void *)((PTR)next - allocSize);

}

void *scalloc (UINT bytes)
{
    FUNC_ENTRY("Bytes: 0x%px", bytes);

    void *mem = salloc (bytes);
    if (mem != NULL)
        k_memset(mem, 0, bytes);
    return mem;
}
