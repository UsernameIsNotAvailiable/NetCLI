#ifndef _NETCLI_CONTEXT_SKELETON
#define _NETCLI_CONTEXT_SKELETON

/*

    Expose the init function that we delcared inside
    skeleton_init.c so NetCLI can find it.

*/
int context_skeleton_entry(int argc_start,const char *context_name);

#endif