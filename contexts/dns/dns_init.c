#include <WS2tcpip.h>
#include <string.h>

#include <inc/log.h>
#include <inc/resolver.h>
#include <inc/context_commands.h>
#include <inc/context.h>

#define COLOR_OK "\033[38;2;106;252;150m"
#define COLOR_FAILED "\033[38;2;252;121;106m"
#define COLOR_CLEAR "\033[0m"

#include "init.h"

void print_sockaddr(const struct sockaddr_storage *addr, int family){
    char buf[INET6_ADDRSTRLEN];
    const void *src = (family == AF_INET6)
        ? (const void *)&((struct sockaddr_in *)addr)->sin_addr
        : (const void *)&((struct sockaddr_in6 *)addr)->sin6_addr;

    inet_ntop(family,src,buf,sizeof(buf));
    printf("%s\n",buf);
}

void resolve(const char *hostname){
    struct sockaddr_storage addr;
    int family;

    if(resolve_hostname_to_ip(hostname,&addr,&family)){
        printf("%s ==> ",hostname);
        print_sockaddr(&addr,family);
    } else {
        ncli_error("failed to resolve hostname: %s\n",hostname);
    }
}

void lookup(const char *ip){
    char host[NI_MAXHOST];

    if(resolve_hostname_string(ip,host,sizeof(host))){
        printf("%s ==> %s\n",ip,host);
    } else {
        ncli_error("failed to lookup ip: %s\n",ip);
    }
}

void dns_context_usage(void){
    ncli_info(
        "usage: %s dns { command | help }\n"
        "\n"
        "COMMAND            DESCRIPTION            \n"
        "  resolve            Hostname to IP       \n"
        "  lookup             IP to Hostname       \n"
        "  flush              Uses an undocumented \n"
        "                     function to flush the\n"
        "                     DNS resolver cache   \n"
        ,__argv[0]
    );
}

void flush(void){
    // cause fuckass billy decided to leave DnsFlushResolverCache undocumented
    ncli_debug("loading dnsapi.dll...\n");
    HMODULE dnsapi = LoadLibraryA("dnsapi.dll");
    if(dnsapi == NULL){
        ncli_error("failed to load dnsapi.dll: 0x%09X\n",GetLastError());
        exit(1);
    }
    DnsFlushResolverCache_ptr DnsFlushResolverCache = 
        (DnsFlushResolverCache_ptr)GetProcAddress(dnsapi,"DnsFlushResolverCache");

    if(DnsFlushResolverCache == NULL){
        ncli_error("failed to load DnsFlushResolverCache from dnsapi.dll: 0x%09X\n",GetLastError());
        FreeLibrary(dnsapi);
        exit(2);
    }
    printf("Attempting to flush the DNS resolver cache... ");
    BOOL result = DnsFlushResolverCache();
    if(result){
        printf(COLOR_OK"done"COLOR_CLEAR"\n");
    } else {
        printf(COLOR_OK"done (0x%09X)\n"COLOR_CLEAR,GetLastError());
    }

    ncli_debug("freeing dnsapi.dll...\n");
    FreeLibrary(dnsapi);
    exit(0);
}

int context_dns_entry(int argc_start,const char *context_name){
    context_entry(context_name);

    if(__argv[argc_start + 1] == NULL){
        dns_context_usage();
        exit(0);
    }

    switch(get_command(__argv[argc_start + 1])){
        
        case DNS_COMMANDID_LOOKUP:
            lookup(__argv[argc_start + 2]);
            exit(0);
            break;
        
        case DNS_COMMANDID_RESOLVE:
            resolve(__argv[argc_start + 2]);
            exit(0);
            break;

        case DNS_COMMANID_SERVERS:
            ncli_error("deprecated\n");
            //servers();
            exit(0);
            break;

        case DNS_COMMANDID_FLUSH:
            flush();
            exit(0);
            break;

        case COMMAND_HELP:        
        default:
            dns_context_usage();
            exit(0);
            break;
    }
}
