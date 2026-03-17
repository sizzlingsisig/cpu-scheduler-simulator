#include "scheduler.h"
#include <string.h>

static Process* sjf_next_process(SchedulerState *state) {
    Process *next = NULL;
    for (int i = 0; i < state->config.num_processes; i++) {
        if (state->config.processes[i].state == STATE_READY) {
            if (next == NULL) {
                next = &state->config.processes[i];
            } else if (state->config.processes[i].burst_time < next->burst_time) {
                next = &state->config.processes[i];
            } else if (state->config.processes[i].burst_time == next->burst_time) {
                if (state->config.processes[i].arrival_time < next->arrival_time) {
                    next = &state->config.processes[i];
                } else if (state->config.processes[i].arrival_time == next->arrival_time) {
                    if (strcmp(state->config.processes[i].pid, next->pid) < 0) {
                        next = &state->config.processes[i];
                    }
                }
            }
        }
    }
    return next;
}

SchedulerPolicy SJF_Policy = {
    .name = "SJF",
    .on_init = NULL,
    .on_arrival = NULL,
    .next_process = sjf_next_process,
    .on_tick = NULL,
    .on_finish = NULL
};
