#include <intrusive_list.h>
#include <intrusive_queue.h>
#include <unittest/unittest.h>

ListNode forward;
ListNode back;

TEST(queue, dequeue_empty_queue) {
  EQ_SCALAR(dequeue_forward(&forward, &back), NULL);
  EQ_SCALAR(dequeue_backward(&forward, &back), NULL);

  END();
}

TEST(queue, enqueue_backward_dequeue_forward_success) {
  ListNode node1, node2;
  list_init(&node1);
  list_init(&node2);

  enqueue_back(&back, &node1);
  enqueue_back(&back, &node2);

  EQ_SCALAR(dequeue_forward(&forward, &back), &node1);
  EQ_SCALAR(dequeue_forward(&forward, &back), &node2);

  END();
}

TEST(queue, enqueue_forward_dequeue_backward_success) {
  ListNode node1, node2;
  list_init(&node1);
  list_init(&node2);

  enqueue_forward(&forward, &node1);
  enqueue_forward(&forward, &node2);

  EQ_SCALAR(dequeue_backward(&back, &back), &node1);
  EQ_SCALAR(dequeue_backward(&back, &back), &node2);

  END();
}

TEST(queue, dequeue_forward_excess) {
  ListNode node1, node2;
  list_init(&node1);
  list_init(&node2);

  enqueue_back(&back, &node1);
  enqueue_back(&back, &node2);

  EQ_SCALAR(dequeue_forward(&forward, &back), &node1);
  EQ_SCALAR(dequeue_forward(&forward, &back), &node2);
  EQ_SCALAR(dequeue_forward(&forward, &back), NULL);
  EQ_SCALAR(dequeue_forward(&forward, &back), NULL);
  END();
}

TEST(queue, dequeue_backward_excess) {
  ListNode node1, node2;
  list_init(&node1);
  list_init(&node2);

  enqueue_forward(&forward, &node1);
  enqueue_forward(&forward, &node2);

  EQ_SCALAR(dequeue_backward(&forward, &back), &node1);
  EQ_SCALAR(dequeue_backward(&forward, &back), &node2);
  EQ_SCALAR(dequeue_backward(&forward, &back), NULL);
  EQ_SCALAR(dequeue_backward(&forward, &back), NULL);
  END();
}

TEST(queue, item_removal_success)
{
  ListNode node1, node2, node3;
  list_init(&node1);
  list_init(&node2);
  list_init(&node3);

  enqueue_forward(&forward, &node1);
  enqueue_forward(&forward, &node2);
  enqueue_forward(&forward, &node3);

  queue_remove(&node3);

  EQ_SCALAR(dequeue_backward(&forward, &back), &node1);
  EQ_SCALAR(dequeue_backward(&forward, &back), &node2);
  EQ_SCALAR(dequeue_backward(&forward, &back), NULL);
  END();
}

TEST(queue, enqueue_dequeue_object) {
  typedef struct Queue {
    int number;
    ListNode runlist;
  } Queue;

  Queue queue[3] = {0};

  // Initialize the items array and add it to the queue
  for (int i = 0; i < 3; i++) {
    queue[i].number = 10 + i;
    list_init(&queue[i].runlist);
    enqueue_back(&back, &queue[i].runlist);
  }

  // Dequeue one by one and check if the items match the order in which they
  // were added.
  ListNode *item = NULL;
  int i = 0;
  while ((item = dequeue_forward(&forward, &back)) != NULL) {
    Queue *q = (Queue *)LIST_ITEM(item, Queue, runlist);
    EQ_SCALAR(q, &queue[i++]);
  }

  END();
}

void reset() { queue_init(&forward, &back); }

int main() {

  dequeue_empty_queue();
  enqueue_backward_dequeue_forward_success();
  enqueue_forward_dequeue_backward_success();
  dequeue_forward_excess();
  dequeue_backward_excess();
  enqueue_dequeue_object();
  item_removal_success();

  return 0;
}
