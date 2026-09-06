#include <windows.h>
#include <gdiplus.h>
#include <powrprof.h>
#include "AILimitPlugin.h"
#include "OptionsDlg.h"
#include "Utils.h"
#include "Log.h"
#include <cstdio>
#define AILog AIDebugLog

CAILimitPlugin::CAILimitPlugin() {
    AILog(L"CTOR this=%p size_before=%zu", this, m_items.size());
    // 不在这里创建 items，等 OnInitialize 加载 config 后再创建
    AILog(L"CTOR done size=%zu", m_items.size());
}
CAILimitPlugin::~CAILimitPlugin() {}

void AILimit_ForceRefresh() { CAILimitPlugin::Instance().ForceRefresh(); }
CAILimitPlugin& CAILimitPlugin::Instance() {
    static CAILimitPlugin s;
    return s;
}

void CAILimitPlugin::OnInitialize(ITrafficMonitor* pApp) {
    AILog(L"OnInitialize pApp=%p", pApp);
    m_app = pApp;
    if (m_app) {
        const wchar_t* dir = m_app->GetPluginConfigDir();
        if (dir) m_configDir = dir;
    }
    if (!m_configDir.empty() && m_configDir.back() != L'\\' && m_configDir.back() != L'/')
        m_configDir += L"\\";
    std::wstring ini = m_configDir + L"AILimitPlugin.ini";
    m_config.Load(ini);
    m_config.configPath = ini;
    // 配置加载完成后创建 items（确保 widthPx 等配置已生效）
    EnsureItems();
    RebuildItemList();
    UpdateTooltip();
    // Go1/Go2 自动注册到宿主 plugin_display_item
    if (m_config.showGoTop || m_config.showGoBottom || m_config.showGo2Top || m_config.showGo2Bottom) {
        std::wstring hostCfg = m_configDir + L"..\\config.ini";
        wchar_t buf[2048] = {};
        GetPrivateProfileStringW(L"task_bar", L"plugin_display_item", L"", buf, 2048, hostCfg.c_str());
        std::wstring pdi = buf;
        if (pdi.find(L"go_top") == std::wstring::npos) {
            pdi += L",go_top,go_bottom";
            WritePrivateProfileStringW(L"task_bar", L"plugin_display_item", pdi.c_str(), hostCfg.c_str());
            AILog(L"OnInitialize added go_top/go_bottom to plugin_display_item");
        }
        if (pdi.find(L"go2_top") == std::wstring::npos) {
            pdi += L",go2_top,go2_bottom";
            WritePrivateProfileStringW(L"task_bar", L"plugin_display_item", pdi.c_str(), hostCfg.c_str());
            AILog(L"OnInitialize added go2_top/go2_bottom to plugin_display_item");
        }
        if (!m_config.dsKey.empty() && pdi.find(L"ds_top") == std::wstring::npos) {
            pdi += L",ds_top,ds_bottom";
            WritePrivateProfileStringW(L"task_bar", L"plugin_display_item", pdi.c_str(), hostCfg.c_str());
            AILog(L"OnInitialize added ds_top/ds_bottom to plugin_display_item");
        }
    }
    // GDI+ startup
    Gdiplus::GdiplusStartupInput si;
    ULONG_PTR token;
    static ULONG_PTR s_token = 0;
    static bool s_inited = false;
    if (!s_inited) {
        Gdiplus::GdiplusStartup(&token, &si, nullptr);
        s_token = token;
        s_inited = true;
    }
}

void CAILimitPlugin::EnsureItems() {
    m_items.clear();
    m_go1Top.reset(); m_go1Bottom.reset();
    m_go2Top.reset(); m_go2Bottom.reset();
    m_orTop.reset(); m_orBottom.reset();
    m_dsTop.reset(); m_dsBottom.reset();
    m_go1Top = std::make_unique<CGoItem>(true, false, &m_config, &m_go1);
    m_go1Bottom = std::make_unique<CGoItem>(false, false, &m_config, &m_go1);
    m_go2Top = std::make_unique<CGoItem>(true, true, &m_config, &m_go2);
    m_go2Bottom = std::make_unique<CGoItem>(false, true, &m_config, &m_go2);
    m_orTop = std::make_unique<COrItem>(true, &m_config, &m_or);
    m_orBottom = std::make_unique<COrItem>(false, &m_config, &m_or);
    m_dsTop = std::make_unique<CDsItem>(true, &m_config, &m_ds, &m_dsSpentToday);
    m_dsBottom = std::make_unique<CDsItem>(false, &m_config, &m_ds, &m_dsSpentToday);
    AILog(L"EnsureItems size=%zu", m_items.size());
}

void CAILimitPlugin::RebuildItemList() {
    m_items.clear();
    AILog(L"RebuildItemList: go1Top=%p go1Bot=%p go2Top=%p go2Bot=%p orTop=%p orBot=%p dsTop=%p dsBot=%p",
        m_go1Top.get(), m_go1Bottom.get(), m_go2Top.get(), m_go2Bottom.get(), m_orTop.get(), m_orBottom.get(),
        m_dsTop.get(), m_dsBottom.get());
    if (m_go1Top) m_items.push_back(m_go1Top.get());
    if (m_go1Bottom) m_items.push_back(m_go1Bottom.get());
    if (m_go2Top) m_items.push_back(m_go2Top.get());
    if (m_go2Bottom) m_items.push_back(m_go2Bottom.get());
    if (m_orTop) m_items.push_back(m_orTop.get());
    if (m_orBottom) m_items.push_back(m_orBottom.get());
    if (m_dsTop) m_items.push_back(m_dsTop.get());
    if (m_dsBottom) m_items.push_back(m_dsBottom.get());
    AILog(L"RebuildItemList done size=%zu", m_items.size());
}

IPluginItem* CAILimitPlugin::GetItem(int index) {
    AILog(L"GetItem %d size=%zu m_app=%p", index, m_items.size(), m_app);
    if (index < 0 || index >= (int)m_items.size()) return nullptr;
    IPluginItem* item = m_items[index];
    if (item) { AILog(L"  -> id=%ls", item->GetItemId()); }
    return item;
}

bool CAILimitPlugin::IsBatterySaver() {
    SYSTEM_POWER_STATUS sps{};
    if (GetSystemPowerStatus(&sps)) {
        // On battery and battery saver? simple: ACLineStatus==0 means on battery
        if (sps.ACLineStatus == 0 && m_config.batteryAuto) return true;
    }
    return false;
}

void CAILimitPlugin::DataRequired() {
    AILog(L"DataRequired tick=%llu", Utils::GetTickMs());
    ULONGLONG now = Utils::GetTickMs();
    // Animation: if animMode !=0, caller may want frequent refresh; but DataRequired is throttled by plugin host (~1s)
    // We still handle throttling for network
    if (now < m_backoffUntilMs) return; // 429 退避
    ULONGLONG intervalMs = (ULONGLONG)m_config.intervalSec * 1000ULL;
    if (m_config.batteryAuto && IsBatterySaver()) {
        intervalMs *= 4; // 笔记本电池：4倍间隔更省电
        if (intervalMs < 120000) intervalMs = 120000; // 最少2分钟
    }
    bool needFetch = m_firstFetch || (now - m_lastFetchMs >= intervalMs);
    if (!needFetch) {
        return;
    }
    // Fetch Go1
    if (!m_config.goKey1.empty()) {
        GoData d; std::string err;
        bool ok = FetchGo(m_config.goKey1, d, err, m_config.endpointGo);
        AILog(L"FetchGo ok=%d err=%hs maxPct=%d", ok, err.c_str(), d.maxPercent);
        if (err.find("429") != std::string::npos || err.find("rate") != std::string::npos) { m_backoffUntilMs = now + 60000; }
        if (ok) {
            m_go1 = d;
            m_go1FailCount = 0;
            int remainTop = d.rateLimited ? 0 : (100 - d.monthly.percent);
            if (remainTop < 0) remainTop = 0; if (remainTop > 100) remainTop = 100;
            int barTop = d.rateLimited ? 100 : remainTop;
            if (m_go1Top) m_go1Top->UpdateAnimTarget(barTop);
            if (m_go1Bottom) {
                int best = d.maxPercent;
                if (!m_config.showGoRolling && !m_config.showGoBottomRolling) {
                    if (d.maxWindow=="5h" || d.maxWindow=="5小时") best = d.weekly.percent;
                }
                bool limited = d.rateLimited && best>=100;
                if (limited) { if (d.rolling.status=="rate-limited" || d.weekly.status=="rate-limited" || d.monthly.status=="rate-limited") limited=true; }
                int remainBottom = limited ? 0 : (100 - best);
                if (remainBottom < 0) remainBottom = 0;
                int barBottom = limited ? 100 : remainBottom;
                m_go1Bottom->UpdateAnimTarget(barBottom);
            }
        } else if (IsAuthError(err)) {
            if (++m_go1FailCount >= 2) m_go1.valid = false;
        }
    } else {
        m_go1.valid = false;
        m_go1FailCount = 2;
    }
    // Go2: 始终获取，无需启用判断（宿主显示设置决定可见性）
    if (!m_config.goKey2.empty()) {
        GoData d; std::string err;
        if (FetchGo(m_config.goKey2, d, err, m_config.endpointGo)) {
            m_go2 = d;
            m_go2FailCount = 0;
            int r1 = d.rateLimited ? 0 : (100 - d.monthly.percent);
            if (r1<0) r1=0; int b1 = d.rateLimited ? 100 : r1;
            int r2 = r1; int b2 = b1;
            if (m_go2Top) m_go2Top->UpdateAnimTarget(b1);
            if (m_go2Bottom) m_go2Bottom->UpdateAnimTarget(b2);
        } else if (IsAuthError(err)) {
            if (++m_go2FailCount >= 2) m_go2.valid = false;
        }
    }
    AILog(L"DataRequired: goKey1=%d orKey=%d showOr=%d", m_config.goKey1.empty()?0:1, m_config.orKey.empty()?0:1, m_config.showOrTop?1:0);
    if (!m_config.orKey.empty() && (m_config.showOrTop || m_config.showOrBottom)) {
        OrData d; std::string err;
        AILog(L"DataRequired OR: keyLen=%zu ep=%ls showTop=%d showBot=%d",
              m_config.orKey.size(), m_config.endpointOr.c_str(),
              m_config.showOrTop?1:0, m_config.showOrBottom?1:0);
        if (FetchOr(m_config.orKey, d, err, m_config.endpointOr)) {
            m_or = d;
            m_orFailCount = 0;
            double bal = d.remaining;
            if (bal < 0) bal = 0;
            AILog(L"DataRequired OR OK: bal=%.2f usage=%.1f valid=%d", bal, d.usage, d.valid?1:0);
            if (m_orTop) m_orTop->UpdateAnimTarget(bal);
            if (m_orBottom) m_orBottom->UpdateAnimTarget(bal);
        } else {
            AILog(L"DataRequired OR FAIL: %hs", err.c_str());
            if (IsAuthError(err) && ++m_orFailCount >= 2) m_or.valid = false;
        }
    } else if (m_config.orKey.empty()) {
        m_or.valid = false;
        m_orFailCount = 2;
    }
    // Fetch DS
    if (!m_config.dsKey.empty()) {
        DsData d; std::string err;
        if (FetchDs(m_config.dsKey, d, err, m_config.endpointDs)) {
            m_ds = d;
            m_dsFailCount = 0;
            std::wstring today = TodayStr();
            if (m_config.dsDay != today) {
                m_config.dsDay = today;
                m_config.dsDayTotal = d.total;
                m_dsSpentToday = 0;
                m_dsToppedUpToday = false;
                m_dsSavedSpent = 0;
                SYSTEMTIME st{};
                GetLocalTime(&st);
                m_dsStartMin = st.wHour * 60 + st.wMinute;
                m_config.Save(L"");
            } else {
                if (d.total > m_config.dsDayTotal + 0.01) {
                    m_config.dsDayTotal = d.total;
                    m_dsSpentToday = 0;
                    m_dsToppedUpToday = true;
                    m_dsSavedSpent = 0;
                    m_config.Save(L"");
                } else {
                    double spent = m_config.dsDayTotal - d.total;
                    if (spent < 0) spent = 0;
                    m_dsSpentToday = spent;
                    if (m_dsSavedSpent < 0 || spent - m_dsSavedSpent > 0.005) {
                        m_dsSavedSpent = spent;
                        m_config.Save(L"");
                    }
                }
            }
            AILog(L"DataRequired DS OK: total=%.2f spent=%.2f", d.total, m_dsSpentToday);
        } else {
            AILog(L"DataRequired DS FAIL: %hs", err.c_str());
            if (IsAuthError(err) && ++m_dsFailCount >= 2) m_ds.valid = false;
        }
    } else {
        m_ds.valid = false;
        m_dsFailCount = 2;
    }

    m_lastFetchMs = now;
    m_firstFetch = false;
    ApplyHidePolicy();
    AlignPartnerWidths();
    UpdateTooltip();
}

bool CAILimitPlugin::IsAuthError(const std::string& err) {
    if (err.empty() || err == "empty key") return true;
    std::string low = err;
    for (auto& c : low) c = (char)tolower((unsigned char)c);
    const char* keys[] = {"401", "402", "403", "unauthorized", "forbidden", "invalid", "expired", "denied", "empty key", "empty endpoint", "bad endpoint"};
    for (auto k : keys) {
        if (low.find(k) != std::string::npos) return true;
    }
    return false;
}

bool CAILimitPlugin::IsGoExhausted(const GoData& d) {
    return d.valid && d.monthly.percent >= 100;
}

bool CAILimitPlugin::IsOrExhausted(const OrData& d) {
    return d.valid && !d.isFreeTier && d.remaining <= 0.001;
}

bool CAILimitPlugin::IsDsExhausted(const DsData& d) {
    return d.valid && (!d.isAvailable || d.total <= 0.001);
}

std::wstring CAILimitPlugin::TodayStr() {
    SYSTEMTIME st{};
    GetLocalTime(&st);
    wchar_t b[16];
    swprintf_s(b, L"%04d%02d%02d", st.wYear, st.wMonth, st.wDay);
    return b;
}

void CAILimitPlugin::ApplyHidePolicy() {
    if (!m_config.hideOnExpired) {
        if (m_go1Top) m_go1Top->SetHidden(false);
        if (m_go1Bottom) m_go1Bottom->SetHidden(false);
        if (m_go2Top) m_go2Top->SetHidden(false);
        if (m_go2Bottom) m_go2Bottom->SetHidden(false);
        if (m_orTop) m_orTop->SetHidden(false);
        if (m_orBottom) m_orBottom->SetHidden(false);
        if (m_dsTop) m_dsTop->SetHidden(false);
        if (m_dsBottom) m_dsBottom->SetHidden(false);
        return;
    }
    bool go1Hide = !m_go1.valid || IsGoExhausted(m_go1);
    bool go2Hide = !m_go2.valid && !m_config.goKey2.empty();
    if (m_config.goKey2.empty()) go2Hide = true;
    else go2Hide = go2Hide || IsGoExhausted(m_go2);
    bool orHide = !m_or.valid || IsOrExhausted(m_or);
    bool dsHide = !m_ds.valid || IsDsExhausted(m_ds);
    if (m_config.dsKey.empty()) dsHide = true;
    if (m_go1Top) m_go1Top->SetHidden(go1Hide);
    if (m_go1Bottom) m_go1Bottom->SetHidden(go1Hide);
    if (m_go2Top) m_go2Top->SetHidden(go2Hide);
    if (m_go2Bottom) m_go2Bottom->SetHidden(go2Hide);
    if (m_orTop) m_orTop->SetHidden(orHide);
    if (m_orBottom) m_orBottom->SetHidden(orHide);
    if (m_dsTop) m_dsTop->SetHidden(dsHide);
    if (m_dsBottom) m_dsBottom->SetHidden(dsHide);
}

void CAILimitPlugin::UpdateTooltip() {
    std::wstring tip;
    if (m_go1.valid) {
        tip += L"Go1: " + m_go1.summary;
        if (m_go1.rateLimited) tip += L" (rate limited)";
    } else {
        tip += L"Go1: --";
    }
    if (m_go2.valid) {
        tip += L"\nGo2: " + m_go2.summary;
    }
    if (m_or.valid) {
        wchar_t buf[128];
        swprintf_s(buf, L"\nOR: $%.0f (%.1f%% used)", m_or.remaining, m_or.usage);
        tip += buf;
    } else if (!m_config.orKey.empty()) {
        tip += L"\nOR: --";
    }
    if (m_ds.valid) {
        const wchar_t* sym = (m_ds.currency == "CNY") ? L"¥" : L"$";
        wchar_t buf[160];
        swprintf_s(buf, L"\nDS: %ls%.0f (今日耗 -%ls%.1f)", sym, m_ds.total, sym, m_dsSpentToday);
        tip += buf;
        if (m_dsToppedUpToday) tip += L"（今日有入账）";
        else if (m_dsStartMin >= 0) {
            wchar_t tb[32];
            swprintf_s(tb, L"（自 %02d:%02d 起统计）", m_dsStartMin / 60, m_dsStartMin % 60);
            tip += tb;
        }
    } else if (!m_config.dsKey.empty()) {
        tip += L"\nDS: --";
    }
    // Add reset times detail
    if (m_config.showResetTime && m_go1.valid) {
        std::wstring extra;
        if (!m_go1.rolling.resetsAt.empty()) {
            extra += L"\n  5h resets: " + m_go1.rolling.resetText;
        }
        if (!m_go1.weekly.resetsAt.empty()) {
            extra += L"\n  Week resets: " + m_go1.weekly.resetText;
        }
        tip += extra;
    }
    m_tooltip = tip;
}

void CAILimitPlugin::AlignPartnerWidths() {
    auto calcWidth = [](IPluginItem* item) -> int {
        const wchar_t* txt = item->GetItemValueText();
        if (!txt || !*txt) return 0;
        int len = (int)wcslen(txt);
        return len * 8 + 12;
    };
    if (m_go1Top && m_go1Bottom) {
        int w1 = calcWidth(m_go1Top.get());
        int w2 = calcWidth(m_go1Bottom.get());
        m_go1Top->m_partnerMaxWidth = w2;
        m_go1Bottom->m_partnerMaxWidth = w1;
    }
    if (m_go2Top && m_go2Bottom) {
        int w1 = calcWidth(m_go2Top.get());
        int w2 = calcWidth(m_go2Bottom.get());
        m_go2Top->m_partnerMaxWidth = w2;
        m_go2Bottom->m_partnerMaxWidth = w1;
    }
    if (m_orTop && m_orBottom) {
        int w1 = calcWidth(m_orTop.get());
        int w2 = calcWidth(m_orBottom.get());
        m_orTop->m_partnerMaxWidth = w2;
        m_orBottom->m_partnerMaxWidth = w1;
    }
    if (m_dsTop && m_dsBottom) {
        int w1 = calcWidth(m_dsTop.get());
        int w2 = calcWidth(m_dsBottom.get());
        m_dsTop->m_partnerMaxWidth = w2;
        m_dsBottom->m_partnerMaxWidth = w1;
    }
}

const wchar_t* CAILimitPlugin::GetTooltipInfo() { return m_tooltip.c_str(); }

const wchar_t* CAILimitPlugin::GetInfo(PluginInfoIndex index) {
    switch (index) {
    case TMI_NAME: return m_infoName.c_str();
    case TMI_DESCRIPTION: return m_infoDesc.c_str();
    case TMI_AUTHOR: return m_infoAuthor.c_str();
    case TMI_COPYRIGHT: return m_infoCopyright.c_str();
    case TMI_VERSION: return m_infoVersion.c_str();
    case TMI_URL: return m_infoUrl.c_str();
    default: return L"";
    }
}

ITMPlugin::OptionReturn CAILimitPlugin::ShowOptionsDialog(void* hParent) {
    CAIConfig backup = m_config;
    COptionsDlg dlg(&m_config);
    dlg.SetLiveData(&m_go1, &m_go2, &m_or);
    dlg.SetLiveDs(&m_ds, &m_dsSpentToday);
    dlg.SetOnApply([this]() {
        RebuildItemList();
        ApplyHidePolicy();
        UpdateTooltip();
        m_lastFetchMs = 0;
        m_firstFetch = true;
    });
    INT_PTR ret = dlg.Show((HWND)hParent);
    if (ret == IDOK) {
        // Save already done in dialog
        RebuildItemList();
        UpdateTooltip();
        // reset fetch timer to fetch immediately
        m_lastFetchMs = 0;
        m_firstFetch = true;
        return OR_OPTION_CHANGED;
    } else {
        m_config = backup;
        return OR_OPTION_UNCHANGED;
    }
}

void CAILimitPlugin::OnExtenedInfo(ExtendedInfoIndex index, const wchar_t* data) {
    AILog(L"OnExtenedInfo %d data=%ls", (int)index, data?data:L"(null)");
    if (index == EI_LABEL_TEXT_COLOR && data) {
        m_config.taskbarLabelColor = (unsigned long)_wtol(data);
        m_config.hasTaskbarColors = true;
        return;
    }
    if (index == EI_VALUE_TEXT_COLOR && data) {
        m_config.taskbarValueColor = (unsigned long)_wtol(data);
        m_config.hasTaskbarColors = true;
        return;
    }
    if (index == EI_CONFIG_DIR && data) {
        m_configDir = data;
        if (!m_configDir.empty() && m_configDir.back() != L'\\' && m_configDir.back() != L'/')
            m_configDir += L"\\";
        std::wstring ini = m_configDir + L"AILimitPlugin.ini";
        m_config.configPath = ini;
        m_config.Load(ini);
        EnsureItems();
        RebuildItemList();
        UpdateTooltip();
    }
}
