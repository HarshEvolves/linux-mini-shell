#ifndef BUILTIN_H
#define BUILTIN_H

/* Return codes for execute_builtin() */
#define BUILTIN_OK    0   /* command executed, continue the loop */
#define BUILTIN_EXIT  1   /* "exit" was called, break the loop   */

/**
 * is_builtin - Checks whether a command name is a built-in.
 * @cmd: The command name (argv[0]).
 *
 * Return: 1 if cmd is a built-in (cd, exit, history), 0 otherwise.
 */
int is_builtin(const char *cmd);

/**
 * execute_builtin - Dispatches and runs a built-in command.
 * @argv: NULL-terminated argument list.
 *
 * Return: BUILTIN_OK to continue the shell loop,
 *         BUILTIN_EXIT to signal the loop should terminate.
 */
int execute_builtin(char *argv[]);

/**
 * builtin_cleanup - Frees all resources held by the builtin module.
 *
 * Must be called before the shell exits to avoid memory leaks.
 */
void builtin_cleanup(void);

#endif /* BUILTIN_H */
