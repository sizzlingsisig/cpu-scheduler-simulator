#include "queue.h"
#include <stdlib.h>

void queue_init(Queue *q) {
    q->head = NULL;
    q->tail = NULL;
    q->size = 0;
}

void enqueue(Queue *q, Process *p) {
    QueueNode *node = malloc(sizeof(QueueNode));
    node->process = p;
    node->next = NULL;
    
    if (q->tail == NULL) {
        q->head = node;
        q->tail = node;
    } else {
        q->tail->next = node;
        q->tail = node;
    }
    q->size++;
}

Process* dequeue(Queue *q) {
    if (q->head == NULL) return NULL;
    
    QueueNode *node = q->head;
    Process *p = node->process;
    q->head = node->next;
    
    if (q->head == NULL) {
        q->tail = NULL;
    }
    
    free(node);
    q->size--;
    return p;
}

int is_empty(Queue *q) {
    return q->size == 0;
}

void queue_free(Queue *q) {
    while (!is_empty(q)) {
        dequeue(q);
    }
}
