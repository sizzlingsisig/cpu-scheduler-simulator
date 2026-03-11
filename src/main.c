#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "process.h"

// TODO: Phase 3/4/5 - Simulation loop and algorithm selection
// TODO: Phase 6 - Metrics calculation and reporting
// TODO: Phase 7 - Memory management and cleanup
// TODO: Refactor into parser.c 
typedef enum {
    OPT_ALGORITHM = 'a',
    OPT_PROCESSES = 'p',
    OPT_INPUT     = 'i',
    OPT_QUANTUM   = 'q',
    OPT_COMPARE   = 1,
} OptKey;

typedef struct {
    char algorithm[16];
    char *processes_str;
    char *input_file;
    int quantum;
    int compare_mode;
} Args;

// Parse command-line arguments into an Args struct.
// Returns 0 on success, 1 on error (unknown option).
static int parse_args(int argc, char *argv[], Args *args) {
    static struct option long_options[] = {
        {"algorithm", required_argument, 0, OPT_ALGORITHM},
        {"processes", required_argument, 0, OPT_PROCESSES},
        {"input",     required_argument, 0, OPT_INPUT    },
        {"quantum",   required_argument, 0, OPT_QUANTUM  },
        {"compare",   no_argument,       0, OPT_COMPARE  },
        {0, 0, 0, 0}
    };

    memset(args, 0, sizeof(*args));

    int opt, option_index = 0;
    while ((opt = getopt_long(argc, argv, "a:p:i:q:", long_options, &option_index)) != -1) {
        switch ((OptKey)opt) {
            case OPT_ALGORITHM:
                strncpy(args->algorithm, optarg, sizeof(args->algorithm) - 1);
                break;
            case OPT_PROCESSES:
                args->processes_str = strdup(optarg);
                break;
            case OPT_INPUT:
                args->input_file = strdup(optarg);
                break;
            case OPT_QUANTUM:
                args->quantum = atoi(optarg);
                break;
            case OPT_COMPARE:
                args->compare_mode = 1;
                break;
            default:
                fprintf(stderr, "Unknown option.\n");
                return 1;
        }
    }
    return 0;
}

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

// Print a debug summary of each parsed process.
static void print_processes(const Process *procs, int num_procs) {
    printf("Parsed %d processes:\n", num_procs);
    for (int i = 0; i < num_procs; i++) {
        printf("  pid: %s\n",            procs[i].pid);
        printf("  arrival_time: %d\n",   procs[i].arrival_time);
        printf("  burst_time: %d\n",     procs[i].burst_time);
        printf("  remaining_time: %d\n", procs[i].remaining_time);
        printf("  start_time: %d\n",     procs[i].start_time);
        printf("  finish_time: %d\n",    procs[i].finish_time);
        printf("  priority: %d\n",       procs[i].priority);
        printf("  allotment_used: %d\n", procs[i].allotment_used);
        printf("  turnaround_time: %d\n",procs[i].turnaround_time);
        printf("  waiting_time: %d\n",   procs[i].waiting_time);
        printf("  response_time: %d\n",  procs[i].response_time);
        printf("---\n");
    }
}

// Free all heap memory owned by args and the process array.
static void cleanup(Args *args, Process *procs) {
    free(procs);
    free(args->processes_str);
    free(args->input_file);
}

int main(int argc, char *argv[]) {
    Args args;
    if (parse_args(argc, argv, &args) != 0)
        return 1;

    int num_procs = 0;
    Process *procs = load_processes(&args, &num_procs);
    if (!procs)
        return 1;

    print_processes(procs, num_procs);

    cleanup(&args, procs);
    return 0;
}