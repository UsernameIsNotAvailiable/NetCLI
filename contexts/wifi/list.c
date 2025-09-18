#include <Windows.h>
#include <wlanapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <inc/log.h>

#define PINK "\033[38;2;201;42;180m"
#define PURPLE "\033[38;2;141;69;214m"
#define ORANGE "\033[38;2;245;149;98m"
#define YELLOW "\033[38;2;252;186;3m"
#define BLUE "\033[38;2;31;219;240m"
#define GREEN "\033[38;2;66;245;147m"
#define RED "\033[38;2;255;145;145m"
#define BOLD "\033[1m"
#define RESET "\033[0m"

#pragma comment(lib,"wlanapi.lib")

bool __verbose;
typedef struct{
    bool onlyNames;
    bool noscan;
    bool verbose;
}__options;

char *LastErrorAsString()
{
    char messageBuffer[256];
    char *return_val = {0};

    FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL,GetLastError(),MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),(LPSTR)&messageBuffer,256,NULL);
    strcpy(return_val,messageBuffer);
            
    return return_val;
}

const char algorithms[12][256] = {
    "80211 Open",
    "80211 Shared key",
    "WPA",
    "WPA-PSK",
    "WPA-NONE",
    "RSNA",
    "RSNA-PSK",
    "WPA3",
    "WPA3",
    "WPA3-SAE",
    "OWE",
    "WPA3-ENT"
};

int scan_time = 5000;

int list(void){
    HANDLE clientHandle = NULL;
    DWORD version = 2; // Use WLAN API version 2
    DWORD result;
    int spaces;
    SetConsoleOutputCP(CP_UTF8);


    PWLAN_CONNECTION_ATTRIBUTES pConnectInfo = NULL;
    DWORD connectInfoSize = sizeof(WLAN_CONNECTION_ATTRIBUTES);
    WLAN_OPCODE_VALUE_TYPE opCode = wlan_opcode_value_type_invalid;
    
    // Open a handle to the WLAN service.
    result = WlanOpenHandle(version, NULL, &version, &clientHandle);
    if (result != ERROR_SUCCESS) {
        ncli_error("WlanOpenHandle failed: %d\n", result);
        return 1;
    }
    ncli_debug("A WLAN handle has been created at 0x%p\n",&clientHandle);


    
    // Enumerate available networks.
    PWLAN_INTERFACE_INFO_LIST interfaceList = NULL;
    result = WlanEnumInterfaces(clientHandle,NULL,&interfaceList);
    ncli_debug("A WLAN_INTERFACE_INFO_LIST list has been created at 0x%p\n",&interfaceList);
    PWLAN_INTERFACE_INFO pInfo = NULL;
    ncli_debug("Enumurated available network...\n");

    ncli_info("Scanning for networks for 5 seconds...\n");
    for(int i = 0; i < interfaceList->dwNumberOfItems; i++){
        WlanScan(clientHandle,&interfaceList->InterfaceInfo[i].InterfaceGuid,NULL,NULL,NULL);
        Sleep(scan_time);
    }

    PWLAN_AVAILABLE_NETWORK_LIST networkList = NULL;
    result = WlanGetAvailableNetworkList(clientHandle,&interfaceList->InterfaceInfo->InterfaceGuid, WLAN_AVAILABLE_NETWORK_INCLUDE_ALL_ADHOC_PROFILES, NULL, &networkList);
    ncli_debug("A WLAN_AVAILABLE_NETWORK_LIST has been created at 0x%p\n",&networkList);

    printf("\n");
    printf("SSID%31sPassword%2sConnectable   Signal   Algorithm%10sCipher%10sFlags\n"," "," "," "," ");
    for(int i = 0; i < 125; i++){
        printf("\xE2\x80\x95");
    }
    printf("\n");
    int printed = 0;
    if (result == ERROR_SUCCESS) {
        for (int i = 0; i < networkList->dwNumberOfItems; i++) {
            PWLAN_AVAILABLE_NETWORK network = &networkList->Network[i];
            pInfo = &interfaceList->InterfaceInfo[i];
            if(network->dot11Ssid.uSSIDLength <= 1){
                continue;
            }
            if(network->dot11Ssid.uSSIDLength < 35){
                spaces = 35 - network->dot11Ssid.uSSIDLength;
            }
            printf(BLUE"%s"RESET, network->dot11Ssid.ucSSID);
            printed++;
            for(int i = 0; i < spaces; i++){
                printf(" ");
            }
            //15
            spaces = 0;
            if(network->bSecurityEnabled){
                printf(GREEN"yes"RESET);
                
                spaces = 7;
            } else {
                printf(RED"no"RESET);
                spaces = 8;
            }
            if(network->bNetworkConnectable){
                printf(GREEN"%*syes"RESET,spaces," ");
                spaces = 14;
            } else {
                printf(RED"%*sno"RESET,spaces," ");
                spaces = 15;
            }
            if(network->wlanSignalQuality > 90){
                printf(GREEN);
            } else if(network->wlanSignalQuality > 50){
                printf(YELLOW);
            } else {
                printf(RED);
            }
            printf("%*lu%%     "RESET,spaces,network->wlanSignalQuality);

            if(strlen(algorithms[network->dot11DefaultAuthAlgorithm]) < 19){
                spaces = 19 - strlen(algorithms[network->dot11DefaultAuthAlgorithm]);
            }
            printf(PURPLE"%s"RESET"%*s",algorithms[network->dot11DefaultAuthAlgorithm],spaces," ");
            char t[256];
            switch(network->dot11DefaultCipherAlgorithm){
                case DOT11_CIPHER_ALGO_NONE:
                    sprintf(t,"none");
                    break;
                case DOT11_CIPHER_ALGO_WEP40:
                    sprintf(t,"WEP40"); 
                    break;
                case DOT11_CIPHER_ALGO_TKIP:
                    sprintf(t,"TKIP");
                    break;
                case DOT11_CIPHER_ALGO_CCMP:
                    sprintf(t,"CCMP");
                    break;
                case DOT11_CIPHER_ALGO_WEP104:
                    sprintf(t,"WEP104");
                    break;
                case DOT11_CIPHER_ALGO_WPA_USE_GROUP:
                    sprintf(t,"WPA-USE-GROUP");
                    break;
                case DOT11_CIPHER_ALGO_WEP: 
                    sprintf(t,"WEP");
                    break;
            }
            if(strlen(t) < 16){
                spaces = 16 - strlen(t);
            }
            printf(PINK"%s"RESET"%*s",t,spaces," ");

            int flagCnt = 0;
            if(network->dwFlags &WLAN_AVAILABLE_NETWORK_CONNECTED){
                //9 
                WLAN_CONNECTION_ATTRIBUTES connectAttrib;
                printf(ORANGE"CONNECTED"RESET);
                spaces += 9;
                flagCnt++;
            }
            if(network->dwFlags &WLAN_AVAILABLE_NETWORK_HAS_PROFILE){
                if(flagCnt != 0){
                    //2
                    printf(", ");
                }
                //
                printf(ORANGE"PROFILE"RESET);
                flagCnt++;
            }
            spaces = 0;
            printf("\n");
        }

        WlanFreeMemory(networkList);
        printf("\n");
        ncli_info("Found %d networks!\n",printed);
    } else {
        ncli_error("WlanGetAvailableNetworkList failed: %d\n",result);
    }
    return 0;
}