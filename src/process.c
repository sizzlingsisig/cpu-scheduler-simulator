// 
#include "process.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void init_process(Process* p, const char* pid, int at, int bt) {
	strncpy(p->pid, pid, MAX_PID_LEN - 1);
	p->pid[MAX_PID_LEN - 1] = '\0';
	p->arrival_time = at;
	p->burst_time = bt;
	p->remaining_time = bt;
	p->start_time = -1;
	p->finish_time = 0;
	p->priority = 0;
	p->allotment_used = 0;
	p->turnaround_time = 0;
	p->waiting_time = 0;
	p->response_time = 0;
}

Process* parse_workload_string(const char* input, int* count) {
    char* data = strdup(input);
    int n = 0;
    
    for (int i = 0; input[i]; i++) {
        if (input[i] == ':') n++;
    }
    *count = n / 2;

    Process* procs = malloc(sizeof(Process) * (*count));
    int i = 0;
    char* token = strtok(data, ",");
    
    while (token != NULL && i < *count) {
        char pid[MAX_PID_LEN];
        int at, bt;
        
        if (sscanf(token, "%15[^:]:%d:%d", pid, &at, &bt) == 3) {
            init_process(&procs[i], pid, at, bt);
            i++;
        }
        token = strtok(NULL, ",");
    }
    
    free(data);
    return procs;
}

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
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') continue;

        if (n >= capacity) {
            capacity *= 2;
            procs = realloc(procs, sizeof(Process) * capacity);
        }

        char pid[MAX_PID_LEN];
        int at, bt;
        
        if (sscanf(line, "%15s %d %d", pid, &at, &bt) == 3) {
            init_process(&procs[n], pid, at, bt);
            n++;
        }
    }
    
    fclose(fp);
    *count = n;
    return procs;
}

