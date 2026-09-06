#pragma once
#include "PluginInterface.h"
#include "Config.h"
#include "ApiClient.h"
#include <string>

class CDsItem : public IPluginItem {
public:
    CDsItem(bool isTop, CAIConfig* cfg, DsData* pData, double* pSpent);
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
    void SetConfig(CAIConfig* cfg) { m_cfg = cfg; }
    void SetHidden(bool h) { m_hidden = h; }
    bool IsHidden() const { return m_hidden; }

    int m_partnerMaxWidth = 0;
private:
    bool m_isTop;
    CAIConfig* m_cfg;
    DsData* m_pData;
    double* m_pSpent;
    std::wstring m_name;
    std::wstring m_id;
    std::wstring m_label;
    bool m_hidden = false;
};
