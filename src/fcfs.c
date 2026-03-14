#include "scheduler.h"
#include <string.h>

// FCFS selects the process in the ready state with the earliest arrival time.
// Since processes are usually added in some order, we just pick the ready process
// with the smallest arrival time. If there's a tie, we pick the one that appeared first (lowest array index).
Process* fcfs_next_process(SchedulerState *state) {
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
