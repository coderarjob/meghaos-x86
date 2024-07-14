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

VMManager* vmm_create (PTR start, PTR end);
void vmm_init (VMManager* vmm);
void vmm_shareMapping (PTR start, SIZE count, U32 processID);
PTR vmm_reserve (VMManager* vmm, SIZE count, PagingMapFlags pgFlags, bool isPremapped);
PTR vmm_reserveAt (VMManager* vmm, PTR va, SIZE count, PagingMapFlags pgFlags, bool isPremapped);
bool vmm_unreserve (VMManager* vmm, PTR start_va);
bool vmm_allocPage (PTR addr);
void vmm_printVASList (VMManager* vmm);
