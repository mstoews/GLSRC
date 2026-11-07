//
// MdiFrame.h - MDI frame and child windows (spec 3.5).
//
// MdiFrame owns the MDICLIENT and lays it out between the docked bars
// (Dock(): Toolbar at the top, module bar and StatusBar at the bottom, as in
// the original TGlWindow::WMSize). It handles the OWL TMDIFrame commands the
// legacy menus still send (CM_TILECHILDREN, ..., CM_EXIT).
//
// Menu merging (spec 10 Q5): MdiChild calls OnMdiChildActivate(child) when a
// child is activated and OnMdiChildActivate(nullptr) when the last one goes
// away. The library ships only the hook; the GL frame's menu swap (MANILA /
// GLE) is in its subclass, gl/port/ManilaGL.cpp (plan M5). The merge points
// are documented in MdiFrame.cpp.
//
#pragma once

#include "Window.h"

#include <vector>

namespace win32fx {

// OWL TMDIFrame command IDs (they appear in the legacy MANILA menu).
inline constexpr WORD CM_ARRANGEICONS    = 24335;
inline constexpr WORD CM_TILECHILDREN    = 24336;
inline constexpr WORD CM_CASCADECHILDREN = 24337;
inline constexpr WORD CM_CLOSECHILDREN   = 24338;
inline constexpr WORD CM_EXIT            = 24340;

enum class DockSide { Top, Bottom };

class MdiFrame : public Window {
public:
    // Registers `className` (once per process) and creates the frame with
    // `menu`. The Window menu (the one that lists children) is submenu
    // `windowMenuPos` of `menu`; -1 for none. MDI child IDs start at firstChildId.
    HWND CreateFrame(HINSTANCE inst, const wchar_t* className, const wchar_t* title, HMENU menu,
                     int windowMenuPos, HICON icon = nullptr, UINT firstChildId = 0xFF00);

    HWND mdiClient() const noexcept { return client_; }
    HINSTANCE instance() const noexcept { return inst_; }

    // A bar docked to an edge, full width, in docking order from the edge
    // inwards. height 0: the bar sizes and places itself on WM_SIZE (a status
    // bar, which always sits at the very bottom: dock it first).
    void Dock(HWND bar, DockSide side, int height);
    void Undock(HWND bar);
    void Layout();

    // The active MDI child, or null.
    HWND activeChild() const noexcept;

    // The menu-merge hook (see the header comment).
    virtual void OnMdiChildActivate(HWND /*child*/) {}

protected:
    bool OnCreate(LPCREATESTRUCTW cs) override;
    bool OnSize(UINT type, int cx, int cy) override;
    bool OnCommand(WORD id, WORD code, HWND ctl) override;
    bool OnDestroy() override;   // PostQuitMessage(0): the frame is the main window
    LRESULT DefaultProc(UINT msg, WPARAM wp, LPARAM lp) override;

private:
    struct Docked { HWND hwnd; DockSide side; int height; };

    HINSTANCE inst_ = nullptr;
    HWND client_ = nullptr;
    HMENU windowMenu_ = nullptr;
    UINT firstChildId_ = 0xFF00;
    std::vector<Docked> docked_;
};

class MdiChild : public Window {
public:
    // Registers `className` (once per process) and creates the child in
    // `parentFrame`'s MDICLIENT. Null on failure.
    HWND CreateChild(MdiFrame& parentFrame, const wchar_t* className, const wchar_t* title,
                     DWORD style = WS_OVERLAPPEDWINDOW, int x = CW_USEDEFAULT, int y = CW_USEDEFAULT,
                     int cx = CW_USEDEFAULT, int cy = CW_USEDEFAULT, HICON icon = nullptr);

    MdiFrame* frame() const noexcept { return frame_; }

protected:
    LRESULT DefaultProc(UINT msg, WPARAM wp, LPARAM lp) override;
    // Reports WM_MDIACTIVATE to the frame; an override must call this first.
    bool OnMessage(UINT msg, WPARAM wp, LPARAM lp, LRESULT& result) override;

private:
    MdiFrame* frame_ = nullptr;
};

} // namespace win32fx
