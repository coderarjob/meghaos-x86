/*
 * --------------------------------------------------------------------------------------------------
 * Megha Operating System V2 - Cross Platform Kernel - Virtual Memory Management
 *
 * Manage virtual memory space of a process
 * --------------------------------------------------------------------------------------------------
 */

#include <pmm.h>
#include <vmm.h>
#include <intrusive_list.h>
#include <types.h>
#include <x86/kernel.h>
#include <process.h>
#include <kassert.h>
#include <memmanage.h>
#include <kstdlib.h>
#include <kerror.h>
#include <x86/memloc.h>

typedef enum VMM_AddressSpaceFlags {
    VMM_ADDR_SPACE_FLAG_NOT_PRESENT   = (1 << 0),
    VMM_ADDR_SPACE_FLAG_WRITE_ENABLED = (1 << 1),
    VMM_ADDR_SPACE_FLAG_KERNEL_PAGE   = (1 << 2),
    VMM_ADDR_SPACE_FLAG_CACHE_ENABLED = (1 << 3),
    VMM_ADDR_SPACE_FLAG_GROWABLE      = (1 << 4),
    VMM_ADDR_SPACE_FLAG_STATIC_ALLOC  = (1 << 5), // Allocated using salloc not kmalloc
} VMM_AddressSpaceFlags;

typedef struct VMM_MemoryShare {
    Physical* pages;
    SIZE count;    // Number of pages added to the pages array
    SIZE refcount; // Number of VirtualAddressSpace objects that share this Memory
} VMM_MemoryShare;

typedef struct VMM_VirtualAddressSpace {
    VMM_AddressSpaceFlags flags;
    USYSINT start_vm;
    SIZE reservedPageCount;  // Number of virtual pages reserved by this Address space
    SIZE allocatedPageCount; // Number of virtual pages allocated
    S32 processID;
    VMM_MemoryShare* share; // MemoryShare associated with this mapping.
    ListNode adjMappingNode;
} VMM_VirtualAddressSpace;

void vmm_init()
{
    list_init (&g_kstate.vmm_virtAddrListHead);
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

typedef enum SearchCriterias {
    SEARCH_CRI_FIND_FREE_VIRT_MEM,
    SEARCH_CRI_FIND_ALLOC_VIRT_MEM,
} SearchCriterias;

static VMM_VirtualAddressSpace* createNewVirtAddrSpace (PTR start_vm, SIZE reservePageCount,
                                                        VMM_AddressSpaceFlags flags)
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
        flags |= VMM_ADDR_SPACE_FLAG_STATIC_ALLOC;
    }

    new->start_vm           = start_vm;
    new->reservedPageCount  = reservePageCount;
    new->allocatedPageCount = 0;
    new->flags              = flags;
    new->processID          = 0;
    new->share              = NULL;
    list_init (&new->adjMappingNode);
    return new;
}

static UINT search (SearchCriterias crit, ListNode* listHead, USYSINT value,
                    VMM_VirtualAddressSpace** out)
{
    ListNode* node                    = NULL;
    VMM_VirtualAddressSpace* vas_prev = NULL; // Used for SEARCH_CRI_FIND_FREE_VIRT_MEM
    UINT count                        = 0;    // Number of items travered before item was found
    list_for_each (listHead, node)
    {
        count += 1;

        VMM_VirtualAddressSpace* vas = LIST_ITEM (node, VMM_VirtualAddressSpace, adjMappingNode);
        k_assert (vas != NULL, "Virtual Address space object in the list cannot be NULL");

        switch (crit) {
        case SEARCH_CRI_FIND_FREE_VIRT_MEM: {
            if (vas_prev != NULL) {
                k_assert (vas->start_vm > vas_prev->start_vm,
                          "Address space list must be in sorted order");

                SIZE addr_space_gap = (vas->reservedPageCount - vas_prev->allocatedPageCount);
                if (addr_space_gap >= value) {
                    *out = vas;
                    goto fin;
                }
            }
            vas_prev = vas;
        }; break;
        case SEARCH_CRI_FIND_ALLOC_VIRT_MEM: {
            if (vas->start_vm == ALIGN_DOWN (value, CONFIG_PAGE_FRAME_SIZE_BYTES)) {
                *out = vas;
                goto fin;
            }
        }; break;
        default:
            UNREACHABLE();
            break;
        }
    }

fin:
    return count;
}

PTR vmm_allocPages (SIZE count, VMM_AddressSpaceFlags flags)
{
    VMM_VirtualAddressSpace* vas = NULL;

    ListNode* listHead            = NULL;
    PTR default_virtAddress_start = 0;

    if (BIT_ISSET (flags, VMM_ADDR_SPACE_FLAG_KERNEL_PAGE)) {
        // Need to search Kernel Virtual Address list for a free Virutal address space
        listHead                  = &g_kstate.vmm_virtAddrListHead;
        default_virtAddress_start = KERNEL_LOW_REGION_START;
    } else {
        // Need to search Process Virtual Address list for a free Virutal address space
        ProcessInfo* pi = NULL;
        k_assert ((pi = kprocess_getCurrentProcess()) != NULL, "Must have a process");

        listHead                  = &pi->vmm_virtAddrListHead;
        default_virtAddress_start = PROCESS_ADDR_SPACE_START;
    }

    // Search virtual address space ilst for a free address range that is at least 'count' pages
    // long. If the list is empty then a default address is taken.
    PTR start_va = (search (SEARCH_CRI_FIND_FREE_VIRT_MEM, listHead, count, &vas) != 0)
                       ? (vas->start_vm + vas->reservedPageCount)
                       : default_virtAddress_start;

    // Create a new Virtual Address Space object
    if ((vas = createNewVirtAddrSpace (start_va, count, flags)) == NULL) {
        k_panic ("Cannot create initial kernel virutal address space object");
    }

    // Need to insert in sorted order
    list_add_before (listHead, &vas->adjMappingNode);
    return vas->start_vm;
}
