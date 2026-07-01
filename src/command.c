#include <string.h>
#include "command.h"

/**
 * command_init - Initializes a Command to a clean, empty state.
 * @cmd: The command structure to initialize.
 *
 * Zeros all fields: argv is NULL-terminated at index 0, redirection
 * targets are NULL, and append is false.
 */
void command_init(Command *cmd)
{
    memset(cmd, 0, sizeof(Command));
}

/**
 * command_free - Resets a Command to its initial state.
 * @cmd: The command structure to reset.
 *
 * Since the string pointers reference the line buffer (not separately
 * allocated memory), this function simply re-initializes the structure.
 */
void command_free(Command *cmd)
{
    command_init(cmd);
}
