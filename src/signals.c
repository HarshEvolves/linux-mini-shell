#include <signal.h>
#include <sys/wait.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "signals.h"

/* --- Signal handlers ---------------------------------------------------- */

/**
 * sigchld_handler - Reaps finished child processes without blocking.
 * @sig: Signal number (unused).
 *
 * Called automatically when any child process terminates.  Loops over
 * waitpid() with WNOHANG to reap all zombie children that have exited
 * since the last invocation.  Preserves errno so that the interrupted
 * code path is not affected.
 */
static void sigchld_handler(int sig)
{
    (void)sig;
    int saved_errno = errno;

    while (waitpid(-1, NULL, WNOHANG) > 0)
        ;

    errno = saved_errno;
}

/**
 * sigint_handler - Dummy handler for SIGINT.
 * @sig: Signal number (unused).
 *
 * This handler does nothing, but because it is installed without
 * SA_RESTART, it interrupts blocking calls like getline() returning
 * EINTR, allowing the shell to reprint the prompt.
 */
static void sigint_handler(int sig)
{
    (void)sig;
    write(STDOUT_FILENO, "\n", 1);
}

/* --- Public API --------------------------------------------------------- */

/**
 * signals_init - Installs all shell signal handlers.
 *
 *   SIGINT:  Set to SIG_IGN so the shell itself is never killed by Ctrl+C.
 *            Forked children reset this to SIG_DFL via signals_child_reset(),
 *            so they still respond to Ctrl+C normally.
 *
 *   SIGCHLD: Handled by sigchld_handler() which reaps background children.
 *            SA_RESTART prevents getline() and other syscalls from failing
 *            with EINTR.  SA_NOCLDSTOP avoids notifications when a child
 *            is merely stopped (not terminated).
 */
void signals_init(void)
{
    struct sigaction sa;

    /* --- Handle SIGINT in the shell ------------------------------------- */
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0; /* No SA_RESTART, interrupt getline() */
    sigaction(SIGINT, &sa, NULL);

    /* --- Handle SIGCHLD to reap background children --------------------- */
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigaction(SIGCHLD, &sa, NULL);
}

/**
 * signals_child_reset - Restores default signal dispositions for a child.
 *
 * Must be called in every forked child process before execvp().
 * The shell ignores SIGINT and installs a custom SIGCHLD handler —
 * these dispositions are inherited by fork().  This function resets
 * both to SIG_DFL so the executed program behaves normally.
 */
void signals_child_reset(void)
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = SIG_DFL;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGCHLD, &sa, NULL);
}
