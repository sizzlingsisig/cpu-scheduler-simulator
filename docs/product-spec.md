# `schedsim` — CPU Scheduling Simulator
### Product Specification

`schedsim` is a discrete-event CPU scheduling simulator designed for educational analysis and systems programming research. It allows users to model, visualize, and compare the efficiency of various process scheduling algorithms under diverse workload conditions.

---

## 1. Key Functionality

`schedsim` provides a sandbox environment to test how different OS scheduling policies affect system throughput, latency, and fairness.

### Supported Scheduling Algorithms

| Algorithm | Type | Description |
|---|---|---|
| **FCFS** — First-Come, First-Served | Non-preemptive | Baseline policy; processes jobs in strict arrival order |
| **SJF** — Shortest Job First | Non-preemptive | Minimizes average turnaround time by selecting the shortest task first |
| **STCF** — Shortest Time-to-Completion First | Preemptive | Dynamic variant of SJF; re-evaluates the queue on every new arrival |
| **RR** — Round Robin | Preemptive | Fair-share rotation using a configurable time quantum |
| **MLFQ** — Multi-Level Feedback Queue | Preemptive | Adaptive scheduler using priority levels and aging (Priority Boost) to balance interactive and batch workloads |

---

## 2. Core Features

### Flexible Workload Ingestion

`schedsim` supports two methods for defining process workloads:

**File-based input** — Read complex workloads from `.txt` files containing PID, Arrival Time, and Burst Time:
```
$ ./schedsim --input=workload.txt
```

**CLI inline strings** — Define quick tests directly in the terminal using the format `PID:Arrival:Burst`:
```
$ ./schedsim --processes="A:0:50,B:5:20"
```

---

### Comparative Analysis Mode

The `--compare` flag runs a single workload through every implemented algorithm simultaneously, generating a side-by-side statistics table.

```
$ ./schedsim --compare --input=test_suite.txt
```

Metrics reported per algorithm:

| Metric | Description |
|---|---|
| **Average Turnaround Time (TT)** | Total time from process arrival to completion |
| **Average Waiting Time (WT)** | Total time spent idle in the ready queue |
| **Average Response Time (RT)** | Time from arrival until first CPU cycle is granted |
| **Context Switch Count** | Total CPU process swaps; useful for overhead analysis |

---

### Visual Execution Timeline (Gantt Charts)

Every simulation run renders an ASCII Gantt chart showing which process held the CPU at each time unit.

- **Auto-Scaling** — For long-running simulations, the chart scales automatically (e.g., 1 character = 10ms) to remain readable in standard terminal windows.

---

## 3. Advanced MLFQ Configuration

The MLFQ implementation is fully customizable via a configuration file, allowing users to define the rules of their specific OS scheduler.

| Parameter | Description |
|---|---|
| **Number of Queues** | Define three or more priority tiers |
| **Variable Quantums** | Set shorter slices for high-priority (interactive) queues; longer for low-priority (batch) queues |
| **Allotment Tracking** | Define how long a process can stay at a priority level before demotion |
| **Priority Boosting** | Set a global timer to reset all processes to the top priority, preventing starvation |

---

## 4. Usage

`schedsim` is designed as a CLI utility, invokable as a standalone binary or as a child process within a `mysh` Unix shell environment.

```bash
# Run a specific algorithm with a file
$ ./schedsim --algorithm=RR --quantum=20 --input=workload.txt

# Run a comparative analysis across all algorithms
$ ./schedsim --compare --input=test_suite.txt
```

---

