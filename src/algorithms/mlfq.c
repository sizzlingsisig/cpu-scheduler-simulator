#include "scheduler.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/**
 * MLFQ Queue Node
 */
typedef struct MLFQNode {
    Process *process;
    struct MLFQNode *next;
} MLFQNode;

/**
 * MLFQ Queue Structure
 * Implements a FIFO queue for Round Robin logic within a priority level.
 */
typedef struct {
    MLFQNode *head;
    MLFQNode *tail;
    int quantum;
    int allotment;
} MLFQQueue;

/**
 * MLFQ State Structure
 * Maintains the array of priority queues and the boost timer.
 */
typedef struct {
    MLFQQueue *queues;
    int num_queues;
    int boost_timer;
    int boost_period;
    int current_quantum;
} MLFQState;

// Queue Helpers
static void enqueue(MLFQQueue *q, Process *p) {
    MLFQNode *node = malloc(sizeof(MLFQNode));
    if (!node) {
        fprintf(stderr, "Memory allocation failed in enqueue\n");
        exit(1);
    }
    node->process = p;
    node->next = NULL;
    if (q->tail == NULL) {
        q->head = node;
        q->tail = node;
    } else {
        q->tail->next = node;
        q->tail = node;
    }
}

static Process* dequeue(MLFQQueue *q) {
    if (q->head == NULL) return NULL;
    MLFQNode *node = q->head;
    Process *p = node->process;
    q->head = node->next;
    if (q->head == NULL) {
        q->tail = NULL;
    }
    free(node);
    return p;
}

static void mlfq_on_init(SchedulerState *state) {
    MLFQState *mlfq = malloc(sizeof(MLFQState));
    if (!mlfq) {
        fprintf(stderr, "Memory allocation failed in mlfq_on_init\n");
        exit(1);
    }
    MLFQConfig *config = &state->config.mlfq_config;
    
    mlfq->num_queues = config->num_queues;
    mlfq->boost_period = config->boost_period;
    mlfq->boost_timer = 0;
    mlfq->current_quantum = 0;
    
    mlfq->queues = malloc(sizeof(MLFQQueue) * mlfq->num_queues);
    if (!mlfq->queues) {
        fprintf(stderr, "Memory allocation failed in mlfq_on_init\n");
        exit(1);
    }
    for (int i = 0; i < mlfq->num_queues; i++) {
        mlfq->queues[i].head = NULL;
        mlfq->queues[i].tail = NULL;
        mlfq->queues[i].quantum = config->quantums[i];
        mlfq->queues[i].allotment = config->allotments[i];
    }
    
    state->policy_state = mlfq;
}

static void mlfq_on_arrival(SchedulerState *state, Process *p) {
    MLFQState *mlfq = (MLFQState *)state->policy_state;
    // Rule 3: Start at highest priority (Q0)
    p->priority = 0;
    p->allotment_used = 0;
    enqueue(&mlfq->queues[0], p);
    
    // Check if we need to preempt the currently running process
    // because a higher priority process arrived.
    if (state->engine.running_process != NULL) {
        if (p->priority < state->engine.running_process->priority) {
            state->engine.preempt_requested = 1;
        }
    }
}

static Process* mlfq_next_process(SchedulerState *state) {
    MLFQState *mlfq = (MLFQState *)state->policy_state;
    
    for (int i = 0; i < mlfq->num_queues; i++) {
        Process *p = dequeue(&mlfq->queues[i]);
        if (p != NULL) {
            // Reset quantum for the time slice
            mlfq->current_quantum = mlfq->queues[i].quantum;
            return p;
        }
    }
    return NULL;
}

static void mlfq_on_tick(SchedulerState *state, Process **current) {
    MLFQState *mlfq = (MLFQState *)state->policy_state;
    
    // Rule 5: Priority Boost
    mlfq->boost_timer++;
    if (mlfq->boost_timer >= mlfq->boost_period) {
        // Move all processes from all queues to Q0
        for (int i = 1; i < mlfq->num_queues; i++) {
            Process *p;
            while ((p = dequeue(&mlfq->queues[i])) != NULL) {
                p->priority = 0;
                p->allotment_used = 0;
                enqueue(&mlfq->queues[0], p);
            }
        }
        
        // Reset allotment for Q0 too
        MLFQNode *curr_node = mlfq->queues[0].head;
        while (curr_node) {
            curr_node->process->allotment_used = 0;
            curr_node = curr_node->next;
        }
        
        mlfq->boost_timer = 0;
        
        // Preempt the current process so we can re-evaluate priorities
        if (*current != NULL) {
            (*current)->priority = 0;
            (*current)->allotment_used = 0;
            state->engine.preempt_requested = 1;
            enqueue(&mlfq->queues[0], *current);
            return; // We preempted, so stop evaluating this tick for demotion
        }
    }
    
    if (*current == NULL) return;
    
    Process *p = *current;
    int q_index = p->priority;
    
    // We consumed 1 tick of CPU
    mlfq->current_quantum--;
    p->allotment_used++;
    
    // Rule 4: Demotion if allotment exhausted
    if (p->allotment_used >= mlfq->queues[q_index].allotment) {
        state->engine.preempt_requested = 1;
        // Demote if not at lowest queue
        if (p->priority < mlfq->num_queues - 1) {
            p->priority++;
        }
        p->allotment_used = 0; // Reset allotment for the new queue
        enqueue(&mlfq->queues[p->priority], p);
    } 
    // Rule 2: Round Robin quantum expired but allotment not exhausted
    else if (mlfq->current_quantum <= 0) {
        state->engine.preempt_requested = 1;
        enqueue(&mlfq->queues[p->priority], p);
    }
    // Preempted by arrival (higher priority came in)
    else if (state->engine.preempt_requested) {
        enqueue(&mlfq->queues[p->priority], p);
    }
}

static void mlfq_on_finish(SchedulerState *state) {
    MLFQState *mlfq = (MLFQState *)state->policy_state;
    if (mlfq) {
        for (int i = 0; i < mlfq->num_queues; i++) {
            Process *p;
            while ((p = dequeue(&mlfq->queues[i])) != NULL) {
                // Free node done by dequeue
            }
        }
        free(mlfq->queues);
        free(mlfq);
        state->policy_state = NULL;
    }
}

SchedulerPolicy MLFQ_Policy = {
    .name = "mlfq",
    .on_init = mlfq_on_init,
    .on_arrival = mlfq_on_arrival,
    .next_process = mlfq_next_process,
    .on_tick = mlfq_on_tick,
    .on_finish = mlfq_on_finish
};
