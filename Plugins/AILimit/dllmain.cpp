#include <windows.h>
#include "AILimitPlugin.h"
#include "Log.h"

HINSTANCE g_hInst = nullptr;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        g_hInst = (HINSTANCE)hModule;
        DisableThreadLibraryCalls(hModule);
        AIDebugLog(L"DLL_PROCESS_ATTACH pid=%lu module=%p", GetCurrentProcessId(), hModule);
    }
    return TRUE;
}

extern "C" __declspec(dllexport) ITMPlugin* TMPluginGetInstance() {
    return &CAILimitPlugin::Instance();
}
