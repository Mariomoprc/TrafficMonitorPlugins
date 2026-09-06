#pragma once
#include "PluginInterface.h"
#include "Config.h"
#include "ApiClient.h"
#include "GoItem.h"
#include "OrItem.h"
#include "DsItem.h"
#include <string>
#include <vector>
#include <memory>

class CAILimitPlugin : public ITMPlugin {
public:
    static CAILimitPlugin& Instance();

    // ITMPlugin
    IPluginItem* GetItem(int index) override;
    void DataRequired() override;
    OptionReturn ShowOptionsDialog(void* hParent) override;
    const wchar_t* GetInfo(PluginInfoIndex index) override;
    const wchar_t* GetTooltipInfo() override;
    void OnExtenedInfo(ExtendedInfoIndex index, const wchar_t* data) override;
    void OnInitialize(ITrafficMonitor* pApp) override;
    void ForceRefresh() { m_lastFetchMs = 0; m_firstFetch = true; }
    CAILimitPlugin();
    ~CAILimitPlugin();

    void EnsureItems();
    void UpdateTooltip();
    void AlignPartnerWidths();
    void ApplyHidePolicy();
    static bool IsAuthError(const std::string& err);
    static bool IsGoExhausted(const GoData& d);
    static bool IsOrExhausted(const OrData& d);
    static bool IsDsExhausted(const DsData& d);
    static std::wstring TodayStr();
    bool IsBatterySaver();

    ITrafficMonitor* m_app = nullptr;
    std::wstring m_configDir;
    CAIConfig m_config;

    GoData m_go1;
    GoData m_go2;
    OrData m_or;
    DsData m_ds;
    double m_dsSpentToday = 0;
    bool m_dsToppedUpToday = false;

    std::unique_ptr<CGoItem> m_go1Top;
    std::unique_ptr<CGoItem> m_go1Bottom;
    std::unique_ptr<CGoItem> m_go2Top;
    std::unique_ptr<CGoItem> m_go2Bottom;
    std::unique_ptr<COrItem> m_orTop;
    std::unique_ptr<COrItem> m_orBottom;
    std::unique_ptr<CDsItem> m_dsTop;
    std::unique_ptr<CDsItem> m_dsBottom;

    std::vector<IPluginItem*> m_items; // ordered active items
    void RebuildItemList();

    std::wstring m_tooltip;
    ULONGLONG m_lastFetchMs = 0;
    ULONGLONG m_backoffUntilMs = 0;
    bool m_firstFetch = true;
    int m_go1FailCount = 0;
    int m_go2FailCount = 0;
    int m_orFailCount = 0;
    int m_dsFailCount = 0;
    double m_dsSavedSpent = -1;
    int m_dsStartMin = -1;

    std::wstring m_infoName = L"AI Limit";
    std::wstring m_infoDesc = L"Display AI quota (Go 5h/Weekly/Monthly, OpenRouter & DeepSeek)";
    std::wstring m_infoAuthor = L"AILimitPlugin";
    std::wstring m_infoCopyright = L"Copyright (c) AILimitPlugin";
    std::wstring m_infoVersion = L"1.2.3";
    std::wstring m_infoUrl = L"https://github.com/Mariomoprc/TrafficMonitorPlugins";
};
