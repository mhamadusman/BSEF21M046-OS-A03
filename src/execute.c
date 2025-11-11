#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

/* Check for built-in commands */
int handle_builtin(char **args) {
    if (strcmp(args[0], "exit") == 0) return 0;
    if (strcmp(args[0], "help") == 0) {
        printf("FCIT Shell built-ins:\n");
        printf("  help  : show this message\n");
        printf("  exit  : exit shell\n");
        return 1;
    }
    return 1; // Not a built-in
}

/* Execute command with I/O redirection and pipes */
int execute(char **args) {
    int i = 0;
    int in_fd = 0;   // input file descriptor
    int out_fd = 1;  // output file descriptor

    // Count pipes
    int pipe_count = 0;
    for (i = 0; args[i]; i++)
        if (strcmp(args[i], "|") == 0) pipe_count++;

    if (pipe_count == 0) {
        // Single command (no pipe)
        pid_t pid = fork();
        if (pid == 0) { // child
            for (i = 0; args[i]; i++) {
                if (strcmp(args[i], "<") == 0) {
                    in_fd = open(args[i + 1], O_RDONLY);
                    if (in_fd < 0) { perror("open input"); exit(1); }
                    dup2(in_fd, STDIN_FILENO);
                    close(in_fd);
                    args[i] = NULL;
                } else if (strcmp(args[i], ">") == 0) {
                    out_fd = open(args[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if (out_fd < 0) { perror("open output"); exit(1); }
                    dup2(out_fd, STDOUT_FILENO);
                    close(out_fd);
                    args[i] = NULL;
                } else if (strcmp(args[i], ">>") == 0) {
                    out_fd = open(args[i + 1], O_WRONLY | O_CREAT | O_APPEND, 0644);
                    if (out_fd < 0) { perror("open append"); exit(1); }
                    dup2(out_fd, STDOUT_FILENO);
                    close(out_fd);
                    args[i] = NULL;
                }
            }
            if (handle_builtin(args) == 0) exit(0);
            execvp(args[0], args);
            perror("execvp");
            exit(1);
        } else if (pid > 0) { // parent
            int status;
            waitpid(pid, &status, 0);
        } else {
            perror("fork");
        }
    } else {
        // Multiple commands (pipe)
        int num_cmds = pipe_count + 1;
        int pipefds[2 * pipe_count];
        for (i = 0; i < pipe_count; i++) {
            if (pipe(pipefds + i*2) < 0) { perror("pipe"); exit(1); }
        }

        int cmd_index = 0;
        int start = 0;
        while (args[start]) {
            // Find next pipe
            int end = start;
            while (args[end] && strcmp(args[end], "|") != 0) end++;
            args[end] = NULL; // temporarily terminate this command

            pid_t pid = fork();
            if (pid == 0) { // child
                if (cmd_index != 0) { // not first cmd
                    dup2(pipefds[(cmd_index-1)*2], STDIN_FILENO);
                }
                if (cmd_index != pipe_count) { // not last cmd
                    dup2(pipefds[cmd_index*2 + 1], STDOUT_FILENO);
                }

                // Close all pipe fds in child
                for (i = 0; i < 2*pipe_count; i++) close(pipefds[i]);

                // Handle I/O redirection in each command
                for (i = start; args[i]; i++) {
                    if (strcmp(args[i], "<") == 0) {
                        int fd = open(args[i+1], O_RDONLY);
                        if (fd < 0) { perror("open input"); exit(1); }
                        dup2(fd, STDIN_FILENO);
                        close(fd);
                        args[i] = NULL;
                    } else if (strcmp(args[i], ">") == 0) {
                        int fd = open(args[i+1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                        if (fd < 0) { perror("open output"); exit(1); }
                        dup2(fd, STDOUT_FILENO);
                        close(fd);
                        args[i] = NULL;
                    } else if (strcmp(args[i], ">>") == 0) {
                        int fd = open(args[i+1], O_WRONLY | O_CREAT | O_APPEND, 0644);
                        if (fd < 0) { perror("open append"); exit(1); }
                        dup2(fd, STDOUT_FILENO);
                        close(fd);
                        args[i] = NULL;
                    }
                }

                execvp(args[start], &args[start]);
                perror("execvp");
                exit(1);
            }
            start = end + 1;
            cmd_index++;
        }

        // Parent closes all pipe fds
        for (i = 0; i < 2*pipe_count; i++) close(pipefds[i]);
        // Wait for all children
        for (i = 0; i <= pipe_count; i++) wait(NULL);
    }

    return 1;
}
