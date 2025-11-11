#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

// ---------------- Built-in command declarations ----------------
char *builtin_str[] = {"cd", "help", "exit", "pwd", "echo"};
int (*builtin_func[])(char **) = {
    &shell_cd, &shell_help, &shell_exit, &shell_pwd, &shell_echo
};

int num_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
}

// ---------------- Built-in Implementations ----------------

int shell_cd(char **args) {
    if (args[1] == NULL) {
        // If no argument, try to go to HOME
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
    printf("help        - Show this help\n");
    printf("exit        - Exit the shell\n");
    return 1;
}

int shell_exit(char **args) {
    printf("Bye!\n");
    return 0; // 0 -> signal to exit shell loop
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

// ---------------- Check & execute builtins ----------------
int execute_builtin(char **args) {
    if (args == NULL || args[0] == NULL) return 1;
    for (int i = 0; i < num_builtins(); i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }
    return -1; // Not a builtin
}

// ---------------- Main execute() used by main.c ----------------
int execute(char* arglist[]) {
    int builtin_result = execute_builtin(arglist);
    if (builtin_result != -1) {
        // builtin_result==0 => exit shell
        return builtin_result;
    }

    int status;
    pid_t cpid = fork();

    switch (cpid) {
        case -1:
            perror("fork failed");
            return 1;
        case 0: // Child process
            execvp(arglist[0], arglist);
            perror("Command not found"); // only runs if execvp failed
            exit(1);
        default: // Parent process
            if (waitpid(cpid, &status, 0) == -1) {
                perror("waitpid");
            }
            return 1;
    }
}
