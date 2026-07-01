#include <stdlib.h>
#include "shell.h"

/**
 * main - Entry point for the mini shell.
 *
 * Delegates all work to shell_loop() and returns success.
 */
int main(void)
{
    shell_loop();
    return EXIT_SUCCESS;
}
