# MLFQ Design & Rationale

This document outlines the architectural decisions made for the Multi-Level Feedback Queue (MLFQ) implementation in `schedsim`.

## 1. Number of Queues
The default configuration uses **3 priority levels** (`Q0`, `Q1`, `Q2`).

**Rationale:** Three queues strike an optimal balance between complexity and performance. 
- `Q0` quickly services interactive (I/O bound) or short jobs.
- `Q1` acts as a middle ground for jobs that need slightly more CPU time but are not fully batch-oriented.
- `Q2` is the background queue for long, CPU-intensive (batch) jobs.

Why specifically three? Adding more queues offers diminishing returns unless workload profiling specifically demands it, while fewer queues restrict the algorithm's ability to discriminate behavior effectively.

### Comparison of Queue Counts

| Number of Queues | Advantages | Disadvantages | Conclusion |
|------------------|------------|---------------|------------|
| **1 Queue** (Standard RR) | Dead simple to implement, extremely low overhead. | Cannot discriminate between short interactive jobs and long CPU-bound tasks. Causes poor Turnaround Time (TT) for long jobs and worse Response Time (RT) for short jobs under load. | Fails the fundamental goal of MLFQ. |
| **2 Queues** | Simple logic, clear division (interactive vs. batch). | Highly binary. Jobs are either "fast" or "slow." Jobs that are "medium" size are quickly demoted to the batch queue and suffer from contention with massive jobs. | Too rigid for dynamic workloads. |
| **3 Queues (Chosen)** | **Balanced.** Allows a buffer level (`Q1`) to catch jobs that need more than a micro-slice but aren't massive batch jobs. Protects `Q0` for purely interactive tasks. | Slightly more memory/overhead than 2 queues, requires tuning 3 sets of quantums and allotments. | **The Sweet Spot.** Provides granular behavior discrimination without over-engineering. |
| **4+ Queues** | Highly granular tracking of process lifespan and behavior. | Diminishing returns. Configuration becomes extremely complex (tuning 4+ quantums/allotments). Context switch overhead increases as jobs "stair-step" down many levels. | Overkill for a general-purpose simulator without specific hardware profiling data. |

## 2. Default Quantums and Allotments
The default configuration uses an **exponentially increasing scale** for both quantums and allotments:
- `Q0`: Quantum = 2, Allotment = 2
- `Q1`: Quantum = 4, Allotment = 4
- `Q2`: Quantum = 8, Allotment = 8

**Rationale:** Short-running or highly interactive processes should get fast but brief access to the CPU to guarantee rapid response times (low RT). Jobs that consistently exhaust their allotment are assumed to be CPU-bound and are demoted. As priority decreases, the time slice increases (8 at `Q2`), which reduces the frequency of context switching overhead for long batch jobs, increasing overall system throughput. Keeping the allotment identical to the quantum ensures a job gets a single full slice before being demoted, accelerating the behavioral classification.

## 3. Priority Boost Period (`S`)
The default priority boost period (`S`) is set to **50 time units**.
**Rationale:** The priority boost rule (Rule 5) strictly guarantees that no long-running batch jobs permanently starve in the lowest priority queue when the system experiences a high influx of short interactive jobs. A value of `50` was chosen empirically; it allows CPU-bound jobs enough time to settle at `Q2` and execute large chunks of their bursts, while ensuring they are rescued frequently enough if short jobs saturate the system.

## 4. Empirical Results and Trade-offs
When tested against simultaneous and staggered workloads:
- **Responsiveness vs Throughput:** MLFQ successfully mimics Shortest Time-to-Completion First (STCF) for new arrivals without needing `burst_time` knowledge. Response times (RT) remain strictly at `0` for freshly arriving processes.
- **Overhead:** MLFQ inherently produces more context switches than FCFS or SJF due to demotion and RR time slicing. However, by lengthening the quantum at lower priority levels, this overhead is effectively bounded.
- **Fairness:** The periodic boost correctly resets the historical memory of a job, allowing a previously CPU-bound process to receive interactive-level priority if its behavior theoretically transitions into an I/O bound phase.
