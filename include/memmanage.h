/*
 * ---------------------------------------------------------------------------------------
 * Megha Operating System V2 - Cross Platform Kernel - Kernel memory management functions
 * ---------------------------------------------------------------------------------------
 */

#ifndef MEMMANAGE_H
#define MEMMANAGE_H

#include <types.h>
#include <utils.h>
#include <intrusive_list.h>

#define SALLOC_GRANUALITY     (8 * Byte)
#define KMALLOC_SIZE_PAGES  (5) // 5 pages to begin with. Virtual MemMap doc says a lot more.
#define KMALLOC_SIZE_BYTES  (PAGEFRAMES_TO_BYTES (KMALLOC_SIZE_PAGES))
#define KMALLOC_GRANULARITY (16 * bytes)

typedef struct MallocHeader
{
    size_t netNodeSize; /// Size of a region together with the header size.
    bool isAllocated;   /// Is the region allocated or free.
    ListNode adjnode;   /// A node in the Adjacent list.
    ListNode freenode;  /// A node in the Free list.
    ListNode allocnode; /// A node in the Allocation list
} MallocHeader;

void salloc_init();
void* salloc (UINT bytes);
void* scalloc (UINT bytes);
void* salloc_arch_preAllocateMemory();
SIZE salloc_getUsedMemory();

void* kmalloc (size_t bytes);
bool kfree (void* addr);
void kmalloc_init();
void* kmalloc_arch_preAllocateMemory();
SIZE kmalloc_getUsedMemory();

#endif // MEMMANAGE_H
