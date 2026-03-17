#ifndef METRICS_H
#define METRICS_H

#include "process.h"

/**
 * The metrics module is responsible for the post-simulation evaluation.
 * By decoupling data collection (in scheduler.c) from reporting (here), 
 * we allow the simulator to easily switch between different output formats 
 * (ASCII tables, CSV for analysis, or Gantt logs) without modifying the 
 * core engine.
 */
void print_metrics_table(Process *procs, int num_procs, int context_switches);

#endif // METRICS_H
