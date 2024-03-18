/*
 * --------------------------------------------------------------------------------------------------
 * Megha Operating System V2 - Cross Platform Kernel - kmalloc allocator
 *
 * This is one of be basic Heap allocators in the kernel
 * --------------------------------------------------------------------------------------------------
 */
#include <kassert.h>
#include <intrusive_list.h>
#include <kdebug.h>
#include <kerror.h>
#include <memmanage.h>
#include <types.h>
#include <utils.h>

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
static ListNode* s_getListHead (KMallocLists list);
static void s_splitFreeNode (size_t bytes, MallocHeader* freeNodeHdr);
static void s_combineAdjFreeNodes (MallocHeader* node);

extern ListNode s_freeHead, s_allocHead, s_adjHead;
static void* s_buffer;

#define NET_ALLOCATION_SIZE(sz_bytes) (sz_bytes + sizeof (MallocHeader))

#ifndef UNITTEST
ListNode s_freeHead, s_allocHead, s_adjHead;
#else
// kmalloc unit test must provide definitions for the list head variables
#endif

/***************************************************************************************************
 * Initializes virtual & physical memory for kmalloc.
 *
 * @return    None
 **************************************************************************************************/
void kmalloc_init()
{
    FUNC_ENTRY();

    list_init (&s_freeHead);
    list_init (&s_allocHead);
    list_init (&s_adjHead);

    s_buffer = kmalloc_arch_preAllocateMemory();

    MallocHeader* newH = s_createNewNode (s_buffer, KMALLOC_SIZE_BYTES);
    list_add_before (&s_freeHead, &newH->freenode);
    list_add_before (&s_adjHead, &newH->adjnode);

    INFO ("Size of MallocHeader: %lu bytes", sizeof (MallocHeader));
    INFO ("Malloc buffer is at: %px", s_buffer);
}

/***************************************************************************************************
 * Allocates at least 'bytes' number of bytes from the kmalloc memory.
 *
 * TODO: kalloc may take flags for alignment requirements to meet various placement requirements.
 *
 * @Input   bytes   Number of bytes to allocate.
 * @return          Poiter to the start of the allocated memory. Or NULL on failure.
 * @error           ERR_OUT_OF_MEM    - There is less memory than requested.
 **************************************************************************************************/
void* kmalloc (size_t bytes)
{
    FUNC_ENTRY ("Bytes: 0x%x", bytes);

    INFO ("Requested net size of %lu bytes", NET_ALLOCATION_SIZE(bytes));

    // Search for suitable node
    size_t searchAllocSize = (bytes + 2 * sizeof (MallocHeader));
    MallocHeader* node     = s_findFirst (FREE_LIST, FIND_CRIT_NODE_SIZE, searchAllocSize);

    if (node != NULL)
    {
        // Split the free node into two.
        k_assert (node->netNodeSize >= NET_ALLOCATION_SIZE(bytes), "Found node too small");
        s_splitFreeNode (bytes, node);
        return (void*)((PTR)node + sizeof (MallocHeader));
    }

    RETURN_ERROR (ERR_OUT_OF_MEM, NULL);
}

/***************************************************************************************************
 * Marks previously allocated memory starting at 'addr' as free.
 *
 * @Input   addr    Pointer to start of a kmalloc allocated memory.
 * @return          True on success. False otherwise.
 * @error           ERR_INVALID_ARGUMENT  - If the input address was not found.
 **************************************************************************************************/
bool kfree (void* addr)
{
    FUNC_ENTRY ("Address: 0x%px", addr);

    void* headerAddress    = (void*)((PTR)addr - sizeof (MallocHeader));
    MallocHeader* allocHdr = s_findFirst (ALLOC_LIST, FIND_CRIT_NODE_ADDRESS, (PTR)headerAddress);
    if (allocHdr != NULL)
    {
        INFO ("Free at %px, Size = %lu", allocHdr, allocHdr->netNodeSize);
        list_remove (&allocHdr->allocnode);
        list_add_after (&s_freeHead, &allocHdr->freenode);
        allocHdr->isAllocated = false;

        s_combineAdjFreeNodes (allocHdr);
        return true;
    }

    INFO ("Adderss not found.");
    RETURN_ERROR(ERR_INVALID_ARGUMENT, false);
}

static void s_combineAdjFreeNodes (MallocHeader* node)
{
    MallocHeader* current = node;
    k_assert (current->isAllocated == false, "Cannot combine allocated node. Invalid input.");

    MallocHeader* next = LIST_ITEM (node->adjnode.next, MallocHeader, adjnode);
    MallocHeader* prev = LIST_ITEM (node->adjnode.prev, MallocHeader, adjnode);

    INFO ("Prev: 0x%px Sz: %lu <-> Current: 0x%px Sz: %lu <-> Next: 0x%px Sz: %lu", prev,
          prev->netNodeSize, current, current->netNodeSize, next, next->netNodeSize);

    if (!next->isAllocated)
    {
        INFO ("Combining NEXT into CURRENT");
        current->netNodeSize += next->netNodeSize;
        list_remove (&next->freenode);
        list_remove (&next->adjnode);
    }

    if (!prev->isAllocated)
    {
        INFO ("Combining CURRENT into PREV");
        prev->netNodeSize += current->netNodeSize;
        list_remove (&current->freenode);
        list_remove (&current->adjnode);
    }
}

static MallocHeader* s_createNewNode (void* at, size_t netSize)
{
    PTR end = (PTR)at + netSize - 1;
    if (end >= ((PTR)s_buffer + KMALLOC_SIZE_BYTES))
    {
        INFO ("FAIL: No space.");
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
        return &s_freeHead;
    case ALLOC_LIST:
        return &s_allocHead;
    case ADJ_LIST:
        return &s_adjHead;
    };
    UNREACHABLE();
    return NULL;
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
    };
    UNREACHABLE();
    return NULL;
}

static MallocHeader* s_findFirst (KMallocLists list, FindCriteria criteria, uint32_t value)
{
    bool found           = false;
    ListNode* head       = s_getListHead (list);
    ListNode* node       = NULL;
    MallocHeader* header = NULL;

    INFO ("Searching for value %lu (0x%lx)", value, value);

    list_for_each (head, node)
    {
        header = s_getMallocHeaderFromList (list, node);
        INFO ("  (%px) Size = %lu...", header, header->netNodeSize);

        switch (criteria)
        {
        case FIND_CRIT_NODE_ADDRESS:
            found = ((PTR)header == value); //
            break;                          //
        case FIND_CRIT_NODE_SIZE:
            found = (header->netNodeSize >= value); //
            break;                                  //
        }

        if (found)
        {
            INFO ("Found.");
            break;
        }
    }
    return (found) ? header : NULL;
}

static void s_splitFreeNode (size_t bytes, MallocHeader* freeNodeHdr)
{
    size_t netAllocSize  = NET_ALLOCATION_SIZE(bytes);
    PTR splitAt          = (PTR)freeNodeHdr + netAllocSize;
    size_t remainingSize = freeNodeHdr->netNodeSize - netAllocSize;
    k_assert (remainingSize >= sizeof (MallocHeader), "Not enough space for kmalloc header");

    MallocHeader* newFreeNodeHdr = s_createNewNode ((void*)splitAt, remainingSize);
    freeNodeHdr->netNodeSize     = netAllocSize;
    freeNodeHdr->isAllocated     = true;
    list_add_after (&freeNodeHdr->freenode, &newFreeNodeHdr->freenode);
    list_add_after (&freeNodeHdr->adjnode, &newFreeNodeHdr->adjnode);
    list_remove (&freeNodeHdr->freenode);
    list_add_before (&s_allocHead, &freeNodeHdr->allocnode);
}
