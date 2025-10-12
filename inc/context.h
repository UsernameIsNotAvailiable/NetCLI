#ifndef NETCLI_CONTEXT
#define NETCLI_CONTEXT

#define CONTEXT_SIZE 50

// Debug + set context
#define context_entry(name) change_context(name)


// Internal allocation guard
#define check_alloc() \
    do { \
        if (!is_current_context_allocated) allocate(); \
    } while(0)


void change_context(const char *new_context);
const char *get_current_context(void);
void free_context(void);
static BOOL allocate_context(const char *new_context);

#endif // NETCLI_CONTEXT
