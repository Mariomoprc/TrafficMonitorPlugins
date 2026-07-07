#pragma once
#include "PluginInterface.h"
#include "BatteryTimeItem.h"
#include "BatteryQuery.h"
#include <string>

class CBatteryTime : public ITMPlugin
{
private:
    CBatteryTime();

public:
    static CBatteryTime& Instance();

    virtual IPluginItem* GetItem(int index) override;
    virtual const wchar_t* GetTooltipInfo() override;
    virtual void DataRequired() override;
    virtual OptionReturn ShowOptionsDialog(void* hParent) override;
    virtual const wchar_t* GetInfo(PluginInfoIndex index) override;
    virtual void OnExtenedInfo(ExtendedInfoIndex index, const wchar_t* data) override;

    static std::wstring FormatTimeString(double seconds);

private:
    static CBatteryTime m_instance;
    CBatteryTimeItem m_item;
    std::wstring m_tooltip_info;
    CBatteryQuery m_batteryQuery;
};

#ifdef __cplusplus
extern "C" {
#endif
    __declspec(dllexport) ITMPlugin* TMPluginGetInstance();

#ifdef __cplusplus
}
#endif
