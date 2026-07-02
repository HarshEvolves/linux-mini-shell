#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "command.h"
#include "parser.h"
#include "execute.h"
#include "pipe.h"
#include "builtin.h"
#include "history.h"
#include "shell.h"

#define PROMPT "myshell$ "

/* --- Shell loop --------------------------------------------------------- */

/**
 * shell_loop - Runs the main read-eval-execute loop of the shell.
 *
 * Displays the prompt, reads input, dispatches built-in commands
 * (cd, exit, history) via the builtin module, and delegates external
 * commands and pipes to the execute / pipe modules.
 * Exits on EOF (Ctrl+D) or the "exit" command.
 */
void shell_loop(void)
{
    char *line = NULL;
    size_t len = 0;
    ssize_t nread;
    char *argv[MAX_ARGS];

    while (1) {
        /* Display the prompt */
        printf("%s", PROMPT);
        fflush(stdout);

        /* Read a line of input; getline() allocates/resizes the buffer */
        nread = getline(&line, &len, stdin);
        if (nread == -1) {
            /* EOF (Ctrl+D) — exit gracefully */
            printf("\n");
            break;
        }

        /*
         * Save a clean copy of the raw line before parse_input()
         * mutates it with strtok().  Strip the trailing newline
         * so the stored string is tidy.
         */
        char *save = strdup(line);
        if (save != NULL) {
            size_t slen = strlen(save);
            if (slen > 0 && save[slen - 1] == '\n')
                save[slen - 1] = '\0';
        }

        /* Parse the line into arguments */
        if (parse_input(line, argv) == 0) {
            free(save);     /* nothing to record for empty input */
            continue;
        }

        /* Record the command in history (includes builtins) */
        if (save != NULL)
            history_add(save);
        free(save);

        /* Dispatch built-in commands (cd, exit, history) */
        if (is_builtin(argv[0])) {
            if (execute_builtin(argv) == BUILTIN_EXIT)
                break;
            continue;
        }

        /* Check for a pipe and route accordingly */
        int pipe_pos = find_pipe(argv);
        if (pipe_pos >= 0) {
            /* Pipeline: split into Command array and execute */
            Command cmds[MAX_PIPELINE];
            int num_cmds = parse_pipeline(argv, cmds, MAX_PIPELINE);
            if (num_cmds > 0)
                execute_pipeline(cmds, num_cmds);
        } else {
            /* Single command: parse and execute directly */
            Command cmd;
            if (parse_command(argv, &cmd) == 0)
                execute_command(&cmd);
        }
    }

    /* Clean up all allocated memory */
    builtin_cleanup();
    free(line);
}
