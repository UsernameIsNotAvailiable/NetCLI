#include <Windows.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <inc/context.h>
#include <inc/log.h>

static char *current_context = NULL;

// Allocate a new context string dynamically
static BOOL allocate_context(const char *new_context) {
    if (!new_context) {
        ncli_debug("[WARN] allocate_context called with NULL\n");
        return FALSE;
    }

    size_t len = strlen(new_context) + 1; // include null terminator
    char *buffer = (char *)malloc(len);
    if (!buffer) {
        ncli_debug("[ERROR] Failed to allocate memory for context\n");
        return FALSE;
    }

    strcpy(buffer, new_context);

    // Free old context if exists
    if (current_context) {
        free(current_context);
    }

    current_context = buffer;
    return TRUE;
}

#define pDbg printf("[CHECK] %d\n",__LINE__)

// Get the current context (empty string if NULL)
const char *get_current_context(void) {
    return current_context ? current_context : "";
}


// Change the current context
void change_context(const char *new_context) {
    if (!new_context) {
        ncli_debug("[WARN] change_context(NULL) ignored\n");
        return;
    }

    ncli_debug("current context is now context::%s\n",new_context);

    if (!allocate_context(new_context)) {
        ncli_debug("[ERROR] change_context failed\n");
    }
}

// Free the context
void free_context(void) {
    if (current_context) {
        free(current_context);
        current_context = NULL;
    }
}
