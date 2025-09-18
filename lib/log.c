#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdbool.h>

#include <inc/log.h>


bool ShowDebugLogs = false;

static void ncli_print_va(
    const char *tag,
    const char *func,
    const char *color,
    const char *fmt,
    va_list args
) {
    char buffer[2048];
    vsnprintf(buffer, sizeof(buffer), fmt, args);

    char prefix[256];
    snprintf(prefix, sizeof(prefix), "%s[%s%s%s]%s ",
             color, func, TAG_FUNC_SEPERATOR, tag, "\x1b[0m");

    const char *p = buffer;
    while (*p) {
        fputs(prefix, stdout);

        const char *nl = strchr(p, '\n');
        if (nl) {
            fwrite(p, 1, nl - p + 1, stdout);
            p = nl + 1;
            if (*p == '\0')
                break;
        } else {
            fputs(p, stdout);
            break;
        }
    }
}

void __impl_ncli_print(
    const char *tag,
    const char *func,
    const char *color,
    const char *fmt, 
    ...
) {
    va_list args;
    va_start(args, fmt);

    ncli_print_va(tag,func,color,fmt,args);

    va_end(args);
}

void __impl_ncli_print_debug(
    const char *tag,
    const char *func,
    const char *color,
    const char *fmt, 
    ...
){
    if(!ShowDebugLogs)
        return;

    va_list args;
    va_start(args, fmt);

    ncli_print_va(tag,func,color,fmt,args);

    va_end(args);
}

void logging_enable_debug(){
    ShowDebugLogs = true;
}

bool is_debug_enabled(void){
    return ShowDebugLogs;
}