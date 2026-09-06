#pragma once
#include <string>

class CAIConfig {
public:
    std::wstring goKey1;
    std::wstring goKey2;
    std::wstring goKey1Name;
    std::wstring goKey2Name;
    std::wstring orKey;
    std::wstring dsKey;
    std::wstring dsKeyName;
    int intervalSec = 300;
    bool showGoTop = true;
    bool showGoBottom = true;
    bool showGoRolling = false;
    bool showGoWeekly = false;
    bool showGoMonthly = false;
    bool showGoBottomRolling = false; // alias for showGoRolling bottom handling
    bool showOrTop = true;
    bool showOrBottom = true;
    bool showGo2Top = false;
    bool showGo2Bottom = false;
    bool showDsTop = true;
    bool showDsBottom = true;
    // showGoRolling flag also controls GoBottom rolling sub-window? Keep explicit
    int displayMode = 0; // 0 bar, 1 text, 2 ring
    int colorMode = 0; // 0 single, 1 three-step
    int animMode = 0; // 0 on-update, 1:200 2:100 3:50
    bool batteryAuto = true;
    bool showResetTime = true;
    int resetTimeFormat = 0;
    int widthPx = 90;
    std::wstring goTopLabel = L"Go";
    std::wstring orTopLabel = L"OR";
    std::wstring go2TopLabel = L"Go2";
    bool showGoLabel = true;
    bool hideOnExpired = true;
    unsigned long taskbarValueColor = 0x00FFFFFF;
    unsigned long taskbarLabelColor = 0x00FFFFFF;
    bool hasTaskbarColors = false;
    std::wstring endpointGo;
    std::wstring endpointOr = L"https://openrouter.ai/api/v1/credits";
    std::wstring endpointDs = L"https://api.deepseek.com/user/balance";
    std::wstring dsDay;
    double dsDayTotal = 0;

    std::wstring configPath;

    void Load(const std::wstring& path);
    void Save(const std::wstring& path) const;

    // helpers for testing
    std::wstring GetIniPath() const { return configPath; }
};
