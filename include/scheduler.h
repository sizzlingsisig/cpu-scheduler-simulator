#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "process.h"

#define MAX_MLFQ_QUEUES 10

/**
 * MLFQConfig defines the custom behavior of the Multi-Level Feedback Queue.
 */
typedef struct {
    int num_queues;
    int quantums[MAX_MLFQ_QUEUES];
    int allotments[MAX_MLFQ_QUEUES];
    int boost_period;
} MLFQConfig;

/**
 * SchedulerConfig defines the static inputs of the simulation.
 * Separation from the runtime engine state allows for easier "compare mode"
 * by passing the same configuration into different scheduling policies.
 */
typedef struct {
    Process *processes;
    int num_processes;
    int quantum;
    MLFQConfig mlfq_config;
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

/**
 * Gantt chart rendering helpers
 * These functions build and display ASCII Gantt charts showing
 * the execution timeline of processes over time.
 */

/**
 * Initializes the Gantt chart log with an empty string.
 * Should be called during scheduler initialization.
 */
void init_gantt_log(SchedulerState *state);

/**
 * Appends a single time unit entry to the Gantt chart log.
 * Use the process PID if running, or '-' for idle CPU.
 */
void append_gantt_entry(SchedulerState *state, const char *entry);

/**
 * Renders the complete Gantt chart from the accumulated log.
 * Prints a formatted ASCII timeline showing process execution.
 */
void render_gantt_chart(SchedulerState *state);

/**
 * Cleanup and Memory Management Helpers (Phase 7)
 * 
 * The cleanup suite ensures proper deallocation of all resources allocated
 * during simulation. These functions follow a strict ownership model:
 * - The Process array is externally owned (allocated by parser)
 * - The gantt_log is owned by SchedulerMetrics
 * - The policy_state is owned by individual policy implementations
 */

/**
 * Cleans up the Gantt chart log memory.
 * Safe to call even if gantt_log is NULL.
 */
void cleanup_gantt_log(SchedulerState *state);

/**
 * Cleans up all resources owned by the SchedulerState, including
 * the Gantt chart log. Does NOT free the Process array, as it is
 * externally owned by the caller.
 * Safe to call multiple times or with partially initialized state.
 */
void cleanup_scheduler_state(SchedulerState *state);

/**
 * Comprehensive cleanup function that handles both the SchedulerState
 * and an optional Process array. This is the recommended cleanup path
 * for most use cases.
 * 
 * Usage: cleanup_simulation(state, procs) after run_simulation() completes.
 */
void cleanup_simulation(SchedulerState *state, Process *procs);

#endif // SCHEDULER_H