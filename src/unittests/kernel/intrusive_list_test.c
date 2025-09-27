#define YUKTI_TEST_STRIP_PREFIX
#define YUKTI_TEST_IMPLEMENTATION
#include <unittest/yukti.h>
#include <intrusive_list.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

TEST (LIST, single_node)
{
    ListNode head = { 0 };
    list_init (&head);

    EQ_SCALAR ((uintptr_t)head.next, (uintptr_t)&head);
    EQ_SCALAR ((uintptr_t)head.prev, (uintptr_t)&head);
    END();
}

TEST (LIST, is_empty)
{
    ListNode head = { 0 };
    list_init (&head);

    // Newly initialized list is empty.
    EQ_SCALAR (list_is_empty (&head), true);

    // Add one item to the list
    ListNode n1;
    list_add_before (&head, &n1);

    // List has one item, so is not empty.
    EQ_SCALAR (list_is_empty (&head), false);

    // Remove all items in the list
    list_remove (&n1);

    // List has zero items, so is empty.
    EQ_SCALAR (list_is_empty (&head), true);
    END();
}

TEST (LIST, three_nodes)
{
    ListNode head = { 0 };
    list_init (&head);

    ListNode l1, l2, l3;
    list_init (&l1);
    list_init (&l2);
    list_init (&l3);

    list_add_before (&head, &l1);
    list_add_before (&head, &l2);
    list_add_before (&head, &l3);

    /*
     * | head | -> | l1 | -> | l2 | -> | l3 | -> | head |
     */

    EQ_SCALAR ((uintptr_t)head.next, (uintptr_t)&l1);
    EQ_SCALAR ((uintptr_t)head.prev, (uintptr_t)&l3);

    EQ_SCALAR ((uintptr_t)l1.next, (uintptr_t)&l2);
    EQ_SCALAR ((uintptr_t)l1.prev, (uintptr_t)&head);

    EQ_SCALAR ((uintptr_t)l2.next, (uintptr_t)&l3);
    EQ_SCALAR ((uintptr_t)l2.prev, (uintptr_t)&l1);

    EQ_SCALAR ((uintptr_t)l3.next, (uintptr_t)&head);
    EQ_SCALAR ((uintptr_t)l3.prev, (uintptr_t)&l2);

    END();
}

TEST (LIST, insert_after)
{
    ListNode head = { 0 };
    list_init (&head);

    ListNode l1, l2, l3, l4;
    list_init (&l1);
    list_init (&l2);
    list_init (&l3);
    list_init (&l4);

    list_add_after (&head, &l1);
    list_add_after (&head, &l2);
    list_add_after (&head, &l3);
    list_add_before (&l2, &l4);

    /*
     * | head | -> | l3 | -> | l4 | -> | l2 | -> | l1 | -> | head |
     */

    EQ_SCALAR ((uintptr_t)head.next, (uintptr_t)&l3);
    EQ_SCALAR ((uintptr_t)head.prev, (uintptr_t)&l1);

    EQ_SCALAR ((uintptr_t)l1.next, (uintptr_t)&head);
    EQ_SCALAR ((uintptr_t)l1.prev, (uintptr_t)&l2);

    EQ_SCALAR ((uintptr_t)l2.next, (uintptr_t)&l1);
    EQ_SCALAR ((uintptr_t)l2.prev, (uintptr_t)&l4);

    EQ_SCALAR ((uintptr_t)l3.next, (uintptr_t)&l4);
    EQ_SCALAR ((uintptr_t)l3.prev, (uintptr_t)&head);

    EQ_SCALAR ((uintptr_t)l4.next, (uintptr_t)&l2);
    EQ_SCALAR ((uintptr_t)l4.prev, (uintptr_t)&l3);

    END();
}

TEST (LIST, remove_node_last)
{
    ListNode head = { 0 };
    list_init (&head);

    ListNode l1, l2, l3;
    list_init (&l1);
    list_init (&l2);
    list_init (&l3);

    list_add_before (&head, &l1);
    list_add_before (&head, &l2);
    list_add_before (&head, &l3);

    list_remove (&l3);

    EQ_SCALAR ((uintptr_t)head.next, (uintptr_t)&l1);
    EQ_SCALAR ((uintptr_t)head.prev, (uintptr_t)&l2);

    EQ_SCALAR ((uintptr_t)l1.next, (uintptr_t)&l2);
    EQ_SCALAR ((uintptr_t)l1.prev, (uintptr_t)&head);

    EQ_SCALAR ((uintptr_t)l2.next, (uintptr_t)&head);
    EQ_SCALAR ((uintptr_t)l2.prev, (uintptr_t)&l1);

    EQ_SCALAR ((uintptr_t)l3.next, (uintptr_t)0);
    EQ_SCALAR ((uintptr_t)l3.prev, (uintptr_t)0);

    END();
}

TEST (LIST, remove_node_middle)
{
    ListNode head = { 0 };
    list_init (&head);

    ListNode l1, l2, l3;
    list_init (&l1);
    list_init (&l2);
    list_init (&l3);

    list_add_before (&head, &l1);
    list_add_before (&head, &l2);
    list_add_before (&head, &l3);

    list_remove (&l2);

    EQ_SCALAR ((uintptr_t)head.next, (uintptr_t)&l1);
    EQ_SCALAR ((uintptr_t)head.prev, (uintptr_t)&l3);

    EQ_SCALAR ((uintptr_t)l1.next, (uintptr_t)&l3);
    EQ_SCALAR ((uintptr_t)l1.prev, (uintptr_t)&head);

    EQ_SCALAR ((uintptr_t)l2.next, (uintptr_t)0);
    EQ_SCALAR ((uintptr_t)l2.prev, (uintptr_t)0);

    EQ_SCALAR ((uintptr_t)l3.next, (uintptr_t)&head);
    EQ_SCALAR ((uintptr_t)l3.prev, (uintptr_t)&l1);

    END();
}

TEST (LIST, remove_node_first)
{
    ListNode head = { 0 };
    list_init (&head);

    ListNode l1, l2, l3;
    list_init (&l1);
    list_init (&l2);
    list_init (&l3);

    list_add_before (&head, &l1);
    list_add_before (&head, &l2);
    list_add_before (&head, &l3);

    list_remove (&l1);

    EQ_SCALAR ((uintptr_t)head.next, (uintptr_t)&l2);
    EQ_SCALAR ((uintptr_t)head.prev, (uintptr_t)&l3);

    EQ_SCALAR ((uintptr_t)l1.next, (uintptr_t)0);
    EQ_SCALAR ((uintptr_t)l1.prev, (uintptr_t)0);

    EQ_SCALAR ((uintptr_t)l2.next, (uintptr_t)&l3);
    EQ_SCALAR ((uintptr_t)l2.prev, (uintptr_t)&head);

    EQ_SCALAR ((uintptr_t)l3.next, (uintptr_t)&head);
    EQ_SCALAR ((uintptr_t)l3.prev, (uintptr_t)&l2);

    END();
}

TEST (LIST, remove_node_all)
{
    ListNode head = { 0 };
    list_init (&head);

    ListNode l1, l2, l3;
    list_init (&l1);
    list_init (&l2);
    list_init (&l3);

    list_add_before (&head, &l1);
    list_add_before (&head, &l2);
    list_add_before (&head, &l3);

    list_remove (&l1);
    list_remove (&l2);
    list_remove (&l3);

    EQ_SCALAR ((uintptr_t)head.next, (uintptr_t)&head);
    EQ_SCALAR ((uintptr_t)head.prev, (uintptr_t)&head);

    EQ_SCALAR ((uintptr_t)l1.next, (uintptr_t)0);
    EQ_SCALAR ((uintptr_t)l1.prev, (uintptr_t)0);

    EQ_SCALAR ((uintptr_t)l2.next, (uintptr_t)0);
    EQ_SCALAR ((uintptr_t)l2.prev, (uintptr_t)0);

    EQ_SCALAR ((uintptr_t)l3.next, (uintptr_t)0);
    EQ_SCALAR ((uintptr_t)l3.prev, (uintptr_t)0);

    END();
}

TEST (LIST, list_position_independence)
{
    ListNode head1 = { 0 };
    ListNode head2 = { 0 };
    struct Foo
    {
        ListNode list1_node;
        char*    val;
        ListNode list2_node;
    } foo = { .val = "HELLO" };

    list_init (&head1);
    list_init (&head2);
    list_init (&foo.list1_node);
    list_init (&foo.list2_node);

    list_add_before (&head1, &foo.list1_node);
    list_add_before (&head2, &foo.list2_node);

    struct Foo* f1 = LIST_ITEM (head1.next, struct Foo, list1_node);
    EQ_STRING (f1->val, "HELLO");

    struct Foo* f2 = LIST_ITEM (head2.next, struct Foo, list2_node);
    EQ_STRING (f2->val, "HELLO");

    END();
}

TEST (LIST, list_item_retrival)
{
#define COUNT 4

    char* input[] = { "ONE", "TWO", "THREE", "FOUR" };

    struct Foo
    {
        char*    val;
        ListNode list_node1; // Inserted using head1
        ListNode list_node2; // Inserted using head2
    } foo[COUNT] = { 0 };

    ListNode head1 = { 0 }; // Inserted using list_add_before
    ListNode head2 = { 0 }; // Inserted using list_add_after
    list_init (&head1);
    list_init (&head2);

    // Initialize foo array and add each Foo to list
    for (int i = 0; i < COUNT; i++)
    {
        foo[i].val = input[i];
        list_init (&foo[i].list_node1);
        list_init (&foo[i].list_node2);
        list_add_before (&head1, &foo[i].list_node1);
        list_add_after (&head2, &foo[i].list_node2);
    }

    // Traverse linked list
    char*     output1[COUNT] = { 0 };
    char*     output2[COUNT] = { 0 };
    ListNode* node           = NULL;
    int       i              = 0;

    // Traverse linked list (using head1)
    i    = 0;
    node = NULL;
    list_for_each (&head1, node)
    {
        struct Foo* f1 = LIST_ITEM (node, struct Foo, list_node1);
        output1[i++]   = f1->val;
    }

    // Traverse linked list (using head2)
    i    = 0;
    node = NULL;
    list_for_each (&head2, node)
    {
        struct Foo* f1 = LIST_ITEM (node, struct Foo, list_node2);
        output2[i++]   = f1->val;
    }

    // Output should match input
    for (int i = 0; i < COUNT; i++)
    {
        // in the order of insertion, because list_add_before was used
        EQ_STRING (output1[i], input[i]);
        // in the reverse order of insertion, because list_add_after was used
        EQ_STRING (output2[i], input[COUNT - 1 - i]);
    }

    END();
}

void yt_reset(void) {}

int main(void)
{
    YT_INIT();
    single_node();
    three_nodes();
    insert_after();
    remove_node_last();
    remove_node_middle();
    remove_node_first();
    remove_node_all();
    list_position_independence();
    list_item_retrival();
    is_empty();
    RETURN_WITH_REPORT();
}
