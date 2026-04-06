#pragma once
#include <stddef.h>
#include <sys/types.h>
#include "lexer.h"

#define BUFFER_SIZE 1024
#define PATH_MAX 4096
#define MAX_BG_JOBS 10
#define HISTORY_SIZE 3

// Part 1: Prompt
void get_prompt(char *prompt, size_t prompt_size);

// Part 2: Expand $VARS (whole tokens)
void expand_env_tokens(tokenlist *tokens);

// Part 3: Expand ~ and ~/...
void expand_tilde_tokens(tokenlist *tokens);

// Part 4: Search PATH
char *search_path(const char *cmd);

// Part 5: Execute command
int execute_command(char *path, char **args);

// Part 6: I/O Redirection
typedef struct {
    char *input_file;
    char *output_file;
} redirection;

int execute_command_with_redirection(char *path, char **args, redirection *redir);

// Part 7: Piping
int execute_pipeline(char ***commands, int num_commands);

// Part 8: Background Processing
typedef struct {
    pid_t pid;
    int job_number;
    char command_line[200];
    int active;
} bg_job;

void init_bg_jobs(bg_job *jobs);
int add_bg_job(bg_job *jobs, pid_t pid, const char *cmd_line, int *job_counter);
void check_bg_jobs(bg_job *jobs);
void wait_all_bg_jobs(bg_job *jobs);
void print_jobs(bg_job *jobs);
int execute_pipeline_bg(char ***commands, int num_commands, bg_job *jobs, int *job_counter, const char *cmd_line);

// Part 9: Internal Commands
void change_directory(char **args, int arg_count);
void add_history(char history[][200], int *history_count, const char *cmd);
void print_exit_history(char history[][200], int history_count);