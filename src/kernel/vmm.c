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

static VMM_VirtualAddressSpace* createNewVirtAddrSpace (PTR start_vm, SIZE reservedBytes,
                                                        PagingMapFlags pgFlags,
                                                        VMM_AddressSpaceFlags vasFlags)
{
    VMM_VirtualAddressSpace* new = NULL;
    if (KERNEL_PHASE_CHECK (KERNEL_PHASE_STATE_KMALLOC_READY)) {
        if ((new = kmalloc (sizeof (VMM_VirtualAddressSpace))) == NULL) {
            RETURN_ERROR (ERROR_PASSTHROUGH, NULL);
        }
    } else {
        if ((new = salloc (sizeof (VMM_VirtualAddressSpace))) == NULL) {
            RETURN_ERROR (ERROR_PASSTHROUGH, NULL);
        }
        vasFlags |= VMM_ADDR_SPACE_FLAG_STATIC_ALLOC;
    }

    new->start_vm       = start_vm;
    new->reservedBytes  = reservedBytes;
    new->allocatedBytes = 0;
    new->pgFlags        = pgFlags;
    new->vasFlags       = vasFlags;
    new->processID      = 0;
    new->share          = NULL;
    list_init (&new->adjMappingNode);
    return new;
}

static bool addNewVirtualAddressSpace (VMManager* vmm, PTR start_va, SIZE szPages,
                                       PagingMapFlags pgFlags, VMM_AddressSpaceFlags vasFlags)
{
    SIZE szBytes = PAGEFRAMES_TO_BYTES (szPages);

    VMM_VirtualAddressSpace* newVas = createNewVirtAddrSpace (start_va, szBytes, pgFlags, vasFlags);
    if (newVas == NULL) {
        // TODO: May be we should panic.
        RETURN_ERROR (ERROR_PASSTHROUGH, false);
    }

    // If virtual address space list is empty, just add a new address space at the head
    if (list_is_empty (&vmm->head)) {
        list_add_after (&vmm->head, &newVas->adjMappingNode);
        return true;
    }

    // If there are already some items in the list, we find a suitable spot such that the list
    // remains sorted (by start_va) in increasing order
    ListNode* node;
    VMM_VirtualAddressSpace* vas = NULL;

    PTR newvas_startvm = newVas->start_vm;
    PTR newvas_endvm   = newvas_startvm + newVas->reservedBytes - 1;
    list_for_each (&vmm->head, node)
    {
        vas = LIST_ITEM (node, VMM_VirtualAddressSpace, adjMappingNode);
        k_assert (vas != NULL, "Virtual Address space object in the list cannot be NULL");

        // Address space overlap/duplication detection
        PTR vas_startvm = vas->start_vm;
        PTR vas_endvm   = vas_startvm + vas->reservedBytes - 1;

        if ((newvas_startvm >= vas_startvm && newvas_startvm <= vas_endvm) ||
            (newvas_endvm >= vas_startvm && newvas_endvm <= vas_endvm)) {
            // Cannot add, new VAS is overlapping existing VAS.
            INFO ("Overlap detected.new vas (%x, %x), vas (%x, %x)", newvas_startvm, newvas_endvm,
                  vas_startvm, vas_endvm);
            RETURN_ERROR (ERR_VMM_OVERLAPING_VAS, false);
        }

        if (vas->start_vm > newVas->start_vm) {
            // Add before the found VAS if there is gap
            list_add_before (&vas->adjMappingNode, &newVas->adjMappingNode);
            return true;
        }
    }

    // The new VAS has the largest VA, so add it at the end, which is before head.
    list_add_before (&vmm->head, &newVas->adjMappingNode);

    return true;
}

static PTR find_next_va (VMManager* vmm, SIZE szPages)
{
    SIZE szBytes = PAGEFRAMES_TO_BYTES (szPages);

    // Check if address space list is empty. If so then return the 'start' address.
    if (list_is_empty (&vmm->head)) {
        return vmm->start;
    }

    // Address space contains some items, so we traverse the list to find a large enough gap in the
    // address space
    ListNode* node                    = NULL;
    VMM_VirtualAddressSpace* vas_prev = NULL;
    PTR new_va                        = 0;

    list_for_each (&vmm->head, node)
    {
        VMM_VirtualAddressSpace* vas = LIST_ITEM (node, VMM_VirtualAddressSpace, adjMappingNode);
        k_assert (vas != NULL, "Virtual Address space object in the list cannot be NULL");

        SIZE addr_space_gap = 0;
        if (vas_prev != NULL) {
            k_assert (vas->start_vm > vas_prev->start_vm,
                      "Address space list must be in sorted order");

            new_va         = (vas_prev->start_vm + vas_prev->reservedBytes);
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
    SIZE addr_space_gap = vmm->end - (vas_prev->start_vm + vas_prev->reservedBytes);
    if (addr_space_gap >= szBytes) {
        new_va = vas_prev->start_vm + vas_prev->reservedBytes;
        return new_va;
    }

    // Suitable free address space not found
    return 0;
}

static VMM_VirtualAddressSpace* find_vas (ListNode* listHead, PTR startVA, bool searchAllocated)
{
    ListNode* node = NULL;
    list_for_each (listHead, node)
    {
        VMM_VirtualAddressSpace* vas = LIST_ITEM (node, VMM_VirtualAddressSpace, adjMappingNode);
        PTR start_vm                 = vas->start_vm;
        PTR end_vm                   = (searchAllocated) ? start_vm + vas->allocatedBytes
                                                         : start_vm + vas->reservedBytes;
        if (startVA >= start_vm && startVA < end_vm) {
            return vas;
        }
    }

    // Did not find a VAS that matches criteria
    return NULL;
}

VMManager* vmm_create (PTR start, PTR end)
{
    FUNC_ENTRY ("start: %x, end: %x", start, end);

    if (start >= end) {
        RETURN_ERROR (ERR_INVALID_ARGUMENT, NULL);
    }

    VMManager* new_vmm = NULL;
    if ((new_vmm = salloc (sizeof (VMManager))) == NULL) {
        k_panicOnError();
    }

    new_vmm->start = start;
    new_vmm->end   = end;
    return new_vmm;
}

void vmm_init (VMManager* vmm)
{
    FUNC_ENTRY();

    list_init (&vmm->head);
}

// bool vmm_freePage (PTR addr)
//{
//     FUNC_ENTRY ("addr: %x", addr);
//
//     VMM_VirtualAddressSpace* vas = NULL;
//
//     // TODO: List head is selected by the virtual address value.
//     if ((vas = find_vas (&g_kstate.vmm_virtAddrListHead, addr, true)) == NULL) {
//         RETURN_ERROR (ERR_VMM_NOT_ALLOCATED, false);
//     }
//
//     if (vas->allocatedPageCount == 0) {
//         RETURN_ERROR(ERR_OUTSIDE_ADDRESSABLE_RANGE, false);
//     }
//
//     vas->allocatedPageCount -= 1;
//     return true;
// }
//
// bool vmm_allocPage (PTR addr)
//{
//     FUNC_ENTRY ("addr: %x", addr);
//
//     VMM_VirtualAddressSpace* vas = NULL;
//
//     // TODO: List head is selected by the virtual address value.
//     if ((vas = find_vas (&g_kstate.vmm_virtAddrListHead, addr, false)) == NULL) {
//         RETURN_ERROR (ERR_VMM_NOT_RESERVED, false);
//     }
//
//     if (vas->allocatedPageCount == vas->reservedPageCount) {
//         RETURN_ERROR(ERR_OUTSIDE_ADDRESSABLE_RANGE, false);
//     }
//
//     vas->allocatedPageCount += 1;
//     return true;
// }

PTR vmm_reserveAt (VMManager* vmm, PTR va, SIZE szPages, PagingMapFlags pgFlags, bool isPremapped)
{
    FUNC_ENTRY ("vmm: %x, va: %x, szPages: %x, paging flags: %x, IsPremapped: %x", vmm, va, szPages,
                pgFlags, isPremapped);

    // Start virtual address must be page aligned
    if (!IS_ALIGNED (va, CONFIG_PAGE_FRAME_SIZE_BYTES)) {
        RETURN_ERROR (ERR_WRONG_ALIGNMENT, 0);
    }

    VMM_AddressSpaceFlags vasFlags = (isPremapped) ? VMM_ADDR_SPACE_FLAG_PREMAP
                                                   : VMM_ADDR_SPACE_FLAG_NONE;

    if (addNewVirtualAddressSpace (vmm, va, szPages, pgFlags, vasFlags) == false) {
        k_panicOnError();
    }

    return va;
}

PTR vmm_reserve (VMManager* vmm, SIZE szPages, PagingMapFlags pgFlags, bool isPremapped)
{
    FUNC_ENTRY ("vmm: %x, szPages: %x, paging flags: %x, IsPremapped: %x", vmm, szPages, pgFlags,
                isPremapped);

    PTR next_va = find_next_va (vmm, szPages);
    if (next_va == 0) {
        // No Virutal memory address space found!!
        RETURN_ERROR (ERR_OUT_OF_MEM, 0);
    }

    return vmm_reserveAt (vmm, next_va, szPages, pgFlags, isPremapped);
}

bool vmm_unreserve (VMManager* vmm, PTR start_va)
{
    FUNC_ENTRY ("vmm: %x, start va: %x", vmm, start_va);

    VMM_VirtualAddressSpace* vas = NULL;

    if ((vas = find_vas (&vmm->head, start_va, false)) == NULL) {
        RETURN_ERROR (ERR_VMM_NOT_RESERVED, false);
    }

    // Address spaces that are reserved using salloc cannot be unreserved.
    if (BIT_ISSET (vas->vasFlags, VMM_ADDR_SPACE_FLAG_STATIC_ALLOC)) {
        RETURN_ERROR (ERR_INVALID_ARGUMENT, false);
    }

    // Can we unreserve if address space is shared?
    // Yes, just decrease the refcount
    if (vas->share != NULL) {
        vas->share->refcount--;
    }

    // TODO: If VAS is not shared or refcount == 0 then
    //       Unmap pagign and unallocate physical pages after freeing virtual address space.

    // We now know that node allocation was done through kmalloc, so it can be freed.
    list_remove (&vas->adjMappingNode);
    kfree (vas);

    return true;
}

void vmm_printVASList (VMManager* vmm)
{
#if (DEBUG_LEVEL & 1) && !defined(UNITTEST)
    ListNode* node = NULL;
    if (list_is_empty (&vmm->head)) {
        INFO ("List is empty");
    }

    list_for_each (&vmm->head, node)
    {
        VMM_VirtualAddressSpace* vas = LIST_ITEM (node, VMM_VirtualAddressSpace, adjMappingNode);

        INFO ("* %x -> %x. Reserved size: %x, allocated size: %x, vasflags: %x, pgFlags: %x, "
              "processID: %u.",
              vas->start_vm, vas->start_vm + vas->reservedBytes - 1, vas->reservedBytes,
              vas->allocatedBytes, vas->vasFlags, vas->pgFlags, vas->processID);
    }
#else
    (void)vmm;
#endif
}
