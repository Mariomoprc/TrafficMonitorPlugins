#include "pch.h"
#include "BatteryTime.h"
#include "Common.h"
#include <fstream>
#include "DataManager.h"
#include "OptionsDlg.h"
#include <sstream>

CBatteryTime CBatteryTime::m_instance;

CBatteryTime::CBatteryTime()
{
}

CBatteryTime& CBatteryTime::Instance()
{
    return m_instance;
}

IPluginItem* CBatteryTime::GetItem(int index)
{
    switch (index)
    {
    case 0:
        return &m_item;
    default:
        break;
    }
    return nullptr;
}

const wchar_t* CBatteryTime::GetTooltipInfo()
{
    if (g_data.m_setting_data.show_battery_in_tooltip)
        return m_tooltip_info.c_str();
    else
        return L"";
}

void CBatteryTime::DataRequired()
{
    static DWORD s_last_refresh = 0;
    DWORD now = GetTickCount();
    if (now - s_last_refresh < static_cast<DWORD>(g_data.m_setting_data.refresh_interval))
        return;
    s_last_refresh = now;

    GetSystemPowerStatus(&g_data.m_sysPowerStatus);

    g_data.UpdatePreview();
    g_data.PomodoroTick();

    std::wstringstream wss;
    wss << L"Battery: " << g_data.m_sysPowerStatus.BatteryLifePercent << L"%";
    if (g_data.IsAcOnline())
        wss << L" (Charging)";
    if (g_data.m_sysPowerStatus.BatteryLifeTime != -1)
        wss << std::endl << L"Remaining: " << CCommon::TimeFormat(g_data.m_sysPowerStatus.BatteryLifeTime);
    if (g_data.PomodoroVisible())
        wss << std::endl << L"\x756A\x8309\x949F: " << g_data.PomodoroPhaseName() << L" " << g_data.PomodoroText();
    m_tooltip_info = wss.str();

    if (g_data.IsPreviewActive())
        return;

    CBatteryQuery::BatteryData bd;
    if (m_batteryQuery.QueryAll(bd))
    {
        double seconds = m_batteryQuery.GetSmoothedRemainingSeconds();
        if (m_batteryQuery.IsCharging() || !m_batteryQuery.IsOnBattery())
        {
            g_data.m_time_string = L"";
        }
        else if (seconds > 0)
        {
            g_data.m_time_string = FormatTimeString(seconds);
        }
        else
        {
            g_data.m_time_string = g_data.StringRes(IDS_BATTERY_TIME_NA).GetString();
        }
    }
    else
    {
        if (g_data.IsAcOnline())
        {
            g_data.m_time_string = L"";
        }
        else if (g_data.m_sysPowerStatus.BatteryFlag == 128)
        {
            g_data.m_time_string = g_data.StringRes(IDS_BATTERY_TIME_NA).GetString();
        }
        else if (g_data.m_sysPowerStatus.BatteryLifeTime != (DWORD)-1)
        {
            g_data.m_time_string = FormatTimeString(static_cast<double>(g_data.m_sysPowerStatus.BatteryLifeTime));
        }
        else
        {
            g_data.m_time_string = L"--";
        }
    }

    BYTE pct = g_data.m_sysPowerStatus.BatteryLifePercent;
    bool ac = g_data.IsAcOnline();
    DWORD full_flag = (pct == 100 && ac) ? 1 : 0;
    DWORD low_flag = (pct <= g_data.m_setting_data.low_battery_threshold && !ac) ? 1 : 0;

    if (g_data.m_setting_data.sound_on_full && full_flag && g_data.GetLastFullState() == 0)
        MessageBeep(MB_OK);
    g_data.SetLastFullState(full_flag);

    if (g_data.m_setting_data.sound_on_low && low_flag && g_data.GetLastLowState() == 0)
        MessageBeep(MB_ICONEXCLAMATION);
    g_data.SetLastLowState(low_flag);
}

std::wstring CBatteryTime::FormatTimeString(double seconds)
{
    if (seconds <= 0)
        return L"--";

    int total_sec = static_cast<int>(seconds);
    int d = total_sec / 86400;
    int h = (total_sec % 86400) / 3600;
    int m = (total_sec % 3600) / 60;
    int s = total_sec % 60;

    if (g_data.m_setting_data.hide_zero)
    {
        if (g_data.m_setting_data.show_seconds && !g_data.m_setting_data.show_days && h == 0 && m == 0 && s == 0)
            return L"--";
        if (!g_data.m_setting_data.show_seconds && d == 0 && h == 0 && m == 0)
            m = 1;
    }

    if (!g_data.m_setting_data.show_days)
    {
        h += d * 24;
        d = 0;
    }

    std::wstring fmt = g_data.m_setting_data.custom_format;

    auto pad2 = [](int v) -> std::wstring
    {
        wchar_t buf[8];
        swprintf_s(buf, L"%02d", v);
        return buf;
    };

    auto replace_all = [](std::wstring& str, const std::wstring& from, const std::wstring& to)
    {
        size_t pos = 0;
        while ((pos = str.find(from, pos)) != std::wstring::npos)
        {
            str.replace(pos, from.length(), to);
            pos += to.length();
        }
    };

    replace_all(fmt, L"{d:02}", pad2(d));
    replace_all(fmt, L"{d}", std::to_wstring(d));
    replace_all(fmt, L"{h:02}", pad2(h));
    replace_all(fmt, L"{h}", std::to_wstring(h));
    replace_all(fmt, L"{m:02}", pad2(m));
    replace_all(fmt, L"{m}", std::to_wstring(m));
    replace_all(fmt, L"{s:02}", pad2(s));
    replace_all(fmt, L"{s}", std::to_wstring(s));

    return fmt;
}

ITMPlugin::OptionReturn CBatteryTime::ShowOptionsDialog(void* hParent)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    CWnd* pParent = CWnd::FromHandle((HWND)hParent);
    COptionsDlg dlg(pParent);
    dlg.m_data = g_data.m_setting_data;
    if (dlg.DoModal() == IDOK)
    {
        g_data.m_setting_data = dlg.m_data;
        g_data.SaveConfig();
        return ITMPlugin::OR_OPTION_CHANGED;
    }
    return ITMPlugin::OR_OPTION_UNCHANGED;
}

const wchar_t* CBatteryTime::GetInfo(PluginInfoIndex index)
{
    static CString str;
    switch (index)
    {
    case TMI_NAME:
        return g_data.StringRes(IDS_PLUGIN_NAME).GetString();
    case TMI_DESCRIPTION:
        return g_data.StringRes(IDS_PLUGIN_DESCRIPTION).GetString();
    case TMI_AUTHOR:
        return L"Mariomoprc";
    case TMI_COPYRIGHT:
        return L"Copyright (C) by Mariomoprc 2026";
    case ITMPlugin::TMI_URL:
        return L"https://github.com/Mariomoprc/TrafficMonitorPlugins";
    case TMI_VERSION:
        return L"1.10";
    default:
        break;
    }
    return L"";
}

void CBatteryTime::OnExtenedInfo(ExtendedInfoIndex index, const wchar_t* data)
{
    switch (index)
    {
    case ITMPlugin::EI_CONFIG_DIR:
        g_data.LoadConfig(std::wstring(data));
        break;
    default:
        break;
    }
}

ITMPlugin* TMPluginGetInstance()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    return &CBatteryTime::Instance();
}
