#include "Log.h"
#include <cstdio>
#include <cstdarg>

#ifdef AILIMIT_DEBUG_LOG
void AIDebugLog(const wchar_t* fmt, ...) {
    wchar_t dir[MAX_PATH];
    GetTempPathW(MAX_PATH, dir);
    wchar_t path[MAX_PATH];
    swprintf_s(path, L"%sAILimit_debug_%lu.log", dir, GetCurrentProcessId());
    FILE* f = nullptr;
    _wfopen_s(&f, path, L"a, ccs=UTF-8");
    if (!f) return;
    va_list ap;
    va_start(ap, fmt);
    vfwprintf(f, fmt, ap);
    va_end(ap);
    fwprintf(f, L"\n");
    fclose(f);
}
#endif
