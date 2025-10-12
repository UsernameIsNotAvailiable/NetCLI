#ifndef _NETCLI_RESOLVER
#define _NETCLI_RESOLVER
#include <stdio.h>

bool translate_ip_addr(const char *ip,struct sockaddr_storage *out, int *family);
bool resolve_hostname_to_ip(const char *hostname, struct sockaddr_storage *out, int *family);
bool resolve_hostname(const struct sockaddr *addr, char *out, size_t out_len);
bool resolve_hostname_string(const char *ip, char *out,size_t out_len);

#endif