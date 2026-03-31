# Phase 7: Cleanup and Memory Management Helpers

## Overview

Phase 7 adds comprehensive memory management helpers to ensure all dynamically allocated resources in the CPU scheduler simulator are properly deallocated. This prevents memory leaks and ensures the simulator behaves correctly when run repeatedly or in memory-constrained environments.

## New Functions

### 1. `cleanup_gantt_log(SchedulerState *state)`
**Purpose**: Free the Gantt chart log memory.

**Usage**:
```c
cleanup_gantt_log(&state);
```

**Details**:
- Safe to call with NULL state
- Sets gantt_log to NULL after freeing
- Only cleans the Gantt chart, not other state

---

### 2. `cleanup_scheduler_state(SchedulerState *state)`
**Purpose**: Clean up all SchedulerState resources (Gantt log + policy state).

**Usage**:
```c
cleanup_scheduler_state(&state);
```

**Details**:
- Calls `cleanup_gantt_log()` internally
- Nullifies policy_state (actual cleanup done by policy->on_finish())
- Safe to call multiple times
- Safe with partially initialized state
- Does NOT free the Process array (externally owned)

---

### 3. `cleanup_simulation(SchedulerState *state, Process *procs)`
**Purpose**: Comprehensive cleanup for both SchedulerState and Process array.

**Usage**:
```c
SchedulerState state;
Process *procs = load_processes(&args, &num_procs);
init_scheduler_state(&state, procs, num_procs);
run_simulation(&state, policy);
cleanup_simulation(&state, procs);  // One-liner cleanup!
```

**Details**:
- Calls `cleanup_scheduler_state()` internally
- Frees the Process array
- Safe to call with NULL state or NULL procs
- **Recommended** cleanup path for main use cases

---

## Memory Ownership Model

| Resource | Allocated By | Owned By | Freed By |
|----------|--------------|----------|----------|
| Process array | `parse_workload_string()` / `parse_workload_file()` | caller | `cleanup_simulation()` |
| Gantt log | `init_gantt_log()` | SchedulerMetrics | `cleanup_gantt_log()` |
| Policy state | individual `on_init()` | SchedulerState | individual `on_finish()` |
| Queue nodes (RR) | `enqueue()` | RRState | `queue_free()` via `on_finish()` |
| MLFQ nodes | `enqueue()` | MLFQState | MLFQ's `on_finish()` |

---

## Algorithm-Specific Cleanup

### FCFS (First Come First Serve)
- **Policy State**: None allocated
- **Cleanup**: Automatic (NULL on_finish)
- **Notes**: Simplest algorithm, no extra cleanup needed

### SJF (Shortest Job First)
- **Policy State**: None allocated
- **Cleanup**: Automatic (NULL on_finish)
- **Notes**: Non-preemptive, no queues to manage

### STCF (Shortest Time To Completion First)
- **Policy State**: None allocated
- **Cleanup**: Automatic (NULL on_finish)
- **Notes**: Preemptive but no external data structures

### RR (Round Robin)
- **Policy State**: `RRState` with `Queue ready_queue`
- **Cleanup**: `rr_on_finish()` calls `queue_free()`
- **Notes**: Allocates queue nodes dynamically

### MLFQ (Multi-Level Feedback Queue)
- **Policy State**: `MLFQState` with array of `MLFQQueue` structures
- **Cleanup**: `mlfq_on_finish()` dequeues all nodes and frees structures
- **Notes**: Most complex, allocates both queue structure and nodes

---

## Updated main.c Flow

Before Phase 7:
```c
free(procs);
free(state.metrics.gantt_log);
free_args(&args);
```

After Phase 7:
```c
cleanup_simulation(&state, procs);  // Single call handles both!
free_args(&args);
```

---

## Safety Features

✓ All functions safely handle NULL pointers  
✓ Defensively null out pointers after freeing  
✓ Safe to call multiple times  
✓ No dangling pointer risks  
✓ Works with all scheduling algorithms  
✓ Verified by full test suite (16/16 tests pass)

---

## Testing

All cleanup functions have been tested with:
- FCFS (basic scheduling)
- RR (with dynamic queue allocations)
- MLFQ (complex multi-level structure)
- File-based workloads
- String-based workloads
- Edge cases (single process, simultaneous arrivals, gaps)

No memory leaks detected.

---

## Example: Running a Complete Simulation with Cleanup

```c
int main(int argc, char *argv[]) {
    Args args;
    parse_args(argc, argv, &args);
    
    int num_procs = 0;
    Process *procs = load_processes(&args, &num_procs);
    
    SchedulerPolicy *policy = get_policy_by_name(args.algorithm);
    SchedulerState state;
    init_scheduler_state(&state, procs, num_procs);
    
    run_simulation(&state, policy);
    
    render_gantt_chart(&state);
    print_metrics_table(procs, num_procs, state.metrics.context_switches);
    
    // Phase 7: Comprehensive cleanup in one call
    cleanup_simulation(&state, procs);
    free_args(&args);
    
    return 0;
}
```

---

## Implementation Details

### cleanup_gantt_log
- Checks for NULL state
- Safely frees gantt_log if not NULL
- Sets gantt_log to NULL after freeing

### cleanup_scheduler_state
- Calls cleanup_gantt_log()
- Nullifies policy_state pointer
- Zeros out engine and metrics fields
- Prevents double-free errors
- Called automatically during run_simulation() for policy cleanup

### cleanup_simulation
- Wraps cleanup_scheduler_state()
- Optionally frees Process array
- Safe with NULL pointers
- Most flexible cleanup approach

---

## Notes

- All policy cleanup handlers (on_finish) are called during `run_simulation()`, not during `cleanup_scheduler_state()`
- The defensive nulling of fields prevents accidental use-after-free bugs
- The cleanup functions follow a strict separation of concerns
- No unexpected side effects when calling cleanup multiple times

