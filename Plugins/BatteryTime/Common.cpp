#include "pch.h"
#include "Common.h"
#include <sstream>

std::wstring CCommon::TimeFormat(DWORD time)
{
    if (time == (DWORD)-1)
        return L"--";

    int hours = time / 3600;
    int minutes = (time % 3600) / 60;

    std::wstringstream wss;
    if (hours > 0)
        wss << hours << L"h " << minutes << L"m";
    else
        wss << minutes << L"m";

    return wss.str();
}
