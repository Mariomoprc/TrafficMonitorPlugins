#include "pch.h"
#include "BatteryTime.h"
#include "OptionsDlg.h"
#include "afxdialogex.h"
#include "DataManager.h"
#include <sstream>

IMPLEMENT_DYNAMIC(COptionsDlg, CDialog)

COptionsDlg::COptionsDlg(CWnd* pParent)
    : CDialog(IDD_OPTIONS_DIALOG, pParent)
{
}

COptionsDlg::~COptionsDlg()
{
}

void COptionsDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_PROGRESS_STYLE_COMBO, m_progress_style_combo);
    DDX_Control(pDX, IDC_TIME_FORMAT_COMBO, m_time_format_combo);
    DDX_Control(pDX, IDC_PROGRESS_HEIGHT_EDIT, m_progress_height_edit);
    DDX_Control(pDX, IDC_PROGRESS_MAX_WIDTH_EDIT, m_progress_max_width_edit);
    DDX_Control(pDX, IDC_FONT_SIZE_EDIT, m_font_size_edit);
    DDX_Control(pDX, IDC_LABEL_TEXT_EDIT, m_label_text_edit);
    DDX_Control(pDX, IDC_LOW_BATTERY_THRESHOLD_EDIT, m_low_battery_threshold_edit);
    DDX_Control(pDX, IDC_CUSTOM_FORMAT_EDIT, m_custom_format_edit);
    DDX_Control(pDX, IDC_POMO_WORK_EDIT, m_pomo_work_edit);
    DDX_Control(pDX, IDC_POMO_BREAK_EDIT, m_pomo_break_edit);
    DDX_Control(pDX, IDC_PREVIEW_STATIC, m_preview_static);
}

BEGIN_MESSAGE_MAP(COptionsDlg, CDialog)
    ON_BN_CLICKED(IDC_PREVIEW_CHECK, &COptionsDlg::OnBnClickedPreviewCheck)
    ON_BN_CLICKED(IDC_PROGRESS_ABOVE_RADIO, &COptionsDlg::OnBnClickedProgressAboveRadio)
    ON_BN_CLICKED(IDC_PROGRESS_BELOW_RADIO, &COptionsDlg::OnBnClickedProgressBelowRadio)
    ON_EN_CHANGE(IDC_PROGRESS_HEIGHT_EDIT, &COptionsDlg::OnEnChangeProgressHeightEdit)
    ON_EN_CHANGE(IDC_PROGRESS_MAX_WIDTH_EDIT, &COptionsDlg::OnEnChangeProgressMaxWidthEdit)
    ON_CBN_SELCHANGE(IDC_PROGRESS_STYLE_COMBO, &COptionsDlg::OnCbnSelchangeProgressStyleCombo)
    ON_EN_CHANGE(IDC_FONT_SIZE_EDIT, &COptionsDlg::OnEnChangeFontSizeEdit)
    ON_CBN_SELCHANGE(IDC_TIME_FORMAT_COMBO, &COptionsDlg::OnCbnSelchangeTimeFormatCombo)
    ON_BN_CLICKED(IDC_SHOW_LABEL_CHECK, &COptionsDlg::OnBnClickedShowLabelCheck)
    ON_EN_CHANGE(IDC_LABEL_TEXT_EDIT, &COptionsDlg::OnEnChangeLabelTextEdit)
    ON_BN_CLICKED(IDC_LOW_BATTERY_CHECK, &COptionsDlg::OnBnClickedLowBatteryCheck)
    ON_EN_CHANGE(IDC_LOW_BATTERY_THRESHOLD_EDIT, &COptionsDlg::OnEnChangeLowBatteryThresholdEdit)
    ON_BN_CLICKED(IDC_SHOW_TOOLTIP_CHECK, &COptionsDlg::OnBnClickedShowTooltipCheck)
    ON_BN_CLICKED(IDC_APPLY_BUTTON, &COptionsDlg::OnBnClickedApplyButton)
    ON_WM_TIMER()
    ON_WM_CTLCOLOR()
    ON_BN_CLICKED(IDC_SHOW_DAYS_CHECK, &COptionsDlg::OnBnClickedShowDaysCheck)
    ON_BN_CLICKED(IDC_SHOW_SECONDS_CHECK, &COptionsDlg::OnBnClickedShowSecondsCheck)
    ON_BN_CLICKED(IDC_HIDE_ZERO_CHECK, &COptionsDlg::OnBnClickedHideZeroCheck)
    ON_EN_CHANGE(IDC_CUSTOM_FORMAT_EDIT, &COptionsDlg::OnEnChangeCustomFormatEdit)
    ON_BN_CLICKED(IDC_POMODORO_CHECK, &COptionsDlg::OnBnClickedPomodoroCheck)
    ON_EN_CHANGE(IDC_POMO_WORK_EDIT, &COptionsDlg::OnEnChangePomoWorkEdit)
    ON_EN_CHANGE(IDC_POMO_BREAK_EDIT, &COptionsDlg::OnEnChangePomoBreakEdit)
    ON_BN_CLICKED(IDC_POMO_CYCLE_CHECK, &COptionsDlg::OnBnClickedPomoCycleCheck)
    ON_BN_CLICKED(IDC_POMO_NOTIFY_CHECK, &COptionsDlg::OnBnClickedPomoNotifyCheck)
END_MESSAGE_MAP()

BOOL COptionsDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    m_progress_style_combo.AddString(L"\x5B9E\x5FC3");
    m_progress_style_combo.AddString(L"\x6E10\x53D8");
    m_progress_style_combo.AddString(L"\x5706\x70B9");
    m_progress_style_combo.SetCurSel(static_cast<int>(m_data.progress_style));

    m_time_format_combo.AddString(L"Xh Xm");
    m_time_format_combo.AddString(L"X:MM");
    m_time_format_combo.AddString(L"HH:MM");
    m_time_format_combo.AddString(L"X\x5C0F\x65F6X\x5206");
    m_time_format_combo.AddString(L"Xd Xh Xm");
    m_time_format_combo.AddString(L"Xm Xs");
    m_time_format_combo.AddString(L"\x81EA\x5B9A\x4E49");
    m_time_format_combo.SetCurSel(static_cast<int>(m_data.time_format));

    CheckDlgButton(IDC_PREVIEW_CHECK, m_data.preview_mode);
    CheckDlgButton(m_data.progress_bar_above ? IDC_PROGRESS_ABOVE_RADIO : IDC_PROGRESS_BELOW_RADIO, BST_CHECKED);
    CheckDlgButton(IDC_SHOW_LABEL_CHECK, m_data.show_label);
    CheckDlgButton(IDC_LOW_BATTERY_CHECK, m_data.low_battery_warning);
    CheckDlgButton(IDC_SHOW_TOOLTIP_CHECK, m_data.show_battery_in_tooltip);
    CheckDlgButton(IDC_SHOW_DAYS_CHECK, m_data.show_days);
    CheckDlgButton(IDC_SHOW_SECONDS_CHECK, m_data.show_seconds);
    CheckDlgButton(IDC_HIDE_ZERO_CHECK, m_data.hide_zero);
    CheckDlgButton(IDC_POMODORO_CHECK, m_data.pomodoro_enabled);
    CheckDlgButton(IDC_POMO_CYCLE_CHECK, m_data.pomodoro_auto_cycle);
    CheckDlgButton(IDC_POMO_NOTIFY_CHECK, m_data.pomodoro_notify);

    CString str;
    str.Format(L"%d", m_data.progress_bar_height);
    m_progress_height_edit.SetWindowText(str);
    str.Format(L"%d", m_data.progress_bar_max_width);
    m_progress_max_width_edit.SetWindowText(str);
    str.Format(L"%d", m_data.font_size);
    m_font_size_edit.SetWindowText(str);
    m_label_text_edit.SetWindowText(m_data.label_text.c_str());
    str.Format(L"%d", m_data.low_battery_threshold);
    m_low_battery_threshold_edit.SetWindowText(str);
    m_custom_format_edit.SetWindowText(m_data.custom_format.c_str());
    str.Format(L"%d", m_data.pomodoro_work_min);
    m_pomo_work_edit.SetWindowText(str);
    str.Format(L"%d", m_data.pomodoro_break_min);
    m_pomo_break_edit.SetWindowText(str);

    SetTimer(1, 1500, NULL);

    return TRUE;
}

void COptionsDlg::RefreshPreview()
{
    DrawPreview();
}

std::wstring COptionsDlg::FormatTimeForPreview(double percent) const
{
    int total_sec = static_cast<int>(percent / 100.0 * 8.0 * 3600);
    int d = total_sec / 86400;
    int h = (total_sec % 86400) / 3600;
    int m = (total_sec % 3600) / 60;
    int s = total_sec % 60;

    if (m_data.hide_zero)
    {
        if (d == 0 && h == 0 && m == 0 && s == 0)
            s = 0;
        if (m_data.show_seconds && !m_data.show_days)
        {
            if (h == 0 && m == 0)
            {
                m = s / 60;
                s = s % 60;
            }
        }
    }

    std::wstringstream wss;
    std::wstring fmt = m_data.custom_format;

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

static COLORREF PreviewLerpColor(COLORREF c1, COLORREF c2, double t)
{
    if (t < 0.0) t = 0.0;
    if (t > 1.0) t = 1.0;
    int r = static_cast<int>(GetRValue(c1) + (GetRValue(c2) - GetRValue(c1)) * t);
    int g = static_cast<int>(GetGValue(c1) + (GetGValue(c2) - GetGValue(c1)) * t);
    int b = static_cast<int>(GetBValue(c1) + (GetBValue(c2) - GetBValue(c1)) * t);
    return RGB(r, g, b);
}

static COLORREF PreviewGetProgressColor(const SettingData& data, double percent)
{
    if (data.smooth_color)
    {
        if (percent > 60)
            return PreviewLerpColor(data.color_low, data.color_high, (percent - 60.0) / 40.0);
        else if (percent > 20)
            return PreviewLerpColor(data.color_critical, data.color_low, (percent - 20.0) / 40.0);
        else
            return data.color_critical;
    }
    else
    {
        if (percent > 60) return data.color_high;
        else if (percent > 20) return data.color_low;
        else return data.color_critical;
    }
}

void COptionsDlg::DrawPreview()
{
    if (!m_preview_static.GetSafeHwnd())
        return;

    CDC* pDC = m_preview_static.GetDC();
    CRect rect;
    m_preview_static.GetClientRect(&rect);

    pDC->FillSolidRect(&rect, RGB(30, 30, 30));

    HDC hDC = ::GetDC(HWND_DESKTOP);
    int dpi = GetDeviceCaps(hDC, LOGPIXELSY);
    ::ReleaseDC(HWND_DESKTOP, hDC);
    auto DPI = [&](int px) { return dpi * px / 96; };

    double percent = m_preview_percent;

    int font_height = DPI(m_data.font_size);
    int bar_height = DPI(m_data.progress_bar_height);
    if (bar_height < 1) bar_height = 1;
    int extra = DPI(m_data.progress_bar_max_width);

    bool progress_above = m_data.progress_bar_above;

    CFont font;
    font.CreateFont(
        font_height, 0, 0, 0,
        FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");

    CFont* pOldFont = pDC->SelectObject(&font);
    pDC->SetBkMode(TRANSPARENT);

    std::wstring time_str = FormatTimeForPreview(percent);

    CString display_str;
    if (m_data.show_label)
        display_str = m_data.label_text.c_str();
    display_str += time_str.c_str();

    CSize text_size = pDC->GetTextExtent(display_str);
    int text_width = text_size.cx;

    int max_bar_width = text_width + extra;
    if (max_bar_width < DPI(20))
        max_bar_width = DPI(20);
    int progress_width = static_cast<int>(max_bar_width * percent / 100.0);
    if (progress_width < 0) progress_width = 0;

    int spacing = DPI(2);
    int total_h = font_height + spacing + bar_height;
    int content_y = rect.top + (rect.Height() - total_h) / 2;
    if (content_y < 0) content_y = 0;

    CRect draw_text_rect;
    int progress_y;
    if (progress_above)
    {
        progress_y = content_y;
        draw_text_rect = CRect(0, content_y + bar_height + spacing,
            rect.Width(), content_y + bar_height + spacing + font_height);
    }
    else
    {
        draw_text_rect = CRect(0, content_y,
            rect.Width(), content_y + font_height);
        progress_y = content_y + font_height + spacing;
    }

    COLORREF text_color = RGB(200, 200, 200);
    if (m_data.low_battery_warning && percent <= m_data.low_battery_threshold)
        text_color = m_data.color_critical;

    if (m_data.text_shadow)
    {
        pDC->SetTextColor(RGB(0, 0, 0));
        CRect shadow_rect = draw_text_rect;
        shadow_rect.OffsetRect(1, 1);
        pDC->DrawText(display_str, shadow_rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
    }

    pDC->SetTextColor(text_color);
    pDC->DrawText(display_str, draw_text_rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    int progress_x = (rect.Width() - max_bar_width) / 2;
    if (progress_x < 0) progress_x = 0;

    COLORREF color = PreviewGetProgressColor(m_data, percent);

    switch (m_data.progress_style)
    {
    case ProgressStyle::SOLID:
    {
        if (progress_width > 0)
        {
            CBrush fg_brush(color);
            CRect bar_rect(progress_x, progress_y, progress_x + progress_width, progress_y + bar_height);
            pDC->FillRect(&bar_rect, &fg_brush);
        }
        break;
    }
    case ProgressStyle::GRADIENT:
    {
        if (progress_width > 0)
        {
            for (int i = 0; i < progress_width; i++)
            {
                float ratio = static_cast<float>(i) / progress_width;
                int r = static_cast<int>(GetRValue(color) * (0.5f + 0.5f * ratio));
                int g = static_cast<int>(GetGValue(color) * (0.5f + 0.5f * ratio));
                int b = static_cast<int>(GetBValue(color) * (0.5f + 0.5f * ratio));
                for (int row = 0; row < bar_height; row++)
                    pDC->SetPixelV(progress_x + i, progress_y + row, RGB(r, g, b));
            }
        }
        break;
    }
    case ProgressStyle::DOTS:
    {
        if (progress_width > 0)
        {
            CBrush brush(color);
            int dot_size = (bar_height < 3) ? bar_height : 3;
            int dot_spacing = DPI(4);
            for (int i = 0; i < progress_width; i += dot_spacing)
            {
                CRect dot_rect(progress_x + i, progress_y,
                    progress_x + i + dot_size, progress_y + dot_size);
                if (dot_rect.right > progress_x + progress_width)
                    dot_rect.right = progress_x + progress_width;
                pDC->FillRect(&dot_rect, &brush);
            }
        }
        break;
    }
    }

    pDC->SelectObject(pOldFont);
    m_preview_static.ReleaseDC(pDC);
}

void COptionsDlg::OnTimer(UINT_PTR nIDEvent)
{
    if (nIDEvent == 1)
    {
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

        DrawPreview();
    }
    CDialog::OnTimer(nIDEvent);
}

HBRUSH COptionsDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
    if (pWnd->GetDlgCtrlID() == IDC_PREVIEW_STATIC)
    {
        static CBrush brush(RGB(30, 30, 30));
        pDC->SetBkColor(RGB(30, 30, 30));
        pDC->SetTextColor(RGB(200, 200, 200));
        return brush;
    }
    return CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
}

void COptionsDlg::OnBnClickedPreviewCheck()
{
    m_data.preview_mode = (IsDlgButtonChecked(IDC_PREVIEW_CHECK) != 0);
    RefreshPreview();
}

void COptionsDlg::OnBnClickedProgressAboveRadio()
{
    m_data.progress_bar_above = true;
    RefreshPreview();
}

void COptionsDlg::OnBnClickedProgressBelowRadio()
{
    m_data.progress_bar_above = false;
    RefreshPreview();
}

void COptionsDlg::OnEnChangeProgressHeightEdit()
{
    CString text;
    m_progress_height_edit.GetWindowText(text);
    int value = _ttoi(text);
    if (value > 0 && value <= 20)
    {
        m_data.progress_bar_height = value;
        RefreshPreview();
    }
}

void COptionsDlg::OnEnChangeProgressMaxWidthEdit()
{
    CString text;
    m_progress_max_width_edit.GetWindowText(text);
    int value = _ttoi(text);
    if (value >= 0 && value <= 500)
    {
        m_data.progress_bar_max_width = value;
        RefreshPreview();
    }
}

void COptionsDlg::OnCbnSelchangeProgressStyleCombo()
{
    m_data.progress_style = static_cast<ProgressStyle>(m_progress_style_combo.GetCurSel());
    RefreshPreview();
}

void COptionsDlg::OnEnChangeFontSizeEdit()
{
    CString text;
    m_font_size_edit.GetWindowText(text);
    int value = _ttoi(text);
    if (value > 0 && value <= 72)
    {
        m_data.font_size = value;
        RefreshPreview();
    }
}

void COptionsDlg::OnCbnSelchangeTimeFormatCombo()
{
    int sel = m_time_format_combo.GetCurSel();
    m_data.time_format = static_cast<TimeFormat>(sel);

    switch (sel)
    {
    case 0: m_data.custom_format = L"{h}h {m}m"; break;
    case 1: m_data.custom_format = L"{h}:{m:02}"; break;
    case 2: m_data.custom_format = L"{h:02}:{m:02}"; break;
    case 3: m_data.custom_format = L"{h}\x5C0F\x65F6{m}\x5206"; break;
    case 4: m_data.custom_format = L"{d}d {h}h {m}m"; break;
    case 5: m_data.custom_format = L"{m}m {s}s"; break;
    case 6: break;
    }
    m_custom_format_edit.SetWindowText(m_data.custom_format.c_str());
    RefreshPreview();
}

void COptionsDlg::OnBnClickedShowLabelCheck()
{
    m_data.show_label = (IsDlgButtonChecked(IDC_SHOW_LABEL_CHECK) != 0);
    RefreshPreview();
}

void COptionsDlg::OnEnChangeLabelTextEdit()
{
    CString text;
    m_label_text_edit.GetWindowText(text);
    m_data.label_text = text.GetString();
    RefreshPreview();
}

void COptionsDlg::OnBnClickedLowBatteryCheck()
{
    m_data.low_battery_warning = (IsDlgButtonChecked(IDC_LOW_BATTERY_CHECK) != 0);
    RefreshPreview();
}

void COptionsDlg::OnEnChangeLowBatteryThresholdEdit()
{
    CString text;
    m_low_battery_threshold_edit.GetWindowText(text);
    int value = _ttoi(text);
    if (value >= 0 && value <= 100)
    {
        m_data.low_battery_threshold = value;
        RefreshPreview();
    }
}

void COptionsDlg::OnBnClickedShowTooltipCheck()
{
    m_data.show_battery_in_tooltip = (IsDlgButtonChecked(IDC_SHOW_TOOLTIP_CHECK) != 0);
    RefreshPreview();
}

void COptionsDlg::OnBnClickedApplyButton()
{
    g_data.m_setting_data = m_data;
    g_data.SaveConfig();
}

void COptionsDlg::OnBnClickedShowDaysCheck()
{
    m_data.show_days = (IsDlgButtonChecked(IDC_SHOW_DAYS_CHECK) != 0);
    RefreshPreview();
}

void COptionsDlg::OnBnClickedShowSecondsCheck()
{
    m_data.show_seconds = (IsDlgButtonChecked(IDC_SHOW_SECONDS_CHECK) != 0);
    RefreshPreview();
}

void COptionsDlg::OnBnClickedHideZeroCheck()
{
    m_data.hide_zero = (IsDlgButtonChecked(IDC_HIDE_ZERO_CHECK) != 0);
    RefreshPreview();
}

void COptionsDlg::OnEnChangeCustomFormatEdit()
{
    CString text;
    m_custom_format_edit.GetWindowText(text);
    m_data.custom_format = text.GetString();
    m_time_format_combo.SetCurSel(6);
    m_data.time_format = TimeFormat::HM;
    RefreshPreview();
}

void COptionsDlg::OnBnClickedPomodoroCheck()
{
    m_data.pomodoro_enabled = (IsDlgButtonChecked(IDC_POMODORO_CHECK) != 0);
}

void COptionsDlg::OnEnChangePomoWorkEdit()
{
    CString text;
    m_pomo_work_edit.GetWindowText(text);
    int value = _ttoi(text);
    if (value >= 1 && value <= 180)
        m_data.pomodoro_work_min = value;
}

void COptionsDlg::OnEnChangePomoBreakEdit()
{
    CString text;
    m_pomo_break_edit.GetWindowText(text);
    int value = _ttoi(text);
    if (value >= 1 && value <= 60)
        m_data.pomodoro_break_min = value;
}

void COptionsDlg::OnBnClickedPomoCycleCheck()
{
    m_data.pomodoro_auto_cycle = (IsDlgButtonChecked(IDC_POMO_CYCLE_CHECK) != 0);
}

void COptionsDlg::OnBnClickedPomoNotifyCheck()
{
    m_data.pomodoro_notify = (IsDlgButtonChecked(IDC_POMO_NOTIFY_CHECK) != 0);
}

BOOL COptionsDlg::PreTranslateMessage(MSG* pMsg)
{
    if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN)
    {
        if (GetFocus() && GetFocus()->GetDlgCtrlID() == IDC_APPLY_BUTTON)
        {
            OnBnClickedApplyButton();
            return TRUE;
        }
    }
    return CDialog::PreTranslateMessage(pMsg);
}
