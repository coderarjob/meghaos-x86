/*
 * -------------------------------------------------------------------------------------------------
 * Megha Operating System V2 - Cross Platform Kernel - Virtual Memory Management
 *
 * Manage virtual memory space of a process
 * -------------------------------------------------------------------------------------------------
 */

#include <pmm.h>
#include <stdbool.h>
#include <vmm.h>
#include <vmm_struct.h>
#include <intrusive_list.h>
#include <types.h>
#include <kassert.h>
#include <memmanage.h>
#include <kstdlib.h>
#include <kerror.h>
#include <kernel.h>

static VMemoryAddressSpace* createNewVirtAddrSpace (PTR start_vm, SIZE allocatedBytes,
                                                    PagingMapFlags pgFlags,
                                                    VMemoryAddressSpaceFlags vasFlags)
{
    VMemoryAddressSpace* new                     = NULL;
    VMemoryAddressSpaceIntFlags vasInternalFlags = VMM_INTERNAL_ADDR_SPACE_FLAG_NONE;

    if (KERNEL_PHASE_CHECK (KERNEL_PHASE_STATE_KMALLOC_READY)) {
        if ((new = kmalloc (sizeof (VMemoryAddressSpace))) == NULL) {
            RETURN_ERROR (ERROR_PASSTHROUGH, NULL);
        }
    } else {
        if ((new = ksalloc (sizeof (VMemoryAddressSpace))) == NULL) {
            RETURN_ERROR (ERROR_PASSTHROUGH, NULL);
        }
        vasInternalFlags |= VMM_INTERNAL_ADDR_SPACE_FLAG_STATIC_ALLOC;
    }

    k_assert (IS_ALIGNED (allocatedBytes, CONFIG_PAGE_FRAME_SIZE_BYTES),
              "Size is not multiple of page size");

    new->start_vm          = start_vm;
    new->allocationSzBytes = allocatedBytes;
    new->pgFlags           = pgFlags;
    new->vasFlags          = vasFlags;
    new->vasInternalFlags  = vasInternalFlags;
    new->processID         = 0;
    new->share             = NULL;
    list_init (&new->adjMappingNode);
    return new;
}

static VMemoryAddressSpace* addNewVirtualAddressSpace (VMemoryManager* vmm, PTR start_va,
                                                       SIZE szPages, PagingMapFlags pgFlags,
                                                       VMemoryAddressSpaceFlags vasFlags)
{
    SIZE szBytes = PAGEFRAMES_TO_BYTES (szPages);

    INFO ("Adding address space: %x -> %x, pgFlags: %x, vas flags: %x", start_va,
          (start_va + szBytes - 1), pgFlags, vasFlags);

    // Start virtual address must be page aligned
    if (!IS_ALIGNED (start_va, CONFIG_PAGE_FRAME_SIZE_BYTES)) {
        RETURN_ERROR (ERR_WRONG_ALIGNMENT, NULL);
    }

    // Check is input start_va is within VMM managed address space
    if (start_va < vmm->start || (start_va + szBytes) > vmm->end) {
        RETURN_ERROR (ERR_INVALID_RANGE, NULL);
    }

    VMemoryAddressSpace* newVas = createNewVirtAddrSpace (start_va, szBytes, pgFlags, vasFlags);
    if (newVas == NULL) {
        // TODO: May be we should panic.
        RETURN_ERROR (ERROR_PASSTHROUGH, NULL);
    }

    // If virtual address space list is empty, just add a new address space at the head
    if (list_is_empty (&vmm->head)) {
        list_add_after (&vmm->head, &newVas->adjMappingNode);
        return newVas;
    }

    // If there are already some items in the list, we find a suitable spot such that the list
    // remains sorted (by start_va) in increasing order
    ListNode* node;
    VMemoryAddressSpace* vas = NULL;

    PTR newvas_startvm = newVas->start_vm;
    PTR newvas_endvm   = newvas_startvm + newVas->allocationSzBytes - 1;
    list_for_each (&vmm->head, node)
    {
        vas = LIST_ITEM (node, VMemoryAddressSpace, adjMappingNode);
        k_assert (vas != NULL, "Virtual Address space object in the list cannot be NULL");

        // Address space overlap/duplication detection
        PTR vas_startvm = vas->start_vm;
        PTR vas_endvm   = vas_startvm + vas->allocationSzBytes - 1;

        if ((newvas_startvm >= vas_startvm && newvas_startvm <= vas_endvm) ||
            (newvas_endvm >= vas_startvm && newvas_endvm <= vas_endvm)) {
            // Cannot add, new VAS is overlapping existing VAS.
            INFO ("Overlap detected.new vas (%x, %x), vas (%x, %x)", newvas_startvm, newvas_endvm,
                  vas_startvm, vas_endvm);
            RETURN_ERROR (ERR_VMM_OVERLAPING_VAS, NULL);
        }

        if (vas->start_vm > newVas->start_vm) {
            // Add before the found VAS if there is gap
            list_add_before (&vas->adjMappingNode, &newVas->adjMappingNode);
            return newVas;
        }
    }

    // The new VAS has the largest VA, so add it at the end, which is before head.
    list_add_before (&vmm->head, &newVas->adjMappingNode);

    return newVas;
}

static PTR find_next_va (VMemoryManager* vmm, SIZE szPages)
{
    SIZE szBytes = PAGEFRAMES_TO_BYTES (szPages);

    // Check if address space list is empty. If so then return the 'start' address.
    if (list_is_empty (&vmm->head)) {
        return vmm->start;
    }

    // Address space contains some items, so we traverse the list to find a large enough gap in the
    // address space
    ListNode* node                = NULL;
    VMemoryAddressSpace* vas_prev = NULL;
    PTR new_va                    = 0;

    list_for_each (&vmm->head, node)
    {
        VMemoryAddressSpace* vas = LIST_ITEM (node, VMemoryAddressSpace, adjMappingNode);
        k_assert (vas != NULL, "Virtual Address space object in the list cannot be NULL");

        SIZE addr_space_gap = 0;
        if (vas_prev != NULL) {
            k_assert (vas->start_vm > vas_prev->start_vm,
                      "Address space list must be in sorted order");

            new_va         = (vas_prev->start_vm + vas_prev->allocationSzBytes);
            addr_space_gap = vas->start_vm - new_va;
        } else {
            new_va         = vmm->start;
            addr_space_gap = vas->start_vm - new_va;
        }

        if (addr_space_gap >= szBytes) {
            return new_va;
        }
        vas_prev = vas;
    }

    k_assert (vas_prev != NULL, "There must have been a single VAS");

    // Since new address space is at the end, check that we are not exceeding the 'end'
    SIZE addr_space_gap = vmm->end - (vas_prev->start_vm + vas_prev->allocationSzBytes);
    if (addr_space_gap >= szBytes) {
        new_va = vas_prev->start_vm + vas_prev->allocationSzBytes;
        return new_va;
    }

    // Suitable free address space not found
    return 0;
}

static VMemoryAddressSpace* find_vas (VMemoryManager const* const vmm, PTR startVA)
{
    ListNode* node = NULL;
    list_for_each (&vmm->head, node)
    {
        VMemoryAddressSpace* vas = LIST_ITEM (node, VMemoryAddressSpace, adjMappingNode);
        PTR start_vm             = vas->start_vm;
        PTR end_vm               = start_vm + vas->allocationSzBytes;

        if (startVA >= start_vm && startVA < end_vm) {
            return vas;
        }
    }

    // Did not find a VAS that matches criteria
    return NULL;
}

static bool commitVirtualPages (VMemoryManager const* const vmm, PTR start, SIZE numPages,
                                const VMemoryAddressSpace* const vas)
{
    FUNC_ENTRY ("start: %px, num pages: %x", start, numPages);

    //  Allocate physical pagees
    Physical paStart;
    if (!kpmm_alloc (&paStart, numPages, vmm->physicalRegion)) {
        RETURN_ERROR (ERROR_PASSTHROUGH, false);
    }

    // Map all the physical pages with the virtual ones
    PageDirectory pd = kpg_temporaryMap (vmm->parentProcessPD);
    if (!kpg_mapContinous (pd, start, paStart, numPages, vas->pgFlags)) {
        kpg_temporaryUnmap();
        RETURN_ERROR (ERROR_PASSTHROUGH, false);
    }

    kpg_temporaryUnmap();
    return true;
}

bool kvmm_delete (VMemoryManager** vmm)
{
    FUNC_ENTRY ("vmm: %px", *vmm);

    k_assert (vmm != NULL, "VMM not provided");

    VMemoryManager* the_vmm = *vmm;

    if (BIT_ISSET (the_vmm->flags, VMM_FLAG_STATIC_ALLOC)) {
        RETURN_ERROR (ERR_INVALID_ARGUMENT, false);
    }

    // Remove every item in the address space list then remove the VMM itself
    ListNode* node;
    while (!list_is_empty (&the_vmm->head)) {
        // Remove the first node every time.
        node                     = the_vmm->head.next;
        VMemoryAddressSpace* vas = LIST_ITEM (node, VMemoryAddressSpace, adjMappingNode);
        if (!kvmm_free (the_vmm, vas->start_vm)) {
            // TODO: There should be a clear understanding when to Panic. I do not think panic is
            // the right response here.
            // Should we panic here?
            // Panic when the error could causes system to be unstable. In this case when we cannot
            // free the data structures for a Address space, it would cause memory leak but would
            // not cause any other problem.
            // Should we assert/BugOn?
            // I think this is something that is plausable. kvmm_free should not fail in the first
            // place.
            k_panicOnError();
        }
    }

    kfree (the_vmm);
    (*vmm) = NULL;
    return true;
}

VMemoryManager* kvmm_new (PTR start, PTR end, Physical pd,
                          KernelPhysicalMemoryRegions physicalRegion)
{
    FUNC_ENTRY ("start: %x, end: %x, PD: %px, physical region: %x", start, end, pd, physicalRegion);

    if (start >= end) {
        RETURN_ERROR (ERR_INVALID_ARGUMENT, NULL);
    }

    k_assert (pd.val != 0, "PageDirectory address is NULL");

    VMemoryManager* new_vmm   = NULL;
    VMemoryManagerFlags flags = VMM_FLAG_NONE;

    if (KERNEL_PHASE_CHECK (KERNEL_PHASE_STATE_KMALLOC_READY)) {
        if ((new_vmm = kmalloc (sizeof (VMemoryManager))) == NULL) {
            k_panicOnError();
        }
    } else {
        // NOTE: Creation of VMManager using salloc only happens for Kernel for it tries to capture
        // the Virtual memory reserved and used regions before kmalloc initialization (this is so
        // because kmalloc memory is dynamically allocated).
        // NOTE: It can happen memory for Addresses spaces in it is allocated using kmalloc. That is
        // not a problem and is expected.
        if ((new_vmm = ksalloc (sizeof (VMemoryManager))) == NULL) {
            k_panicOnError();
        }
        flags |= VMM_FLAG_STATIC_ALLOC;
    }

    new_vmm->start           = start;
    new_vmm->end             = end;
    new_vmm->flags           = flags;
    new_vmm->parentProcessPD = pd;
    new_vmm->physicalRegion  = physicalRegion;
    list_init (&new_vmm->head);

    return new_vmm;
}

PTR kvmm_findFree (VMemoryManager* vmm, SIZE szPages)
{
    FUNC_ENTRY ("vmm: %x, szPages: %x", vmm, szPages);

    k_assert (vmm != NULL, "VMM not provided");
    k_assert (szPages != 0, "Number pages is zero.");

    PTR next_va = find_next_va (vmm, szPages);
    return next_va;
}

PTR kvmm_allocAt (VMemoryManager* vmm, PTR va, SIZE szPages, PagingMapFlags pgFlags,
                  VMemoryAddressSpaceFlags vasFlags)
{
    FUNC_ENTRY ("vmm: %x, va: %x, szPages: %x, paging flags: %x, Vas flags: %x", vmm, va, szPages,
                pgFlags, vasFlags);

    k_assert (vmm != NULL, "VMM not provided");

    if (BIT_ISSET (vasFlags, VMM_ADDR_SPACE_FLAG_NULLPAGE)) {
        k_assert ((BIT_ISUNSET (vasFlags, VMM_ADDR_SPACE_FLAG_PRECOMMIT) &&
                   BIT_ISUNSET (vasFlags, VMM_ADDR_SPACE_FLAG_PREMAP)),
                  "Invalid option for precommit");
    }

    const VMemoryAddressSpace* newVas = NULL;
    if ((newVas = addNewVirtualAddressSpace (vmm, va, szPages, pgFlags, vasFlags)) == NULL) {
        RETURN_ERROR (ERROR_PASSTHROUGH, (PTR)NULL);
    }

    if (BIT_ISSET (vasFlags, VMM_ADDR_SPACE_FLAG_PRECOMMIT)) {
        if (!commitVirtualPages (vmm, va, szPages, newVas)) {
            RETURN_ERROR (ERROR_PASSTHROUGH, (PTR)NULL);
        }
    }

    return va;
}

PTR kvmm_alloc (VMemoryManager* vmm, SIZE szPages, PagingMapFlags pgFlags,
                VMemoryAddressSpaceFlags vasFlags)
{
    FUNC_ENTRY ("vmm: %x, szPages: %x, paging flags: %x, Vas flags: %x", vmm, szPages, pgFlags,
                vasFlags);

    k_assert (vmm != NULL, "VMM not provided");
    k_assert (szPages != 0, "Number pages is zero.");

    if (BIT_ISSET (vasFlags, VMM_ADDR_SPACE_FLAG_NULLPAGE)) {
        k_assert ((BIT_ISUNSET (vasFlags, VMM_ADDR_SPACE_FLAG_PRECOMMIT) &&
                   BIT_ISUNSET (vasFlags, VMM_ADDR_SPACE_FLAG_PREMAP)),
                  "Invalid option for precommit");
    }

    PTR next_va = find_next_va (vmm, szPages);
    if (next_va == 0) {
        // No Virutal memory address space found!!
        RETURN_ERROR (ERR_OUT_OF_MEM, 0);
    }

    const VMemoryAddressSpace* newVas = NULL;
    if ((newVas = addNewVirtualAddressSpace (vmm, next_va, szPages, pgFlags, vasFlags)) == NULL) {
        RETURN_ERROR (ERROR_PASSTHROUGH, (PTR)NULL);
    }

    if (BIT_ISSET (vasFlags, VMM_ADDR_SPACE_FLAG_PRECOMMIT)) {
        if (!commitVirtualPages (vmm, next_va, szPages, newVas)) {
            RETURN_ERROR (ERROR_PASSTHROUGH, (PTR)NULL);
        }
    }

    return next_va;
}

bool kvmm_free (VMemoryManager* vmm, PTR start_va)
{
    FUNC_ENTRY ("vmm: %x, start va: %x", vmm, start_va);

    k_assert (vmm != NULL, "VMM not provided");
    k_assert (start_va != 0, "Start address of zero is invalid.");

    VMemoryAddressSpace* vas = NULL;

    if ((vas = find_vas (vmm, start_va)) == NULL) {
        RETURN_ERROR (ERR_VMM_NOT_ALLOCATED, false);
    }

    // Address spaces that are allocated using salloc cannot be unreserved.
    if (BIT_ISSET (vas->vasInternalFlags, VMM_INTERNAL_ADDR_SPACE_FLAG_STATIC_ALLOC)) {
        RETURN_ERROR (ERR_INVALID_ARGUMENT, false);
    }

    // Can we unreserve if address space is shared?
    // Yes, just decrease the refcount
    if (vas->share != NULL) {
        vas->share->refcount--;
    }

    // We can continue and unmap virtual addresses from the physical onces and also free the
    // physical page.
    PTR va       = start_va;
    SIZE szPages = BYTES_TO_PAGEFRAMES_CEILING (vas->allocationSzBytes);

    // TODO: Since we are operating on a VMM, and a VMM is linked to a process, we store PD of the
    // process in the VMManager struct and use that whereever PD is required in VMM.
    PageDirectory pd = kpg_temporaryMap (vmm->parentProcessPD);
    for (SIZE pgIndex = 0; pgIndex < szPages; pgIndex++, va += CONFIG_PAGE_FRAME_SIZE_BYTES) {
        Physical pa;
        if (kpg_doesMappingExists (pd, va, &pa)) {
            if (!kpg_unmap (pd, va)) {
                k_panicOnError();
            }
            // We cannot unallocate physical pages if the address space is being shared.
            if (vas->share == NULL || (vas->share != NULL && vas->share->refcount == 1)) {
                if (!kpmm_free (pa, 1)) {
                    k_panicOnError();
                }
            }
        }
    }
    kpg_temporaryUnmap();

    // We now know that node allocation was done through kmalloc, so it can be freed.
    list_remove (&vas->adjMappingNode);
    kfree (vas);

    return true;
}

#if (DEBUG_LEVEL & 1) && !defined(UNITTEST)
void kvmm_printVASList (VMemoryManager* vmm)
{
    FUNC_ENTRY ("vmm: %x", vmm);

    ListNode* node = NULL;
    if (list_is_empty (&vmm->head)) {
        INFO ("List is empty");
    }

    list_for_each (&vmm->head, node)
    {
        VMemoryAddressSpace* vas = LIST_ITEM (node, VMemoryAddressSpace, adjMappingNode);

        INFO ("* %x -> %x. allocated size: %x, vasflags: %x, pgFlags: %x, "
              "processID: %u.",
              vas->start_vm, vas->start_vm + vas->allocationSzBytes - 1, vas->allocationSzBytes,
              vas->vasFlags, vas->pgFlags, vas->processID);
    }
}
#endif

bool kvmm_commitPage (VMemoryManager* vmm, PTR va)
{
    FUNC_ENTRY ("vmm: %x, va: %px", vmm, va);

    k_assert (vmm != NULL, "VMM not provided");

    VMemoryAddressSpace* vas = NULL;
    if ((vas = find_vas (vmm, va)) == NULL) {
        RETURN_ERROR (ERR_VMM_NOT_ALLOCATED, false);
    }

    if (BIT_ISSET (vas->vasFlags, VMM_ADDR_SPACE_FLAG_PREMAP) ||
        BIT_ISSET (vas->vasFlags, VMM_ADDR_SPACE_FLAG_PRECOMMIT)) {
        RETURN_ERROR (ERR_DOUBLE_ALLOC, false);
    }

    if (BIT_ISSET (vas->vasFlags, VMM_ADDR_SPACE_FLAG_NULLPAGE)) {
        RETURN_ERROR (ERR_VMM_NULL_PAGE_ACCESS, false);
    }

    PTR pageStart = ALIGN_DOWN (va, CONFIG_PAGE_FRAME_SIZE_BYTES);
    if (!commitVirtualPages (vmm, pageStart, 1, vas)) {
        RETURN_ERROR (ERROR_PASSTHROUGH, false);
    }

    INFO ("Commit successful for VA: %px", va);
    return true;
}

Physical kvmm_getPageDirectory (const VMemoryManager* vmm)
{
    FUNC_ENTRY ("vmm: %x", vmm);

    k_assert (vmm != NULL, "VMM not provided");

    return vmm->parentProcessPD;
}
