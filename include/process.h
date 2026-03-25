#ifndef PROCESS_H
#define PROCESS_H

#define MAX_PID_LEN 16

/**
 * ProcessState identifies where a process is in its lifecycle.
 * Decoupling the simulation loop from raw boolean flags (like 'is_done') 
 * allows for easier debugging and more granular tracing of the 
 * process's transition from arrival to execution to termination.
 */
typedef enum {
    STATE_NOT_ARRIVED = 0,
    STATE_READY,
    STATE_RUNNING,
    STATE_FINISHED
} ProcessState;

/**
 * Process is the primary data model for the simulation.
 * It contains both the static inputs (arrival, burst) and the dynamic 
 * results (turnaround, response). 
 * 
 * Note: 'remaining_time' and 'allotment_used' are critical for 
 * preemptive and multi-level queue algorithms to track behavioral 
 * state without using "cheating" knowledge of the future.
 */
typedef struct {
    // Accepted inputs
    char pid[MAX_PID_LEN];
    int arrival_time;
    int burst_time;
    
    // Discrete event simulation state
    int remaining_time;
    int start_time;
    int finish_time;
    int priority;
    int allotment_used;
    
    // Final scheduling metrics
    int turnaround_time;
    int waiting_time;
    int response_time;
    
    // Internal state
    ProcessState state;
} Process;

/**
 * Standardizes the birth of a process by zeroing metrics and 
 * setting the initial arrival and burst requirements.
 */
void init_process(Process* p, const char* pid, int at, int bt);

/**
 * Utility to parse workloads directly from CLI strings (e.g. A:0:10,B:2:5).
 * Useful for quick validation of small edge cases without creating files.
 */
Process* parse_workload_string(const char* input, int* count);

/**
 * Utility to parse workloads from text files.
 * This is the primary method for running large, reproducible test suites.
 */
Process* parse_workload_file(const char* filename, int* count);

/**
 * Transitions a process to RUNNING. 
 * If it's the first time, it captures the 'start_time' to calculate 
 * Response Time (RT) accurately as (start - arrival).
 */
void process_start(Process *p, int current_time);

/**
 * Simulates the consumption of a single CPU time unit.
 * This is decoupled from the main scheduler engine to isolate 
 * individual process state from the global simulation clock.
 */
void process_tick(Process *p);

/**
 * Transitions a process to FINISHED and calculates final metrics.
 * Wait Time (WT) is derived as (Turnaround - Burst) to account for 
 * preemption-related gaps in execution.
 */
void process_finish(Process *p, int current_time);

/**
 * Metrics calculation helpers
 * These inline functions centralize the calculation of scheduling metrics
 * to ensure consistency and reusability across the codebase.
 */
static inline int calculate_turnaround_time(int finish_time, int arrival_time) {
    return finish_time - arrival_time;
}

static inline int calculate_waiting_time(int turnaround_time, int burst_time) {
    return turnaround_time - burst_time;
}

static inline int calculate_response_time(int start_time, int arrival_time) {
    return start_time - arrival_time;
}

// TODO: Phase 7 - Memory management docs
// TODO: Phase 5 - Extend struct for MLFQ if needed

#endif // PROCESS_H
