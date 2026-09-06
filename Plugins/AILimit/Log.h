#pragma once
#include <windows.h>

// AILimit diagnostic log. Compiled out in Release unless AILIMIT_DEBUG_LOG is defined.
// Debug output goes to %TEMP%\AILimit_debug_<pid>.log (no hardcoded user paths).
#ifdef AILIMIT_DEBUG_LOG
void AIDebugLog(const wchar_t* fmt, ...);
#else
inline void AIDebugLog(const wchar_t*, ...) {}
#endif
