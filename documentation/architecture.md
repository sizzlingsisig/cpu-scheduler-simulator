# schedsim — CPU Scheduling Simulator

A discrete-event CPU scheduling simulator that ingests workload specifications, processes them through configurable scheduling algorithms, and outputs performance metrics alongside an ASCII Gantt chart.

---

## Table of Contents

- [Overview](#overview)
- [Architecture](#architecture)
- [Data Model](#data-model)
- [Simulation Engine](#simulation-engine)
- [Scheduling Algorithms](#scheduling-algorithms)
- [Usage](#usage)
- [Integration & Portability](#integration--portability)

---

## Overview

`schedsim` operates as a pipeline:

```
Workload Spec → CLI Parser → Discrete-Event Engine → Algorithm Module → Metrics & Gantt Chart
```

| Component | Responsibility |
|---|---|
| **CLI Parser** | Handles `--algorithm`, `--input`, `--quantum` and parses workload strings or files |
| **Discrete-Event Engine** | Manages the simulation clock and triggers arrival, preemption, and completion events |
| **Algorithm Modules** | Pluggable functions implementing FCFS, SJF, STCF, RR, and MLFQ |
| **Metrics & Visualization** | Computes TT, WT, RT and renders the execution timeline in ASCII |

---

## Architecture

The design separates **policy from mechanism**. Each scheduling algorithm is a self-contained module that interacts with a shared `SchedulerState` — it does not reach into global variables or cross-module state.

This allows the same workload to be passed to multiple algorithms simultaneously via `--compare` mode.

---

## Data Model

### `Process`

Tracks the full lifecycle of a task, distinguishing between static input and dynamic simulation state.

```c
typedef struct {
    char pid[16];
    int arrival_time;
    int burst_time;       // Static: Total CPU time needed
    int remaining_time;   // Dynamic: Decremented during execution
    int start_time;       // First time CPU was acquired (used for RT)
    int finish_time;      // Time of completion (used for TT)
    int priority;         // Current MLFQ queue level
    int time_in_queue;    // Allotment tracking for MLFQ
} Process;
```

### `SchedulerState`

Encapsulates all state for a single simulation run.

```c
typedef struct {
    Process *processes;
    int num_processes;
    int current_time;
    int context_switches;
    char *gantt_log;      // Buffer storing PID sequence for Gantt rendering
} SchedulerState;
```

---

## Simulation Engine

`schedsim` uses a **time-stepped discrete-event simulation**. The virtual clock advances one tick at a time; at each tick, the engine evaluates the ready queue and drives the simulation forward.

### Execution Loop

At each time step `t`:

1. **Arrival Handling** — Move all processes where `arrival_time == t` into the Ready Queue.
2. **Preemption Check** — For preemptive algorithms (STCF, RR, MLFQ), determine if the running process should be swapped out.
3. **Selection** — If the CPU is idle, invoke the algorithm's `select_next_process()`.
4. **Execution** — Decrement `remaining_time` for the active process and log its PID to the Gantt buffer.
5. **Completion** — If `remaining_time == 0`, set `finish_time = t + 1`.

---

## Scheduling Algorithms

| Algorithm | Type | Selection Logic |
|---|---|---|
| **FCFS** | Non-preemptive | Order of arrival (queue) |
| **SJF** | Non-preemptive | Shortest `burst_time` in Ready Queue |
| **STCF** | Preemptive | Shortest `remaining_time` at every tick |
| **RR** | Preemptive | Fixed quantum expiration via FIFO queue |
| **MLFQ** | Preemptive | Multi-priority queues with allotment and aging |

### MLFQ Details

The MLFQ module implements a heuristic-based scheduler with the following properties:

- Maintains an array of `MLFQQueue` structures at different priority levels.
- **Does not access `burst_time`** — behavior is discovered dynamically at runtime.
- Uses a `BOOST_PERIOD` to periodically promote all processes to the highest-priority queue, preventing starvation.

---

## Usage

```bash
# Run with a specific algorithm
schedsim --algorithm fcfs --input workload.txt

# Set a custom time quantum for Round Robin
schedsim --algorithm rr --quantum 4 --input workload.txt

# Compare all algorithms on the same workload
schedsim --compare --input workload.txt
```

**Workload format** (inline string or file):

```
PID  arrival_time  burst_time
P1   0             8
P2   2             4
P3   4             1
```

### Output

- Turnaround Time (TT), Waiting Time (WT), and Response Time (RT) per process and as averages.
- ASCII Gantt chart of the execution timeline.

---

## Integration & Portability

`schedsim` is designed for compatibility with Unix shell environments (including Lab 1 shell):

- **Fork/Exec compatible** — All state is self-contained; reads from `stdin` or files, writes to `stdout`.
- **Exit codes** — Returns `0` on success, non-zero on parsing or memory errors.
- **Memory management** — Follows a strict `init → simulate → cleanup` lifecycle to ensure no leaks during long-running `--compare` sessions.

---

## Metrics Reference

| Metric | Formula |
|---|---|
| Turnaround Time (TT) | `finish_time - arrival_time` |
| Waiting Time (WT) | `TT - burst_time` |
| Response Time (RT) | `start_time - arrival_time` |