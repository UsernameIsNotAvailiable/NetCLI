#include <Windows.h>
#include <WbemCli.h>
#include <WbemIdl.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <inc/context.h>
#include <inc/log.h>
#include <inc/error.h>
#include <inc/context_commands.h>

#include "init.h"

#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")
#pragma comment(lib, "wbemuuid.lib")

void network_context_usage(void){
    ncli_info(
        "usage: %s network { command | help }\n"
        "\n"
        "COMMAND            DESCRIPTION            \n"
        "  adapters           List all adapters.   \n"
        "  connections        List all connections.\n"
        "  protocols          List all protocols.  \n"
        ,__argv[0]
    );
}

REMOVED_FN(routetables);

int context_network_entry(int argc_start,const char *context_name){
    context_entry(context_name);

    if(__argv[argc_start + 1] == NULL){
        network_context_usage();
        exit(0);
    }

    switch(get_command(__argv[argc_start + 1])){
        case NETWORK_COMMANDID_ADAPTERS:
            adapters();
            break;
        
        case NETWORK_COMMANDID_CONNECTIONS:
            connections();
            exit(0);
            break;

        case NETWORK_COMMANDID_PROTOCOLS:
            protocols();
            break;

        case COMMAND_HELP:
            network_context_usage();
            exit(0);
            break;

        case COMMAND_NONE:

        default:
            ncli_error("no args specified to context::%s\n",context_name);
            network_context_usage();
    }

    return 0;
}