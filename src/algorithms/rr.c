#include "scheduler.h"
#include "queue.h"
#include <stdlib.h>

/**
 * RRState maintains the algorithm's specific execution context.
 * This is decoupled from the main SchedulerState to allow the core 
 * engine to remain agnostic to algorithm-specific data structures 
 * like ready queues or quantum counters.
 */
typedef struct {
    Queue ready_queue;
    int current_quantum;
} RRState;

/**
 * on_init creates the private state for the RR policy.
 * This pattern ensures that memory is only allocated for algorithms 
 * that are actually selected by the user.
 */
static void rr_on_init(SchedulerState *state) {
    RRState *rr = malloc(sizeof(RRState));
    queue_init(&rr->ready_queue);
    rr->current_quantum = state->config.quantum;
    state->policy_state = rr;
}

/**
 * New arrivals are simply placed at the back of the ready queue, 
 * maintaining the FIFO fairness requirement of the Round Robin policy.
 */
static void rr_on_arrival(SchedulerState *state, Process *p) {
    RRState *rr = (RRState *)state->policy_state;
    enqueue(&rr->ready_queue, p);
}

/**
 * rr_next_process implements the 'next in line' dispatch rule.
 * By resetting the current_quantum here, we ensure that every process 
 * starts its time slice with a full allocation from the simulation config.
 */
static Process* rr_next_process(SchedulerState *state) {
    RRState *rr = (RRState *)state->policy_state;
    Process *next = dequeue(&rr->ready_queue);
    if (next != NULL) {
        rr->current_quantum = state->config.quantum;
    }
    return next;
}

/**
 * on_tick handles the expiration of a time slice.
 * By setting the preempt_requested flag rather than modifying the 
 * process state directly, the algorithm signals its intent to the 
 * core engine, which then handles the context switch accounting.
 */
static void rr_on_tick(SchedulerState *state, Process **current) {
    RRState *rr = (RRState *)state->policy_state;
    if (*current == NULL) return;

    rr->current_quantum--;
    if (rr->current_quantum <= 0 && (*current)->remaining_time > 0) {
        // Signal preemption: engine will handle state transitions
        state->engine.preempt_requested = 1;
        enqueue(&rr->ready_queue, *current);
    }
}

static void rr_on_finish(SchedulerState *state) {
    RRState *rr = (RRState *)state->policy_state;
    if (rr) {
        queue_free(&rr->ready_queue);
        free(rr);
        state->policy_state = NULL;
    }
}

SchedulerPolicy RR_Policy = {
    .name = "RR",
    .on_init = rr_on_init,
    .on_arrival = rr_on_arrival,
    .next_process = rr_next_process,
    .on_tick = rr_on_tick,
    .on_finish = rr_on_finish
};
