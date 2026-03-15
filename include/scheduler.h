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
    
    // Algorithm-specific state (e.g., quantums, queues)
    void *policy_state;
} SchedulerState;

// Forward declaration of SchedulerPolicy
struct SchedulerPolicy;

// Scheduler algorithm function pointer types
typedef void (*OnInit)(SchedulerState *state);
typedef void (*OnArrival)(SchedulerState *state, Process *p);
typedef Process* (*NextProcess)(SchedulerState *state);
typedef void (*OnTick)(SchedulerState *state, Process **current);
typedef void (*OnFinish)(SchedulerState *state); // Cleanup policy state

typedef struct SchedulerPolicy {
    const char *name;
    OnInit on_init;
    OnArrival on_arrival;
    NextProcess next_process;
    OnTick on_tick;
    OnFinish on_finish;
} SchedulerPolicy;

// Initialize a generic scheduler state
void init_scheduler_state(SchedulerState *state, Process *procs, int num_procs);

// Runs the simulation until all processes are finished using the provided scheduler policy.
void run_simulation(SchedulerState *state, SchedulerPolicy *policy);

#endif