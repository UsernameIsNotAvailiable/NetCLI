#ifndef _NETCLI_CONTEXT_DNS
#define _NETCLI_CONTEXT_DNS

// type stuff cause billy decided dns should remain undocumented for some reason
typedef BOOL (WINAPI *DnsFlushResolverCache_ptr)();

int context_dns_entry(int argc_start,const char *context_name);
int servers(void);

#endif 