#ifndef EXECUTE_H
#define EXECUTE_H

/**
 * execute_command - Forks a child process to run an external command.
 * @argv: NULL-terminated argument list (argv[0] is the program name).
 *
 * Parses any I/O redirection operators from argv, then forks.  The child
 * sets up redirection before calling execvp().  The parent waits for the
 * child to finish.
 */
void execute_command(char *argv[]);

/**
 * execute_pipeline - Runs two commands connected by a single pipe.
 * @argv: Full argument list containing a '|' token at position @pipe_pos.
 * @pipe_pos: Index of the '|' token in argv.
 *
 * Splits argv into left and right commands, creates a pipe, forks two
 * children, and waits for both to finish.
 */
void execute_pipeline(char *argv[], int pipe_pos);

#endif /* EXECUTE_H */
