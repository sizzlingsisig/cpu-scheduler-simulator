#include "scheduler.h"
#include <stdlib.h>

void init_scheduler_state(SchedulerState *state, Process *procs, int num_procs) {
    state->processes = procs;
    state->num_processes = num_procs;
    state->current_time = 0;
    state->context_switches = 0;
    state->gantt_log = NULL; // Phase 6
    state->running_process = NULL;
    state->quantum = 0;
    state->policy_state = NULL;
}

void run_simulation(SchedulerState *state, SchedulerPolicy *policy) {
    int completed = 0;
    state->running_process = NULL;

    if (policy->on_init != NULL) {
        policy->on_init(state);
    }

    while (completed < state->num_processes) {
        // 1. Handle completion and preemption from previous tick
        if (state->running_process != NULL) {
            if (state->running_process->remaining_time == 0) {
                process_finish(state->running_process, state->current_time);
                completed++;
                state->running_process = NULL;
            }
        }

        if (completed == state->num_processes) break;

        Process *previous_running = state->running_process;

        // 2. Handle arrivals (New arrivals enter queue)
        for (int i = 0; i < state->num_processes; i++) {
            if (state->processes[i].state == STATE_NOT_ARRIVED && 
                state->processes[i].arrival_time <= state->current_time) {
                state->processes[i].state = STATE_READY;
                if (policy->on_arrival != NULL) {
                    policy->on_arrival(state, &state->processes[i]);
                }
            }
        }

        // 3. Handle clock tick for policies (e.g., RR quantum expiration)
        // If the policy preempts, it sets state->running_process to NULL
        if (state->running_process != NULL) {
            if (policy->on_tick != NULL) {
                policy->on_tick(state, &state->running_process);
                if (state->running_process == NULL) {
                    previous_running->state = STATE_READY;
                    state->context_switches++;
                }
            }
        }

        // 4. Dispatch next process if CPU is idle
        if (state->running_process == NULL) {
            if (policy->next_process != NULL) {
                state->running_process = policy->next_process(state);
            }
            if (state->running_process != NULL) {
                if (state->running_process != previous_running) {
                    process_start(state->running_process, state->current_time);
                } else {
                    state->running_process->state = STATE_RUNNING;
                }
            }
        }

        // 5. Execute for one tick
        if (state->running_process != NULL) {
            process_tick(state->running_process);
        }

        // 6. Advance time
        state->current_time++;
    }

    if (policy->on_finish != NULL) {
        policy->on_finish(state);
    }
}
