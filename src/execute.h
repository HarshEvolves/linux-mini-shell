#ifndef EXECUTE_H
#define EXECUTE_H

#include "command.h"

/**
 * execute_command - Forks a child process to run an external command.
 * @cmd: A fully parsed Command (argv + redirection targets).
 *
 * The child applies any I/O redirection, then calls execvp().
 * The parent waits for the child to finish.
 */
void execute_command(Command *cmd);

#endif /* EXECUTE_H */
