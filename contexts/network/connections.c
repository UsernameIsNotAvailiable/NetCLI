#include <WinSock2.h>
#include <iphlpapi.h>
#include <WS2tcpip.h>
#include <TlHelp32.h>
#include <stdio.h>
#include <Psapi.h>

#include <inc/error.h>
#include <inc/log.h>

#include "init.h"

#pragma comment(lib,"iphlpapi.lib")
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"psapi.lib")

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

#define ARROW "==>"

#define ndbg ncli_debug

#define CLR_PINK "\033[38;2;201;42;180m"
#define CLR_PURPLE "\033[38;2;141;69;214m"
#define CLR_ORANGE "\033[38;2;245;149;98m"
#define CLR_YELLOW "\033[38;2;252;186;3m"
#define CLR_BLUE "\033[38;2;31;219;240m"
#define CLR_GREEN "\033[38;2;66;245;147m"
#define CLR_RED "\033[38;2;255;145;145m"

#define CLR_TCP "\033[38;2;48;79;225m"
#define CLR_UDP "\033[38;2;255;48;107m"


char* translate_inet(DWORD port,DWORD addr){
    char* ret_val = (char*)malloc(256);

    sprintf(ret_val,"%u.%u.%u.%u:%u",
        (BYTE)(addr),
        (BYTE)(addr >> 8),
        (BYTE)(addr >> 16),
        (BYTE)(addr >> 24),
        ntohs((u_short)port)
    );


    return ret_val;
}

char* strip_to_last_slash(const char* str) {
    if (str == NULL) {
        return NULL; // Handle null input
    }

    const char* last_slash = strrchr(str, '\\');
    if (last_slash == NULL) {
        char* result = strdup(str);
        return result;
    } else {
        char* result = strdup(last_slash + 1);
        return result;
    }
}

char* get_proc_name(DWORD proc_id){
    PROCESSENTRY32 proc;
    proc.dwSize = sizeof(PROCESSENTRY32);

    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    char *proc_name = (char*)malloc(MAX_PATH);
    if(Process32First(hSnap,&proc)){
        do {
            if(proc.th32ProcessID != proc_id)
                continue;
                
            //found it, we can break now >:D
            char* exefile_p = proc.szExeFile;
            strncpy(proc_name, exefile_p, MAX_PATH);
            proc_name[MAX_PATH - 1] = '\0';
            break;
        } while(Process32Next(hSnap,&proc));
    }
    return proc_name;
}


#define BOX_HLINE "\xE2\x94\x80"
#define BOX_VLINE "\xE2\x94\x82"

void bar(int l){
    for(int i = 0; i < l; i++){
        printf(BOX_HLINE);
    }
    printf("\n");
}

/*
OWNER                 STATE           Local Address            Remote Address
Discord.exe           ESTABLISHED     192.168.0.102:56516  ==> 162.159.135.234:443
System                N/A             192.168.124.1:137    ==> N/A


PROTOCOL  OWNER       STATE           Local Address            Remote Address
TCP       Discord.exe ESTABLISHED     192.168.0.102:56516  ==> 162.159.135.234:443
UDP       System      N/A             192.168.124.1:137    ==> N/A
*/

void show_tcp_ip4(struct tcp_statistics *stats){
    PMIB_TCPTABLE_OWNER_PID tcp_tbl;
    DWORD size = 0;
    DWORD ret = 0;
    int i;

    ret = GetExtendedTcpTable(NULL,&size,FALSE,AF_INET,TCP_TABLE_OWNER_PID_ALL,0);

    if(ret != ERROR_INSUFFICIENT_BUFFER){
        ncli_error("a call to GetExtendedTcpTable failed: %d\n",GetLastError());
        exit(1);
    }

    tcp_tbl = (PMIB_TCPTABLE_OWNER_PID)malloc(size);
    if(tcp_tbl == NULL){
        ncli_error("memory alloc failed\n");
        exit(1);
    }

    ret = GetExtendedTcpTable(tcp_tbl,&size,TRUE,AF_INET,TCP_TABLE_OWNER_PID_ALL,0);
    if(ret != NO_ERROR){
        ncli_error("a call to GetExtendedTcpTable failed: %d\n",GetLastError());
    }

    for(DWORD i = 0; i < tcp_tbl->dwNumEntries; i++){
        MIB_TCPROW_OWNER_PID row = tcp_tbl->table[i];

        printf(CLR_TCP"TCP       "DV_CLR_RESET);

        char *proc_name = get_proc_name(row.dwOwningPid);
        if (proc_name == NULL) {
            proc_name = "<unknown>";
        }

        const size_t VISIBLE = 20;
        const size_t ELLIPSIS_LEN = 3;
        size_t pname_len = strlen(proc_name);
        char owner_buf[21];

        if (pname_len > VISIBLE) {
            size_t copy_len = VISIBLE - ELLIPSIS_LEN;
            memcpy(owner_buf, proc_name, copy_len);
            memcpy(owner_buf + copy_len, "...", ELLIPSIS_LEN);
            owner_buf[VISIBLE] = '\0';
        } else {
            memcpy(owner_buf, proc_name, pname_len + 1);
        }

        printf(CLR_BLUE"%-20s  "DV_CLR_RESET, owner_buf);

        if (row.dwOwningPid != 4 && row.dwOwningPid != 0) {
            free(proc_name);
        }

        int scnt = 0;
        int scnt2 = 0;

        char state_label[32] = {0};
        const char *state_color = "";
        switch(row.dwState){
            case MIB_TCP_STATE_CLOSED:
                strcpy(state_label, "CLOSED");
                state_color = CLR_RED;
                stats->closed++;
                break;

            case MIB_TCP_STATE_LISTEN:
                strcpy(state_label, "LISTEN");
                state_color = CLR_GREEN;
                stats->listening++;
                break;

            case MIB_TCP_STATE_SYN_SENT:
                strcpy(state_label, "SYN-SENT");
                state_color = CLR_ORANGE;
                break;

            case MIB_TCP_STATE_SYN_RCVD:
                strcpy(state_label, "SYN-RECEIVED");
                state_color = CLR_ORANGE;
                break;

            case MIB_TCP_STATE_ESTAB:
                strcpy(state_label, "ESTABLISHED");
                state_color = CLR_GREEN;
                stats->established++;
                break;

            case MIB_TCP_STATE_FIN_WAIT1:
                strcpy(state_label, "FIN-WAIT-1");
                state_color = CLR_ORANGE;
                break;

            case MIB_TCP_STATE_FIN_WAIT2:
                strcpy(state_label, "FIN-WAIT-2");
                state_color = CLR_ORANGE;
                break;

            case MIB_TCP_STATE_CLOSE_WAIT:
                strcpy(state_label, "CLOSE-WAIT");
                state_color = CLR_RED;
                stats->close_wait++;
                break;

            case MIB_TCP_STATE_CLOSING:
                strcpy(state_label, "CLOSING");
                state_color = CLR_RED;
                break;

            case MIB_TCP_STATE_LAST_ACK:
                strcpy(state_label, "LAST-ACK");
                state_color = CLR_ORANGE;
                break;

            case MIB_TCP_STATE_TIME_WAIT:
                strcpy(state_label, "TIME-WAIT");
                state_color = CLR_ORANGE;
                stats->time_wait++;
                break;

            case MIB_TCP_STATE_DELETE_TCB:
                strcpy(state_label, "DELETE-TCP");
                state_color = CLR_RED;
                break;

            default:
                strcpy(state_label, "UNKNOWN");
                state_color = "";
                break;
        }

        if(strlen(state_label) < 14)
            scnt2 = 14 - strlen(state_label);

        if (*state_color != '\0')
            printf("%s%s%s", state_color, state_label, DV_CLR_RESET);
        else
            printf("%s", state_label);

        printf("%*s  ", scnt2, " ");

        /* avoid pre-allocating then overwriting (causes leak) */
        char *home = translate_inet(row.dwLocalPort,row.dwLocalAddr);
        char *dest = translate_inet(row.dwRemotePort,row.dwRemoteAddr);

        if(strlen(home) < 20)
            scnt = 20 - strlen(home);

        printf(CLR_PINK"%s"DV_CLR_RESET, home);
        printf(" %*s"ARROW" ",scnt," ");
        printf(CLR_PURPLE"%s"DV_CLR_RESET, dest);

        printf("\n");

        free(home);
        free(dest);
    }
}

void show_udp_ip4(void){
    PMIB_UDPTABLE_OWNER_PID  udp_tbl;
    DWORD size = 0;
    DWORD ret = 0;
    int i;

    ret = GetExtendedUdpTable(NULL,&size,FALSE,AF_INET,UDP_TABLE_OWNER_PID,0);

    if(ret != ERROR_INSUFFICIENT_BUFFER){
        ncli_error("a call to GetExtendedUdpTable failed: %d\n",GetLastError());
        exit(1);
    }

    udp_tbl = (PMIB_UDPTABLE_OWNER_PID)malloc(size);
    if(udp_tbl == NULL){
        ncli_error("memory alloc failed\n");
        exit(1);
    }

    ret = GetExtendedUdpTable(udp_tbl,&size,TRUE,AF_INET,UDP_TABLE_OWNER_PID,0);
    if(ret != NO_ERROR){
        ncli_error("a call to GetExtendedUdpTable failed: %d\n",GetLastError());
    }

    DWORD established =   0;
    DWORD listening   =   0;
    DWORD time_wait   =   0;
    DWORD close_wait  =   0;
    DWORD closed      =   0;

    //printf("OWNER                 Local Address\n");
    //bar(strlen("OWNER                 Local Address") + 5);
    for(DWORD i = 0; i < udp_tbl->dwNumEntries; i++){
        MIB_UDPROW_OWNER_PID row = udp_tbl->table[i];

        printf(CLR_UDP"UDP       "DV_CLR_RESET);

        char *proc_name = get_proc_name(row.dwOwningPid);
        if (proc_name == NULL) {
            proc_name = "<unknown>";
        }

        const size_t VISIBLE = 20;
        const size_t ELLIPSIS_LEN = 3;
        size_t pname_len = strlen(proc_name);
        char owner_buf[21];

        if (pname_len > VISIBLE) {
            size_t copy_len = VISIBLE - ELLIPSIS_LEN;
            memcpy(owner_buf, proc_name, copy_len);
            memcpy(owner_buf + copy_len, "...", ELLIPSIS_LEN);
            owner_buf[VISIBLE] = '\0';
        } else {
            memcpy(owner_buf, proc_name, pname_len + 1);
        }

        printf(CLR_BLUE"%-20s  "DV_CLR_RESET, owner_buf);

        if (row.dwOwningPid != 4 && row.dwOwningPid != 0) {
            free(proc_name);
        }

        printf("%16s"," ");

        /* avoid pre-allocating then overwriting (causes leak) */
        char *home = translate_inet(row.dwLocalPort,row.dwLocalAddr);

        //printf(CLR_PINK"%s"DV_CLR_RESET, home);
        int scnt = 0;
        if(strlen(home) < 20)
            scnt = 20 - strlen(home);

        printf(CLR_PINK"%s"DV_CLR_RESET, home);
        printf(" %*s"ARROW" "CLR_PURPLE"*"DV_CLR_RESET,scnt," ");


        printf("\n");
        free(home);
    }
}

int connections(void){
    // TODO
    struct tcp_statistics stats = {0};
    printf("PROTOCOL  OWNER                 STATE           Local Address            Remote Address\n");
    bar(strlen("PROTOCOL  OWNER                 STATE           Local Address            Remote Address") + 5);
    show_tcp_ip4(&stats);
    show_udp_ip4();

    printf("\n");
    printf(
        CLR_GREEN"Established"DV_CLR_RESET": %d | "CLR_GREEN"Listening"DV_CLR_RESET": %d | "CLR_ORANGE"Time Wait"DV_CLR_RESET": %d | "CLR_RED"Close Wait"DV_CLR_RESET": %d | "CLR_RED"Closed"DV_CLR_RESET": %d",
        stats.established,
        stats.listening,
        stats.time_wait,
        stats.close_wait,
        stats.closed
    );
    return 0;
}