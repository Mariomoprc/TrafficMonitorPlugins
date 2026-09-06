#include "OrItem.h"
#include "Utils.h"
#include <windows.h>
#include <shellapi.h>
#include <gdiplus.h>
extern void AILimit_ForceRefresh();

COrItem::COrItem(bool isTop, CAIConfig* cfg, OrData* pData)
    : m_isTop(isTop), m_cfg(cfg), m_pData(pData) {
    m_name = isTop ? L"OR Top" : L"OR Bot";
    m_id = isTop ? L"or_top" : L"or_bottom";
}

const wchar_t* COrItem::GetItemName() const { return m_name.c_str(); }
const wchar_t* COrItem::GetItemId() const { return m_id.c_str(); }
const wchar_t* COrItem::GetItemLableText() const { return m_label.c_str(); }
const wchar_t* COrItem::GetItemValueText() const {
    if (m_hidden) return L"";
    if (m_pData && m_pData->valid) {
        static wchar_t buf[64];
        if (m_isTop) {
            // 上排：余额（total_available，OpenRouter /api/v1/credits 返回）
            swprintf_s(buf, L"$%.0f", m_pData->remaining);
            return buf;
        } else {
            // 下排：今日消费
            if (m_pData->usageDaily > 0.001) {
                swprintf_s(buf, L"-$%.1f", m_pData->usageDaily);
                return buf;
            }
            return L"-";
        }
    }
    return L"--";
}
const wchar_t* COrItem::GetItemValueSampleText() const { return L"$99"; }

int COrItem::OnMouseEvent(MouseEventType type, int x, int y, void* hWnd, int flag) {
    if (type == MT_DBCLICKED) {
        ShellExecuteW(nullptr, L"open", L"https://openrouter.ai", nullptr, nullptr, SW_SHOWNORMAL);
        return 1;
    }
    if (type == MT_WHEEL_UP || type == MT_WHEEL_DOWN) { AILimit_ForceRefresh(); return 1; }
    return 0;
}

int COrItem::GetItemWidthEx(void* hDC) const {
    if (m_hidden) return 0;
    if (!hDC) return m_cfg ? m_cfg->widthPx : 0;
    const wchar_t* txt = GetItemValueText();
    if (!txt || !*txt) return m_cfg ? m_cfg->widthPx : 0;
    SIZE sz{};
    GetTextExtentPoint32W((HDC)hDC, txt, (int)wcslen(txt), &sz);
    int w = sz.cx + 12;
    if (m_partnerMaxWidth > w) w = m_partnerMaxWidth;
    return w;
}

void COrItem::UpdateAnimTarget(double pct) {
    if (m_targetPercent == pct) return;
    m_animStartPercent = CurrentAnimatedPercent();
    m_targetPercent = pct;
    m_animStartMs = Utils::GetTickMs();
}

double COrItem::CurrentAnimatedPercent() const {
    ULONGLONG now = Utils::GetTickMs();
    ULONGLONG elapsed = now - m_animStartMs;
    if (elapsed >= 800) return m_targetPercent;
    double t = (double)elapsed / 800.0;
    return m_animStartPercent + (m_targetPercent - m_animStartPercent) * t;
}

void COrItem::DrawItem(void* hDC, int x, int y, int w, int h, bool dark_mode) {
    if (m_hidden) return;
    HDC hdc = (HDC)hDC;
    if (!hdc) return;
    const wchar_t* txt = GetItemValueText();
    if (!txt || !*txt) return;
    unsigned long clr = (m_cfg && m_cfg->hasTaskbarColors) ? m_cfg->taskbarValueColor
        : (dark_mode ? 0x00FFFFFF : 0x00000000);
    int oldBk = SetBkMode(hdc, TRANSPARENT);
    COLORREF oldClr = SetTextColor(hdc, (COLORREF)clr);
    RECT rc{ x, y, x + w, y + h };
    DrawTextW(hdc, txt, -1, &rc, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX);
    SetTextColor(hdc, oldClr);
    SetBkMode(hdc, oldBk);
}
