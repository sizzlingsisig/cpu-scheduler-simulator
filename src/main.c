#include <stdio.h>
#include <stdlib.h>
#include "process.h"
 #include <getopt.h>
 #include <string.h>

// TODO: Phase 3/4/5 - Simulation loop and algorithm selection
// TODO: Phase 6 - Metrics calculation and reporting
// TODO: Phase 7 - Memory management and cleanup


int main(int argc, char *argv[]) {
    static struct option long_options[] = {
        {"algorithm", required_argument, 0, 'a'},
        {"processes", required_argument, 0, 'p'},
        {"input", required_argument, 0, 'i'},
        {"quantum", required_argument, 0, 'q'},
        {"compare", no_argument, 0, 1},
        {0, 0, 0, 0}
    };

    char algorithm[16] = "";
    char *processes_str = NULL;
    char *input_file = NULL;
    // int quantum = 0;
    // int compare_mode = 0;

    int opt;
    int option_index = 0;
    while ((opt = getopt_long(argc, argv, "a:p:i:q:", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'a':
                strncpy(algorithm, optarg, 15);
                algorithm[15] = '\0';
                break;
            case 'p':
                processes_str = strdup(optarg);
                break;
            case 'i':
                input_file = strdup(optarg);
                break;
            // case 'q':
            //     quantum = atoi(optarg);
            //     break;
            // case 1:
            //     compare_mode = 1;
            //     break;
            default:
                fprintf(stderr, "Unknown option.\n");
                return 1;
        }
    }

    // Load processes
    Process *procs = NULL;
    int num_procs = 0;
    if (processes_str) {
        procs = parse_workload_string(processes_str, &num_procs);
    } else if (input_file) {
        procs = parse_workload_file(input_file, &num_procs);
    } else {
        fprintf(stderr, "No workload specified. Use --processes or --input.\n");
        return 1;
    }

    // Debug print parsed processes
    printf("Parsed %d processes:\n", num_procs);
    for (int i = 0; i < num_procs; i++) {
        printf("  pid: %s\n", procs[i].pid);
        printf("  arrival_time: %d\n", procs[i].arrival_time);
        printf("  burst_time: %d\n", procs[i].burst_time);
        printf("  remaining_time: %d\n", procs[i].remaining_time);
        printf("  start_time: %d\n", procs[i].start_time);
        printf("  finish_time: %d\n", procs[i].finish_time);
        printf("  priority: %d\n", procs[i].priority);
        printf("  allotment_used: %d\n", procs[i].allotment_used);
        printf("  turnaround_time: %d\n", procs[i].turnaround_time);
        printf("  waiting_time: %d\n", procs[i].waiting_time);
        printf("  response_time: %d\n", procs[i].response_time);
        printf("---\n");
    }

    // Cleanup
    if (procs) free(procs);
    if (processes_str) free(processes_str);
    if (input_file) free(input_file);
    return 0;
}