#include "scheduler.h"
#include <stdlib.h>
#include <string.h>

typedef struct {
    Process **queue;
    int head;
    int tail;
    int size;
    int capacity;
    int current_quantum;
} RRState;

static void rr_enqueue(RRState *rr, Process *p) {
    if (rr->size == rr->capacity) {
        rr->capacity *= 2;
        rr->queue = realloc(rr->queue, sizeof(Process*) * rr->capacity);
    }
    rr->queue[rr->tail] = p;
    rr->tail = (rr->tail + 1) % rr->capacity;
    rr->size++;
}

static Process* rr_dequeue(RRState *rr) {
    if (rr->size == 0) return NULL;
    Process *p = rr->queue[rr->head];
    rr->head = (rr->head + 1) % rr->capacity;
    rr->size--;
    return p;
}

static void rr_on_init(SchedulerState *state) {
    RRState *rr = malloc(sizeof(RRState));
    rr->capacity = state->config.num_processes + 1;
    rr->queue = malloc(sizeof(Process*) * rr->capacity);
    rr->head = 0;
    rr->tail = 0;
    rr->size = 0;
    rr->current_quantum = state->config.quantum;
    state->policy_state = rr;
}

static void rr_on_arrival(SchedulerState *state, Process *p) {
    RRState *rr = (RRState *)state->policy_state;
    rr_enqueue(rr, p);
}

static Process* rr_next_process(SchedulerState *state) {
    RRState *rr = (RRState *)state->policy_state;
    Process *next = rr_dequeue(rr);
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
        rr_enqueue(rr, *current);
    }
}

static void rr_on_finish(SchedulerState *state) {
    RRState *rr = (RRState *)state->policy_state;
    if (rr) {
        free(rr->queue);
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
