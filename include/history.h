#ifndef HISTORY_H
#define HISTORY_H

/**
 * history_add - Records a command string in the history list.
 * @cmd: The raw command string (will be duplicated via strdup).
 *
 * Grows the backing array with realloc() when capacity is reached.
 */
void history_add(const char *cmd);

/**
 * history_print - Prints all stored commands with 1-based line numbers.
 */
void history_print(void);

/**
 * history_free - Frees every stored command string and the array itself.
 *
 * Must be called before the shell exits to avoid memory leaks.
 */
void history_free(void);

#endif /* HISTORY_H */
