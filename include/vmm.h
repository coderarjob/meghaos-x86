/*
 * --------------------------------------------------------------------------------------------------
 * Megha Operating System V2 - Cross Platform Kernel - Virtual Memory Management Header
 * --------------------------------------------------------------------------------------------------
 */

#pragma once

#include <types.h>
#include <paging.h>

void vmm_init();
void vmm_shareMapping (PTR start, SIZE count, U32 processID);
PTR vmm_reserve (SIZE count, PagingMapFlags flags);
bool vmm_unreserve (PTR start_va);
bool vmm_allocPage (PTR addr);
