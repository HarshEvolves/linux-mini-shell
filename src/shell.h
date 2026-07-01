#ifndef SHELL_H
#define SHELL_H

/**
 * shell_loop - Runs the main read-eval-execute loop of the shell.
 *
 * Displays the prompt, reads input, dispatches built-in commands
 * (cd, exit, history), and delegates external commands and pipes
 * to the execute module.  Exits on EOF (Ctrl+D) or the "exit" command.
 */
void shell_loop(void);

#endif /* SHELL_H */
