#include <stdio.h>
#include <string.h>
#include "parser.h"

/**
 * parse_input - Tokenizes a command line into an argv-style array.
 * @line: The raw input string (modified in place by strtok).
 * @argv: Output array of argument pointers, NULL-terminated.
 *
 * Return: Number of arguments parsed (0 if the line was empty).
 */
int parse_input(char *line, char *argv[])
{
    int argc = 0;
    char *token = strtok(line, " \t\n");

    while (token != NULL && argc < MAX_ARGS - 1) {
        argv[argc++] = token;
        token = strtok(NULL, " \t\n");
    }
    argv[argc] = NULL;
    return argc;
}

/**
 * parse_redirection - Scans argv for <, >, >> tokens and extracts filenames.
 * @argv:  The argument array (modified in place — redirection tokens are
 *         removed so argv contains only the command and its arguments).
 * @redir: Output structure filled with any redirection targets found.
 *
 * Return: 0 on success, -1 if a redirection operator has no filename after it.
 */
int parse_redirection(char *argv[], redirection_t *redir)
{
    redir->infile  = NULL;
    redir->outfile = NULL;
    redir->append  = 0;

    int i = 0;  /* read index  */
    int j = 0;  /* write index */

    while (argv[i] != NULL) {
        if (strcmp(argv[i], ">") == 0) {
            /* Output redirection (truncate) */
            if (argv[i + 1] == NULL) {
                fprintf(stderr, "syntax error: expected filename after >\n");
                return -1;
            }
            redir->outfile = argv[i + 1];
            redir->append  = 0;
            i += 2;  /* skip '>' and the filename */

        } else if (strcmp(argv[i], ">>") == 0) {
            /* Output redirection (append) */
            if (argv[i + 1] == NULL) {
                fprintf(stderr, "syntax error: expected filename after >>\n");
                return -1;
            }
            redir->outfile = argv[i + 1];
            redir->append  = 1;
            i += 2;

        } else if (strcmp(argv[i], "<") == 0) {
            /* Input redirection */
            if (argv[i + 1] == NULL) {
                fprintf(stderr, "syntax error: expected filename after <\n");
                return -1;
            }
            redir->infile = argv[i + 1];
            i += 2;

        } else {
            /* Regular argument — keep it */
            argv[j++] = argv[i++];
        }
    }
    argv[j] = NULL;
    return 0;
}

/**
 * find_pipe - Locates the first '|' token in the argument list.
 * @argv: NULL-terminated argument array.
 *
 * Return: Index of the '|' token, or -1 if no pipe is found.
 */
int find_pipe(char *argv[])
{
    for (int i = 0; argv[i] != NULL; i++) {
        if (strcmp(argv[i], "|") == 0)
            return i;
    }
    return -1;
}
