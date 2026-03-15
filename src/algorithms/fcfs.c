#include "scheduler.h"
#include <string.h>

static Process* fcfs_next_process(SchedulerState *state) {
    Process *next = NULL;
    for (int i = 0; i < state->num_processes; i++) {
        if (state->processes[i].state == STATE_READY) {
            if (next == NULL || state->processes[i].arrival_time < next->arrival_time) {
                next = &state->processes[i];
            }
        }
    }
    return next;
}

SchedulerPolicy FCFS_Policy = {
    .name = "FCFS",
    .on_init = NULL,
    .on_arrival = NULL,
    .next_process = fcfs_next_process,
    .on_tick = NULL,
    .on_finish = NULL
};
