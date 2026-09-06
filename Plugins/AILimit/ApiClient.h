#pragma once
#include <string>
#include <windows.h>

struct GoWindow {
    int percent = 0;
    std::string status;
    std::string resetsAt;
    std::wstring resetText;
};

struct GoData {
    GoWindow rolling; // 5h
    GoWindow weekly;
    GoWindow monthly;
    std::wstring summary;
    int maxPercent = 0;
    std::string maxWindow; // "5h"/"weekly"/"monthly"
    bool rateLimited = false;
    bool valid = false;
};

struct OrData {
    double limit = 0;
    double remaining = 0;
    double usage = 0;
    double usageDaily = 0;
    bool isFreeTier = false;
    std::string limitReset;
    std::wstring display;
    bool valid = false;
};

struct DsData {
    double total = 0;
    double granted = 0;
    double toppedUp = 0;
    std::string currency = "CNY";
    bool isAvailable = false;
    bool valid = false;
};

bool FetchGo(const std::wstring& key, GoData& out, std::string& err, const std::wstring& endpoint);
bool FetchOr(const std::wstring& key, OrData& out, std::string& err, const std::wstring& endpoint);
bool FetchDs(const std::wstring& key, DsData& out, std::string& err, const std::wstring& endpoint);
