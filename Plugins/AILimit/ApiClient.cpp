#include "ApiClient.h"
#include "HttpClient.h"
#include "Utils.h"
#include "Log.h"
#include "nlohmann/json.hpp"
#include <map>

using json = nlohmann::json;

static bool ParseEndpoint(const std::wstring& endpoint, std::wstring& host, std::wstring& path, int& port, bool& https) {
    // endpoint like https://host/v1/usage
    std::wstring url = endpoint;
    https = false;
    if (url.rfind(L"https://", 0) == 0) { https = true; url = url.substr(8); }
    else if (url.rfind(L"http://", 0) == 0) { https = false; url = url.substr(7); }
    else { https = true; } // default https
    size_t slash = url.find(L'/');
    std::wstring hostPort;
    if (slash == std::wstring::npos) { hostPort = url; path = L"/"; }
    else { hostPort = url.substr(0, slash); path = url.substr(slash); }
    size_t colon = hostPort.find(L':');
    if (colon != std::wstring::npos) {
        host = hostPort.substr(0, colon);
        port = _wtoi(hostPort.substr(colon + 1).c_str());
    } else {
        host = hostPort;
        port = https ? 443 : 80;
    }
    if (path.empty()) path = L"/";
    return !host.empty();
}

bool FetchGo(const std::wstring& key, GoData& out, std::string& err, const std::wstring& endpoint) {
    out = GoData();
    if (key.empty()) { err = "empty key"; return false; }
    if (endpoint.empty()) { err = "empty endpoint"; return false; }
    std::wstring host, path; int port; bool https;
    const std::wstring& ep = endpoint;
    if (!ParseEndpoint(ep, host, path, port, https)) { err = "bad endpoint"; return false; }

    std::map<std::wstring, std::wstring> headers;
    headers[L"Authorization"] = L"Bearer " + key;
    headers[L"Content-Type"] = L"application/json";

    HttpResult hr = HttpGet(host, path, port, https, headers, 5000);
    if (!hr.err.empty()) { err = hr.err; return false; }
    if (hr.status == 429) { out.rateLimited = true; err = "rate limited 429"; return false; }
    if (hr.status < 200 || hr.status >= 300) { err = "http " + std::to_string(hr.status) + " " + hr.body.substr(0, 512); return false; }

    try {
        json j = json::parse(hr.body);
        // opencode Go API: {"usage":{"rolling":{...},"weekly":{...},"monthly":{...}}}
        json top = j;
        if (j.contains("usage") && j["usage"].is_object()) top = j["usage"];

        auto parseWindow = [&](const std::string& keyName, GoWindow& w) {
            if (top.contains(keyName) && top[keyName].is_object()) {
                auto& o = top[keyName];
                w.percent = o.value("percent", 0);
                if (o.contains("utilization")) w.percent = o.value("utilization", w.percent);
                if (o.contains("usage")) w.percent = o.value("usage", w.percent);
                // status
                if (o.contains("status") && o["status"].is_string()) w.status = o["status"].get<std::string>();
                if (o.contains("resetsAt") && o["resetsAt"].is_string()) w.resetsAt = o["resetsAt"].get<std::string>();
                else if (o.contains("resets_at") && o["resets_at"].is_string()) w.resetsAt = o["resets_at"].get<std::string>();
                else if (o.contains("resetAt") && o["resetAt"].is_string()) w.resetsAt = o["resetAt"].get<std::string>();
                w.resetText = Utils::FormatResetTime(w.resetsAt);
            }
        };

        // Common keys: try multiple naming
        if (top.contains("5h")) parseWindow("5h", out.rolling);
        else if (top.contains("rolling")) parseWindow("rolling", out.rolling);
        else if (top.contains("five_hour")) parseWindow("five_hour", out.rolling);
        else if (top.contains("session")) parseWindow("session", out.rolling);

        if (top.contains("weekly")) parseWindow("weekly", out.weekly);
        else if (top.contains("week")) parseWindow("week", out.weekly);

        if (top.contains("monthly")) parseWindow("monthly", out.monthly);
        else if (top.contains("month")) parseWindow("month", out.monthly);

        // Also handle flat structure like { "windows": { ... } }
        if (top.contains("windows") && top["windows"].is_object()) {
            auto& w = top["windows"];
            if (w.contains("5h")) { GoWindow tmp; int p = w["5h"].value("percent", 0); tmp.percent = p; if(w["5h"].contains("resetsAt")) tmp.resetsAt=w["5h"]["resetsAt"].get<std::string>(); tmp.resetText=Utils::FormatResetTime(tmp.resetsAt); out.rolling=tmp; }
            if (w.contains("weekly")) { GoWindow tmp; tmp.percent=w["weekly"].value("percent",0); if(w["weekly"].contains("resetsAt")) tmp.resetsAt=w["weekly"]["resetsAt"].get<std::string>(); tmp.resetText=Utils::FormatResetTime(tmp.resetsAt); out.weekly=tmp; }
        }

        // Fallback: if still zero, try top-level percent
        if (out.rolling.percent==0 && out.weekly.percent==0 && out.monthly.percent==0) {
            if (j.contains("percent")) {
                out.weekly.percent = j.value("percent", 0);
                if (j.contains("resetsAt")) out.weekly.resetsAt = j["resetsAt"].get<std::string>();
                out.weekly.resetText = Utils::FormatResetTime(out.weekly.resetsAt);
            }
        }

        // Max percent
        out.maxPercent = out.weekly.percent;
        out.maxWindow = "weekly";
        if (out.rolling.percent > out.maxPercent) { out.maxPercent = out.rolling.percent; out.maxWindow = "5h"; }
        if (out.monthly.percent > out.maxPercent) { out.maxPercent = out.monthly.percent; out.maxWindow = "monthly"; }

        // Summary
        wchar_t buf[128];
        swprintf_s(buf, L"%d%% (%s)", out.maxPercent, Utils::Utf8ToWide(out.maxWindow).c_str());
        out.summary = buf;
        if (!out.rolling.resetText.empty() || !out.weekly.resetText.empty()) {
            // prefer most紧张 window's reset
            std::wstring rt;
            if (out.maxWindow=="5h") rt = out.rolling.resetText;
            else if (out.maxWindow=="monthly") rt = out.monthly.resetText;
            else rt = out.weekly.resetText;
            if (!rt.empty()) {
                out.summary += L" ";
                out.summary += rt;
            }
        }
        out.valid = true;
        return true;
    } catch (std::exception& e) {
        err = std::string("json parse: ") + e.what();
        return false;
    }
}

bool FetchOr(const std::wstring& key, OrData& out, std::string& err, const std::wstring& endpoint) {
    AIDebugLog(L"FetchOr keyLen=%zu", key.size());
    out = OrData();
    if (key.empty()) { err = "empty key"; return false; }
    std::wstring host, path; int port; bool https;
    std::wstring ep = endpoint.empty() ? L"https://openrouter.ai/api/v1/credits" : endpoint;
    if (!ParseEndpoint(ep, host, path, port, https)) { err = "bad endpoint"; return false; }

    std::map<std::wstring, std::wstring> headers;
    headers[L"Authorization"] = L"Bearer " + key;

    HttpResult hr = HttpGet(host, path, port, https, headers, 5000);
    if (!hr.err.empty()) { err = hr.err; return false; }
    if (hr.status < 200 || hr.status >= 300) { err = "http " + std::to_string(hr.status) + " " + hr.body.substr(0, 512); return false; }

    try {
        json j = json::parse(hr.body);
        json d = j;
        if (j.contains("data") && j["data"].is_object()) d = j["data"];
        AIDebugLog(L"FetchOr parsed");

        double limit = 0, usage = 0, remaining = 0;
        // /api/v1/credits: balance = total_credits - total_usage
        if (d.contains("total_credits") && !d["total_credits"].is_null()) {
            if (d["total_credits"].is_number()) limit = d["total_credits"].get<double>();
            else if (d["total_credits"].is_string()) limit = atof(d["total_credits"].get<std::string>().c_str());
        }
        if (d.contains("total_usage") && !d["total_usage"].is_null()) {
            if (d["total_usage"].is_number()) usage = d["total_usage"].get<double>();
            else if (d["total_usage"].is_string()) usage = atof(d["total_usage"].get<std::string>().c_str());
        }
        if (limit > 0) {
            remaining = limit - usage;
            if (remaining < 0) remaining = 0;
        }
        // Legacy /api/v1/key fields
        if (d.contains("total_available") && !d["total_available"].is_null()) {
            if (d["total_available"].is_number()) remaining = d["total_available"].get<double>();
            else if (d["total_available"].is_string()) remaining = atof(d["total_available"].get<std::string>().c_str());
        }
        if (d.contains("limit_remaining") && !d["limit_remaining"].is_null()) {
            if (d["limit_remaining"].is_number()) remaining = d["limit_remaining"].get<double>();
            else if (d["limit_remaining"].is_string()) remaining = atof(d["limit_remaining"].get<std::string>().c_str());
        }
        if (limit == 0 && d.contains("limit") && !d["limit"].is_null()) {
            if (d["limit"].is_number()) limit = d["limit"].get<double>();
            else if (d["limit"].is_string()) limit = atof(d["limit"].get<std::string>().c_str());
        }
        if (usage == 0 && d.contains("usage")) {
            std::string udump = d["usage"].dump();
            int utype = static_cast<int>(d["usage"].type());
            try { usage = std::stod(udump); } catch(...) { usage = 0; }
            AIDebugLog(L"FetchOr usage parsed=%.1f", usage);
        }
        // Free tier detection
        bool isFree = false;
        if (d.contains("is_free_tier")) isFree = d["is_free_tier"].get<bool>();
        else if (d.contains("free_tier")) isFree = d["free_tier"].get<bool>();
        else if (limit == 0) isFree = true;

        out.limit = limit;
        out.remaining = remaining;
        out.isFreeTier = isFree;
        if (d.contains("limit_reset") && d["limit_reset"].is_string())
            out.limitReset = d["limit_reset"].get<std::string>();
        if (d.contains("usage_daily")) {
            try { out.usageDaily = std::stod(d["usage_daily"].dump()); } catch(...) { out.usageDaily = 0; }
        }
        if (limit > 0) out.usage = (usage / limit) * 100.0;
        else out.usage = usage;

        double pctRemain = 100.0 - out.usage;
        if (pctRemain < 0) pctRemain = 0;
        if (pctRemain > 100) pctRemain = 100;
        wchar_t buf[64];
        swprintf_s(buf, L"%.0f%%", pctRemain);
        out.display = buf;
        if (isFree) out.display += L" Free";
        out.valid = true;
        AIDebugLog(L"FetchOr OK usage=%.1f remaining=%.1f limit=%.1f", out.usage, out.remaining, out.limit);
        return true;
    } catch (std::exception& e) {
        err = std::string("json parse: ") + e.what();
        AIDebugLog(L"FetchOr FAIL");
        return false;
    }
}

bool FetchDs(const std::wstring& key, DsData& out, std::string& err, const std::wstring& endpoint) {
    out = DsData();
    if (key.empty()) { err = "empty key"; return false; }
    std::wstring host, path; int port; bool https;
    std::wstring ep = endpoint.empty() ? L"https://api.deepseek.com/user/balance" : endpoint;
    if (!ParseEndpoint(ep, host, path, port, https)) { err = "bad endpoint"; return false; }

    std::map<std::wstring, std::wstring> headers;
    headers[L"Authorization"] = L"Bearer " + key;

    HttpResult hr = HttpGet(host, path, port, https, headers, 5000);
    if (!hr.err.empty()) { err = hr.err; return false; }
    if (hr.status < 200 || hr.status >= 300) { err = "http " + std::to_string(hr.status) + " " + hr.body.substr(0, 512); return false; }

    try {
        json j = json::parse(hr.body);
        out.isAvailable = j.value("is_available", false);
        if (j.contains("balance_infos") && j["balance_infos"].is_array() && !j["balance_infos"].empty()) {
            json b = j["balance_infos"][0];
            for (auto& it : j["balance_infos"]) {
                if (it.contains("currency") && it["currency"].is_string() && it["currency"].get<std::string>() == "CNY") { b = it; break; }
            }
            if (b.contains("currency") && b["currency"].is_string()) out.currency = b["currency"].get<std::string>();
            auto num = [&](const char* k) -> double {
                if (!b.contains(k) || b[k].is_null()) return 0;
                if (b[k].is_number()) return b[k].get<double>();
                if (b[k].is_string()) { try { return std::stod(b[k].get<std::string>()); } catch (...) { return 0; } }
                return 0;
            };
            out.total = num("total_balance");
            out.granted = num("granted_balance");
            out.toppedUp = num("topped_up_balance");
        }
        out.valid = true;
        return true;
    } catch (std::exception& e) {
        err = std::string("json parse: ") + e.what();
        return false;
    }
}
