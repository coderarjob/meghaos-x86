/*
 * ---------------------------------------------------------------------------------------
 * Megha Operating System V2 - Cross Platform Kernel - Kernel memory management functions
 * ---------------------------------------------------------------------------------------
 */

#ifndef MEMMANAGE_H
#define MEMMANAGE_H

#include <types.h>
#include <intrusive_list.h>

#define SALLOC_MEM_SIZE_PAGES (5) // 5 pages to begin with. Virtual MemMap doc says 256 pages.
#define SALLOC_GRANUALITY     (8 * Byte)

#define KMALLOC_SIZE_PAGES  (5) // 5 pages to begin with. Virtual MemMap doc says a lot more.
#define KMALLOC_SIZE_BYTES  PAGEFRAMES_TO_BYTES (KMALLOC_SIZE_PAGES)
#define KMALLOC_GRANULARITY (16 * bytes)

typedef struct MallocHeader
{
    size_t netNodeSize;
    bool isAllocated;
    ListNode adjnode;
    ListNode freenode;
    ListNode allocnode;
} MallocHeader;

void salloc_init();
void* salloc (UINT bytes);
void* scalloc (UINT bytes);

void* kmalloc (size_t bytes);
void kfree (void* addr);
void kmalloc_init();
void* kmalloc_arch_preAllocateMemory();

#endif // MEMMANAGE_H
