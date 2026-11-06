//
// StringUtil.h - string helpers for the Win32 / SQLite boundaries (spec 5.2).
//
// Win32 text is UTF-16 (std::wstring); cb4sqlite text is UTF-8 (std::string).
//
#pragma once

#include <Windows.h>

#include <string>
#include <string_view>

namespace win32fx {

// A string resource of the EXE (GetModuleHandleW(nullptr)), or of `inst`.
// Empty if the ID doesn't exist.
std::wstring LoadResString(UINT id);
std::wstring LoadResString(HINSTANCE inst, UINT id);

// swprintf-style formatting, any length.
std::wstring Format(const wchar_t* fmt, ...);

// Invalid input becomes U+FFFD; never throws on bad encoding.
std::wstring Utf8ToWide(std::string_view utf8);
std::string WideToUtf8(std::wstring_view wide);

// Leading and trailing spaces, tabs, CR and LF removed (the original StrTrim).
std::wstring Trim(std::wstring_view s);

// The window text of hwnd, or of dialog item `id` of hwnd.
std::wstring GetText(HWND hwnd);
std::wstring GetDlgItemString(HWND dialog, int id);

} // namespace win32fx
