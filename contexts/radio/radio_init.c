#include <Windows.h>
#include <wlanapi.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <inc/context.h>
#include <inc/log.h>
#include <inc/context_commands.h>

#pragma comment(lib,"wlanapi.lib")

#define COLOR_ENABLED "\033[38;2;106;252;150m"
#define COLOR_DISABLED "\033[38;2;252;121;106m"
#define COLOR_CLEAR "\033[0m"

// WIFI ENABLE & DISABLE
#define WLAN_ERR(x,e) \
    ncli_error("%s: %d\n",x,e) 

bool get_wifi_interface_guid(GUID* guid){
    DWORD err = 0;
    HANDLE hClient = NULL;
    DWORD version = 2;

    err = WlanOpenHandle(version,NULL,&version,&hClient);
    if(err != ERROR_SUCCESS){
        WLAN_ERR("failed to open wlan handle",err);
        exit(1);
    }

    PWLAN_INTERFACE_INFO_LIST p_interface_list = NULL;
    err = WlanEnumInterfaces(hClient, NULL, &p_interface_list);
    if(err != ERROR_SUCCESS){
        WLAN_ERR("failed to enum wlan interfaces",err);
        WlanCloseHandle(hClient,NULL);
        exit(1);
    }

    bool found = false;

    for(DWORD i = 0; i < p_interface_list->dwNumberOfItems; i++){
        WLAN_INTERFACE_INFO info = p_interface_list->InterfaceInfo[i];
        if(info.isState != wlan_interface_state_not_ready){
            *guid = info.InterfaceGuid;
            found = true;
            break;
        }
    }

    if(p_interface_list){
        WlanFreeMemory(p_interface_list);
    }
    WlanCloseHandle(hClient,NULL);
    if(!found){
        ncli_error("no active interfaces found");
    }

    return found;
}

bool toggle_wifi_radio(GUID interface_guid,bool enable){
    DWORD err = 0;
    HANDLE hClient = NULL;
    DWORD version = 2;

    err = WlanOpenHandle(version,NULL,&version,&hClient);
    if(err != ERROR_SUCCESS){
        WLAN_ERR("failed to open wlan handle",err);
        return false;
    }

    WLAN_PHY_RADIO_STATE radio_state;
    radio_state.dwPhyIndex = 0;
    radio_state.dot11SoftwareRadioState = enable ? dot11_radio_state_on : dot11_radio_state_off;

    err = WlanSetInterface(
        hClient,
        &interface_guid,
        wlan_intf_opcode_radio_state,
        sizeof(WLAN_PHY_RADIO_STATE),
        (PVOID)&radio_state,
        NULL
    );

    WlanCloseHandle(hClient,NULL);

    if(err != ERROR_SUCCESS){
        WLAN_ERR("couldn't set interface radio state data",err);
        return false;
    }

    return true;
}

void wifi_radio_on(void){
    GUID interface_guid = {0};
    if(!get_wifi_interface_guid(&interface_guid)){
        ncli_error("couldn't get an interface guid...\n");
    }

    if(toggle_wifi_radio(interface_guid,true)){
        printf(COLOR_ENABLED"enabled"COLOR_CLEAR" Wi-Fi\n");
    } else {
        ncli_error(COLOR_DISABLED"failed"COLOR_CLEAR" to enable Wi-Fi\n");
    }
}

void wifi_radio_off(void){
    GUID interface_guid = {0};
    if(!get_wifi_interface_guid(&interface_guid)){
        ncli_error("couldn't get an interface guid...\n");
    }

    if(toggle_wifi_radio(interface_guid,false)){
        printf(COLOR_DISABLED"disabled"COLOR_CLEAR" Wi-Fi\n");
    } else {
        ncli_error(COLOR_DISABLED"failed"COLOR_CLEAR" to disable Wi-Fi\n");
    }
}

void wifi_radio_reconnect(void){
    GUID interface_guid = {0};

    printf("reconnecting Wi-Fi...\n");
    wifi_radio_off();
    Sleep(2000);
    wifi_radio_on();
}

bool wifi_get_status(void){
    DWORD err = 0;
    HANDLE hClient = NULL;
    DWORD version = 2;
    PWLAN_INTERFACE_INFO_LIST p_interface_list = NULL;
    bool is_connected = false;

    err = WlanOpenHandle(version, NULL, &version, &hClient);
    if (err != ERROR_SUCCESS) {
        WLAN_ERR("failed to open wlan handle", err);
        return false;
    }

    err = WlanEnumInterfaces(hClient, NULL, &p_interface_list);
    if (err != ERROR_SUCCESS) {
        WLAN_ERR("failed to enum wlan interfaces", err);
        WlanCloseHandle(hClient, NULL);
        return false;
    }

    for (DWORD i = 0; i < p_interface_list->dwNumberOfItems; i++) {
        WLAN_INTERFACE_INFO info = p_interface_list->InterfaceInfo[i];
        if (info.isState == wlan_interface_state_connected) {
            is_connected = true;
            break;
        }
    }

    if (p_interface_list) {
        WlanFreeMemory(p_interface_list);
    }
    WlanCloseHandle(hClient, NULL);

    return is_connected;
}

void radio_context_usage(void){
    ncli_info(
        "usage: %s radio { command | help }\n"
        "\n"
        "COMMAND            DESCRIPTION            \n"
        "  wifi               Group of commands.\n"
        "    on               Turns Wi-Fi on.\n"
        "    off              Turns Wi-Fi off.\n"
        "    status           Tells you if Wi-Fi is\n"
        "                     enabled or disabled. \n"
        "    reconnect        Turns Wi-Fi off then waits\n"
        "                     two seconds before turning\n"
        "                     Wi-Fi back on."
        ,__argv[0]
    );
}

void show_wifi_radio_status(){
    printf("Wi-Fi is %s"COLOR_CLEAR"\n",
        wifi_get_status() ? COLOR_ENABLED"enabled" : COLOR_DISABLED"disabled"
    );
}

void radio_command(int argc_start){
    if(__argv[argc_start + 2] == NULL){
        show_wifi_radio_status();
        exit(0);
    }
    switch(get_command(__argv[argc_start + 2])){

        case RADIO_COMMANDID_WIFI_OFF:
            wifi_radio_off();
            exit(0);
            break;

        case RADIO_COMMANDID_WIFI_ON:
            wifi_radio_on();
            exit(0);
            break;

        case RADIO_COMMANDID_WIFI_RECONNECT:
            wifi_radio_reconnect();
            exit(0);
            break;

        case RADIO_COMMANDID_WIFI_STATUS:
        case COMMAND_NONE:
            show_wifi_radio_status();
            exit(0);
    }
}

int context_radio_entry(int argc_start,const char *context_name){
    context_entry(context_name);

    if(__argv[argc_start + 1] == NULL){
        radio_context_usage();
        exit(0);
    }

    switch(get_command(__argv[argc_start + 1])){

        case RADIO_COMMANDID_WIFI:
            radio_command(argc_start);

        case COMMAND_HELP:
            radio_context_usage();
            exit(0);
            break;
        
        case COMMAND_NONE:
        
        default:
            ncli_error("no args specified to context::%s\n",context_name);
            wifi_context_usage();
    }

    return 0;
}