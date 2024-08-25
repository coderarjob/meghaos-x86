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

typedef enum VMemoryMemMapFlags {
    VMM_MEMMAP_FLAG_NONE        = 0,
    VMM_MEMMAP_FLAG_KERNEL_PAGE = (1 << 0),
    VMM_MEMMAP_FLAG_READONLY    = (1 << 1),
    VMM_MEMMAP_FLAG_NULLPAGE    = (1 << 2), // Never backed, page fault on access.
    VMM_MEMMAP_FLAG_IMMCOMMIT   = (1 << 3), // Commit physical pages (use provided input) now.
    VMM_MEMMAP_FLAG_COMMITTED   = (1 << 4), // VAs are already mapped outside VMM.
} VMemoryMemMapFlags;

typedef struct VMemoryManager VMemoryManager;

VMemoryManager* kvmm_new (PTR start, PTR end, Physical pd,
                          KernelPhysicalMemoryRegions physicalRegion);
Physical kvmm_getPageDirectory (const VMemoryManager* vmm);
bool kvmm_delete (VMemoryManager** vmm);
bool kvmm_free (VMemoryManager* vmm, PTR start_va);
bool kvmm_commitPage (VMemoryManager* vmm, PTR va);
PTR kvmm_findFree (VMemoryManager* vmm, SIZE szPages);
PTR kvmm_memmap (VMemoryManager* vmm, PTR va, Physical const* const pa, SIZE szPages,
                 VMemoryMemMapFlags flags, Physical* const outPA);

#if (DEBUG_LEVEL & 1) && !defined(UNITTEST)
void kvmm_printVASList (VMemoryManager* vmm);
#else
    #define kvmm_printVASList(a) (void)0
#endif

#ifdef DEBUG
void kvmm_setAddressSpaceMetadata (VMemoryManager const* const vmm, PTR addr,
                                   CHAR const* const purpose, UINT const* const pid);
#else
    #define kvmm_setAddressSpaceMetadata(a, b, c, d) (void)0
#endif // DEBUG
