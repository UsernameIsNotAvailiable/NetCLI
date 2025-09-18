#ifndef _NETCLI_ERROR
#define _NETCLI_ERROR

#include <Windows.h>

#define NETCLI_ERR_BAD_CONTEXT 1

LONG WINAPI netcli_exception_filter(LPEXCEPTION_POINTERS excep);

#define IF_FAILED(fn)\
    int _ret; \
    _ret = fn(); \
    if(_ret != 0) \
        

#endif