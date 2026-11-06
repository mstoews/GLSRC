//
// StatusBar.h - a common-controls status bar (msctls_statusbar32).
//
// A thin wrapper: the control has its own window procedure, so this isn't a
// Window subclass. Dock it with MdiFrame::Dock(bar, DockSide::Bottom, 0): it
// sizes itself.
//
#pragma once

#include <Windows.h>

#include <initializer_list>

namespace win32fx {

class StatusBar {
public:
    StatusBar() noexcept = default;
    StatusBar(const StatusBar&) = delete;
    StatusBar& operator=(const StatusBar&) = delete;

    // A child of `parent` with ID `id` and a size grip.
    HWND Create(HWND parent, UINT id, HINSTANCE inst);

    // Right edges of each part in px; -1 for "to the end".
    void SetParts(std::initializer_list<int> rightEdges);
    void SetText(int part, const wchar_t* text);

    HWND hwnd() const noexcept { return hwnd_; }
    operator HWND() const noexcept { return hwnd_; }

private:
    HWND hwnd_ = nullptr;
};

} // namespace win32fx
