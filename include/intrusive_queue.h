#ifndef INTRU_QUEUE_H
#define INTRU_QUEUE_H
#include <intrusive_list.h>

static inline void enqueue (ListNode* head, ListNode* item)
{
    list_add_before (head, item);
}

static inline ListNode* dequeue (ListNode* head)
{
    if (head->next == head) {
        return NULL;
    }
    ListNode* node = head->next;
    list_remove (head->next);
    return node;
}

static inline void queue_remove (ListNode* node)
{
    list_remove (node);
}

static inline void enqueue_front (ListNode* head, ListNode* item)
{
    list_add_after (head, item);
}

static inline ListNode* dequeue_back (ListNode* head)
{
    if (head->prev == head) {
        return NULL;
    }

    ListNode* node = head->prev;
    list_remove (head->prev);
    return node;
}

#define queue_for_each(head, node) \
    for ((node) = (head)->next; (node) != (head); (node) = (node)->next)

#define queue_for_each_backward(head, node) \
    for ((node) = (head)->prev; (node) != (head); (node) = (node)->prev)

#endif // INTRU_QUEUE_H
