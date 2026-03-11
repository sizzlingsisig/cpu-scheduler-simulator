# SchedSim — Test Commands Reference

Run all commands from the project root (`cpu-scheduler-simulator/`).  
Build the binary first before running any test: `make all`

---

## 0. Build

```bash
# Clean build
make clean && make all

# Verify binary exists
ls -l schedsim
```

---

## Phase 2 — Parsing & CLI (Currently Implemented)

These tests validate that the CLI flags and workload parsers are working correctly.  
Expected output: a debug dump of every field for each parsed process.

### File Input — Default Workload

```bash
./schedsim --input=workload.txt
./schedsim -i workload.txt
```

**Expected:** 3 processes printed (A, B, C) with correct arrival/burst times.

---

### File Input — Edge Case Workloads

```bash
# Single process
./schedsim --input=tests/workload_single.txt

# All arrive at t=0
./schedsim --input=tests/workload_simultaneous.txt

# Staggered arrivals with a CPU idle gap
./schedsim --input=tests/workload_staggered.txt
```

**Single process check:** `remaining_time` must equal `burst_time` (10). `start_time` must be `-1`.

---

### Inline String Input

```bash
# Short inline workload
./schedsim --processes="A:0:10,B:2:5,C:4:7"
./schedsim -p "A:0:10,B:2:5,C:4:7"

# Single process via string
./schedsim --processes="Z:0:100"

# Many processes
./schedsim --processes="P1:0:50,P2:10:30,P3:20:20,P4:30:10,P5:40:5"
```

---

### Algorithm Flag (Parsed but Not Yet Used)

```bash
# These should parse without errors — algorithm string is captured but not acted on yet
./schedsim --algorithm=FCFS --input=workload.txt
./schedsim --algorithm=SJF  --input=workload.txt
./schedsim --algorithm=STCF --input=workload.txt
./schedsim --algorithm=RR   --input=workload.txt
./schedsim --algorithm=MLFQ --input=workload.txt

# Short flags
./schedsim -a FCFS -i workload.txt
```

---

### Error Handling

```bash
# No workload source — should print error and exit non-zero
./schedsim --algorithm=FCFS
echo "Exit code: $?"

# Non-existent file — should print perror message and exit
./schedsim --input=tests/no_such_file.txt
echo "Exit code: $?"

# No flags at all
./schedsim
echo "Exit code: $?"
```

**Expected exit codes:** all of the above return non-zero (1).

---

### Memory Leak Check (Phase 2)

```bash
valgrind --leak-check=full --error-exitcode=1 ./schedsim --input=workload.txt
valgrind --leak-check=full --error-exitcode=1 ./schedsim --processes="A:0:10,B:2:5"
```

**Expected:** `ERROR SUMMARY: 0 errors from 0 contexts`

---

## Phase 3 — FCFS & SJF (Not Yet Implemented)

> Uncomment `--quantum` and `--compare` in `main.c` when reaching this phase.

```bash
# FCFS — strict arrival order
./schedsim --algorithm=FCFS --input=workload.txt
./schedsim --algorithm=FCFS --input=tests/workload_simultaneous.txt
./schedsim --algorithm=FCFS --input=tests/workload_staggered.txt

# SJF — shortest burst first among ready processes
./schedsim --algorithm=SJF --input=workload.txt
./schedsim --algorithm=SJF --input=tests/workload_simultaneous.txt

# Quiz 3 correctness check — FCFS avg TT must equal 515
./schedsim --algorithm=FCFS --input=tests/workload_quiz3.txt
```

**Expected output (per-process):** turnaround time, waiting time, response time, and averages.

**Hand-calculated FCFS for `workload.txt` (A=0:10, B=2:5, C=4:7):**

| PID | AT | BT | FT | TT | WT | RT |
|-----|----|----|-----|----|----|-----|
| A   | 0  | 10 | 10  | 10 | 0  | 0   |
| B   | 2  | 5  | 15  | 13 | 8  | 8   |
| C   | 4  | 7  | 22  | 18 | 11 | 11  |
| **Avg** |   |    |     | **13.67** | **6.33** | **6.33** |

---

## Phase 4 — STCF & Round Robin (Not Yet Implemented)

```bash
# STCF — preempts for shorter remaining time
./schedsim --algorithm=STCF --input=workload.txt
./schedsim --algorithm=STCF --input=tests/workload_simultaneous.txt

# Round Robin — requires --quantum flag
./schedsim --algorithm=RR --quantum=2 --input=workload.txt
./schedsim --algorithm=RR --quantum=4 --input=workload.txt
./schedsim --algorithm=RR --quantum=2 --processes="A:0:10,B:2:5,C:4:7"

# Edge case: quantum larger than all burst times (behaves like FCFS)
./schedsim --algorithm=RR --quantum=100 --input=workload.txt
```

**Hand-calculated STCF for `workload.txt`:**

| PID | AT | BT | FT | TT | WT | RT |
|-----|----|----|-----|----|----|-----|
| A   | 0  | 10 | 22  | 22 | 12 | 0   |
| B   | 2  | 5  | 7   | 5  | 0  | 0   |
| C   | 4  | 7  | 14  | 10 | 3  | 3   |
| **Avg** |  |   |     | **12.33** | **5.0** | **1.0** |

---

## Phase 5 — MLFQ (Not Yet Implemented)

```bash
# Basic MLFQ run
./schedsim --algorithm=MLFQ --quantum=2 --input=workload.txt

# Longer workload — long-running process should get demoted
./schedsim --algorithm=MLFQ --quantum=4 --processes="A:0:50,B:10:10,C:20:5"

# Starvation prevention — verify priority boost promotes long-waiting processes
./schedsim --algorithm=MLFQ --quantum=2 --input=tests/workload_staggered.txt
```

**Key correctness checks:**
- A long-running process must never use `burst_time` for demotion — only `allotment_used`.
- After the boost period, all active processes must appear in Q0.
- Short jobs that finish quickly should have low TT and RT.

---

## Phase 6 — Output, Gantt Chart & Compare Mode (Not Yet Implemented)

```bash
# Single algorithm with Gantt chart output
./schedsim --algorithm=FCFS --input=workload.txt
./schedsim --algorithm=RR --quantum=2 --input=workload.txt

# Compare mode — runs all 5 algorithms, prints side-by-side table
./schedsim --compare --input=workload.txt
./schedsim --compare --input=tests/workload_simultaneous.txt
./schedsim --compare --quantum=4 --input=workload.txt
```

**Expected compare table columns:** Algorithm | Avg TT | Avg WT | Avg RT | Context Switches

---

## Phase 7 — Final Verification Checklist

```bash
# Full Valgrind check on compare mode (highest memory pressure)
valgrind --leak-check=full --track-origins=yes --error-exitcode=1 \
  ./schedsim --compare --input=workload.txt

# Quiz 3 correctness
./schedsim --algorithm=FCFS --input=tests/workload_quiz3.txt
# Verify: Avg TT == 515

# Shell integration — invoke from mysh via fork/exec
# (run inside your mysh shell)
schedsim --algorithm=FCFS --input=workload.txt
echo $?
```

---

## Workload Files Reference

| File | Processes | Purpose |
|------|-----------|---------|
| `workload.txt` | A 0 10, B 2 5, C 4 7 | Default dev workload |
| `tests/workload_single.txt` | A 0 10 | One process, baseline |
| `tests/workload_simultaneous.txt` | A-C, all at t=0 | No arrival staggering |
| `tests/workload_staggered.txt` | A 0 10, B 3 4, C 15 6 | CPU idle gap between B and C |
| `tests/workload_quiz3.txt` | TBD from lecture | FCFS avg TT must = 515 |
