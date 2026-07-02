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

/**
 * parse_pipeline - Splits a tokenized argument list at '|' tokens and
 *                  builds an array of Command objects.
 * @argv:     The full tokenized argument list (modified in place — '|'
 *            tokens are replaced with NULL to create segments).
 * @cmds:     Output array of Command structures.
 * @max_cmds: Maximum number of commands the array can hold.
 *
 * Walks argv looking for '|' tokens.  Each segment between pipes is
 * passed to parse_command() which extracts redirection and fills the
 * Command.  Validates that no segment is empty.
 *
 * Return: Number of commands parsed (>= 1), or -1 on error.
 */
int parse_pipeline(char *argv[], Command cmds[], int max_cmds)
{
    int num_cmds = 0;
    int start = 0;

    for (int i = 0; argv[i] != NULL; i++) {
        if (strcmp(argv[i], "|") == 0) {
            /* Replace '|' with NULL to terminate this segment */
            argv[i] = NULL;

            /* Validate: segment must not be empty */
            if (argv[start] == NULL) {
                fprintf(stderr, "syntax error: empty command in pipeline\n");
                return -1;
            }
            if (num_cmds >= max_cmds) {
                fprintf(stderr, "error: too many commands in pipeline\n");
                return -1;
            }

            /* Parse this segment into a Command */
            if (parse_command(&argv[start], &cmds[num_cmds]) != 0)
                return -1;
            num_cmds++;

            /* Next segment starts after the '|' */
            start = i + 1;
        }
    }

    /* Parse the final segment (after the last '|', or the only command) */
    if (argv[start] == NULL) {
        fprintf(stderr, "syntax error: empty command in pipeline\n");
        return -1;
    }
    if (num_cmds >= max_cmds) {
        fprintf(stderr, "error: too many commands in pipeline\n");
        return -1;
    }
    if (parse_command(&argv[start], &cmds[num_cmds]) != 0)
        return -1;
    num_cmds++;

    return num_cmds;
}
