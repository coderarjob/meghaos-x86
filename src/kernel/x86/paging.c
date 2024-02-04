/*
 * ---------------------------------------------------------------------------
 * Megha Operating System V2 - x86 Kernel - Kernel and usermode paging routines
 *
 * Manages all the page directories and page tables used by the kernel for its
 * use as well as when allocating memory for user space processes.
 * ---------------------------------------------------------------------------
 */
#include <mem.h>
#include <pmm.h>
#include <kerror.h>
#include <x86/paging.h>
#include <paging.h>
#include <kdebug.h>
#include <config.h>
#include <disp.h>
#include <types.h>
#include <assert.h>
#include <utils.h>

typedef struct IndexInfo
{
    UINT pdeIndex;
    UINT pteIndex;
    UINT offset;
} IndexInfo;

static IndexInfo s_getTableIndices (PTR va);
static void s_setupPTE (ArchPageTableEntry *pte, Physical pa, PagingMapFlags flags);
static void s_setupPDE (ArchPageDirectoryEntry *pde, Physical pa, PagingMapFlags flags);
static ArchPageDirectoryEntry *s_getPdeFromCurrentPd (UINT pdeIndex);
static ArchPageTableEntry *s_getPteFromCurrentPd (UINT pdeIndex, UINT pteIndex);

#ifndef UNITTEST
// TODO: Functions whose both declaration and its implementation are arch dependent can be named
// differently, to indicate that such functions must never be called from any other architecture.
// TODO: The above statement can be extended to other types like constants, macros, types. Those
// items which uniquely exists for a architecture can be named accordingly.
static ArchPageDirectoryEntry *s_getPdeFromCurrentPd (UINT pdeIndex)
{
    k_assert ((pdeIndex < 1024), "Invalid PD/PT/offset index");
    PTR addr = (RECURSIVE_PD_INDEX << PDE_SHIFT) | (RECURSIVE_PD_INDEX << PTE_SHIFT) |
               (pdeIndex * sizeof (ArchPageDirectoryEntry));
    return (ArchPageDirectoryEntry *)addr;
}

static ArchPageTableEntry *s_getPteFromCurrentPd (UINT pdeIndex, UINT pteIndex)
{
    k_assert ((pdeIndex < 1024) && (pteIndex < 1024), "Invalid PD/PT/offset index");
    PTR addr = (RECURSIVE_PD_INDEX << PDE_SHIFT) | (pdeIndex << PTE_SHIFT) |
               (pteIndex * sizeof (ArchPageTableEntry));
    return (ArchPageTableEntry *)addr;
}
#endif

static IndexInfo s_getTableIndices (PTR va)
{
    IndexInfo info = { 0 };
    info.pdeIndex  = (va & PDE_MASK) >> PDE_SHIFT;
    info.pteIndex  = (va & PTE_MASK) >> PTE_SHIFT;
    info.offset    = (va & OFFSET_MASK) >> OFFSET_SHIFT;

    k_assert ((info.pdeIndex < 1024) && (info.pteIndex < 1024) && (info.offset < 4096),
              "Invalid PD/PT/offset index");

    return info;
}

static void s_setupPTE (ArchPageTableEntry *pte, Physical pa, PagingMapFlags flags)
{
    k_assert (IS_ALIGNED (pa.val, CONFIG_PAGE_FRAME_SIZE_BYTES), "Wrong alignment");
    pte->pageFrame            = PHYSICAL_TO_PAGEFRAME (pa.val);
    pte->present              = 1;
    pte->page_attribute_table = 0;
    pte->global_page          = 0;
    pte->cache_disabled       = BIT_ISUNSET(flags, PG_MAP_CACHE_ENABLED);
    pte->write_through_cache  = 1;
    pte->write_allowed        = BIT_ISSET(flags, PG_MAP_WRITABLE);
    pte->user_accessable      = BIT_ISUNSET(flags, PG_MAP_KERNEL);
}

static void s_setupPDE (ArchPageDirectoryEntry *pde, Physical pa, PagingMapFlags flags)
{
    k_assert (IS_ALIGNED (pa.val, CONFIG_PAGE_FRAME_SIZE_BYTES), "Wrong alignment");
    pde->pageTableFrame      = PHYSICAL_TO_PAGEFRAME (pa.val);
    pde->present             = 1;
    pde->user_accessable     = BIT_ISUNSET(flags, PG_MAP_KERNEL);
    pde->write_allowed       = BIT_ISSET(flags, PG_MAP_WRITABLE);
    pde->write_through_cache = 0;
    pde->cache_disabled      = BIT_ISUNSET(flags, PG_MAP_CACHE_ENABLED);
}

void kpg_temporaryUnmap()
{
    ArchPageDirectoryEntry *pd = s_getPdeFromCurrentPd (TEMPORARY_PD_INDEX);
    k_assert (pd->present == 1, "Temporary mapping not present");
    pd->present = 0;
}

void *kpg_temporaryMap (Physical pa)
{
    if (!IS_ALIGNED (pa.val, CONFIG_PAGE_FRAME_SIZE_BYTES))
        RETURN_ERROR (ERR_WRONG_ALIGNMENT, NULL);

    ArchPageDirectoryEntry *pde = s_getPdeFromCurrentPd (TEMPORARY_PD_INDEX);
    k_assert (pde->present == 0, "Temporary mapping already present");

    // TODO: Temporary mapping must always be for Kernel.
    s_setupPDE(pde, pa, PG_MAP_WRITABLE | PG_MAP_CACHE_ENABLED);
    return (void*)s_getPteFromCurrentPd(TEMPORARY_PD_INDEX, 0);
}

PageDirectory kpg_getcurrentpd()
{
    ArchPageDirectoryEntry *pde = s_getPdeFromCurrentPd (0);
    return (PageDirectory)pde;
}

bool kpg_unmap (PageDirectory pd, PTR va)
{
    k_assert(pd != NULL, "Page Directory is null.");

    if (!IS_ALIGNED (va, CONFIG_PAGE_FRAME_SIZE_BYTES))
        RETURN_ERROR (ERR_WRONG_ALIGNMENT, false);

    IndexInfo info               = s_getTableIndices (va);
    ArchPageDirectoryEntry *pde  = &pd[info.pdeIndex];
    k_assert(pde->present, "Page table is already unmapped.");

    Physical pt_phyaddr = PHYSICAL(PAGEFRAME_TO_PHYSICAL(pde->pageTableFrame));
    PageTable pt = (PageTable)kpg_temporaryMap(pt_phyaddr);
    ArchPageTableEntry *pte = &pt[info.pteIndex];

    if (!pte->present) {
        kpg_temporaryUnmap();
        // Panic or assert may not be the right decision here. At this time I want the caller to
        // take action. The caller will be at a better position to take decision.
        RETURN_ERROR (ERR_DOUBLE_FREE, false);
    }

    pte->present = false;
    kpg_temporaryUnmap();

    return true;
}

bool kpg_map (PageDirectory pd, PTR va, Physical pa, PagingMapFlags flags)
{
    k_assert(pd != NULL, "Page Directory is null.");

    if (!IS_ALIGNED (va, CONFIG_PAGE_FRAME_SIZE_BYTES) ||
        !IS_ALIGNED (pa.val, CONFIG_PAGE_FRAME_SIZE_BYTES))
        RETURN_ERROR (ERR_WRONG_ALIGNMENT, false);

    IndexInfo info             = s_getTableIndices (va);
    ArchPageDirectoryEntry *pde  = &pd[info.pdeIndex];
    if (!pde->present)
    {
        // Allocate phy mem for new page table.
        Physical pa_new;
        if (kpmm_alloc (&pa_new, 1, PMM_REGION_ANY) == false)
            k_panic ("%s", "Memory allocaiton failed");

        // Initialize the page table.
        void *tempva = kpg_temporaryMap (pa_new);
        k_memset (tempva, 0, CONFIG_PAGE_FRAME_SIZE_BYTES);
        kpg_temporaryUnmap();

        // Referene the page table in the PDE.
        s_setupPDE (pde, pa_new, flags);
    }

    // In order to access the page table a temporary mapping is required.
    Physical ptaddr = PHYSICAL (PAGEFRAME_TO_PHYSICAL (pde->pageTableFrame));
    PageTable tempva = (PageTable)kpg_temporaryMap (ptaddr);
    ArchPageTableEntry *pte = &tempva[info.pteIndex];

    if (pte->present) {
        kpg_temporaryUnmap();
        RETURN_ERROR (ERR_DOUBLE_ALLOC, false);
    }

    s_setupPTE (pte, pa, flags);
    kpg_temporaryUnmap();
    return true;
}
