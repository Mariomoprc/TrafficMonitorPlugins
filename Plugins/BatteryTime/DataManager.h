#pragma once
#include <string>
#include <map>
#include "resource.h"

#define g_data CDataManager::Instance()

enum class ProgressStyle
{
    SOLID,
    GRADIENT,
    DOTS,
    RING
};

enum class TimeFormat
{
    HM,
    HM_CN,
    HHMM
};

struct SettingData
{
    bool preview_mode{};
    bool progress_bar_above{};
    int progress_bar_height{ 5 };
    int progress_bar_max_width{ 30 };
    COLORREF color_high{ RGB(128, 194, 105) };
    COLORREF color_low{ RGB(241, 197, 0) };
    COLORREF color_critical{ RGB(206, 23, 0) };
    ProgressStyle progress_style{ ProgressStyle::GRADIENT };
    int font_size{ 16 };
    TimeFormat time_format{ TimeFormat::HM };
    COLORREF text_color{ RGB(255, 255, 255) };
    bool show_label{};
    std::wstring label_text = L"";
    bool low_battery_warning{};
    int low_battery_threshold{ 20 };
    bool show_battery_in_tooltip{};
    bool auto_theme{};
    bool text_shadow{ true };
    bool smooth_color{ true };
    int refresh_interval{ 1000 };
    bool sound_on_full{};
    bool sound_on_low{};
    bool show_days{};
    bool show_seconds{};
    bool hide_zero{};
    std::wstring custom_format = L"{h}h {m}m";
};

class CDataManager
{
private:
    CDataManager();
    ~CDataManager();

public:
    static CDataManager& Instance();

    void LoadConfig(const std::wstring& config_dir);
    void SaveConfig() const;
    const CString& StringRes(UINT id);
    void DPIFromWindow(CWnd* pWnd);
    int DPI(int pixel);
    float DPIF(float pixel);
    int RDPI(int pixel);

    SettingData m_setting_data;
    SYSTEM_POWER_STATUS m_sysPowerStatus{};
    std::wstring m_time_string;
    double m_preview_percent{ 100.0 };

    void UpdatePreview();
    bool IsPreviewActive() const;
    bool IsAcOnline() const;
    bool IsDarkMode() const;
    COLORREF GetTextColor() const;
    DWORD GetLastFullState() const { return m_last_full_state; }
    DWORD GetLastLowState() const { return m_last_low_state; }
    void SetLastFullState(DWORD s) { m_last_full_state = s; }
    void SetLastLowState(DWORD s) { m_last_low_state = s; };

private:
    static CDataManager m_instance;
    std::wstring m_config_path;
    std::map<UINT, CString> m_string_table;
    int m_dpi{ 96 };
    DWORD m_last_preview_time{};
    int m_preview_step{};
    DWORD m_last_full_state{};
    DWORD m_last_low_state{};
};
