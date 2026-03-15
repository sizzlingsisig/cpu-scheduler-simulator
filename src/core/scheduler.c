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
    state->running_process = NULL;

    if (policy->on_init != NULL) {
        policy->on_init(state);
    }

    while (completed < state->num_processes) {
        Process *previous_running = state->running_process;

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

        // 2. Check for preemption from on_arrival
        if (previous_running != NULL && state->running_process == NULL && previous_running->state == STATE_RUNNING) {
            previous_running->state = STATE_READY;
            state->context_switches++;
        }

        // 3. Dispatch next process if CPU is idle
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

        // 4. Execute for one tick
        if (state->running_process != NULL) {
            process_tick(state->running_process);
            
            if (policy->on_tick != NULL) {
                policy->on_tick(state, &state->running_process);
                // If on_tick preempted by setting state->running_process = NULL
                if (state->running_process == NULL && previous_running != NULL && previous_running->remaining_time > 0) {
                    previous_running->state = STATE_READY;
                    state->context_switches++;
                }
            }

            if (state->running_process != NULL && state->running_process->remaining_time == 0) {
                process_finish(state->running_process, state->current_time + 1);
                completed++;
                state->running_process = NULL;
            }
        }

        // 5. Advance time
        if (completed < state->num_processes) {
            state->current_time++;
        }
    }

    if (policy->on_finish != NULL) {
        policy->on_finish(state);
    }
}
