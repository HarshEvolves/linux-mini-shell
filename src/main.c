#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_INPUT_LEN 1024
#define MAX_ARGS 64
#define PROMPT "minishell$ "

/**
 * parse_input - Splits the input line into an array of arguments.
 * @line: The raw input string (will be modified in place).
 * @args: Output array of argument pointers.
 *
 * Return: Number of arguments parsed.
 */
static int parse_input(char *line, char *args[])
{
    int argc = 0;
    char *token = strtok(line, " \t\n");

    while (token != NULL && argc < MAX_ARGS - 1) {
        args[argc++] = token;
        token = strtok(NULL, " \t\n");
    }
    args[argc] = NULL;
    return argc;
}

/**
 * builtin_cd - Handle the built-in 'cd' command.
 * @args: Argument list where args[1] is the target directory.
 *
 * Return: 0 on success, -1 on failure.
 */
static int builtin_cd(char *args[])
{
    const char *dir = args[1];

    if (dir == NULL) {
        dir = getenv("HOME");
        if (dir == NULL) {
            fprintf(stderr, "cd: HOME not set\n");
            return -1;
        }
    }

    if (chdir(dir) != 0) {
        perror("cd");
        return -1;
    }
    return 0;
}

/**
 * execute_command - Fork and exec an external command.
 * @args: NULL-terminated argument list.
 *
 * Return: Exit status of the child process, or -1 on fork failure.
 */
static int execute_command(char *args[])
{
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return -1;
    }

    if (pid == 0) {
        /* Child process */
        execvp(args[0], args);
        fprintf(stderr, "%s: %s\n", args[0], strerror(errno));
        exit(EXIT_FAILURE);
    }

    /* Parent process */
    int status;
    waitpid(pid, &status, 0);

    if (WIFEXITED(status))
        return WEXITSTATUS(status);

    return -1;
}

int main(void)
{
    char input[MAX_INPUT_LEN];
    char *args[MAX_ARGS];

    while (1) {
        printf("%s", PROMPT);
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL) {
            /* Handle EOF (Ctrl+D) */
            printf("\n");
            break;
        }

        int argc = parse_input(input, args);
        if (argc == 0)
            continue;

        /* Built-in: exit */
        if (strcmp(args[0], "exit") == 0)
            break;

        /* Built-in: cd */
        if (strcmp(args[0], "cd") == 0) {
            builtin_cd(args);
            continue;
        }

        execute_command(args);
    }

    return EXIT_SUCCESS;
}
