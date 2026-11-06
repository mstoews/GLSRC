//
// Window.cpp - see Window.h.
//

#include "Window.h"
#include "Internal.h"

#include <exception>

namespace win32fx {

namespace {

// Messages DefMDIChildProc must see even when the child handled them.
bool mdiChildNeedsDefault(UINT msg) noexcept
{
    switch (msg) {
    case WM_CHILDACTIVATE: case WM_GETMINMAXINFO: case WM_MENUCHAR: case WM_MOVE:
    case WM_NEXTMENU: case WM_SETFOCUS: case WM_SIZE: case WM_SYSCOMMAND:
        return true;
    default:
        return false;
    }
}

bool isMdiChild(HWND h) noexcept { return (GetWindowLongW(h, GWL_EXSTYLE) & WS_EX_MDICHILD) != 0; }

} // namespace

Window::~Window()
{
    if (HWND h = hwnd_) {
        SetWindowLongPtrW(h, GWLP_USERDATA, 0);   // no more dispatch to this object
        hwnd_ = nullptr;
        DestroyWindow(h);
    }
}

WNDCLASSEXW Window::ClassInfo(HINSTANCE inst, const wchar_t* className) noexcept
{
    WNDCLASSEXW wc{};
    wc.cbSize = sizeof wc;
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wc.lpfnWndProc = &Window::WndProc;
    wc.hInstance = inst;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wc.lpszClassName = className;
    return wc;
}

HWND Window::Create(DWORD exStyle, const wchar_t* className, const wchar_t* title, DWORD style,
                    int x, int y, int cx, int cy, HWND parent, HMENU menuOrId, HINSTANCE inst)
{
    return CreateWindowExW(exStyle, className, title, style, x, y, cx, cy, parent, menuOrId, inst, this);
}

Window* Window::FromHandle(HWND h) noexcept
{
    if (!h || reinterpret_cast<WNDPROC>(GetClassLongPtrW(h, GCLP_WNDPROC)) != &Window::WndProc)
        return nullptr;
    return reinterpret_cast<Window*>(GetWindowLongPtrW(h, GWLP_USERDATA));
}

LRESULT CALLBACK Window::WndProc(HWND h, UINT msg, WPARAM wp, LPARAM lp)
{
    Window* self = nullptr;
    if (msg == WM_NCCREATE) {
        const auto* cs = reinterpret_cast<const CREATESTRUCTW*>(lp);
        LPARAM param = reinterpret_cast<LPARAM>(cs->lpCreateParams);
        if ((cs->dwExStyle & WS_EX_MDICHILD) && param)   // CreateMDIWindowW: lpParam is inside
            param = reinterpret_cast<const MDICREATESTRUCTW*>(param)->lParam;
        self = reinterpret_cast<Window*>(param);
        if (self) {
            self->hwnd_ = h;
            SetWindowLongPtrW(h, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
        }
    } else {
        self = reinterpret_cast<Window*>(GetWindowLongPtrW(h, GWLP_USERDATA));
    }
    if (!self)
        return isMdiChild(h) ? DefMDIChildProcW(h, msg, wp, lp) : DefWindowProcW(h, msg, wp, lp);

    LRESULT r = 0;
    try {
        r = self->Dispatch(msg, wp, lp);
    } catch (const std::exception& e) {
        detail::ReportException(h, e.what());
        r = (msg == WM_CREATE) ? -1 : 0;
    } catch (...) {
        detail::ReportException(h, nullptr);
        r = (msg == WM_CREATE) ? -1 : 0;
    }

    if (msg == WM_NCDESTROY && reinterpret_cast<Window*>(GetWindowLongPtrW(h, GWLP_USERDATA)) == self) {
        SetWindowLongPtrW(h, GWLP_USERDATA, 0);
        self->hwnd_ = nullptr;
        try {
            self->OnFinalMessage();
        } catch (const std::exception& e) {
            detail::ReportException(nullptr, e.what());
        } catch (...) {
            detail::ReportException(nullptr, nullptr);
        }
    }
    return r;
}

LRESULT Window::Dispatch(UINT msg, WPARAM wp, LPARAM lp)
{
    // A handler may destroy this object; after it runs only locals are used.
    const HWND h = hwnd_;
    const bool mdiDefault = mdiChildNeedsDefault(msg) && isMdiChild(h);
    LRESULT r = 0;
    bool handled = ProcessMessageMap(msg, wp, lp, r);
    if (!handled) {
        switch (msg) {
        case WM_CREATE:
            handled = OnCreate(reinterpret_cast<LPCREATESTRUCTW>(lp));
            break;
        case WM_COMMAND:
            handled = OnCommand(LOWORD(wp), HIWORD(wp), reinterpret_cast<HWND>(lp));
            break;
        case WM_SIZE:
            handled = OnSize(static_cast<UINT>(wp), static_cast<short>(LOWORD(lp)), static_cast<short>(HIWORD(lp)));
            break;
        case WM_DESTROY:
            handled = OnDestroy();
            break;
        default:
            break;
        }
        if (handled) r = 0;
    }
    if (!handled) handled = OnMessage(msg, wp, lp, r);
    if (!handled) return DefaultProc(msg, wp, lp);
    if (mdiDefault) return DefMDIChildProcW(h, msg, wp, lp);
    return r;
}

LRESULT Window::DefaultProc(UINT msg, WPARAM wp, LPARAM lp)
{
    return DefWindowProcW(hwnd_, msg, wp, lp);
}

} // namespace win32fx
