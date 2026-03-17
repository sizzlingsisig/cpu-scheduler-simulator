#include "scheduler.h"
#include <stdlib.h>

// Helper to sort processes by arrival time
static int compare_arrival_time(const void *a, const void *b) {
    const Process *p1 = (const Process *)a;
    const Process *p2 = (const Process *)b;
    
    // Sort by arrival time primarily
    if (p1->arrival_time != p2->arrival_time) {
        return p1->arrival_time - p2->arrival_time;
    }
    // Tie-breaker: sort by PID lexicographically to ensure determinism
    // Assuming PID format is typically single letter or small strings
    return p1->pid[0] - p2->pid[0]; 
}

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
    state->metrics.gantt_log = NULL; // Phase 6

    state->policy_state = NULL;
}

void run_simulation(SchedulerState *state, SchedulerPolicy *policy) {
    int completed = 0;
    state->engine.running_process = NULL;

    if (policy->on_init != NULL) {
        policy->on_init(state);
    }

    while (completed < state->config.num_processes) {
        // 1. Handle completion and preemption from previous tick
        if (state->engine.running_process != NULL) {
            if (state->engine.running_process->remaining_time == 0) {
                process_finish(state->engine.running_process, state->engine.current_time);
                completed++;
                state->engine.running_process = NULL;
            }
        }

        if (completed == state->config.num_processes) break;

        // 2. Handle arrivals (New arrivals enter queue)
        while (state->engine.next_arrival_idx < state->config.num_processes) {
            Process *p = &state->config.processes[state->engine.next_arrival_idx];
            if (p->arrival_time <= state->engine.current_time && p->state == STATE_NOT_ARRIVED) {
                p->state = STATE_READY;
                if (policy->on_arrival != NULL) {
                    policy->on_arrival(state, p);
                }
                state->engine.next_arrival_idx++;
            } else {
                // Since processes are sorted by arrival time, if the next process
                // hasn't arrived, no subsequent process has arrived either.
                break;
            }
        }

        // 3. Handle clock tick for policies (e.g., RR quantum expiration)
        if (state->engine.running_process != NULL && policy->on_tick != NULL) {
            policy->on_tick(state, &state->engine.running_process);
        }

        // 4. Dispatch next process if CPU is idle or preemption was requested
        if (state->engine.running_process == NULL || state->engine.preempt_requested) {
            Process *previous = state->engine.running_process;
            
            // Temporarily mark as ready so policy can consider it
            if (previous != NULL) previous->state = STATE_READY;
            
            Process *next = NULL;
            if (policy->next_process != NULL) {
                next = policy->next_process(state);
            }

            if (next != previous) {
                if (previous != NULL) {
                    state->metrics.context_switches++;
                }
                state->engine.running_process = next;
                if (next != NULL) {
                    process_start(next, state->engine.current_time);
                }
            } else {
                // Keep running the same process
                state->engine.running_process = previous;
                if (previous != NULL) {
                    previous->state = STATE_RUNNING;
                }
            }
            state->engine.preempt_requested = 0;
        }

        // 5. Execute for one tick
        if (state->engine.running_process != NULL) {
            process_tick(state->engine.running_process);
        }

        // 6. Advance time
        state->engine.current_time++;
    }

    if (policy->on_finish != NULL) {
        policy->on_finish(state);
    }
}
