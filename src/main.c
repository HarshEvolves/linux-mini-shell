#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "shell.h"

#define MINISHELL_VERSION "v1.0.0"

void print_usage(void) {
    printf("Linux Mini Shell - A professional open-source Linux CLI application\n\n");
    printf("Usage:\n");
    printf("  minishell [OPTIONS]\n\n");
    printf("Options:\n");
    printf("  -h, --help     Show this help message and exit\n");
    printf("  -v, --version  Show version information and exit\n\n");
    printf("Examples:\n");
    printf("  minishell      # Start the interactive shell\n");
    printf("  minishell -h   # Display help\n\n");
    printf("Author:  Harsh (Red Hat & Qualcomm Engineer)\n");
    printf("GitHub:  https://github.com/HarshEvolves/linux-mini-shell\n");
    printf("Version: %s\n", MINISHELL_VERSION);
}

void print_version(void) {
    printf("Linux Mini Shell %s\n", MINISHELL_VERSION);
}

/**
 * main - Entry point for the mini shell.
 *
 * Handles command line arguments and delegates interactive
 * work to shell_loop().
 */
int main(int argc, char **argv)
{
    if (argc > 1) {
        if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
            print_usage();
            return EXIT_SUCCESS;
        } else if (strcmp(argv[1], "--version") == 0 || strcmp(argv[1], "-v") == 0) {
            print_version();
            return EXIT_SUCCESS;
        } else {
            fprintf(stderr, "minishell: unrecognized option '%s'\n", argv[1]);
            fprintf(stderr, "Try 'minishell --help' for more information.\n");
            return EXIT_FAILURE;
        }
    }

    shell_loop();
    return EXIT_SUCCESS;
}
