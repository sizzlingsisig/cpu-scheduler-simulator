// 
#include "process.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/**
 * Initializes a process struct. 
 * Metric fields are set to -1 or 0 to allow the reporting module to 
 * detect if a process never started or never finished (e.g. in case of 
 * simulation failure).
 */
void init_process(Process* process, const char* pid, int at, int bt) {
	strncpy(process->pid, pid, MAX_PID_LEN - 1);
	process->pid[MAX_PID_LEN - 1] = '\0';
	process->arrival_time = at;
	process->burst_time = bt;
	process->remaining_time = bt;
	process->start_time = -1;
	process->finish_time = 0;
	process->priority = 0;
	process->allotment_used = 0;
	process->turnaround_time = 0;
	process->waiting_time = 0;
	process->response_time = 0;
	process->state = STATE_NOT_ARRIVED;
}

// Count the number of processes encoded in an inline workload string.
// Each process contributes exactly 2 colons (PID:AT:BT), so total colons / 2 = process count.
/**
 * Pre-scanning the input to count colons allows us to allocate the exact 
 * amount of memory needed for the process array in one step, avoiding 
 * the complexity of dynamic resizing for CLI-based inputs.
 */
static int count_processes(const char* input) {
    int colons = 0;
    for (int i = 0; input[i]; i++) {
        if (input[i] == ':') colons++;
    }
    return colons / 2;
}

/**
 * String parsing relies on sscanf for tokens to strictly enforce the 
 * PID:AT:BT format while ignoring common delimiter issues.
 */
static int parse_process_token(const char* token, Process* out) {
    char pid[MAX_PID_LEN];
    int at, bt;
    if (sscanf(token, "%15[^:]:%d:%d", pid, &at, &bt) != 3) return 0;
    init_process(out, pid, at, bt);
    return 1;
}

/**
 * File parsing ignores comments (#) and empty lines to allow for 
 * documented workload files, as required by the lab guidelines.
 */
static int parse_process_line(const char* line, Process* out) {
    if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') return 0;
    char pid[MAX_PID_LEN];
    int at, bt;
    if (sscanf(line, "%15s %d %d", pid, &at, &bt) != 3) return 0;
    init_process(out, pid, at, bt);
    return 1;
}

// Parses an inline workload string like "P1:0:5,P2:2:3" into an array of Process structs.
// The caller is responsible for freeing the returned array.
/**
 * Parses an inline workload string into an array of Process structs.
 * This is primarily intended for short test scenarios and quick validation 
 * of scheduler behavior on staggered arrivals.
 */
Process* parse_workload_string(const char* input, int* count) {
    *count = count_processes(input);

    Process* procs = malloc(sizeof(Process) * (*count));
    char* data = strdup(input);

    int i = 0;
    char* token = strtok(data, ",");
    while (token != NULL && i < *count) {
        if (parse_process_token(token, &procs[i])) i++;
        token = strtok(NULL, ",");
    }

    free(data);
    return procs;
}

// Parses a workload file where each line is "PID AT BT" into an array of Process structs.
// The caller is responsible for freeing the returned array.
/**
 * Parses a workload file into an array of Process structs.
 * Using a dynamic array (with capacity doubling) allows us to handle 
 * arbitrarily large workloads while minimizing the number of 
 * realloc() calls during initialization.
 */
Process* parse_workload_file(const char* filename, int* count) {
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        perror("Error opening workload file");
        return NULL;
    }

    int capacity = 10;
    int n = 0;
    Process* procs = malloc(sizeof(Process) * capacity);
    char line[256];

    while (fgets(line, sizeof(line), fp)) {
        if (n >= capacity) {
            capacity *= 2;
            procs = realloc(procs, sizeof(Process) * capacity);
        }
        if (parse_process_line(line, &procs[n])) n++;
    }
    
    fclose(fp);
    *count = n;
    return procs;
}

/**
 * Capturing the response time only on the first execution event ensures 
 * that preemption does not skew the 'first contact' metric.
 */
void process_start(Process *p, int current_time) {
    if (p->start_time == -1) {
        p->start_time = current_time;
        p->response_time = p->start_time - p->arrival_time;
    }
    p->state = STATE_RUNNING;
}

/**
 * Atomic decrement of remaining time prevents timing drifts between 
 * the engine clock and individual process runtime.
 */
void process_tick(Process *p) {
    if (p->remaining_time > 0) {
        p->remaining_time--;
    }
}

/**
 * Wait time is computed here using the standard formula WT = Turnaround - Burst.
 * This implicitly accounts for any time spent in the READY state due to 
 * preemption or other processes running.
 */
void process_finish(Process *p, int current_time) {
    p->state = STATE_FINISHED;
    p->finish_time = current_time;
    p->turnaround_time = p->finish_time - p->arrival_time;
    p->waiting_time = p->turnaround_time - p->burst_time;
}

