#ifndef PIPE_H
#define PIPE_H

/**
 * execute_pipe - Runs two commands connected by a single pipe.
 * @argv: Full argument list containing a '|' token at position @pipe_pos.
 * @pipe_pos: Index of the '|' token in argv.
 *
 * Splits argv into left (before '|') and right (after '|') commands.
 * Creates a pipe via pipe(), forks two child processes:
 *   - Left child:  stdout → pipe write end, then execvp().
 *   - Right child: stdin  → pipe read end,  then execvp().
 * Each child also applies any I/O redirection (< / > / >>).
 * The parent closes the pipe and waits for both children.
 */
void execute_pipe(char *argv[], int pipe_pos);

#endif /* PIPE_H */
