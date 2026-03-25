#!/bin/bash

# SchedSim Regression Tester
# Tests FCFS, SJF, STCF, and RR (Phase 1-4)

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo "--- SchedSim Phase 1-4 Tester ---"

# 1. Build the project
echo -n "Building project... "
make clean > /dev/null 2>&1
make all > /dev/null 2>&1
if [ $? -ne 0 ]; then
    echo -e "${RED}FAILED${NC}"
    exit 1
fi
echo -e "${GREEN}PASSED${NC}"

# Helper to run a test and check for success (exit code 0)
# Usage: run_test "Test Label" "--algorithm=RR --quantum=2 --input=tests/workload.txt"
run_test() {
    local label=$1
    local args=$2
    
    echo -n "Test: $label... "
    ./schedsim $args > /dev/null 2>&1
    
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}PASSED${NC}"
    else
        echo -e "${RED}FAILED${NC}"
        # Run again without suppression to show error for debugging
        ./schedsim $args
    fi
}

# --- FCFS Tests ---
run_test "FCFS Basic (File)" "--algorithm=FCFS --input=tests/workload_single.txt"
run_test "FCFS Simultaneous (String)" "--algorithm=FCFS --processes=A:0:5,B:0:3,C:0:8"
run_test "FCFS With Gaps (File)" "--algorithm=FCFS --input=tests/workload_gap.txt"

# --- SJF Tests ---
run_test "SJF Basic (File)" "--algorithm=SJF --input=tests/workload_single.txt"
run_test "SJF Tie-breaking (String)" "--algorithm=SJF --processes=A:0:5,B:0:5,C:0:5"

# --- STCF Tests (Preemptive SJF) ---
run_test "STCF Basic (File)" "--algorithm=STCF --input=tests/workload_staggered.txt"
run_test "STCF Preemption (String)" "--algorithm=STCF --processes=A:0:10,B:2:2"

# --- RR Tests ---
run_test "RR Basic (File)" "--algorithm=RR --quantum=2 --input=tests/workload_staggered.txt"
run_test "RR Time Slicing (String)" "--algorithm=RR --quantum=1 --processes=A:0:3,B:0:3"
run_test "RR Arrival at Expiry (File)" "--algorithm=RR --quantum=2 --input=tests/rr_edge_case.txt"

# --- MLFQ Tests ---
run_test "MLFQ Default Config (File)" "--algorithm=mlfq --input=tests/workload_mlfq.txt"
run_test "MLFQ Custom Config (String)" "--algorithm=mlfq --mlfq-config=2:2,4:2,4:20 --processes=A:0:10,B:2:5"
run_test "MLFQ Starvation Prevention (String)" "--algorithm=mlfq --processes=A:0:100,B:5:10,C:10:5"

# --- Smoke Tests for Error Handling ---
echo -n "Test: Invalid Algorithm... "
./schedsim --algorithm=INVALID --processes=A:0:1 > /dev/null 2>&1
if [ $? -ne 0 ]; then
    echo -e "${GREEN}PASSED${NC} (Correctly failed)"
else
    echo -e "${RED}FAILED${NC} (Expected failure but returned 0)"
fi

echo -n "Test: Missing Input File... "
./schedsim --algorithm=FCFS --input=non_existent_file.txt > /dev/null 2>&1
if [ $? -ne 0 ]; then
    echo -e "${GREEN}PASSED${NC} (Correctly failed)"
else
    echo -e "${RED}FAILED${NC} (Expected failure but returned 0)"
fi

echo "--- Testing Complete ---"
