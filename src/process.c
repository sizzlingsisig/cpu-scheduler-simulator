// 
#include "process.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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
static int count_processes(const char* input) {
    int colons = 0;
    for (int i = 0; input[i]; i++) {
        if (input[i] == ':') colons++;
    }
    return colons / 2;
}

// Parse a single "PID:AT:BT" token (inline string format).
// Returns 1 on success, 0 if malformed.
static int parse_process_token(const char* token, Process* out) {
    char pid[MAX_PID_LEN];
    int at, bt;
    if (sscanf(token, "%15[^:]:%d:%d", pid, &at, &bt) != 3) return 0;
    init_process(out, pid, at, bt);
    return 1;
}

// Parse a single line from the workload file, which should be "PID AT BT".
// Returns 1 on success, 0 if malformed or comment/empty line.
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

