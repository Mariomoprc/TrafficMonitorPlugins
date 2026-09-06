#pragma once
#include "PluginInterface.h"
#include "DataManager.h"

class CBatteryTimeItem : public IPluginItem
{
public:
    CBatteryTimeItem();

    virtual const wchar_t* GetItemName() const override;
    virtual const wchar_t* GetItemId() const override;
    virtual const wchar_t* GetItemLableText() const override;
    virtual const wchar_t* GetItemValueText() const override;
    virtual const wchar_t* GetItemValueSampleText() const override;
    virtual bool IsCustomDraw() const override;
    virtual int GetItemWidthEx(void* hDC) const override;
    virtual void DrawItem(void* hDC, int x, int y, int w, int h, bool dark_mode) override;

private:
    COLORREF GetProgressColor(double percent) const;
    void DrawProgressBar(CDC* pDC, int x, int y, int width, int height, double percent, bool dark_mode);
    void DrawProgressBarGradient(CDC* pDC, int x, int y, int width, int height, double percent);
    void DrawProgressBarDots(CDC* pDC, int x, int y, int width, int height, double percent);
    void DrawRing(CDC* pDC, const CRect& ring_box, double percent, bool dark_mode);
};
