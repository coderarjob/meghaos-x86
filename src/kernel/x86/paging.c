/*
 * ---------------------------------------------------------------------------
 * Megha Operating System V2 - x86 Kernel - Kernel mode paging routines
 *
 * Manages all the page directories and page tables used by the kernel for its
 * use as well as when allocating memory for user space processes.
 * ---------------------------------------------------------------------------
 */
#include "x86/types.h"
#include <kstdlib.h>
#include <pmm.h>
#include <kerror.h>
#include <x86/paging.h>
#include <paging.h>
#include <config.h>
#include <disp.h>
#include <types.h>
#include <kassert.h>
#include <utils.h>
#include <panic.h>
#include <kdebug.h>
#include <x86/cpu.h>

typedef struct IndexInfo {
    UINT pdeIndex;
    UINT pteIndex;
    UINT offset;
} IndexInfo;

static IndexInfo s_getTableIndices (PTR va);
static ArchPageDirectoryEntry* s_getPdeFromCurrentPd (UINT pdeIndex);
static ArchPageTableEntry* s_getPteFromCurrentPd (UINT pdeIndex, UINT pteIndex);
static void* s_getLinearAddress (UINT pdeIndex, UINT pteIndex, UINT offset);
static void s_setupPTE (PTR associatedVA, ArchPageTableEntry* pte, Physical pa,
                        PagingMapFlags flags);
static void s_setupPDE (PTR associatedVA, ArchPageDirectoryEntry* pde, Physical pa,
                        PagingMapFlags flags);

#ifndef UNITTEST
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

/* A separate function is required so that we can override it in Unittests. If this was a const
 * variable I see no way to make it work in unittests. */
static void* s_getLinearAddress (UINT pdeIndex, UINT pteIndex, UINT offset)
{
    return (void*)LINEAR_ADDR (pdeIndex, pteIndex, offset);
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

static void s_setupPTE (PTR associatedVA, ArchPageTableEntry* pte, Physical pa,
                        PagingMapFlags flags)
{
    k_assert (IS_ALIGNED (pa.val, CONFIG_PAGE_FRAME_SIZE_BYTES), "Wrong alignment");

    ArchPageTableEntry newPTE   = { 0 };
    newPTE.pageFrame            = PHYSICAL_TO_PAGEFRAME (pa.val);
    newPTE.present              = BIT_ISUNSET (flags, PG_MAP_FLAG_NOT_PRESENT);
    newPTE.page_attribute_table = 0;
    newPTE.global_page          = 0;
    newPTE.cache_disabled       = BIT_ISUNSET (flags, PG_MAP_FLAG_CACHE_ENABLED);
    newPTE.write_through_cache  = 0;
    newPTE.write_allowed        = BIT_ISSET (flags, PG_MAP_FLAG_WRITABLE);
    newPTE.user_accessable      = BIT_ISUNSET (flags, PG_MAP_FLAG_KERNEL);

    k_memcpy (pte, &newPTE, sizeof (ArchPageTableEntry));
    x86_TLB_INVAL_SINGLE (associatedVA);
}

static void s_setupPDE (PTR associatedVA, ArchPageDirectoryEntry* pde, Physical pa,
                        PagingMapFlags flags)
{
    k_assert (IS_ALIGNED (pa.val, CONFIG_PAGE_FRAME_SIZE_BYTES), "Wrong alignment");

    ArchPageDirectoryEntry newPDE = { 0 };
    newPDE.pageTableFrame         = PHYSICAL_TO_PAGEFRAME (pa.val);
    newPDE.present                = BIT_ISUNSET (flags, PG_MAP_FLAG_NOT_PRESENT);
    newPDE.user_accessable        = BIT_ISUNSET (flags, PG_MAP_FLAG_KERNEL);
    newPDE.write_allowed          = BIT_ISSET (flags, PG_MAP_FLAG_WRITABLE);
    newPDE.write_through_cache    = 0;
    newPDE.cache_disabled         = BIT_ISUNSET (flags, PG_MAP_FLAG_CACHE_ENABLED);

    k_memcpy (pde, &newPDE, sizeof (ArchPageDirectoryEntry));
    x86_TLB_INVAL_SINGLE (associatedVA);
}

/***************************************************************************************************
 * Disassociates mapping between a physical page and the fixed virtual page used for temporary
 * mapping.
 *
 * NOTE: The Kernel PT will be mapped in every process and as we are using one of its entry for
 *        temporary mapping, a temporary map is global (not specific to a particular process). This
 *        means in between temporary map and unmap we must not switch processes.
 *
 * @return          Nothing
 * @error           Kernel panic  - When temporary map does not exist.
 **************************************************************************************************/
void kpg_temporaryUnmap()
{
    FUNC_ENTRY();

    // TODO: As KERNEL_PDE will always be present and same across every process, recursive mapping
    // is not really required. That is to say the address of the PTE used for temporary mapping is
    // constant.
    ArchPageTableEntry* pte = s_getPteFromCurrentPd (KERNEL_PDE_INDEX, TEMPORARY_PTE_INDEX);
    if (!pte->present) {
        k_panic ("Temporary mapping not present");
    }

    pte->present = 0;

    x86_TLB_INVAL_SINGLE (s_getLinearAddress (KERNEL_PDE_INDEX, TEMPORARY_PTE_INDEX, 0));
}

/***************************************************************************************************
 * Associates mapping between a physical page and fixed virtual page using the page directory of the
 * current process. If the fixed virtual page is in the kernel address space, it will be available
 * in every process (as the kernel address space is shared among processes), so care must be taken
 * when using temporary mapped addresses.
 *
 * NOTE: The Kernel PT will be mapped in every process and as we are using one of its entry for
 *        temporary mapping, a temporary map is global (not specific to a particular process). This
 *        means in between temporary map and unmap we must not switch processes.
 *
 * @Input   pa      Physical page which will be mapped. Must be page aligned.
 * @return          True if mapping was successful, false otherwise. Error number is set.
 * @error           Kernel panic  - When temporary map already exists (to some physical page)
 *                  ERR_WRONG_ALIGNMENT - Input is not page aligned.
 **************************************************************************************************/
void* kpg_temporaryMap (Physical pa)
{
    FUNC_ENTRY ("Physical address: 0x%px", pa.val);

    // TODO: As KERNEL_PDE will always be present, recursive mapping is not really required. That is
    // to say the address of the PTE used for temporary mapping is constant.
    if (!IS_ALIGNED (pa.val, CONFIG_PAGE_FRAME_SIZE_BYTES)) {
        RETURN_ERROR (ERR_WRONG_ALIGNMENT, NULL);
    }

    void* temporaryAddress  = s_getLinearAddress (KERNEL_PDE_INDEX, TEMPORARY_PTE_INDEX, 0);
    ArchPageTableEntry* pte = s_getPteFromCurrentPd (KERNEL_PDE_INDEX, TEMPORARY_PTE_INDEX);
    if (pte->present) {
        k_panic ("Temporary mapping already present");
    }

    s_setupPTE ((PTR)temporaryAddress, pte, pa,
                PG_MAP_FLAG_KERNEL | PG_MAP_FLAG_WRITABLE | PG_MAP_FLAG_CACHE_ENABLED);

    return temporaryAddress;
}

/***************************************************************************************************
 * Gets the pointer (virtual address) to the current page directory.
 *
 * @return          Pointer to the Page Directory of the current process.
 **************************************************************************************************/
PageDirectory kpg_getcurrentpd()
{
    FUNC_ENTRY();

    ArchPageDirectoryEntry* pde = s_getPdeFromCurrentPd (0);
    return (PageDirectory)pde;
}

/***************************************************************************************************
 * Disassociates mapping between a physical page and virtual page. It does not deallocate paging
 * structures that were allocated for this mapping to work.
 *
 * @Input   pd      Page directory which will contain this virtual address.
 * @Input   va      Virtual address which will be unmapped. Must be page aligned.
 * @return          True if unmapping was successful, false otherwise. Error number is set.
 * @error           ERR_DOUBLE_FREE  - Virtual address is already not present.
 *                  ERR_WRONG_ALIGNMENT - Input is not page aligned.
 **************************************************************************************************/
bool kpg_unmap (PageDirectory pd, PTR va)
{
    FUNC_ENTRY ("PD: 0x%px, VA: 0x%px", pd, va);

    k_assert (pd != NULL, "Page Directory is null.");

    if (!IS_ALIGNED (va, CONFIG_PAGE_FRAME_SIZE_BYTES)) {
        RETURN_ERROR (ERR_WRONG_ALIGNMENT, false);
    }

    IndexInfo info              = s_getTableIndices (va);
    ArchPageDirectoryEntry* pde = &pd[info.pdeIndex];
    if (!pde->present) {
        RETURN_ERROR (ERR_DOUBLE_FREE, false); // Page table is already unmapped.
    }

    Physical pt_phyaddr     = PHYSICAL (PAGEFRAME_TO_PHYSICAL (pde->pageTableFrame));
    PageTable pt            = (PageTable)kpg_temporaryMap (pt_phyaddr);
    ArchPageTableEntry* pte = &pt[info.pteIndex];

    if (!pte->present) {
        kpg_temporaryUnmap();
        // Panic or assert may not be the right decision here. At this time I want the caller to
        // take action. The caller will be at a better position to take decision.
        RETURN_ERROR (ERR_DOUBLE_FREE, false);
    }

    pte->present = false;
    x86_TLB_INVAL_SINGLE (va); // This PTE effects virtual address VA. Thus flushing va.

    kpg_temporaryUnmap();

    return true;
}

/***************************************************************************************************
 * Associates a physical page with a virtual page. It will create necessary paging structures if it
 * does not exist for the mapping to work.
 *
 * @Input   pd      Page directory which will contain this virtual address.
 * @Input   va      Virtual address which will map to the physical address. Must be page aligned.
 * @Input   pa      Physical address. Must be page aligned.
 * @Input   flags   PDE/PTE flags to be used for the mapping. PG_MAP_FLAG_* items.
 * @return          True if mapping was successful, false otherwise. Error number is set.
 * @error           ERR_DOUBLE_ALLOC    - Virtual address is already present.
 *                  ERR_WRONG_ALIGNMENT - Inputs are not page aligned.
 **************************************************************************************************/
bool kpg_map (PageDirectory pd, PTR va, Physical pa, PagingMapFlags flags)
{
    FUNC_ENTRY ("PD: 0x%px, VA: 0x%px, PA: 0x%px, flags: 0x%x", pd, va, pa.val, flags);

    k_assert (pd != NULL, "Page Directory is null.");

    if (!IS_ALIGNED (va, CONFIG_PAGE_FRAME_SIZE_BYTES) ||
        !IS_ALIGNED (pa.val, CONFIG_PAGE_FRAME_SIZE_BYTES)) {
        RETURN_ERROR (ERR_WRONG_ALIGNMENT, false);
    }

    IndexInfo info              = s_getTableIndices (va);
    ArchPageDirectoryEntry* pde = &pd[info.pdeIndex];
    if (!pde->present) {
        INFO ("Creating new page table for address 0x%px", va);

        // Allocate phy mem for new page table.
        Physical pa_new;
        if (kpmm_alloc (&pa_new, 1, PMM_REGION_ANY) == false) {
            k_panic ("Memory allocation failed");
        }

        // Initialize the page table.
        void* tempva = kpg_temporaryMap (pa_new);
        k_memset (tempva, 0, CONFIG_PAGE_FRAME_SIZE_BYTES);
        kpg_temporaryUnmap();

        // Reference the page table in the PDE.
        s_setupPDE (va, pde, pa_new, flags);
    }

    // In order to access the page table a temporary mapping is required.
    Physical ptaddr         = PHYSICAL (PAGEFRAME_TO_PHYSICAL (pde->pageTableFrame));
    PageTable tempva        = (PageTable)kpg_temporaryMap (ptaddr);
    ArchPageTableEntry* pte = &tempva[info.pteIndex];

    if (pte->present) {
        kpg_temporaryUnmap();
        RETURN_ERROR (ERR_DOUBLE_ALLOC, false);
    }

    s_setupPTE (va, pte, pa, flags);
    kpg_temporaryUnmap();
    return true;
}

/***************************************************************************************************
 * Gets the physical address for the associated virtual address as per mapping in its Page
 * directory.
 *
 * @Input    pd      Page directory which will contain this virtual address.
 * @Input    va      Virtual address which will map to the physical address.
 * @Output   pa      Pointer where the Physical address will be stored.
 * @return           True if mapping exists, false otherwise. Error number is set.
 * @error            ERR_INVALID_ARGUMENT - Pointer to page directory is null.
 **************************************************************************************************/
bool kpg_getPhysicalMapping (PageDirectory pd, PTR va, Physical* pa)
{
    FUNC_ENTRY ("Page Directory: 0x%px, VA: 0x%px, Page Attributes: 0x%px", pd, va, pa);

    if (pd == NULL) {
        RETURN_ERROR (ERR_INVALID_ARGUMENT, false);
    }

    IndexInfo info              = s_getTableIndices (va);
    ArchPageDirectoryEntry* pde = &pd[info.pdeIndex];

    if (!pde->present) {
        RETURN_ERROR (ERR_PAGE_WRONG_STATE, false);
    }

    Physical pt_phyaddr     = PHYSICAL (PAGEFRAME_TO_PHYSICAL (pde->pageTableFrame));
    void* pt_vaddr          = kpg_temporaryMap (pt_phyaddr);
    ArchPageTableEntry* pte = &((ArchPageTableEntry*)pt_vaddr)[info.pteIndex];

    if (!pte->present) {
        kpg_temporaryUnmap();
        RETURN_ERROR (ERR_PAGE_WRONG_STATE, false);
    }

    Physical phy_addr = PHYSICAL (PAGEFRAME_TO_PHYSICAL (pte->pageFrame) | info.offset);
    *pa               = phy_addr;

    kpg_temporaryUnmap();

    return true;
}

/***************************************************************************************************
 * Searches virtual address space corresponding to the provided page directory, for numPages number
 * of continuous free virtual pages. No allocation/mapping are done.
 *
 * @Input   pd              Page directory wherein the search takes place.
 * @Input   numPages        This many continuous pages are searched for.
 * @Input   region_start    Start the search at this virtual address. Must be page aligned.
 * @Input   region_end      Find within this virtual address. Must be page aligned.
 * @return                  Virtual address found, NULL otherwise.
 **************************************************************************************************/
PTR kpg_findVirtualAddressSpace (PageDirectory pd, SIZE numPages, PTR region_start, PTR region_end)
{
    FUNC_ENTRY ("Page Directory: 0x%px, num Pages: 0x%px, Region start: 0x%px Region end: 0x%px",
                pd, numPages, region_start, region_end);

    if (pd == NULL) {
        RETURN_ERROR (ERR_INVALID_ARGUMENT, (PTR)NULL);
    }

    IndexInfo startIndexInfo = s_getTableIndices (region_start);
    IndexInfo endIndexInfo   = s_getTableIndices (region_end);

    SIZE foundPageCount       = 0;
    bool startAnew            = true;
    bool found                = false;
    UINT found_start_pdeIndex = 0, found_start_pteIndex = 0;

    ArchPageDirectoryEntry* pde = &pd[startIndexInfo.pdeIndex];

    for (UINT pdeIndex = startIndexInfo.pdeIndex; pdeIndex <= endIndexInfo.pdeIndex;
         pde++, pdeIndex++) {
        SIZE pteStartIndex = (pdeIndex == startIndexInfo.pdeIndex) ? startIndexInfo.pteIndex : 0;
        SIZE pteEndIndex   = (pdeIndex == endIndexInfo.pdeIndex) ? endIndexInfo.pteIndex : 1023;

        if (startAnew) {
            startAnew            = false;
            foundPageCount       = 0;
            found_start_pdeIndex = pdeIndex;
            found_start_pteIndex = pteStartIndex;
        }

        if (pde->present == false) {
            foundPageCount += 1024; // Whole page table is empty.
            if (foundPageCount >= numPages) {
                INFO ("Found at least %u pages empty from [0x%x:0x%x] to [0x%x:0x%x]", numPages,
                      found_start_pdeIndex, found_start_pteIndex, pdeIndex, 1023);
                found = true;
                break; // We have found enough pages. Now stop.
            }
            continue; // Require more pages, so continue with the next PDE.
        }

        Physical pt_phyaddr     = PHYSICAL (PAGEFRAME_TO_PHYSICAL (pde->pageTableFrame));
        ArchPageTableEntry* pte = (ArchPageTableEntry*)kpg_temporaryMap (pt_phyaddr);
        pte                     = &pte[pteStartIndex];

        for (UINT pteIndex = pteStartIndex; pteIndex <= pteEndIndex; pte++, pteIndex++) {
            if (startAnew) {
                startAnew            = false;
                foundPageCount       = 0;
                found_start_pdeIndex = pdeIndex;
                found_start_pteIndex = pteIndex;
            }

            if (pte->present == false) {
                foundPageCount += 1;
                if (foundPageCount >= numPages) {
                    INFO ("Found at least %u pages empty from [0x%x:0x%x] to [0x%x:0x%x]", numPages,
                          found_start_pdeIndex, found_start_pteIndex, pdeIndex, pteIndex);
                    found = true;
                    break; // We have found enough pages. Now stop.
                }
            } else {
                // Abandon the currently searched entires as not enough free pages were found in
                // them.
                startAnew = true;
            }
        }

        kpg_temporaryUnmap();

        if (found) {
            break; // We have found enough pages. Now stop.
        }
    }

    if (found) {
        return (PTR)s_getLinearAddress (found_start_pdeIndex, found_start_pteIndex, 0);
    }

    return (PTR)NULL;
}

/***************************************************************************************************
 * Create a new Page Directory and sets it up according to flags.
 *
 * @Output  newPD       Location where physical address of the new Page directory will be stored.
 * @Input   flags       Flags that determine the setup of the Page directory.
 * @return              True if success, false otherwise.
 **************************************************************************************************/
bool kpg_createNewPageDirectory (Physical* newPD, PagingOperationFlags flags)
{
    FUNC_ENTRY("newPD return addr: 0x%px, Flags: %x", newPD, flags);

    if (kpmm_alloc (newPD, 1, PMM_REGION_ANY) == false) {
        RETURN_ERROR(ERROR_PASSTHROUGH, false);     // PMM alloc failure
    }

    INFO ("New PD physical location: 0x%px", newPD->val);

    PageDirectory pd        = kpg_temporaryMap (*newPD);
    k_memset(pd, 0, CONFIG_PAGE_FRAME_SIZE_BYTES);

    // Temporary map this PD and copy kernel page table entries
    if (BIT_ISSET (flags, PG_NEWPD_FLAG_COPY_KERNEL_PAGES)) {
        PageDirectory currentPD = kpg_getcurrentpd();
        for (UINT pdi = KERNEL_PDE_INDEX; pdi < 1024; pdi++) {
            pd[pdi] = currentPD[pdi];
        }

        if (BIT_ISSET (flags, PG_NEWPD_FLAG_RECURSIVE_MAP)) {
            pd[RECURSIVE_PDE_INDEX].pageTableFrame = PHYSICAL_TO_PAGEFRAME (newPD->val);
        }

    }
    kpg_temporaryUnmap();

    return true; // Success
}

bool kpg_deletePageDirectory (Physical pd, PagingOperationFlags flags)
{
    FUNC_ENTRY ("PD addr: 0x%px, Flags: %x", pd, flags);

    // Cannot delete the current physical directory
    x86_CR3 cr3 = { 0 };
    x86_READ_REG (CR3, cr3);
    k_assert (pd.val != PAGEFRAME_TO_PHYSICAL (cr3.physical), "Cannot delete the current PD");

    // Deallocate physical memory used by the page tables referenced by this page directory.
    PageDirectory pd_vaddr = kpg_temporaryMap (pd);
    UINT endIndex = BIT_ISSET (flags, PG_DELPD_FLAG_KEEP_KERNEL_PAGES) ? KERNEL_PDE_INDEX : 1024;

    INFO ("Freeing PDE from index 0 to index %u", endIndex);

    for (UINT i = 0; i < endIndex; i++) {
        ArchPageDirectoryEntry pde = pd_vaddr[i];
        if (pde.present == 1) {
            Physical pt = PHYSICAL (PAGEFRAME_TO_PHYSICAL (pde.pageTableFrame));
            INFO ("Freeing PDE Index: %u, Page Table physical address: 0x%x", i, pt.val);
            if (!kpmm_free (pt, 1)) {
                RETURN_ERROR (ERROR_PASSTHROUGH, false);
            }
        }
    }

    kpg_temporaryUnmap();

    // Now deallocate page used by the page directory itself.
    if (!kpmm_free (pd, 1)) {
        RETURN_ERROR (ERROR_PASSTHROUGH, false);
    }

    return true;
}
