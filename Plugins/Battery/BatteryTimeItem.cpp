#include "pch.h"
#include "BatteryTimeItem.h"
#include "DataManager.h"

CBatteryTimeItem::CBatteryTimeItem()
{
}

const wchar_t* CBatteryTimeItem::GetItemName() const
{
    return g_data.StringRes(IDS_BATTERY_TIME);
}

const wchar_t* CBatteryTimeItem::GetItemId() const
{
    return L"b5R31TME";
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
    return L"23h 59m";
}
