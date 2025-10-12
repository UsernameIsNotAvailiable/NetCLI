#include <WinSock2.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <WbemCli.h>
#include <WbemIdl.h>
#include <stdint.h>
#include <netioapi.h>

#include <inc/error.h>
#include <inc/log.h>

#include "init.h"

#pragma comment(lib,"wbemuuid.lib")
#pragma comment(lib,"Ole32.lib")
#pragma comment(lib,"OleAut32.lib")

#define MIDLN_NONE "\xE2\x94\x82"
#define MIDLN "\xE2\x94\x9C\xE2\x94\x80 "
#define BTMLN "\xE2\x94\x94\xE2\x94\x80 "

#define OK 0
#define DV_CLR_OK          "\033[38;2;72;224;112m"

#define PROBLEM 1
#define DV_CLR_PROBLEM     "\033[38;2;237;115;50m"

#define BAD 2
#define DV_CLR_BAD         "\033[38;2;230;83;83m"

#define DV_CLR_RESET       "\033[0m"

#define ndbg ncli_debug

#define CONCAT_IMPL(x,y) x##y
#define CONCAT(a, b) CONCAT_IMPL(a,b)

#define UNIQUE_VAR(x) CONCAT(name,__LINE__)

#define STRINGIFY(x) #x

#define HRSTR(hr) "(HRESULT="
#define hr_error(hr,msg,...) ncli_error("%s (HRESULT=0x%09X)\n",msg, hr)
#define _release(obj) obj->lpVtbl->Release(obj)
#define _getpcls(which) \
    HRESULT UNIQUE_VAR(hr) = pclsObj->lpVtbl->Get(pclsObj,which,0,&vtProp,0,0); \
    if(SUCCEEDED(UNIQUE_VAR(hr)))

#define _vclear VariantClear(&vtProp)
#define _vinit  VariantInit(&vtProp)

#define _setstatus(typ,str) \
    switch(typ){ \
        case OK: \
            sprintf(buffer,DV_CLR_OK""str""DV_CLR_RESET); \
            break; \
        case PROBLEM: \
            sprintf(buffer,DV_CLR_PROBLEM""str""DV_CLR_RESET); \
            break; \
        case BAD: \
            sprintf(buffer,DV_CLR_BAD""str""DV_CLR_RESET); \
            break; \
    } \

static inline const char* strip_firstN(const char *str,size_t n){
    if (!str) return "";
    size_t len = strlen(str);
    return (len <= n) ? "" : str + n;
}

static inline void _setstatus_buf(char* buffer, int typ, const char* str){
    ndbg("_setstatus_buf(%p,%d,%s)\n",buffer,typ,str);
    switch(typ){
        case OK:     
            sprintf(buffer, DV_CLR_OK"%s"DV_CLR_RESET, str); 
            break;

        case PROBLEM:
            sprintf(buffer, DV_CLR_PROBLEM"%s"DV_CLR_RESET, str); 
            break;

        case BAD:    
            sprintf(buffer, DV_CLR_BAD"%s"DV_CLR_RESET, str);
            break;
    }
}

#define _switch_setstatus(typ,x) \
    case x: \
        _setstatus(typ,STRINGIFY(x)); \
        break; \

//#define _switch_setstatus_spec(typ,x) _switch_setstatus(typ,strip_firstN(STRINGIFY(x),7))

#define _switch_setstatus_spec(typ,x) \
    case x: { \
        _setstatus_buf(buffer,typ,strip_firstN(STRINGIFY(x),7)); \
    } break;

void helper_print_time(const char* label, SYSTEMTIME st) {
    printf(MIDLN"%s: %04d-%02d-%02d %02d:%02d:%02d UTC\n",
           label,
           st.wYear, st.wMonth, st.wDay,
           st.wHour, st.wMinute, st.wSecond);
}

#define _hlp_status_enum_ex(name,enm) \
    if(_stricmp(status,STRINGIFY(name)) == 0) return enm \

#define _hlp_status_enum(x) _hlp_status_enum_ex(x,x)


struct config_mgr_translate_tbl_t{
    const char *text;
    int type;
};

static const struct config_mgr_translate_tbl_t config_mgr_translate[] = {
    {"This device is working properly.", OK},                                 
    {"This device is not configured correctly.", BAD},                        
    {"Windows cannot load the driver for this device.", BAD},                 
    {"The driver might be corrupted, or the system is low on memory/resources.", BAD},
    {"Device is not working properly. One of its drivers or the registry might be corrupted.", BAD},
    {"The driver requires a resource that Windows cannot manage.", BAD},      
    {"The boot configuration conflicts with other devices.", BAD},            
    {"Cannot filter.", BAD},                                                  
    {"Driver loader for the device is missing.", BAD},                        
    {"Firmware is incorrectly reporting the resources for this device.", BAD},
    {"This device cannot start.", BAD},                                        
    {"This device failed.", BAD},                                              
    {"Cannot find enough free resources to use.", BAD},                        
    {"Windows cannot verify the device's resources.", BAD},                    
    {"Device cannot work properly until the computer is restarted.", BAD},     
    {"Device is not working properly due to a possible re-enumeration problem.", BAD}, 
    {"Windows cannot identify all of the resources that the device uses.", BAD}, 
    {"Device is requesting an unknown resource type.", BAD},                   
    {"Reinstall the drivers for this device.", BAD},                           
    {"Failure using the VxD loader.", BAD},                                    
    {"Registry might be corrupted.", BAD},                                     
    {"System failure: Windows is removing this device.", BAD},                 
    {"This device is disabled.", BAD},                                         
    {"System failure: Try changing the driver or see the hardware documentation.", BAD}, 
    {"Device is not present, not working properly, or missing drivers.", BAD}, 
    {"Windows is still setting up this device (first time).", OK},             
    {"Windows is still setting up this device (second time).", OK},            
    {"Device does not have a valid log configuration.", BAD},                  
    {"Device drivers are not installed.", BAD},                                
    {"Device is disabled. Firmware did not provide the required resources.", BAD}, 
    {"Device is using an IRQ resource that another device is using.", BAD},    
    {"Device is not working properly. Windows cannot load the required device drivers.", BAD} 
};

const struct config_mgr_translate_tbl_t *config_mgr_lookup_error(uint32_t code){
    static const struct config_mgr_translate_tbl_t unknown = {"Unknown ConfigManagerErrorCode.", BAD};
    if (code < sizeof(config_mgr_translate)/sizeof(config_mgr_translate[0]))
        return &config_mgr_translate[code];
    return &unknown;
}


enum DeviceStatus helper_get_status_as_enum(char *status){
    ndbg("wmi statuts input is \"%s\"\n",*status); // prints "wmi status input is "(null)""
    if (_stricmp(status, "OK") == 0) return status_OK;
    if (_stricmp(status, "Error") == 0) return status_Error;
    if (_stricmp(status, "Degraded") == 0) return status_Degraded;
    if (_stricmp(status, "Unknown") == 0) return status_Unknown;
    if (_stricmp(status, "Pred Fail") == 0) return status_Prep_Fail;
    if (_stricmp(status, "Starting") == 0) return status_Starting;
    if (_stricmp(status, "Stopping") == 0) return status_Stopping;
    if (_stricmp(status, "Service") == 0) return status_Service;
    if (_stricmp(status, "Stressed") == 0) return status_Stressed;
    if (_stricmp(status, "NonRecover") == 0) return status_NonRecover;
    if (_stricmp(status, "No Contact") == 0) return status_No_Contact;
    if (_stricmp(status, "Lost Comm") == 0) return status_Lost_Comm;

    return status_Unknown; // Default return to avoid missing return value
}

void show_config(IWbemServices* pSvc,int dev_id){
    HRESULT hr;

    wchar_t query[128];
    swprintf(query,128,L"SELECT * FROM Win32_NetworkAdapterConfiguration WHERE Index = %d",dev_id);

    IEnumWbemClassObject* pEnumCfg = NULL;
    hr = pSvc->lpVtbl->ExecQuery(pSvc,binstr(L"WQL"),binstr(query),
                                 WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                                 NULL, &pEnumCfg);

    if(FAILED(hr)){
        hr_error(hr,"query failed under show_config segment...");
        return;
    }

    IWbemClassObject* pclsObj = NULL;
    ULONG retCfg = 0;

    hr = pEnumCfg->lpVtbl->Next(pEnumCfg, WBEM_INFINITE, 1, &pclsObj, &retCfg);
    if(retCfg){
        VARIANT vtProp;
        BOOL dhcp_enabled = FALSE;

        _vinit;
        _getpcls(L"DatabasePath"){
            printf(MIDLN"Internet Database File(s): %ls\n",vtProp.bstrVal);

            _vclear;
        }

        _vinit;
        _getpcls(L"DHCPEnabled"){
            dhcp_enabled = vtProp.boolVal;

            printf(MIDLN"DHCP Enabled: %s"DV_CLR_RESET"\n",vtProp.boolVal ? DV_CLR_OK"yes" : DV_CLR_BAD"no");

            _vclear;
        }

        if(dhcp_enabled){
            ndbg("dhcp is enabled, showing...\n");

            _vinit;
            _getpcls(L"DHCPServer"){
                printf(MIDLN"DHCP Server: %ls\n",vtProp.bstrVal);
                
                _vclear;
            }

            _vinit;
            _getpcls(L"DNSDomain"){
                if (vtProp.vt == VT_NULL || vtProp.vt == VT_EMPTY) {
                    printf(MIDLN"DNS Domain: "DV_CLR_PROBLEM"<none>"DV_CLR_RESET"\n");
                } else {
                    printf(MIDLN"DNS Domain: %ls\n", vtProp.bstrVal);
                }

                _vclear;
            }
        }

        _vinit;
        _getpcls(L"DNSHostName"){
            if (vtProp.vt != VT_NULL && vtProp.vt != VT_EMPTY)
                printf(MIDLN"DNS Hostname: %ls\n", vtProp.bstrVal);

            _vclear;
        }

        _vinit;
        _getpcls(L"DNSDomainSuffixSearchOrder"){
            if(vtProp.vt & VT_ARRAY){
                SAFEARRAY* psa = vtProp.parray;
                BSTR* pbstr;
                SafeArrayAccessData(psa,(void HUGEP**)&pbstr);

                LONG lBound, uBound;
                SafeArrayGetLBound(psa,1,&lBound);
                SafeArrayGetUBound(psa,1,&uBound);

                for(LONG i = lBound; i <= uBound; i++){
                    if(i == lBound){
                        printf(MIDLN"DNS Suffix Search Order: %ls\n",pbstr[i]);
                    } else {
                        printf(MIDLN_NONE"                           %ls\n",pbstr[i]);
                    }
                }

                SafeArrayUnaccessData(psa);
            }

            _vclear;
        }

        _vinit;
        _getpcls(L"DNSServerSearchOrder"){
            if(vtProp.vt & VT_ARRAY){
                SAFEARRAY* psa = vtProp.parray;
                BSTR* pbstr;
                SafeArrayAccessData(psa,(void HUGEP**)&pbstr);

                LONG lBound, uBound;
                SafeArrayGetLBound(psa,1,&lBound);
                SafeArrayGetUBound(psa,1,&uBound);

                for(LONG i = lBound; i <= uBound; i++){
                    if(i == lBound){
                        printf(MIDLN"DNS Server Search Order: %ls\n",pbstr[i]);
                    } else {
                        printf(MIDLN_NONE"                           %ls\n",pbstr[i]);
                    }
                }

                SafeArrayUnaccessData(psa);
            }
            
            _vclear;
        }

        _vinit;
        _getpcls(L"IPEnabled"){
            printf(MIDLN"IP Enabled: %s"DV_CLR_RESET"\n",vtProp.boolVal ? DV_CLR_OK"yes" : DV_CLR_BAD"no");

            _vclear;
        }

        _vinit;
        _getpcls(L"IPFilterSecurityEnabled"){
            printf(MIDLN"IP Filter Security Enabled: %s"DV_CLR_RESET"\n",vtProp.boolVal ? DV_CLR_OK"yes" : DV_CLR_BAD"no");
            
            _vclear;
        }

        _vinit;
        _getpcls(L"DefaultIPGateway"){
            if((vtProp.vt & VT_ARRAY) && vtProp.parray){
                SAFEARRAY* psa = vtProp.parray;
                BSTR* pbstr;
                SafeArrayAccessData(psa,(void HUGEP**)&pbstr);

                LONG lBound, uBound;
                SafeArrayGetLBound(psa,1,&lBound);
                SafeArrayGetUBound(psa,1,&uBound);

                for(LONG i = lBound; i <= uBound; i++){
                    if(i == 0){
                        printf(MIDLN     "Default Gateway: %ls\n",pbstr[i]);
                    } else {
                        printf(MIDLN_NONE"                   %ls\n",pbstr[i]);
                    }
                }

                SafeArrayUnaccessData(psa);

                _vclear;
            } else {
                printf(MIDLN"Default Gateway: "DV_CLR_PROBLEM"<none>"DV_CLR_RESET"\n");
            }
        }

        _vinit;
        _getpcls(L"DeadGWDetectEnabled"){
            printf(MIDLN"Detect Dead Gateways: %s"DV_CLR_RESET"\n",vtProp.boolVal ? DV_CLR_OK"yes" : DV_CLR_BAD"no");

            _vclear;
        }

        _vinit;
        _getpcls(L"IPAddress"){
            if((vtProp.vt & VT_ARRAY) && vtProp.parray){
                SAFEARRAY* psa = vtProp.parray;
                BSTR* pbstr;
                SafeArrayAccessData(psa,(void HUGEP**)&pbstr);

                LONG lBound, uBound;
                SafeArrayGetLBound(psa,1,&lBound);
                SafeArrayGetUBound(psa,1,&uBound);

                for(LONG i = lBound; i <= uBound; i++){
                    if(i == 0){
                        printf(MIDLN     "IP Addresses: %ls\n",pbstr[i]);
                    } else {
                        printf(MIDLN_NONE"                %ls\n",pbstr[i]);
                    }
                }

                SafeArrayUnaccessData(psa);

                _vclear;
            } else {
                printf(MIDLN"IP Addresses: "DV_CLR_PROBLEM"<none>"DV_CLR_RESET"\n");
            }
            
            _vclear;
        }

        _vinit;
        _getpcls(L"IPSubnet"){
            if((vtProp.vt & VT_ARRAY) && vtProp.parray){
                SAFEARRAY* psa = vtProp.parray;
                BSTR* pbstr;
                SafeArrayAccessData(psa,(void HUGEP**)&pbstr);

                LONG lBound, uBound;
                SafeArrayGetLBound(psa,1,&lBound);
                SafeArrayGetUBound(psa,1,&uBound);

                for(LONG i = lBound; i <= uBound; i++){
                    if(i == 0){
                        printf(MIDLN     "IP Subnet(s): %ls\n",pbstr[i]);
                    } else {
                        printf(MIDLN_NONE"                %ls\n",pbstr[i]);
                    }
                }

                SafeArrayUnaccessData(psa);

                _vclear;
            } else {
                printf(MIDLN"IP Subnet(s): "DV_CLR_PROBLEM"<none>"DV_CLR_RESET"\n");
            }
            
            _vclear;
        }

        // TCP

        _vinit;
        _getpcls(L"TcpMaxConnectRetransmissions"){
            printf(MIDLN"TCP Maximum Connect Retransmissions: %d\n",vtProp.intVal);

            _vclear;
        }

        _vinit;
        _getpcls(L"TcpMaxDataRetransmissions"){
            printf(MIDLN"TCP Maximum Data Retransmissions: %d\n",vtProp.intVal);

            _vclear;
        }

        _vinit;
        _getpcls(L"TcpNumConnections"){
            printf(BTMLN"TCP Maximum Connections: %d\n",vtProp.intVal);

            _vclear;
        }

        _release(pclsObj);
    } else {
        ncli_error("retCfg=%lu something failed... triggering hr_error()\n");\
        hr_error(hr,"for information");
    }
    _release(pEnumCfg);
}

void show_rx_and_tx(uint32_t if_index){
    PIP_ADAPTER_ADDRESSES addresses = NULL;
    ULONG out_len = 15000;
    DWORD ret = 0;

    addresses = (IP_ADAPTER_ADDRESSES *)malloc(out_len);
    if(addresses == NULL){
        ncli_error("failed to allocate memory for IP_ADAPTER_ADDRESSES...\n");
        return;
    }

    ret = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL, addresses, &out_len);

    if(ret == ERROR_BUFFER_OVERFLOW){
        free(adapters);
        addresses = (IP_ADAPTER_ADDRESSES *)malloc(out_len);
        if(addresses == NULL){
            ncli_error("failed to allocate memory for IP_ADAPTER_ADDRESSES after overflow...\n");
            return;
        }

        ret = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_INCLUDE_PREFIX, NULL, addresses, &out_len);
    }

    if(ret == NO_ERROR){
        PIP_ADAPTER_ADDRESSES adapter = addresses;
        while(adapter){
            if(adapter->IfIndex == if_index){
                printf(MIDLN"RX: %.04f kb/s \n",adapter->ReceiveLinkSpeed * 1000);
                printf(MIDLN"TX: %.04f kb/s \n",adapter->TransmitLinkSpeed * 1000);

                break;
            }
            adapter = adapter->Next;
        }
    } else {
        ncli_error("GetAdaptersAddresses failed: 0x%09X\n",GetLastError());
    }

    if(addresses){
        free(addresses);
    }
}

int adapters(void){
    HRESULT hr;

    hr = CoInitializeEx(0,COINIT_MULTITHREADED);
    if(FAILED(hr)){
        hr_error(hr,"Failed to initialize COM library!");
        exit(1);
    }
    ndbg("initialized COM...\n");

    hr = CoInitializeSecurity(NULL,-1,NULL,NULL,RPC_C_AUTHN_LEVEL_DEFAULT,
                              RPC_C_IMP_LEVEL_IMPERSONATE,NULL,EOAC_NONE,NULL);
    
    if(FAILED(hr) && hr != RPC_E_TOO_LATE){ // we don't care about RPC_E_TOO_LATE
        hr_error(hr,"Failed to initialize COM security!");
        CoUninitialize();
        exit(1);
    }
    ndbg("initialized COM security...\n");

    IWbemLocator *pLoc = NULL;

    hr = CoCreateInstance(&CLSID_WbemLocator,0,CLSCTX_INPROC_SERVER,&IID_IWbemLocator,(LPVOID*)&pLoc);
    if(FAILED(hr)){
        hr_error(hr,"Failed to create IWbemLocator!");
        CoUninitialize();
        exit(1);
    }
    ndbg("created IWbemLocator via CoCreateInstance()...\n");

    BSTR Server = binstr(L"ROOT\\CIMV2");
    IWbemServices* pSvc = NULL;

    ndbg("connecting to WMI resource: %ls...\n",Server);
    hr = pLoc->lpVtbl->ConnectServer(pLoc,Server,NULL,NULL,0,0,0,0,&pSvc);
    if(FAILED(hr)){
        hr_error(hr,"Failed to connect to the WMI namespace!");
        _release(pLoc);
        CoUninitialize();
        exit(1);
    }

    if(SUCCEEDED(hr)){
        ndbg("connected to the WMI resource!\n");
    }

    hr = CoSetProxyBlanket((IUnknown*)pSvc,RPC_C_AUTHN_WINNT,RPC_C_AUTHZ_NONE,
                           NULL,RPC_C_AUTHN_LEVEL_CALL,RPC_C_IMP_LEVEL_IMPERSONATE,NULL,EOAC_NONE);

    if(FAILED(hr)){
        hr_error(hr,"Failed to set proxy blanket!");
        _release(pSvc);
        _release(pLoc);
        CoUninitialize();
    }
    if(SUCCEEDED(hr)){
        ndbg("set proxy blanket\n");
    }

    IEnumWbemClassObject *pEnum;
    BSTR query = binstr(L"SELECT * FROM Win32_NetworkAdapter");

    ndbg("executing query: \"%ls\"...\n",query);
    hr = pSvc->lpVtbl->ExecQuery(pSvc,binstr(L"WQL"),query,
                                 WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                                 NULL,&pEnum);
    
    if(FAILED(hr)){
        hr_error(hr,"Failed to execute query!");
        _release(pSvc);
        _release(pLoc);
        CoUninitialize();
        exit(1);
    }
    if(SUCCEEDED(hr)){
        ndbg("query success!\n");
    }

    IWbemClassObject* pclsObj = NULL;
    ULONG uret = 0;
    while(pEnum){
        hr = pEnum->lpVtbl->Next(pEnum,WBEM_INFINITE,1,&pclsObj,&uret);
        if(0 == uret){
            break;
        }

        VARIANT vtProp;
        int dev_id = -1;
        uint32_t if_index;
        
        _vinit;ndbg("found an entity!\n");
        _getpcls(L"Name"){
            printf("%ls\n",vtProp.bstrVal);

            _vclear;
        }

        _vinit;
        _getpcls(L"Manufacturer"){
            printf(MIDLN"Manufacturer: %ls\n",vtProp.bstrVal);

            _vclear;
        }

        _vinit;
        _getpcls(L"PNPDeviceID"){
            printf(MIDLN"PNP Device ID: %ls\n",vtProp.bstrVal);
            
            _vclear;
        }

        _vinit;
        _getpcls(L"ServiceName"){
            printf(MIDLN"Service name: %ls\n",vtProp.bstrVal);

            _vclear;
        }

        _vinit;
        _getpcls(L"MACAddress"){
            printf(MIDLN"MAC Address: %ls\n",vtProp.bstrVal);

            _vclear;
        }

        _vinit;
        _getpcls(L"DeviceID"){
            dev_id = _wtoi(vtProp.bstrVal);
            printf(MIDLN"Device ID: %ls\n",vtProp.bstrVal);
            
            _vclear;
        }

        _vinit;
        _getpcls(L"AdapterType"){
            printf(MIDLN"Adapter type: %ls\n",vtProp.bstrVal);

            _vclear;
        }

        _vinit;
        _getpcls(L"GUID"){
            printf(MIDLN"GUID: %ls\n",vtProp.bstrVal);

            _vclear;
        }

        _vinit;
        _getpcls(L"PhysicalAdapter"){
            printf(MIDLN"Physical adapter: %s"DV_CLR_RESET"\n",vtProp.boolVal ? DV_CLR_OK"yes" : DV_CLR_BAD"no");

            _vclear;
        }

        _vinit;
        _getpcls(L"NetEnabled"){
            printf(MIDLN"Enabled: %s"DV_CLR_RESET"\n",vtProp.boolVal ? DV_CLR_OK"yes" : DV_CLR_BAD"no");

            _vclear;
        }

        _vinit;
        _getpcls(L"NetConnectionStatus"){
            UINT status = vtProp.uintVal;
            char *buffer = malloc(128);

            switch((enum ConnectionStatus)status){
                case Disconnected: // special handler
                    if(vtProp.vt == 1)
                        sprintf(buffer,DV_CLR_PROBLEM"<unknown>"DV_CLR_RESET);
                        break;
                    
                    sprintf(buffer,DV_CLR_BAD"Disconnected"DV_CLR_RESET);
                    break;

                _switch_setstatus(OK,               Connecting)
                _switch_setstatus(OK,               Connected)
                _switch_setstatus(PROBLEM,          Disconnecting)
                _switch_setstatus(PROBLEM,          HardwareNotPresent)
                _switch_setstatus(BAD,              HardwareDisabled)
                _switch_setstatus(BAD,              HardwareMalfunction)
                _switch_setstatus(PROBLEM,          MediaDisconnected)
                _switch_setstatus(OK,               Authenticating)
                _switch_setstatus(OK,               AuthenticationSucceeded)
                _switch_setstatus(BAD,              AuthenticationFailed)
                _switch_setstatus(BAD,              InvalidAddress)
                _switch_setstatus(PROBLEM,          CredentialsRequired)
            }
            printf(MIDLN"Connection status: %s\n",buffer);

            free(buffer);

            _vclear;
        }

        _vinit;
        _getpcls(L"ConfigManagerErrorCode"){
            int code = vtProp.uintVal;
            char *buffer = malloc(1024);

            const struct config_mgr_translate_tbl_t *info = config_mgr_lookup_error(code);

            sprintf(buffer,"%s%s (%d)"DV_CLR_RESET,
                info->type == OK ? DV_CLR_OK : DV_CLR_BAD,
                info->text,
                code);

            printf(MIDLN"Config manager error code: %s\n",buffer);

            free(buffer);
            _vclear;
        }

        uint32_t if_index;
        _vinit;
        _getpcls(L"InterfaceIndex"){
            if_index = vtProp.uintVal;
        }

        show_rx_and_tx(if_index);

        ndbg("showing Win32_NetworkAdapterConfiguration...\n");
        show_config(pSvc,dev_id);
        
        printf("\n");
        _release(pclsObj);
    }

    _release(pEnum);
    _release(pSvc);
    _release(pLoc);
    CoUninitialize();
    exit(0);
}