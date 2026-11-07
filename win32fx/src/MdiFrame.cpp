//
// MdiFrame.cpp - see MdiFrame.h.
//
// Menu merge (spec 10 Q5). A subclass overrides OnMdiChildActivate(child);
// gl/port/ManilaGL.cpp does (plan M5). Merge points, from the legacy code:
//   - frame menu MANILA (HGLMenu, loaded in WinMain); its Window menu is
//     submenu 7 (TManilaGL: ChildMenuPos = 7), passed to CreateFrame;
//   - a GL child brings the GLE menu (HGLExMenu); GLSUB is the right-click
//     popup (TGlWindow::PopUpMenu), not merged;
//   - swap with WM_MDISETMENU(frameMenu, windowMenu) to the client, then
//     DrawMenuBar(frame). OnMdiChildActivate(nullptr) restores MANILA.
//

#include "MdiFrame.h"

#include <algorithm>
#include <stdexcept>

namespace win32fx {

namespace {

bool ensureClass(HINSTANCE inst, const wchar_t* className, HICON icon, HBRUSH background)
{
    WNDCLASSEXW existing{};
    existing.cbSize = sizeof existing;
    if (GetClassInfoExW(inst, className, &existing)) return true;
    WNDCLASSEXW wc = Window::ClassInfo(inst, className);
    wc.hIcon = icon;
    wc.hIconSm = icon;
    wc.hbrBackground = background;
    return RegisterClassExW(&wc) != 0;
}

} // namespace

// ---- MdiFrame ----

HWND MdiFrame::CreateFrame(HINSTANCE inst, const wchar_t* className, const wchar_t* title, HMENU menu,
                           int windowMenuPos, HICON icon, UINT firstChildId)
{
    inst_ = inst;
    firstChildId_ = firstChildId;
    windowMenu_ = (menu && windowMenuPos >= 0) ? GetSubMenu(menu, windowMenuPos) : nullptr;
    if (!ensureClass(inst, className, icon, reinterpret_cast<HBRUSH>(COLOR_APPWORKSPACE + 1))) return nullptr;
    return Create(0, className, title, WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, CW_USEDEFAULT, CW_USEDEFAULT,
                  CW_USEDEFAULT, CW_USEDEFAULT, nullptr, menu, inst);
}

bool MdiFrame::OnCreate(LPCREATESTRUCTW)
{
    CLIENTCREATESTRUCT ccs{};
    ccs.hWindowMenu = windowMenu_;
    ccs.idFirstChild = firstChildId_;
    client_ = CreateWindowExW(WS_EX_CLIENTEDGE, L"MDICLIENT", nullptr,
                              WS_CHILD | WS_CLIPCHILDREN | WS_VSCROLL | WS_HSCROLL | WS_VISIBLE,
                              0, 0, 0, 0, hwnd(), nullptr, inst_, &ccs);
    if (!client_) throw std::runtime_error("cannot create the MDI client window");
    return true;
}

void MdiFrame::Dock(HWND bar, DockSide side, int height)
{
    Undock(bar);
    docked_.push_back({bar, side, height});
    Layout();
}

void MdiFrame::Undock(HWND bar)
{
    std::erase_if(docked_, [bar](const Docked& d) { return d.hwnd == bar; });
}

void MdiFrame::Layout()
{
    if (!hwnd()) return;
    RECT rc{};
    GetClientRect(hwnd(), &rc);
    const int width = rc.right - rc.left;
    int top = rc.top, bottom = rc.bottom;
    for (const Docked& d : docked_) {
        if (!IsWindow(d.hwnd) || !IsWindowVisible(d.hwnd)) continue;
        int h = d.height;
        if (h == 0) {   // sizes and places itself (status bar): bottom edge
            SendMessageW(d.hwnd, WM_SIZE, 0, 0);
            RECT wr{};
            GetWindowRect(d.hwnd, &wr);
            h = wr.bottom - wr.top;
            if (d.side == DockSide::Bottom) { bottom -= h; continue; }
        }
        if (d.side == DockSide::Top) {
            MoveWindow(d.hwnd, 0, top, width, h, TRUE);
            top += h;
        } else {
            bottom -= h;
            MoveWindow(d.hwnd, 0, bottom, width, h, TRUE);
        }
    }
    if (client_) MoveWindow(client_, 0, top, width, std::max(0, bottom - top), TRUE);
}

bool MdiFrame::OnSize(UINT type, int, int)
{
    if (type != SIZE_MINIMIZED) Layout();
    return true;   // not DefFrameProc: it would size the client over the bars
}

bool MdiFrame::OnCommand(WORD id, WORD, HWND)
{
    switch (id) {
    case CM_TILECHILDREN:    SendMessageW(client_, WM_MDITILE, MDITILE_VERTICAL, 0); return true;
    case CM_CASCADECHILDREN: SendMessageW(client_, WM_MDICASCADE, 0, 0); return true;
    case CM_ARRANGEICONS:    SendMessageW(client_, WM_MDIICONARRANGE, 0, 0); return true;
    case CM_CLOSECHILDREN: {
        std::vector<HWND> children;
        for (HWND c = GetWindow(client_, GW_CHILD); c; c = GetWindow(c, GW_HWNDNEXT))
            if (!GetWindow(c, GW_OWNER)) children.push_back(c);
        for (HWND c : children) SendMessageW(c, WM_CLOSE, 0, 0);
        return true;
    }
    case CM_EXIT:
        PostMessageW(hwnd(), WM_CLOSE, 0, 0);
        return true;
    default:
        return false;   // DefFrameProc: Window-menu child IDs, forwarding to the active child
    }
}

bool MdiFrame::OnDestroy()
{
    PostQuitMessage(0);
    return true;
}

HWND MdiFrame::activeChild() const noexcept
{
    return client_ ? reinterpret_cast<HWND>(SendMessageW(client_, WM_MDIGETACTIVE, 0, 0)) : nullptr;
}

LRESULT MdiFrame::DefaultProc(UINT msg, WPARAM wp, LPARAM lp)
{
    return DefFrameProcW(hwnd(), client_, msg, wp, lp);
}

// ---- MdiChild ----

HWND MdiChild::CreateChild(MdiFrame& parentFrame, const wchar_t* className, const wchar_t* title, DWORD style,
                           int x, int y, int cx, int cy, HICON icon)
{
    frame_ = &parentFrame;
    if (!ensureClass(parentFrame.instance(), className, icon, reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1)))
        return nullptr;
    return CreateMDIWindowW(className, title, style, x, y, cx, cy, parentFrame.mdiClient(), parentFrame.instance(),
                            reinterpret_cast<LPARAM>(static_cast<Window*>(this)));
}

bool MdiChild::OnMessage(UINT msg, WPARAM, LPARAM lp, LRESULT&)
{
    if (msg == WM_MDIACTIVATE && frame_) {
        const HWND activated = reinterpret_cast<HWND>(lp);
        if (activated == hwnd()) frame_->OnMdiChildActivate(activated);
        else if (!activated) frame_->OnMdiChildActivate(nullptr);   // the last child is going
    }
    return false;   // DefMDIChildProc too
}

LRESULT MdiChild::DefaultProc(UINT msg, WPARAM wp, LPARAM lp)
{
    return DefMDIChildProcW(hwnd(), msg, wp, lp);
}

} // namespace win32fx
