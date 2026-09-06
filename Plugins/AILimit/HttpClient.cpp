#include "HttpClient.h"
#include <windows.h>
#include <winhttp.h>
#include <string>
#include <map>

#pragma comment(lib, "winhttp.lib")

static HttpResult DoRequest(const std::wstring& host, const std::wstring& path, int port, bool https,
                            const std::map<std::wstring, std::wstring>& headers,
                            const std::string* postBody, int timeoutMs) {
    HttpResult res;
    HINTERNET hSession = WinHttpOpen(L"AILimitPlugin/1.0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                     WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) {
        res.err = "WinHttpOpen failed";
        return res;
    }
    WinHttpSetTimeouts(hSession, timeoutMs, timeoutMs, timeoutMs, timeoutMs);

    HINTERNET hConnect = WinHttpConnect(hSession, host.c_str(), (INTERNET_PORT)port, 0);
    if (!hConnect) {
        res.err = "WinHttpConnect failed";
        WinHttpCloseHandle(hSession);
        return res;
    }

    DWORD flags = https ? WINHTTP_FLAG_SECURE : 0;
    const wchar_t* verb = (postBody ? L"POST" : L"GET");
    HINTERNET hRequest = WinHttpOpenRequest(hConnect, verb, path.c_str(), nullptr,
                                            WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, flags);
    if (!hRequest) {
        res.err = "WinHttpOpenRequest failed";
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return res;
    }

    // Apply timeouts to request as well
    WinHttpSetTimeouts(hRequest, timeoutMs, timeoutMs, timeoutMs, timeoutMs);

    for (auto& kv : headers) {
        std::wstring hdr = kv.first + L": " + kv.second;
        WinHttpAddRequestHeaders(hRequest, hdr.c_str(), (DWORD)hdr.size(), WINHTTP_ADDREQ_FLAG_ADD);
    }

    BOOL bResult = FALSE;
    if (postBody) {
        // Ensure Content-Length handled; WinHttpSendRequest will handle
        bResult = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                                     (LPVOID)postBody->data(), (DWORD)postBody->size(),
                                     (DWORD)postBody->size(), 0);
    } else {
        bResult = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                                     WINHTTP_NO_REQUEST_DATA, 0, 0, 0);
    }
    if (!bResult) {
        res.err = "WinHttpSendRequest failed: " + std::to_string(GetLastError());
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return res;
    }

    bResult = WinHttpReceiveResponse(hRequest, nullptr);
    if (!bResult) {
        res.err = "WinHttpReceiveResponse failed: " + std::to_string(GetLastError());
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return res;
    }

    DWORD statusCode = 0;
    DWORD sz = sizeof(statusCode);
    WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                        WINHTTP_HEADER_NAME_BY_INDEX, &statusCode, &sz, WINHTTP_NO_HEADER_INDEX);
    res.status = (int)statusCode;

    // Read body loop
    std::string body;
    DWORD dwSize = 0;
    do {
        dwSize = 0;
        if (!WinHttpQueryDataAvailable(hRequest, &dwSize)) {
            res.err = "WinHttpQueryDataAvailable failed";
            break;
        }
        if (dwSize == 0) break;
        std::string buf(dwSize, 0);
        DWORD dwRead = 0;
        if (!WinHttpReadData(hRequest, &buf[0], dwSize, &dwRead)) {
            res.err = "WinHttpReadData failed";
            break;
        }
        buf.resize(dwRead);
        body += buf;
    } while (dwSize > 0);

    res.body = body;

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    return res;
}

HttpResult HttpGet(const std::wstring& host, const std::wstring& path, int port, bool https,
                   const std::map<std::wstring, std::wstring>& headers, int timeoutMs) {
    return DoRequest(host, path, port, https, headers, nullptr, timeoutMs);
}

HttpResult HttpPost(const std::wstring& host, const std::wstring& path, int port, bool https,
                    const std::map<std::wstring, std::wstring>& headers,
                    const std::string& postBody, int timeoutMs) {
    return DoRequest(host, path, port, https, headers, &postBody, timeoutMs);
}
