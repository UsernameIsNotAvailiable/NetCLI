#include <Windows.h>
#include <string.h>
#include <stdio.h>
#include <inc/bstr.h>

BSTR binstr(const OLECHAR *str){
    return SysAllocString(str);
}

char* BSTRToCharArray(BSTR bstr) {
    int len = WideCharToMultiByte(CP_UTF8, 0, bstr, -1, NULL, 0, NULL, NULL);
    char* str = (char*)malloc(len);
    if (str) {
        WideCharToMultiByte(CP_UTF8, 0, bstr, -1, str, len, NULL, NULL);
    }
    return str;
}

SYSTEMTIME FtToSys(FILETIME ft) {
    SYSTEMTIME st;
    FileTimeToSystemTime(&ft, &st);
    return st;
}

int WmiTime2SysTime(const char* wmi_datetime, SYSTEMTIME* st) {
    if (!wmi_datetime || strlen(wmi_datetime) < 25) {
        return 1; // Invalid format
    }

    // WMI format: yyyymmddHHMMSS.ffffffsUTC
    char buf[5]; // Buffer for year, month, etc. (4 digits + null terminator)

    // Year
    strncpy_s(buf, sizeof(buf), wmi_datetime, 4);
    buf[4] = '\0';
    st->wYear = (WORD)atoi(buf);

    // Month
    strncpy_s(buf, sizeof(buf), wmi_datetime + 4, 2);
    buf[2] = '\0';
    st->wMonth = (WORD)atoi(buf);

    // Day
    strncpy_s(buf, sizeof(buf), wmi_datetime + 6, 2);
    buf[2] = '\0';
    st->wDay = (WORD)atoi(buf);

    // Hour
    strncpy_s(buf, sizeof(buf), wmi_datetime + 8, 2);
    buf[2] = '\0';
    st->wHour = (WORD)atoi(buf);

    // Minute
    strncpy_s(buf, sizeof(buf), wmi_datetime + 10, 2);
    buf[2] = '\0';
    st->wMinute = (WORD)atoi(buf);

    // Second
    strncpy_s(buf, sizeof(buf), wmi_datetime + 12, 2);
    buf[2] = '\0';
    st->wSecond = (WORD)atoi(buf);

    // Microseconds to Milliseconds
    strncpy_s(buf, sizeof(buf), wmi_datetime + 15, 6);
    buf[6] = '\0';
    st->wMilliseconds = (WORD)(atoi(buf) / 1000);

    // DayOfWeek is not in the WMI string, can be calculated later
    // st->wDayOfWeek = ...;

    // Handle UTC offset
    int utcOffset = atoi(wmi_datetime + 22);
    if (wmi_datetime[21] == '+') {
        utcOffset = -utcOffset; // Offset is reversed for adjustment
    }

    FILETIME ft;
    SystemTimeToFileTime(st, &ft);

    ULARGE_INTEGER uli;
    uli.LowPart = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;
    uli.QuadPart += (LONGLONG)utcOffset * 60 * 10000000LL; // 60 seconds/min * 10000000 100ns units/sec

    ft.dwLowDateTime = uli.LowPart;
    ft.dwHighDateTime = uli.HighPart;

    FileTimeToSystemTime(&ft, st);

    return 0;
}