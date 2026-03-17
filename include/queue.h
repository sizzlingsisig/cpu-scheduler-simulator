#ifndef QUEUE_H
#define QUEUE_H

#include "process.h"

typedef struct QueueNode {
    Process *process;
    struct QueueNode *next;
} QueueNode;

typedef struct {
    QueueNode *head;
    QueueNode *tail;
    int size;
} Queue;

void queue_init(Queue *q);
void enqueue(Queue *q, Process *p);
Process* dequeue(Queue *q);
int is_empty(Queue *q);
void queue_free(Queue *q);

#endif // QUEUE_H
