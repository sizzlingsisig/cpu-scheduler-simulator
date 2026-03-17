#ifndef PARSER_H
#define PARSER_H

/**
 * Args struct centralizes all command-line inputs.
 * This pattern avoids "argument list bloat" in the main simulation 
 * functions and provides a single location to modify the CLI surface 
 * as new features like MLFQ are added.
 */
typedef struct {
    char algorithm[16];
    char *processes_str;
    char *input_file;
    char *mlfq_config;
    int quantum;
    int compare_mode;
} Args;

/**
 * parse_args handles the initial translation from raw CLI tokens to 
 * the simulation's configuration model. 
 * Using getopt_long ensures the simulator follows POSIX/GNU conventions, 
 * making it familiar to users and easier to integrate with Lab 1 shell scripts.
 */
int parse_args(int argc, char *argv[], Args *args);

/**
 * free_args encapsulates the cleanup logic for heap-allocated CLI strings.
 * This ensures that strings like the MLFQ config or input file paths 
 * don't leak, regardless of whether the simulation succeeds or fails.
 */
void free_args(Args *args);

#endif // PARSER_H
