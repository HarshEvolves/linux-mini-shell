#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "parser.h"
#include "redirect.h"
#include "execute.h"

/**
 * execute_command - Forks a child process to run an external command.
 * @argv: NULL-terminated argument list (argv[0] is the program name).
 *
 * Parses any I/O redirection operators from argv, then forks.  The child
 * sets up redirection before calling execvp().  The parent waits for the
 * child to finish.
 */
void execute_command(char *argv[])
{
    redirection_t redir;

    /* Extract redirection operators from argv */
    if (parse_redirection(argv, &redir) != 0)
        return;

    /* Ensure a command remains after stripping redirection tokens */
    if (argv[0] == NULL)
        return;

    pid_t pid = fork();

    if (pid < 0) {
        /* fork() failed */
        perror("fork");
        return;
    }

    if (pid == 0) {
        /* Child: apply any redirections, then exec */
        apply_redirection(&redir);
        execvp(argv[0], argv);
        /* execvp only returns on failure */
        perror(argv[0]);
        exit(EXIT_FAILURE);
    }

    /* Parent: wait for the child to finish */
    int status;
    waitpid(pid, &status, 0);
}
