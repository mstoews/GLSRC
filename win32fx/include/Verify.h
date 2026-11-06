//
// Verify.h - FX_VERIFY, the programmer-error check (spec 7.2).
//
// Active in Debug and Release. On failure it writes a line to
// OutputDebugStringW, then breaks into the debugger (Debug) or calls
// FatalAppExitW (Release). Only for invariants that indicate a bug, never for
// user input or I/O that can fail; use assert() for cheap Debug-only checks.
//
#pragma once

#include <Windows.h>

namespace win32fx::detail {
[[noreturn]] void VerifyFailed(const wchar_t* expr, const wchar_t* msg, const wchar_t* file, int line) noexcept;
}

#define FX_WIDEN2(x) L##x
#define FX_WIDEN(x) FX_WIDEN2(x)

#define FX_VERIFY(cond, msg)                                                                  \
    do {                                                                                      \
        if (!(cond))                                                                          \
            ::win32fx::detail::VerifyFailed(FX_WIDEN(#cond), (msg), FX_WIDEN(__FILE__), __LINE__); \
    } while (0)
