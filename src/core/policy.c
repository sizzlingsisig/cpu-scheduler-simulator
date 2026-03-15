#include "policy.h"
#include <string.h>

// Phase 3 Schedulers
extern SchedulerPolicy FCFS_Policy;
extern SchedulerPolicy SJF_Policy;

// Add new policies here as they are implemented
SchedulerPolicy* get_policy_by_name(const char *name) {
    if (strcmp(name, "FCFS") == 0) return &FCFS_Policy;
    if (strcmp(name, "SJF") == 0)  return &SJF_Policy;
    return NULL;
}
