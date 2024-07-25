/*
 * --------------------------------------------------------------------------------------------------
 * Megha Operating System V2 - Cross Platform Kernel - Virtual Memory Management Header
 * --------------------------------------------------------------------------------------------------
 */

#pragma once

#include <intrusive_list.h>
#include <types.h>
#include <paging.h>

typedef enum VMemoryAddressSpaceFlags {
    VMM_ADDR_SPACE_FLAG_NONE   = 0,
    VMM_ADDR_SPACE_FLAG_PREMAP = (1 << 1),   // Address mappings are not handled by VMM. So no
                                             // depend allocation, unreserve, freePage is allowed.
                                             // These addresses spaces are assumed fully allocated.
    VMM_ADDR_SPACE_FLAG_NULLPAGE = (1 << 2), // Will be treated as a NULL page. VMM will not
                                             // allocate a physical page for it.
} VMemoryAddressSpaceFlags;

typedef struct VMemoryManager VMemoryManager;

VMemoryManager* vmm_new (PTR start, PTR end);
PTR kvmm_alloc (VMemoryManager* vmm, SIZE szPages, PagingMapFlags pgFlags,
                VMemoryAddressSpaceFlags vasFlags);
PTR kvmm_allocAt (VMemoryManager* vmm, PTR va, SIZE szPages, PagingMapFlags pgFlags,
                  VMemoryAddressSpaceFlags vasFlags);
bool kvmm_free (VMemoryManager* vmm, PTR start_va);
bool kvmm_commitPage (VMemoryManager* vmm, PTR va);
#if (DEBUG_LEVEL & 1) && !defined(UNITTEST)
void vmm_printVASList (VMemoryManager* vmm);
#else
    #define vmm_printVASList(a) (void)0
#endif
