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
    ~StatusBar();

    // A child of `parent` with ID `id` and a size grip, in the system's status
    // font for the parent's DPI.
    HWND Create(HWND parent, UINT id, HINSTANCE inst);

    // After a DPI change (UI refresh spec 5.2): the font for the bar's new
    // DPI. The bar takes its new height on the frame's next layout.
    void UpdateFont();

    // Right edges of each part in px; -1 for "to the end".
    void SetParts(std::initializer_list<int> rightEdges);
    void SetText(int part, const wchar_t* text);

    HWND hwnd() const noexcept { return hwnd_; }
    operator HWND() const noexcept { return hwnd_; }

private:
    HWND hwnd_ = nullptr;
    HFONT font_ = nullptr;   // owned
};

} // namespace win32fx
