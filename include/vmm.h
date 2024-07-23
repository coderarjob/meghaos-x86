/*
 * --------------------------------------------------------------------------------------------------
 * Megha Operating System V2 - Cross Platform Kernel - Virtual Memory Management Header
 * --------------------------------------------------------------------------------------------------
 */

#pragma once

#include <intrusive_list.h>
#include <types.h>
#include <paging.h>

typedef enum VMM_AddressSpaceFlags {
    VMM_ADDR_SPACE_FLAG_NONE   = 0,
    VMM_ADDR_SPACE_FLAG_PREMAP = (1 << 1), // Address mappings are not handled by VMM. So no
                                           // depend allocation, unreserve, freePage is allowed.
                                           // These addresses spaces are assumed fully allocated.
} VMM_AddressSpaceFlags;

typedef struct VMManager VMManager;

VMManager* vmm_new (PTR start, PTR end);
PTR kvmm_alloc (VMManager* vmm, SIZE szPages, PagingMapFlags pgFlags);
PTR kvmm_allocAt (VMManager* vmm, PTR va, SIZE szPages, PagingMapFlags pgFlags,
                  VMM_AddressSpaceFlags vasFlags);
bool kvmm_free (VMManager* vmm, PTR start_va);
bool kvmm_commitPage (VMManager* vmm, PTR va);
#if (DEBUG_LEVEL & 1) && !defined(UNITTEST)
void vmm_printVASList (VMManager* vmm);
#else
    #define vmm_printVASList(a) (void)0
#endif
