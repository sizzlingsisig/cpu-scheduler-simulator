#include "scheduler.h"
#include "queue.h"
#include <stdlib.h>

typedef struct {
    Queue ready_queue;
    int current_quantum;
} RRState;

static void rr_on_init(SchedulerState *state) {
    RRState *rr = malloc(sizeof(RRState));
    queue_init(&rr->ready_queue);
    rr->current_quantum = state->config.quantum;
    state->policy_state = rr;
}

static void rr_on_arrival(SchedulerState *state, Process *p) {
    RRState *rr = (RRState *)state->policy_state;
    enqueue(&rr->ready_queue, p);
}

static Process* rr_next_process(SchedulerState *state) {
    RRState *rr = (RRState *)state->policy_state;
    Process *next = dequeue(&rr->ready_queue);
    if (next != NULL) {
        rr->current_quantum = state->config.quantum;
    }
    return next;
}

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
