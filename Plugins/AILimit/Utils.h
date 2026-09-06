#pragma once
#include <string>
#include <windows.h>

namespace Utils {

std::wstring Utf8ToWide(const std::string& utf8);
std::string WideToUtf8(const std::wstring& wide);

std::string Base64Encode(const BYTE* data, DWORD len);
std::string Base64Decode(const std::string& b64, std::string& out);
bool Base64DecodeToBytes(const std::string& b64, std::string& outBytes);

std::string ProtectData(const std::string& plain);
std::string UnprotectData(const std::string& encB64);

std::wstring IniReadString(const std::wstring& file, const std::wstring& section, const std::wstring& key, const std::wstring& defVal);
int IniReadInt(const std::wstring& file, const std::wstring& section, const std::wstring& key, int defVal);
bool IniReadBool(const std::wstring& file, const std::wstring& section, const std::wstring& key, bool defVal);
void IniWriteString(const std::wstring& file, const std::wstring& section, const std::wstring& key, const std::wstring& val);
void IniWriteInt(const std::wstring& file, const std::wstring& section, const std::wstring& key, int val);
void IniWriteBool(const std::wstring& file, const std::wstring& section, const std::wstring& key, bool val);

bool ParseISO8601(const std::string& s, ULONGLONG& fileTime100ns);
std::wstring FormatResetTime(const std::string& resetsAtISO);

ULONGLONG GetTickMs();

} // namespace Utils
