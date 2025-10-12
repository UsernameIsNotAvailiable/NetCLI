
#include <WS2tcpip.h>
#include <ip2string.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <inc/resolver.h>
#include <inc/log.h>

#pragma comment(lib,"ntdll.lib")
#pragma comment(lib, "ws2_32.lib")

#define zero_mem(x) memset(&(x), 0, sizeof(x))

bool translate_ip_addr(const char *ip, struct sockaddr_storage *out, int *family) {
    ncli_debug("translate_ip_addr: %s\n", ip);

    if (!ip || !out)
        return false;

    PCSTR terminator = NULL;
    zero_mem(*out);

    if (strchr(ip, ':')) {
        // IPv6
        struct in6_addr ipv6;
        NTSTATUS status = RtlIpv6StringToAddressA(ip, &terminator, &ipv6);
        if (status != 0)
            return false;

        struct sockaddr_in6 *sa6 = (struct sockaddr_in6 *)out;
        zero_mem(*sa6);

        sa6->sin6_family = AF_INET6;
        sa6->sin6_addr = ipv6;
        if (family)
            *family = AF_INET6;

        return true;
    } else {
        // IPv4
        struct in_addr ipv4;
        NTSTATUS status = RtlIpv4StringToAddressA(ip, FALSE, &terminator, &ipv4);
        if (status != 0)
            return false;

        struct sockaddr_in *sa4 = (struct sockaddr_in *)out;
        zero_mem(*sa4);

        sa4->sin_family = AF_INET;
        sa4->sin_addr = ipv4;
        if (family)
            *family = AF_INET;

        return true;
    }
}

bool resolve_hostname_to_ip(const char *hostname, struct sockaddr_storage *out, int *family) {
    ncli_debug("resolve_hostname_to_ip: %s\n", hostname);

    if (!hostname || !out)
        return false;

    struct addrinfo hints, *res = NULL;
    zero_mem(hints);
    zero_mem(*out);

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_ADDRCONFIG;

    int ret = getaddrinfo(hostname, NULL, &hints, &res);
    if (ret != 0 || !res)
        return false;

    memcpy(out, res->ai_addr, res->ai_addrlen);
    if (family)
        *family = res->ai_family;

    freeaddrinfo(res);
    return true;
}

bool resolve_hostname(const struct sockaddr *addr, char *out, size_t out_len) {
    if (!addr || !out)
        return false;

    int ret = getnameinfo(
        addr,
        (addr->sa_family == AF_INET)
            ? sizeof(struct sockaddr_in)
            : sizeof(struct sockaddr_in6),
        out,
        (DWORD)out_len,
        NULL, 0, NI_NAMEREQD
    );

    return (ret == 0);
}

bool resolve_hostname_string(const char *ip, char *out, size_t out_len) {
    ncli_debug("resolve_hostname_string: %s\n", ip);

    if (!ip || !out)
        return false;

    struct sockaddr_storage addr;
    int family;
    if (!translate_ip_addr(ip, &addr, &family))
        return false;

    return resolve_hostname((struct sockaddr *)&addr, out, out_len);
}
