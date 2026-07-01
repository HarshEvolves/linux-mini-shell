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
 * parse_command - Builds a Command from a tokenized argument list.
 * @argv: The argument array (scanned for <, >, >> tokens which are
 *        extracted into the Command and removed from argv).
 * @cmd:  Output Command structure populated with argv and redirection.
 *
 * Scans argv with a two-pointer approach: redirection operators and
 * their filenames are stored in cmd->input_file / cmd->output_file,
 * while regular arguments are compacted into cmd->argv.
 *
 * Return: 0 on success, -1 if a redirection operator has no filename.
 */
int parse_command(char *argv[], Command *cmd)
{
    command_init(cmd);

    int i = 0;  /* read index  */
    int j = 0;  /* write index */

    while (argv[i] != NULL) {
        if (strcmp(argv[i], ">") == 0) {
            /* Output redirection (truncate) */
            if (argv[i + 1] == NULL) {
                fprintf(stderr, "syntax error: expected filename after >\n");
                return -1;
            }
            cmd->output_file = argv[i + 1];
            cmd->append      = 0;
            i += 2;

        } else if (strcmp(argv[i], ">>") == 0) {
            /* Output redirection (append) */
            if (argv[i + 1] == NULL) {
                fprintf(stderr, "syntax error: expected filename after >>\n");
                return -1;
            }
            cmd->output_file = argv[i + 1];
            cmd->append      = 1;
            i += 2;

        } else if (strcmp(argv[i], "<") == 0) {
            /* Input redirection */
            if (argv[i + 1] == NULL) {
                fprintf(stderr, "syntax error: expected filename after <\n");
                return -1;
            }
            cmd->input_file = argv[i + 1];
            i += 2;

        } else {
            /* Regular argument — store in cmd->argv */
            cmd->argv[j++] = argv[i++];
        }
    }
    cmd->argv[j] = NULL;
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
