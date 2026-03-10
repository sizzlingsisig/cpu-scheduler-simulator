#ifndef PROCESS_H
#define PROCESS_H

#define MAX_PID_LEN 16

typedef struct {
    // Accepted inputs
    char pid[MAX_PID_LEN];
    int arrival_time;
    int burst_time;
    
    // Discrete event simulations
    int remaining_time;
    int start_time;
    int finish_time;
    int priority;
    int allotment_used;
    
    // Final scheduling metrics
    int turnaround_time;
    int waiting_time;
    int response_time;
} Process;

// Function prototypes
// Function prototypes
void init_process(Process* p, const char* pid, int at, int bt);

// TODO: Phase 2 - CLI parsing helpers
// TODO: Phase 6 - Metrics calculation helpers
// TODO: Phase 7 - Memory management docs
// TODO: Phase 5 - Extend struct for MLFQ if needed

#endif