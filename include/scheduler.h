// TODO: Phase 3 - Implement FCFS and SJF scheduling functions
// TODO: Phase 4 - Implement STCF and RR scheduling functions
// TODO: Phase 5 - Implement MLFQ scheduling functions
// TODO: Phase 6 - Add Gantt chart rendering helpers
// TODO: Phase 7 - Add cleanup and memory management helpers

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "process.h"

typedef struct {
    Process *processes;
    int num_processes;
    int current_time;
    int context_switches;
    char *gantt_log;
} SchedulerState;

// Scheduler algorithm function pointer type
typedef Process* (*SchedulerFunc)(SchedulerState *state);

// Initialize a generic scheduler state
void init_scheduler_state(SchedulerState *state, Process *procs, int num_procs);

// Phase 3 Schedulers
Process* fcfs_next_process(SchedulerState *state);
Process* sjf_next_process(SchedulerState *state);

#endif