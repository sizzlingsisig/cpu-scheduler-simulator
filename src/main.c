

#include <stdio.h>
#include <stdlib.h>
#include "process.h"

// TODO: Phase 2 - Implement CLI parsing (getopt_long)
// TODO: Phase 2 - Workload parsing (file/string)
// TODO: Phase 3/4/5 - Simulation loop and algorithm selection
// TODO: Phase 6 - Metrics calculation and reporting
// TODO: Phase 7 - Memory management and cleanup

int main(int argc, char *argv[]) {
    // Phase 1: Demonstrate Process initialization
    Process p;
    init_process(&p, "A", 0, 10);
    printf("Initialized Process:\n");
    printf("  pid: %s\n", p.pid);
    printf("  arrival_time: %d\n", p.arrival_time);
    printf("  burst_time: %d\n", p.burst_time);
    printf("  remaining_time: %d\n", p.remaining_time);
    printf("  start_time: %d\n", p.start_time);
    printf("  finish_time: %d\n", p.finish_time);
    printf("  priority: %d\n", p.priority);
    printf("  allotment_used: %d\n", p.allotment_used);
    printf("  turnaround_time: %d\n", p.turnaround_time);
    printf("  waiting_time: %d\n", p.waiting_time);
    printf("  response_time: %d\n", p.response_time);
    return 0;
}