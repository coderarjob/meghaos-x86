/*
 * ---------------------------------------------------------------------------
 * Megha Operating System V2 - Cross Platform Kernel - VMM abstract structures
 * ---------------------------------------------------------------------------
 */

#pragma once

#include <intrusive_list.h>
#include <types.h>
#include <vmm.h>
#include <pmm.h>

typedef struct VMemoryShare {
    Physical* pages;
    SIZE count;    // Number of pages added to the pages array
    SIZE refcount; // Number of VirtualAddressSpace objects that share this Memory
} VMemoryShare;

struct VMemoryManager {
    PTR start;
    PTR end;
    bool isStaticAllocated;
    Physical parentProcessPD;
    KernelPhysicalMemoryRegions physicalRegion;
    ListNode head;
};

typedef struct VMemoryAddressSpace {
    bool isStaticAllocated;
    VMemoryMemMapFlags flags;
    PTR start_vm;            // Address space starts from this Virtual address
    SIZE allocationSzBytes;  // Number of virtual pages reserved by this Address space
    S32 processID;           // 0 - Associated with kernel, otherwise this is the process ID
    VMemoryShare* share;     // MemoryShare associated with this mapping.
    ListNode adjMappingNode; // Adds to Virtual Address space list through this node.
} VMemoryAddressSpace;
