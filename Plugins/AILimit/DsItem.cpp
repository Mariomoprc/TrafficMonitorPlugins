#include "DsItem.h"
#include "Utils.h"
#include <windows.h>
#include <shellapi.h>
extern void AILimit_ForceRefresh();

CDsItem::CDsItem(bool isTop, CAIConfig* cfg, DsData* pData, double* pSpent)
    : m_isTop(isTop), m_cfg(cfg), m_pData(pData), m_pSpent(pSpent) {
    m_name = isTop ? L"DS Top" : L"DS Bot";
    m_id = isTop ? L"ds_top" : L"ds_bottom";
}

const wchar_t* CDsItem::GetItemName() const { return m_name.c_str(); }
const wchar_t* CDsItem::GetItemId() const { return m_id.c_str(); }
const wchar_t* CDsItem::GetItemLableText() const { return m_label.c_str(); }

static wchar_t s_dsTopText[64];
static wchar_t s_dsBotText[64];

static void DsCur(std::wstring& out, const std::string& currency, double v, bool oneDecimal) {
    const wchar_t* sym = (currency == "CNY") ? L"¥" : L"$";
    if (oneDecimal) swprintf_s(s_dsBotText, L"-%ls%.1f", sym, v);
    else swprintf_s(s_dsTopText, L"%ls%.0f", sym, v);
    out = oneDecimal ? s_dsBotText : s_dsTopText;
}

const wchar_t* CDsItem::GetItemValueText() const {
    if (m_hidden) return L"";
    if (m_pData && m_pData->valid) {
        static thread_local std::wstring cache;
        if (m_isTop) {
            DsCur(cache, m_pData->currency, m_pData->total, false);
            return cache.c_str();
        } else {
            double spent = m_pSpent ? *m_pSpent : 0;
            if (spent > 0.001) {
                DsCur(cache, m_pData->currency, spent, true);
                return cache.c_str();
            }
            return L"-";
        }
    }
    return L"--";
}
const wchar_t* CDsItem::GetItemValueSampleText() const {
    return m_isTop ? L"¥110" : L"-¥2.3";
}

int CDsItem::OnMouseEvent(MouseEventType type, int x, int y, void* hWnd, int flag) {
    if (type == MT_DBCLICKED) {
        ShellExecuteW(nullptr, L"open", L"https://platform.deepseek.com", nullptr, nullptr, SW_SHOWNORMAL);
        return 1;
    }
    if (type == MT_WHEEL_UP || type == MT_WHEEL_DOWN) { AILimit_ForceRefresh(); return 1; }
    return 0;
}

int CDsItem::GetItemWidthEx(void* hDC) const {
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

void CDsItem::DrawItem(void* hDC, int x, int y, int w, int h, bool dark_mode) {
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
