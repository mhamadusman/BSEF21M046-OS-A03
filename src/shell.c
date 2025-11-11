#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* Simple history buffer (circular) */
static char *history_buf[HISTORY_SIZE];
static int history_start = 0; /* index of oldest stored */
static int history_len = 0;   /* number of stored entries (<= HISTORY_SIZE) */

/* Add command to internal history (makes a copy) */
void hist_add(const char* cmdline) {
    if (cmdline == NULL) return;
    /* allocate copy */
    char *copy = strdup(cmdline);
    if (!copy) return;

    if (history_len < HISTORY_SIZE) {
        int pos = (history_start + history_len) % HISTORY_SIZE;
        history_buf[pos] = copy;
        history_len++;
    } else {
        /* overwrite oldest */
        free(history_buf[history_start]);
        history_buf[history_start] = copy;
        history_start = (history_start + 1) % HISTORY_SIZE;
    }
}

/* print history with 1-based numbering */
void hist_print(void) {
    int total = hist_count();
    for (int i = 0; i < total; ++i) {
        int idx = (history_start + i) % HISTORY_SIZE;
        printf("%d %s\n", i+1, history_buf[idx]);
    }
}

/* get 1-based index from history; returns NULL if out of range */
const char* hist_get(int index) {
    if (index <= 0 || index > history_len) return NULL;
    int idx = (history_start + (index - 1)) % HISTORY_SIZE;
    return history_buf[idx];
}

int hist_count(void) {
    return history_len;
}

/* Simple tokenizer: splits on whitespace; returns null-terminated array of strings (allocated).
   Caller must free each string and the array. */
char** tokenize(char* cmdline) {
    if (cmdline == NULL) return NULL;

    /* we will support up to MAXARGS tokens (+1 for NULL) */
    char **args = calloc(MAXARGS + 1, sizeof(char*));
    if (!args) {
        perror("calloc");
        return NULL;
    }

    int argc = 0;
    char *p = cmdline;
    while (*p != '\0' && argc < MAXARGS) {
        /* skip whitespace */
        while (isspace((unsigned char)*p)) p++;
        if (*p == '\0') break;

        /* handle quoted strings (single or double quotes) */
        if (*p == '"' || *p == '\'') {
            char quote = *p++;
            char *start = p;
            /* find matching quote */
            while (*p && *p != quote) p++;
            size_t len = p - start;
            char *tok = malloc(len + 1);
            if (!tok) { perror("malloc"); goto token_err; }
            memcpy(tok, start, len);
            tok[len] = '\0';
            args[argc++] = tok;
            if (*p == quote) p++;
        } else {
            /* normal unquoted token */
            char *start = p;
            while (*p && !isspace((unsigned char)*p)) p++;
            size_t len = p - start;
            char *tok = malloc(len + 1);
            if (!tok) { perror("malloc"); goto token_err; }
            memcpy(tok, start, len);
            tok[len] = '\0';
            args[argc++] = tok;
        }
    }

    args[argc] = NULL;
    return args;

token_err:
    for (int i = 0; i < argc; ++i) free(args[i]);
    free(args);
    return NULL;
}

/* Builtin implementations */

int shell_cd(char **args) {
    if (args == NULL || args[0] == NULL) return 1;
    if (args[1] == NULL) {
        char *home = getenv("HOME");
        if (home) {
            if (chdir(home) != 0) perror("cd");
        } else {
            fprintf(stderr, "cd: HOME not set\n");
        }
    } else {
        if (chdir(args[1]) != 0) perror("cd");
    }
    return 1;
}

int shell_help(char **args) {
    (void)args;
    printf("Supported built-in commands:\n");
    printf("  cd [dir]     - Change directory\n");
    printf("  pwd          - Print current working directory\n");
    printf("  echo [text]  - Display text\n");
    printf("  history      - Show command history (numbered)\n");
    printf("  help         - Show this help message\n");
    printf("  exit         - Exit the shell\n");
    return 1;
}

int shell_exit(char **args) {
    (void)args;
    printf("Bye!\n");
    return 0; /* signal to main loop to exit */
}

int shell_pwd(char **args) {
    (void)args;
    char cwd[4096];
    if (getcwd(cwd, sizeof(cwd)) != NULL) printf("%s\n", cwd);
    else perror("pwd");
    return 1;
}

int shell_echo(char **args) {
    if (args == NULL) return 1;
    for (int i = 1; args[i] != NULL; ++i) {
        if (i > 1) putchar(' ');
        fputs(args[i], stdout);
    }
    putchar('\n');
    return 1;
}

/* expose history as a builtin */
int shell_history_builtin(char **args) {
    (void)args;
    hist_print();
    return 1;
}

/* built-in command names and function pointers */
static char *builtin_str[] = {
    "cd", "help", "exit", "pwd", "echo", "history"
};

static int (*builtin_func[])(char **) = {
    &shell_cd, &shell_help, &shell_exit, &shell_pwd, &shell_echo, &shell_history_builtin
};

int num_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
}

/* dispatcher: returns 0 if builtin requested shell exit; 1 if handled; -1 if not a builtin */
int execute_builtin(char **args) {
    if (args == NULL || args[0] == NULL) return 1;
    for (int i = 0; i < num_builtins(); ++i) {
        if (strcmp(args[0], builtin_str[i]) == 0) {
            return (*builtin_func[i])(args);
        }
    }
    return -1;
}
