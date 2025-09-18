#ifndef _NETCLI_CONTEXT_NETWORK
#define _NETCLI_CONTEXT_NETWORK

#include <inc/context.h>
#include <inc/bstr.h>

enum ConnectionStatus {
    Disconnected = 0,
    Connecting = 1,
    Connected = 2,
    Disconnecting = 3,
    HardwareNotPresent = 4,
    HardwareDisabled = 5,
    HardwareMalfunction = 6,
    MediaDisconnected = 7,
    Authenticating = 8,
    AuthenticationSucceeded = 9,
    AuthenticationFailed = 10,
    InvalidAddress = 11,
    CredentialsRequired = 12
};

/*
OK
Error
Degraded
Unknown
Prep_Fail
Starting
Stopping
Service
Stressed
NonRecover
No_Contact
Lost_Comm
*/

enum DeviceStatus {
    status_OK,
    status_Error,
    status_Degraded,
    status_Unknown,
    status_Prep_Fail,
    status_Starting,
    status_Stopping,
    status_Service,
    status_Stressed,
    status_NonRecover,
    status_No_Contact,
    status_Lost_Comm
};

struct tcp_statistics {
    DWORD established;
    DWORD listening;
    DWORD time_wait;
    DWORD close_wait;
    DWORD closed;
};

int context_network_entry(int argc_start,const char *context_name);
int adapters(void);
int connections(void);
int protocols(void);

#endif