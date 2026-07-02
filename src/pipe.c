#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include "command.h"
#include "redirect.h"
#include "signals.h"
#include "pipe.h"

/**
 * close_all_pipes - Closes every file descriptor in the pipe array.
 * @pipefd:    Array of pipe fd pairs.
 * @num_pipes: Number of pipes (N-1 for N commands).
 */
static void close_all_pipes(int pipefd[][2], int num_pipes)
{
    for (int i = 0; i < num_pipes; i++) {
        close(pipefd[i][0]);
        close(pipefd[i][1]);
    }
}

/**
 * run_child - Wires pipe fds, applies redirection, and exec's a command.
 * @cmd:       The Command to execute.
 * @index:     Position of this command in the pipeline (0-based).
 * @num_cmds:  Total number of commands.
 * @pipefd:    Array of pipe fd pairs.
 * @num_pipes: Number of pipes.
 *
 * This function never returns — it either replaces the process via
 * execvp() or exits on failure.
 *
 * Wiring rules:
 *   - If not the first command:  stdin  ← pipefd[index-1] read end.
 *   - If not the last command:   stdout → pipefd[index] write end.
 *   - All pipe fds are closed after dup2().
 *   - File redirection (< / > / >>) is applied after pipe wiring,
 *     allowing e.g. the last command to redirect output to a file.
 */
static void run_child(const Command *cmd, int index, int num_cmds,
                       int pipefd[][2], int num_pipes)
{
    /* Wire stdin from previous pipe (skip for first command) */
    if (index > 0)
        dup2(pipefd[index - 1][0], STDIN_FILENO);

    /* Wire stdout to next pipe (skip for last command) */
    if (index < num_cmds - 1)
        dup2(pipefd[index][1], STDOUT_FILENO);

    /* Close all pipe fds — the child only uses the dup'd copies */
    close_all_pipes(pipefd, num_pipes);

    /* Restore default signal handling for the child */
    signals_child_reset();

    /* Apply any file-based I/O redirection */
    apply_redirection(cmd);

    /* Replace this process with the command */
    execvp(cmd->argv[0], cmd->argv);
    perror(cmd->argv[0]);
    exit(EXIT_FAILURE);
}

/**
 * execute_pipeline - Runs N commands connected by N-1 pipes.
 * @cmds:     Array of fully parsed Command structures.
 * @num_cmds: Number of commands in the pipeline (>= 1).
 *
 * Execution flow:
 *   1. Create N-1 pipes.
 *   2. Fork N children, each wired to the appropriate pipe ends.
 *   3. Parent closes all pipe file descriptors.
 *   4. In foreground mode, parent waits for every child.
 *      In background mode, parent returns immediately.
 *
 * For a single command (num_cmds == 1), no pipes are created and
 * the command is simply forked and exec'd.
 */
void execute_pipeline(Command cmds[], int num_cmds, int background)
{
    int num_pipes = num_cmds - 1;
    int pipefd[MAX_PIPELINE][2];
    pid_t pids[MAX_PIPELINE];

    /* --- Step 1: create all pipes --------------------------------------- */
    for (int i = 0; i < num_pipes; i++) {
        if (pipe(pipefd[i]) < 0) {
            perror("pipe");
            /* Close any pipes already created */
            for (int j = 0; j < i; j++) {
                close(pipefd[j][0]);
                close(pipefd[j][1]);
            }
            return;
        }
    }

    /* --- Step 2: fork one child per command ------------------------------ */
    for (int i = 0; i < num_cmds; i++) {
        pids[i] = fork();
        if (pids[i] < 0) {
            perror("fork");
            close_all_pipes(pipefd, num_pipes);
            /* Wait for any children already forked */
            for (int j = 0; j < i; j++)
                waitpid(pids[j], NULL, 0);
            return;
        }

        if (pids[i] == 0) {
            /* Child process — wire pipes, apply redirection, exec */
            run_child(&cmds[i], i, num_cmds, pipefd, num_pipes);
            /* run_child never returns */
        }
    }

    /* --- Step 3: parent closes all pipe fds ------------------------------ */
    close_all_pipes(pipefd, num_pipes);

    /* --- Step 4: wait or return ------------------------------------------ */
    if (background) {
        /* Background: print the PID of the last command and return */
        printf("[%d]\n", pids[num_cmds - 1]);
    } else {
        /* Foreground: wait for every child to finish */
        for (int i = 0; i < num_cmds; i++) {
            while (waitpid(pids[i], NULL, 0) == -1) {
                if (errno != EINTR)
                    break;
            }
        }
    }
}
