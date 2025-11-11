#include "shell.h"
#include <ctype.h>

/* main loop:
   - read line
   - if begins with !n => replace with history command
   - add non-empty commands to history
   - tokenize and execute
*/

int main() {
    char* cmdline = NULL;
    char** arglist = NULL;

    history_init();

    while (1) {
        cmdline = read_cmd(PROMPT, stdin);
        if (cmdline == NULL) {
            /* EOF (Ctrl-D) */
            printf("\nShell exited.\n");
            break;
        }

        /* Trim whitespace to check emptiness */
        char *trimmed = strtrim(cmdline);
        if (trimmed[0] == '\0') {
            free(trimmed);
            free(cmdline);
            continue;
        }

        /* Handle !n before tokenizing or adding to history */
        if (trimmed[0] == '!') {
            char *numstr = trimmed + 1;
            int n = atoi(numstr);
            if (n <= 0) {
                fprintf(stderr, "Invalid history reference: %s\n", trimmed);
                free(trimmed);
                free(cmdline);
                continue;
            }
            char *histcmd = history_get_n(n);
            if (histcmd == NULL) {
                fprintf(stderr, "No such history entry: %d\n", n);
                free(trimmed);
                free(cmdline);
                continue;
            }
            /* replace cmdline with history command */
            free(cmdline);
            cmdline = strdup(histcmd);
            if (cmdline == NULL) {
                perror("strdup");
                free(trimmed);
                continue;
            }
            /* we DO NOT add the !n string to history; we will add the expanded command later */
        } else {
            /* For normal commands, we'll add them to history below */
        }

        free(trimmed); /* done with trimmed copy */

        /* Add non-empty commands to history */
        history_add(cmdline);

        /* Tokenize */
        arglist = tokenize(cmdline);
        if (arglist != NULL) {
            int exec_result = execute(arglist);
            /* free arglist memory */
            for (int i = 0; arglist[i] != NULL; i++) {
                free(arglist[i]);
            }
            free(arglist);

            /* If execute returns 0 -> exit requested */
            if (exec_result == 0) {
                free(cmdline);
                break;
            }
        }

        free(cmdline);
    }

    /* cleanup: history strings freed in history implementation if needed */
    return 0;
}
