
# Design Notes: `schedsim` CPU Scheduling Simulator

---

## Implementation Notes

The most reliable way to build this simulator is using a time-stepped discrete-event loop rather than jumping between chronological events. By advancing a global clock integer (t) by exactly one unit per iteration, you naturally handle edge cases where a process arrives at the exact same millisecond a time quantum expires.

Memory management needs to be a priority from the beginning. Because the simulator will eventually run all five algorithms sequentially in comparison mode, any leaked queues or process arrays will compound and cause failures. Every structure you allocate during an algorithm's execution must be meticulously freed before that specific function returns control to the main loop.

For the MLFQ implementation, the core constraint is absolute blindness to the burst_time variable. The logic must be strictly heuristic. You will need to track the consecutive time a process spends in a specific queue layer. Once that tracked time exceeds the queue's allotment, the process is demoted, and its tracking integer is reset.

---

## Step-by-Step Execution Plan

1. **Establish the Build Pipeline and State Structures**
	- Populate your Makefile to ensure your compiler correctly links the src and include directories.
	- Define your core Process and SchedulerState structures in the header files. This gives you a compiling, executable target, even if it does nothing yet.

2. **Implement the Parser and CLI Interface**
	- Write the string parsing and file reading logic using strtok and sscanf.
	- Wire up getopt_long in your main entry point to handle the flags.
	- Test this phase by printing the parsed arrival and burst times to the terminal to guarantee your simulator is ingesting the workload correctly.

3. **Build the Time-Stepped Engine and Non-Preemptive Policies**
	- Draft the main simulation loop that increments your global time variable.
	- Implement First-Come First-Serve (FCFS) and Shortest Job First (SJF).
	- Because these algorithms run to completion, you only need to worry about sorting the ready queue and advancing the clock by the total burst time of the selected process.
	- Validate your FCFS turnaround time against the Quiz 3 lecture expected value (515).

4. **Introduce Preemption and Time Slicing**
	- Implement Shortest Time-to-Completion First (STCF) and Round Robin (RR).
	- This requires modifying your engine to evaluate the ready queue at every single clock tick.
	- You will add logic to interrupt the active process, increment a context switch counter, and cycle processes to the back of the queue when their time quantum expires.

5. **Construct the Multi-Level Feedback Queue**
	- Build the MLFQ logic using an array of priority queues.
	- Implement the allotment demotion mechanism and a global priority boost timer that returns all active processes to the top tier to prevent starvation.

6. **Format Output and Verify Shell Integration**
	- Write the post-simulation logic to compute turnaround, waiting, and response times.
	- Render the ASCII Gantt chart using a character buffer.
	- Finally, compile the binary and execute it from within your custom Unix shell using fork and execvp to ensure standard input/output behaves exactly as expected by the operating system.

---

## Phase 1: Foundation & Data Structures

The core of the simulator is the **Discrete-Event Loop**, using a time-stepped approach (`t = 0, 1, 2...`) to simulate the CPU clock.

### Process Control Block (PCB)

Defined in `include/process.h`, the `Process` struct tracks static input data, dynamic simulation state, and final computed metrics.

| Category | Fields |
|---|---|
| **Static** | `pid`, `arrival_time`, `burst_time` |
| **Dynamic** | `remaining_time`, `start_time` (init to `-1`), `finish_time`, `priority`, `allotment_used` |
| **Final Metrics** | `turnaround_time`, `waiting_time`, `response_time` |

Storing final metrics directly on the `Process` struct keeps the data co-located and simplifies both the `--compare` mode (no separate results buffer) and Gantt-chart annotation.

---

## Phase 2: Input Parsing & CLI Interface

Flags are handled via `getopt_long` for compatibility with shell invocation (e.g., `mysh`).

### Supported Flags

| Flag | Argument | Description |
|---|---|---|
| `--algorithm` / `-a` | `FCFS` \| `SJF` \| `STCF` \| `RR` \| `MLFQ` | Selects the scheduling algorithm |
| `--processes` / `-p` | `"PID:AT:BT,..."` | Inline workload string |
| `--input` / `-i` | `<file>` | Path to a workload file |
| `--quantum` / `-q` | `<int>` | Time-slice length (required for RR and MLFQ) |
| `--compare` | — | Runs all algorithms on the workload and outputs a side-by-side summary |

### Parsing Logic

**File input** — Use `fgets` + `sscanf` to skip comment lines (prefixed with `#`) and extract space-separated fields in `PID AT BT` format.

```
# workload.txt
A 0 50
B 5 20
```

**Inline string input** — Use `strtok` with a `,` delimiter to split processes, then `sscanf` with a `:` delimiter to extract individual fields.

```bash
--processes="A:0:50,B:5:20"
```

> **Note:** File input is space-separated; inline input is colon-separated. `parse_workload_file` and `parse_workload_string` are intentionally separate to avoid conflating the two formats.

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

The `gantt_log` field on `SchedulerState` is a dynamically-allocated `char` buffer where each index maps to one time unit. If process `A` ran from `t=0` to `t=5`, then `gantt_log[0..4] = 'A'`. Using a single flat buffer (rather than a list of intervals) makes the renderer algorithm-agnostic.

- **Scaling** — For long simulations, the renderer groups `N` time units per character block to prevent terminal line-wrapping.
- **`--compare` output** — When comparing algorithms, each algorithm gets its own `gantt_log`; the renderer prints them stacked with a label per row.

---

## Phase 7: Verification & Testing

Checklist to ensure the simulator is lab-ready for CMSC 125.

### 1. Memory — Valgrind Check
Confirm all `malloc`'d queues and process arrays are properly `free`'d. Zero leaks, zero errors.

### 2. Correctness — Lecture Alignment
Run the **Quiz 3** workload. If FCFS average TT ≠ **515**, debug the arrival handling logic.

### 3. Comparative Analysis — `--compare` Mode
Run the same Quiz 3 workload through `--compare` and verify that per-algorithm averages match hand-calculated expectations. Confirm that each algorithm's `gantt_log` is independently allocated and freed without cross-contamination.

### 4. Integration — Shell Compatibility
Invoke `./schedsim` from within `mysh` to verify seamless `fork`/`exec` handling.