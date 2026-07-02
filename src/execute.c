#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#include "command.h"
#include "redirect.h"
#include "signals.h"
#include "execute.h"

/**
 * execute_command - Forks a child process to run an external command.
 * @cmd:        A fully parsed Command (argv + redirection targets).
 * @background: If true, the parent does not wait for the child;
 *              the child's PID is printed and it runs in the background.
 *
 * The child restores default signal handling, applies any I/O redirection,
 * then calls execvp().  In foreground mode the parent blocks until the
 * child exits; if the child was killed by a signal (e.g. Ctrl+C), a
 * newline is printed so the next prompt appears on a fresh line.
 */
void execute_command(Command *cmd, int background)
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
        /* Child: restore signals, apply redirections, then exec */
        signals_child_reset();
        apply_redirection(cmd);
        execvp(cmd->argv[0], cmd->argv);
        /* execvp only returns on failure */
        perror(cmd->argv[0]);
        exit(EXIT_FAILURE);
    }

    if (background) {
        /* Background: print PID and return immediately */
        printf("[%d]\n", pid);
    } else {
        /* Foreground: wait for the child to finish */
        int status;
        while (waitpid(pid, &status, 0) == -1) {
            if (errno != EINTR)
                break;
        }
    }
}
