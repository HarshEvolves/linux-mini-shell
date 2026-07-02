#ifndef PIPE_H
#define PIPE_H

#include "command.h"

/**
 * execute_pipeline - Runs N commands connected by N-1 pipes.
 * @cmds:     Array of fully parsed Command structures.
 * @num_cmds: Number of commands in the pipeline (>= 1).
 *
 * For N commands, creates N-1 pipes and forks N children:
 *   - First child:   stdout → pipe[0] write end.
 *   - Middle child i: stdin ← pipe[i-1] read end, stdout → pipe[i] write end.
 *   - Last child:    stdin ← pipe[N-2] read end.
 * Each child also applies its own I/O redirection (< / > / >>).
 * The parent closes all pipe file descriptors and waits for every child.
 */
void execute_pipeline(Command cmds[], int num_cmds);

#endif /* PIPE_H */
