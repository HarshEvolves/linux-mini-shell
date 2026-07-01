#ifndef PARSER_H
#define PARSER_H

#define MAX_ARGS 64

/**
 * struct redirection - Holds parsed redirection targets for a command.
 * @infile:   Filename for input  redirection (< file),  or NULL.
 * @outfile:  Filename for output redirection (> / >> file), or NULL.
 * @append:   If true, open outfile in append mode (>>); otherwise truncate (>).
 */
typedef struct {
    char *infile;
    char *outfile;
    int   append;
} redirection_t;

/**
 * parse_input - Tokenizes a command line into an argv-style array.
 * @line: The raw input string (modified in place by strtok).
 * @argv: Output array of argument pointers, NULL-terminated.
 *
 * Return: Number of arguments parsed (0 if the line was empty).
 */
int parse_input(char *line, char *argv[]);

/**
 * parse_redirection - Scans argv for <, >, >> tokens and extracts filenames.
 * @argv:  The argument array (modified in place — redirection tokens are
 *         removed so argv contains only the command and its arguments).
 * @redir: Output structure filled with any redirection targets found.
 *
 * Return: 0 on success, -1 if a redirection operator has no filename after it.
 */
int parse_redirection(char *argv[], redirection_t *redir);

/**
 * find_pipe - Locates the first '|' token in the argument list.
 * @argv: NULL-terminated argument array.
 *
 * Return: Index of the '|' token, or -1 if no pipe is found.
 */
int find_pipe(char *argv[]);

#endif /* PARSER_H */
