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
#include <config.h>
#include <disp.h>
#include <types.h>
#include <assert.h>
#include <utils.h>
#include <panic.h>

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
static void *s_getLinearAddress(UINT pdeIndex, UINT pteIndex, UINT offset);

#ifndef UNITTEST
    #define tlb_inval(addr)      __asm__ volatile("invlpg %0;" ::"m"(addr))
    #define tlb_inval_complete() __asm__ volatile("mov eax, cr3; mov cr3, eax;" ::: "eax")

// TODO: Functions whose both declaration and its implementation are arch dependent can be named
// differently, to indicate that such functions must never be called from any other architecture.
// TODO: The above statement can be extended to other types like constants, macros, types. Those
// items which uniquely exists for a architecture can be named accordingly.
static ArchPageDirectoryEntry* s_getPdeFromCurrentPd (UINT pdeIndex)
{
    k_assert ((pdeIndex < 1024), "Invalid PD/PT/offset index");
    PTR addr = LINEAR_ADDR (RECURSIVE_PDE_INDEX, RECURSIVE_PDE_INDEX,
                            pdeIndex * sizeof (ArchPageDirectoryEntry));
    return (ArchPageDirectoryEntry*)addr;
}

static ArchPageTableEntry* s_getPteFromCurrentPd (UINT pdeIndex, UINT pteIndex)
{
    k_assert ((pdeIndex < 1024) && (pteIndex < 1024), "Invalid PD/PT/offset index");
    PTR addr = LINEAR_ADDR (RECURSIVE_PDE_INDEX, pdeIndex, pteIndex * sizeof (ArchPageTableEntry));
    return (ArchPageTableEntry*)addr;
}

static void *s_getLinearAddress(UINT pdeIndex, UINT pteIndex, UINT offset) 
{
    return (void*)LINEAR_ADDR(pdeIndex, pteIndex, offset);
}
#else
    #define tlb_inval(addr)      (void)0
    #define tlb_inval_complete() (void)0
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

    ArchPageTableEntry newPTE;
    newPTE.pageFrame            = PHYSICAL_TO_PAGEFRAME (pa.val);
    newPTE.present              = BIT_ISUNSET(flags, PG_MAP_FLAG_NOT_PRESENT);
    newPTE.page_attribute_table = 0;
    newPTE.global_page          = 0;
    newPTE.cache_disabled       = BIT_ISUNSET(flags, PG_MAP_FLAG_CACHE_ENABLED);
    newPTE.write_through_cache  = 0;
    newPTE.write_allowed        = BIT_ISSET (flags, PG_MAP_FLAG_WRITABLE);
    newPTE.user_accessable      = BIT_ISUNSET(flags, PG_MAP_FLAG_KERNEL);

    k_memcpy (pte, &newPTE, sizeof (ArchPageTableEntry));
    tlb_inval_complete();
}

static void s_setupPDE (ArchPageDirectoryEntry *pde, Physical pa, PagingMapFlags flags)
{
    k_assert (IS_ALIGNED (pa.val, CONFIG_PAGE_FRAME_SIZE_BYTES), "Wrong alignment");

    ArchPageDirectoryEntry newPDE;
    newPDE.pageTableFrame      = PHYSICAL_TO_PAGEFRAME (pa.val);
    newPDE.present             = BIT_ISUNSET(flags, PG_MAP_FLAG_NOT_PRESENT);
    newPDE.user_accessable     = BIT_ISUNSET(flags, PG_MAP_FLAG_KERNEL);
    newPDE.write_allowed       = BIT_ISSET(flags, PG_MAP_FLAG_WRITABLE);
    newPDE.write_through_cache = 0;
    newPDE.cache_disabled      = BIT_ISUNSET(flags, PG_MAP_FLAG_CACHE_ENABLED);

    k_memcpy (pde, &newPDE, sizeof (ArchPageDirectoryEntry));
    tlb_inval_complete();
}

void kpg_temporaryUnmap()
{
    // TODO: As KERNEL_PDE will always be present and same across every process, recursive mapping
    // is not really required. That is to say the address of the PTE used for temporary mapping is
    // constant.
    // NOTE: The Kernel PT will be mapped in every process and as we are using one of its entry for
    // temporary mapping, a temporary map is global (not specific to a particular process). This
    // means in between temporary map and unmap we must not switch processes.
    ArchPageTableEntry *pte = s_getPteFromCurrentPd (KERNEL_PDE_INDEX, TEMPORARY_PTE_INDEX);
    k_assert (pte->present == 1, "Temporary mapping not present");
    pte->present = 0;
    tlb_inval_complete();
}

void *kpg_temporaryMap (Physical pa)
{
    // TODO: As KERNEL_PDE will always be present, recursive mapping is not really required. That is
    // to say the address of the PTE used for temporary mapping is constant.
    // NOTE: The Kernel PT will be mapped in every process and as we are using one of its entry for
    // temporary mapping, a temporary map is global (not specific to a particular process). This
    // means in between temporary map and unmap we must not switch processes.
    if (!IS_ALIGNED (pa.val, CONFIG_PAGE_FRAME_SIZE_BYTES))
        RETURN_ERROR (ERR_WRONG_ALIGNMENT, NULL);

    ArchPageTableEntry *pte = s_getPteFromCurrentPd (KERNEL_PDE_INDEX, TEMPORARY_PTE_INDEX);
    k_assert (pte->present == 0, "Temporary mapping already present");

    s_setupPTE(pte, pa, PG_MAP_FLAG_KERNEL | PG_MAP_FLAG_WRITABLE | PG_MAP_FLAG_CACHE_ENABLED);

    return s_getLinearAddress(KERNEL_PDE_INDEX, TEMPORARY_PTE_INDEX, 0);
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
            k_panic ("Memory allocaiton failed");

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
