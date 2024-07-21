/*
 * --------------------------------------------------------------------------------------------------
 * Megha Operating System V2 - Cross Platform Kernel - Virtual Memory Management Header
 * --------------------------------------------------------------------------------------------------
 */

#pragma once

#include <intrusive_list.h>
#include <types.h>
#include <paging.h>

typedef struct VMManager VMManager;

VMManager* vmm_new (PTR start, PTR end);
PTR kvmm_alloc (VMManager* vmm, SIZE szPages, PagingMapFlags pgFlags);
PTR kvmm_allocAt (VMManager* vmm, PTR va, SIZE pgCount, PagingMapFlags pgFlags, bool isPremapped);
bool kvmm_free (VMManager* vmm, PTR start_va);
bool kvmm_commitPage (VMManager *vmm, PTR va);
#if (DEBUG_LEVEL & 1) && !defined(UNITTEST)
void vmm_printVASList (VMManager* vmm);
#else
    #define vmm_printVASList(a) (void)0
#endif
