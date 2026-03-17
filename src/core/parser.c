#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "parser.h"

/**
 * OptKey maps command-line characters to internal enum values.
 * Using characters as enum values makes the switch statement highly 
 * readable and directly maps to the 'getopt_long' short-flag string.
 */
typedef enum {
    OPT_ALGORITHM = 'a',
    OPT_PROCESSES = 'p',
    OPT_INPUT     = 'i',
    OPT_QUANTUM   = 'q',
    OPT_COMPARE   = 1,
    OPT_MLFQ_CONFIG = 2,
} OptKey;

/**
 * Implementation of CLI parsing using the GNU getopt_long library.
 * This approach is chosen over manual argv scanning to robustly handle 
 * edge cases like combined flags, space-separated vs equals-separated 
 * arguments, and out-of-order flag placement.
 */
int parse_args(int argc, char *argv[], Args *args) {
    static struct option long_options[] = {
        {"algorithm", required_argument, 0, OPT_ALGORITHM},
        {"processes", required_argument, 0, OPT_PROCESSES},
        {"input",     required_argument, 0, OPT_INPUT    },
        {"quantum",   required_argument, 0, OPT_QUANTUM  },
        {"compare",   no_argument,       0, OPT_COMPARE  },
        {"mlfq-config", required_argument, 0, OPT_MLFQ_CONFIG},
        {0, 0, 0, 0}
    };

    memset(args, 0, sizeof(*args));

    int opt, option_index = 0;
    // The colon suffix in the optstring indicates that the flag requires an argument.
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
            case OPT_MLFQ_CONFIG:
                args->mlfq_config = strdup(optarg);
                break;
            default:
                fprintf(stderr, "Unknown option.\n");
                return 1;
        }
    }
    return 0;
}

/**
 * Centralized destructor for the Args struct.
 * Using strdup() in parse_args necessitates this function to ensure 
 * that memory ownership is correctly relinquished before program exit.
 */
void free_args(Args *args) {
    free(args->processes_str);
    free(args->input_file);
    free(args->mlfq_config);
}
