#include <stdio.h>
#include <inc/context_commands.h>

#define _CMD(cmdv,ret) \
    if(strcmpi(cmd,cmdv) == 0) \
        return ret \

int get_command(const char *cmd){
    if(cmd == NULL){
        return COMMAND_NONE;
    }
    _CMD("help",COMMAND_HELP);
    
    _CMD("list",WIFI_COMMANDID_LIST);

    _CMD("adapters",NETWORK_COMMANDID_ADAPTERS);
    _CMD("connections",NETWORK_COMMANDID_CONNECTIONS);
    _CMD("protocols",NETWORK_COMMANDID_PROTOCOLS);
    _CMD("routetables",NETWORK_COMMANDID_ROUTETABLES);

    _CMD("wifi",RADIO_COMMANDID_WIFI);
        _CMD("on",RADIO_COMMANDID_WIFI_ON);
        _CMD("off",RADIO_COMMANDID_WIFI_OFF);
        _CMD("status",RADIO_COMMANDID_WIFI_STATUS);
        _CMD("reconnect",RADIO_COMMANDID_WIFI_RECONNECT);

    return COMMAND_NONE;
}