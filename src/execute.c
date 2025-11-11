#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>

/* execute() integrates builtin check and external command execution.
   Returns:
     0 => builtin requested shell exit
     1 => continue shell
*/
int execute(char* arglist[]) {
    if (arglist == NULL || arglist[0] == NULL) return 1;

    int builtin_result = execute_builtin(arglist);
    if (builtin_result != -1) {
        /* builtin_result may be 0 (exit) or 1 (handled) */
        return builtin_result;
    }

    pid_t cpid = fork();
    if (cpid < 0) {
        perror("fork");
        return 1;
    } else if (cpid == 0) {
        /* Child: execute external command */
        execvp(arglist[0], arglist);
        /* execvp returned => error */
        fprintf(stderr, "%s: Command not found or failed to execute: %s\n", arglist[0], strerror(errno));
        exit(127);
    } else {
        /* Parent: wait for child to finish */
        int status;
        if (waitpid(cpid, &status, 0) == -1) {
            perror("waitpid");
        }
        return 1;
    }
}
