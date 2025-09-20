#ifndef __NETCLI
#define __NETCLI
#include <inc/compile_time.h>

static inline __impl_netcli_build_getter(void){
    srand(UNIX_TIMESTAMP);
    return rand();
}

#define _NETCLI_RELEASE "1.0.2"
#define _NETCLI_BUILD __impl_netcli_build_getter()


#endif /* __NETCLI */