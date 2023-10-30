#ifndef KMALLOC_H
#define KMALLOC_H

#include <stddef.h>
#include <stdbool.h>
#include "list.h"

#define INITIAL_HEAP_SIZE (4096U * 2U)
#define ALLOCATION_GRANULARITY 0x10

typedef struct MallocHeader {
    size_t netNodeSize;
    bool isAllocated;
    ListNode adjnode;
    ListNode freenode;
    ListNode allocnode;
} MallocHeader;

void *kmalloc (size_t bytes);
void kfree(void *addr);
void kmalloc_init();

extern char buffer[INITIAL_HEAP_SIZE];
extern ListNode freeHead, allocHead, adjHead;
#endif // KMALLOC_H
