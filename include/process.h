#ifndef PROCESS_H
#define PROCESS_H

#define MAX_PID_LEN 16

typedef enum {
    STATE_NOT_ARRIVED = 0,
    STATE_READY,
    STATE_RUNNING,
    STATE_FINISHED
} ProcessState;

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
    
    // Internal state
    ProcessState state;
} Process;

// Function prototypes
void init_process(Process* p, const char* pid, int at, int bt);
Process* parse_workload_string(const char* input, int* count);
Process* parse_workload_file(const char* filename, int* count);

// State transition helpers
void process_start(Process *p, int current_time);
void process_tick(Process *p);
void process_finish(Process *p, int current_time);

// TODO: Phase 6 - Metrics calculation helpers
// TODO: Phase 7 - Memory management docs
// TODO: Phase 5 - Extend struct for MLFQ if needed

#endif