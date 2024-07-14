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

static VMM_VirtualAddressSpace* createNewVirtAddrSpace (PTR start_vm, SIZE reservePageCount,
                                                        PagingMapFlags pgFlags,
                                                        VMM_AddressSpaceFlags vasFlags)
{
    KERNEL_PHASE_VALIDATE (KERNEL_PHASE_STATE_SALLOC_READY);

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

    new->start_vm           = start_vm;
    new->reservedPageCount  = reservePageCount;
    new->allocatedPageCount = 0;
    new->pgFlags            = pgFlags;
    new->vasFlags           = vasFlags;
    new->processID          = 0;
    new->share              = NULL;
    list_init (&new->adjMappingNode);
    return new;
}

static bool addNewVirtualAddressSpace (VMManager* vmm, PTR start_va, SIZE pageCount,
                                       PagingMapFlags pgFlags, VMM_AddressSpaceFlags vasFlags)
{
    VMM_VirtualAddressSpace* newVas = createNewVirtAddrSpace (start_va, pageCount, pgFlags,
                                                              vasFlags);
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
    bool found                   = false;
    VMM_VirtualAddressSpace* vas = NULL;
    list_for_each (&vmm->head, node)
    {
        vas = LIST_ITEM (node, VMM_VirtualAddressSpace, adjMappingNode);
        k_assert (vas != NULL, "Virtual Address space object in the list cannot be NULL");

        if (vas->start_vm > newVas->start_vm) {
            found = true;
            break;
        }
    }

    if (found == false) {
        // The new VAS has the largest VA, so add it at the end, which is before head.
        list_add_before (&vmm->head, &newVas->adjMappingNode);
    } else {
        // Add before the found VAS if there is gap
        if (newVas->start_vm + PAGEFRAMES_TO_BYTES (pageCount) > vas->start_vm) {
            // Cannot add, new VAS is overlapping existing VAS.
            RETURN_ERROR (ERR_VMM_OVERLAPING_VAS, false);
        }
        list_add_before (&vas->adjMappingNode, &newVas->adjMappingNode);
    }

    return true;
}

static PTR find_next_va (VMManager* vmm, SIZE pageCount)
{
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

            addr_space_gap = vas->start_vm - (vas_prev->start_vm +
                                              PAGEFRAMES_TO_BYTES (vas_prev->reservedPageCount));
            new_va         = vas_prev->start_vm + PAGEFRAMES_TO_BYTES (vas_prev->reservedPageCount);

        } else {
            addr_space_gap = vas->start_vm - vmm->start;
            new_va         = vmm->start;
        }

        if (addr_space_gap >= PAGEFRAMES_TO_BYTES (pageCount)) {
            return new_va;
        }
        vas_prev = vas;
    }

    k_assert (vas_prev != NULL, "There must have been a single VAS");

    // Since new address space is at the end, check that we are not exceeding the 'end'
    SIZE addr_space_gap = vmm->end -
                          (vas_prev->start_vm + PAGEFRAMES_TO_BYTES (vas_prev->reservedPageCount));
    if (addr_space_gap >= PAGEFRAMES_TO_BYTES (pageCount)) {
        new_va = vas_prev->start_vm + PAGEFRAMES_TO_BYTES (vas_prev->reservedPageCount);
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
        PTR end_vm                   = (searchAllocated)
                                           ? start_vm + PAGEFRAMES_TO_BYTES (vas->allocatedPageCount) - 1
                                           : start_vm + PAGEFRAMES_TO_BYTES (vas->reservedPageCount) - 1;
        if (startVA >= start_vm && startVA <= end_vm) {
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

void vmm_shareMapping (PTR start, SIZE count, U32 processID)
{
    (void)start;
    (void)count;
    (void)processID;

    // To be called from the source process/kernel. Need to decide (NTD) whether only the process
    // who owns the memory can share, or can the process with which it was shared can reshare to
    // another process.

    // Region is a collection of a fixed number of pages. These many physical pages can be tracked
    // in one Region. Now when a Mapping is created, it also has a size, which comes from the input.
    // If
    // Find region from the 'start' address.
    // If region has KERNEL flag then
    //     Create new Mapping object with the new Region
    //         Start_VM      = start
    //         size_numPages = count (this must always be less than the VMM_REGION_SIZE)
    //         ProcessID     = ProcessID
    // Otherwise
    //     Search Process Mapping list to find a free virtual address
    //     Create new Mapping object with the new Region
    //         Start_VM      = found virtual addresss
    //         size_numPages = count (this must always be less than the VMM_REGION_SIZE)
    //         ProcessID     = ProcessID
    // Add new mapping to Mappings List of the ProcessID process.
    // Add new mapping to Region's Mapping list
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

PTR vmm_reserveAt (VMManager* vmm, PTR va, SIZE count, PagingMapFlags pgFlags, bool isPremapped)
{
    FUNC_ENTRY ("vmm: %x, va: %x, count: %u, paging flags: %x, IsPremapped: %x", vmm, va, count,
                pgFlags, isPremapped);

    VMM_AddressSpaceFlags vasFlags = (isPremapped) ? VMM_ADDR_SPACE_FLAG_PREMAP
                                                   : VMM_ADDR_SPACE_FLAG_NONE;

    if (addNewVirtualAddressSpace (vmm, va, count, pgFlags, vasFlags) == false) {
        k_panicOnError();
    }

    return va;
}

PTR vmm_reserve (VMManager* vmm, SIZE count, PagingMapFlags pgFlags, bool isPremapped)
{
    FUNC_ENTRY ("vmm: %x, count: %u, paging flags: %x, IsPremapped: %x", vmm, count, pgFlags,
                isPremapped);

    PTR next_va = find_next_va (vmm, count);
    if (next_va == 0) {
        // No Virutal memory address space found!!
        RETURN_ERROR (ERR_OUT_OF_MEM, 0);
    }

    return vmm_reserveAt (vmm, next_va, count, pgFlags, isPremapped);
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
    ListNode* node = NULL;
    if (list_is_empty (&vmm->head)) {
        INFO ("List is empty");
    }

    list_for_each (&vmm->head, node)
    {
        VMM_VirtualAddressSpace* vas = LIST_ITEM (node, VMM_VirtualAddressSpace, adjMappingNode);

        INFO ("* start: %x, reserved size: %u, allocated size: %x, vasflags: %x, pgFlags: %x, "
              "processID: %u.",
              vas->start_vm, vas->reservedPageCount, vas->allocatedPageCount, vas->vasFlags,
              vas->pgFlags, vas->processID);
    }
}
