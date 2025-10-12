#include <Windows.h>
#include <WS2tcpip.h>
#include <string.h>

#include <inc/log.h>
#include <inc/resolver.h>
#include <inc/context_commands.h>
#include <inc/context.h>

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
        "usage %s dns { command | help }\n"
        "\n"
        "COMMAND            DESCRIPTION            \n"
        "  resolve            IP to hostname       \n"
        "  lookup             Hostname to IP       \n"
        ,__argv[0]
    );
}

int context_dns_entry(int argc_start,const char *context_name){
    context_entry(context_name);

    if(__argv[argc_start + 1] == NULL){
        dns_context_usage();
        exit(0);
    }

    switch(get_command(__argv[argc_start + 1])){
        
        case DNS_COMMANDID_LOOKUP:
        lookup(__argv[argc_start + 1]);
        exit(0);
        break;
        
        case DNS_COMMANDID_RESOLVE:
        resolve(__argv[argc_start + 2]);
        exit(0);
        break;

        case COMMAND_HELP:

        default:
            dns_context_usage();
            exit(0);
            break;
    }
}