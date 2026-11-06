//
// Verify.cpp - FX_VERIFY failure (spec 7.2) and the dispatch-boundary report.
//

#include "Verify.h"
#include "Internal.h"
#include "StringUtil.h"

#include <intrin.h>

namespace win32fx::detail {

void VerifyFailed(const wchar_t* expr, const wchar_t* msg, const wchar_t* file, int line) noexcept
{
    wchar_t text[1024];
    swprintf_s(text, L"FX_VERIFY(%ls) failed: %ls (%ls:%d)\n", expr, msg ? msg : L"", file, line);
    OutputDebugStringW(text);
#ifdef _DEBUG
    __debugbreak();
#endif
    FatalAppExitW(0, text);
    ExitProcess(3);
}

void ReportException(HWND hwnd, const char* what) noexcept
{
    try {
        const std::wstring detail = what ? Utf8ToWide(what) : std::wstring(L"unknown exception");
        OutputDebugStringW((L"win32fx: unhandled exception: " + detail + L"\n").c_str());
        HWND owner = hwnd ? GetAncestor(hwnd, GA_ROOT) : nullptr;
        const std::wstring caption = owner ? GetText(owner) : std::wstring();
        MessageBoxW(owner, detail.c_str(), caption.empty() ? L"Error" : caption.c_str(), MB_OK | MB_ICONERROR);
    } catch (...) {
        OutputDebugStringW(L"win32fx: unhandled exception (no memory to report it)\n");
    }
}

} // namespace win32fx::detail
