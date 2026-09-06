#pragma once
#include "PluginInterface.h"
#include "Config.h"
#include "ApiClient.h"
#include <string>

class COrItem : public IPluginItem {
public:
    COrItem(bool isTop, CAIConfig* cfg, OrData* pData);
    const wchar_t* GetItemName() const override;
    const wchar_t* GetItemId() const override;
    const wchar_t* GetItemLableText() const override;
    const wchar_t* GetItemValueText() const override;
    const wchar_t* GetItemValueSampleText() const override;
    bool IsCustomDraw() const override { return true; }
    int GetItemWidth() const override { return m_hidden ? 0 : (m_cfg ? m_cfg->widthPx : 90); }
    int GetItemWidthEx(void* hDC) const override;
    void DrawItem(void* hDC, int x, int y, int w, int h, bool dark_mode) override;
    int OnMouseEvent(MouseEventType type, int x, int y, void* hWnd, int flag) override;
    void UpdateAnimTarget(double pct);
    void SetConfig(CAIConfig* cfg) { m_cfg = cfg; }
    void SetHidden(bool h) { m_hidden = h; }
    bool IsHidden() const { return m_hidden; }

    int m_partnerMaxWidth = 0;
private:
    bool m_isTop;
    CAIConfig* m_cfg;
    OrData* m_pData;
    std::wstring m_name;
    std::wstring m_id;
    std::wstring m_label;
    std::wstring m_value;
    std::wstring m_sample = L"88%";
    bool m_hidden = false;
    double m_displayPercent = 0;
    double m_targetPercent = 0;
    ULONGLONG m_animStartMs = 0;
    double m_animStartPercent = 0;
    double CurrentAnimatedPercent() const;
};
