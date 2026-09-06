#include "GoItem.h"
#include "Utils.h"
#include "Log.h"
#include <windows.h>
#include <shellapi.h>
#include <gdiplus.h>
#include <cstdio>
#define GoAILog AIDebugLog

CGoItem::CGoItem(bool isTop, bool isGo2, CAIConfig* cfg, GoData* pData)
    : m_isTop(isTop), m_isGo2(isGo2), m_cfg(cfg), m_pData(pData) {
    if (isGo2) {
        m_name = isTop ? L"Go2 Top" : L"Go2 Bot";
        m_id = isTop ? L"go2_top" : L"go2_bottom";
    } else {
        m_name = isTop ? L"Go Top" : L"Go Bot";
        m_id = isTop ? L"go_top" : L"go_bottom";
    }
    if (cfg) GoAILog(L"CTOR id=%ls widthPx=%d", m_id.c_str(), cfg->widthPx);
}

const wchar_t* CGoItem::GetItemName() const { return m_name.c_str(); }
const wchar_t* CGoItem::GetItemId() const { return m_id.c_str(); }
const wchar_t* CGoItem::GetItemLableText() const { return m_name.c_str(); }
bool CGoItem::IsRateLimited() const {
    if (!m_pData || !m_pData->valid) return false;
    if (m_pData->rateLimited) return true;
    if (m_pData->rolling.status == "rate-limited") return true;
    if (m_pData->weekly.status == "rate-limited") return true;
    if (m_pData->monthly.status == "rate-limited") return true;
    // 兜底：API 未标记 status 但已接近满额，视为限流
    if (m_pData->weekly.percent >= 99 || m_pData->rolling.percent >= 99 || m_pData->monthly.percent >= 99) return true;
    if (m_pData->maxPercent >= 99) return true;
    return false;
}
std::wstring CGoItem::GetRateLimitResetText() const {
    if (!m_pData || !m_pData->valid) return L"";
    auto pick = [&](const GoWindow& w) -> std::wstring {
        if (!w.resetText.empty()) return w.resetText;
        if (!w.resetsAt.empty()) return Utils::FormatResetTime(w.resetsAt);
        return L"";
    };
    if (m_pData->rolling.status == "rate-limited") { auto t=pick(m_pData->rolling); if(!t.empty()) return t; }
    if (m_pData->weekly.status == "rate-limited") { auto t=pick(m_pData->weekly); if(!t.empty()) return t; }
    if (m_pData->monthly.status == "rate-limited") { auto t=pick(m_pData->monthly); if(!t.empty()) return t; }
    // 兜底：已满额但未标记，取最满窗口的时间
    if (m_pData->weekly.percent >= 99) { auto t=pick(m_pData->weekly); if(!t.empty()) return t; }
    if (m_pData->rolling.percent >= 99) { auto t=pick(m_pData->rolling); if(!t.empty()) return t; }
    if (m_pData->monthly.percent >= 99) { auto t=pick(m_pData->monthly); if(!t.empty()) return t; }
    return L"";
}
std::wstring CGoItem::GetGoLabel() const {
    if (!m_cfg || !m_cfg->showGoLabel) return L"";
    bool multiGo = !m_cfg->goKey2.empty();
    if (m_isGo2) return multiGo ? L"Go2 " : L"Go ";
    return multiGo ? L"Go1 " : L"Go ";
}
std::wstring CGoItem::BuildTopText() const {
    if (!m_pData || !m_pData->valid) return L"--";
    auto pickTime = [&](const GoWindow& w)->std::wstring{
        if (!w.resetText.empty()) return w.resetText;
        if (!w.resetsAt.empty()) return Utils::FormatResetTime(w.resetsAt);
        return L"";
    };
    std::wstring monthlyTime = pickTime(m_pData->monthly);
    if (monthlyTime.empty()) monthlyTime = pickTime(m_pData->weekly);
    if (monthlyTime.empty()) monthlyTime = pickTime(m_pData->rolling);
    bool monthlyOk = !monthlyTime.empty();
    if (monthlyOk) monthlyTime += L"(M)";
    else {
        std::wstring any = GetRateLimitResetText();
        if (!any.empty()) return any + L"(R)";
        return L"--";
    }
    if (!IsRateLimited()) return monthlyTime;
    std::wstring recover = GetRateLimitResetText();
    if (recover.empty()) {
        if (!m_pData->weekly.resetText.empty()) recover = m_pData->weekly.resetText;
        else if (!m_pData->rolling.resetText.empty()) recover = m_pData->rolling.resetText;
        else if (!m_pData->weekly.resetsAt.empty()) recover = Utils::FormatResetTime(m_pData->weekly.resetsAt);
        else if (!m_pData->rolling.resetsAt.empty()) recover = Utils::FormatResetTime(m_pData->rolling.resetsAt);
        else if (!m_pData->monthly.resetText.empty()) recover = m_pData->monthly.resetText;
        else if (!m_pData->monthly.resetsAt.empty()) recover = Utils::FormatResetTime(m_pData->monthly.resetsAt);
        else recover = monthlyTime;
    }
    if (recover.empty()) return L"--";
    if (recover.size() >= 3 && recover.substr(recover.size()-3)==L"(R)") return recover;
    if (recover.size() >= 3 && recover.substr(recover.size()-3)==L"(M)") return recover;
    return recover + L"(R)";
}
static thread_local std::wstring s_goTopText;
static thread_local std::wstring s_goBotText;
const wchar_t* CGoItem::GetItemValueText() const {
    if (m_hidden) return L"";
    if (m_pData && m_pData->valid) {
        if (m_isTop) {
            // 下排：套餐剩余时间
            s_goTopText = BuildTopText();
            return s_goTopText.c_str();
        } else {
            // 上排：月剩余额度百分比
            int remain = 100 - m_pData->monthly.percent;
            if (remain < 0) remain = 0;
            wchar_t tmp[64];
            swprintf_s(tmp, L"%d%%", remain);
            s_goBotText = tmp;
            return s_goBotText.c_str();
        }
    }
    s_goTopText = L"--";
    return s_goTopText.c_str();
}
const wchar_t* CGoItem::GetItemValueSampleText() const {
    return L"29d(M)";
}

int CGoItem::GetItemWidth() const {
    if (m_hidden) return 0;
    int w = m_cfg ? m_cfg->widthPx : 0;
    GoAILog(L"GetItemWidth id=%ls w=%d", m_id.c_str(), w);
    return w;
}
int CGoItem::GetItemWidthEx(void* hDC) const {
    if (m_hidden) return 0;
    if (!hDC) return m_cfg ? m_cfg->widthPx : 0;
    const wchar_t* txt = GetItemValueText();
    if (!txt || !*txt) return m_cfg ? m_cfg->widthPx : 0;
    SIZE sz{};
    GetTextExtentPoint32W((HDC)hDC, txt, (int)wcslen(txt), &sz);
    int w = sz.cx + 12;
    if (m_partnerMaxWidth > w) w = m_partnerMaxWidth;
    GoAILog(L"GetItemWidthEx id=%ls txt=[%ls] sz.cx=%d partner=%d w=%d", m_id.c_str(), txt, sz.cx, m_partnerMaxWidth, w);
    return w;
}

extern void AILimit_ForceRefresh();
int CGoItem::OnMouseEvent(MouseEventType type, int x, int y, void* hWnd, int flag) {
    if (type == MT_DBCLICKED) {
        ShellExecuteW(nullptr, L"open", L"https://opencode.ai/go", nullptr, nullptr, SW_SHOWNORMAL);
        return 1;
    }
    if (type == MT_WHEEL_UP || type == MT_WHEEL_DOWN) { AILimit_ForceRefresh(); return 1; }
    return 0;
}

void CGoItem::UpdateAnimTarget(int percent) {
    if ((int)m_targetPercent == percent) return;
    m_animStartPercent = CurrentAnimatedPercent();
    m_targetPercent = (double)percent;
    m_animStartMs = Utils::GetTickMs();
}

double CGoItem::CurrentAnimatedPercent() const {
    if (!m_cfg) return m_targetPercent;
    ULONGLONG now = Utils::GetTickMs();
    ULONGLONG elapsed = now - m_animStartMs;
    if (elapsed >= 800) return m_targetPercent;
    double t = (double)elapsed / 800.0;
    return m_animStartPercent + (m_targetPercent - m_animStartPercent) * t;
}

unsigned long CGoItem::ColorForPercent(int pct, bool dark) const {
    if (!m_cfg || m_cfg->colorMode == 0) {
        return dark ? 0x00FFAA33 : 0x00CC6600;
    } else {
        if (pct < 60) return dark ? 0x0055FF55 : 0x0000AA00;
        if (pct < 80) return dark ? 0x0055FFFF : 0x0000AAAA;
        return dark ? 0x005555FF : 0x000000FF;
    }
}

void CGoItem::DrawItem(void* hDC, int x, int y, int w, int h, bool dark_mode) {
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
