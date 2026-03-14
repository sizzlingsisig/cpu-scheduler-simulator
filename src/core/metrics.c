#include "metrics.h"
#include <stdio.h>
#include <stdlib.h>

void print_metrics_table(Process *procs, int num_procs) {
    if (num_procs == 0) return;

    printf("\n--- Scheduling Metrics ---\n");
    printf("%-5s %-5s %-5s %-5s %-5s %-5s %-5s\n", "PID", "AT", "BT", "FT", "TT", "WT", "RT");
    printf("--------------------------------------------\n");

    int total_tt = 0;
    int total_wt = 0;
    int total_rt = 0;

    for (int i = 0; i < num_procs; i++) {
        Process *p = &procs[i];
        printf("%-5s %-5d %-5d %-5d %-5d %-5d %-5d\n",
            p->pid, p->arrival_time, p->burst_time,
            p->finish_time, p->turnaround_time,
            p->waiting_time, p->response_time);
        
        total_tt += p->turnaround_time;
        total_wt += p->waiting_time;
        total_rt += p->response_time;
    }

    printf("--------------------------------------------\n");
    printf("Average Turnaround Time (TT): %.2f\n", (float)total_tt / num_procs);
    printf("Average Waiting Time (WT):    %.2f\n", (float)total_wt / num_procs);
    printf("Average Response Time (RT):   %.2f\n", (float)total_rt / num_procs);
    printf("--------------------------------------------\n");
}
