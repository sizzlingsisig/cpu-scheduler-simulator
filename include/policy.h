#ifndef POLICY_H
#define POLICY_H

#include "scheduler.h"

/**
 * The policy module serves as the primary extension point of the simulator.
 * Adding a new algorithm simply requires implementing the SchedulerPolicy 
 * hooks and registering the new policy by name in policy.c.
 */
SchedulerPolicy* get_policy_by_name(const char *name);

#endif // POLICY_H
