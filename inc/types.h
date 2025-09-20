#ifndef __NETCLI_TYPES
#define __NETCLI_TYPES

typedef void (*context_entry_fn)(int,const char*);

struct netcli_context_t{
    const char *name;
    context_entry_fn entry;
};


#endif