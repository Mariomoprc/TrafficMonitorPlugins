#pragma once
#include "PluginInterface.h"
#include "Config.h"
#include "ApiClient.h"
#include <string>

class CGoItem : public IPluginItem {
public:
    CGoItem(bool isTop, bool isGo2, CAIConfig* cfg, GoData* pData);
    // IPluginItem
    const wchar_t* GetItemName() const override;
    const wchar_t* GetItemId() const override;
    const wchar_t* GetItemLableText() const override;
    const wchar_t* GetItemValueText() const override;
    const wchar_t* GetItemValueSampleText() const override;
    bool IsCustomDraw() const override { return true; }
    int GetItemWidth() const override;
    int GetItemWidthEx(void* hDC) const override;
    void DrawItem(void* hDC, int x, int y, int w, int h, bool dark_mode) override;
    int OnMouseEvent(MouseEventType type, int x, int y, void* hWnd, int flag) override;

    void UpdateAnimTarget(int percent);
    void SetConfig(CAIConfig* cfg) { m_cfg = cfg; }
    void SetHidden(bool h) { m_hidden = h; }
    bool IsHidden() const { return m_hidden; }

    int m_partnerMaxWidth = 0;  // 配对 item 的最大文本宽度（用于上下对齐）

private:
    bool m_isTop;
    bool m_isGo2;
    CAIConfig* m_cfg;
    GoData* m_pData;
    std::wstring m_name;
    std::wstring m_id;
    std::wstring m_label;
    mutable std::wstring m_value;
    std::wstring m_sample = L"13h(R)";
    bool m_hidden = false;

    // animation
    double m_displayPercent = 0;
    double m_targetPercent = 0;
    ULONGLONG m_animStartMs = 0;
    double m_animStartPercent = 0;

    // top 轮播：月剩余天数 ↔ 限流恢复倒计时，每 5s 切换，仅限流时轮播
    mutable int m_rotateState = 0;
    mutable ULONGLONG m_lastRotateMs = 0;
    bool IsRateLimited() const;
    std::wstring GetRateLimitResetText() const;
    std::wstring BuildTopText() const;

    unsigned long ColorForPercent(int pct, bool dark) const;
    double CurrentAnimatedPercent() const;
    std::wstring GetGoLabel() const;
};
