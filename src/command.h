#ifndef COMMAND_H
#define COMMAND_H

#define MAX_ARGS 64

/**
 * struct Command - Represents a single parsed shell command.
 * @argv:        NULL-terminated argument list (pointers into the line buffer).
 * @input_file:  Filename for input  redirection (< file),  or NULL.
 * @output_file: Filename for output redirection (> / >> file), or NULL.
 * @append:      If true, open output_file in append mode (>>); else truncate (>).
 *
 * The string pointers (argv entries, input_file, output_file) all point into
 * the line buffer owned by the shell loop — they are NOT separately allocated.
 */
typedef struct {
    char *argv[MAX_ARGS];
    char *input_file;
    char *output_file;
    int   append;
} Command;

/**
 * command_init - Initializes a Command to a clean, empty state.
 * @cmd: The command structure to initialize.
 */
void command_init(Command *cmd);

/**
 * command_free - Resets a Command to its initial state.
 * @cmd: The command structure to reset.
 *
 * Since the string pointers reference the line buffer (not separately
 * allocated memory), this function simply zeroes the structure.
 */
void command_free(Command *cmd);

#endif /* COMMAND_H */
