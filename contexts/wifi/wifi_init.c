#ifndef _NETCLI_CONTEXT_WIFI
#define _NETCLI_CONTEXT_WIFI

#include <Windows.h>
#include <stdlib.h>
#include <stdio.h>

#include <inc/context.h>
#include <inc/log.h>
#include <inc/context_commands.h>


int list(void);

void wifi_context_usage(void){
    ncli_info(
        "usage: %s wifi { command | help }\n"
        "\n"
        "COMMAND            DESCRIPTION            \n"
        "  list               Find and list networks.\n"
        ,__argv[0]
    );
}

int context_wifi_entry(int argc_start,const char *context_name){
    context_entry(context_name);

    if(__argv[argc_start + 1] == NULL){
        wifi_context_usage();
        exit(0);
    }

    switch(get_command(__argv[argc_start + 1])){
        case WIFI_COMMANDID_LIST:
            list();
            break;

        case COMMAND_HELP:
            wifi_context_usage();
            exit(0);
            break;
        
        case COMMAND_NONE:
        
        default:
            ncli_error("no args specified to context::%s\n",context_name);
            wifi_context_usage();
    }

    return 0;
}

#endif