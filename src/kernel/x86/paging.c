/*
 * ---------------------------------------------------------------------------
 * Megha Operating System V2 - x86 Kernel - Kernel and usermode paging routines
 *
 * Manages all the page directories and page tables used by the kernel for its
 * use as well as when allocating memory for user space processes.
 * ---------------------------------------------------------------------------
 */
#include "x86/config.h"
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
static void s_setupPTE (ArchPageTableEntry *pte, const Physical *pageFrame);
static void s_setupPDE (ArchPageDirectoryEntry *pde, const Physical *pageFrame);
static ArchPageDirectoryEntry *s_getPTE (UINT pdeIndex, UINT pteIndex);
// static PageAttributes s_extractPDAttributes (ArchPageDirectoryEntry *pde);

#define PAGE_FRAME_SIZE_BITS      (31U - CONFIG_PAGE_SIZE_BITS)
#define PAGEFRAME_TO_PHYSICAL(pf) ((USYSINT)(pf) << CONFIG_PAGE_SIZE_BITS)
#define PHYSICAL_TO_PAGEFRAME(addr)                                                                \
    ((addr / CONFIG_PAGE_FRAME_SIZE_BYTES) & BIT_MASK (PAGE_FRAME_SIZE_BITS, 0))

/*static PageAttributes s_extractPDAttributes (ArchPageDirectoryEntry *pde)
{
    PageAttributes pa = { 0 };
    pa.present        = pde->present == 1;
    pa.writable       = pde->write_allowed == 1;
    pa.userAccessable = pde->user_accessable == 1;
    pa.cacheDisable   = pde->cache_disabled == 1;
    pa.writeThrough   = pde->write_through_cache == 1;
    return pa;
}*/

static ArchPageDirectoryEntry *s_getPDE (UINT pdeIndex)
{
    k_assert ((pdeIndex < 1024), "Invalid PD/PT/offset index");
    PTR addr = (RECURSIVE_PD_INDEX << PDE_SHIFT) | (RECURSIVE_PD_INDEX << PTE_SHIFT) |
               (pdeIndex * sizeof (ArchPageDirectoryEntry));
    return (ArchPageDirectoryEntry *)addr;
}

static ArchPageDirectoryEntry *s_getPTE (UINT pdeIndex, UINT pteIndex)
{
    k_assert ((pdeIndex < 1024) && (pdeIndex < 1024), "Invalid PD/PT/offset index");
    PTR addr = (RECURSIVE_PD_INDEX << PDE_SHIFT) | (pdeIndex << PTE_SHIFT) |
               (pteIndex * sizeof (ArchPageTableEntry));
    return (ArchPageDirectoryEntry *)addr;
}

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

static void s_setupPTE (ArchPageTableEntry *pte, const Physical *pageFrame)
{
    k_assert (IS_ALIGNED (pageFrame->val, CONFIG_PAGE_FRAME_SIZE_BYTES), "Wrong alignment");
    pte->pageFrame            = PHYSICAL_TO_PAGEFRAME (pageFrame->val);
    pte->present              = 1;
    pte->page_attribute_table = 0;
    pte->global_page          = 0;
    pte->cache_disabled       = 1;
    pte->write_through_cache  = 1;
    pte->write_allowed        = 1;
    pte->user_accessable      = 1;
}

static void s_setupPDE (ArchPageDirectoryEntry *pde, const Physical *pageFrame)
{
    k_assert (IS_ALIGNED (pageFrame->val, CONFIG_PAGE_FRAME_SIZE_BYTES), "Wrong alignment");
    pde->pageTableFrame      = PHYSICAL_TO_PAGEFRAME (pageFrame->val);
    pde->present             = 1;
    pde->user_accessable     = 1;
    pde->write_allowed       = 1;
    pde->write_through_cache = 0;
    pde->cache_disabled      = 1;
}

void kpg_temporaryUnmap()
{
    ArchPageDirectoryEntry *pd = s_getPDE (TEMPORARY_PD_INDEX);
    k_assert (pd->present == 1, "Temporary mapping not present");
    pd->present = 0;
}

PTR kpg_temporaryMap (Physical pageFrame)
{
    k_assert (IS_ALIGNED (pageFrame.val, CONFIG_PAGE_FRAME_SIZE_BYTES), "Wrong alignment");

    ArchPageDirectoryEntry *pde = s_getPDE (TEMPORARY_PD_INDEX);
    k_assert (pde->present == 0, "Temporary mapping already present");

    s_setupPDE (pde, &pageFrame);
    return (PTR)s_getPTE (TEMPORARY_PD_INDEX, 0);
}

PageDirectory kpg_getcurrentpd()
{
    ArchPageDirectoryEntry *pde = s_getPDE (0);
    PageDirectory pd            = { .pd = pde, .attr = { 0 } };
    return pd;
}

bool kpg_map (PageDirectory *pd, PageMapAttributes attr, PTR va, Physical *pa)
{
    (void)attr;

    k_assert (pd != NULL, "PD is null");

    if (!IS_ALIGNED (pa->val, CONFIG_PAGE_FRAME_SIZE_BYTES))
        RETURN_ERROR (ERR_WRONG_ALIGNMENT, false);

    IndexInfo info             = s_getTableIndices (va);
    ArchPageDirectoryEntry pde = pd->pd[info.pdeIndex];
    if (!pde.present) RETURN_ERROR (ERR_INVALID_ARGUMENT, false);

    // In order to access the page table a temporary mapping is required.
    // Note: An alternate to temporary map can to have recursive map within each page table as well.
    // However the temporary map solution seems better to me.
    Physical ptaddr         = PHYSICAL (PAGEFRAME_TO_PHYSICAL (pde.pageTableFrame));
    PTR tempva              = kpg_temporaryMap (ptaddr);
    ArchPageTableEntry *pte = &((ArchPageTableEntry *)tempva)[info.pteIndex];

    s_setupPTE (pte, pa);

    kpg_temporaryUnmap();
    return true;
}
