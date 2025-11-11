#ifndef SHELL_H
#define SHELL_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_LEN 512
#define MAXARGS 10
#define ARGLEN 30
#define PROMPT "FCIT> "

// Function prototypes (match the starter code)
char* read_cmd(char* prompt, FILE* fp);
char** tokenize(char* cmdline);
int execute(char** arglist);

// (Optionally exposed built-in declarations)
int shell_cd(char **args);
int shell_help(char **args);
int shell_exit(char **args);
int shell_pwd(char **args);
int shell_echo(char **args);
int execute_builtin(char **args);

#endif // SHELL_H
