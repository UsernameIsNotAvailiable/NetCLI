#include <Windows.h>
#include <stdio.h>
#include <WbemCli.h>
#include <WbemIdl.h>
#include <stdint.h>

#include <inc/bstr.h>
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
#define hr_error(hr,msg,...) ncli_error("%s (HRESULT=0x%08X)\n",msg, hr)
#define _release(obj) obj->lpVtbl->Release(obj)
#define _getpcls(which) \
    HRESULT UNIQUE_VAR(hr) = pclsObj->lpVtbl->Get(pclsObj,which,0,&vtProp,0,0); \
    if(SUCCEEDED(UNIQUE_VAR(hr)))

#define _vclear VariantClear(&vtProp)
#define _vinit  VariantInit(&vtProp)

REMOVED_FN(servers);

int __servers(void){
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

    BSTR Server = binstr(L"root\\microsoftdns");
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
    BSTR query = binstr(L"SELECT * FROM MicrosoftDNS_Server");

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
        printf("hi");
        hr = pEnum->lpVtbl->Next(pEnum,WBEM_INFINITE,1,&pclsObj,&uret);
        if(0 == uret){
            break;
        }

        VARIANT vtProp;

        _vinit;ndbg("found an entity!\n");
        _getpcls(L"Name"){
            printf("%ls\n",vtProp.bstrVal);

            _vclear;
        }
        
        printf("\n");
        _release(pclsObj);
    }

    _release(pEnum);
    _release(pSvc);
    _release(pLoc);
    CoUninitialize();
    exit(0);
}