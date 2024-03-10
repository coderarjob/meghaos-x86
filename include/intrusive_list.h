#ifndef INTRUSIVE_LIST_H
#define INTRUSIVE_LIST_H

#include <stddef.h>
#include <stdint.h>

typedef struct ListNode
{
    struct ListNode* next;
    struct ListNode* prev;
} ListNode;

static inline void list_init (ListNode* node)
{
    node->next = node;
    node->prev = node;
}

static inline void list_add_before (ListNode* node, ListNode* item)
{
    item->next       = node;
    item->prev       = node->prev;
    node->prev->next = item;
    node->prev       = item;
}

static inline void list_add_after (ListNode* node, ListNode* item)
{
    item->prev       = node;
    item->next       = node->next;
    node->next->prev = item;
    node->next       = item;
}

static inline void list_remove (ListNode* item)
{
    item->prev->next = item->next;
    item->next->prev = item->prev;
    item->next       = NULL;
    item->prev       = NULL;
}

#define LIST_ITEM(node, type, member) (type*)((uintptr_t)node - offsetof (type, member))

#define list_for_each(head, node) \
    for ((node) = (head)->next; (node) != (head); (node) = (node)->next)

#endif // INTRUSIVE_LIST_H
