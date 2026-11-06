//
// StringUtil.cpp - see StringUtil.h.
//

#include "StringUtil.h"

#include <cstdarg>
#include <cstdio>

namespace win32fx {

std::wstring LoadResString(UINT id) { return LoadResString(GetModuleHandleW(nullptr), id); }

std::wstring LoadResString(HINSTANCE inst, UINT id)
{
    const wchar_t* p = nullptr;   // length 0: a read-only pointer into the resource
    const int n = LoadStringW(inst, id, reinterpret_cast<LPWSTR>(&p), 0);
    return (n > 0 && p) ? std::wstring(p, static_cast<size_t>(n)) : std::wstring();
}

std::wstring Format(const wchar_t* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    va_list copy;
    va_copy(copy, args);
    const int n = _vscwprintf(fmt, copy);
    va_end(copy);
    std::wstring s;
    if (n > 0) {
        s.resize(static_cast<size_t>(n) + 1);
        vswprintf_s(s.data(), s.size(), fmt, args);
        s.resize(static_cast<size_t>(n));
    }
    va_end(args);
    return s;
}

std::wstring Utf8ToWide(std::string_view utf8)
{
    if (utf8.empty()) return {};
    const int n = MultiByteToWideChar(CP_UTF8, 0, utf8.data(), static_cast<int>(utf8.size()), nullptr, 0);
    std::wstring s(static_cast<size_t>(n), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, utf8.data(), static_cast<int>(utf8.size()), s.data(), n);
    return s;
}

std::string WideToUtf8(std::wstring_view wide)
{
    if (wide.empty()) return {};
    const int n = WideCharToMultiByte(CP_UTF8, 0, wide.data(), static_cast<int>(wide.size()), nullptr, 0,
                                      nullptr, nullptr);
    std::string s(static_cast<size_t>(n), '\0');
    WideCharToMultiByte(CP_UTF8, 0, wide.data(), static_cast<int>(wide.size()), s.data(), n, nullptr, nullptr);
    return s;
}

std::wstring Trim(std::wstring_view s)
{
    constexpr wchar_t ws[] = L" \t\r\n";
    const size_t b = s.find_first_not_of(ws);
    if (b == std::wstring_view::npos) return {};
    return std::wstring(s.substr(b, s.find_last_not_of(ws) - b + 1));
}

std::wstring GetText(HWND hwnd)
{
    std::wstring s(static_cast<size_t>(GetWindowTextLengthW(hwnd)) + 1, L'\0');
    s.resize(static_cast<size_t>(GetWindowTextW(hwnd, s.data(), static_cast<int>(s.size()))));
    return s;
}

std::wstring GetDlgItemString(HWND dialog, int id) { return GetText(GetDlgItem(dialog, id)); }

} // namespace win32fx
