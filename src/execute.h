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

#endif /* EXECUTE_H */
