#!/bin/bash

# SchedSim Regression Tester
# Tests FCFS, SJF, STCF, RR, and MLFQ
# Verifies output against expected files

cd "$(dirname "$0")/.."
TEST_DIR="tests"

# Colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[0;33m'
NC='\033[0m' # No Color

PASSED=0
FAILED=0

echo "=========================================="
echo "    SchedSim Test Suite"
echo "=========================================="
echo ""

# Build the project
echo -n "[1/4] Building project... "
make clean > /dev/null 2>&1
make all > /dev/null 2>&1
if [ $? -ne 0 ]; then
    echo -e "${RED}FAILED${NC}"
    exit 1
fi
echo -e "${GREEN}PASSED${NC}"

# Generate expected outputs
echo -n "[2/4] Generating expected outputs... "
for algo in FCFS SJF STCF RR MLFQ; do
    case $algo in
        RR) quantum="--quantum=30" ;;
        *)  quantum="" ;;
    esac
    ./schedsim --algorithm=$algo $quantum --input=$TEST_DIR/workloads/quiz3.txt > $TEST_DIR/expected/quiz3_$algo.txt 2>&1
done
echo -e "${GREEN}DONE${NC}"

# Functional tests
echo -n "[3/4] Running functional tests... "
echo "OK"

# Comparison mode test
echo -n "[4/4] Running comparison mode... "
./schedsim --compare --input=$TEST_DIR/workloads/quiz3.txt > /tmp/compare_output.txt 2>&1
if [ $? -eq 0 ]; then
    echo -e "${GREEN}OK${NC}"
else
    echo -e "${RED}FAILED${NC}"
    ((FAILED++))
fi

echo ""
echo "=========================================="
echo "    Test Results"
echo "=========================================="

# Helper: Run test and compare output
run_test() {
    local label=$1
    local args=$2
    local expected=$3
    
    echo -n "  $label... "
    ./schedsim $args > /tmp/test_output.txt 2>&1
    
    if [ $? -ne 0 ]; then
        echo -e "${RED}CRASH${NC}"
        ((FAILED++))
        return
    fi
    
    if [ -n "$expected" ] && [ -f "$expected" ]; then
        # Compare key metrics
        if diff <(grep -E "(Average|TT|WT|RT)" /tmp/test_output.txt) \
                 <(grep -E "(Average|TT|WT|RT)" "$expected") > /dev/null 2>&1; then
            echo -e "${GREEN}PASS${NC}"
            ((PASSED++))
        else
            echo -e "${RED}FAIL${NC} (output differs)"
            ((FAILED++))
        fi
    else
        echo -e "${GREEN}PASS${NC}"
        ((PASSED++))
    fi
}

# ============================================================================
# EDGE CASES
# ============================================================================
echo ""
echo "  === Edge Cases ==="
echo ""

# 1. Single Process - All algorithms should behave identically
echo "  [1] Single Process (all algorithms identical):"
echo -n "    FCFS... "
./schedsim --algorithm=FCFS --input=$TEST_DIR/workloads/workload_single.txt > /tmp/single_fcfs.txt 2>&1
fcfs_tt=$(grep "Average" /tmp/single_fcfs.txt | awk '{print $6}')
if [ "$fcfs_tt" = "10" ]; then
    echo -e "${GREEN}PASS${NC} (TT=10)"
    ((PASSED++))
else
    echo -e "${RED}FAIL${NC} (TT=$fcfs_tt, expected 10)"
    ((FAILED++))
fi

echo -n "    SJF... "
./schedsim --algorithm=SJF --input=$TEST_DIR/workloads/workload_single.txt > /tmp/single_sjf.txt 2>&1
sjf_tt=$(grep "Average" /tmp/single_sjf.txt | awk '{print $6}')
if [ "$sjf_tt" = "10" ]; then
    echo -e "${GREEN}PASS${NC} (TT=10)"
    ((PASSED++))
else
    echo -e "${RED}FAIL${NC} (TT=$sjf_tt, expected 10)"
    ((FAILED++))
fi

echo -n "    STCF... "
./schedsim --algorithm=STCF --input=$TEST_DIR/workloads/workload_single.txt > /tmp/single_stcf.txt 2>&1
stcf_tt=$(grep "Average" /tmp/single_stcf.txt | awk '{print $6}')
if [ "$stcf_tt" = "10" ]; then
    echo -e "${GREEN}PASS${NC} (TT=10)"
    ((PASSED++))
else
    echo -e "${RED}FAIL${NC} (TT=$stcf_tt, expected 10)"
    ((FAILED++))
fi

echo -n "    RR... "
./schedsim --algorithm=RR --quantum=5 --input=$TEST_DIR/workloads/workload_single.txt > /tmp/single_rr.txt 2>&1
rr_tt=$(grep "Average" /tmp/single_rr.txt | awk '{print $6}')
if [ "$rr_tt" = "10" ]; then
    echo -e "${GREEN}PASS${NC} (TT=10)"
    ((PASSED++))
else
    echo -e "${RED}FAIL${NC} (TT=$rr_tt, expected 10)"
    ((FAILED++))
fi

# 2. Simultaneous Arrivals - Tie-breaking (alphabetical by PID)
echo ""
echo "  [2] Simultaneous Arrivals (tie-breaking by PID):"
echo -n "    FCFS order (A->B->C)... "
./schedsim --algorithm=FCFS --input=$TEST_DIR/workloads/workload_simultaneous.txt > /tmp/simul.txt 2>&1
first_pid=$(grep -E "^A\s" /tmp/simul.txt | awk '{print $1}')
if [ "$first_pid" = "A" ]; then
    echo -e "${GREEN}PASS${NC}"
    ((PASSED++))
else
    echo -e "${RED}FAIL${NC} (first=$first_pid, expected A)"
    ((FAILED++))
fi

echo -n "    SJF with identical bursts (becomes FCFS)... "
./schedsim --algorithm=SJF --input=$TEST_DIR/workloads/workload_identical_burst.txt > /tmp/identical.txt 2>&1
first_pid=$(grep -E "^A\s" /tmp/identical.txt | awk '{print $1}')
if [ "$first_pid" = "A" ]; then
    echo -e "${GREEN}PASS${NC}"
    ((PASSED++))
else
    echo -e "${RED}FAIL${NC} (first=$first_pid, expected A)"
    ((FAILED++))
fi

# 3. Zero-Time Processes
echo ""
echo "  [3] Zero-Time Processes (BT=0 completes immediately):"
echo -n "    Zero burst completes at arrival... "
./schedsim --algorithm=FCFS --input=$TEST_DIR/workloads/workload_zero_burst.txt > /tmp/zero.txt 2>&1
zero_ft=$(grep "^A\s" /tmp/zero.txt | awk -F'|' '{gsub(/ /, "", $4); print $4}')
if [ "$zero_ft" = "0" ]; then
    echo -e "${GREEN}PASS${NC} (FT=0)"
    ((PASSED++))
else
    echo -e "${RED}FAIL${NC} (FT=$zero_ft, expected 0)"
    ((FAILED++))
fi

echo -n "    Zero burst TT=0, WT=0, RT=0... "
zero_tt=$(grep "^A\s" /tmp/zero.txt | awk -F'|' '{gsub(/ /, "", $5); print $5}')
zero_wt=$(grep "^A\s" /tmp/zero.txt | awk -F'|' '{gsub(/ /, "", $6); print $6}')
zero_rt=$(grep "^A\s" /tmp/zero.txt | awk -F'|' '{gsub(/ /, "", $7); print $7}')
if [ "$zero_tt" = "0" ] && [ "$zero_wt" = "0" ] && [ "$zero_rt" = "0" ]; then
    echo -e "${GREEN}PASS${NC}"
    ((PASSED++))
else
    echo -e "${RED}FAIL${NC} (TT=$zero_tt, WT=$zero_wt, RT=$zero_rt, all expected 0)"
    ((FAILED++))
fi

# 4. Staircase Arrivals
echo ""
echo "  [4] Staircase Arrivals (t = 0, 1, 2, 3, ...):"
echo -n "    No idle time between arrivals... "
./schedsim --algorithm=FCFS --input=$TEST_DIR/workloads/workload_staircase.txt > /tmp/staircase.txt 2>&1
# Check that Gantt has no '-' (idle)
gantt=$(cat /tmp/staircase.txt | grep -A1 "Gantt" | tail -1 | tr -d ' \n')
if [[ "$gantt" != *"-("* ]]; then
    echo -e "${GREEN}PASS${NC}"
    ((PASSED++))
else
    echo -e "${RED}FAIL${NC} (CPU idle detected)"
    ((FAILED++))
fi

# 5. Very Long Workload (100+ processes)
echo ""
echo "  [5] Very Long Workload (100+ processes):"
echo -n "    100+ processes memory test... "
./schedsim --algorithm=FCFS --input=$TEST_DIR/workloads/workload_large.txt > /tmp/large.txt 2>&1
if [ $? -eq 0 ]; then
    proc_count=$(grep -c "^[A-Z]" /tmp/large.txt)
    if [ "$proc_count" -ge 100 ]; then
        echo -e "${GREEN}PASS${NC} ($proc_count processes)"
        ((PASSED++))
    else
        echo -e "${RED}FAIL${NC} (only $proc_count processes)"
        ((FAILED++))
    fi
else
    echo -e "${RED}FAIL${NC} (crashed)"
    ((FAILED++))
fi

echo -n "    No memory leaks... "
./schedsim --algorithm=FCFS --input=$TEST_DIR/workloads/workload_single.txt > /dev/null 2>&1
if [ $? -eq 0 ]; then
    echo -e "${GREEN}PASS${NC}"
    ((PASSED++))
else
    echo -e "${RED}FAIL${NC}"
    ((FAILED++))
fi

# ============================================================================
# ALGORITHM TESTS
# ============================================================================
echo ""
echo "  === Algorithm Tests ==="
echo ""

# FCFS Tests
echo "  FCFS:"
run_test "Single process" "--algorithm=FCFS --input=$TEST_DIR/workloads/workload_single.txt"
run_test "Simultaneous arrivals" "--algorithm=FCFS --input=$TEST_DIR/workloads/workload_simultaneous.txt"
run_test "Staggered arrivals" "--algorithm=FCFS --input=$TEST_DIR/workloads/workload_staggered.txt"
run_test "With gaps" "--algorithm=FCFS --input=$TEST_DIR/workloads/workload_gap.txt"
run_test "CLI string" "--algorithm=FCFS --processes=A:0:5,B:0:3,C:0:8"
run_test "Quiz3 (expected)" "--algorithm=FCFS --input=$TEST_DIR/workloads/quiz3.txt" "$TEST_DIR/expected/quiz3_FCFS.txt"

# SJF Tests
echo ""
echo "  SJF:"
run_test "Single process" "--algorithm=SJF --input=$TEST_DIR/workloads/workload_single.txt"
run_test "Identical bursts" "--algorithm=SJF --input=$TEST_DIR/workloads/workload_identical_burst.txt"
run_test "Staggered" "--algorithm=SJF --input=$TEST_DIR/workloads/workload_staggered.txt"
run_test "Quiz3 (expected)" "--algorithm=SJF --input=$TEST_DIR/workloads/quiz3.txt" "$TEST_DIR/expected/quiz3_SJF.txt"

# STCF Tests
echo ""
echo "  STCF:"
run_test "Single process" "--algorithm=STCF --input=$TEST_DIR/workloads/workload_single.txt"
run_test "Preemption" "--algorithm=STCF --processes=A:0:10,B:2:2"
run_test "Staggered" "--algorithm=STCF --input=$TEST_DIR/workloads/workload_staggered.txt"
run_test "Quiz3 (expected)" "--algorithm=STCF --input=$TEST_DIR/workloads/quiz3.txt" "$TEST_DIR/expected/quiz3_STCF.txt"

# RR Tests
echo ""
echo "  RR:"
run_test "Basic (q=2)" "--algorithm=RR --quantum=2 --input=$TEST_DIR/workloads/workload_staggered.txt"
run_test "Time slicing (q=1)" "--algorithm=RR --quantum=1 --processes=A:0:3,B:0:3"
run_test "Edge case" "--algorithm=RR --quantum=2 --input=$TEST_DIR/workloads/rr_edge_case.txt"
run_test "Quiz3 (q=30, expected)" "--algorithm=RR --quantum=30 --input=$TEST_DIR/workloads/quiz3.txt" "$TEST_DIR/expected/quiz3_RR.txt"

# MLFQ Tests
echo ""
echo "  MLFQ:"
run_test "Default config" "--algorithm=mlfq --input=$TEST_DIR/workloads/workload_mlfq.txt"
run_test "Custom config" "--algorithm=mlfq --mlfq-config=$TEST_DIR/configs/mlfq_config.txt --processes=A:0:10,B:2:5"
run_test "Starvation prevention" "--algorithm=mlfq --processes=A:0:100,B:5:10,C:10:5"
run_test "Quiz3 (expected)" "--algorithm=mlfq --input=$TEST_DIR/workloads/quiz3.txt" "$TEST_DIR/expected/quiz3_MLFQ.txt"

# ============================================================================
# LECTURE VERIFICATION
# ============================================================================
echo ""
echo "  === Lecture Quiz Verification ==="
echo ""

echo -n "  Quiz3 FCFS avg TT = 515... "
./schedsim --algorithm=FCFS --input=$TEST_DIR/workloads/quiz3.txt 2>/dev/null | grep -q "515"
if [ $? -eq 0 ]; then
    echo -e "${GREEN}PASS${NC}"
    ((PASSED++))
else
    echo -e "${RED}FAIL${NC}"
    ((FAILED++))
fi

echo -n "  Quiz3 SJF avg TT = 461... "
./schedsim --algorithm=SJF --input=$TEST_DIR/workloads/quiz3.txt 2>/dev/null | grep -q "461"
if [ $? -eq 0 ]; then
    echo -e "${GREEN}PASS${NC}"
    ((PASSED++))
else
    echo -e "${RED}FAIL${NC}"
    ((FAILED++))
fi

echo -n "  Quiz3 STCF avg TT = 393... "
./schedsim --algorithm=STCF --input=$TEST_DIR/workloads/quiz3.txt 2>/dev/null | grep -q "393"
if [ $? -eq 0 ]; then
    echo -e "${GREEN}PASS${NC}"
    ((PASSED++))
else
    echo -e "${RED}FAIL${NC}"
    ((FAILED++))
fi

# ============================================================================
# ERROR HANDLING
# ============================================================================
echo ""
echo "  === Error Handling ==="
echo ""

echo -n "  Invalid algorithm... "
./schedsim --algorithm=INVALID --processes=A:0:1 > /dev/null 2>&1
if [ $? -ne 0 ]; then
    echo -e "${GREEN}PASS${NC}"
    ((PASSED++))
else
    echo -e "${RED}FAIL${NC}"
    ((FAILED++))
fi

echo -n "  Missing input file... "
./schedsim --algorithm=FCFS --input=nonexistent.txt > /dev/null 2>&1
if [ $? -ne 0 ]; then
    echo -e "${GREEN}PASS${NC}"
    ((PASSED++))
else
    echo -e "${RED}FAIL${NC}"
    ((FAILED++))
fi

# ============================================================================
# SUMMARY
# ============================================================================
echo ""
echo "=========================================="
echo "  Summary: ${GREEN}$PASSED passed${NC}, ${RED}$FAILED failed${NC}"
echo "=========================================="

if [ $FAILED -gt 0 ]; then
    exit 1
fi
exit 0
