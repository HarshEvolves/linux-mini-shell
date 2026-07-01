#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "history.h"

#define HISTORY_INIT  16   /* initial history capacity */

/* --- History storage (file-scoped) -------------------------------------- */

static char **history;       /* dynamic array of saved command strings      */
static int    hist_count;    /* number of commands stored                   */
static int    hist_cap;      /* current allocated capacity                  */

/**
 * history_add - Records a command string in the history list.
 * @cmd: The raw command string (will be duplicated via strdup).
 *
 * Grows the backing array with realloc() when capacity is reached.
 */
void history_add(const char *cmd)
{
    /* First call: allocate the initial array */
    if (history == NULL) {
        hist_cap = HISTORY_INIT;
        history = malloc(sizeof(char *) * hist_cap);
        if (history == NULL) {
            perror("malloc");
            return;
        }
    }

    /* Grow the array if full */
    if (hist_count >= hist_cap) {
        hist_cap *= 2;
        char **tmp = realloc(history, sizeof(char *) * hist_cap);
        if (tmp == NULL) {
            perror("realloc");
            return;
        }
        history = tmp;
    }

    history[hist_count] = strdup(cmd);
    if (history[hist_count] == NULL) {
        perror("strdup");
        return;
    }
    hist_count++;
}

/**
 * history_print - Prints all stored commands with 1-based line numbers.
 */
void history_print(void)
{
    for (int i = 0; i < hist_count; i++)
        printf("  %d  %s\n", i + 1, history[i]);
}

/**
 * history_free - Frees every stored command string and the array itself.
 */
void history_free(void)
{
    for (int i = 0; i < hist_count; i++)
        free(history[i]);
    free(history);
    history = NULL;
    hist_count = 0;
    hist_cap = 0;
}
