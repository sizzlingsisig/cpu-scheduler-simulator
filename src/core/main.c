#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "process.h"
#include "scheduler.h"
#include "metrics.h"
#include "policy.h"

// TODO: Phase 3/4/5 - Simulation loop and algorithm selection
// TODO: Phase 6 - Metrics calculation and reporting
// TODO: Phase 7 - Memory management and cleanup

// Load processes from the workload source specified in args.
// Returns a heap-allocated array of Process structs, or NULL on failure.
static Process *load_processes(const Args *args, int *num_procs) {
    if (args->processes_str) {
        return parse_workload_string(args->processes_str, num_procs);
    } else if (args->input_file) {
        return parse_workload_file(args->input_file, num_procs);
    }
    fprintf(stderr, "No workload specified. Use --processes or --input.\n");
    return NULL;
}

int main(int argc, char *argv[]) {
    Args args;
    if (parse_args(argc, argv, &args) != 0)
        return 1;

    int num_procs = 0;
    Process *procs = load_processes(&args, &num_procs);
    if (!procs) {
        free_args(&args);
        return 1;
    }

    SchedulerPolicy *policy = get_policy_by_name(args.algorithm);
    if (!policy) {
        fprintf(stderr, "Unknown or unspecified algorithm: %s\n", args.algorithm);
        free(procs);
        free_args(&args);
        return 1;
    }

    SchedulerState state;
    init_scheduler_state(&state, procs, num_procs);
    state.quantum = args.quantum;

    run_simulation(&state, policy);

    print_metrics_table(procs, num_procs, state.context_switches);

    free(procs);
    free_args(&args);
    return 0;
}