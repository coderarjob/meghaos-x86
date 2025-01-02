#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <utils.h>
#include <mosunittest.h>
#include <intrusive_list.h>
#include <unittest/unittest.h>
#include <unittest/fake.h>
#ifdef LIBCM
    #include <kcmlib.h>
    #include <cm/cm.h>
    #include <cm/err.h>
    #include <cm/osif.h>
    #include <mock/cm/cm.h>

    typedef CM_MallocHeader MallocHeader;
    #define MALLOC_FN_UNDER_TEST      cm_malloc
    #define FREE_FN_UNDER_TEST        cm_free
    #define MALLOCZ_FN_UNDER_TEST     cm_calloc
    #define MALLOC_INIT_FN_UNDER_TEST cm_malloc_init
#else
    #include <memmanage.h>
    #include <kerror.h>
    #include <kernel.h>
    #include <mock/kernel/vmm.h>
    #include <mock/kernel/kstdlib.h>

    typedef KMallocHeader MallocHeader;
    #define MALLOC_FN_UNDER_TEST      kmalloc
    #define FREE_FN_UNDER_TEST        kfree
    #define MALLOCZ_FN_UNDER_TEST     kmallocz
    #define MALLOC_INIT_FN_UNDER_TEST kmalloc_init
#endif // LIBCM

/* |Test case description                                       | Test function name             |
 * |------------------------------------------------------------|--------------------------------|
 * | malloc: Space available for allocation. Success            | allocation_space_available     |
 * | malloc: Space un-available for allocation. Out of memory   | allocation_space_uavailable    |
 * | free: Address input found. No combining. Freed             | free_success                   |
 * | free: Address input found. Combining next. Freed           | free_combining_next_adj_nodes  |
 * | free: Address input found. Combining prev. Freed           | free_combining_prev_adj_nodes  |
 * | free: Address input not found. Invalid argument error      | free_wrong_input               |
 * | malloc_getUsedMemory: When no memory is allocated          | used_memory_test               |
 * | malloc_getUsedMemory: When some memory is allocated        | used_memory_test               |
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

// The malloc buffer size must be large enough to meet the test expectations.
#define UT_MALLOC_SIZE_BYTES 400
char malloc_buffer[UT_MALLOC_SIZE_BYTES];

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
    memset (malloc_buffer, 0xFF, UT_MALLOC_SIZE_BYTES);

    // Need to re-init kmalloc buffer since we have overriden initial headers with the previous
    // memset.
    MALLOC_INIT_FN_UNDER_TEST();

    // Not an exact match, but will do for now. Ideally we could have used EXPECT_CALL macro to
    // check if k_memset is called with the expected arguments.
#ifdef LIBCM
    cm_memset_fake.handler = memset;
#else
    k_memset_fake.handler = memset;
#endif
    // ------------------------------------------------------------------------------------------
    U8* addr1       = MALLOCZ_FN_UNDER_TEST (10);
    U8 expected[10] = { 0 };

    EQ_MEM (addr1, expected, 10);

    END();
}

TEST (kmalloc, allocation_space_available)
{
    // Pre-Condition: Nothing
    // ------------------------------------------------------------------------------------------
    void* addr1 = MALLOC_FN_UNDER_TEST (UT_MALLOC_SIZE_BYTES / 3);
    void* addr2 = MALLOC_FN_UNDER_TEST (UT_MALLOC_SIZE_BYTES / 4);

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
    EQ_SCALAR (MALLOC_FN_UNDER_TEST (UT_MALLOC_SIZE_BYTES), NULL);
#ifdef LIBCM
    EQ_SCALAR (cm_error_num, CM_ERR_OUT_OF_HEAP_MEM);
#else
    EQ_SCALAR (g_kstate.errorNumber, ERR_OUT_OF_MEM);
#endif

    // Alloc, Free list sizes must not change.
    EQ_SCALAR (getCapacity (FREE_LIST), freeListCapPrev);
    EQ_SCALAR (getCapacity (ALLOC_LIST), allocListCapPrev);
    END();
}

TEST (kfree, free_combining_next_adj_nodes)
{
    // Pre-condition: A number of successful allocations and freeing such that the left
    void *addr1, *addr2;
    NEQ_SCALAR ((addr1 = MALLOC_FN_UNDER_TEST (100)), NULL);
    NEQ_SCALAR ((addr2 = MALLOC_FN_UNDER_TEST (50)), NULL);
    NEQ_SCALAR (MALLOC_FN_UNDER_TEST (50), NULL); //[A,A,A,F]
    EQ_SCALAR (FREE_FN_UNDER_TEST (addr2), true); //[A,F,A,F]
    // ------------------------------------------------------------------------------------------
    EQ_SCALAR (FREE_FN_UNDER_TEST (addr1), true); //[F][A][F]

    // Check if adjacent free nodes were combined as required.
    SectionAttributes expAttrs[] = {
        { getNodeSize (100) + getNodeSize (50), false },
        { getNodeSize (50), true },
        { UT_MALLOC_SIZE_BYTES - (getNodeSize (100) + getNodeSize (50)) - getNodeSize (50), false }
    };

    matchSectionPlacementAndAttributes (expAttrs, ARRAY_LENGTH (expAttrs));
    END();
}

TEST (kfree, free_combining_prev_adj_nodes)
{
    // Pre-condition: A number of successful allocations and freeing such that the left
    void *addr1, *addr2;
    NEQ_SCALAR ((addr1 = MALLOC_FN_UNDER_TEST (100)), NULL);
    NEQ_SCALAR ((addr2 = MALLOC_FN_UNDER_TEST (50)), NULL);
    NEQ_SCALAR (MALLOC_FN_UNDER_TEST (50), NULL); //[A][A][A][F]
    EQ_SCALAR (FREE_FN_UNDER_TEST (addr1), true); //[F][A][A][F]
    // ------------------------------------------------------------------------------------------

    EQ_SCALAR (FREE_FN_UNDER_TEST (addr2), true); //[F][A][F]

    // Check if adjacent free nodes were combined as required.
    SectionAttributes expAttrs[] = {
        { getNodeSize (100) + getNodeSize (50), false },
        { getNodeSize (50), true },
        { UT_MALLOC_SIZE_BYTES - (getNodeSize (100) + getNodeSize (50)) - getNodeSize (50), false }
    };

    matchSectionPlacementAndAttributes (expAttrs, ARRAY_LENGTH (expAttrs));

    END();
}

TEST (kfree, free_success)
{
    // Pre-condition: Allocations are successfull, such that there is little space left in free
    // list.
    void* addr1 = MALLOC_FN_UNDER_TEST (UT_MALLOC_SIZE_BYTES / 5);
    void* addr2 = MALLOC_FN_UNDER_TEST (UT_MALLOC_SIZE_BYTES / 4);
    void* addr3 = MALLOC_FN_UNDER_TEST (UT_MALLOC_SIZE_BYTES / 7);

    NEQ_SCALAR (addr1, NULL);
    NEQ_SCALAR (addr2, NULL);
    NEQ_SCALAR (addr3, NULL);
    // ------------------------------------------------------------------------------------------

    EQ_SCALAR (true, FREE_FN_UNDER_TEST (addr2));

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

TEST (kfree, free_wrong_input)
{
    EQ_SCALAR (FREE_FN_UNDER_TEST (NULL), false);
#ifdef LIBCM
    EQ_SCALAR (cm_panic_invoked, true);
#else
    EQ_SCALAR (g_kstate.errorNumber, ERR_DOUBLE_FREE);
#endif
    END();
}

#ifndef LIBCM
// Kernel only test. No corresponding function in libcm.
TEST (kmalloc_getUsedMemory, used_memory_test)
{
    // When there are no allocation
    EQ_SCALAR (kmalloc_getUsedMemory(), 0U);

    // When some amount of memory is allocated.
    NEQ_SCALAR (MALLOC_FN_UNDER_TEST (100), NULL);
    NEQ_SCALAR (MALLOC_FN_UNDER_TEST (50), NULL);

    EQ_SCALAR (kmalloc_getUsedMemory(), getNodeSize (100) + getNodeSize (50));
    END();
}
#endif

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

#ifdef LIBCM
// TODO: I do not know a clean way to mock a inline function - cm_process_get_datamem_start in this
// case. A workaround is to mock the function called by the inline function, which in this case is
// the syscall function.
S32 syscall_handler (OSIF_SYSCALLS fn, U32 arg1, U32 arg2, U32 arg3, U32 arg4, U32 arg5)
{
    switch (fn) {
    case OSIF_SYSCALL_PROCESS_GET_DATAMEM_START:
        return malloc_buffer;
        break;
    default:
        assert (false);
        break;
    }
    return 0;
}
#endif

void reset()
{
#ifdef LIBCM
    syscall_fake.handler                = syscall_handler;
    g_utmm.cm_arch_mem_len_bytes_malloc = UT_MALLOC_SIZE_BYTES;
#else
    resetVMMFake();
    kvmm_memmap_fake.ret              = (PTR)malloc_buffer;
    g_utmm.arch_mem_len_bytes_kmalloc = UT_MALLOC_SIZE_BYTES;
#endif
    MALLOC_INIT_FN_UNDER_TEST();
}

int main()
{
    allocation_space_available();
    allocation_space_uavailable();
    free_success();
    free_combining_prev_adj_nodes();
    free_combining_next_adj_nodes();
    free_wrong_input();
#ifndef LIBCM
    used_memory_test();
#endif
    zero_fill_allocation();

    return 0;
}
