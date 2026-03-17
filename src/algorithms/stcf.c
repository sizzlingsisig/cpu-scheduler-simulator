#include "scheduler.h"
#include <string.h>

/**
 * STCF implements a greedy selection based on the shortest remaining time.
 * This rule minimizes the average wait time by always prioritizing the 
 * process that is closest to completion. 
 * 
 * The tie-breaker uses arrival_time first (for fairness) and then 
 * lexicographical PID to ensure deterministic simulation results.
 */
static Process* stcf_next_process(SchedulerState *state) {
    Process *next = NULL;
    for (int i = 0; i < state->config.num_processes; i++) {
        if (state->config.processes[i].state == STATE_READY) {
            if (next == NULL || state->config.processes[i].remaining_time < next->remaining_time) {
                next = &state->config.processes[i];
            } else if (state->config.processes[i].remaining_time == next->remaining_time) {
                if (state->config.processes[i].arrival_time < next->arrival_time) {
                    next = &state->config.processes[i];
                } else if (state->config.processes[i].arrival_time == next->arrival_time) {
                    if (strcmp(state->config.processes[i].pid, next->pid) < 0) {
                        next = &state->config.processes[i];
                    }
                }
            }
        }
    }
    return next;
}

/**
 * STCF provides preemption on every arrival event.
 * If a new process arrives with a shorter remaining time than the 
 * currently running process, the algorithm signals a preemption to 
 * ensure the CPU is always assigned to the globally shortest task.
 */
static void stcf_on_arrival(SchedulerState *state, Process *p) {
    if (state->engine.running_process != NULL) {
        if (p->remaining_time < state->engine.running_process->remaining_time) {
            // Signal preemption: engine will handle state transitions
            state->engine.preempt_requested = 1;
        }
    }
}

SchedulerPolicy STCF_Policy = {
    .name = "STCF",
    .on_init = NULL,
    .on_arrival = stcf_on_arrival,
    .next_process = stcf_next_process,
    .on_tick = NULL,
    .on_finish = NULL
};
