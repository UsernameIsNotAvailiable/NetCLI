#ifndef _NETCLI_COMMANDS
#define _NETCLI_COMMANDS

// global
#define COMMAND_NONE 0x0
#define COMMAND_HELP 0x1


#define WIFI_COMMANDID_LIST 0xa1


#define NETWORK_COMMANDID_ADAPTERS                  0xb1
#define NETWORK_COMMANDID_CONNECTIONS               0xb2
#define NETWORK_COMMANDID_PROTOCOLS                 0xb3
#define NETWORK_COMMANDID_ROUTETABLES               0xb4

#define RADIO_COMMANDID_WIFI                        0xc1
#define     RADIO_COMMANDID_WIFI_OFF                0xc2
#define     RADIO_COMMANDID_WIFI_ON                 0xc3
#define     RADIO_COMMANDID_WIFI_STATUS             0xc4
#define     RADIO_COMMANDID_WIFI_RECONNECT          0xc5

#define GENERAL_COMMANDID_HOSTNAMES                 0xd1

struct netcli_command_t{
    const char *command;
    int value;
};

static const struct netcli_command_t commands_list[] = {
    {"help",COMMAND_HELP},
    {"list",WIFI_COMMANDID_LIST},
    {"adapters",NETWORK_COMMANDID_ADAPTERS},
    {"connections",NETWORK_COMMANDID_CONNECTIONS},
    {"protocols",NETWORK_COMMANDID_PROTOCOLS},
    {"routetables",NETWORK_COMMANDID_ROUTETABLES},

    {"wifi",RADIO_COMMANDID_WIFI},
        {"on",RADIO_COMMANDID_WIFI_ON},
        {"off",RADIO_COMMANDID_WIFI_OFF},
        {"status",RADIO_COMMANDID_WIFI_STATUS},
        {"reconnect",RADIO_COMMANDID_WIFI_RECONNECT},

    {"hostnames",GENERAL_COMMANDID_HOSTNAMES},
};
#define COMMAND_COUNT (sizeof(commands_list)/sizeof(commands_list[0]))

int get_command(const char *cmd);

#endif