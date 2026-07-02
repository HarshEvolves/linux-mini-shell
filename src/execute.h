#ifndef EXECUTE_H
#define EXECUTE_H

#include "command.h"

/**
 * execute_command - Forks a child process to run an external command.
 * @cmd:        A fully parsed Command (argv + redirection targets).
 * @background: If true, the parent does not wait for the child;
 *              the child's PID is printed and it runs in the background.
 *
 * The child applies any I/O redirection, then calls execvp().
 * In foreground mode the parent blocks until the child exits.
 */
void execute_command(Command *cmd, int background);

#endif /* EXECUTE_H */
