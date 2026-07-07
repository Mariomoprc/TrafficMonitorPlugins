#include "pch.h"
#include "DataManager.h"
#include <vector>
#include <sstream>

CDataManager CDataManager::m_instance;

CDataManager::CDataManager()
{
    HDC hDC = ::GetDC(HWND_DESKTOP);
    m_dpi = GetDeviceCaps(hDC, LOGPIXELSY);
    ::ReleaseDC(HWND_DESKTOP, hDC);
}

CDataManager::~CDataManager()
{
    SaveConfig();
}

CDataManager& CDataManager::Instance()
{
    return m_instance;
}

static void WritePrivateProfileInt(const wchar_t* app_name, const wchar_t* key_name, int value, const wchar_t* file_path)
{
    wchar_t buff[16];
    swprintf_s(buff, L"%d", value);
    WritePrivateProfileString(app_name, key_name, buff, file_path);
}

void CDataManager::LoadConfig(const std::wstring& config_dir)
{
    HMODULE hModule = reinterpret_cast<HMODULE>(&__ImageBase);
    wchar_t path[MAX_PATH];
    GetModuleFileNameW(hModule, path, MAX_PATH);
    std::wstring module_path = path;
    m_config_path = module_path;
    if (!config_dir.empty())
    {
        size_t index = module_path.find_last_of(L"\\/");
        std::wstring module_file_name = module_path.substr(index + 1);
        m_config_path = config_dir + module_file_name;
    }
    m_config_path += L".ini";

    m_setting_data.preview_mode = (GetPrivateProfileInt(L"config", L"preview_mode", 0, m_config_path.c_str()) != 0);
    m_setting_data.progress_bar_above = (GetPrivateProfileInt(L"config", L"progress_bar_above", 0, m_config_path.c_str()) != 0);
    m_setting_data.progress_bar_height = GetPrivateProfileInt(L"config", L"progress_bar_height", 5, m_config_path.c_str());
    m_setting_data.progress_bar_max_width = GetPrivateProfileInt(L"config", L"progress_bar_max_width", 30, m_config_path.c_str());
    m_setting_data.color_high = GetPrivateProfileInt(L"config", L"color_high", RGB(128, 194, 105), m_config_path.c_str());
    m_setting_data.color_low = GetPrivateProfileInt(L"config", L"color_low", RGB(241, 197, 0), m_config_path.c_str());
    m_setting_data.color_critical = GetPrivateProfileInt(L"config", L"color_critical", RGB(206, 23, 0), m_config_path.c_str());
    m_setting_data.progress_style = static_cast<ProgressStyle>(GetPrivateProfileInt(L"config", L"progress_style", 1, m_config_path.c_str()));
    m_setting_data.font_size = GetPrivateProfileInt(L"config", L"font_size", 16, m_config_path.c_str());
    m_setting_data.time_format = static_cast<TimeFormat>(GetPrivateProfileInt(L"config", L"time_format", 0, m_config_path.c_str()));
    m_setting_data.text_color = GetPrivateProfileInt(L"config", L"text_color", RGB(255, 255, 255), m_config_path.c_str());
    m_setting_data.show_label = (GetPrivateProfileInt(L"config", L"show_label", 0, m_config_path.c_str()) != 0);
    m_setting_data.low_battery_warning = (GetPrivateProfileInt(L"config", L"low_battery_warning", 0, m_config_path.c_str()) != 0);
    m_setting_data.low_battery_threshold = GetPrivateProfileInt(L"config", L"low_battery_threshold", 20, m_config_path.c_str());
    m_setting_data.show_battery_in_tooltip = (GetPrivateProfileInt(L"config", L"show_battery_in_tooltip", 1, m_config_path.c_str()) != 0);
    m_setting_data.auto_theme = (GetPrivateProfileInt(L"config", L"auto_theme", 0, m_config_path.c_str()) != 0);
    m_setting_data.text_shadow = (GetPrivateProfileInt(L"config", L"text_shadow", 1, m_config_path.c_str()) != 0);
    m_setting_data.smooth_color = (GetPrivateProfileInt(L"config", L"smooth_color", 1, m_config_path.c_str()) != 0);
    m_setting_data.refresh_interval = GetPrivateProfileInt(L"config", L"refresh_interval", 1000, m_config_path.c_str());
    m_setting_data.sound_on_full = (GetPrivateProfileInt(L"config", L"sound_on_full", 0, m_config_path.c_str()) != 0);
    m_setting_data.sound_on_low = (GetPrivateProfileInt(L"config", L"sound_on_low", 0, m_config_path.c_str()) != 0);
    m_setting_data.show_days = (GetPrivateProfileInt(L"config", L"show_days", 0, m_config_path.c_str()) != 0);
    m_setting_data.show_seconds = (GetPrivateProfileInt(L"config", L"show_seconds", 0, m_config_path.c_str()) != 0);
    m_setting_data.hide_zero = (GetPrivateProfileInt(L"config", L"hide_zero", 0, m_config_path.c_str()) != 0);

    wchar_t buf_fmt[256];
    GetPrivateProfileString(L"config", L"custom_format", L"{h}h {m}m", buf_fmt, 256, m_config_path.c_str());
    m_setting_data.custom_format = buf_fmt;

    wchar_t buf[256];
    GetPrivateProfileString(L"config", L"label_text", L"", buf, 256, m_config_path.c_str());
    m_setting_data.label_text = buf;
}

void CDataManager::SaveConfig() const
{
    if (!m_config_path.empty())
    {
        WritePrivateProfileInt(L"config", L"preview_mode", m_setting_data.preview_mode, m_config_path.c_str());
        WritePrivateProfileInt(L"config", L"progress_bar_above", m_setting_data.progress_bar_above, m_config_path.c_str());
        WritePrivateProfileInt(L"config", L"progress_bar_height", m_setting_data.progress_bar_height, m_config_path.c_str());
        WritePrivateProfileInt(L"config", L"progress_bar_max_width", m_setting_data.progress_bar_max_width, m_config_path.c_str());
        WritePrivateProfileInt(L"config", L"color_high", m_setting_data.color_high, m_config_path.c_str());
        WritePrivateProfileInt(L"config", L"color_low", m_setting_data.color_low, m_config_path.c_str());
        WritePrivateProfileInt(L"config", L"color_critical", m_setting_data.color_critical, m_config_path.c_str());
        WritePrivateProfileInt(L"config", L"progress_style", static_cast<int>(m_setting_data.progress_style), m_config_path.c_str());
        WritePrivateProfileInt(L"config", L"font_size", m_setting_data.font_size, m_config_path.c_str());
        WritePrivateProfileInt(L"config", L"time_format", static_cast<int>(m_setting_data.time_format), m_config_path.c_str());
        WritePrivateProfileInt(L"config", L"text_color", m_setting_data.text_color, m_config_path.c_str());
        WritePrivateProfileInt(L"config", L"show_label", m_setting_data.show_label, m_config_path.c_str());
        WritePrivateProfileInt(L"config", L"low_battery_warning", m_setting_data.low_battery_warning, m_config_path.c_str());
        WritePrivateProfileInt(L"config", L"low_battery_threshold", m_setting_data.low_battery_threshold, m_config_path.c_str());
        WritePrivateProfileInt(L"config", L"show_battery_in_tooltip", m_setting_data.show_battery_in_tooltip, m_config_path.c_str());
        WritePrivateProfileInt(L"config", L"auto_theme", m_setting_data.auto_theme, m_config_path.c_str());
        WritePrivateProfileInt(L"config", L"text_shadow", m_setting_data.text_shadow, m_config_path.c_str());
        WritePrivateProfileInt(L"config", L"smooth_color", m_setting_data.smooth_color, m_config_path.c_str());
        WritePrivateProfileInt(L"config", L"refresh_interval", m_setting_data.refresh_interval, m_config_path.c_str());
        WritePrivateProfileInt(L"config", L"sound_on_full", m_setting_data.sound_on_full, m_config_path.c_str());
        WritePrivateProfileInt(L"config", L"sound_on_low", m_setting_data.sound_on_low, m_config_path.c_str());
        WritePrivateProfileInt(L"config", L"show_days", m_setting_data.show_days, m_config_path.c_str());
        WritePrivateProfileInt(L"config", L"show_seconds", m_setting_data.show_seconds, m_config_path.c_str());
        WritePrivateProfileInt(L"config", L"hide_zero", m_setting_data.hide_zero, m_config_path.c_str());
        WritePrivateProfileString(L"config", L"custom_format", m_setting_data.custom_format.c_str(), m_config_path.c_str());

        WritePrivateProfileString(L"config", L"label_text", m_setting_data.label_text.c_str(), m_config_path.c_str());
    }
}

const CString& CDataManager::StringRes(UINT id)
{
    auto iter = m_string_table.find(id);
    if (iter != m_string_table.end())
    {
        return iter->second;
    }
    else
    {
        AFX_MANAGE_STATE(AfxGetStaticModuleState());
        m_string_table[id].LoadString(id);
        return m_string_table[id];
    }
}

void CDataManager::DPIFromWindow(CWnd* pWnd)
{
    CWindowDC dc(pWnd);
    HDC hDC = dc.GetSafeHdc();
    m_dpi = GetDeviceCaps(hDC, LOGPIXELSY);
}

int CDataManager::DPI(int pixel)
{
    return m_dpi * pixel / 96;
}

float CDataManager::DPIF(float pixel)
{
    return m_dpi * pixel / 96;
}

int CDataManager::RDPI(int pixel)
{
    return pixel * 96 / m_dpi;
}

bool CDataManager::IsAcOnline() const
{
    return m_sysPowerStatus.ACLineStatus == 1;
}

bool CDataManager::IsDarkMode() const
{
    DWORD value = 1;
    DWORD size = sizeof(value);
    DWORD type = REG_DWORD;
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_CURRENT_USER,
        L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
        0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        RegQueryValueExW(hKey, L"AppsUseLightTheme", NULL, &type, (LPBYTE)&value, &size);
        RegCloseKey(hKey);
    }
    return value == 0;
}

COLORREF CDataManager::GetTextColor() const
{
    if (m_setting_data.auto_theme)
    {
        return IsDarkMode() ? RGB(255, 255, 255) : RGB(0, 0, 0);
    }
    return m_setting_data.text_color;
}

void CDataManager::UpdatePreview()
{
    if (!m_setting_data.preview_mode)
        return;

    DWORD now = GetTickCount();
    if (m_last_preview_time == 0)
        m_last_preview_time = now;

    if (now - m_last_preview_time >= 2000)
    {
        m_last_preview_time = now;
        m_preview_step++;
        if (m_preview_step > 4)
            m_preview_step = 0;

        switch (m_preview_step)
        {
        case 0: m_preview_percent = 100.0; break;
        case 1: m_preview_percent = 80.0; break;
        case 2: m_preview_percent = 50.0; break;
        case 3: m_preview_percent = 20.0; break;
        case 4: m_preview_percent = 5.0; break;
        }
    }

    double hours = m_preview_percent / 100.0 * 8.0;
    int total_sec = static_cast<int>(hours * 3600);
    int d = total_sec / 86400;
    int h = (total_sec % 86400) / 3600;
    int m = (total_sec % 3600) / 60;
    int s = total_sec % 60;

    if (m_setting_data.hide_zero && !m_setting_data.show_seconds)
    {
        if (d == 0 && h == 0 && m == 0)
            m = 1;
    }

    if (!m_setting_data.show_days)
    {
        h += d * 24;
        d = 0;
    }

    std::wstring fmt = m_setting_data.custom_format;

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

    m_time_string = fmt;
}

bool CDataManager::IsPreviewActive() const
{
    return m_setting_data.preview_mode;
}
