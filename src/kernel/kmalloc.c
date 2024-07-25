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
#include <kernel.h>
#include <memloc.h>
#include <vmm.h>

typedef enum FindCriteria
{
    FIND_CRIT_NODE_SIZE,
    FIND_CRIT_NODE_ADDRESS
} FindCriteria;

static MallocHeader* s_createNewNode (void* at, size_t netSize);
static MallocHeader* s_findFirst (ListNode* head, FindCriteria criteria, PTR value);
static MallocHeader* s_getMallocHeaderFromList (ListNode* head, ListNode* node);
static void s_splitFreeNode (size_t bytes, MallocHeader* freeNodeHdr);
static void s_combineAdjFreeNodes (MallocHeader* currentNode);

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

    KERNEL_PHASE_VALIDATE(KERNEL_PHASE_STATE_VMM_READY);

    s_buffer = (void*)kvmm_alloc (g_kstate.kernelVMM,
                                  BYTES_TO_PAGEFRAMES_CEILING (ARCH_MEM_LEN_BYTES_KMALLOC),
                                  PG_MAP_FLAG_KERNEL_DEFAULT, VMM_ADDR_SPACE_FLAG_NONE);
    if (s_buffer == NULL) {
        k_panicOnError();
    }

    MallocHeader* newH = s_createNewNode (s_buffer, ARCH_MEM_LEN_BYTES_KMALLOC);
    list_add_before (&s_freeHead, &newH->freenode);
    list_add_before (&s_adjHead, &newH->adjnode);

    KERNEL_PHASE_SET(KERNEL_PHASE_STATE_KMALLOC_READY);

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
    FUNC_ENTRY ("Bytes: %x", bytes);

    KERNEL_PHASE_VALIDATE(KERNEL_PHASE_STATE_KMALLOC_READY);

    INFO ("Requested net size of %lu bytes", NET_ALLOCATION_SIZE (bytes));

    // Search for suitable node
    size_t searchAllocSize = NET_ALLOCATION_SIZE (bytes) + sizeof (MallocHeader);
    MallocHeader* node     = s_findFirst (&s_freeHead, FIND_CRIT_NODE_SIZE, searchAllocSize);

    if (node != NULL)
    {
        k_assert (node->netNodeSize >= NET_ALLOCATION_SIZE (bytes), "Found node too small");

        // Split the free node into two.
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
    FUNC_ENTRY ("Address: %px", addr);

    KERNEL_PHASE_VALIDATE(KERNEL_PHASE_STATE_KMALLOC_READY);

    void* headerAddress    = (void*)((PTR)addr - sizeof (MallocHeader));
    MallocHeader* allocHdr = s_findFirst (&s_allocHead, FIND_CRIT_NODE_ADDRESS, (PTR)headerAddress);
    if (allocHdr != NULL)
    {
        INFO ("Free at %px, Size = %lu", allocHdr, allocHdr->netNodeSize);
        list_remove (&allocHdr->allocnode);
        list_add_after (&s_freeHead, &allocHdr->freenode);
        allocHdr->isAllocated = false;

        s_combineAdjFreeNodes (allocHdr);
        return true;
    }

    ERROR ("Adderss '%px' not found.", addr);
    RETURN_ERROR (ERR_INVALID_ARGUMENT, false);
}

/***************************************************************************************************
 * Gets the amount of memory allocated by kmalloc
 *
 * @return          Amount of allocated memory in bytes.
 **************************************************************************************************/
SIZE kmalloc_getUsedMemory()
{
    FUNC_ENTRY();

    KERNEL_PHASE_VALIDATE(KERNEL_PHASE_STATE_KMALLOC_READY);

    SIZE usedSz = 0U;
    ListNode* node;

    INFO ("Kmalloc buffer size: %u bytes", ARCH_MEM_LEN_BYTES_KMALLOC);

    list_for_each (&s_allocHead, node)
    {
        MallocHeader* header = LIST_ITEM (node, MallocHeader, allocnode);
        INFO ("Node size: %u bytes", header->netNodeSize);
        k_assert (header && (header->netNodeSize < ARCH_MEM_LEN_BYTES_KMALLOC),
                  "Invalid state of kmalloc data");
        usedSz += header->netNodeSize;
    }

    return usedSz;
}

static void s_combineAdjFreeNodes (MallocHeader* currentNode)
{
    MallocHeader* next = NULL;
    MallocHeader* prev = NULL;

    k_assert (currentNode->isAllocated == false, "Cannot combine allocated node. Invalid input.");

    // Prev node could be the list head (which is not a MallocHeader). That would mean beginning of
    // list and is not taken into account.
    if (currentNode->adjnode.prev != &s_adjHead)
    {
        prev = LIST_ITEM (currentNode->adjnode.prev, MallocHeader, adjnode);
    }

    // Next node could be the list head (which is not a MallocHeader). That would mean end of list
    // and is not taken into account (Practically this is not possible as there will always be
    // one Free section at the end of kmalloc buffer).
    if (currentNode->adjnode.next != &s_adjHead)
    {
        next = LIST_ITEM (currentNode->adjnode.next, MallocHeader, adjnode);
    }

    if (next && !next->isAllocated)
    {
        INFO ("Combining NEXT into CURRENT");
        currentNode->netNodeSize += next->netNodeSize;
        list_remove (&next->freenode);
        list_remove (&next->adjnode);
    }

    if (prev && !prev->isAllocated)
    {
        INFO ("Combining CURRENT into PREV");
        prev->netNodeSize += currentNode->netNodeSize;
        list_remove (&currentNode->freenode);
        list_remove (&currentNode->adjnode);
    }
}

static MallocHeader* s_createNewNode (void* at, size_t netSize)
{
    k_assert (((PTR)at + netSize - 1) < ((PTR)s_buffer + ARCH_MEM_LEN_BYTES_KMALLOC),
              "Node netSize too large");

    MallocHeader* newH = at;
    newH->netNodeSize  = netSize;
    newH->isAllocated  = false;
    list_init (&newH->freenode);
    list_init (&newH->allocnode);
    list_init (&newH->adjnode);
    return newH;
}

static MallocHeader* s_getMallocHeaderFromList (ListNode* head, ListNode* node)
{
    MallocHeader* ret = NULL;
    if (head == &s_allocHead)
    {
        ret = LIST_ITEM (node, MallocHeader, allocnode);
    }
    else if (head == &s_freeHead)
    {
        ret = LIST_ITEM (node, MallocHeader, freenode);
    }

    return ret;
}

static MallocHeader* s_findFirst (ListNode* head, FindCriteria criteria, PTR value)
{
    bool found           = false;
    ListNode* node       = NULL;
    MallocHeader* header = NULL;

    INFO ("Searching for value %lu (%lx)", value, value);

    list_for_each (head, node)
    {
        header = s_getMallocHeaderFromList (head, node);
        INFO ("  (%px) Size = %lu...", header, header->netNodeSize);

        switch (criteria)
        {
        case FIND_CRIT_NODE_ADDRESS:
            found = ((PTR)header == value);
            break;
        case FIND_CRIT_NODE_SIZE:
            found = (header->netNodeSize >= value);
            break;
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
    size_t netAllocSize  = NET_ALLOCATION_SIZE (bytes);
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
