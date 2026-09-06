#include "Utils.h"
#include <wincrypt.h>
#include <string>
#include <vector>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace Utils {

std::wstring Utf8ToWide(const std::string& utf8) {
    if (utf8.empty()) return L"";
    int len = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), (int)utf8.size(), nullptr, 0);
    if (len <= 0) return L"";
    std::wstring out(len, 0);
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), (int)utf8.size(), &out[0], len);
    return out;
}

std::string WideToUtf8(const std::wstring& wide) {
    if (wide.empty()) return "";
    int len = WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), (int)wide.size(), nullptr, 0, nullptr, nullptr);
    if (len <= 0) return "";
    std::string out(len, 0);
    WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), (int)wide.size(), &out[0], len, nullptr, nullptr);
    return out;
}

std::string Base64Encode(const BYTE* data, DWORD len) {
    if (!data || len == 0) return "";
    DWORD outLen = 0;
    if (!CryptBinaryToStringA(data, len, CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, nullptr, &outLen))
        return "";
    std::string out(outLen, 0);
    if (!CryptBinaryToStringA(data, len, CRYPT_STRING_BASE64 | CRYPT_STRING_NOCRLF, &out[0], &outLen))
        return "";
    // CryptBinaryToString includes null terminator in outLen, out string has it; trim
    if (!out.empty() && out.back() == '\0') out.pop_back();
    // Remove possible embedded nulls at end
    while (!out.empty() && out.back() == '\0') out.pop_back();
    out.resize(strlen(out.c_str()));
    return out;
}

std::string Base64Decode(const std::string& b64, std::string& out) {
    return Base64DecodeToBytes(b64, out) ? out : std::string();
}

bool Base64DecodeToBytes(const std::string& b64, std::string& outBytes) {
    outBytes.clear();
    if (b64.empty()) return true;
    DWORD decLen = 0;
    if (!CryptStringToBinaryA(b64.c_str(), (DWORD)b64.size(), CRYPT_STRING_BASE64, nullptr, &decLen, nullptr, nullptr))
        return false;
    std::string buf(decLen, 0);
    if (!CryptStringToBinaryA(b64.c_str(), (DWORD)b64.size(), CRYPT_STRING_BASE64, (BYTE*)buf.data(), &decLen, nullptr, nullptr))
        return false;
    buf.resize(decLen);
    outBytes = buf;
    return true;
}

std::string ProtectData(const std::string& plain) {
    if (plain.empty()) return "";
    DATA_BLOB in{}, out{};
    in.pbData = (BYTE*)plain.data();
    in.cbData = (DWORD)plain.size();
    if (!CryptProtectData(&in, L"AILimitPlugin", nullptr, nullptr, nullptr, 0, &out))
        return "";
    std::string enc = Base64Encode(out.pbData, out.cbData);
    LocalFree(out.pbData);
    return enc;
}

std::string UnprotectData(const std::string& encB64) {
    if (encB64.empty()) return "";
    std::string dec;
    if (!Base64DecodeToBytes(encB64, dec) || dec.empty()) return "";
    DATA_BLOB in{}, out{};
    in.pbData = (BYTE*)dec.data();
    in.cbData = (DWORD)dec.size();
    if (!CryptUnprotectData(&in, nullptr, nullptr, nullptr, nullptr, 0, &out)) return "";
    std::string plain((char*)out.pbData, out.cbData);
    LocalFree(out.pbData);
    return plain;
}

std::wstring IniReadString(const std::wstring& file, const std::wstring& section, const std::wstring& key, const std::wstring& defVal) {
    wchar_t buf[4096] = {};
    GetPrivateProfileStringW(section.c_str(), key.c_str(), defVal.c_str(), buf, 4096, file.c_str());
    return std::wstring(buf);
}
int IniReadInt(const std::wstring& file, const std::wstring& section, const std::wstring& key, int defVal) {
    return (int)GetPrivateProfileIntW(section.c_str(), key.c_str(), defVal, file.c_str());
}
bool IniReadBool(const std::wstring& file, const std::wstring& section, const std::wstring& key, bool defVal) {
    return GetPrivateProfileIntW(section.c_str(), key.c_str(), defVal ? 1 : 0, file.c_str()) != 0;
}
void IniWriteString(const std::wstring& file, const std::wstring& section, const std::wstring& key, const std::wstring& val) {
    WritePrivateProfileStringW(section.c_str(), key.c_str(), val.c_str(), file.c_str());
}
void IniWriteInt(const std::wstring& file, const std::wstring& section, const std::wstring& key, int val) {
    wchar_t buf[32];
    swprintf_s(buf, L"%d", val);
    WritePrivateProfileStringW(section.c_str(), key.c_str(), buf, file.c_str());
}
void IniWriteBool(const std::wstring& file, const std::wstring& section, const std::wstring& key, bool val) {
    WritePrivateProfileStringW(section.c_str(), key.c_str(), val ? L"1" : L"0", file.c_str());
}

bool ParseISO8601(const std::string& s, ULONGLONG& fileTime100ns) {
    // Expect like 2026-08-29T12:34:56Z or 2026-08-29T12:34:56.123Z or with +00:00
    if (s.empty()) return false;
    int y=0, mo=0, d=0, h=0, mi=0, sec=0;
    // Strip fractional and timezone for sscanf
    std::string t = s;
    // Find 'T'
    size_t posT = t.find('T');
    if (posT == std::string::npos) return false;
    // Extract date/time numeric parts
    // Use sscanf on the prefix
    int n = sscanf_s(t.c_str(), "%d-%d-%dT%d:%d:%d", &y, &mo, &d, &h, &mi, &sec);
    if (n < 6) return false;
    SYSTEMTIME st{};
    st.wYear = (WORD)y;
    st.wMonth = (WORD)mo;
    st.wDay = (WORD)d;
    st.wHour = (WORD)h;
    st.wMinute = (WORD)mi;
    st.wSecond = (WORD)sec;
    st.wMilliseconds = 0;
    FILETIME ft{};
    if (!SystemTimeToFileTime(&st, &ft)) return false;
    ULARGE_INTEGER uli;
    uli.LowPart = ft.dwLowDateTime;
    uli.HighPart = ft.dwHighDateTime;
    fileTime100ns = uli.QuadPart;
    return true;
}

std::wstring FormatResetTime(const std::string& resetsAtISO) {
    if (resetsAtISO.empty()) return L"";
    ULONGLONG ftReset = 0;
    if (!ParseISO8601(resetsAtISO, ftReset)) return L"";
    FILETIME ftNow;
    GetSystemTimeAsFileTime(&ftNow);
    ULARGE_INTEGER now, reset;
    now.LowPart = ftNow.dwLowDateTime; now.HighPart = ftNow.dwHighDateTime;
    reset.LowPart = (DWORD)(ftReset & 0xFFFFFFFF); reset.HighPart = (DWORD)(ftReset >> 32);
    // Actually ftReset already computed, use it
    ULARGE_INTEGER r2;
    r2.QuadPart = ftReset;
    if (r2.QuadPart <= now.QuadPart) return L"0m";
    ULONGLONG diff100ns = r2.QuadPart - now.QuadPart;
    ULONGLONG totalSec = diff100ns / 10000000ULL;
    ULONGLONG days = totalSec / 86400;
    ULONGLONG hours = (totalSec % 86400) / 3600;
    ULONGLONG mins = (totalSec % 3600) / 60;
    wchar_t buf[64];
    if (days >= 30) {
        swprintf_s(buf, L"%llud", days);
    } else if (days >= 5) {
        swprintf_s(buf, L"%llud", days);
    } else if (days >= 1) {
        if (hours == 0 && mins > 0)
            swprintf_s(buf, L"%llud %llum", days, mins);
        else
            swprintf_s(buf, L"%llud %lluh %llum", days, hours, mins);
    } else {
        if (hours > 0) {
            swprintf_s(buf, L"%lluh", hours);
        } else {
            swprintf_s(buf, L"%llum", mins == 0 ? 1ULL : mins);
        }
    }
    return std::wstring(buf);
}

ULONGLONG GetTickMs() {
    return GetTickCount64();
}

} // namespace Utils
