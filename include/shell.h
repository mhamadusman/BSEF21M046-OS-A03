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
#define MAXARGS 64
#define PROMPT "FCIT> "
#define HISTORY_SIZE 200

/* I/O and core */
char* read_cmd(const char* prompt);
char** tokenize(char* cmdline);
int execute(char** arglist);

/* Builtins */
int shell_cd(char **args);
int shell_help(char **args);
int shell_exit(char **args);
int shell_pwd(char **args);
int shell_echo(char **args);

/* Builtin dispatcher */
int execute_builtin(char **args);

/* History helpers (implemented in shell.c) */
void hist_add(const char* cmdline);         /* add to history buffer (internal) */
void hist_print(void);                      /* builtin: print history */
const char* hist_get(int index);            /* 1-based index get, return NULL if OOB */
int hist_count(void);                       /* how many stored */

#endif /* SHELL_H */
