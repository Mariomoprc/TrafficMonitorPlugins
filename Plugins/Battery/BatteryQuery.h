#pragma once
#include <string>
#include <vector>
#include <windows.h>

class CBatteryQuery
{
public:
    CBatteryQuery();
    ~CBatteryQuery();

    struct BatteryData
    {
        ULONG powerState{};
        ULONG capacity{};
        LONG rate{};
        ULONG fullCapacity{};
    };

    bool QueryAll(BatteryData& data);

    double GetSmoothedRemainingSeconds() const { return m_remaining_seconds; }
    bool IsOnBattery() const { return m_on_battery; }
    bool IsCharging() const { return m_charging; }
    bool IsFull() const { return m_full; }
    bool HasBattery() const { return m_has_battery; }

private:
    struct BatteryHandle
    {
        HANDLE h{ INVALID_HANDLE_VALUE };
        ULONG tag{};
    };

    bool EnumerateBatteries();
    bool QueryBatteryStatus(HANDLE hBattery, ULONG batteryTag, ULONG& powerState, ULONG& capacity, LONG& rate);
    bool QueryBatteryInfo(HANDLE hBattery, ULONG batteryTag, ULONG& fullCapacity);
    void CloseBattery(HANDLE hBattery);

    std::vector<BatteryHandle> m_handles;
    bool m_enumerated{};

    double m_smooth_rate{};
    bool m_first_sample{ true };
    double m_remaining_seconds{};
    bool m_on_battery{};
    bool m_charging{};
    bool m_full{};
    bool m_has_battery{};

    DWORD m_last_query_time{};
    static constexpr double EMA_ALPHA{ 0.3 };
};
