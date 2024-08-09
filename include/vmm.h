/*
 * --------------------------------------------------------------------------------------------------
 * Megha Operating System V2 - Cross Platform Kernel - Virtual Memory Management Header
 * --------------------------------------------------------------------------------------------------
 */

#pragma once

#include <intrusive_list.h>
#include <types.h>
#include <paging.h>
#include <pmm.h>

typedef enum VMemoryAddressSpaceFlags {
    VMM_ADDR_SPACE_FLAG_NONE   = 0,
    VMM_ADDR_SPACE_FLAG_PREMAP = (1 << 1),    // Address mappings are not handled by VMM. So no
                                              // depend allocation, unreserve, freePage is allowed.
                                              // These addresses spaces are assumed fully allocated.
    VMM_ADDR_SPACE_FLAG_NULLPAGE = (1 << 2),  // Will be treated as a NULL page. VMM will not
                                              // allocate a physical page for it.
    VMM_ADDR_SPACE_FLAG_PRECOMMIT = (1 << 3), // Allocate virtual address space, allocate
                                              // physical pages then map them to the virtual
                                              // address space.
} VMemoryAddressSpaceFlags;

typedef struct VMemoryManager VMemoryManager;

VMemoryManager* kvmm_new (PTR start, PTR end, Physical pd,
                          KernelPhysicalMemoryRegions physicalRegion);
Physical kvmm_getPageDirectory (const VMemoryManager* vmm);
bool kvmm_delete (VMemoryManager** vmm);
PTR kvmm_alloc (VMemoryManager* vmm, SIZE szPages, PagingMapFlags pgFlags,
                VMemoryAddressSpaceFlags vasFlags);
PTR kvmm_allocAt (VMemoryManager* vmm, PTR va, SIZE szPages, PagingMapFlags pgFlags,
                  VMemoryAddressSpaceFlags vasFlags);
bool kvmm_free (VMemoryManager* vmm, PTR start_va);
bool kvmm_commitPage (VMemoryManager* vmm, PTR va);
PTR kvmm_findFree (VMemoryManager* vmm, SIZE szPages);

#if (DEBUG_LEVEL & 1) && !defined(UNITTEST)
void kvmm_printVASList (VMemoryManager* vmm);
#else
    #define kvmm_printVASList(a) (void)0
#endif
