#ifndef PARSER_H
#define PARSER_H

#include "command.h"

/**
 * parse_input - Tokenizes a command line into an argv-style array.
 * @line: The raw input string (modified in place by strtok).
 * @argv: Output array of argument pointers, NULL-terminated.
 *
 * Return: Number of arguments parsed (0 if the line was empty).
 */
int parse_input(char *line, char *argv[]);

/**
 * parse_command - Builds a Command from a tokenized argument list.
 * @argv: The argument array (scanned for <, >, >> tokens which are
 *        extracted into the Command and removed from argv).
 * @cmd:  Output Command structure populated with argv and redirection.
 *
 * Return: 0 on success, -1 if a redirection operator has no filename.
 */
int parse_command(char *argv[], Command *cmd);

/**
 * find_pipe - Locates the first '|' token in the argument list.
 * @argv: NULL-terminated argument array.
 *
 * Return: Index of the '|' token, or -1 if no pipe is found.
 */
int find_pipe(char *argv[]);

#endif /* PARSER_H */
