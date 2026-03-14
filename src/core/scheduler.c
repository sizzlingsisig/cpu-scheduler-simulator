#include "scheduler.h"
#include <stdlib.h>

void init_scheduler_state(SchedulerState *state, Process *procs, int num_procs) {
    state->processes = procs;
    state->num_processes = num_procs;
    state->current_time = 0;
    state->context_switches = 0;
    state->gantt_log = NULL; // Phase 6
    state->policy_state = NULL;
}

void run_simulation(SchedulerState *state, SchedulerPolicy *policy) {
    int completed = 0;
    Process *current_running = NULL;

    if (policy->on_init != NULL) {
        policy->on_init(state);
    }

    while (completed < state->num_processes) {
        // 1. Handle arrivals
        for (int i = 0; i < state->num_processes; i++) {
            if (state->processes[i].state == STATE_NOT_ARRIVED && 
                state->processes[i].arrival_time <= state->current_time) {
                state->processes[i].state = STATE_READY;
                if (policy->on_arrival != NULL) {
                    policy->on_arrival(state, &state->processes[i]);
                }
            }
        }

        // 2. Process currently running task
        if (current_running != NULL) {
            process_tick(current_running);
            if (policy->on_tick != NULL) {
                policy->on_tick(state, &current_running); // Pass address so tick can preempt by setting to NULL
            }
            
            if (current_running != NULL && current_running->remaining_time == 0) {
                process_finish(current_running, state->current_time);
                completed++;
                current_running = NULL;
            }
        }

        // 3. Dispatch next process if CPU is idle
        // (Note: This is non-preemptive logic for Phase 3)
        while (current_running == NULL) {
            if (policy->next_process != NULL) {
                current_running = policy->next_process(state);
            }
            if (current_running != NULL) {
                process_start(current_running, state->current_time);
                
                // Edge case: process with 0 burst time finishes immediately
                if (current_running->remaining_time == 0) {
                    process_finish(current_running, state->current_time);
                    completed++;
                    current_running = NULL;
                } else {
                    break;
                }
            } else {
                break;
            }
        }

        // 4. Advance time
        if (completed < state->num_processes) {
            state->current_time++;
        }
    }

    if (policy->on_finish != NULL) {
        policy->on_finish(state);
    }
}
