/*
 * --------------------------------------------------------------------------------------------------
 * Megha Operating System V2 - App Library - malloc allocator
 * --------------------------------------------------------------------------------------------------
 */
#include <intrusive_list.h>
#include <cm/err.h>
#include <types.h>
#include <utils.h>
#include <cm/debug.h>
#include <cm/cm.h>
#include <kcmlib.h>

typedef enum FindCriteria {
    FIND_CRIT_NODE_SIZE,
    FIND_CRIT_NODE_ADDRESS
} FindCriteria;

static CM_MallocHeader* s_createNewNode (void* at, size_t netSize);
static CM_MallocHeader* s_findFirst (ListNode* head, FindCriteria criteria, PTR value);
static CM_MallocHeader* s_getMallocHeaderFromList (ListNode* head, ListNode* node);
static void s_splitFreeNode (size_t bytes, CM_MallocHeader* freeNodeHdr);
static void s_combineAdjFreeNodes (CM_MallocHeader* currentNode);

extern ListNode s_freeHead, s_allocHead, s_adjHead;
static void* s_buffer;

#define NET_ALLOCATION_SIZE(sz_bytes) (sz_bytes + sizeof (CM_MallocHeader))

#ifndef UNITTEST
ListNode s_freeHead, s_allocHead, s_adjHead;
#else
// cm_malloc unit test must provide definitions for the list head variables
#endif

/***************************************************************************************************
 * Initializes virtual & physical memory for cm_malloc.
 *
 * @return    None
 **************************************************************************************************/
void cm_malloc_init(void)
{
    CM_DBG_FUNC_ENTRY();

    list_init (&s_freeHead);
    list_init (&s_allocHead);
    list_init (&s_adjHead);

    s_buffer = cm_process_get_datamem_start();

    CM_MallocHeader* newH = s_createNewNode (s_buffer, CM_MALLOC_MEM_SIZE_BYTES);
    list_add_before (&s_freeHead, &newH->freenode);
    list_add_before (&s_adjHead, &newH->adjnode);

    CM_DBG_INFO ("Size of CM_MallocHeader: %lu bytes", sizeof (CM_MallocHeader));
    CM_DBG_INFO ("Malloc buffer is at: %px", s_buffer);
}

/***************************************************************************************************
 * Allocates at least 'bytes' number of bytes from the cm_malloc memory. If successful then zeros the
 * memory before returning.
 *
 * @Input   bytes   Number of bytes to allocate.
 * @return          Poiter to the start of the allocated memory. Or NULL on failure.
 **************************************************************************************************/
void* cm_calloc (size_t bytes)
{
    CM_DBG_FUNC_ENTRY ("Bytes: %x", bytes);

    void* addr = cm_malloc (bytes);
    if (addr != NULL) {
        cm_memset (addr, 0, bytes);
    }
    return addr;
}

/***************************************************************************************************
 * Allocates at least 'bytes' number of bytes from the cm_malloc memory.
 *
 * @Input   bytes   Number of bytes to allocate.
 * @return          Poiter to the start of the allocated memory. Or NULL on failure.
 * @error           ERR_OUT_OF_MEM    - There is less memory than requested.
 **************************************************************************************************/
void* cm_malloc (size_t bytes)
{
    CM_DBG_FUNC_ENTRY ("Bytes: %x", bytes);

    if (bytes == 0) {
        CM_RETURN_ERROR (CM_ERR_INVALID_INPUT, NULL);
    }

    CM_DBG_INFO ("Requested net size of %lu bytes", NET_ALLOCATION_SIZE (bytes));

    // Search for suitable node
    size_t searchAllocSize = NET_ALLOCATION_SIZE (bytes) + sizeof (CM_MallocHeader);
    CM_MallocHeader* node  = s_findFirst (&s_freeHead, FIND_CRIT_NODE_SIZE, searchAllocSize);

    if (node != NULL) {
        cm_assert (node->netNodeSize >= NET_ALLOCATION_SIZE (bytes)); // Found node too small

        // Split the free node into two.
        s_splitFreeNode (bytes, node);
        return (void*)((PTR)node + sizeof (CM_MallocHeader));
    }

    CM_RETURN_ERROR (CM_ERR_OUT_OF_HEAP_MEM, NULL);
}

/***************************************************************************************************
 * Marks previously allocated memory starting at 'addr' as free.
 *
 * @Input   addr    Pointer to start of a cm_malloc allocated memory.
 * @return          True on success. False otherwise.
 * @error           ERR_INVALID_ARGUMENT  - If the input address was not found.
 **************************************************************************************************/
bool cm_free (void* addr)
{
    CM_DBG_FUNC_ENTRY ("Address: %px", addr);

    void* headerAddress       = (void*)((PTR)addr - sizeof (CM_MallocHeader));
    CM_MallocHeader* allocHdr = s_findFirst (&s_allocHead, FIND_CRIT_NODE_ADDRESS,
                                             (PTR)headerAddress);
    if (allocHdr != NULL) {
        CM_DBG_INFO ("Free at %px, Size = %lu", allocHdr, allocHdr->netNodeSize);
        list_remove (&allocHdr->allocnode);
        list_add_after (&s_freeHead, &allocHdr->freenode);
        allocHdr->isAllocated = false;

        s_combineAdjFreeNodes (allocHdr);
        return true;
    }

    // Could not find allocated node. Either double free or tatal error.
    cm_panic();
    return false;
}

static void s_combineAdjFreeNodes (CM_MallocHeader* currentNode)
{
    CM_MallocHeader* next = NULL;
    CM_MallocHeader* prev = NULL;

    cm_assert (currentNode->isAllocated == false); // Cannot combine allocated node. Invalid input.

    // Prev node could be the list head (which is not a CM_MallocHeader). That would mean beginning
    // of list and is not taken into account.
    if (currentNode->adjnode.prev != &s_adjHead) {
        prev = LIST_ITEM (currentNode->adjnode.prev, CM_MallocHeader, adjnode);
    }

    // Next node could be the list head (which is not a CM_MallocHeader). That would mean end of
    // list and is not taken into account (Practically this is not possible as there will always be
    // one Free section at the end of cm_malloc buffer).
    if (currentNode->adjnode.next != &s_adjHead) {
        next = LIST_ITEM (currentNode->adjnode.next, CM_MallocHeader, adjnode);
    }

    if (next && !next->isAllocated) {
        CM_DBG_INFO ("Combining NEXT into CURRENT");
        currentNode->netNodeSize += next->netNodeSize;
        list_remove (&next->freenode);
        list_remove (&next->adjnode);
    }

    if (prev && !prev->isAllocated) {
        CM_DBG_INFO ("Combining CURRENT into PREV");
        prev->netNodeSize += currentNode->netNodeSize;
        list_remove (&currentNode->freenode);
        list_remove (&currentNode->adjnode);
    }
}

static CM_MallocHeader* s_createNewNode (void* at, size_t netSize)
{
    // Node netSize too large. Not possible.
    cm_assert (((PTR)at + netSize - 1) < ((PTR)s_buffer + CM_MALLOC_MEM_SIZE_BYTES));

    CM_MallocHeader* newH = at;
    newH->netNodeSize     = netSize;
    newH->isAllocated     = false;
    list_init (&newH->freenode);
    list_init (&newH->allocnode);
    list_init (&newH->adjnode);
    return newH;
}

static CM_MallocHeader* s_getMallocHeaderFromList (ListNode* head, ListNode* node)
{
    CM_MallocHeader* ret = NULL;
    if (head == &s_allocHead) {
        ret = LIST_ITEM (node, CM_MallocHeader, allocnode);
    } else if (head == &s_freeHead) {
        ret = LIST_ITEM (node, CM_MallocHeader, freenode);
    }

    return ret;
}

static CM_MallocHeader* s_findFirst (ListNode* head, FindCriteria criteria, PTR value)
{
    bool found              = false;
    ListNode* node          = NULL;
    CM_MallocHeader* header = NULL;

    CM_DBG_INFO ("Searching for value %lu (%lx)", value, value);

    list_for_each (head, node)
    {
        header = s_getMallocHeaderFromList (head, node);
        CM_DBG_INFO ("  (%px) Size = %lu...", header, header->netNodeSize);

        switch (criteria) {
        case FIND_CRIT_NODE_ADDRESS:
            found = ((PTR)header == value);
            break;
        case FIND_CRIT_NODE_SIZE:
            found = (header->netNodeSize >= value);
            break;
        }

        if (found) {
            CM_DBG_INFO ("Found.");
            break;
        }
    }
    return (found) ? header : NULL;
}

static void s_splitFreeNode (size_t bytes, CM_MallocHeader* freeNodeHdr)
{
    size_t netAllocSize  = NET_ALLOCATION_SIZE (bytes);
    PTR splitAt          = (PTR)freeNodeHdr + netAllocSize;
    size_t remainingSize = freeNodeHdr->netNodeSize - netAllocSize;
    cm_assert (remainingSize >= sizeof (CM_MallocHeader)); // Not enough space for cm_malloc header

    CM_MallocHeader* newFreeNodeHdr = s_createNewNode ((void*)splitAt, remainingSize);
    freeNodeHdr->netNodeSize        = netAllocSize;
    freeNodeHdr->isAllocated        = true;

    list_add_after (&freeNodeHdr->freenode, &newFreeNodeHdr->freenode);
    list_add_after (&freeNodeHdr->adjnode, &newFreeNodeHdr->adjnode);
    list_remove (&freeNodeHdr->freenode);
    list_add_before (&s_allocHead, &freeNodeHdr->allocnode);
}
