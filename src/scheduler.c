#include "scheduler.h"
#include <stdlib.h>

void init_scheduler_state(SchedulerState *state, Process *procs, int num_procs) {
    state->processes = procs;
    state->num_processes = num_procs;
    state->current_time = 0;
    state->context_switches = 0;
    state->gantt_log = NULL; // Phase 6
}
