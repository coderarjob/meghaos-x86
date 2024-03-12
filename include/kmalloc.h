#ifndef KMALLOC_H
#define KMALLOC_H

#include <stddef.h>
#include <stdbool.h>
#include <intrusive_list.h>

#define KMALLOC_SIZE_PAGES     (5)
#define ALLOCATION_GRANULARITY 0x10

typedef struct MallocHeader
{
    size_t   netNodeSize;
    bool     isAllocated;
    ListNode adjnode;
    ListNode freenode;
    ListNode allocnode;
} MallocHeader;

void* kmalloc (size_t bytes);
void  kfree (void* addr);
void  kmalloc_init();

extern ListNode freeHead, allocHead, adjHead;
#endif // KMALLOC_H
