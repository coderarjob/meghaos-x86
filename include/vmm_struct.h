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

typedef enum VMM_InternalAddressSpaceFlags {
    VMM_INTERNAL_ADDR_SPACE_FLAG_NONE         = 0,
    VMM_INTERNAL_ADDR_SPACE_FLAG_STATIC_ALLOC = (1 << 1), // Allocated using salloc not kmalloc
} VMM_InternalAddressSpaceFlags;

typedef struct VMM_VirtualAddressSpace {
    VMM_AddressSpaceFlags vasFlags;                 // Arch independent VMM flags
    VMM_InternalAddressSpaceFlags vasInternalFlags; // Internal VMM flags
    PagingMapFlags pgFlags;                         // Arch independent Paging flags
    PTR start_vm;           // Address space starts from this Virtual address
    SIZE allocationSzBytes; // Number of virtual pages reserved by this Address space
    S32 processID;          // 0 - Not associated with any process, otherwise this is the process ID
    VMM_MemoryShare* share; // MemoryShare associated with this mapping.
    ListNode adjMappingNode; // Adds to Virtual Address space list through this node.
} VMM_VirtualAddressSpace;
