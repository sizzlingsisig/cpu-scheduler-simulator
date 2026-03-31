#include "scheduler.h"
#include "gantt.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/**
 * Sorting by arrival time is the foundational optimization for the engine.
 * By ensuring the process array is ordered, we can handle arrivals in O(1) 
 * time per tick, which is essential for maintaining simulation performance 
 * on long workloads with thousands of processes.
 * 
 */
static int compare_arrival_time(const void *a, const void *b) {
    const Process *p1 = (const Process *)a;
    const Process *p2 = (const Process *)b;
    
    // Sort by arrival time primarily
    if (p1->arrival_time != p2->arrival_time) {
        return p1->arrival_time - p2->arrival_time;
    }

    return p1->pid[0] - p2->pid[0]; 
}

/**
 * Initializes the global scheduler context. 
 * By grouping the state into config, engine, and metrics, we enforce a 
 * clear separation between the simulation's initial conditions, its 
 * current runtime progression, and the resulting performance data.
 */
void init_scheduler_state(SchedulerState *state, Process *procs, int num_procs) {
    // Sort processes by arrival time initially
    qsort(procs, num_procs, sizeof(Process), compare_arrival_time);

    state->config.processes = procs;
    state->config.num_processes = num_procs;
    state->config.quantum = 0;

    state->engine.current_time = 0;
    state->engine.next_arrival_idx = 0;
    state->engine.running_process = NULL;
    state->engine.preempt_requested = 0;

    state->metrics.context_switches = 0;
    state->metrics.gantt_log = NULL; 

    state->policy_state = NULL;
    
    // Initialize Gantt chart log
    init_gantt_log(state);
}

// Simulation Step Helpers

/**
 * Completion checks must happen at the very start of a tick.
 * This ensures that if a process finished exactly at the current clock, 
 * its resources are freed immediately, allowing the dispatcher to 
 * utilize the CPU for the entire duration of the tick.
 */
static void handle_completions(SchedulerState *state, int *completed) {
    if (state->engine.running_process != NULL && state->engine.running_process->remaining_time == 0) {
        process_finish(state->engine.running_process, state->engine.current_time);
        (*completed)++;
        state->engine.running_process = NULL;
    }
}

/**
 * Batch-handling arrivals allows the engine to be time-accurate even 
 * when multiple processes arrive at the same discrete clock tick.
 * The arrival logic is decoupled from the algorithm policy to ensure 
 * that 'on_arrival' hooks can be used by algorithms like MLFQ to 
 * correctly place new processes in their initial queues.
 */
static void handle_arrivals(SchedulerState *state, SchedulerPolicy *policy) {
    while (state->engine.next_arrival_idx < state->config.num_processes) {
        Process *p = &state->config.processes[state->engine.next_arrival_idx];
        if (p->arrival_time <= state->engine.current_time && p->state == STATE_NOT_ARRIVED) {
            p->state = STATE_READY;
            if (policy->on_arrival != NULL) {
                policy->on_arrival(state, p);
            }
            state->engine.next_arrival_idx++;
        } else {
            break;
        }
    }
}

/**
 * The dispatch stage is the core of the Policy/Mechanism separation.
 * The engine (Mechanism) handles the context switch accounting and state 
 * transitions, while the policy (Algorithm) simply chooses which PID 
 * should run next.
 * 
 * Context switches are only counted when the running process actually 
 * changes, preventing false metrics in Round Robin scenarios where 
 * a process might preempt itself if no other work is available.
 */
static void handle_dispatch(SchedulerState *state, SchedulerPolicy *policy) {
    if (state->engine.running_process == NULL || state->engine.preempt_requested) {
        Process *previous = state->engine.running_process;
        if (previous != NULL) previous->state = STATE_READY;
        
        Process *next = (policy->next_process != NULL) ? policy->next_process(state) : NULL;

        if (next != previous) {
            if (previous != NULL) state->metrics.context_switches++;
            state->engine.running_process = next;
            if (next != NULL) process_start(next, state->engine.current_time);
        } else if (previous != NULL) {
            previous->state = STATE_RUNNING;
        }
        state->engine.preempt_requested = 0;
    }
}

/**
 * step_simulation implements the discrete event simulation timeline.
 * The order of stages is critical:
 * 1. Completion: Free the CPU.
 * 2. Arrival: New work enters the system.
 * 3. Tick: Policy evaluates current progress (e.g. quantum expiry).
 * 4. Dispatch: Decide what runs next. If process has BT=0, finish immediately.
 * 5. Execute: Consume one time unit.
 * 6. Completion: Catch processes that finished during this tick.
 * 7. Clock: Advance time.
 */
void step_simulation(SchedulerState *state, SchedulerPolicy *policy, int *completed) {
    // 1. Completion check
    handle_completions(state, completed);
    if (*completed == state->config.num_processes) return;

    // 2. Arrival check
    handle_arrivals(state, policy);

    // 3. Policy tick (preemption signal)
    if (state->engine.running_process != NULL && policy->on_tick != NULL) {
        policy->on_tick(state, &state->engine.running_process);
    }

    // 4. Dispatch check
    handle_dispatch(state, policy);

    // 4b. Handle BT=0 processes that complete immediately upon dispatch
    if (state->engine.running_process != NULL && 
        state->engine.running_process->remaining_time == 0) {
        process_finish(state->engine.running_process, state->engine.current_time);
        (*completed)++;
        state->engine.running_process = NULL;
        append_gantt_entry(state, "-");  // No Gantt entry for instant-complete
        state->engine.current_time++;
        return;
    }

    // 5. Execution
    if (state->engine.running_process != NULL) {
        process_tick(state->engine.running_process);
        append_gantt_entry(state, state->engine.running_process->pid);
    } else {
        append_gantt_entry(state, "-");
    }

    // 6. Clock Advance
    state->engine.current_time++;
}

/**
 * run_simulation is the master driver for the scheduling engine.
 * It remains agnostic to the specific scheduling policy being used, 
 * allowing the simulator to switch between FCFS, SJF, RR, and MLFQ 
 * simply by swapping the policy object.
 */
void run_simulation(SchedulerState *state, SchedulerPolicy *policy) {
    int completed = 0;
    state->engine.running_process = NULL;

    if (policy->on_init != NULL) {
        policy->on_init(state);
    }

    while (completed < state->config.num_processes) {
        step_simulation(state, policy, &completed);
    }

    if (policy->on_finish != NULL) {
        policy->on_finish(state);
    }
}

/**
 * Cleans up all resources owned by the SchedulerState.
 * This includes the Gantt chart log and any policy-specific state.
 * The Process array is NOT freed, as it is externally owned.
 * 
 * Safe to call multiple times or with partially initialized state.
 */
void cleanup_scheduler_state(SchedulerState *state) {
    if (state == NULL) return;
    
    // Clean up the Gantt chart log
    cleanup_gantt_log(state);
    
    // Note: Policy state cleanup is handled by policy->on_finish() 
    // during run_simulation(), so policy_state should already be NULL
    // by the time this function is called. However, we defensively
    // null it out in case the simulation was interrupted.
    state->policy_state = NULL;
    
    // Zero out the engine and metrics to prevent dangling pointers
    state->engine.running_process = NULL;
    state->metrics.context_switches = 0;
}

/**
 * Comprehensive cleanup function that handles both the SchedulerState
 * and an optional Process array. This is the recommended cleanup path
 * for typical use cases where the Process array is stack-allocated
 * or needs to be freed by the cleanup function.
 * 
 * If procs is NULL, only the SchedulerState is cleaned up.
 * Safe to call even if either parameter is NULL.
 */
void cleanup_simulation(SchedulerState *state, Process *procs) {
    if (state != NULL) {
        cleanup_scheduler_state(state);
    }
    
    if (procs != NULL) {
        free(procs);
    }
}
