#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "parser.h"
#include "process.h"
#include "scheduler.h"
#include "metrics.h"
#include "policy.h"

// Forward declaration
static Process *load_processes(const Args *args, int *num_procs);

/**
 * Structure to hold comparison results for each algorithm
 */
typedef struct {
    char algorithm[16];
    float avg_tt;
    float avg_wt;
    float avg_rt;
    int context_switches;
} CompareResult;

/**
 * Runs a single algorithm and collects its metrics
 */
static CompareResult run_algorithm(const Args *args, Process *procs, int num_procs, const char *algorithm_name) {
    CompareResult result;
    strncpy(result.algorithm, algorithm_name, sizeof(result.algorithm) - 1);

    // Deep-copy processes to ensure each algorithm starts with clean state
    Process *procs_copy = malloc(num_procs * sizeof(Process));
    if (!procs_copy) {
        fprintf(stderr, "Memory allocation failed in run_algorithm\n");
        result.avg_tt = result.avg_wt = result.avg_rt = -1;
        result.context_switches = -1;
        return result;
    }
    memcpy(procs_copy, procs, num_procs * sizeof(Process));

    SchedulerState state;
    init_scheduler_state(&state, procs_copy, num_procs);
    state.config.quantum = args->quantum;
    parse_mlfq_config(args->mlfq_config, &state.config.mlfq_config);

    if (strcasecmp(algorithm_name, "fcfs") == 0) {
        schedule_fcfs(&state);
    } else if (strcasecmp(algorithm_name, "sjf") == 0) {
        schedule_sjf(&state);
    } else if (strcasecmp(algorithm_name, "stcf") == 0) {
        schedule_stcf(&state);
    } else if (strcasecmp(algorithm_name, "rr") == 0) {
        schedule_rr(&state, state.config.quantum);
    } else if (strcasecmp(algorithm_name, "mlfq") == 0) {
        schedule_mlfq(&state, &state.config.mlfq_config);
    } else {
        fprintf(stderr, "Unknown algorithm: %s\n", algorithm_name);
        result.avg_tt = result.avg_wt = result.avg_rt = -1;
        result.context_switches = -1;
        free(procs_copy);
        return result;
    }

    // Calculate averages
    int total_tt = 0, total_wt = 0, total_rt = 0;
    for (int i = 0; i < num_procs; i++) {
        total_tt += procs_copy[i].turnaround_time;
        total_wt += procs_copy[i].waiting_time;
        total_rt += procs_copy[i].response_time;
    }

    result.avg_tt = (float)total_tt / num_procs;
    result.avg_wt = (float)total_wt / num_procs;
    result.avg_rt = (float)total_rt / num_procs;
    result.context_switches = state.metrics.context_switches;

    cleanup_scheduler_state(&state);
    free(procs_copy);
    return result;
}

/**
 * Prints the comparison table for all algorithms
 */
static void print_comparison_table(CompareResult *results, int num_results) {
    printf("\n--- Algorithm Comparison ---\n");
    printf("%-8s %-8s %-8s %-8s %-8s\n", "Algorithm", "Avg TT", "Avg WT", "Avg RT", "Ctx Sw");
    printf("-------------------------------------------------\n");

    for (int i = 0; i < num_results; i++) {
        printf("%-8s %-8.2f %-8.2f %-8.2f %-8d\n",
            results[i].algorithm,
            results[i].avg_tt,
            results[i].avg_wt,
            results[i].avg_rt,
            results[i].context_switches);
    }
    printf("-------------------------------------------------\n");
}
//  This function abstracts the choice between inline strings and workload 
//  files, ensuring that the main execution loop doesnt need to know 
//  the source of the process data.
//  
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
 * 3. Either run comparison mode (all algorithms) or single algorithm mode.
 * 4. Report results and relinquish all allocated resources.
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

    if (args.compare_mode) {
        // Comparison mode: run all algorithms
        const char *algorithms[] = {"FCFS", "SJF", "STCF", "RR", "MLFQ"};
        int num_algorithms = 5;
        CompareResult results[5];

        printf("Running comparison of all scheduling algorithms...\n");
        printf("Workload: %d processes\n", num_procs);

        for (int i = 0; i < num_algorithms; i++) {
            // Need to reset process states for each algorithm run
            for (int j = 0; j < num_procs; j++) {
                procs[j].remaining_time = procs[j].burst_time;
                procs[j].start_time = -1;
                procs[j].finish_time = 0;
                procs[j].turnaround_time = 0;
                procs[j].waiting_time = 0;
                procs[j].response_time = 0;
                procs[j].state = STATE_NOT_ARRIVED;
                procs[j].priority = 0;
                procs[j].allotment_used = 0;
            }
            results[i] = run_algorithm(&args, procs, num_procs, algorithms[i]);
        }

        print_comparison_table(results, num_algorithms);
    } else {
        // Single algorithm mode
        SchedulerState state;
        init_scheduler_state(&state, procs, num_procs);
        state.config.quantum = args.quantum;

        // Parse MLFQ config from args and store it in SchedulerConfig
        parse_mlfq_config(args.mlfq_config, &state.config.mlfq_config);

        if (strcasecmp(args.algorithm, "fcfs") == 0) {
            schedule_fcfs(&state);
        } else if (strcasecmp(args.algorithm, "sjf") == 0) {
            schedule_sjf(&state);
        } else if (strcasecmp(args.algorithm, "stcf") == 0) {
            schedule_stcf(&state);
        } else if (strcasecmp(args.algorithm, "rr") == 0) {
            schedule_rr(&state, state.config.quantum);
        } else if (strcasecmp(args.algorithm, "mlfq") == 0) {
            schedule_mlfq(&state, &state.config.mlfq_config);
        } else {
            fprintf(stderr, "Unknown or unspecified algorithm: %s\n", args.algorithm);
            free(procs);
            free_args(&args);
            return 1;
        }

        render_gantt_chart(&state);
        print_metrics_table(procs, num_procs);

        cleanup_scheduler_state(&state);
    }

    // Final cleanup: Free the Process array
    free(procs);
    free_args(&args);
    return 0;
}