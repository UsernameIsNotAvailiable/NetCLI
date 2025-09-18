#ifndef _NETCLI_LOG
#define _NETCLI_LOG
#include <stdbool.h>

void __impl_ncli_print(
    const char *tag,
    const char *func,
    const char *color,
    const char *fmt, 
    ...
);

void __impl_ncli_print_debug(
    const char *tag,
    const char *func,
    const char *color,
    const char *fmt, 
    ...
);

void logging_enable_debug(void);
bool is_debug_enabled(void);

#define _COLOR_INFO     "\033[38;2;39;121;214m"
#define _COLOR_WARN     "\033[38;2;237;115;50m"
#define _COLOR_ERROR    "\033[38;2;250;60;60m"
#define _COLOR_DEBUG    "\033[38;2;92;92;92m"

/* the stuff between the tag & function */
#define TAG_FUNC_SEPERATOR "/"

#define ncli_info(fmt, ...) \
    __impl_ncli_print("INFO", __FUNCTION__, _COLOR_INFO, fmt __VA_OPT__(,) __VA_ARGS__)

#define ncli_warn(fmt, ...) \
    __impl_ncli_print("WARN", __FUNCTION__, _COLOR_WARN, fmt __VA_OPT__(,) __VA_ARGS__)

#define ncli_error(fmt, ...) \
    __impl_ncli_print("ERROR", __FUNCTION__, _COLOR_ERROR, fmt __VA_OPT__(,) __VA_ARGS__); \

#define ncli_debug(fmt, ...) \
    __impl_ncli_print_debug("DEBUG", __FUNCTION__, _COLOR_DEBUG, fmt __VA_OPT__(,) __VA_ARGS__) \

#define REMOVED() \
    ncli_warn("***** ATTEMPTED TO CALL A REMOVED FUNCTION *****\n"); \
    ncli_warn("*****        caller="__FUNCTION__"()\n"); \
    ncli_warn("***** ABORTING *****\n"); \
    abort(); \

#define REMOVED_FN(name) \
    void name(void){ \
        REMOVED(); \
    } \

#endif