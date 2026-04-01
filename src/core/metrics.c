#include "metrics.h"
#include <stdio.h>
#include <stdlib.h>

void calculate_metrics(Process *processes, int num_procs) {
    for (int i = 0; i < num_procs; i++) {
        processes[i].turnaround_time = processes[i].finish_time - processes[i].arrival_time;
        processes[i].waiting_time = processes[i].turnaround_time - processes[i].burst_time;
        processes[i].response_time = processes[i].start_time - processes[i].arrival_time;
    }
}

void print_metrics_table(Process *procs, int num_procs) {
    if (num_procs == 0) return;

    printf("\n=== Metrics ===\n");
    printf("Process | AT  | BT  | FT  | TT  | WT  | RT  \n");
    printf("--------|-----|-----|-----|-----|-----|-----\n");

    int total_tt = 0;
    int total_wt = 0;
    int total_rt = 0;

    for (int i = 0; i < num_procs; i++) {
        Process *p = &procs[i];
        printf("%-7s | %3d | %3d | %3d | %3d | %3d | %3d \n",
            p->pid, p->arrival_time, p->burst_time,
            p->finish_time, p->turnaround_time,
            p->waiting_time, p->response_time);
        
        total_tt += p->turnaround_time;
        total_wt += p->waiting_time;
        total_rt += p->response_time;
    }

    printf("--------|-----|-----|-----|-----|-----|-----\n");
    int avg_tt = total_tt / num_procs;
    int avg_wt = total_wt / num_procs;
    int avg_rt = total_rt / num_procs;
    printf("Average |     |     |     | %3d | %3d | %3d \n", avg_tt, avg_wt, avg_rt);

    for (int i = 0; i < num_procs; i++) {
        Process *p = &procs[i];
        // Identify convoy effect (long job blocking a short job)
        if (p->waiting_time > 100 && p->waiting_time > p->burst_time) {
            fprintf(stderr, "\nConvoy effect detected: Process %s waited %d time units\n", p->pid, p->waiting_time);
            break;
        }
    }
}
