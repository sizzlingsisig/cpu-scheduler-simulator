# Design Notes: `schedsim` CPU Scheduling Simulator

---

## Phase 1: Foundation & Data Structures

The core of the simulator is the **Discrete-Event Loop**, using a time-stepped approach (`t = 0, 1, 2...`) to simulate the CPU clock.

### Process Control Block (PCB)

Defined in `include/process.h`, the `Process` struct tracks both static input data and dynamic simulation state.

| Category | Fields |
|---|---|
| **Static** | `pid`, `arrival_time`, `burst_time` |
| **Dynamic** | `remaining_time`, `start_time` (init to `-1`), `finish_time`, `priority`, `allotment_used` |

---

## Phase 2: Input Parsing & CLI Interface

Flags are handled via `getopt_long` for compatibility with shell invocation (e.g., `mysh`).

### Parsing Logic

**File input** — Use `fgets` + `sscanf` to skip comment lines (prefixed with `#`) and extract fields in `PID:AT:BT` format.

**Inline string input** — Use `strtok` with a `,` delimiter to split processes, then `sscanf` with a `:` delimiter to extract individual fields.

```bash
--processes="A:0:50,B:5:20"
```

---

## Phase 3: Non-Preemptive Algorithms (FCFS & SJF)

These are **run-to-completion** algorithms — once a process is selected, the simulation clock jumps forward by the process's full `burst_time`.

- **FCFS** — Sort the initial process array by `arrival_time`. Pick the next available process in order.
- **SJF** — Maintain a Ready Queue. At time `t`, move all arrived processes into the queue, sort by `burst_time`, and pick the shortest.

---

## Phase 4: Preemptive Algorithms (STCF & RR)

These algorithms evaluate system state at **every single time tick**.

### STCF — Shortest Time-to-Completion First

At every `t`, check if a newly arrived process has a `remaining_time` shorter than the currently running process. If so:

1. Increment `context_switches`.
2. Move the current process back to the Ready Queue.
3. Switch to the new process.

### Round Robin (RR)

Maintains a FIFO queue and rotates processes on quantum expiration.

- If a process runs for `quantum` units, move it to the back of the queue.
- **Edge case:** If a new process arrives at the exact same tick a quantum expires, the new arrival joins the queue *before* the preempted process.

---

## Phase 5: MLFQ — The Heuristic Scheduler

The most complex phase. MLFQ **observes** process behavior rather than relying on known burst times.

### The Five Rules

| Rule | Description |
|---|---|
| **Rule 1** | If `Priority(A) > Priority(B)`, A runs; B does not. |
| **Rule 2** | If `Priority(A) == Priority(B)`, A and B run Round Robin using the queue's `quantum`. |
| **Rule 3** | When a job enters the system, it is placed at the highest priority queue (`Q0`). |
| **Rule 4** | Once a job exhausts its allotment at a given level (regardless of CPU yields), its priority is reduced by one. |
| **Rule 5** | After a period `S`, all jobs in the system are moved to `Q0` (Priority Boost), preventing starvation. |

---

## Phase 6: Visualization & Reporting

Post-simulation logic handles metric calculation and Gantt chart rendering.

### Metrics Formulas

| Metric | Formula |
|---|---|
| Turnaround Time (TT) | `finish_time - arrival_time` |
| Waiting Time (WT) | `TT - burst_time` |
| Response Time (RT) | `start_time - arrival_time` |

### Gantt Chart Rendering

A `char` buffer is used where each index maps to one time unit. If process `A` ran from `t=0` to `t=5`, then `buffer[0..4] = 'A'`.

- **Scaling** — For long simulations, the renderer groups `N` time units per character block to prevent terminal line-wrapping.

---

## Phase 7: Verification & Testing

Checklist to ensure the simulator is lab-ready for CMSC 125.

### 1. Memory — Valgrind Check
Confirm all `malloc`'d queues and process arrays are properly `free`'d. Zero leaks, zero errors.

### 2. Correctness — Lecture Alignment
Run the **Quiz 3** workload. If FCFS average TT ≠ **515**, debug the arrival handling logic.

### 3. Integration — Shell Compatibility
Invoke `./schedsim` from within `mysh` to verify seamless `fork`/`exec` handling.