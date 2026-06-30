#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_ARGS 64
#define PROMPT "myshell$ "

/**
 * parse_input - Tokenizes a command line into an argv-style array.
 * @line: The raw input string (modified in place by strtok).
 * @argv: Output array of argument pointers, NULL-terminated.
 *
 * Return: Number of arguments parsed (0 if the line was empty).
 */
static int parse_input(char *line, char *argv[])
{
    int argc = 0;
    char *token = strtok(line, " \t\n");

    while (token != NULL && argc < MAX_ARGS - 1) {
        argv[argc++] = token;
        token = strtok(NULL, " \t\n");
    }
    argv[argc] = NULL;
    return argc;
}

/**
 * builtin_cd - Changes the current working directory.
 * @argv: Argument list where argv[1] is the target directory.
 *
 * If no argument is given, changes to the HOME directory.
 * Runs in the parent process (no fork) so the directory change persists.
 */
static void builtin_cd(char *argv[])
{
    const char *dir = argv[1];

    /* Default to HOME if no argument is provided */
    if (dir == NULL) {
        dir = getenv("HOME");
        if (dir == NULL) {
            fprintf(stderr, "cd: HOME not set\n");
            return;
        }
    }

    if (chdir(dir) != 0)
        perror("cd");
}

/**
 * execute_command - Forks a child process to run an external command.
 * @argv: NULL-terminated argument list (argv[0] is the program name).
 *
 * The parent waits for the child to finish. If execvp() fails in the
 * child, an error is printed via perror() and the child exits.
 */
static void execute_command(char *argv[])
{
    pid_t pid = fork();

    if (pid < 0) {
        /* fork() failed */
        perror("fork");
        return;
    }

    if (pid == 0) {
        /* Child: replace this process with the requested command */
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
 * main - Entry point for the mini shell.
 *
 * Runs an infinite read-eval-execute loop:
 *   1. Print the prompt.
 *   2. Read one line of input with getline().
 *   3. Skip empty lines.
 *   4. Parse the line into argv tokens.
 *   5. Fork + exec the command.
 *
 * The loop exits gracefully on EOF (Ctrl+D).
 */
int main(void)
{
    char *line = NULL;
    size_t len = 0;
    ssize_t nread;
    char *argv[MAX_ARGS];

    while (1) {
        /* Display the prompt */
        printf("%s", PROMPT);
        fflush(stdout);

        /* Read a line of input; getline() allocates/resizes the buffer */
        nread = getline(&line, &len, stdin);
        if (nread == -1) {
            /* EOF (Ctrl+D) — exit gracefully */
            printf("\n");
            break;
        }

        /* Parse the line into arguments */
        if (parse_input(line, argv) == 0)
            continue;   /* empty input, re-prompt */

        /* Built-in: cd (must run in parent, not a child process) */
        if (strcmp(argv[0], "cd") == 0) {
            builtin_cd(argv);
            continue;
        }

        /* Built-in: exit (break the loop, clean up, and return 0) */
        if (strcmp(argv[0], "exit") == 0)
            break;

        /* Fork and execute the external command */
        execute_command(argv);
    }

    /* Clean up the buffer allocated by getline() */
    free(line);

    return EXIT_SUCCESS;
}
