#include "policy.h"
#include <string.h>

// Phase 3 Schedulers
extern SchedulerPolicy FCFS_Policy;
extern SchedulerPolicy SJF_Policy;

// Phase 4 Schedulers
extern SchedulerPolicy STCF_Policy;
extern SchedulerPolicy RR_Policy;

/**
 * get_policy_by_name acts as the Strategy Factory for the simulator.
 * Centralizing the algorithm registry here prevents the main engine from 
 * being coupled to any specific implementation, making it easy to add 
 * new policies (like MLFQ) without modifying the orchestration logic.
 */
SchedulerPolicy* get_policy_by_name(const char *name) {
    if (strcmp(name, "FCFS") == 0) return &FCFS_Policy;
    if (strcmp(name, "SJF") == 0)  return &SJF_Policy;
    if (strcmp(name, "STCF") == 0) return &STCF_Policy;
    if (strcmp(name, "RR") == 0)    return &RR_Policy;
    return NULL;
}
