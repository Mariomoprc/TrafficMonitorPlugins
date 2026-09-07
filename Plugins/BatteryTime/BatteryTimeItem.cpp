#include "pch.h"
#include "BatteryTimeItem.h"
#include "DataManager.h"
#include <sstream>

CBatteryTimeItem::CBatteryTimeItem()
{
}

const wchar_t* CBatteryTimeItem::GetItemName() const
{
    return g_data.StringRes(IDS_BATTERY_TIME);
}

const wchar_t* CBatteryTimeItem::GetItemId() const
{
    return L"BT001TIME";
}

const wchar_t* CBatteryTimeItem::GetItemLableText() const
{
    return L"";
}

const wchar_t* CBatteryTimeItem::GetItemValueText() const
{
    return g_data.m_time_string.c_str();
}

const wchar_t* CBatteryTimeItem::GetItemValueSampleText() const
{
    return L"99h 59m";
}

bool CBatteryTimeItem::IsCustomDraw() const
{
    return true;
}

int CBatteryTimeItem::GetItemWidthEx(void* hDC) const
{
    CDC* pDC = CDC::FromHandle((HDC)hDC);

    CFont font;
    font.CreateFont(
        g_data.DPI(g_data.m_setting_data.font_size), 0, 0, 0,
        FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");

    CFont* pOldFont = pDC->SelectObject(&font);

    CString sample_str;
    if (g_data.m_setting_data.show_label)
        sample_str = g_data.m_setting_data.label_text.c_str();
    sample_str += L"99h 59m";

    CSize text_size = pDC->GetTextExtent(sample_str);
    pDC->SelectObject(pOldFont);

    int extra = g_data.DPI(g_data.m_setting_data.progress_bar_max_width);
    int width = text_size.cx + extra;
    if (g_data.m_setting_data.progress_style == ProgressStyle::RING)
    {
        int diameter = g_data.DPI(g_data.m_setting_data.font_size) + g_data.DPI(10);
        int need = text_size.cx + diameter + g_data.DPI(6);
        if (width < need)
            width = need;
    }
    if (width < g_data.DPI(20))
        width = g_data.DPI(20);
    return width;
}

static COLORREF LerpColor(COLORREF c1, COLORREF c2, double t)
{
    if (t < 0.0) t = 0.0;
    if (t > 1.0) t = 1.0;
    int r = static_cast<int>(GetRValue(c1) + (GetRValue(c2) - GetRValue(c1)) * t);
    int g = static_cast<int>(GetGValue(c1) + (GetGValue(c2) - GetGValue(c1)) * t);
    int b = static_cast<int>(GetBValue(c1) + (GetBValue(c2) - GetBValue(c1)) * t);
    return RGB(r, g, b);
}

void CBatteryTimeItem::DrawItem(void* hDC, int x, int y, int w, int h, bool dark_mode)
{
    CDC* pDC = CDC::FromHandle((HDC)hDC);
    CRect rect(CPoint(x, y), CSize(w, h));

    double percent = 0.0;
    std::wstring time_str = g_data.m_time_string;
    bool charging = false;
    if (g_data.IsPreviewActive())
    {
        percent = g_data.m_preview_percent;
    }
    else if (g_data.IsAcOnline())
    {
        if (!g_data.m_setting_data.show_charging_display)
            return;
        charging = true;
        if (g_data.m_sysPowerStatus.BatteryFlag != 128)
            percent = g_data.m_sysPowerStatus.BatteryLifePercent;
        else
            percent = 100.0;
        time_str = g_data.StringRes(IDS_BATTERY_TIME_CHARGING).GetString();
    }
    else
    {
        if (g_data.m_sysPowerStatus.BatteryFlag != 128)
            percent = g_data.m_sysPowerStatus.BatteryLifePercent;
    }

    int font_height = g_data.DPI(g_data.m_setting_data.font_size);
    int progress_height = g_data.DPI(g_data.m_setting_data.progress_bar_height);
    int spacing = g_data.DPI(2);

    bool progress_above = g_data.m_setting_data.progress_bar_above;

    CRect text_rect = rect;
    if (progress_above)
        text_rect.top = rect.bottom - font_height - g_data.DPI(2);
    else
        text_rect.top = rect.top + g_data.DPI(2);
    text_rect.bottom = text_rect.top + font_height;

    CFont font;
    font.CreateFont(
        font_height, 0, 0, 0,
        FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");

    CFont* pOldFont = pDC->SelectObject(&font);

    COLORREF text_color = g_data.GetTextColor();

    if (!charging && g_data.m_setting_data.low_battery_warning && percent <= g_data.m_setting_data.low_battery_threshold)
        text_color = g_data.m_setting_data.color_critical;

    pDC->SetBkMode(TRANSPARENT);

    bool is_ring = (g_data.m_setting_data.progress_style == ProgressStyle::RING);
    int ring_d = 0;
    if (is_ring)
    {
        ring_d = font_height + g_data.DPI(8);
        int max_d = rect.Height() - g_data.DPI(4);
        if (ring_d > max_d)
            ring_d = max_d;
        DrawRing(pDC, CRect(rect.left, rect.top, rect.left + ring_d, rect.top + rect.Height()), percent, dark_mode);
        text_rect.left = rect.left + ring_d + g_data.DPI(4);
    }

    CString display_str;
    if (g_data.m_setting_data.show_label)
        display_str = g_data.m_setting_data.label_text.c_str();
    display_str += time_str.c_str();

    if (g_data.m_setting_data.text_shadow)
    {
        pDC->SetTextColor(RGB(0, 0, 0));
        CRect shadow_rect = text_rect;
        shadow_rect.OffsetRect(1, 1);
        pDC->DrawText(display_str, shadow_rect, (is_ring ? DT_LEFT : DT_CENTER) | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
    }

    pDC->SetTextColor(text_color);
    pDC->DrawText(display_str, text_rect, (is_ring ? DT_LEFT : DT_CENTER) | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);

    CSize text_size = pDC->GetTextExtent(display_str);
    int text_width = text_size.cx;
    int extra = g_data.DPI(g_data.m_setting_data.progress_bar_max_width);
    int max_bar_width = text_width + extra;
    if (max_bar_width < g_data.DPI(20))
        max_bar_width = g_data.DPI(20);
    int progress_width = static_cast<int>(max_bar_width * percent / 100.0);
    if (progress_width < 0)
        progress_width = 0;

    int progress_x = rect.left + (rect.Width() - max_bar_width) / 2;
    int progress_y;
    if (progress_above)
        progress_y = rect.top + g_data.DPI(2);
    else
        progress_y = text_rect.bottom + spacing;

    if (!is_ring)
        DrawProgressBar(pDC, progress_x, progress_y, progress_width, progress_height, percent, dark_mode);

    pDC->SelectObject(pOldFont);
}

COLORREF CBatteryTimeItem::GetProgressColor(double percent) const
{
    if (g_data.m_setting_data.smooth_color)
    {
        COLORREF c_green = g_data.m_setting_data.color_high;
        COLORREF c_yellow = g_data.m_setting_data.color_low;
        COLORREF c_red = g_data.m_setting_data.color_critical;

        if (percent > 60)
            return LerpColor(c_yellow, c_green, (percent - 60.0) / 40.0);
        else if (percent > 20)
            return LerpColor(c_red, c_yellow, (percent - 20.0) / 40.0);
        else
            return c_red;
    }
    else
    {
        if (percent > 60)
            return g_data.m_setting_data.color_high;
        else if (percent > 20)
            return g_data.m_setting_data.color_low;
        else
            return g_data.m_setting_data.color_critical;
    }
}

void CBatteryTimeItem::DrawProgressBar(CDC* pDC, int x, int y, int width, int height, double percent, bool dark_mode)
{
    if (height <= 0)
        return;

    COLORREF color = GetProgressColor(percent);
    int max_w = width + g_data.DPI(g_data.m_setting_data.progress_bar_max_width);

    switch (g_data.m_setting_data.progress_style)
    {
    case ProgressStyle::SOLID:
    {
        if (width > 0)
        {
            CBrush fg_brush(color);
            CRect bar_rect(x, y, x + width, y + height);
            pDC->FillRect(&bar_rect, &fg_brush);
        }
        break;
    }
    case ProgressStyle::GRADIENT:
    {
        if (width > 0)
        {
            for (int i = 0; i < width; i++)
            {
                float ratio = static_cast<float>(i) / width;
                int r = static_cast<int>(GetRValue(color) * (0.5f + 0.5f * ratio));
                int g = static_cast<int>(GetGValue(color) * (0.5f + 0.5f * ratio));
                int b = static_cast<int>(GetBValue(color) * (0.5f + 0.5f * ratio));

                CPen pen(PS_SOLID, 1, RGB(r, g, b));
                CPen* pOldPen = pDC->SelectObject(&pen);
                pDC->MoveTo(x + i, y);
                pDC->LineTo(x + i, y + height);
                pDC->SelectObject(pOldPen);
            }
        }
        break;
    }
    case ProgressStyle::DOTS:
    {
        if (width > 0)
        {
            CBrush brush(color);
            int dot_spacing = g_data.DPI(4);
            int dot_size = (height < 3) ? height : 3;
            for (int i = 0; i < width; i += dot_spacing)
            {
                CRect dot_rect(x + i, y, x + i + dot_size, y + dot_size);
                pDC->FillRect(&dot_rect, &brush);
            }
        }
        break;
    }
    case ProgressStyle::RING:
        break;
    }
}

void CBatteryTimeItem::DrawRing(CDC* pDC, const CRect& ring_box, double percent, bool dark_mode)
{
    int side = ring_box.Width() < ring_box.Height() ? ring_box.Width() : ring_box.Height();
    if (side < g_data.DPI(8))
        return;
    int radius = side / 2 - 1;
    int cx = ring_box.left + ring_box.Width() / 2;
    int cy = ring_box.top + ring_box.Height() / 2;

    COLORREF track = dark_mode ? RGB(80, 80, 80) : RGB(200, 200, 200);
    CPen track_pen(PS_SOLID, 2, track);
    CPen* pOldPen = pDC->SelectObject(&track_pen);
    pDC->Arc(cx - radius, cy - radius, cx + radius + 1, cy + radius + 1, cx, cy - radius, cx, cy - radius);
    pDC->SelectObject(pOldPen);

    COLORREF color = GetProgressColor(percent);
    if (g_data.m_setting_data.low_battery_warning && percent <= g_data.m_setting_data.low_battery_threshold)
        color = g_data.m_setting_data.color_critical;
    COLORREF halo = LerpColor(color, dark_mode ? RGB(24, 24, 24) : RGB(235, 235, 235), 0.55);
    const double PI = 3.141592653589793;
    int lit = static_cast<int>(60.0 * percent / 100.0 + 0.5);
    for (int i = 0; i < lit && i < 60; i++)
    {
        double a = -PI / 2.0 + static_cast<double>(i) * 2.0 * PI / 60.0;
        int px = cx + static_cast<int>(radius * cos(a));
        int py = cy + static_cast<int>(radius * sin(a));
        pDC->FillSolidRect(px - 2, py - 2, 5, 5, halo);
        pDC->FillSolidRect(px - 1, py - 1, 3, 3, color);
    }
}
