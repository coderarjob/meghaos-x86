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

#define SALLOC_GRANUALITY   (8 * Byte)
#define KMALLOC_GRANULARITY (16 * bytes)

typedef struct MallocHeader
{
    size_t netNodeSize; /// Size of a region together with the header size.
    bool isAllocated;   /// Is the region allocated or free.
    ListNode adjnode;   /// A node in the Adjacent list.
    ListNode freenode;  /// A node in the Free list.
    ListNode allocnode; /// A node in the Allocation list
} MallocHeader;

void ksalloc_init();
void* ksalloc (UINT bytes);
void* kscalloc (UINT bytes);
SIZE ksalloc_getUsedMemory();

void* kmalloc (size_t bytes);
void* kmallocz (size_t bytes);
bool kfree (void* addr);
void kmalloc_init();
SIZE kmalloc_getUsedMemory();

#endif // MEMMANAGE_H
