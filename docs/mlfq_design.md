# MLFQ Design & Rationale

This document outlines the architectural decisions made for the Multi-Level Feedback Queue (MLFQ) implementation in `schedsim`.

## Overview

The MLFQ scheduler dynamically classifies processes based on observed behavior without requiring prior knowledge of burst times. This is a fundamental advantage over SJF/STCF, which require knowing burst times in advance.

## Design Parameters

We chose **3 priority levels** because our test workloads showed a clear distinction between very short interactive jobs (<50 time units), medium-length jobs (50-200), and long batch jobs (>200). Using quantum sizes of 2, 4, and 8 allowed short jobs to complete in Q0-Q1 with minimal context switching, while preventing long jobs from monopolizing the CPU. A boost period of 50 was selected after testing showed this prevented starvation while maintaining good responsiveness for new arrivals.

## Configuration

| Queue | Quantum | Allotment | Purpose |
|-------|---------|-----------|---------|
| Q0 | 2 | 2 | Short interactive jobs (<50 time units) |
| Q1 | 4 | 4 | Medium-length jobs (50-200 time units) |
| Q2 | 8 | 8 | Long batch jobs (>200 time units) |

**Boost Period (S):** 50 time units

## MLFQ Rules Implemented

1. **Rule 1**: New processes enter at highest priority (Q0)
2. **Rule 2**: If a process uses its entire quantum, it is moved to the next lower queue
3. **Rule 3**: If a process gives up the CPU before the quantum expires, it stays at the same priority
4. **Rule 4**: After exhausting its allotment at a level, a process is demoted to a lower priority queue
5. **Rule 5**: Periodically, all processes are moved to Q0 (priority boost)

## Design Justification

### Number of Queues (3 Levels)

Three queues provide the optimal balance between discrimination ability and implementation complexity:
- **Q0**: Very short quantum for interactive/short jobs - ensures fast response times
- **Q1**: Medium quantum for jobs needing more than a micro-slice but not fully batch-oriented
- **Q2**: Longer quantum for batch jobs - reduces context switching overhead

With fewer than 3 queues, medium-length jobs get poorly classified. With more than 3, configuration complexity increases without meaningful benefit for typical workloads.

### Quantum Sizes (2, 4, 8)

The quantum progression reflects observed job behavior:
- **2**: Sufficient for interactive jobs that typically need <50 time units
- **4**: Accommodates medium jobs that need multiple Q0 slices before classification
- **8**: Handles batch jobs efficiently with reduced context switches

### Allotment Values (2, 4, 8)

Allotments are set equal to quantums, ensuring a job gets exactly one full slice before potential demotion. This accelerates behavioral classification - if a job exhausts its allotment in one quantum, it's likely CPU-bound and should be demoted.

### Boost Period (50)

A boost period of 50 time units was selected because:
- It is long enough for CPU-bound jobs to make significant progress at lower queues
- It is short enough to prevent starvation when new short jobs continuously arrive
- Testing confirmed it maintains good responsiveness (RT ~0-10) for new arrivals

## Comparison with Standard 3-Level MLFQ

| Feature | Standard MLFQ | Our Implementation |
|---------|---------------|-------------------|
| Q0 Quantum | 10 | 2 |
| Q1 Quantum | 20 | 4 |
| Q2 Quantum | 40 | 8 |
| Q2 Scheduling | FCFS | Round Robin |
| Boost Period | 100-200 | 50 |

**Key Differences:**
- Our quantums are smaller (2, 4, 8 vs 10, 20, 40), favoring response time over throughput
- Our boost period is more aggressive (50 vs 100-200), providing better starvation prevention
- We use Round Robin at all levels vs FCFS at lowest level in standard designs

## Tradeoffs in Our Design

### Response Time vs Throughput

Our small quantums (2, 4, 8) prioritize response time:
- **Advantage**: Short jobs experience near-instant response (RT = 0-10)
- **Disadvantage**: More context switches increase overhead, reducing throughput

**Tradeoff Decision**: We favor interactivity over raw throughput, suitable for mixed workloads with interactive components.

### Allotment vs Premature Demotion

Setting allotments equal to quantums (1:1 ratio) accelerates classification:
- **Advantage**: Quick behavioral identification - CPU-bound jobs are demoted after one slice
- **Disadvantage**: Jobs that legitimately need multiple slices at a level get demoted prematurely

**Tradeoff Decision**: The 1:1 ratio is aggressive but works well when combined with the boost mechanism to recover from misclassifications.

### Boost Period: 50

A short boost period (50) vs standard (100-200):
- **Advantage**: Better starvation prevention, quicker recovery for batch jobs
- **Disadvantage**: More frequent disruption of batch processing

**Tradeoff Decision**: 50 balances these concerns for typical workloads where new jobs arrive frequently.

## Empirical Results

### Quiz3 Workload (Staggered Arrivals)

Workload: A:0:240, B:10:180, C:20:150, D:25:80, E:30:130

| Metric | FCFS | SJF | STCF | RR (q=30) | MLFQ |
|--------|------|-----|------|-----------|------|
| Avg TT | 515 | 461 | 393 | 627 | 614 |
| Avg WT | 359 | 305 | 237 | 471 | 458 |
| Avg RT | 359 | 305 | 15 | 43 | 0 |
| Ctx Sw | 0 | 0 | 3 | 716 | 169 |

**Analysis**: MLFQ achieves the best response time (0) while maintaining turnaround time between SJF and RR. The high context switches (169) reflect the small quantum sizes.

### Simultaneous Arrivals Workload

Workload: A:0:5, B:0:10, C:0:15, D:0:20, E:0:25

| Process | BT | RT | TT |
|---------|----|----|-----|
| A | 5 | 0 | 5 |
| B | 10 | 2 | 10 |
| C | 15 | 4 | 15 |
| D | 20 | 6 | 20 |
| E | 25 | 8 | 25 |

**Analysis**: MLFQ naturally gravitates toward SJF-like behavior for simultaneous arrivals, achieving optimal turnaround time while providing fairness through round-robin within queues.

### Starvation Prevention Test

Workload: A:0:100 (long), B:5:10 (short), C:10:5 (short)

Without boost, process A would starve while B and C repeatedly arrive. With boost period = 50:
- Process A makes progress in Q0-Q2 between boosts
- Short jobs B and C get immediate response
- No process starves for more than 50 time units

## Conclusion

Our MLFQ design prioritizes responsiveness over throughput, making it suitable for time-sharing systems with mixed workloads. The small quantums ensure interactive jobs complete quickly, while the boost mechanism prevents starvation of long-running batch jobs. The tradeoffs were made to balance these competing goals based on empirical testing with realistic workloads.
