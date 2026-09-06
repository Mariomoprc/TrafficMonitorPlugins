#include "Config.h"
#include "Utils.h"
#include "Log.h"
#include <windows.h>

void CAIConfig::Load(const std::wstring& path) {
    configPath = path;
    // Encrypted keys: GoKey1Enc, GoKey2Enc, OrKeyEnc stored as Base64 of DPAPI
    auto s1 = Utils::IniReadString(path, L"AI", L"GoKey1Enc", L"");
    auto s2 = Utils::IniReadString(path, L"AI", L"GoKey2Enc", L"");
    auto s3 = Utils::IniReadString(path, L"AI", L"OrKeyEnc", L"");
    auto s4 = Utils::IniReadString(path, L"AI", L"DsKeyEnc", L"");
    // Also support legacy plain for migration? But spec says Enc only. Try plain fallback.
    if (!s1.empty()) {
        std::string dec = Utils::UnprotectData(Utils::WideToUtf8(s1));
        if (!dec.empty()) {
            goKey1 = Utils::Utf8ToWide(dec);
        } else {
            auto plain = Utils::IniReadString(path, L"AI", L"GoKey1", L"");
            if (!plain.empty()) goKey1 = plain;
        }
    } else {
        auto plain = Utils::IniReadString(path, L"AI", L"GoKey1", L"");
        goKey1 = plain;
    }
    if (!s2.empty()) {
        std::string dec = Utils::UnprotectData(Utils::WideToUtf8(s2));
        if (!dec.empty()) goKey2 = Utils::Utf8ToWide(dec);
        else goKey2 = Utils::IniReadString(path, L"AI", L"GoKey2", L"");
    } else {
        goKey2 = Utils::IniReadString(path, L"AI", L"GoKey2", L"");
    }
    if (!s3.empty()) {
        std::string dec = Utils::UnprotectData(Utils::WideToUtf8(s3));
        if (!dec.empty()) orKey = Utils::Utf8ToWide(dec);
        else {
            orKey = Utils::IniReadString(path, L"AI", L"OrKey", L"");
            AIDebugLog(L"Config: OrKey DPAPI fail, fallback plain");
        }
    } else {
        orKey = Utils::IniReadString(path, L"AI", L"OrKey", L"");
    }
    if (!s4.empty()) {
        std::string dec = Utils::UnprotectData(Utils::WideToUtf8(s4));
        if (!dec.empty()) dsKey = Utils::Utf8ToWide(dec);
        else dsKey = Utils::IniReadString(path, L"AI", L"DsKey", L"");
    } else {
        dsKey = Utils::IniReadString(path, L"AI", L"DsKey", L"");
    }

    goKey1Name = Utils::IniReadString(path, L"AI", L"GoKey1Name", L"Go1");
    goKey2Name = Utils::IniReadString(path, L"AI", L"GoKey2Name", L"Go2");
    dsKeyName = Utils::IniReadString(path, L"AI", L"DsKeyName", L"DS");
    intervalSec = Utils::IniReadInt(path, L"AI", L"IntervalSec", 30);
    if (intervalSec < 30) intervalSec = 30;
    if (intervalSec > 3600) intervalSec = 3600;

    showGoTop = Utils::IniReadBool(path, L"AI", L"ShowGoTop", true);
    showGoBottom = Utils::IniReadBool(path, L"AI", L"ShowGoBottom", true);
    showGoRolling = Utils::IniReadBool(path, L"AI", L"ShowGoRolling", false);
    showGoWeekly = Utils::IniReadBool(path, L"AI", L"ShowGoWeekly", false);
    showGoMonthly = Utils::IniReadBool(path, L"AI", L"ShowGoMonthly", false);
    showGoBottomRolling = Utils::IniReadBool(path, L"AI", L"ShowGoBottomRolling", false);
    showOrTop = Utils::IniReadBool(path, L"AI", L"ShowOrTop", true);
    showOrBottom = Utils::IniReadBool(path, L"AI", L"ShowOrBottom", true);
    showGo2Top = Utils::IniReadBool(path, L"AI", L"ShowGo2Top", false);
    showGo2Bottom = Utils::IniReadBool(path, L"AI", L"ShowGo2Bottom", false);
    showDsTop = Utils::IniReadBool(path, L"AI", L"ShowDsTop", true);
    showDsBottom = Utils::IniReadBool(path, L"AI", L"ShowDsBottom", true);

    displayMode = Utils::IniReadInt(path, L"AI", L"DisplayMode", 0);
    colorMode = Utils::IniReadInt(path, L"AI", L"ColorMode", 0);
    animMode = Utils::IniReadInt(path, L"AI", L"AnimMode", 0);
    batteryAuto = Utils::IniReadBool(path, L"AI", L"BatteryAuto", true);
    showResetTime = Utils::IniReadBool(path, L"AI", L"ShowResetTime", true);
    resetTimeFormat = Utils::IniReadInt(path, L"AI", L"ResetTimeFormat", 0);
    // ini 是 UTF-16LE 编码，GetPrivateProfileIntW 读不到，改用 IniReadString + atoi
    std::wstring ws = Utils::IniReadString(path, L"AI", L"WidthPx", L"90");
    widthPx = _wtoi(ws.c_str());
    AIDebugLog(L"Config widthPx parsed=%d", widthPx);
    if (widthPx < 50) widthPx = 50;
    if (widthPx > 300) widthPx = 300;

    goTopLabel = Utils::IniReadString(path, L"AI", L"GoTopLabel", L"Go");
    orTopLabel = Utils::IniReadString(path, L"AI", L"OrTopLabel", L"OR");
    go2TopLabel = Utils::IniReadString(path, L"AI", L"Go2TopLabel", L"Go2");
    showGoLabel = Utils::IniReadBool(path, L"AI", L"ShowGoLabel", true);
    hideOnExpired = Utils::IniReadBool(path, L"AI", L"HideOnExpired", true);

    endpointGo = Utils::IniReadString(path, L"AI", L"EndpointGo", L"");
    endpointOr = Utils::IniReadString(path, L"AI", L"EndpointOr", L"https://openrouter.ai/api/v1/key");
    endpointDs = Utils::IniReadString(path, L"AI", L"EndpointDs", L"https://api.deepseek.com/user/balance");
    dsDay = Utils::IniReadString(path, L"AI", L"DsDay", L"");
    {
        std::wstring v = Utils::IniReadString(path, L"AI", L"DsDayTotal", L"0");
        dsDayTotal = _wtof(v.c_str());
    }
}

void CAIConfig::Save(const std::wstring& path) const {
    std::wstring p = path.empty() ? configPath : path;
    // Encrypt keys
    if (!goKey1.empty()) {
        std::string enc = Utils::ProtectData(Utils::WideToUtf8(goKey1));
        Utils::IniWriteString(p, L"AI", L"GoKey1Enc", Utils::Utf8ToWide(enc));
        // clear plain if existed
        WritePrivateProfileStringW(L"AI", L"GoKey1", nullptr, p.c_str());
    } else {
        WritePrivateProfileStringW(L"AI", L"GoKey1Enc", nullptr, p.c_str());
        WritePrivateProfileStringW(L"AI", L"GoKey1", nullptr, p.c_str());
    }
    if (!goKey2.empty()) {
        std::string enc = Utils::ProtectData(Utils::WideToUtf8(goKey2));
        Utils::IniWriteString(p, L"AI", L"GoKey2Enc", Utils::Utf8ToWide(enc));
        WritePrivateProfileStringW(L"AI", L"GoKey2", nullptr, p.c_str());
    } else {
        WritePrivateProfileStringW(L"AI", L"GoKey2Enc", nullptr, p.c_str());
        WritePrivateProfileStringW(L"AI", L"GoKey2", nullptr, p.c_str());
    }
    if (!orKey.empty()) {
        std::string enc = Utils::ProtectData(Utils::WideToUtf8(orKey));
        Utils::IniWriteString(p, L"AI", L"OrKeyEnc", Utils::Utf8ToWide(enc));
        WritePrivateProfileStringW(L"AI", L"OrKey", nullptr, p.c_str());
    } else {
        WritePrivateProfileStringW(L"AI", L"OrKeyEnc", nullptr, p.c_str());
        WritePrivateProfileStringW(L"AI", L"OrKey", nullptr, p.c_str());
    }
    if (!dsKey.empty()) {
        std::string enc = Utils::ProtectData(Utils::WideToUtf8(dsKey));
        Utils::IniWriteString(p, L"AI", L"DsKeyEnc", Utils::Utf8ToWide(enc));
        WritePrivateProfileStringW(L"AI", L"DsKey", nullptr, p.c_str());
    } else {
        WritePrivateProfileStringW(L"AI", L"DsKeyEnc", nullptr, p.c_str());
        WritePrivateProfileStringW(L"AI", L"DsKey", nullptr, p.c_str());
    }

    Utils::IniWriteString(p, L"AI", L"GoKey1Name", goKey1Name);
    Utils::IniWriteString(p, L"AI", L"GoKey2Name", goKey2Name);
    Utils::IniWriteString(p, L"AI", L"DsKeyName", dsKeyName);
    Utils::IniWriteInt(p, L"AI", L"IntervalSec", intervalSec);
    Utils::IniWriteBool(p, L"AI", L"ShowGoTop", showGoTop);
    Utils::IniWriteBool(p, L"AI", L"ShowGoBottom", showGoBottom);
    Utils::IniWriteBool(p, L"AI", L"ShowGoRolling", showGoRolling);
    Utils::IniWriteBool(p, L"AI", L"ShowGoWeekly", showGoWeekly);
    Utils::IniWriteBool(p, L"AI", L"ShowGoMonthly", showGoMonthly);
    Utils::IniWriteBool(p, L"AI", L"ShowGoBottomRolling", showGoBottomRolling);
    Utils::IniWriteBool(p, L"AI", L"ShowOrTop", showOrTop);
    Utils::IniWriteBool(p, L"AI", L"ShowOrBottom", showOrBottom);
    Utils::IniWriteBool(p, L"AI", L"ShowGo2Top", showGo2Top);
    Utils::IniWriteBool(p, L"AI", L"ShowGo2Bottom", showGo2Bottom);
    Utils::IniWriteBool(p, L"AI", L"ShowDsTop", showDsTop);
    Utils::IniWriteBool(p, L"AI", L"ShowDsBottom", showDsBottom);
    Utils::IniWriteInt(p, L"AI", L"DisplayMode", displayMode);
    Utils::IniWriteInt(p, L"AI", L"ColorMode", colorMode);
    Utils::IniWriteInt(p, L"AI", L"AnimMode", animMode);
    Utils::IniWriteBool(p, L"AI", L"BatteryAuto", batteryAuto);
    Utils::IniWriteBool(p, L"AI", L"ShowResetTime", showResetTime);
    Utils::IniWriteInt(p, L"AI", L"ResetTimeFormat", resetTimeFormat);
    Utils::IniWriteInt(p, L"AI", L"WidthPx", widthPx);
    Utils::IniWriteString(p, L"AI", L"GoTopLabel", goTopLabel);
    Utils::IniWriteString(p, L"AI", L"OrTopLabel", orTopLabel);
    Utils::IniWriteString(p, L"AI", L"Go2TopLabel", go2TopLabel);
    Utils::IniWriteBool(p, L"AI", L"ShowGoLabel", showGoLabel);
    Utils::IniWriteBool(p, L"AI", L"HideOnExpired", hideOnExpired);
    Utils::IniWriteString(p, L"AI", L"EndpointGo", endpointGo);
    Utils::IniWriteString(p, L"AI", L"EndpointOr", endpointOr);
    Utils::IniWriteString(p, L"AI", L"EndpointDs", endpointDs);
    Utils::IniWriteString(p, L"AI", L"DsDay", dsDay);
    {
        wchar_t b[64];
        swprintf_s(b, L"%.3f", dsDayTotal);
        Utils::IniWriteString(p, L"AI", L"DsDayTotal", b);
    }
}
