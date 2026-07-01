#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "builtin.h"
#include "history.h"

/* --- Built-in commands -------------------------------------------------- */

/**
 * builtin_cd - Changes the current working directory.
 * @argv: Argument list where argv[1] is the target directory.
 *
 * If no argument is given, changes to the HOME directory.
 * Runs in the parent process (no fork) so the directory change persists.
 */
static void builtin_cd(char *argv[])
{
    const char *dir = argv[1];

    /* Default to HOME if no argument is provided */
    if (dir == NULL) {
        dir = getenv("HOME");
        if (dir == NULL) {
            fprintf(stderr, "cd: HOME not set\n");
            return;
        }
    }

    if (chdir(dir) != 0)
        perror("cd");
}

/* --- Public API --------------------------------------------------------- */

/**
 * is_builtin - Checks whether a command name is a built-in.
 * @cmd: The command name (argv[0]).
 *
 * Return: 1 if cmd is a built-in (cd, exit, history), 0 otherwise.
 */
int is_builtin(const char *cmd)
{
    return (strcmp(cmd, "cd") == 0 ||
            strcmp(cmd, "exit") == 0 ||
            strcmp(cmd, "history") == 0);
}

/**
 * execute_builtin - Dispatches and runs a built-in command.
 * @argv: NULL-terminated argument list.
 *
 * Return: BUILTIN_OK to continue the shell loop,
 *         BUILTIN_EXIT to signal the loop should terminate.
 */
int execute_builtin(char *argv[])
{
    if (strcmp(argv[0], "cd") == 0) {
        builtin_cd(argv);
        return BUILTIN_OK;
    }

    if (strcmp(argv[0], "exit") == 0)
        return BUILTIN_EXIT;

    if (strcmp(argv[0], "history") == 0) {
        history_print();
        return BUILTIN_OK;
    }

    return BUILTIN_OK;
}

/**
 * builtin_cleanup - Frees all resources held by the builtin module.
 *
 * Delegates to history_free() for history cleanup.
 */
void builtin_cleanup(void)
{
    history_free();
}
