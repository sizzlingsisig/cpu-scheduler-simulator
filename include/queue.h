#ifndef QUEUE_H
#define QUEUE_H

#include "process.h"

/**
 * QueueNode represents an entry in the ready queue.
 * Using a linked list structure allows the scheduler to handle an arbitrary
 * number of arriving processes without the overhead of array-shifting or
 * frequent reallocations that could jitter simulation timing.
 */
typedef struct QueueNode {
    Process *process;
    struct QueueNode *next;
} QueueNode;

/**
 * Queue maintains the FIFO state for scheduling algorithms.
 * Storing a tail pointer ensures that enqueue operations remain O(1),
 * preventing the scheduler's performance from degrading as the number
 * of ready processes grows.
 */
typedef struct {
    QueueNode *head;
    QueueNode *tail;
    int size;
} Queue;

/**
 * Initializes a queue to a safe empty state.
 */
void queue_init(Queue *q);

/**
 * Adds a process to the back of the queue.
 * This is the standard mechanism for FCFS and RR to maintain fairness
 * by respecting the order of arrival or preemption.
 */
void enqueue(Queue *q, Process *p);

/**
 * Removes and returns the process from the front of the queue.
 * Returns NULL if the queue is empty, allowing the scheduler to 
 * detect idle CPU windows.
 */
Process* dequeue(Queue *q);

/**
 * Checks if the queue has no processes, used to decide if the 
 * CPU should transition to an idle state.
 */
int is_empty(Queue *q);

/**
 * Frees the memory allocated for the queue's nodes.
 * This only destroys the queue structure; it does not free the 
 * underlying Process structs, as they are owned by the main simulation state.
 */
void queue_free(Queue *q);

#endif // QUEUE_H
