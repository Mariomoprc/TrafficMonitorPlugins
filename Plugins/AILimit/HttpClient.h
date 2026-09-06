#pragma once
#include <string>
#include <map>

struct HttpResult {
    int status = 0;
    std::string body;
    std::string err;
};

HttpResult HttpGet(const std::wstring& host, const std::wstring& path, int port, bool https,
                   const std::map<std::wstring, std::wstring>& headers, int timeoutMs = 5000);

HttpResult HttpPost(const std::wstring& host, const std::wstring& path, int port, bool https,
                    const std::map<std::wstring, std::wstring>& headers,
                    const std::string& postBody, int timeoutMs = 5000);
