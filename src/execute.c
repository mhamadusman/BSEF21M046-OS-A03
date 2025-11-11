#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

/* built-in command names and function pointers */
char *builtin_str[] = {"cd", "help", "exit", "pwd", "echo", "history"};
int (*builtin_func[])(char **) = {
    &shell_cd, &shell_help, &shell_exit, &shell_pwd, &shell_echo, NULL /* history handled separately */
};

int num_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
}

/* Built-in implementations */

int shell_cd(char **args) {
    if (args[1] == NULL) {
        char *home = getenv("HOME");
        if (home != NULL) {
            if (chdir(home) != 0) perror("cd");
        } else {
            fprintf(stderr, "cd: HOME not set\n");
        }
    } else {
        if (chdir(args[1]) != 0) {
            perror("cd");
        }
    }
    return 1;
}

int shell_help(char **args) {
    printf("Supported commands:\n");
    printf("cd [dir]    - Change directory\n");
    printf("pwd         - Print current working directory\n");
    printf("echo [text] - Display text\n");
    printf("history     - Show command history\n");
    printf("help        - Show this help\n");
    printf("exit        - Exit the shell\n");
    return 1;
}

int shell_exit(char **args) {
    printf("Bye!\n");
    return 0; /* signal to exit shell */
}

int shell_pwd(char **args) {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL)
        printf("%s\n", cwd);
    else
        perror("pwd");
    return 1;
}

int shell_echo(char **args) {
    for (int i = 1; args[i] != NULL; i++) {
        printf("%s", args[i]);
        if (args[i+1] != NULL) printf(" ");
    }
    printf("\n");
    return 1;
}

/* Check & execute builtins.
   Return:
     0 -> shell should exit
     1 -> continue shell loop
    -1 -> not a builtin
*/
int execute_builtin(char **args) {
    if (args == NULL || args[0] == NULL) return 1;

    /* history builtin special-case */
    if (strcmp(args[0], "history") == 0) {
        history_print();
        return 1;
    }

    for (int i = 0; i < num_builtins(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            /* call function pointer */
            if (builtin_func[i] != NULL) {
                return (*builtin_func[i])(args);
            } else {
                return 1;
            }
        }
    }
    return -1; /* not a builtin */
}

/* execute external command or builtin */
int execute(char* arglist[]) {
    int builtin_result = execute_builtin(arglist);
    if (builtin_result != -1) {
        return builtin_result;
    }

    pid_t cpid = fork();
    int status = 0;

    if (cpid == -1) {
        perror("fork failed");
        return 1;
    } else if (cpid == 0) {
        /* Child */
        execvp(arglist[0], arglist);
        /* If execvp returns, an error occurred */
        perror("Command not found");
        exit(1);
    } else {
        /* Parent: wait for child */
        if (waitpid(cpid, &status, 0) == -1) {
            perror("waitpid");
        }
    }
    return 1;
}
