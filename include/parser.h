#ifndef PARSER_H
#define PARSER_H

typedef struct {
    char algorithm[16];
    char *processes_str;
    char *input_file;
    char *mlfq_config;
    int quantum;
    int compare_mode;
} Args;

// Parse command-line arguments into an Args struct.
// Returns 0 on success, 1 on error (unknown option).
int parse_args(int argc, char *argv[], Args *args);

// Free all heap memory owned by args.
void free_args(Args *args);

#endif // PARSER_H
