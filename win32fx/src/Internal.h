//
// Internal.h - shared by the win32fx sources.
//
#pragma once

#include <Windows.h>

namespace win32fx::detail {

// The dispatch-boundary report (spec 7.2): OutputDebugStringW plus a
// MessageBox owned by `hwnd`'s top-level window. `what` is UTF-8; null for a
// non-std exception.
void ReportException(HWND hwnd, const char* what) noexcept;

} // namespace win32fx::detail
