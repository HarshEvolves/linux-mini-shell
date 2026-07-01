#ifndef REDIRECT_H
#define REDIRECT_H

#include "parser.h"

/**
 * apply_redirection - Opens files and redirects stdin/stdout via dup2().
 * @redir: The parsed redirection targets.
 *
 * Must be called in the child process before execvp().  If any open()
 * or dup2() call fails, an error is printed via perror() and the child
 * process exits immediately.
 */
void apply_redirection(const redirection_t *redir);

#endif /* REDIRECT_H */
