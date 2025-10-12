#ifndef _NETCLI_CONTEXT_DNS
#define _NETCLI_CONTEXT_DNS

typedef BOOL (WINAPI *DnsFlushResolverCache_ptr)();

int context_dns_entry(int argc_start,const char *context_name);

#endif