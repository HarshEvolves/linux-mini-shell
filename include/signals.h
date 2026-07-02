#ifndef SIGNALS_H
#define SIGNALS_H

/**
 * signals_init - Installs all shell signal handlers.
 *
 * Must be called once before the shell loop starts.
 *   - SIGINT:  ignored by the shell (Ctrl+C does not kill the shell).
 *   - SIGCHLD: reaps finished background children with waitpid(WNOHANG).
 */
void signals_init(void);

/**
 * signals_child_reset - Restores default signal dispositions for a child.
 *
 * Must be called in every forked child process before execvp() so that
 * the executed program receives signals normally (e.g. SIGINT from Ctrl+C).
 */
void signals_child_reset(void);

#endif /* SIGNALS_H */
