#include <kmalloc.h>
#include <intrusive_list.h>
#include <mock/kernel/kmalloc.h>
#include <unittest/unittest.h>
#include <stdlib.h>
#include <stdio.h>
#include <kerror.h>
#include <utils.h>

#define KMALLOC_SIZE_BYTES PAGEFRAMES_TO_BYTES (KMALLOC_SIZE_PAGES)

#ifdef DEBUG
    #define PRINT_EVERY_LIST(title)                     \
        do                                              \
        {                                               \
            printf ("\n=======[ %s ]=======\n", title); \
            printf ("Buffer is at %p", buffer);         \
            printList (FREE_LIST);                      \
            printList (ALLOC_LIST);                     \
            printList (ADJ_LIST);                       \
            printf ("\n");                              \
        } while (0)
#else
    #define PRINT_EVERY_LIST(title) (void)0
#endif

typedef enum MallocLists
{
    FREE_LIST,
    ALLOC_LIST,
    ADJ_LIST
} MallocLists;

ListNode freeHead, allocHead, adjHead;

static void                 printList (MallocLists list);
static inline size_t        getNodeSize (size_t allocSz);
static inline MallocHeader* calculateHeaderLocation (void* usableAddrStart);
static bool                 headerContainsAddress (uintptr_t addr, MallocHeader* node);
static MallocHeader*        getMallocHeaderFromList (MallocLists list, ListNode* node);
static bool                 isAddressFoundInList (void* addr, MallocLists list);
static size_t               getCapacity (MallocLists list);

__attribute__ ((aligned (4096))) char kmalloc_buffer[KMALLOC_SIZE_BYTES];

KernelErrorCodes k_errorNumber;
char*            k_errorText[] = {};

TEST (kmalloc, allocation_space_available)
{
    // Pre-Condition: Nothing
    // ------------------------------------------------------------------------------------------
    void* addr1 = kmalloc (KMALLOC_SIZE_BYTES / 3);
    void* addr2 = kmalloc (KMALLOC_SIZE_BYTES / 4);

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
    // Pre-condition: This large allocation causes the 2nd allocation to fail.
    size_t allocSz = KMALLOC_SIZE_BYTES - sizeof (MallocHeader) * 2;
    NEQ_SCALAR (kmalloc (allocSz), NULL);
    size_t freeListCapPrev  = getCapacity (FREE_LIST);
    size_t allocListCapPrev = getCapacity (ALLOC_LIST);
    // ------------------------------------------------------------------------------------------

    // Allocation fails because there is not enough space.
    void* addr = kmalloc (1);
    EQ_SCALAR (addr, NULL);

    // Alloc, Free list sizes must not change.
    EQ_SCALAR (getCapacity (FREE_LIST), freeListCapPrev);
    EQ_SCALAR (getCapacity (ALLOC_LIST), allocListCapPrev);
    END();
}

TEST (kmalloc, adj_list_continous)
{
    // Pre-condition: A number of successful allocations and freeing done.
    void* addr;
    NEQ_SCALAR (kmalloc (KMALLOC_SIZE_BYTES / 5), NULL);
    NEQ_SCALAR ((addr = kmalloc (KMALLOC_SIZE_BYTES / 4)), NULL);
    NEQ_SCALAR (kmalloc (KMALLOC_SIZE_BYTES / 7), NULL);

    kfree (addr);

    NEQ_SCALAR (kmalloc (KMALLOC_SIZE_BYTES / 7), NULL);
    // ------------------------------------------------------------------------------------------
    // Adjacent list must have nodes in sorted order of addresses.
    MallocHeader* prevHeader = NULL;
    MallocHeader* header     = NULL;
    ListNode*     node       = NULL;
    list_for_each (&adjHead, node)
    {
        header = LIST_ITEM (node, MallocHeader, adjnode);
        GEQ_SCALAR (header, prevHeader);
        prevHeader = header;
    }

    // The total capacity of Adj list must always be equal to HEAP size.
    EQ_SCALAR (getCapacity (ADJ_LIST), KMALLOC_SIZE_BYTES);

    END();
}

TEST (kfree, kfree_combining_adj_nodes)
{
    // Pre-condition: A number of successful allocations and freeing done.
    void *addr1, *addr2, *addr3;
    NEQ_SCALAR (kmalloc (100), NULL);
    NEQ_SCALAR ((addr1 = kmalloc (50)), NULL);
    NEQ_SCALAR ((addr2 = kmalloc (20)), NULL); //[++++++++++][+++++][++++][-----------------------]

    kfree (addr1); //[++++++++++][-----][++++][-----------------------]
    kfree (addr2); //[++++++++++][--------------------------------]

    NEQ_SCALAR ((addr3 = kmalloc (30)), NULL);
    NEQ_SCALAR (kmalloc (20), NULL); //[++++++++++][++++][+++][-------------------------]
    kfree (addr3);                   //[++++++++++][----][+++][-------------------------]

    // ------------------------------------------------------------------------------------------
    // Check if adjacent free nodes were combined as required.
    struct nodeAttrib
    {
        size_t nodeSize;
        bool   isAllocated;
    } expNodeAttribs[] = {
        { getNodeSize (100), true },
        { getNodeSize (30), false },
        { getNodeSize (20), true },
        { KMALLOC_SIZE_BYTES - getNodeSize (100) - getNodeSize (30) - getNodeSize (20), false }
    };

    ListNode*     node   = NULL;
    MallocHeader* header = NULL;
    int           i      = 0;
    list_for_each (&adjHead, node)
    {
        header = LIST_ITEM (node, MallocHeader, adjnode);
        EQ_SCALAR (header->netNodeSize, expNodeAttribs[i].nodeSize);
        EQ_SCALAR (header->isAllocated, expNodeAttribs[i].isAllocated);
        i++;
    }

    END();
}

#if 0

// I am not sure of the benefits of each allocation not crossing page boundary. It may not always be
// fesible either!

TEST(kmalloc, allocation_alignment) {
    // Pre-condition: KMALLOC_SIZE_BYTES > 4096 (1 page size)
    // ------------------------------------------------------------------------------------------

    //time_t epoch_seconds;
    //time (&epoch_seconds);
    //srand (epoch_seconds);

    void *addr = NULL;
    for (size_t size = 10; (addr = kmalloc(size)) != NULL; size = rand() % 1000) {
        MallocHeader *header = calculateHeaderLocation(addr);
        uintptr_t start = (uintptr_t)header;
        uintptr_t end = (uintptr_t)header + header->netNodeSize;

        uintptr_t startPageNumber = start / 4096;
        uintptr_t endPageNumber = end / 4096;

        EQ_SCALAR(startPageNumber, endPageNumber);
    }
    END();
}
#endif

TEST (kmalloc, allocation_list_capacity)
{
    // Pre-condition: Allocations are successfull, such that there is little space left in free
    // list.
    size_t totalNodeSizes = 0;

    NEQ_SCALAR (kmalloc (KMALLOC_SIZE_BYTES / 3), NULL);
    NEQ_SCALAR (kmalloc (KMALLOC_SIZE_BYTES / 4), NULL);
    totalNodeSizes += getNodeSize (KMALLOC_SIZE_BYTES / 3);
    totalNodeSizes += getNodeSize (KMALLOC_SIZE_BYTES / 4);
    // ------------------------------------------------------------------------------------------

    // Total capacity must combine to be equal to buffer size.
    EQ_SCALAR (getCapacity (FREE_LIST) + getCapacity (ALLOC_LIST), KMALLOC_SIZE_BYTES);

    // Free list capacity must reduce equal to the node size.
    EQ_SCALAR (getCapacity (FREE_LIST), (KMALLOC_SIZE_BYTES - totalNodeSizes));

    // Alloc list capacity must increase equal to the node size.
    EQ_SCALAR (getCapacity (ALLOC_LIST), totalNodeSizes);

    END();
}

TEST (kfree, kfree_success)
{
    // Pre-condition: Allocations are successfull, such that there is little space left in free
    // list.
    void* addr1 = kmalloc (KMALLOC_SIZE_BYTES / 5);
    void* addr2 = kmalloc (KMALLOC_SIZE_BYTES / 4);
    void* addr3 = kmalloc (KMALLOC_SIZE_BYTES / 7);

    NEQ_SCALAR (addr1, NULL);
    NEQ_SCALAR (addr2, NULL);
    NEQ_SCALAR (addr3, NULL);
    // ------------------------------------------------------------------------------------------

    kfree (addr2);

    // Free list must now contain the freed address
    EQ_SCALAR (false, isAddressFoundInList (addr1, FREE_LIST));
    EQ_SCALAR (true, isAddressFoundInList (addr2, FREE_LIST));
    EQ_SCALAR (false, isAddressFoundInList (addr3, FREE_LIST));

    // Alloc list must not contain the freed address
    EQ_SCALAR (true, isAddressFoundInList (addr1, ALLOC_LIST));
    EQ_SCALAR (false, isAddressFoundInList (addr2, ALLOC_LIST));
    EQ_SCALAR (true, isAddressFoundInList (addr3, ALLOC_LIST));

    // IsAllocated flag must be set for nodes with are in allocation list.
    MallocHeader* addr1header = calculateHeaderLocation (addr1);
    MallocHeader* addr2header = calculateHeaderLocation (addr2);
    MallocHeader* addr3header = calculateHeaderLocation (addr3);
    EQ_SCALAR (addr1header->isAllocated, true);
    EQ_SCALAR (addr2header->isAllocated, false);
    EQ_SCALAR (addr3header->isAllocated, true);

    END();
}

TEST (kmalloc, kmalloc_header_placement)
{
    // Pre-condition: Allocations are successfull, such that there is little space left in free
    // list.
    void*         addr1 = kmalloc (KMALLOC_SIZE_BYTES / 5);
    MallocHeader* node1 = calculateHeaderLocation (addr1);

    void*         addr2 = kmalloc (KMALLOC_SIZE_BYTES / 6);
    MallocHeader* node2 = calculateHeaderLocation (addr2);

    void*         addr3 = kmalloc (KMALLOC_SIZE_BYTES / 10);
    MallocHeader* node3 = calculateHeaderLocation (addr3);

    NEQ_SCALAR (addr1, NULL);
    NEQ_SCALAR (addr2, NULL);
    NEQ_SCALAR (addr3, NULL);
    // ------------------------------------------------------------------------------------------

    EQ_MEM (kmalloc_buffer, node1, sizeof (MallocHeader));
    EQ_MEM (kmalloc_buffer + node1->netNodeSize, node2, sizeof (MallocHeader));
    EQ_MEM (kmalloc_buffer + node1->netNodeSize + node2->netNodeSize, node3, sizeof (MallocHeader));
    END();
}

void reset()
{
    s_PreAllocateMemory_fake.ret = kmalloc_buffer;

    kmalloc_init();
}

static bool headerContainsAddress (uintptr_t addr, MallocHeader* node)
{
    uintptr_t start = (uintptr_t)node;
    uintptr_t end   = (uintptr_t)node + node->netNodeSize;
    return (addr >= start && addr < end);
}

static ListNode* getListHead (MallocLists list)
{
    switch (list)
    {
    case FREE_LIST:
        return &freeHead;
    case ALLOC_LIST:
        return &allocHead;
    case ADJ_LIST:
        return &adjHead;
    };
}

static MallocHeader* getMallocHeaderFromList (MallocLists list, ListNode* node)
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
}

static bool isAddressFoundInList (void* addr, MallocLists list)
{
    ListNode* head = getListHead (list);
    ListNode* node = NULL;

    list_for_each (head, node)
    {
        if (headerContainsAddress ((uintptr_t)addr, getMallocHeaderFromList (list, node)))
            return true;
    }

    return false;
}

static size_t getCapacity (MallocLists list)
{
    ListNode* head = getListHead (list);
    ListNode* node = NULL;
    size_t    capa = 0;

    list_for_each (head, node) { capa += getMallocHeaderFromList (list, node)->netNodeSize; }

    return capa;
}

static inline size_t getNodeSize (size_t allocSz) { return sizeof (MallocHeader) + allocSz; }

static inline MallocHeader* calculateHeaderLocation (void* usableAddrStart)
{
    return usableAddrStart - sizeof (MallocHeader);
}

static void printList (MallocLists list)
{
    char* listName = NULL;
    switch (list)
    {
    case FREE_LIST:
        listName = "FREE LIST";
        break;
    case ALLOC_LIST:
        listName = "ALLOC LIST";
        break;
    case ADJ_LIST:
        listName = "ADJ LIST";
        break;
    }

    printf ("\n-------[ %s ]--------\n", listName);

    ListNode* head = getListHead (list);
    printf ("[{%p}] <->", head);
    ListNode* node = NULL;
    list_for_each (head, node)
    {
        MallocHeader* header = getMallocHeaderFromList (list, node);
        printf ("[%p {0x%p} Sz = %lu {%c}] <->", header, node, header->netNodeSize,
                (header->isAllocated) ? 'A' : 'F');
    }

    printf ("[{%p}]", node);
}

int main()
{
    allocation_space_available();
    allocation_space_uavailable();
    allocation_list_capacity();
    // allocation_alignment();
    kmalloc_header_placement();
    kfree_success();
    kfree_combining_adj_nodes();
    adj_list_continous();

    return 0;
}
