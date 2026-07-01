#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "redirect.h"

/**
 * apply_redirection - Opens files and redirects stdin/stdout via dup2().
 * @redir: The parsed redirection targets.
 *
 * Must be called in the child process before execvp().  If any open()
 * or dup2() call fails, an error is printed via perror() and the child
 * process exits immediately.
 */
void apply_redirection(const redirection_t *redir)
{
    /* Input redirection: open file as read-only and map to stdin */
    if (redir->infile != NULL) {
        int fd = open(redir->infile, O_RDONLY);
        if (fd < 0) {
            perror(redir->infile);
            exit(EXIT_FAILURE);
        }
        dup2(fd, STDIN_FILENO);
        close(fd);
    }

    /* Output redirection: open/create file and map to stdout */
    if (redir->outfile != NULL) {
        int flags = O_WRONLY | O_CREAT;
        flags |= redir->append ? O_APPEND : O_TRUNC;

        int fd = open(redir->outfile, flags, 0644);
        if (fd < 0) {
            perror(redir->outfile);
            exit(EXIT_FAILURE);
        }
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }
}
