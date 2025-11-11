#include "shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* History implementation (circular buffer) */
static char* history_buf[HISTORY_SIZE];
static int history_count = 0;  /* number of entries currently stored (<= HISTORY_SIZE) */
static int history_start = 0;  /* index of the oldest stored entry (0..HISTORY_SIZE-1) */

/* initialize history */
void history_init(void) {
    for (int i = 0; i < HISTORY_SIZE; i++) history_buf[i] = NULL;
    history_count = 0;
    history_start = 0;
}

/* Helper trim: returns newly allocated trimmed string (must be freed by caller) */
char* strtrim(const char *s) {
    if (s == NULL) return strdup("");
    /* leading */
    while (isspace((unsigned char)*s)) s++;
    if (*s == 0) return strdup("");
    /* trailing */
    const char *end = s + strlen(s) - 1;
    while (end > s && isspace((unsigned char)*end)) end--;
    size_t len = end - s + 1;
    char *out = (char*)malloc(len + 1);
    if (out) {
        memcpy(out, s, len);
        out[len] = '\0';
    }
    return out;
}

/* Add a command to history (duplicates the string) */
void history_add(const char* cmdline) {
    if (cmdline == NULL) return;
    char *trimmed = strtrim(cmdline);
    if (trimmed == NULL) return;
    if (trimmed[0] == '\0') { free(trimmed); return; }
    free(trimmed);

    /* If buffer not full, place at (history_start + history_count) % HISTORY_SIZE */
    if (history_count < HISTORY_SIZE) {
        int idx = (history_start + history_count) % HISTORY_SIZE;
        history_buf[idx] = strdup(cmdline);
        if (history_buf[idx] == NULL) { perror("strdup"); return; }
        history_count++;
    } else {
        /* overwrite oldest at history_start */
        free(history_buf[history_start]);
        history_buf[history_start] = strdup(cmdline);
        if (history_buf[history_start] == NULL) { perror("strdup"); return; }
        history_start = (history_start + 1) % HISTORY_SIZE; /* move start to next oldest */
    }
}

/* Print history numbered starting at 1 (oldest entry = 1) */
void history_print(void) {
    for (int i = 0; i < history_count; i++) {
        int idx = (history_start + i) % HISTORY_SIZE;
        printf("%d %s\n", i+1, history_buf[idx]);
    }
}

/* Get nth history command where n is 1..history_count (1 = oldest) */
char* history_get_n(int n) {
    if (n < 1 || n > history_count) return NULL;
    int idx = (history_start + (n - 1)) % HISTORY_SIZE;
    return history_buf[idx];
}

/* Read command with prompt from fp; returns malloc'd string or NULL on EOF.
   Caller must free. */
char* read_cmd(char* prompt, FILE* fp) {
    if (prompt) {
        /* write prompt to stdout (not to fp) */
        fputs(prompt, stdout);
        fflush(stdout);
    }

    char buffer[MAX_LEN];
    if (fgets(buffer, sizeof(buffer), fp) == NULL) {
        return NULL; /* EOF or error */
    }

    /* strip trailing newline */
    size_t len = strlen(buffer);
    if (len > 0 && buffer[len - 1] == '\n') buffer[len - 1] = '\0';

    return strdup(buffer);
}

/* Tokenize a command line into a NULL-terminated array of strings.
   Allocates memory for the array and for each token (use free as done in main). */
char** tokenize(char* cmdline) {
    if (cmdline == NULL) return NULL;

    char *copy = strdup(cmdline);
    if (copy == NULL) return NULL;

    char **args = malloc((MAXARGS + 1) * sizeof(char*));
    if (args == NULL) { free(copy); return NULL; }

    int argc = 0;
    char *token = strtok(copy, " \t");
    while (token != NULL && argc < MAXARGS) {
        args[argc] = strdup(token);
        if (args[argc] == NULL) { perror("strdup"); /* cleanup */ 
            for (int j = 0; j < argc; j++) free(args[j]);
            free(args);
            free(copy);
            return NULL;
        }
        argc++;
        token = strtok(NULL, " \t");
    }
    args[argc] = NULL;
    free(copy);
    return args;
}
