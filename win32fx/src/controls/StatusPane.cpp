//
// StatusPane.cpp - "tbStatusBar", the Toolbox status pane a dialog template
// can hold (dialog 1045, Create Cheques: IDD_STATUSBOX). Without the class
// the template fails to load (as "tbTime" did, Spec #4 S4-M2).
//
// A one-line pane: the window text, drawn left-aligned in the dialog's font
// on the button face, in a sunken border. WM_SETTEXT (SetDlgItemText)
// changes it. The template's style bits (Toolbox's alignment and border
// options) are ignored.
//

#include "ControlsInternal.h"

namespace {

struct PaneState {
    HFONT font = nullptr;
};

LRESULT CALLBACK PaneProc(HWND h, UINT msg, WPARAM wp, LPARAM lp)
{
    auto* st = reinterpret_cast<PaneState*>(GetWindowLongPtrW(h, GWLP_USERDATA));
    switch (msg) {
    case WM_NCCREATE:
        SetWindowLongPtrW(h, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(new PaneState));
        break;
    case WM_NCDESTROY:
        delete st;
        SetWindowLongPtrW(h, GWLP_USERDATA, 0);
        break;
    case WM_SETFONT:
        if (st) st->font = reinterpret_cast<HFONT>(wp);
        if (LOWORD(lp)) InvalidateRect(h, nullptr, TRUE);
        return 0;
    case WM_GETFONT:
        return st ? reinterpret_cast<LRESULT>(st->font) : 0;
    case WM_SETTEXT: {
        const LRESULT r = DefWindowProcW(h, msg, wp, lp);
        InvalidateRect(h, nullptr, TRUE);
        return r;
    }
    case WM_ERASEBKGND: return 1;
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC dc = BeginPaint(h, &ps);
        RECT rc{};
        GetClientRect(h, &rc);
        FillRect(dc, &rc, GetSysColorBrush(COLOR_BTNFACE));
        DrawEdge(dc, &rc, BDR_SUNKENOUTER, BF_RECT);
        const int len = GetWindowTextLengthW(h);
        if (len > 0) {
            std::wstring text(static_cast<size_t>(len) + 1, L'\0');
            GetWindowTextW(h, text.data(), len + 1);
            text.resize(static_cast<size_t>(len));
            HGDIOBJ old = st && st->font ? SelectObject(dc, st->font) : nullptr;
            SetBkMode(dc, TRANSPARENT);
            SetTextColor(dc, GetSysColor(COLOR_BTNTEXT));
            RECT t = rc;
            InflateRect(&t, -4, -1);
            DrawTextW(dc, text.c_str(), len, &t, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
            if (old) SelectObject(dc, old);
        }
        EndPaint(h, &ps);
        return 0;
    }
    default: break;
    }
    return DefWindowProcW(h, msg, wp, lp);
}

} // namespace

bool win32fx::detail::RegisterStatusPane(HINSTANCE inst)
{
    WNDCLASSEXW wc{};
    wc.cbSize = sizeof wc;
    if (GetClassInfoExW(inst, L"tbStatusBar", &wc)) return true;
    wc.cbSize = sizeof wc;
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = PaneProc;
    wc.hInstance = inst;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.lpszClassName = L"tbStatusBar";
    return RegisterClassExW(&wc) != 0 || GetLastError() == ERROR_CLASS_ALREADY_EXISTS;
}
