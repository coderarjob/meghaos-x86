/*
 * ---------------------------------------------------------------------------
 * Megha Operating System V2 - x86 Kernel - Kernel memory management functions
 * ---------------------------------------------------------------------------
 */

#ifndef MEMMANAGE_H
#define MEMMANAGE_H

#include <types.h>

#define SALLOC_MEM_SIZE_PAGES (5) // 5 pages to begin with. Virtual MemMap doc says 256 pages.
#define SALLOC_GRANUALITY     (8 * Byte)

void  salloc_init();
void* salloc (UINT bytes);
void* scalloc (UINT bytes);

void  kmalloc_init();
void* kmalloc (size_t bytes);
void  kfree (void* addr);

#endif // MEMMANAGE_H
