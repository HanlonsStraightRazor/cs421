#ifndef QUEUE_H
#define QUEUE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct queue queue;

/* Initializes the queue */
queue *queue_init();

/* Returns the queue's size */
long queue_size(queue *q);

/* Returns whether the queue is empty */
bool queue_empty(queue *q);

/* Returns the first element in
 * the queue without removing it */
void *queue_peek(queue *q);

/* Inserts an element into the queue */
bool queue_enqueue(queue *q, void *val);

/* Removes an element from
 * the queue and returns it */
void *queue_dequeue(queue *q);

/* Deallocates the queue
 * and its constituent elements */
void queue_free(queue *q);

#endif
