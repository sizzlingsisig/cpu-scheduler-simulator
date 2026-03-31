# CMSC 125 Lab Defense - CPU Scheduler Simulator

## Setup Commands

```bash
# Navigate to project directory
cd ~/cpu-scheduler-simulator

# View project structure
tree -L 3

# Build the project
make clean && make
```

---

## Demo 1: Basic Execution

```bash
# Run with input file
./schedsim --algorithm=FCFS --input=tests/workloads/quiz3.txt

# Run with CLI string
./schedsim --algorithm=FCFS --processes="A:0:240,B:10:180,C:20:150,D:25:80,E:30:130"
```

---

## Demo 2: Algorithm Comparison

```bash
# Compare all algorithms
./schedsim --compare --input=tests/workloads/quiz3.txt
```

---

## Demo 3: Individual Algorithms

```bash
# FCFS (First Come First Serve)
./schedsim --algorithm=FCFS --input=tests/workloads/quiz3.txt

# SJF (Shortest Job First)
./schedsim --algorithm=SJF --input=tests/workloads/quiz3.txt

# STCF (Shortest Time to Completion First)
./schedsim --algorithm=STCF --input=tests/workloads/quiz3.txt

# Round Robin (with quantum)
./schedsim --algorithm=RR --quantum=30 --input=tests/workloads/quiz3.txt

# MLFQ (Multi-Level Feedback Queue)
./schedsim --algorithm=MLFQ --input=tests/workloads/quiz3.txt
```

---

## Demo 4: MLFQ Configuration

```bash
# Default MLFQ
./schedsim --algorithm=MLFQ --input=tests/workloads/quiz3.txt

# Custom MLFQ config
./schedsim --algorithm=MLFQ --mlfq-config=tests/configs/mlfq_config.txt --input=tests/workloads/quiz3.txt
```

---

## Demo 5: Test Suite

```bash
# Run automated tests
bash tests/test_suite.sh
```

---

## Demo 6: Fork/Exec (Child Process)

```bash
# Verify it works as child process
pid_t pid = fork();
if (pid == 0) {
    char *args[] = {"./schedsim", "--algorithm=FCFS", "--input=tests/workloads/quiz3.txt", NULL};
    execvp(args[0], args);
    exit(1);
}
waitpid(pid, &status, 0);
```

---

## Key Features to Highlight

1. **MLFQ does NOT use burst_time** - scheduling decisions based on observed behavior
2. **Metrics calculated correctly** - TT, WT, RT for each process
3. **Gantt chart generation** - visual execution timeline
4. **Comparison mode** - side-by-side algorithm analysis
5. **Edge cases handled** - zero burst time processes

---


## File Structure Reference

```
schedsim/
├── Makefile
├── docs/
│   └── mlfq_design.md       # MLFQ justification
├── include/                   # Header files
├── src/
│   ├── algorithms/          # FCFS, SJF, STCF, RR, MLFQ
│   └── core/                # Scheduler, parser, metrics
└── tests/
    ├── workloads/            # Test input files
    ├── expected/            # Expected outputs
    ├── configs/             # MLFQ config
    └── test_suite.sh        # Automated testing
```
