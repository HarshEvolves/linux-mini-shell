#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "command.h"
#include "redirect.h"
#include "execute.h"

/**
 * execute_command - Forks a child process to run an external command.
 * @cmd: A fully parsed Command (argv + redirection targets).
 *
 * The child applies any I/O redirection, then calls execvp().
 * The parent waits for the child to finish.
 */
void execute_command(Command *cmd)
{
    /* Ensure the command has a program to run */
    if (cmd->argv[0] == NULL)
        return;

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return;
    }

    if (pid == 0) {
        /* Child: apply any redirections, then exec */
        apply_redirection(cmd);
        execvp(cmd->argv[0], cmd->argv);
        /* execvp only returns on failure */
        perror(cmd->argv[0]);
        exit(EXIT_FAILURE);
    }

    /* Parent: wait for the child to finish */
    int status;
    waitpid(pid, &status, 0);
}
