#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

int execute(char **arglist) {
    if (arglist[0] == NULL) return 1;

    /* built-in exit */
    if (strcmp(arglist[0], "exit") == 0) return 0;

    /* built-in cd */
    if (strcmp(arglist[0], "cd") == 0) {
        if (arglist[1] == NULL) {
            fprintf(stderr, "cd: missing argument\n");
        } else {
            if (chdir(arglist[1]) != 0)
                perror("cd");
        }
        return 1;
    }

    /* check if background (&) */
    int background = 0;
    int last = 0;
    while (arglist[last] != NULL) last++;
    if (last > 0 && strcmp(arglist[last-1], "&") == 0) {
        background = 1;
        free(arglist[last-1]);
        arglist[last-1] = NULL;
    }

    pid_t pid = fork();
    if (pid == 0) {
        /* child executes command */
        execvp(arglist[0], arglist);
        perror("execvp");
        exit(1);
    } else if (pid > 0) {
        if (!background) {
            int status;
            waitpid(pid, &status, 0);
        } else {
            printf("[Background pid %d]\n", pid);
        }
    } else {
        perror("fork");
    }

    return 1;
}
