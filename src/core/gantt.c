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
    if (state->metrics.gantt_log == NULL) return;
    
    size_t current_len = strlen(state->metrics.gantt_log);
    size_t new_len = current_len + strlen(entry) + 1;
    
    char *new_log = realloc(state->metrics.gantt_log, new_len);
    if (new_log != NULL) {
        state->metrics.gantt_log = new_log;
        strcat(state->metrics.gantt_log, entry);
    }
}

struct Block {
    char pid;
    int start;
    int end;
    int duration;
};

void render_gantt_chart(SchedulerState *state) {
    if (state->metrics.gantt_log == NULL || strlen(state->metrics.gantt_log) == 0) {
        printf("No Gantt chart data available.\n");
        return;
    }
    
    printf("\n=== Gantt Chart ===\n");
    
    size_t total_time = strlen(state->metrics.gantt_log);
    
    if (total_time > 50) {
        printf("Each char = 10 time units:\n");
        
        int chunk_size = 10;
        char current_char = state->metrics.gantt_log[0];
        
        int *start_times = malloc((total_time/chunk_size + 2) * sizeof(int));
        int *block_widths = malloc((total_time/chunk_size + 2) * sizeof(int));
        int num_blocks = 0;
        
        int current_start = 0;
        int current_width = 0;
        
        for (size_t i = 0; i < total_time; i += chunk_size) {
            char chunk_char = state->metrics.gantt_log[i]; 
            
            if (i == 0 || chunk_char == current_char) {
                current_width++;
            } else {
                start_times[num_blocks] = current_start;
                block_widths[num_blocks] = current_width;
                num_blocks++;
                
                current_width = 1;
                current_start = i;
            }
            current_char = chunk_char;
        }
        
        start_times[num_blocks] = current_start;
        block_widths[num_blocks] = current_width;
        num_blocks++;
        
        // Scale block widths appropriately so they don't get absurdly long
        int max_width_allowed = 4;
        for (int i = 0; i < num_blocks; i++) {
             if (block_widths[i] > max_width_allowed) {
                  block_widths[i] = max_width_allowed;
             }
             if (block_widths[i] < 3) {
                  block_widths[i] = 3;
             }
        }
        
        // Print the blocks
        for (int i = 0; i < num_blocks; i++) {
            printf("[");
            char pid = state->metrics.gantt_log[start_times[i]];
            for (int j = 0; j < block_widths[i]; j++) {
                printf("%c", pid);
            }
            printf("]");
        }
        printf("\n");

        printf("Time: 0");
        int time_cursor = 6; 
        int chart_cursor = 0;
        
        for (int i = 0; i < num_blocks; i++) {
            chart_cursor += block_widths[i] + 2; 
            
            int end_time = (i + 1 < num_blocks) ? start_times[i+1] : (int)total_time;
            
            char time_str[16];
            sprintf(time_str, "%d", end_time);
            
            int target_cursor = chart_cursor - strlen(time_str)/2 + 1;
            if (target_cursor < time_cursor + 1) target_cursor = time_cursor + 1; 
            
            while (time_cursor < target_cursor) {
                printf(" ");
                time_cursor++;
            }
            
            printf("%s", time_str);
            time_cursor += strlen(time_str);
        }
        printf("\n");
        
        free(start_times);
        free(block_widths);
    } else {
        // Parse log into blocks
        struct Block *blocks = malloc(total_time * sizeof(struct Block));
        int num_blocks = 0;
        
        char current_char = state->metrics.gantt_log[0];
        int current_start = 0;
        int current_duration = 0;
        
        for (size_t i = 0; i <= total_time; i++) {
            if (i < total_time && state->metrics.gantt_log[i] == current_char) {
                current_duration++;
            } else {
                blocks[num_blocks].pid = current_char;
                blocks[num_blocks].start = current_start;
                blocks[num_blocks].end = current_start + current_duration;
                blocks[num_blocks].duration = current_duration;
                num_blocks++;
                
                if (i < total_time) {
                    current_char = state->metrics.gantt_log[i];
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
            int left_pad = (widths[i] - 1) / 2;
            int right_pad = widths[i] - 1 - left_pad;
            for (int j = 0; j < left_pad; j++) printf(" ");
            printf("%c", blocks[i].pid);
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
}

void cleanup_gantt_log(SchedulerState *state) {
    if (state == NULL) return;
    
    if (state->metrics.gantt_log != NULL) {
        free(state->metrics.gantt_log);
        state->metrics.gantt_log = NULL;
    }
}
