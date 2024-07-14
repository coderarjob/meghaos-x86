/*
 * ---------------------------------------------------------------------------
 * Megha Operating System V2 - Cross Platform Kernel - VMM abstract structures
 * ---------------------------------------------------------------------------
 */

#pragma once

#include <intrusive_list.h>
#include <types.h>
#include <vmm.h>

struct VMManager {
    PTR start;
    PTR end;
    ListNode head;
};

typedef struct VMM_MemoryShare {
    Physical* pages;
    SIZE count;    // Number of pages added to the pages array
    SIZE refcount; // Number of VirtualAddressSpace objects that share this Memory
} VMM_MemoryShare;

typedef enum VMM_AddressSpaceFlags {
    VMM_ADDR_SPACE_FLAG_NONE         = 0,
    VMM_ADDR_SPACE_FLAG_NOT_PRESENT  = (1 << 0),
    VMM_ADDR_SPACE_FLAG_GROWABLE     = (1 << 1),
    VMM_ADDR_SPACE_FLAG_STATIC_ALLOC = (1 << 2), // Allocated using salloc not kmalloc
    VMM_ADDR_SPACE_FLAG_PREMAP       = (1 << 3), // Address mappings are not handled by VMM. So no
                                           // depend allocation, unreserve, freePage is allowed.
                                           // These addresses spaces are assumed fully allocated.
} VMM_AddressSpaceFlags;

typedef struct VMM_VirtualAddressSpace {
    VMM_AddressSpaceFlags vasFlags; // Arch independent VMM flags
    PagingMapFlags pgFlags;         // Arch independent Paging flags
    PTR start_vm;                   // Address space starts from this Virtual address
    SIZE reservedPageCount;         // Number of virtual pages reserved by this Address space
    SIZE allocatedPageCount;        // Number of virtual pages allocated
    S32 processID;          // 0 - Not associated with any process, otherwise this is the process ID
    VMM_MemoryShare* share; // MemoryShare associated with this mapping.
    ListNode adjMappingNode; // Adds to Virtual Address space list through this node.
} VMM_VirtualAddressSpace;
