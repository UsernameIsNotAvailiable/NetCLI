#include <Windows.h>
#include <stdio.h>

#include <inc/log.h>
#include <inc/context.h>
#include <inc/context_commands.h>

#include "init.h"

void general_context_usage(void){

    ncli_info(
        "usage: %s general { command | help }\n"
        "\n"
        "COMMAND            DESCRIPTION                 \n"
        "  hostnames          Tells you the hostname in \n"
        "                     every hostname format.    \n"
        "  state              Tells you the name and flags\n"
        "                     Of the current connection.\n"
        ,__argv[0]
    );

}

void hostnames(void){
    char buffer[256];
    char description[8][32] = {
        "NetBIOS", "DNS hostname", "DNS domain",
        "DNS fully-qualified", "Physical NetBIOS",
        "Physical DNS hostanme", "Physical DNS domain",
        "Physical DNS fully-qualified"
    };
    int i = 0;
    DWORD dwSize = _countof(buffer);

    for(i = 0; i < ComputerNameMax; i++){
        if(!GetComputerNameExA((COMPUTER_NAME_FORMAT)i,buffer,&dwSize)){
            ncli_error("GetComputerNameEx failed: %d\n",GetLastError());
            exit(1);
        } else {
            printf("%s: %s\n",description[i],buffer);
        }

        dwSize = _countof(buffer);
        ZeroMemory(buffer,dwSize);
    }
}

int context_general_entry(int argc_start,const char *context_name){
    context_entry(context_name);

    if(__argv[argc_start + 1] == NULL){
        general_context_usage();
        exit(0);
    }

    switch(get_command(__argv[argc_start + 1])){
        case GENERAL_COMMANDID_HOSTNAMES:
            hostnames();
            exit(0);
            break;

        case GENERAL_COMMANDID_STATE:
            state();
            exit(0);
            break;

        case COMMAND_HELP:
            general_context_usage();
            exit(0);
            break;


        case COMMAND_NONE:

        default:
            //ncli_error("no args or unknown args specified to context::%s\n",context_name);
            general_context_usage();
    }

   return 0;
}