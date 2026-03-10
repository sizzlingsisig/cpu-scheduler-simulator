// 
#include "../include/process.h"
#include <stdlib.h>
#include <string.h>

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

