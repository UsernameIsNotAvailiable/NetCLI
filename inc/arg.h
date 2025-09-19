#ifndef __NETCLI_ARG
#define __NETCLI_ARG
#include <stdbool.h>

bool does_arg_exist(
    const char *arg,
    /* out */ int* index
);

bool does_arg_exist_i(
    const char *arg,
    /* out */ int* index
);

#endif