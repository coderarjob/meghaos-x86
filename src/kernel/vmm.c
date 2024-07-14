/*
 * --------------------------------------------------------------------------------------------------
 * Megha Operating System V2 - Cross Platform Kernel - Virtual Memory Management
 *
 * Manage virtual memory space of a process
 * --------------------------------------------------------------------------------------------------
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

static VMM_VirtualAddressSpace* find_next_va (ListNode* listHead, SIZE pageCount)
{
    ListNode* node                    = NULL;
    VMM_VirtualAddressSpace* vas_prev = NULL; // Used for SEARCH_CRI_FIND_FREE_VIRT_MEM

    list_for_each (listHead, node)
    {
        VMM_VirtualAddressSpace* vas = LIST_ITEM (node, VMM_VirtualAddressSpace, adjMappingNode);
        k_assert (vas != NULL, "Virtual Address space object in the list cannot be NULL");

        INFO ("VAS found: start: %x, total size: %u, flags: %x, processID: %u", vas->start_vm,
              vas->reservedPageCount, vas->vasFlags, vas->processID);

        if (vas_prev != NULL) {
            k_assert (vas->start_vm > vas_prev->start_vm,
                      "Address space list must be in sorted order");

            SIZE addr_space_gap = (vas->start_vm -
                                   (vas_prev->start_vm +
                                    PAGEFRAMES_TO_BYTES (vas_prev->reservedPageCount)));

            if (addr_space_gap >= PAGEFRAMES_TO_BYTES (pageCount)) {
                return vas_prev;
            }
        }
        vas_prev = vas;
    }

    if (vas_prev != NULL) {
        // TODO: Need to check that we not not going past a limit.
        // For Kernel and user the limits are 4GB - 4KB and 3GB - 4KB respectively.
        INFO ("Found");
        return vas_prev;
    }

    // Suitable free address space not found
    return NULL;
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

PTR vmm_reserve (VMManager* vmm, SIZE count, PagingMapFlags pgFlags, bool isPremapped)
{
    FUNC_ENTRY ("vmm: %x, count: %u, paging flags: %x, IsPremapped: %x", vmm, count, pgFlags,
                isPremapped);

    VMM_AddressSpaceFlags vasFlags   = (isPremapped) ? VMM_ADDR_SPACE_FLAG_PREMAP
                                                     : VMM_ADDR_SPACE_FLAG_NONE;
    VMM_VirtualAddressSpace* new_vas = NULL;

    if (list_is_empty (&vmm->head)) {
        // Since there are no address reserved, we crate a new reservation at the start of the
        // adderss space.
        if ((new_vas = createNewVirtAddrSpace (vmm->start, count, pgFlags, vasFlags)) == NULL) {
            k_panic ("Cannot create initial virutal address space object");
        }

        // Insert using 'list_add_after' ensures sorted order
        list_add_after (&vmm->head, &new_vas->adjMappingNode);
    } else {
        // There are some address spaces already reserved, so we need to search for a large enough
        // gap in the address space that is not reserved and we can use next.
        VMM_VirtualAddressSpace* before_vas = NULL;
        if ((before_vas = find_next_va (&vmm->head, count)) == NULL) {
            // No Virutal memory address space found!!
            RETURN_ERROR (ERR_OUT_OF_MEM, 0);
        }

        // Next address space starts where the 'found' address space finished
        PTR next_va = before_vas->start_vm + PAGEFRAMES_TO_BYTES (before_vas->reservedPageCount);

        if ((new_vas = createNewVirtAddrSpace (next_va, count, pgFlags, vasFlags)) == NULL) {
            k_panic ("Cannot create initial virutal address space object");
        }

        // Insert using 'list_add_after' ensures sorted order
        list_add_after (&before_vas->adjMappingNode, &new_vas->adjMappingNode);
    }

    k_assert (IS_ALIGNED (new_vas->start_vm, CONFIG_PAGE_FRAME_SIZE_BYTES),
              "Address must be page aligned");

    INFO ("New address allocated is: %x & length: %u pages", new_vas->start_vm,
          new_vas->reservedPageCount);

    return new_vas->start_vm;
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
