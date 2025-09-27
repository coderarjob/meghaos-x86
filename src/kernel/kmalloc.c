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
#include <kstdlib.h>

typedef enum FindCriteria
{
    FIND_CRIT_NODE_SIZE,
    FIND_CRIT_NODE_ADDRESS
} FindCriteria;

static KMallocHeader* s_createNewNode (void* at, size_t netSize);
static KMallocHeader* s_findFirst (ListNode* head, FindCriteria criteria, PTR value);
static KMallocHeader* s_getMallocHeaderFromList (ListNode* head, ListNode* node);
static void s_splitFreeNode (size_t bytes, KMallocHeader* freeNodeHdr);
static void s_combineAdjFreeNodes (KMallocHeader* currentNode);

extern ListNode s_freeHead, s_allocHead, s_adjHead;
static void* s_buffer;

#define NET_ALLOCATION_SIZE(sz_bytes) (sz_bytes + sizeof (KMallocHeader))

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
void kmalloc_init(void)
{
    FUNC_ENTRY();

    list_init (&s_freeHead);
    list_init (&s_allocHead);
    list_init (&s_adjHead);

    KERNEL_PHASE_VALIDATE(KERNEL_PHASE_STATE_VMM_READY);

    if (!(s_buffer = (void*)kvmm_memmap (g_kstate.context, (PTR)NULL, NULL,
                                         BYTES_TO_PAGEFRAMES_CEILING (ARCH_MEM_LEN_BYTES_KMALLOC),
                                         VMM_MEMMAP_FLAG_KERNEL_PAGE, NULL))) {
        FATAL_BUG(); // Should not fail.
    }

    KMallocHeader* newH = s_createNewNode (s_buffer, ARCH_MEM_LEN_BYTES_KMALLOC);
    list_add_before (&s_freeHead, &newH->freenode);
    list_add_before (&s_adjHead, &newH->adjnode);

    KERNEL_PHASE_SET(KERNEL_PHASE_STATE_KMALLOC_READY);

    INFO ("Size of KMallocHeader: %lu bytes", sizeof (KMallocHeader));
    INFO ("Malloc buffer is at: %px", s_buffer);
}


/***************************************************************************************************
 * Allocates at least 'bytes' number of bytes from the kmalloc memory. If successful then zeros the
 * memory before returning.
 *
 * @Input   bytes   Number of bytes to allocate.
 * @return          Poiter to the start of the allocated memory. Or NULL on failure.
 **************************************************************************************************/
void* kmallocz (size_t bytes)
{
    FUNC_ENTRY ("Bytes: %x", bytes);

    void* addr = kmalloc (bytes);
    if (addr != NULL) {
        k_memset (addr, 0, bytes);
    }
    return addr;
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
    size_t searchAllocSize = NET_ALLOCATION_SIZE (bytes) + sizeof (KMallocHeader);
    KMallocHeader* node     = s_findFirst (&s_freeHead, FIND_CRIT_NODE_SIZE, searchAllocSize);

    if (node != NULL)
    {
        k_assert (node->netNodeSize >= NET_ALLOCATION_SIZE (bytes), "Found node too small");

        // Split the free node into two.
        s_splitFreeNode (bytes, node);
        return (void*)((PTR)node + sizeof (KMallocHeader));
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

    void* headerAddress    = (void*)((PTR)addr - sizeof (KMallocHeader));
    KMallocHeader* allocHdr = s_findFirst (&s_allocHead, FIND_CRIT_NODE_ADDRESS, (PTR)headerAddress);
    if (allocHdr != NULL)
    {
        INFO ("Free at %px, Size = %lu", allocHdr, allocHdr->netNodeSize);
        list_remove (&allocHdr->allocnode);
        list_add_after (&s_freeHead, &allocHdr->freenode);
        allocHdr->isAllocated = false;

        s_combineAdjFreeNodes (allocHdr);
        return true;
    }

    BUG();
    RETURN_ERROR (ERR_DOUBLE_FREE, false);
}

/***************************************************************************************************
 * Gets the amount of memory allocated by kmalloc
 *
 * @return          Amount of allocated memory in bytes.
 **************************************************************************************************/
SIZE kmalloc_getUsedMemory(void)
{
    FUNC_ENTRY();

    KERNEL_PHASE_VALIDATE(KERNEL_PHASE_STATE_KMALLOC_READY);

    SIZE usedSz = 0U;
    ListNode* node;

    INFO ("Kmalloc buffer size: %u bytes", ARCH_MEM_LEN_BYTES_KMALLOC);

    list_for_each (&s_allocHead, node)
    {
        KMallocHeader* header = LIST_ITEM (node, KMallocHeader, allocnode);
        INFO ("Node size: %u bytes", header->netNodeSize);
        k_assert (header && (header->netNodeSize < ARCH_MEM_LEN_BYTES_KMALLOC),
                  "Invalid state of kmalloc data");
        usedSz += header->netNodeSize;
    }

    return usedSz;
}

static void s_combineAdjFreeNodes (KMallocHeader* currentNode)
{
    KMallocHeader* next = NULL;
    KMallocHeader* prev = NULL;

    k_assert (currentNode->isAllocated == false, "Cannot combine allocated node. Invalid input.");

    // Prev node could be the list head (which is not a KMallocHeader). That would mean beginning of
    // list and is not taken into account.
    if (currentNode->adjnode.prev != &s_adjHead)
    {
        prev = LIST_ITEM (currentNode->adjnode.prev, KMallocHeader, adjnode);
    }

    // Next node could be the list head (which is not a KMallocHeader). That would mean end of list
    // and is not taken into account (Practically this is not possible as there will always be
    // one Free section at the end of kmalloc buffer).
    if (currentNode->adjnode.next != &s_adjHead)
    {
        next = LIST_ITEM (currentNode->adjnode.next, KMallocHeader, adjnode);
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

static KMallocHeader* s_createNewNode (void* at, size_t netSize)
{
    k_assert (((PTR)at + netSize - 1) < ((PTR)s_buffer + ARCH_MEM_LEN_BYTES_KMALLOC),
              "Node netSize too large");

    KMallocHeader* newH = at;
    newH->netNodeSize  = netSize;
    newH->isAllocated  = false;
    list_init (&newH->freenode);
    list_init (&newH->allocnode);
    list_init (&newH->adjnode);
    return newH;
}

static KMallocHeader* s_getMallocHeaderFromList (ListNode* head, ListNode* node)
{
    KMallocHeader* ret = NULL;
    if (head == &s_allocHead)
    {
        ret = LIST_ITEM (node, KMallocHeader, allocnode);
    }
    else if (head == &s_freeHead)
    {
        ret = LIST_ITEM (node, KMallocHeader, freenode);
    }

    return ret;
}

static KMallocHeader* s_findFirst (ListNode* head, FindCriteria criteria, PTR value)
{
    bool found           = false;
    ListNode* node       = NULL;
    KMallocHeader* header = NULL;

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

static void s_splitFreeNode (size_t bytes, KMallocHeader* freeNodeHdr)
{
    size_t netAllocSize  = NET_ALLOCATION_SIZE (bytes);
    PTR splitAt          = (PTR)freeNodeHdr + netAllocSize;
    size_t remainingSize = freeNodeHdr->netNodeSize - netAllocSize;
    k_assert (remainingSize >= sizeof (KMallocHeader), "Not enough space for kmalloc header");

    KMallocHeader* newFreeNodeHdr = s_createNewNode ((void*)splitAt, remainingSize);
    freeNodeHdr->netNodeSize     = netAllocSize;
    freeNodeHdr->isAllocated     = true;

    list_add_after (&freeNodeHdr->freenode, &newFreeNodeHdr->freenode);
    list_add_after (&freeNodeHdr->adjnode, &newFreeNodeHdr->adjnode);
    list_remove (&freeNodeHdr->freenode);
    list_add_before (&s_allocHead, &freeNodeHdr->allocnode);
}
