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

/**
 * Dispatcher for workload loading.
 * This function abstracts the choice between inline strings and workload 
 * files, ensuring that the main execution loop doesn't need to know 
 * the source of the process data.
 */
static Process *load_processes(const Args *args, int *num_procs) {
    if (args->processes_str) {
        return parse_workload_string(args->processes_str, num_procs);
    } else if (args->input_file) {
        return parse_workload_file(args->input_file, num_procs);
    }
    fprintf(stderr, "No workload specified. Use --processes or --input.\n");
    return NULL;
}

/**
 * The master orchestrator for the SchedSim executable.
 * 
 * Execution flow:
 * 1. Parse CLI arguments into a high-level Config.
 * 2. Load the requested workload into a contiguous memory block.
 * 3. Resolve the scheduling policy by its name string.
 * 4. Initialize and run the discrete-event simulation engine.
 * 5. Report results and relinquish all allocated resources.
 */
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
    state.config.quantum = args.quantum;

    // Parse MLFQ config from args and store it in SchedulerConfig
    parse_mlfq_config(args.mlfq_config, &state.config.mlfq_config);

    run_simulation(&state, policy);

    print_metrics_table(procs, num_procs, state.metrics.context_switches);

    // Final cleanup: procs was allocated by load_processes, 
    // and args strings were allocated by parse_args.
    free(procs);
    free_args(&args);
    return 0;
}