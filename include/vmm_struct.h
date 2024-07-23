/*
 * ---------------------------------------------------------------------------
 * Megha Operating System V2 - Cross Platform Kernel - VMM abstract structures
 * ---------------------------------------------------------------------------
 */

#pragma once

#include <intrusive_list.h>
#include <types.h>
#include <vmm.h>

struct VMemoryManager {
    PTR start;
    PTR end;
    ListNode head;
};

typedef struct VMemoryShare {
    Physical* pages;
    SIZE count;    // Number of pages added to the pages array
    SIZE refcount; // Number of VirtualAddressSpace objects that share this Memory
} VMemoryShare;

typedef enum VMemoryAddressSpaceIntFlags {
    VMM_INTERNAL_ADDR_SPACE_FLAG_NONE         = 0,
    VMM_INTERNAL_ADDR_SPACE_FLAG_STATIC_ALLOC = (1 << 1), // Allocated using salloc not kmalloc
} VMemoryAddressSpaceIntFlags;

typedef struct VMemoryAddressSpace {
    VMemoryAddressSpaceFlags vasFlags;            // Arch independent VMM flags
    VMemoryAddressSpaceIntFlags vasInternalFlags; // Internal VMM flags
    PagingMapFlags pgFlags;                       // Arch independent Paging flags
    PTR start_vm;                                 // Address space starts from this Virtual address
    SIZE allocationSzBytes; // Number of virtual pages reserved by this Address space
    S32 processID;          // 0 - Not associated with any process, otherwise this is the process ID
    VMemoryShare* share;    // MemoryShare associated with this mapping.
    ListNode adjMappingNode; // Adds to Virtual Address space list through this node.
} VMemoryAddressSpace;
