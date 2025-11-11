#include "shell.h"
#include <readline/readline.h>
#include <readline/history.h>
#include <ctype.h>

/* check if a line is all whitespace */
static int is_all_ws(const char *s) {
    while (*s) {
        if (!isspace((unsigned char)*s)) return 0;
        s++;
    }
    return 1;
}

/* read_cmd implemented using readline */
char* read_cmd(const char* prompt) {
    char *line = readline(prompt); /* readline returns malloc'd string or NULL on EOF */
    if (line == NULL) return NULL;
    /* If line is empty or only whitespace, return empty string to caller (so they can skip) */
    if (line[0] == '\0' || is_all_ws(line)) {
        free(line);
        return strdup(""); /* return an allocated empty string */
    }
    return line;
}

int main() {
    char *cmdline = NULL;
    char **arglist = NULL;

    /* Enable tab completion provided by readline for filenames/commands */
    rl_bind_key('\t', rl_complete);

    while (1) {
        cmdline = read_cmd(PROMPT);
        if (cmdline == NULL) {
            /* EOF (Ctrl-D) */
            printf("\nShell exited.\n");
            break;
        }

        /* if empty string (only whitespace), prompt again */
        if (strlen(cmdline) == 0) {
            free(cmdline);
            continue;
        }

        /* Handle !n re-execution BEFORE tokenization and before adding to history */
        if (cmdline[0] == '!' ) {
            char *endptr = NULL;
            long n = strtol(cmdline + 1, &endptr, 10);
            if (endptr == cmdline + 1 || *endptr != '\0' || n <= 0) {
                fprintf(stderr, "Invalid history reference: %s\n", cmdline);
                free(cmdline);
                continue;
            }
            const char *h = hist_get((int)n); /* hist_get is 1-based */
            if (h == NULL) {
                fprintf(stderr, "No such history entry: %ld\n", n);
                free(cmdline);
                continue;
            }
            free(cmdline);
            cmdline = strdup(h);
            if (cmdline == NULL) {
                perror("strdup");
                continue;
            }
            printf("%s\n", cmdline); /* show the expanded command */
        }

        /* Add to both our internal history and readline history */
        hist_add(cmdline);
        add_history(cmdline);

        /* Tokenize and execute */
        arglist = tokenize(cmdline);
        if (arglist != NULL) {
            int exec_ret = execute(arglist);
            /* free tokenized memory */
            for (int i = 0; arglist[i] != NULL; ++i) free(arglist[i]);
            free(arglist);

            /* if execute returned 0, built-in requested exit */
            if (exec_ret == 0) {
                free(cmdline);
                break;
            }
        }

        free(cmdline);
    }

    return 0;
}
