#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "parser.h"
#include "execute.h"

/**
 * setup_redirection - Opens files and redirects stdin/stdout via dup2().
 * @redir: The parsed redirection targets.
 *
 * Must be called in the child process before execvp().  If any open()
 * or dup2() call fails, an error is printed and the child exits.
 */
static void setup_redirection(const redirection_t *redir)
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
        /* Child: set up any redirections, then exec */
        setup_redirection(&redir);
        execvp(argv[0], argv);
        /* execvp only returns on failure */
        perror(argv[0]);
        exit(EXIT_FAILURE);
    }

    /* Parent: wait for the child to finish */
    int status;
    waitpid(pid, &status, 0);
}

/**
 * execute_pipeline - Runs two commands connected by a single pipe.
 * @argv: Full argument list containing a '|' token at position @pipe_pos.
 * @pipe_pos: Index of the '|' token in argv.
 *
 * Splits argv into left (argv[0..pipe_pos-1]) and right (argv[pipe_pos+1..])
 * commands.  Creates a pipe, forks two children:
 *   - Left child:  stdout → pipe write end, then exec.
 *   - Right child: stdin  → pipe read end,  then exec.
 * Each child also handles its own I/O redirection (< / > / >>).
 * The parent closes the pipe and waits for both children.
 */
void execute_pipeline(char *argv[], int pipe_pos)
{
    /* Split argv into two commands at the pipe token */
    argv[pipe_pos] = NULL;
    char **left_argv  = argv;               /* command before '|' */
    char **right_argv = &argv[pipe_pos + 1]; /* command after  '|' */

    /* Validate both sides have a command */
    if (left_argv[0] == NULL || right_argv[0] == NULL) {
        fprintf(stderr, "syntax error: invalid pipe\n");
        return;
    }

    /* Parse redirection for each side independently */
    redirection_t left_redir, right_redir;
    if (parse_redirection(left_argv, &left_redir) != 0)
        return;
    if (parse_redirection(right_argv, &right_redir) != 0)
        return;

    /* Create the pipe */
    int pipefd[2];
    if (pipe(pipefd) < 0) {
        perror("pipe");
        return;
    }

    /* --- Left child: writes to the pipe --------------------------------- */
    pid_t pid_left = fork();
    if (pid_left < 0) {
        perror("fork");
        close(pipefd[0]);
        close(pipefd[1]);
        return;
    }

    if (pid_left == 0) {
        /* Redirect stdout to the pipe write end */
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);

        /* Apply any additional file redirection (e.g. < infile) */
        setup_redirection(&left_redir);

        execvp(left_argv[0], left_argv);
        perror(left_argv[0]);
        exit(EXIT_FAILURE);
    }

    /* --- Right child: reads from the pipe -------------------------------- */
    pid_t pid_right = fork();
    if (pid_right < 0) {
        perror("fork");
        close(pipefd[0]);
        close(pipefd[1]);
        waitpid(pid_left, NULL, 0);
        return;
    }

    if (pid_right == 0) {
        /* Redirect stdin to the pipe read end */
        dup2(pipefd[0], STDIN_FILENO);
        close(pipefd[0]);
        close(pipefd[1]);

        /* Apply any additional file redirection (e.g. > outfile) */
        setup_redirection(&right_redir);

        execvp(right_argv[0], right_argv);
        perror(right_argv[0]);
        exit(EXIT_FAILURE);
    }

    /* --- Parent: close pipe and wait for both children ------------------- */
    close(pipefd[0]);
    close(pipefd[1]);
    waitpid(pid_left, NULL, 0);
    waitpid(pid_right, NULL, 0);
}
