#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "shell.h"
#include "lexer.h"

int main(void) {
    char prompt[BUFFER_SIZE];
    bg_job jobs[MAX_BG_JOBS];
    int job_counter = 0;
    char history[HISTORY_SIZE][200];
    int history_count = 0;

    init_bg_jobs(jobs);

    while (1) {

        // check for finished bg jobs
        check_bg_jobs(jobs);

        // display prompt + read input
        get_prompt(prompt, sizeof(prompt));
        printf("%s", prompt);
        fflush(stdout);

        char *input = get_input();
        if (input == NULL) {
            printf("\n");
            break;
        }

        if (input[0] == '\0') {
            free(input);
            continue;
        }

        tokenlist *tokens = get_tokens(input);

        // expand environment vars and ~ tokens
        expand_env_tokens(tokens);
        expand_tilde_tokens(tokens);

        if (tokens->size == 0) {
            free_tokens(tokens);
            free(input);
            continue;
        }

        // checks if command should run in bg
        int background = 0;
        if (tokens->size > 0 && strcmp(tokens->items[tokens->size - 1], "&") == 0) {
            background = 1;
            free(tokens->items[tokens->size - 1]);
            tokens->items[tokens->size - 1] = NULL;
            tokens->size--;
        }

        // command line string for history + job tracking
        char cmd_line[200] = "";
        for (size_t i = 0; i < tokens->size; i++) {
            if (i > 0) strcat(cmd_line, " ");
            strncat(cmd_line, tokens->items[i], 199 - strlen(cmd_line));
        }

        // exit
        if (strcmp(tokens->items[0], "exit") == 0) {
            wait_all_bg_jobs(jobs);
            print_exit_history(history, history_count);
            free_tokens(tokens);
            free(input);
            break;
        }

        // change directory
        if (strcmp(tokens->items[0], "cd") == 0) {
            change_directory(tokens->items, tokens->size);
            add_history(history, &history_count, cmd_line);
            free_tokens(tokens);
            free(input);
            continue;
        }

        // jobs
         if (strcmp(tokens->items[0], "jobs") == 0) {
            print_jobs(jobs);
            add_history(history, &history_count, cmd_line);
            free_tokens(tokens);
            free(input);
            continue;
         }
            
        // Check for pipes
        int has_pipe = 0;
        for (size_t i = 0; i < tokens->size; i++) {
            if (strcmp(tokens->items[i], "|") == 0) {
                has_pipe = 1;
                break;
            }
        }

        if (has_pipe) {
            // split tokens into separate cmds at each pipe
            char **commands[tokens->size];
            char *cmd_args[tokens->size];
            int num_commands = 0;
            int arg_count = 0;

            for (size_t i = 0; i < tokens->size; i++) {
                if (strcmp(tokens->items[i], "|") == 0) {
                    cmd_args[arg_count] = NULL;
                    
                    commands[num_commands] = malloc((arg_count + 1) * sizeof(char *));
                    for (int j = 0; j < arg_count; j++) {
                        commands[num_commands][j] = cmd_args[j];
                    }
                    commands[num_commands][arg_count] = NULL;
                    
                    num_commands++;
                    arg_count = 0;
                } else {
                    cmd_args[arg_count++] = tokens->items[i];
                }
            }

            cmd_args[arg_count] = NULL;
            commands[num_commands] = malloc((arg_count + 1) * sizeof(char *));
            for (int j = 0; j < arg_count; j++) {
                commands[num_commands][j] = cmd_args[j];
            }
            commands[num_commands][arg_count] = NULL;
            num_commands++;

            if (background) {
                execute_pipeline_bg(commands, num_commands, jobs, &job_counter, cmd_line);
            } else {
                execute_pipeline(commands, num_commands);
            }

            for (int i = 0; i < num_commands; i++) {
                free(commands[i]);
            }
        } else {
            // parse < > fir redirection
            redirection redir = {NULL, NULL};
            char *new_args[tokens->size + 1];
            int arg_count = 0;

            for (size_t i = 0; i < tokens->size; i++) {
                if (strcmp(tokens->items[i], "<") == 0) {
                    if (i + 1 < tokens->size) {
                        redir.input_file = tokens->items[i + 1];
                        i++;
                    }
                } else if (strcmp(tokens->items[i], ">") == 0) {
                    if (i + 1 < tokens->size) {
                        redir.output_file = tokens->items[i + 1];
                        i++;
                    }
                } else {
                    new_args[arg_count++] = tokens->items[i];
                }
            }
            new_args[arg_count] = NULL;

            if (arg_count == 0) {
                free_tokens(tokens);
                free(input);
                continue;
            }

            // path search
            char *path = search_path(new_args[0]);
            if (path == NULL) {
                printf("%s: command not found\n", new_args[0]);
            } else {
                if (background) {
                    pid_t pid = fork();
                    if (pid < 0) {
                        perror("fork");
                    } else if (pid == 0) {
                        if (redir.input_file) {
                            int fd = open(redir.input_file, O_RDONLY);
                            if (fd < 0) { perror(redir.input_file); exit(1); }
                            dup2(fd, STDIN_FILENO);
                            close(fd);
                        }
                        if (redir.output_file) {
                            int fd = open(redir.output_file, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
                            if (fd < 0) { perror(redir.output_file); exit(1); }
                            dup2(fd, STDOUT_FILENO);
                            close(fd);
                        }
                        execv(path, new_args);
                        perror("execv");
                        exit(1);
                    } else {
                        add_bg_job(jobs, pid, cmd_line, &job_counter);
                    }
                } else {
                    execute_command_with_redirection(path, new_args, &redir);
                }
                free(path);
            }
        }
        add_history(history, &history_count, cmd_line);

        free_tokens(tokens);
        free(input);
    }

    return 0;
}