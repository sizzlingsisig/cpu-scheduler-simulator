#include "gantt.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void init_gantt_log(SchedulerState *state) {
    state->metrics.gantt_log = malloc(1);
    if (state->metrics.gantt_log != NULL) {
        state->metrics.gantt_log[0] = '\0';
    }
}

void append_gantt_entry(SchedulerState *state, const char *entry) {
    if (state == NULL || entry == NULL) return;
    if (state->metrics.gantt_log == NULL) return;

    size_t current_len = strlen(state->metrics.gantt_log);
    size_t entry_len = strlen(entry);
    size_t separator_len = current_len > 0 ? 1 : 0;
    size_t new_len = current_len + separator_len + entry_len + 1;

    char *new_log = realloc(state->metrics.gantt_log, new_len);
    if (new_log != NULL) {
        state->metrics.gantt_log = new_log;
        if (separator_len > 0) {
            strcat(state->metrics.gantt_log, "|");
        }
        strcat(state->metrics.gantt_log, entry);
    }
}

struct Block {
    char pid[MAX_PID_LEN];
    int start;
    int end;
    int duration;
};

static int split_gantt_log(const char *log, char ***out_tokens) {
    if (!log || !out_tokens) return 0;

    // Count entries (separated by '|')
    size_t count = 0;
    const char *p = log;
    while (*p) {
        count += 1;
        const char *sep = strchr(p, '|');
        if (!sep) break;
        p = sep + 1;
    }

    char **tokens = calloc(count, sizeof(char *));
    if (!tokens) return 0;

    char *copy = strdup(log);
    if (!copy) {
        free(tokens);
        return 0;
    }

    size_t idx = 0;
    char *token = strtok(copy, "|");
    while (token != NULL && idx < count) {
        tokens[idx++] = strdup(token);
        token = strtok(NULL, "|");
    }
    free(copy);

    *out_tokens = tokens;
    return (int)idx;
}

void render_gantt_chart(SchedulerState *state) {
    if (state->metrics.gantt_log == NULL || strlen(state->metrics.gantt_log) == 0) {
        printf("No Gantt chart data available.\n");
        return;
    }

    printf("\n=== Gantt Chart ===\n");

    char **entries = NULL;
    int total_time = split_gantt_log(state->metrics.gantt_log, &entries);
    if (total_time <= 0) {
        printf("No Gantt chart data available.\n");
        return;
    }
    
    if (total_time > 50) {
        printf("Each block = 10 time units (aggregated):\n");

        int chunk_size = 10;
        int max_blocks = total_time / chunk_size + 2;
        int *start_times = malloc(max_blocks * sizeof(int));
        int *block_widths = malloc(max_blocks * sizeof(int));
        char **block_pids = malloc(max_blocks * sizeof(char *));
        int num_blocks = 0;

        int current_start = 0;
        int current_width = 0;
        char *current_pid = entries[0];

        for (int i = 0; i < total_time; i += chunk_size) {
            char *chunk_pid = entries[i];
            if (i == 0 || strcmp(chunk_pid, current_pid) == 0) {
                current_width++;
            } else {
                start_times[num_blocks] = current_start;
                block_widths[num_blocks] = current_width;
                block_pids[num_blocks] = current_pid;
                num_blocks++;

                current_width = 1;
                current_start = i;
                current_pid = chunk_pid;
            }
        }

        start_times[num_blocks] = current_start;
        block_widths[num_blocks] = current_width;
        block_pids[num_blocks] = current_pid;
        num_blocks++;

        // Cap block width so chart remains readable
        int max_width_allowed = 8;
        for (int i = 0; i < num_blocks; i++) {
            if (block_widths[i] > max_width_allowed) {
                block_widths[i] = max_width_allowed;
            }
            if (block_widths[i] < 1) {
                block_widths[i] = 1;
            }
        }

        // Print the blocks (PID labels centered in block)
        for (int i = 0; i < num_blocks; i++) {
            int width = block_widths[i];
            int label_len = (int)strlen(block_pids[i]);
            int pad_left = (width > label_len ? (width - label_len) / 2 : 0);
            int pad_right = width - label_len - pad_left;

            printf("[");
            for (int j = 0; j < pad_left; j++) printf(" ");
            printf("%s", block_pids[i]);
            for (int j = 0; j < pad_right; j++) printf(" ");
            printf("]");
        }
        printf("\n");

        printf("Time: 0");
        int time_cursor = 6;
        int chart_cursor = 0;

        for (int i = 0; i < num_blocks; i++) {
            chart_cursor += block_widths[i] + 2;
            int end_time = (i + 1 < num_blocks) ? start_times[i + 1] : total_time;

            char time_str[16];
            sprintf(time_str, "%d", end_time);

            int target_cursor = chart_cursor - (int)strlen(time_str) / 2 + 1;
            if (target_cursor < time_cursor + 1) target_cursor = time_cursor + 1;

            while (time_cursor < target_cursor) {
                printf(" ");
                time_cursor++;
            }

            printf("%s", time_str);
            time_cursor += (int)strlen(time_str);
        }
        printf("\n");

        free(start_times);
        free(block_widths);
        free(block_pids);
    } else {
        // Parse log into blocks
        struct Block *blocks = malloc(total_time * sizeof(struct Block));
        int num_blocks = 0;

        char *current_pid = entries[0];
        int current_start = 0;
        int current_duration = 0;

        for (int i = 0; i <= total_time; i++) {
            if (i < total_time && strcmp(entries[i], current_pid) == 0) {
                current_duration++;
            } else {
                strncpy(blocks[num_blocks].pid, current_pid, MAX_PID_LEN - 1);
                blocks[num_blocks].pid[MAX_PID_LEN - 1] = '\0';
                blocks[num_blocks].start = current_start;
                blocks[num_blocks].end = current_start + current_duration;
                blocks[num_blocks].duration = current_duration;
                num_blocks++;

                if (i < total_time) {
                    current_pid = entries[i];
                    current_start = i;
                    current_duration = 1;
                }
            }
        }
        
        // Calculate display widths
        int *widths = malloc(num_blocks * sizeof(int));
        for (int i = 0; i < num_blocks; i++) {
            widths[i] = (blocks[i].duration * 60) / total_time;
            if (widths[i] < 3) widths[i] = 3; // Ensure room for at least " A "
        }
        
        // Top bar
        printf(" ");
        for (int i = 0; i < num_blocks; i++) {
            for (int j = 0; j < widths[i]; j++) printf("-");
            printf(" ");
        }
        printf("\n");
        
        // Middle bar
        printf("|");
        for (int i = 0; i < num_blocks; i++) {
            int label_len = (int)strlen(blocks[i].pid);
            int left_pad = (widths[i] - label_len) / 2;
            if (left_pad < 0) left_pad = 0;
            int right_pad = widths[i] - label_len - left_pad;
            if (right_pad < 0) right_pad = 0;

            for (int j = 0; j < left_pad; j++) printf(" ");
            printf("%s", blocks[i].pid);
            for (int j = 0; j < right_pad; j++) printf(" ");
            printf("|");
        }
        printf("\n");
        
        // Bottom bar
        printf(" ");
        for (int i = 0; i < num_blocks; i++) {
            for (int j = 0; j < widths[i]; j++) printf("-");
            printf(" ");
        }
        printf("\n");
        
        // Time bar
        int cursor = 0;
        printf("0");
        char time_str[32];
        sprintf(time_str, "0");
        cursor += strlen(time_str);
        
        int next_pipe_pos = 0;
        for (int i = 0; i < num_blocks; i++) {
            next_pipe_pos += widths[i] + 1;
            sprintf(time_str, "%d", blocks[i].end);
            
            int spaces_needed = next_pipe_pos - cursor;
            if (spaces_needed < 1) spaces_needed = 1;
            
            for (int j = 0; j < spaces_needed; j++) printf(" ");
            printf("%s", time_str);
            cursor += spaces_needed + strlen(time_str);
        }
        printf("\n");
        
        free(blocks);
        free(widths);
    }

    for (int i = 0; i < total_time; i++) {
        free(entries[i]);
    }
    free(entries);
}

void cleanup_gantt_log(SchedulerState *state) {
    if (state == NULL) return;
    
    if (state->metrics.gantt_log != NULL) {
        free(state->metrics.gantt_log);
        state->metrics.gantt_log = NULL;
    }
}
