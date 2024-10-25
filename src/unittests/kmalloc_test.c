#include <memmanage.h>
#include <intrusive_list.h>
#include <unittest/unittest.h>
#include <stdlib.h>
#include <stdio.h>
#include <kerror.h>
#include <utils.h>
#include <assert.h>
#include <kernel.h>
#include <mosunittest.h>
#include <mock/kernel/vmm.h>
#include <mock/kernel/kstdlib.h>

/* |Test case description                                       | Test function name             |
 * |------------------------------------------------------------|--------------------------------|
 * | kmalloc: Space available for allocation. Success           | allocation_space_available     |
 * | kmalloc: Space un-available for allocation. Out of memory  | allocation_space_uavailable    |
 * | kfree: Address input found. No combining. Freed            | kfree_success                  |
 * | kfree: Address input found. Combining next. Freed          | kfree_combining_next_adj_nodes |
 * | kfree: Address input found. Combining prev. Freed          | kfree_combining_prev_adj_nodes |
 * | kfree: Address input not found. Invalid argument error     | kfree_wrong_input              |
 * | kmalloc_getUsedMemory: When no memory is allocated         | used_memory_test               |
 * | kmalloc_getUsedMemory: When some memory is allocated       | used_memory_test               |
 * |------------------------------------------------------------|--------------------------------|
 */

typedef enum MallocLists {
    FREE_LIST,
    ALLOC_LIST,
    ADJ_LIST
} MallocLists;

typedef struct SectionAttributes {
    size_t nodeSize;
    bool isAllocated;
} SectionAttributes;

extern ListNode s_freeHead, s_allocHead, s_adjHead;
ListNode s_freeHead, s_allocHead, s_adjHead;

// At this point we cannot create Fake of functions with variac parameters.
void kdisp_importantPrint (char* fmt, ...)
{ // Dummy function
}

// The kmalloc buffer size must be large enough to meet the test expectations.
#define UT_KMALLOC_SIZE_BYTES 400
char kmalloc_buffer[UT_KMALLOC_SIZE_BYTES];

static inline size_t getNodeSize (size_t usableSize)
{
    return sizeof (MallocHeader) + usableSize;
}

static inline MallocHeader* calculateHeaderLocation (void* usableAddrStart)
{
    return usableAddrStart - sizeof (MallocHeader);
}

static inline void* calculateUsableAddressStart (MallocHeader* headerAddr)
{
    return (void*)headerAddr + sizeof (MallocHeader);
}

static MallocHeader* getMallocHeaderFromList (MallocLists list, ListNode* node);
static bool isAddressFoundInList (void* addr, MallocLists list);
static size_t getCapacity (MallocLists list);
static void matchSectionPlacementAndAttributes (SectionAttributes* secAttrs, size_t count);

TEST (kmallocz, zero_fill_allocation)
{
    // Pre-Condition: Prefill the kmalloc array with non-zeros to check if kmallocz worked.
    memset (kmalloc_buffer, 0xFF, UT_KMALLOC_SIZE_BYTES);

    // Need to re-init kmalloc buffer since we have overriden initial headers with the previous
    // memset.
    kmalloc_init();

    // Not an exact match, but will do for now. Ideally we could have used EXPECT_CALL macro to
    // check if k_memset is called with the expected arguments.
    k_memset_fake.handler = memset;
    // ------------------------------------------------------------------------------------------
    U8* addr1       = kmallocz (10);
    U8 expected[10] = { 0 };

    EQ_MEM (addr1, expected, 10);

    END();
}

TEST (kmalloc, allocation_space_available)
{
    // Pre-Condition: Nothing
    // ------------------------------------------------------------------------------------------
    void* addr1 = kmalloc (UT_KMALLOC_SIZE_BYTES / 3);
    void* addr2 = kmalloc (UT_KMALLOC_SIZE_BYTES / 4);

    // Two addresses must be different.
    NEQ_SCALAR (addr2, addr1);

    // Free list must not contain these addresses.
    EQ_SCALAR (false, isAddressFoundInList (addr1, FREE_LIST));
    EQ_SCALAR (false, isAddressFoundInList (addr2, FREE_LIST));

    // Alloc list must contain these addresses.
    EQ_SCALAR (true, isAddressFoundInList (addr1, ALLOC_LIST));
    EQ_SCALAR (true, isAddressFoundInList (addr2, ALLOC_LIST));

    END();
}

TEST (kmalloc, allocation_space_uavailable)
{
    // Pre-condition: Nothing
    size_t freeListCapPrev  = getCapacity (FREE_LIST);
    size_t allocListCapPrev = getCapacity (ALLOC_LIST);
    // ------------------------------------------------------------------------------------------

    // Allocation fails because there is not enough space.
    EQ_SCALAR (kmalloc (UT_KMALLOC_SIZE_BYTES), NULL);
    EQ_SCALAR (g_kstate.errorNumber, ERR_OUT_OF_MEM);

    // Alloc, Free list sizes must not change.
    EQ_SCALAR (getCapacity (FREE_LIST), freeListCapPrev);
    EQ_SCALAR (getCapacity (ALLOC_LIST), allocListCapPrev);
    END();
}

TEST (kfree, kfree_combining_next_adj_nodes)
{
    // Pre-condition: A number of successful allocations and freeing such that the left
    void *addr1, *addr2;
    NEQ_SCALAR ((addr1 = kmalloc (100)), NULL);
    NEQ_SCALAR ((addr2 = kmalloc (50)), NULL);
    NEQ_SCALAR (kmalloc (50), NULL); //[A,A,A,F]
    EQ_SCALAR (kfree (addr2), true); //[A,F,A,F]
    // ------------------------------------------------------------------------------------------
    EQ_SCALAR (kfree (addr1), true); //[F][A][F]

    // Check if adjacent free nodes were combined as required.
    SectionAttributes expAttrs[] = {
        { getNodeSize (100) + getNodeSize (50), false },
        { getNodeSize (50), true },
        { UT_KMALLOC_SIZE_BYTES - (getNodeSize (100) + getNodeSize (50)) - getNodeSize (50), false }
    };

    matchSectionPlacementAndAttributes (expAttrs, ARRAY_LENGTH (expAttrs));
    END();
}

TEST (kfree, kfree_combining_prev_adj_nodes)
{
    // Pre-condition: A number of successful allocations and freeing such that the left
    void *addr1, *addr2;
    NEQ_SCALAR ((addr1 = kmalloc (100)), NULL);
    NEQ_SCALAR ((addr2 = kmalloc (50)), NULL);
    NEQ_SCALAR (kmalloc (50), NULL); //[A][A][A][F]
    EQ_SCALAR (kfree (addr1), true); //[F][A][A][F]
    // ------------------------------------------------------------------------------------------

    EQ_SCALAR (kfree (addr2), true); //[F][A][F]

    // Check if adjacent free nodes were combined as required.
    SectionAttributes expAttrs[] = {
        { getNodeSize (100) + getNodeSize (50), false },
        { getNodeSize (50), true },
        { UT_KMALLOC_SIZE_BYTES - (getNodeSize (100) + getNodeSize (50)) - getNodeSize (50), false }
    };

    matchSectionPlacementAndAttributes (expAttrs, ARRAY_LENGTH (expAttrs));

    END();
}

TEST (kfree, kfree_success)
{
    // Pre-condition: Allocations are successfull, such that there is little space left in free
    // list.
    void* addr1 = kmalloc (UT_KMALLOC_SIZE_BYTES / 5);
    void* addr2 = kmalloc (UT_KMALLOC_SIZE_BYTES / 4);
    void* addr3 = kmalloc (UT_KMALLOC_SIZE_BYTES / 7);

    NEQ_SCALAR (addr1, NULL);
    NEQ_SCALAR (addr2, NULL);
    NEQ_SCALAR (addr3, NULL);
    // ------------------------------------------------------------------------------------------

    EQ_SCALAR (true, kfree (addr2));

    // Free list must now contain the freed address
    EQ_SCALAR (false, isAddressFoundInList (addr1, FREE_LIST));
    EQ_SCALAR (true, isAddressFoundInList (addr2, FREE_LIST));
    EQ_SCALAR (false, isAddressFoundInList (addr3, FREE_LIST));

    // Alloc list must not contain the freed address
    EQ_SCALAR (true, isAddressFoundInList (addr1, ALLOC_LIST));
    EQ_SCALAR (false, isAddressFoundInList (addr2, ALLOC_LIST));
    EQ_SCALAR (true, isAddressFoundInList (addr3, ALLOC_LIST));

    // IsAllocated flag must be set for nodes with are in allocation list.
    EQ_SCALAR (calculateHeaderLocation (addr1)->isAllocated, true);
    EQ_SCALAR (calculateHeaderLocation (addr2)->isAllocated, false);
    EQ_SCALAR (calculateHeaderLocation (addr3)->isAllocated, true);

    END();
}

TEST (kfree, kfree_wrong_input)
{
    EQ_SCALAR (kfree (NULL), false);
    EQ_SCALAR (g_kstate.errorNumber, ERR_DOUBLE_FREE);
    END();
}

TEST (kmalloc_getUsedMemory, used_memory_test)
{
    // When there are no allocation
    EQ_SCALAR (kmalloc_getUsedMemory(), 0U);

    // When some amount of memory is allocated.
    NEQ_SCALAR (kmalloc (100), NULL);
    NEQ_SCALAR (kmalloc (50), NULL);

    EQ_SCALAR (kmalloc_getUsedMemory(), getNodeSize (100) + getNodeSize (50));
    END();
}

// ------------------------------------------------------------------------------------------

static void matchSectionPlacementAndAttributes (SectionAttributes* secAttrs, size_t count)
{
    ListNode* node       = NULL;
    MallocHeader* header = NULL;
    int i                = 0;
    list_for_each (&s_adjHead, node)
    {
        assert (i < count);
        header = LIST_ITEM (node, MallocHeader, adjnode);
        EQ_SCALAR (header->netNodeSize, secAttrs[i].nodeSize);
        EQ_SCALAR (header->isAllocated, secAttrs[i].isAllocated);
        i++;
    }
}

static ListNode* getListHead (MallocLists list)
{
    switch (list) {
    case FREE_LIST:
        return &s_freeHead;
    case ALLOC_LIST:
        return &s_allocHead;
    case ADJ_LIST:
        return &s_adjHead;
    };
}

static MallocHeader* getMallocHeaderFromList (MallocLists list, ListNode* node)
{
    switch (list) {
    case FREE_LIST:
        return LIST_ITEM (node, MallocHeader, freenode);
    case ALLOC_LIST:
        return LIST_ITEM (node, MallocHeader, allocnode);
    case ADJ_LIST:
        return LIST_ITEM (node, MallocHeader, adjnode);
    };
}

static bool isAddressFoundInList (void* addr, MallocLists list)
{
    ListNode* head = getListHead (list);
    ListNode* node = NULL;

    list_for_each (head, node)
    {
        MallocHeader* header = getMallocHeaderFromList (list, node);
        if ((PTR)header == (PTR)calculateHeaderLocation (addr))
            return true;
    }

    return false;
}

static size_t getCapacity (MallocLists list)
{
    ListNode* head = getListHead (list);
    ListNode* node = NULL;
    size_t capa    = 0;

    list_for_each (head, node)
    {
        capa += getMallocHeaderFromList (list, node)->netNodeSize;
    }

    return capa;
}

void reset()
{
    resetVMMFake();
    kvmm_memmap_fake.ret              = (PTR)kmalloc_buffer;
    g_utmm.arch_mem_len_bytes_kmalloc = UT_KMALLOC_SIZE_BYTES;

    kmalloc_init();
}

int main()
{
    allocation_space_available();
    allocation_space_uavailable();
    kfree_success();
    kfree_combining_prev_adj_nodes();
    kfree_combining_next_adj_nodes();
    kfree_wrong_input();
    used_memory_test();
    zero_fill_allocation();

    return 0;
}
