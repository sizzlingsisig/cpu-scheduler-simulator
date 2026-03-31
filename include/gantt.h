#ifndef GANTT_H
#define GANTT_H

#include "scheduler.h"

// Initialize the Gantt chart log
void init_gantt_log(SchedulerState *state);

// Append an entry to the Gantt chart log
void append_gantt_entry(SchedulerState *state, const char *entry);

// Render the complete Gantt chart
void render_gantt_chart(SchedulerState *state);

// Clean up Gantt log memory
void cleanup_gantt_log(SchedulerState *state);

#endif // GANTT_H
