#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "command.h"
#include "parser.h"
#include "redirect.h"
#include "pipe.h"

/**
 * run_child - Helper that applies redirection and exec's a command.
 * @cmd: A fully parsed Command.
 *
 * This function never returns — it either replaces the process via
 * execvp() or exits on failure.
 */
static void run_child(const Command *cmd)
{
    apply_redirection(cmd);
    execvp(cmd->argv[0], cmd->argv);
    perror(cmd->argv[0]);
    exit(EXIT_FAILURE);
}

/**
 * execute_pipe - Runs two commands connected by a single pipe.
 * @argv: Full argument list containing a '|' token at position @pipe_pos.
 * @pipe_pos: Index of the '|' token in argv.
 *
 * Execution flow:
 *   1. Split argv into left and right halves at the '|' token.
 *   2. Validate both sides are non-empty.
 *   3. Parse each side into a Command (argv + redirection).
 *   4. Create the pipe.
 *   5. Fork the left child  — stdout → pipe write end.
 *   6. Fork the right child — stdin  → pipe read end.
 *   7. Parent closes both pipe ends and waits for both children.
 */
void execute_pipe(char *argv[], int pipe_pos)
{
    /* --- Step 1: split argv at the pipe token ---------------------------- */
    argv[pipe_pos] = NULL;
    char **left_argv  = argv;               /* tokens before '|' */
    char **right_argv = &argv[pipe_pos + 1]; /* tokens after  '|' */

    /* --- Step 2: validate both sides ------------------------------------ */
    if (left_argv[0] == NULL || right_argv[0] == NULL) {
        fprintf(stderr, "syntax error: invalid pipe\n");
        return;
    }

    /* --- Step 3: parse each side into a Command ------------------------- */
    Command left_cmd, right_cmd;
    if (parse_command(left_argv, &left_cmd) != 0)
        return;
    if (parse_command(right_argv, &right_cmd) != 0)
        return;

    /* --- Step 4: create the pipe ---------------------------------------- */
    int pipefd[2];
    if (pipe(pipefd) < 0) {
        perror("pipe");
        return;
    }

    /* --- Step 5: left child — writes to the pipe ------------------------ */
    pid_t pid_left = fork();
    if (pid_left < 0) {
        perror("fork");
        close(pipefd[0]);
        close(pipefd[1]);
        return;
    }

    if (pid_left == 0) {
        /* Map stdout to the pipe write end */
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);
        run_child(&left_cmd);  /* does not return */
    }

    /* --- Step 6: right child — reads from the pipe ---------------------- */
    pid_t pid_right = fork();
    if (pid_right < 0) {
        perror("fork");
        close(pipefd[0]);
        close(pipefd[1]);
        waitpid(pid_left, NULL, 0);
        return;
    }

    if (pid_right == 0) {
        /* Map stdin to the pipe read end */
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);
        run_child(&right_cmd);  /* does not return */
    }

    /* --- Step 7: parent — close pipe, wait for both children ------------ */
    close(pipefd[0]);
    close(pipefd[1]);
    waitpid(pid_left, NULL, 0);
    waitpid(pid_right, NULL, 0);
}
