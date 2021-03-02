#include "queue.h"

/* Singly linked queue node */
typedef struct snode {
  // The node that comes after it
  struct snode *next;
  // The value the node contains
  void *val;
} snode;
/* Whole queue */
typedef struct queue {
  // The first element in the queue
  snode *head;
  // The last element in the queue
  snode *tail;
  // The number of elements in the queue
  unsigned long size;
} queue;

/* Initializes the queue. */
queue *queue_init() {
  queue *q = (queue *) malloc(sizeof(queue));
  if (q == NULL) {
    return NULL;
  }
  q->head = NULL;
  q->tail = NULL;
  q->size = 0;
  return q;
}

/* Returns the queue's size. */
long queue_size(queue *q) {
  return q->size;
}

/* Returns whether the queue is empty. */
bool queue_empty(queue *q) {
  return (q->size) == 0;
}

/* Returns the first element in
 * the queue without removing it. */
void *queue_peek(queue *q) {
  return q->head->val;
}

/* Inserts an element into the queue. */
bool queue_enqueue(queue *q, void *val) {
  snode *node = (snode *) malloc(sizeof(snode));
  if (node == NULL) {
    return false;
  }
  node->next = NULL;
  node->val = val;
  if (q->tail == NULL) {
    q->head = node;
    q->tail = node;
  } else {
    q->tail->next = node;
    q->tail = node;
  }
  q->size++;
  return true;
}

/* Removes an element from
 * the queue and returns it. */
void *queue_dequeue(queue *q) {
  snode *node = q->head;
  if (node == NULL) {
    return NULL;
  }
  void *val = node->val;
  q->head = node->next;
  if (node->next == NULL) {
    q->tail = NULL;
  }
  q->size--;
  free(node);
  return val;
}

/* Deallocates the queue
 * and its constituent elements. */
void queue_free(queue *q) {
  while (!queue_empty(q)) {
    queue_dequeue(q);
  }
  free(q);
}
