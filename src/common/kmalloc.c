#include <stdint.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <kmalloc.h>
#include <intrusive_list.h>
#include <kdebug.h>
#include <utils.h>

#ifdef DEBUG
    #define PRINT(...) INFO (__VA_ARGS)
#else
    #define PRINT(fnt, ...) (void)0
#endif

typedef enum KMallocLists
{
    FREE_LIST,
    ALLOC_LIST,
    ADJ_LIST
} KMallocLists;

typedef enum FindCriteria
{
    FIND_CRIT_NODE_SIZE,
    FIND_CRIT_NODE_ADDRESS
} FindCriteria;

static MallocHeader* s_createNewNode (void* at, size_t netSize);
static MallocHeader* s_findFirst (KMallocLists list, FindCriteria criteria, uint32_t value);
static MallocHeader* s_getMallocHeaderFromList (KMallocLists list, ListNode* node);
static ListNode*     s_getListHead (KMallocLists list);
static void          s_splitFreeNode (size_t bytes, MallocHeader* freeNodeHdr);
static void          s_combineAdjFreeNodes (MallocHeader* node);

void kmalloc_init()
{
    list_init (&freeHead);
    list_init (&allocHead);
    list_init (&adjHead);

    MallocHeader* newH = s_createNewNode (buffer, INITIAL_HEAP_SIZE);
    list_add_before (&freeHead, &newH->freenode);
    list_add_before (&adjHead, &newH->adjnode);

    PRINT ("Size of MallocHeader: %lu bytes", sizeof (MallocHeader));
    PRINT ("Malloc buffer is at: %p", buffer);
}

void* kmalloc (size_t bytes)
{
    size_t netAllocSize = (bytes + sizeof (MallocHeader));
    PRINT ("Requested net size of %lu bytes", netAllocSize);

    // Search for suitable node
    size_t        searchAllocSize = (bytes + 2 * sizeof (MallocHeader));
    MallocHeader* node            = s_findFirst (FREE_LIST, FIND_CRIT_NODE_SIZE, searchAllocSize);

    if (node != NULL)
    {
        // Split the free node into two.
        k_assert (node->netNodeSize >= netAllocSize, "");
        s_splitFreeNode (bytes, node);
        return (void*)node + sizeof (MallocHeader);
    }
    else
    {
        // Ask for new memory
        PRINT ("Require mode memory.");
    }
    return NULL;
}

void kfree (void* addr)
{
    void*         headerAddress = addr - sizeof (MallocHeader);
    MallocHeader* allocHdr      = s_findFirst (ALLOC_LIST, FIND_CRIT_NODE_ADDRESS,
                                               (uintptr_t)headerAddress);
    if (allocHdr != NULL)
    {
        PRINT ("Free at %p, Size = %lu", allocHdr, allocHdr->netNodeSize);
        list_remove (&allocHdr->allocnode);
        list_add_after (&freeHead, &allocHdr->freenode);
        allocHdr->isAllocated = false;

        s_combineAdjFreeNodes (allocHdr);
    }
    else
    {
        PRINT ("Adderss not found.");
    }
}

static void s_combineAdjFreeNodes (MallocHeader* node)
{
    MallocHeader* current = node;
    k_assert (current->isAllocated == false, "");

    MallocHeader* next = LIST_ITEM (node->adjnode.next, MallocHeader, adjnode);
    MallocHeader* prev = LIST_ITEM (node->adjnode.prev, MallocHeader, adjnode);

    PRINT ("Prev: 0x%p Sz: %lu <-> Current: 0x%p Sz: %lu <-> Next: 0x%p Sz: %lu", prev,
           prev->netNodeSize, current, current->netNodeSize, next, next->netNodeSize);

    if (!next->isAllocated)
    {
        PRINT ("Combining NEXT into CURRENT");
        current->netNodeSize += next->netNodeSize;
        list_remove (&next->freenode);
        list_remove (&next->adjnode);
    }

    if (!prev->isAllocated)
    {
        PRINT ("Combining CURRENT into PREV");
        prev->netNodeSize += current->netNodeSize;
        list_remove (&current->freenode);
        list_remove (&current->adjnode);
    }
}

static MallocHeader* s_createNewNode (void* at, size_t netSize)
{
    uintptr_t end = (uintptr_t)at + netSize - 1;
    if (end >= (uintptr_t)(buffer + INITIAL_HEAP_SIZE))
    {
        PRINT ("FAIL: No space.");
        return NULL;
    }

    MallocHeader* newH = at;
    newH->netNodeSize  = netSize;
    newH->isAllocated  = false;
    list_init (&newH->freenode);
    list_init (&newH->allocnode);
    list_init (&newH->adjnode);
    return newH;
}

static ListNode* s_getListHead (KMallocLists list)
{
    switch (list)
    {
    case FREE_LIST:
        return &freeHead;
    case ALLOC_LIST:
        return &allocHead;
    case ADJ_LIST:
        return &adjHead;
    default:
        UNREACHABLE();
    };
}

static MallocHeader* s_getMallocHeaderFromList (KMallocLists list, ListNode* node)
{
    switch (list)
    {
    case FREE_LIST:
        return LIST_ITEM (node, MallocHeader, freenode);
    case ALLOC_LIST:
        return LIST_ITEM (node, MallocHeader, allocnode);
    case ADJ_LIST:
        return LIST_ITEM (node, MallocHeader, adjnode);
    default:
        UNREACHABLE();
    };
}

static MallocHeader* s_findFirst (KMallocLists list, FindCriteria criteria, uint32_t value)
{
    bool          found  = false;
    ListNode*     head   = s_getListHead (list);
    ListNode*     node   = NULL;
    MallocHeader* header = NULL;

    PRINT ("Searching for value %lu (0x%lx)", value, value);

    list_for_each (head, node)
    {
        header = s_getMallocHeaderFromList (list, node);
        PRINT ("  (%p) Size = %lu...", header, header->netNodeSize);

        switch (criteria)
        {
        case FIND_CRIT_NODE_ADDRESS:
            found = ((uintptr_t)header == value); //
            break;                                //
        case FIND_CRIT_NODE_SIZE:
            found = (header->netNodeSize >= value); //
            break;                                  //
        }

        if (found)
        {
            PRINT ("Found.");
            break;
        }
    }
    return (found) ? header : NULL;
}

static void s_splitFreeNode (size_t bytes, MallocHeader* freeNodeHdr)
{
    size_t    netAllocSize  = bytes + sizeof (MallocHeader);
    uintptr_t splitAt       = (uintptr_t)freeNodeHdr + netAllocSize;
    size_t    remainingSize = freeNodeHdr->netNodeSize - netAllocSize;
    k_assert (remainingSize >= sizeof (MallocHeader), "");

    MallocHeader* newFreeNodeHdr = s_createNewNode ((void*)splitAt, remainingSize);
    freeNodeHdr->netNodeSize     = netAllocSize;
    freeNodeHdr->isAllocated     = true;
    list_add_after (&freeNodeHdr->freenode, &newFreeNodeHdr->freenode);
    list_add_after (&freeNodeHdr->adjnode, &newFreeNodeHdr->adjnode);
    list_remove (&freeNodeHdr->freenode);
    list_add_before (&allocHead, &freeNodeHdr->allocnode);
}
