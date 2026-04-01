#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <limits.h>
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
            case OPT_QUANTUM: {
                char *endptr;
                errno = 0;
                long val = strtol(optarg, &endptr, 10);
                if (errno == ERANGE || endptr == optarg || *endptr != '\0' || val <= 0 || val > INT_MAX) {
                    fprintf(stderr, "Invalid quantum value: %s. Must be a positive integer within range.\n", optarg);
                    return 1;
                }
                args->quantum = (int)val;
                break;
            }
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

void parse_mlfq_config(const char *config_str, MLFQConfig *config) {
    if (!config) return;

    // Default values
    config->num_queues = 3;
    config->quantums[0] = 2;
    config->quantums[1] = 4;
    config->quantums[2] = 8;
    config->allotments[0] = 2;
    config->allotments[1] = 4;
    config->allotments[2] = 8;
    config->boost_period = 50;

    if (!config_str) return;

    FILE* fp = fopen(config_str, "r");
    if (!fp) {
        fprintf(stderr, "Warning: Could not open MLFQ config file %s. Using defaults.\n", config_str);
        return;
    }

    char line[256];
    int queue_idx = 0;
    while (fgets(line, sizeof(line), fp)) {
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') continue;

        char *token = strtok(line, " \t\n\r");
        if (!token) continue;

        if (strncmp(token, "Q", 1) == 0 && queue_idx < MAX_MLFQ_QUEUES) {
            char *quantum_tok = strtok(NULL, " \t\n\r");
            char *allot_tok = strtok(NULL, " \t\n\r");
            if (quantum_tok && allot_tok) {
                {
                    char *endptr;
                    errno = 0;
                    long val = strtol(quantum_tok, &endptr, 10);
                    if (errno == ERANGE || endptr == quantum_tok || *endptr != '\0' || val <= 0 || val > INT_MAX) {
                        fprintf(stderr, "Invalid quantum value in MLFQ config: %s. Must be a positive integer.\n", quantum_tok);
                        fclose(fp);
                        exit(1);
                    }
                    config->quantums[queue_idx] = (int)val;
                }
                {
                    char *endptr;
                    errno = 0;
                    long val = strtol(allot_tok, &endptr, 10);
                    if (errno == ERANGE || endptr == allot_tok || *endptr != '\0' || val <= 0 || val > INT_MAX) {
                        fprintf(stderr, "Invalid allotment value in MLFQ config: %s. Must be a positive integer.\n", allot_tok);
                        fclose(fp);
                        exit(1);
                    }
                    config->allotments[queue_idx] = (int)val;
                }
                queue_idx++;
            }
        } else if (strcmp(token, "BOOST_PERIOD") == 0) {
            char *boost_tok = strtok(NULL, " \t\n\r");
            if (boost_tok) {
                char *endptr;
                errno = 0;
                long val = strtol(boost_tok, &endptr, 10);
                if (errno == ERANGE || endptr == boost_tok || *endptr != '\0' || val <= 0 || val > INT_MAX) {
                    fprintf(stderr, "Invalid boost period value in MLFQ config: %s. Must be a positive integer.\n", boost_tok);
                    fclose(fp);
                    exit(1);
                }
                config->boost_period = (int)val;
            }
        }
    }
    config->num_queues = queue_idx > 0 ? queue_idx : 3;

    fclose(fp);
}
