#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "redirect.h"

/**
 * apply_redirection - Opens files and redirects stdin/stdout via dup2().
 * @cmd: The parsed command containing redirection targets.
 *
 * Must be called in the child process before execvp().  If any open()
 * or dup2() call fails, an error is printed via perror() and the child
 * process exits immediately.
 */
void apply_redirection(const Command *cmd)
{
    /* Input redirection: open file as read-only and map to stdin */
    if (cmd->input_file != NULL) {
        int fd = open(cmd->input_file, O_RDONLY);
        if (fd < 0) {
            perror(cmd->input_file);
            exit(EXIT_FAILURE);
        }
        dup2(fd, STDIN_FILENO);
        close(fd);
    }

    /* Output redirection: open/create file and map to stdout */
    if (cmd->output_file != NULL) {
        int flags = O_WRONLY | O_CREAT;
        flags |= cmd->append ? O_APPEND : O_TRUNC;

        int fd = open(cmd->output_file, flags, 0644);
        if (fd < 0) {
            perror(cmd->output_file);
            exit(EXIT_FAILURE);
        }
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }
}
