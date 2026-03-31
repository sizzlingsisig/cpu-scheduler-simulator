# SchedSim: CPU Scheduling Simulator

A discrete-event CPU scheduling simulator built in C for CMSC 125 (Lab 2). This simulator models how an operating system allocates CPU time to competing processes, prioritizing metrics like turnaround time, response time, and fairness.

## Group Members

- Christian Hernia
- Julo Bretana

## Features and Algorithms

This simulator implements five distinct scheduling policies:

1. **First-Come First-Serve (FCFS):** Non-preemptive, strict arrival-time execution.
2. **Shortest Job First (SJF):** Non-preemptive, prioritizes shortest total burst time.
3. **Shortest Time-to-Completion First (STCF):** Preemptive SJF, dynamically interrupts for shorter incoming jobs.
4. **Round Robin (RR):** Preemptive, time-sliced execution for fairness with configurable quantum.
5. **Multi-Level Feedback Queue (MLFQ):** Heuristic-based preemptive scheduler that learns process behavior without prior knowledge of burst times.

### Additional Features

- Gantt chart generation for visualizing execution timelines
- Comprehensive metrics: Turnaround Time (TT), Waiting Time (WT), Response Time (RT)
- Algorithm comparison mode
- Configurable MLFQ parameters via config file
- Discrete-event simulation engine
- Memory-safe implementation with proper cleanup

## Compilation

A `Makefile` is provided for standard Unix build environments.

```bash
# Compile the simulator
make clean && make

# Run the automated test suite
bash tests/test_suite.sh

# Clean compiled binaries and object files
make clean
```

## Usage

### Running with a workload file

```bash
./schedsim --algorithm=FCFS --input=tests/workloads/quiz3.txt
```

### Running with a command-line string

```bash
./schedsim --algorithm=RR --quantum=30 --processes="A:0:240,B:10:180,C:20:150"
```

### Comparison Mode

```bash
./schedsim --compare --input=tests/workloads/quiz3.txt
```

### MLFQ with custom configuration

```bash
./schedsim --algorithm=MLFQ --mlfq-config=tests/configs/mlfq_config.txt --input=tests/workloads/quiz3.txt
```

## Command-Line Options

| Option | Description |
|--------|-------------|
| `--algorithm=NAME` | Scheduling algorithm: FCFS, SJF, STCF, RR, MLFQ |
| `--input=FILE` | Input workload file |
| `--processes=STR` | Inline workload string (format: PID:AT:BT,...) |
| `--quantum=N` | Time quantum for RR (default: 30) |
| `--mlfq-config=FILE` | MLFQ configuration file |
| `--compare` | Run all algorithms and compare results |

## Example Output

### FCFS on Quiz3 Workload

```
=== Gantt Chart ===
Each char = 10 time units:
[AAAA][BBBB][CCCC][DDDD][EEEE]
Time: 0 240  420   570   650   780

=== Metrics ===
Process | AT  | BT  | FT  | TT  | WT  | RT  
--------|-----|-----|-----|-----|-----|-----
A       |   0 | 240 | 240 | 240 |   0 |   0
B       |  10 | 180 | 420 | 410 | 230 | 230
C       |  20 | 150 | 570 | 550 | 400 | 400
D       |  25 |  80 | 650 | 625 | 545 | 545
E       |  30 | 130 | 780 | 750 | 620 | 620
--------|-----|-----|-----|-----|-----|-----
Average |     |     |     | 515 | 359 | 359

Convoy effect detected: Process B waited 230 time units
```

### Algorithm Comparison

```
Running comparison of all scheduling algorithms...
Workload: 5 processes

--- Algorithm Comparison ---
Algorithm Avg TT   Avg WT   Avg RT   Ctx Sw  
-------------------------------------------------
FCFS     515.00   359.00   359.00   0       
SJF      461.00   305.00   305.00   0       
STCF     393.00   237.00   15.00    3       
RR       630.60   474.60   1.20     716     
MLFQ     614.00   458.00   0.00     169     
-------------------------------------------------
```

## Known Limitations and Assumptions

1. **Time is discrete:** The simulator operates in integer time units; all arrival times and burst times must be non-negative integers.

2. **No I/O simulation:** This simulator only models CPU burst times. Interactive processes with I/O phases are not simulated.

3. **Context switch overhead:** Context switches are counted but no overhead cost is modeled in the time calculations.

4. **Single CPU:** Only single-processor scheduling is simulated. Multi-core systems are not modeled.

5. **Fixed priority tie-breaking:** When processes have equal priority (same burst time, same arrival time), PIDs are ordered alphabetically. This ensures deterministic results.

6. **MLFQ quantum behavior:** The lowest priority queue (Q2) uses a finite quantum (8) rather than FCFS/infinite time. This is a design choice for consistency.

7. **Memory management:** While proper cleanup is implemented, the simulator is designed for short-lived executions and is not optimized for continuous 24/7 operation.

8. **Process identification:** PIDs are limited to 16 characters and should contain no whitespace.

## Project Structure

```
schedsim/
├── Makefile
├── README.md
├── docs/
│   ├── mlfq_design.md       # MLFQ design justification
│   └── defense_script.md    # Demo commands for lab defense
├── include/                  # Header files
├── src/
│   ├── algorithms/          # Scheduling algorithm implementations
│   └── core/               # Core simulation engine
└── tests/
    ├── workloads/           # Test input files
    ├── expected/           # Expected output files
    ├── configs/            # MLFQ configuration
    └── test_suite.sh       # Automated test script
```
