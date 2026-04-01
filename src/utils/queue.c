#include "queue.h"
#include <stdlib.h>
#include <stdio.h>

/**
 * Ensures the queue starts in a predictable, empty state to prevent 
 * dereferencing uninitialized pointers during the simulation's first tick.
 */
void queue_init(Queue *q) {
    q->head = NULL;
    q->tail = NULL;
    q->size = 0;
}

/**
 * Nodes are dynamically allocated because the number of ready processes
 * is unpredictable and varies based on the workload and preemption logic.
 * By updating the tail pointer, we keep enqueue operations at O(1) time.
 */
void enqueue(Queue *q, Process *p) {
    QueueNode *node = malloc(sizeof(QueueNode));
    if (!node) {
        fprintf(stderr, "Memory allocation failed in enqueue\n");
        exit(1);
    }
    node->process = p;
    node->next = NULL;
    
    // If the queue was empty, the new node becomes both head and tail.
    if (q->tail == NULL) {
        q->head = node;
        q->tail = node;
    } else {
        q->tail->next = node;
        q->tail = node;
    }
    q->size++;
}

/**
 * Only the node's memory is reclaimed here, as the Process struct is 
 * shared and persists in the main simulation's process array. 
 * If the queue becomes empty, we nullify the tail to reset the enqueue logic.
 */
Process* dequeue(Queue *q) {
    if (q->head == NULL) return NULL;
    
    QueueNode *node = q->head;
    Process *p = node->process;
    q->head = node->next;
    
    // Ensure the tail pointer doesn't dangle if we removed the last item.
    if (q->head == NULL) {
        q->tail = NULL;
    }
    
    free(node);
    q->size--;
    return p;
}

/**
 * Returns a boolean-style integer to simplify control flow in algorithms 
 * that need to check if they have any work left to do.
 */
int is_empty(Queue *q) {
    return q->size == 0;
}

/**
 * Reclaims all node memory at the end of a simulation run or algorithm 
 * switch to prevent memory leaks while keeping the process data intact.
 */
void queue_free(Queue *q) {
    while (!is_empty(q)) {
        dequeue(q);
    }
}
