#include "scheduler.h"
#include <string.h>

static Process* stcf_next_process(SchedulerState *state) {
    Process *next = NULL;
    for (int i = 0; i < state->num_processes; i++) {
        if (state->processes[i].state == STATE_READY) {
            if (next == NULL || state->processes[i].remaining_time < next->remaining_time) {
                next = &state->processes[i];
            } else if (state->processes[i].remaining_time == next->remaining_time) {
                if (state->processes[i].arrival_time < next->arrival_time) {
                    next = &state->processes[i];
                } else if (state->processes[i].arrival_time == next->arrival_time) {
                    if (strcmp(state->processes[i].pid, next->pid) < 0) {
                        next = &state->processes[i];
                    }
                }
            }
        }
    }
    return next;
}

static void stcf_on_arrival(SchedulerState *state, Process *p) {
    if (state->running_process != NULL) {
        if (p->remaining_time < state->running_process->remaining_time) {
            // Preempt the current process
            state->running_process = NULL;
        }
    }
}

SchedulerPolicy STCF_Policy = {
    .name = "STCF",
    .on_init = NULL,
    .on_arrival = stcf_on_arrival,
    .next_process = stcf_next_process,
    .on_tick = NULL,
    .on_finish = NULL
};
