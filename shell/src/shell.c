#define _DEFAULT_SOURCE
#define _BSD_SOURCE

#include "shell.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

// build prompt
void get_prompt(char *prompt, size_t prompt_size) {
    const char *user = getenv("USER");
    if (!user) user = "unknown";

    char hostname[256];
    if (gethostname(hostname, sizeof(hostname)) != 0) {
        strcpy(hostname, "unknown");
    }
    hostname[sizeof(hostname) - 1] = '\0';

    char cwd[PATH_MAX];
    if (!getcwd(cwd, sizeof(cwd))) {
        strcpy(cwd, "?");
    }

    snprintf(prompt, prompt_size, "%s@%s:%s> ", user, hostname, cwd);
}

// replace $ tokens w/ their env value
void expand_env_tokens(tokenlist *tokens) {
    for (size_t i = 0; i < tokens->size; i++) {
        char *t = tokens->items[i];
        if (!t) continue;

        if (t[0] == '$' && t[1] != '\0') {
            const char *val = getenv(t + 1);
            if (!val) val = "";

            char *rep = strdup(val);
            if (!rep) continue;

            free(tokens->items[i]);
            tokens->items[i] = rep;
        }
    }
}

// handles ~ + ~/path expansion
void expand_tilde_tokens(tokenlist *tokens) {
    const char *home = getenv("HOME");
    if (!home) return;

    size_t home_len = strlen(home);

    for (size_t i = 0; i < tokens->size; i++) {
        char *t = tokens->items[i];
        if (!t) continue;

        if (strcmp(t, "~") == 0) {
            char *rep = strdup(home);
            if (!rep) continue;
            free(tokens->items[i]);
            tokens->items[i] = rep;
        } else if (strncmp(t, "~/", 2) == 0) {
            size_t rest_len = strlen(t + 1);
            char *rep = (char *)malloc(home_len + rest_len + 1);
            if (!rep) continue;

            strcpy(rep, home);
            strcat(rep, t + 1);

            free(tokens->items[i]);
            tokens->items[i] = rep;
        }
    }
}

// search path directories
char *search_path(const char *cmd) {
    if (strchr(cmd, '/') != NULL) {
        if (access(cmd, X_OK) == 0) {
            return strdup(cmd);
        }
        return NULL;
    }

    char *path_env = getenv("PATH");
    if (!path_env) return NULL;

    char *path_copy = strdup(path_env);
    char *dir = strtok(path_copy, ":");

    while (dir != NULL) {
        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", dir, cmd);

        if (access(full_path, X_OK) == 0) {
            free(path_copy);
            return strdup(full_path);
        }

        dir = strtok(NULL, ":");
    }

    free(path_copy);
    return NULL;
}

// fork + exec a command
int execute_command(char *path, char **args) {
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return -1;
    } else if (pid == 0) {
        execv(path, args);
        perror("execv");
        exit(1);
    } else {
        int status;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        }
        return -1;
    }
}

// fork + exec w/ optional I/O file redirection
int execute_command_with_redirection(char *path, char **args, redirection *redir) {
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return -1;
    } else if (pid == 0) {
        if (redir && redir->input_file) {
            int fd_in = open(redir->input_file, O_RDONLY);
            if (fd_in < 0) {
                perror(redir->input_file);
                exit(1);
            }

            struct stat st;
            if (fstat(fd_in, &st) < 0 || !S_ISREG(st.st_mode)) {
                fprintf(stderr, "%s: not a regular file\n", redir->input_file);
                close(fd_in);
                exit(1);
            }

            dup2(fd_in, STDIN_FILENO);
            close(fd_in);
        }

        if (redir && redir->output_file) {
            int fd_out = open(redir->output_file,
                            O_WRONLY | O_CREAT | O_TRUNC,
                            S_IRUSR | S_IWUSR);
            if (fd_out < 0) {
                perror(redir->output_file);
                exit(1);
            }

            dup2(fd_out, STDOUT_FILENO);
            close(fd_out);
        }

        execv(path, args);
        perror("execv");
        exit(1);
    } else {
        int status;
        waitpid(pid, &status, 0);

        if (WIFEXITED(status)) {
            return WEXITSTATUS(status);
        }
        return -1;
    }
}

// setup pipes + execute
int execute_pipeline(char ***commands, int num_commands) {
    int pipefds[2 * (num_commands - 1)];

    for (int i = 0; i < num_commands - 1; i++) {
        if (pipe(pipefds + i * 2) < 0) {
            perror("pipe");
            return -1;
        }
    }

    pid_t pids[num_commands];

    for (int i = 0; i < num_commands; i++) {
        char *path = search_path(commands[i][0]);
        if (path == NULL) {
            fprintf(stderr, "%s: command not found\n", commands[i][0]);
            for (int j = 0; j < 2 * (num_commands - 1); j++) {
                close(pipefds[j]);
            }
            return -1;
        }

        pids[i] = fork();

        if (pids[i] < 0) {
            perror("fork");
            free(path);
            return -1;
        } else if (pids[i] == 0) {
            // read from prev pipe
            if (i > 0) {
                dup2(pipefds[(i - 1) * 2], STDIN_FILENO);
            }
            
            // write to next pipe
            if (i < num_commands - 1) {
                dup2(pipefds[i * 2 + 1], STDOUT_FILENO);
            }

            for (int j = 0; j < 2 * (num_commands - 1); j++) {
                close(pipefds[j]);
            }

            execv(path, commands[i]);
            perror("execv");
            exit(1);
        }

        free(path);
    }

    // close pipe fds in parent
    for (int i = 0; i < 2 * (num_commands - 1); i++) {
        close(pipefds[i]);
    }

    // wait for children
    for (int i = 0; i < num_commands; i++) {
        int status;
        waitpid(pids[i], &status, 0);
    }

    return 0;
}

// bg job funcs

void init_bg_jobs(bg_job *jobs) {
    for (int i = 0; i < MAX_BG_JOBS; i++) {
        jobs[i].active = 0;
    }
}

int add_bg_job(bg_job *jobs, pid_t pid, const char *cmd_line, int *job_counter) {
    for (int i = 0; i < MAX_BG_JOBS; i++) {
        if (!jobs[i].active) {
            (*job_counter)++;
            jobs[i].pid = pid;
            jobs[i].job_number = *job_counter;
            strncpy(jobs[i].command_line, cmd_line, 199);
            jobs[i].command_line[199] = '\0';
            jobs[i].active = 1;
            printf("[%d] %d\n", jobs[i].job_number, pid);
            return i;
        }
    }
    return -1;
}

void check_bg_jobs(bg_job *jobs) {
    for (int i = 0; i < MAX_BG_JOBS; i++) {
        if (jobs[i].active) {
            int status;
            pid_t result = waitpid(jobs[i].pid, &status, WNOHANG);
            if (result > 0) {
                printf("[%d]+ done %s\n", jobs[i].job_number, jobs[i].command_line);
                jobs[i].active = 0;
            }
        }
    }
}

void wait_all_bg_jobs(bg_job *jobs) {
    for (int i = 0; i < MAX_BG_JOBS; i++) {
        if (jobs[i].active) {
            int status;
            waitpid(jobs[i].pid, &status, 0);
            printf("[%d]+ done %s\n", jobs[i].job_number, jobs[i].command_line);
            jobs[i].active = 0;
        }
    }
}

void print_jobs(bg_job *jobs) {
    int found = 0;
    for (int i = 0; i < MAX_BG_JOBS; i++) {
        if (jobs[i].active) {
            printf("[%d]+ %d %s\n", jobs[i].job_number, jobs[i].pid, jobs[i].command_line);
            found = 1;
        }
    }
    if (!found) {
        printf("No active background processes\n");
    }
}

// doesn't wait, adds to bg jobs
int execute_pipeline_bg(char ***commands, int num_commands, bg_job *jobs, int *job_counter, const char *cmd_line) {
    int pipefds[2 * (num_commands - 1)];

    for (int i = 0; i < num_commands - 1; i++) {
        if (pipe(pipefds + i * 2) < 0) {
            perror("pipe");
            return -1;
        }
    }

    pid_t last_pid = -1;

    for (int i = 0; i < num_commands; i++) {
        char *path = search_path(commands[i][0]);
        if (path == NULL) {
            fprintf(stderr, "%s: command not found\n", commands[i][0]);
            for (int j = 0; j < 2 * (num_commands - 1); j++) {
                close(pipefds[j]);
            }
            return -1;
        }

        pid_t pid = fork();

        if (pid < 0) {
            perror("fork");
            free(path);
            return -1;
        } else if (pid == 0) {
            if (i > 0) {
                dup2(pipefds[(i - 1) * 2], STDIN_FILENO);
            }
            if (i < num_commands - 1) {
                dup2(pipefds[i * 2 + 1], STDOUT_FILENO);
            }
            for (int j = 0; j < 2 * (num_commands - 1); j++) {
                close(pipefds[j]);
            }
            execv(path, commands[i]);
            perror("execv");
            exit(1);
        }

        if (i == num_commands - 1) {
            last_pid = pid;
        }
        free(path);
    }

    for (int i = 0; i < 2 * (num_commands - 1); i++) {
        close(pipefds[i]);
    }

    add_bg_job(jobs, last_pid, cmd_line, job_counter);

    return 0;
}

// built in cmds
void change_directory(char **args, int arg_count) {
    if (arg_count > 2) {
        fprintf(stderr, "cd: too many arguments\n");
        return;
    }

    const char *target;
    if (arg_count < 2 || args[1] == NULL) {
        target = getenv("HOME");
        if (!target) {
            fprintf(stderr, "cd: HOME not set\n");
            return;
        }
    } else {
        target = args[1];
    }

    struct stat st;
    if (stat(target, &st) != 0) {
        fprintf(stderr, "cd: %s: No such file or directory\n", target);
        return;
    }
    if (!S_ISDIR(st.st_mode)) {
        fprintf(stderr, "cd: %s: Not a directory\n", target);
        return;
    }

    if (chdir(target) != 0) {
        perror("cd");
    }
}

// keeps history of last 3 commands for exit
void add_history(char history[][200], int *history_count, const char *cmd) {
    if (*history_count < HISTORY_SIZE) {
        strncpy(history[*history_count], cmd, 199);
        history[*history_count][199] = '\0';
        (*history_count)++;
    } else {
        for (int i = 0; i < HISTORY_SIZE - 1; i++) {
            strcpy(history[i], history[i + 1]);
        }
        strncpy(history[HISTORY_SIZE - 1], cmd, 199);
        history[HISTORY_SIZE - 1][199] = '\0';
    }
}

void print_exit_history(char history[][200], int history_count) {
    if (history_count == 0) {
        printf("No valid commands in history\n");
        return;
    }
    for (int i = 0; i < history_count; i++) {
        printf("%s\n", history[i]);
    }
}