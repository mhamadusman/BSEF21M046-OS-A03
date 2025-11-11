#include "shell.h"
#include <readline/readline.h>
#include <readline/history.h>
#include <ctype.h>
#include <signal.h>

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
    if (line[0] == '\0' || is_all_ws(line)) {
        free(line);
        return strdup("");
    }
    return line;
}

/* ignore Ctrl-C in the shell itself */
void sigint_handler(int sig) {
    printf("\n"); // just move to next line
    rl_on_new_line();
    rl_replace_line("", 0);
    rl_redisplay();
}

int main() {
    char *cmdline = NULL;
    char **arglist = NULL;

    /* enable tab completion */
    rl_bind_key('\t', rl_complete);

    /* set signal handler */
    signal(SIGINT, sigint_handler);

    printf("Welcome to FCIT Shell (v6 — Background & Signals)\n");
    printf("Type 'help' for built-in commands, use '!' to recall history entries.\n\n");

    while (1) {
        cmdline = read_cmd(PROMPT);
        if (cmdline == NULL) {
            printf("\nShell exited.\n");
            break;
        }

        if (strlen(cmdline) == 0) {
            free(cmdline);
            continue;
        }

        /* handle !n history recall */
        if (cmdline[0] == '!') {
            char *endptr = NULL;
            long n = strtol(cmdline + 1, &endptr, 10);
            if (endptr == cmdline + 1 || *endptr != '\0' || n <= 0) {
                fprintf(stderr, "Invalid history reference: %s\n", cmdline);
                free(cmdline);
                continue;
            }
            const char *h = hist_get((int)n);
            if (h == NULL) {
                fprintf(stderr, "No such history entry: %ld\n", n);
                free(cmdline);
                continue;
            }
            free(cmdline);
            cmdline = strdup(h);
            if (cmdline == NULL) { perror("strdup"); continue; }
            printf("%s\n", cmdline);
        }

        /* add to history */
        hist_add(cmdline);
        add_history(cmdline);

        /* tokenize and execute */
        arglist = tokenize(cmdline);
        if (arglist != NULL) {
            int exec_ret = execute(arglist);
            for (int i = 0; arglist[i] != NULL; ++i) free(arglist[i]);
            free(arglist);
            if (exec_ret == 0) {
                free(cmdline);
                break;
            }
        }

        free(cmdline);
    }

    printf("Goodbye!\n");
    return 0;
}
