#include "pch.h"
#include "BatteryQuery.h"
#include <setupapi.h>
#include <devguid.h>
#include <initguid.h>
#pragma comment(lib, "setupapi.lib")

// Define battery IOCTL structures and codes (from poclass.h)
// These are stable across all Windows versions and don't require additional headers.
#define FILE_DEVICE_BATTERY         0x00000029

#ifndef METHOD_NEITHER
#define METHOD_NEITHER              3
#endif
#ifndef FILE_ANY_ACCESS
#define FILE_ANY_ACCESS             0
#endif
#ifndef CTL_CODE
#define CTL_CODE(DeviceType, Function, Method, Access) \
    (((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method))
#endif

#define IOCTL_BATTERY_QUERY_TAG     CTL_CODE(FILE_DEVICE_BATTERY, 0x10, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IOCTL_BATTERY_QUERY_STATUS  CTL_CODE(FILE_DEVICE_BATTERY, 0x11, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IOCTL_BATTERY_QUERY_INFORMATION CTL_CODE(FILE_DEVICE_BATTERY, 0x12, METHOD_NEITHER, FILE_ANY_ACCESS)

#define BatteryInformation          0

#define BATTERY_SYSTEM_BATTERY      0x80000000
#define BATTERY_IS_SHORT_TERM       0x20000000

#define BATTERY_POWER_ON_LINE       0x00000001
#define BATTERY_DISCHARGING         0x00000002
#define BATTERY_CHARGING            0x00000004
#define BATTERY_CRITICAL            0x00000008

struct BATTERY_QUERY_INFORMATION
{
    ULONG BatteryTag;
    ULONG InformationLevel;
    ULONG AtRate;
};

struct BATTERY_INFORMATION
{
    ULONG Capabilities;
    UCHAR Technology;
    UCHAR Reserved[3];
    UCHAR Chemistry[4];
    ULONG DesignedCapacity;
    ULONG FullChargedCapacity;
    ULONG DefaultAlert1;
    ULONG DefaultAlert2;
    ULONG CriticalBias;
    ULONG CycleCount;
};

struct BATTERY_WAIT_STATUS
{
    ULONG BatteryTag;
    ULONG Timeout;
    ULONG PowerState;
    ULONG LowCapacity;
    ULONG HighCapacity;
};

struct BATTERY_STATUS
{
    ULONG PowerState;
    ULONG Capacity;
    ULONG Voltage;
    LONG  Rate;
};

CBatteryQuery::CBatteryQuery()
{
}

CBatteryQuery::~CBatteryQuery()
{
    for (auto& bh : m_handles)
    {
        if (bh.h != INVALID_HANDLE_VALUE)
            CloseHandle(bh.h);
    }
}

bool CBatteryQuery::EnumerateBatteries()
{
    if (m_enumerated)
        return !m_handles.empty();

    m_enumerated = true;

    HDEVINFO hdev = SetupDiGetClassDevs(&GUID_DEVCLASS_BATTERY, nullptr, nullptr,
        DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
    if (hdev == INVALID_HANDLE_VALUE)
        return false;

    for (int idev = 0; idev < 100; idev++)
    {
        SP_DEVICE_INTERFACE_DATA did{};
        did.cbSize = sizeof(did);

        if (!SetupDiEnumDeviceInterfaces(hdev, nullptr, &GUID_DEVCLASS_BATTERY, idev, &did))
            break;

        DWORD cbRequired = 0;
        SetupDiGetDeviceInterfaceDetail(hdev, &did, nullptr, 0, &cbRequired, nullptr);
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
            continue;

        auto pdidd = (PSP_DEVICE_INTERFACE_DETAIL_DATA)LocalAlloc(LPTR, cbRequired);
        if (!pdidd)
            continue;

        pdidd->cbSize = sizeof(*pdidd);
        if (!SetupDiGetDeviceInterfaceDetail(hdev, &did, pdidd, cbRequired, &cbRequired, nullptr))
        {
            LocalFree(pdidd);
            continue;
        }

        HANDLE hBattery = CreateFile(pdidd->DevicePath,
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

        if (hBattery != INVALID_HANDLE_VALUE)
        {
            ULONG batteryTag = 0;
            DWORD dwOut = 0;
            DWORD dwWait = 0;

            if (DeviceIoControl(hBattery, IOCTL_BATTERY_QUERY_TAG,
                &dwWait, sizeof(dwWait),
                &batteryTag, sizeof(batteryTag),
                &dwOut, nullptr) && batteryTag != 0)
            {
                BATTERY_QUERY_INFORMATION bqi{};
                bqi.BatteryTag = batteryTag;
                bqi.InformationLevel = BatteryInformation;

                BATTERY_INFORMATION bi{};
                if (DeviceIoControl(hBattery, IOCTL_BATTERY_QUERY_INFORMATION,
                    &bqi, sizeof(bqi), &bi, sizeof(bi), &dwOut, nullptr))
                {
                    if ((bi.Capabilities & BATTERY_SYSTEM_BATTERY) &&
                        !(bi.Capabilities & BATTERY_IS_SHORT_TERM))
                    {
                        BatteryHandle bh;
                        bh.h = hBattery;
                        bh.tag = batteryTag;
                        m_handles.push_back(bh);
                    }
                    else
                    {
                        CloseHandle(hBattery);
                    }
                }
                else
                {
                    CloseHandle(hBattery);
                }
            }
            else
            {
                CloseHandle(hBattery);
            }
        }
        LocalFree(pdidd);
    }
    SetupDiDestroyDeviceInfoList(hdev);
    return !m_handles.empty();
}

bool CBatteryQuery::QueryBatteryStatus(HANDLE hBattery, ULONG batteryTag,
    ULONG& powerState, ULONG& capacity, LONG& rate)
{
    BATTERY_WAIT_STATUS bws{};
    bws.BatteryTag = batteryTag;

    BATTERY_STATUS bs{};
    DWORD dwOut = 0;

    if (!DeviceIoControl(hBattery, IOCTL_BATTERY_QUERY_STATUS,
        &bws, sizeof(bws), &bs, sizeof(bs), &dwOut, nullptr))
        return false;

    powerState = bs.PowerState;
    capacity = bs.Capacity;
    rate = bs.Rate;
    return true;
}

bool CBatteryQuery::QueryBatteryInfo(HANDLE hBattery, ULONG batteryTag, ULONG& fullCapacity)
{
    BATTERY_QUERY_INFORMATION bqi{};
    bqi.BatteryTag = batteryTag;
    bqi.InformationLevel = BatteryInformation;

    BATTERY_INFORMATION bi{};
    DWORD dwOut = 0;

    if (!DeviceIoControl(hBattery, IOCTL_BATTERY_QUERY_INFORMATION,
        &bqi, sizeof(bqi), &bi, sizeof(bi), &dwOut, nullptr))
        return false;

    fullCapacity = bi.FullChargedCapacity;
    return true;
}

bool CBatteryQuery::QueryAll(BatteryData& data)
{
    if (!EnumerateBatteries())
        return false;

    ULONG totalCapacity = 0;
    LONG totalRate = 0;
    ULONG totalFullCapacity = 0;
    ULONG powerState = 0;
    bool anyValid = false;

    for (auto& bh : m_handles)
    {
        DWORD dwOut = 0;
        DWORD dwWait = 0;
        ULONG currentTag = 0;

        // Re-verify battery tag in case battery was removed
        if (!DeviceIoControl(bh.h, IOCTL_BATTERY_QUERY_TAG,
            &dwWait, sizeof(dwWait),
            &currentTag, sizeof(currentTag),
            &dwOut, nullptr) || currentTag == 0)
        {
            continue;
        }
        bh.tag = currentTag;

        ULONG cap{}, fullCap{};
        LONG rate{};
        ULONG ps{};

        if (!QueryBatteryStatus(bh.h, bh.tag, ps, cap, rate))
            continue;

        if (!QueryBatteryInfo(bh.h, bh.tag, fullCap))
            continue;

        powerState |= ps;
        totalCapacity += cap;
        totalRate += rate;
        totalFullCapacity += fullCap;
        anyValid = true;
    }

    if (!anyValid)
        return false;

    data.powerState = powerState;
    data.capacity = totalCapacity;
    data.rate = totalRate;
    data.fullCapacity = totalFullCapacity;

    m_has_battery = true;
    m_on_battery = !(powerState & BATTERY_POWER_ON_LINE);
    m_charging = (powerState & BATTERY_CHARGING) != 0;
    m_full = (powerState & BATTERY_POWER_ON_LINE) && (totalCapacity >= totalFullCapacity);

    // Smooth the absolute rate with EMA
    {
        double absRate = (totalRate >= 0) ? static_cast<double>(totalRate) : -static_cast<double>(totalRate);
        if (m_first_sample)
        {
            m_smooth_rate = (absRate > 0) ? absRate : m_smooth_rate;
            m_first_sample = false;
        }
        else if (absRate > 0)
        {
            m_smooth_rate = EMA_ALPHA * absRate + (1.0 - EMA_ALPHA) * m_smooth_rate;
        }
    }

    // Calculate remaining time
    if (m_charging && totalRate < 0 && totalCapacity < totalFullCapacity)
    {
        ULONG remainingCap = totalFullCapacity - totalCapacity;
        double rateForCalc = (m_smooth_rate > 0) ? m_smooth_rate : 1.0;
        m_remaining_seconds = static_cast<double>(remainingCap) / rateForCalc * 3600.0;
    }
    else if (m_on_battery && totalRate > 0)
    {
        double rateForCalc = (m_smooth_rate > 0) ? m_smooth_rate : 1.0;
        m_remaining_seconds = static_cast<double>(totalCapacity) / rateForCalc * 3600.0;
    }
    else
    {
        m_remaining_seconds = 0.0;
    }

    return true;
}

void CBatteryQuery::CloseBattery(HANDLE hBattery)
{
    if (hBattery != INVALID_HANDLE_VALUE)
        CloseHandle(hBattery);
}
