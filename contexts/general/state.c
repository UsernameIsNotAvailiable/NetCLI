#include <Windows.h>
#include <wininet.h>
#include <stdio.h>

#include <WinDNS.h>

#pragma comment(lib,"Wininet.lib")

#include <inc/log.h>

int state(void){
    
    DWORD flags;
    char *connection_name = malloc(1024);
    if(!InternetGetConnectedStateExA(&flags,connection_name,1024,0)){
        ncli_error("InternetGetConnectedStateExA failed!\n"
                   "This is because there is no internet connection or\n"
                   "all connections are not active." 
                  );
        exit(1);
    }

    printf("Connection name: %s\n",connection_name);

    printf("Flags:\n");
    if(flags &INTERNET_CONNECTION_CONFIGURED)
        printf("    INTERNET_CONNECTION_CONFIGURED\n");

    if(flags &INTERNET_CONNECTION_LAN)
        printf("    INTERNET_CONNECTION_LAN\n");

    if(flags &INTERNET_CONNECTION_MODEM)
        printf("    INTERNET_CONNECTION_MODEM\n");
    
    if(flags &INTERNET_CONNECTION_OFFLINE)
        printf("    INTERNET_CONNECTION_OFFLINE\n");

    if(flags &INTERNET_CONNECTION_PROXY)
        printf("    INTERNET_CONNECTION_PROXY\n");

    free(connection_name);

    exit(0);

    return 0;
}