# SchedSim: CPU Scheduling Simulator

A discrete-event CPU scheduling simulator built in C for CMSC 125 (Lab 2). This simulator models how an operating system allocates CPU time to competing processes, prioritizing metrics like turnaround time, response time, and fairness. 

**Group Members:**
* Christian
* Julo Bretana

## Features and Algorithms
This simulator implements five distinct scheduling policies:
1. **First-Come First-Serve (FCFS):** Non-preemptive, strict arrival-time execution.
2. **Shortest Job First (SJF):** Non-preemptive, prioritizes shortest total burst time.
3. **Shortest Time-to-Completion First (STCF):** Preemptive SJF, dynamically interrupts for shorter incoming jobs.
4. **Round Robin (RR):** Preemptive, time-sliced execution for fairness.
5. **Multi-Level Feedback Queue (MLFQ):** Heuristic-based preemptive scheduler that learns process behavior without prior knowledge of burst times.

## Compilation
A `Makefile` is provided for standard Unix build environments.

```bash
# Compile the simulator
make all

# Run the automated test suite
make test


# Clean compiled binaries and object files
make clean
```

## Usage
The simulator operates as a standalone executable and is designed to be safely invoked via `fork()` and `execvp()` from a custom Unix shell.

### Running with a workload file
```bash
./schedsim --algorithm=MLFQ --mlfq-config=mlfq_config.txt --input=workload.txt
```

### Running with a command-line string
```bash
./schedsim --algorithm=RR --quantum=30 --processes="A:0:240,B:10:180,C:20:150"
```

### Comparison Mode
```bash
./schedsim --compare --input=workload.txt
```

## Known Limitations and Assumptions