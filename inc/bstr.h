#ifndef __NETCLI_BSTR
#define __NETCLI_BSTR

#include <Windows.h>

BSTR binstr(const OLECHAR *str);

char* BSTRToCharArray(BSTR bstr);

SYSTEMTIME FtToSys(FILETIME ft);
int WmiTime2SysTime(const char* wmi_datetime, SYSTEMTIME* st);

#endif