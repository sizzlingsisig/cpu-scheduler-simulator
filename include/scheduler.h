// TODO: Phase 3 - Implement FCFS and SJF scheduling functions
// TODO: Phase 4 - Implement STCF and RR scheduling functions
// TODO: Phase 5 - Implement MLFQ scheduling functions
// TODO: Phase 6 - Add Gantt chart rendering helpers
// TODO: Phase 7 - Add cleanup and memory management helpers

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "process.h"

/**
 * SchedulerConfig defines the static inputs of the simulation.
 * Separation from the runtime engine state allows for easier "compare mode"
 * by passing the same configuration into different scheduling policies.
 */
typedef struct {
    Process *processes;
    int num_processes;
    int quantum;
} SchedulerConfig;

/**
 * SchedulerEngine tracks the active progression of the simulation.
 * These fields change every discrete tick and represent the "now"
 * state of the CPU and the arriving process stream.
 */
typedef struct {
    int current_time;
    int next_arrival_idx;
    Process *running_process;
    int preempt_requested;
} SchedulerEngine;

/**
 * SchedulerMetrics collects data points required for the final evaluation.
 * Decoupling metrics from the engine logic allows for modular reporting
 * and the future addition of complex data like Gantt charts.
 */
typedef struct {
    int context_switches;
    char *gantt_log;
} SchedulerMetrics;

/**
 * SchedulerState is the root context object passed between core and algorithms.
 * Using a nested structure keeps the object addressable while preventing
 * a "flat list" of variables from becoming cognitively overwhelming.
 */
typedef struct {
    SchedulerConfig config;
    SchedulerEngine engine;
    SchedulerMetrics metrics;
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

/**
 * SchedulerPolicy provides the abstract interface for all scheduling algorithms.
 * By defining specific hooks (on_init, on_arrival, etc.), we enforce a strict
 * Policy/Mechanism separation: the core engine manages the clock, while
 * the policy only makes scheduling decisions.
 */
typedef struct SchedulerPolicy {
    const char *name;
    OnInit on_init;
    OnArrival on_arrival;
    NextProcess next_process;
    OnTick on_tick;
    OnFinish on_finish;
} SchedulerPolicy;

/**
 * Prepares the simulation context by sorting processes and zeroing engine state.
 */
void init_scheduler_state(SchedulerState *state, Process *procs, int num_procs);

/**
 * Performs a single discrete time step in the simulation.
 * This function orchestrates the sequence of completion, arrival, policy
 * decision, and execution stages.
 */
void step_simulation(SchedulerState *state, SchedulerPolicy *policy, int *completed);

/**
 * The master simulation loop that drives the discrete time clock until
 * all processes reach their termination state.
 */
void run_simulation(SchedulerState *state, SchedulerPolicy *policy);

#endif // SCHEDULER_H