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

/* History settings */
#define HISTORY_SIZE 20

/* I/O / core functions */
char* read_cmd(char* prompt, FILE* fp);
char** tokenize(char* cmdline);
int execute(char** arglist);

/* Built-ins (from v2) */
int shell_cd(char **args);
int shell_help(char **args);
int shell_exit(char **args);
int shell_pwd(char **args);
int shell_echo(char **args);
int execute_builtin(char **args);

/* History functions (v3) */
void history_init(void);
void history_add(const char* cmdline);
void history_print(void);
char* history_get_n(int n); /* returns pointer to stored string or NULL */

/* helper */
char* strtrim(const char *s);

#endif // SHELL_H
