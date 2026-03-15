#ifndef POLICY_H
#define POLICY_H

#include "scheduler.h"

// Returns the policy struct for the given name, or NULL if not found.
SchedulerPolicy* get_policy_by_name(const char *name);

#endif
