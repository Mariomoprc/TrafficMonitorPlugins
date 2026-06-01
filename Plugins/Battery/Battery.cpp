#include "pch.h"
#include "Battery.h"
#include "Common.h"
#include <fstream>
#include "DataManager.h"
#include "OptionsDlg.h"
#include <sstream>

CBattery CBattery::m_instance;

CBattery::CBattery()
{
}

CBattery& CBattery::Instance()
{
    return m_instance;
}

IPluginItem* CBattery::GetItem(int index)
{
    switch (index)
    {
    case 0:
        return &m_item;
    case 1:
        return &m_time_item;
    default:
        break;
    }
    return nullptr;
}

const wchar_t* CBattery::GetTooltipInfo()
{
    if (g_data.m_setting_data.show_battery_in_tooltip)
        return m_tooltop_info.c_str();
    else
        return L"";
}

void CBattery::DataRequired()
{
    //获取系统电量（兼容原有电池百分比显示）
    GetSystemPowerStatus(&g_data.m_sysPowerStatus);
    //生成鼠标提示信息
    std::wstringstream wss;
    wss << g_data.StringRes(IDS_BATTERY).GetString() << L": " << g_data.GetBatteryString();
    if (g_data.IsAcOnline())
        wss << L" " << g_data.StringRes(IDS_CHARGING).GetString();
    if (g_data.m_sysPowerStatus.BatteryLifeTime != -1)
        wss << std::endl << g_data.StringRes(IDS_BATTERY_LIFE_TIME).GetString() << L": " << CCommon::TimeFormat(g_data.m_sysPowerStatus.BatteryLifeTime);
    if (g_data.m_sysPowerStatus.BatteryFullLifeTime != -1)
        wss << std::endl << g_data.StringRes(IDS_BATTERY_FULL_LIFE_TIME).GetString() << L": " << CCommon::TimeFormat(g_data.m_sysPowerStatus.BatteryFullLifeTime);
    m_tooltop_info = wss.str();

    // IOCTL 精确查询电池数据用于剩余时间显示
    CBatteryQuery::BatteryData bd;
    if (m_batteryQuery.QueryAll(bd))
    {
        double seconds = m_batteryQuery.GetSmoothedRemainingSeconds();
        if (m_batteryQuery.IsCharging())
        {
            if (m_batteryQuery.IsFull())
            {
                g_data.m_time_string = g_data.StringRes(IDS_BATTERY_TIME_FULL).GetString();
            }
            else if (seconds > 0)
            {
                g_data.m_time_string = L"+" + FormatTimeString(seconds);
            }
            else
            {
                g_data.m_time_string = g_data.StringRes(IDS_BATTERY_TIME_NA).GetString();
            }
        }
        else if (m_batteryQuery.IsOnBattery())
        {
            if (seconds > 0)
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
            g_data.m_time_string = g_data.StringRes(IDS_BATTERY_TIME_FULL).GetString();
        }
    }
    else
    {
        // 回退：使用 GetSystemPowerStatus 的数据
        if (g_data.m_sysPowerStatus.BatteryFlag == 128)
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
}

std::wstring CBattery::FormatTimeString(double seconds)
{
    if (seconds <= 0)
        return L"--";

    int total_seconds = static_cast<int>(seconds);
    int days = total_seconds / 86400;
    int hours = (total_seconds % 86400) / 3600;
    int minutes = (total_seconds % 3600) / 60;

    std::wstringstream wss;
    if (days > 0)
    {
        wss << days << L"d " << hours << L"h";
    }
    else if (hours > 0)
    {
        wss << hours << L"h " << minutes << L"m";
    }
    else
    {
        if (minutes < 1)
            minutes = 1;
        wss << minutes << L"m";
    }
    return wss.str();
}

ITMPlugin::OptionReturn CBattery::ShowOptionsDialog(void* hParent)
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

const wchar_t* CBattery::GetInfo(PluginInfoIndex index)
{
    static CString str;
    switch (index)
    {
    case TMI_NAME:
        return g_data.StringRes(IDS_PLUGIN_NAME).GetString();
    case TMI_DESCRIPTION:
        return g_data.StringRes(IDS_PLUGIN_DESCRIPTION).GetString();
    case TMI_AUTHOR:
        return L"zhongyang219";
    case TMI_COPYRIGHT:
        return L"Copyright (C) by Zhong Yang 2025";
    case ITMPlugin::TMI_URL:
        return L"https://github.com/zhongyang219/TrafficMonitorPlugins";
        break;
    case TMI_VERSION:
        return L"1.03";
    default:
        break;
    }
    return L"";
}

void CBattery::OnExtenedInfo(ExtendedInfoIndex index, const wchar_t* data)
{
    switch (index)
    {
    case ITMPlugin::EI_CONFIG_DIR:
        //从配置文件读取配置
        g_data.LoadConfig(std::wstring(data));

        break;
    default:
        break;
    }
}

void* CBattery::GetPluginIcon()
{
    return g_data.GetIcon(IDI_BATTERY_DARK);
}

ITMPlugin* TMPluginGetInstance()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    return &CBattery::Instance();
}
