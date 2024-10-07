#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

#define MAX_INPUT_SIZE 512
#define MAX_ARGS 64
#define MAX_PIPES 30
#define MAX_JOBS 20

struct job {
    pid_t pid;
    char command[MAX_INPUT_SIZE];
};

struct job jobs[MAX_JOBS];
int job_count = 0;

void add_job(pid_t pid, char *command) {
    if (job_count < MAX_JOBS) {
        jobs[job_count].pid = pid;
        strncpy(jobs[job_count].command, command, MAX_INPUT_SIZE);
        job_count++;
    }
}

void remove_job(pid_t pid) {
    for (int i = 0; i < job_count; i++) {
        if (jobs[i].pid == pid) {
            for (int j = i; j < job_count - 1; j++) {
                jobs[j] = jobs[j + 1];
            }
            job_count--;
            break;
        }
    }
}

void list_jobs(void) {
    for (int i = 0; i < job_count; i++) {
        printf("[%d] %d %s\n", i + 1, jobs[i].pid, jobs[i].command);
    }
}

int is_background(char **args) {
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "&") == 0) {
            args[i] = NULL;
            return 1;
        }
    }
    return 0;
}

void parse_input(char *input, char **args) {
    char *token = strtok(input, " \t\n");
    int i = 0;
    while (token != NULL && i < MAX_ARGS - 1) {
        args[i++] = token;
        token = strtok(NULL, " \t\n");
    }
    args[i] = NULL;
}

int handle_redirection(char **args) {
    int in_fd = -1, out_fd = -1;

    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "<") == 0) {
            in_fd = open(args[i+1], O_RDONLY);
            if (in_fd == -1) {
                perror("Error: Input redirection failed");
                return -1;
            }
            dup2(in_fd, STDIN_FILENO);
            close(in_fd);
            args[i] = NULL;
            args[i+1] = NULL;
        } else if (strcmp(args[i], ">") == 0) {
            out_fd = open(args[i+1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (out_fd == -1) {
                perror("Error: Output redirection failed");
                return -1;
            }
            dup2(out_fd, STDOUT_FILENO);
            close(out_fd);
            args[i] = NULL;
            args[i+1] = NULL;
        }
    }
    return 0;
}

int execute_piped_commands(char **args) {
    int pipe_count = 0;
    int pipe_fds[MAX_PIPES][2];
    pid_t pid, last_pid;
    int background = is_background(args);

    // Count pipes
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "|") == 0) {
            pipe_count++;
        }
    }

    if (pipe_count == 0) {
        return 0; // No pipes, return to normal execution
    }

    // Create pipes
    for (int i = 0; i < pipe_count; i++) {
        if (pipe(pipe_fds[i]) < 0) {
            perror("Error: Pipe creation failed");
            return -1;
        }
    }

    // Save original stdin and stdout
    int original_stdin = dup(STDIN_FILENO);
    int original_stdout = dup(STDOUT_FILENO);

    // Execute commands
    int k = 0;
    for (int i = 0; i <= pipe_count; i++) {
        int j = k;
        while (args[k] != NULL && strcmp(args[k], "|") != 0) {
            k++;
        }
        args[k] = NULL;

        pid = fork();
        if (pid == 0) {
            // Child process
            if (i < pipe_count) {
                dup2(pipe_fds[i][1], STDOUT_FILENO);
            } else {
                dup2(original_stdout, STDOUT_FILENO);
            }
            if (i > 0) {
                dup2(pipe_fds[i-1][0], STDIN_FILENO);
            }

            // Close all pipe file descriptors
            for (int m = 0; m < pipe_count; m++) {
                close(pipe_fds[m][0]);
                close(pipe_fds[m][1]);
            }

            // Handle redirection for the first and last command in the pipe
            if (i == 0 || i == pipe_count) {
                handle_redirection(&args[j]);
            }

            execvp(args[j], &args[j]);
            fprintf(stderr, "Error: Command not found - %s\n", args[j]);
            exit(1);
        } else if (pid < 0) {
            perror("Error: Fork failed");
            return -1;
        }
        // Parent process
        if (i > 0) {
            close(pipe_fds[i-1][0]);
        }
        if (i < pipe_count) {
            close(pipe_fds[i][1]);
        }

        last_pid = pid;
        k++;
    }

    // Restore original stdin and stdout
    dup2(original_stdin, STDIN_FILENO);
    dup2(original_stdout, STDOUT_FILENO);
    close(original_stdin);
    close(original_stdout);

    // Close all pipe file descriptors in parent
    for (int i = 0; i < pipe_count; i++) {
        close(pipe_fds[i][0]);
        close(pipe_fds[i][1]);
    }
    
    if (!background) {
        // Wait for all child processes
        for (int i = 0; i <= pipe_count; i++) {
            wait(NULL);
        }
    } else {
        printf("[%d] %d\n", job_count + 1, last_pid);
        add_job(last_pid, args[0]);
    }
    return 1;
}

void execute_command(char **args) {
    if (args[0] == NULL) {
        fprintf(stderr, "Error: Invalid command\n");
        return;
    }
    if (strcmp(args[0], "jobs") == 0) {
        list_jobs();
        return;
    }
    int background = is_background(args);
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        if (handle_redirection(args) == -1) {
            exit(1);
        }
        execvp(args[0], args);
        fprintf(stderr, "Error: Command not found - %s\n", args[0]);
        exit(1);
    } else if (pid > 0) {
        // Parent process
        if (background) {
            printf("[%d] %d\n", job_count + 1, pid);
            add_job(pid, args[0]);
        } else {
            waitpid(pid, NULL, 0);
        }
    } else {
        perror("Error: Fork failed");
    }
}

void execute_multiple_commands(char *input) {
    char *command;
    char *saveptr;

    command = strtok_r(input, ";", &saveptr);
    while (command != NULL) {
        char *args[MAX_ARGS];
        parse_input(command, args);
        if (strlen(command) == 0) {
            continue;  // Skip empty commands
        }

        if (strcmp(args[0], "exit") == 0 || strcmp(args[0], "quit") == 0 || strcmp(args[0], "!q") == 0) {
             exit(0);
        }
        if (execute_piped_commands(args) == 0) {
            // If there are no pipes, execute as a single command
            execute_command(args);
        }
        command = strtok_r(NULL, ";", &saveptr);
    }
}

void sigchld_handler(int signum) {
    pid_t pid;
    int status;
    (void)signum;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        if (WIFEXITED(status) || WIFSIGNALED(status)) {
            for (int i = 0; i < job_count; i++) {
                if (jobs[i].pid == pid) {
                    printf("Background process %d (%s) completed\n", pid, jobs[i].command);
                    remove_job(pid);
                    break;
                }
            }
        }
    }
}

int main(void) {
    char input[MAX_INPUT_SIZE];
    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    if (sigaction(SIGCHLD, &sa, 0) == -1) {
        perror("sigaction");
        exit(1);
    }

    while (1) {
        printf("ASHell> ");
        fflush(stdout);
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }
        input[strcspn(input, "\n")] = '\0';
        if (strlen(input) == 0) {
            continue;
        }

        if (strlen(input) >= MAX_INPUT_SIZE - 1) {
            fprintf(stderr, "Error: Command line too long\n");
            continue;
        }

        execute_multiple_commands(input);
    }

    return 0;
}
