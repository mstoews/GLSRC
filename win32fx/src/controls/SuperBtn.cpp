//
// SuperBtn.cpp - tbSuperBtn: a push button that can show a picture.
//
// A superclass of the system BUTTON, so it is themed, takes part in the
// dialog manager's default-button handling (WM_GETDLGCODE is BUTTON's:
// DLGC_BUTTON | DLGC_DEFPUSHBUTTON / DLGC_UNDEFPUSHBUTTON) and sends BN_CLICKED.
//
// Styles: SBS_DEFPUSHBUTTON becomes BS_DEFPUSHBUTTON; the other SBS_* layout
// and frame bits are dropped. Buttons styled SBS_TEXTBITMAP / SBS_TEXTICON,
// or whose text is just "tbSuperBtn" (the toolbar buttons), show only their
// picture once SBM_SETPICT gives them one; other buttons show text and picture.
// SBS_NOPOINTERFOCUS is not implemented: a click focuses the button.
//
// SBM_SETPICT: wParam SUPERBTN_PICT_BITMAP or _ICON with lParam a resource
// ID or name in the button's module (loaded and owned), or with
// SUPERBTN_PICT_HANDLE an HBITMAP/HICON the caller keeps. SUPERBTN_PICT_NONE
// removes the picture. The other SBM_* messages are accepted and ignored.
//

#include "ControlsInternal.h"

#include <memory>

namespace win32fx::detail {

namespace {

constexpr DWORD tbSBS_DEFPUSHBUTTON = 0x0001;
constexpr DWORD tbSBS_TEXTICON      = 0x1000;
constexpr DWORD tbSBS_TEXTBITMAP    = 0x2000;

struct BtnState {
    HANDLE image = nullptr;
    UINT imageType = IMAGE_BITMAP;
    bool owned = false;
    bool pictureOnly = false;
};

WNDCLASSEXW g_button{};
int g_buttonExtra = 0;

LRESULT base(HWND h, UINT m, WPARAM w, LPARAM l) { return CallWindowProcW(g_button.lpfnWndProc, h, m, w, l); }

void freeImage(BtnState& st) noexcept
{
    if (st.owned && st.image) {
        if (st.imageType == IMAGE_ICON) DestroyIcon(static_cast<HICON>(st.image));
        else DeleteObject(st.image);
    }
    st.image = nullptr;
    st.owned = false;
}

LRESULT setPicture(BtnState& st, HWND h, WPARAM type, LPARAM lp)
{
    HANDLE image = nullptr;
    const UINT imageType = (type & SUPERBTN_PICT_ICON) ? IMAGE_ICON : IMAGE_BITMAP;
    bool owned = false;
    if (type & SUPERBTN_PICT_HANDLE) {
        image = reinterpret_cast<HANDLE>(lp);
    } else if (type & (SUPERBTN_PICT_BITMAP | SUPERBTN_PICT_ICON)) {
        auto inst = reinterpret_cast<HINSTANCE>(GetWindowLongPtrW(h, GWLP_HINSTANCE));
        image = LoadImageW(inst, reinterpret_cast<const wchar_t*>(lp), imageType, 0, 0, LR_DEFAULTCOLOR);
        if (!image) return FALSE;
        owned = true;
    }
    base(h, BM_SETIMAGE, imageType, reinterpret_cast<LPARAM>(image));
    freeImage(st);
    st.image = image;
    st.imageType = imageType;
    st.owned = owned;

    LONG_PTR style = GetWindowLongPtrW(h, GWL_STYLE) & ~static_cast<LONG_PTR>(BS_BITMAP | BS_ICON);
    if (st.pictureOnly && image) style |= (imageType == IMAGE_ICON) ? BS_ICON : BS_BITMAP;
    SetWindowLongPtrW(h, GWL_STYLE, style);
    InvalidateRect(h, nullptr, TRUE);
    return TRUE;
}

LRESULT onCreate(HWND h, WPARAM wp, LPARAM lp)
{
    auto* cs = reinterpret_cast<CREATESTRUCTW*>(lp);
    auto st = std::make_unique<BtnState>();
    const DWORD tb = static_cast<DWORD>(cs->style) & 0xFFFF;
    const bool classNameText = cs->lpszName && !IS_INTRESOURCE(cs->lpszName) &&
                               CompareStringOrdinal(cs->lpszName, -1, L"tbSuperBtn", -1, TRUE) == CSTR_EQUAL;
    st->pictureOnly = (tb & (tbSBS_TEXTICON | tbSBS_TEXTBITMAP)) || classNameText;
    const DWORD style = (static_cast<DWORD>(cs->style) & 0xFFFF0000u) |
                        ((tb & tbSBS_DEFPUSHBUTTON) ? BS_DEFPUSHBUTTON : BS_PUSHBUTTON);
    SetWindowLongPtrW(h, GWL_STYLE, static_cast<LONG_PTR>(style));
    cs->style = static_cast<LONG>(style);

    SetWindowLongPtrW(h, g_buttonExtra, reinterpret_cast<LONG_PTR>(st.get()));
    const LRESULT r = base(h, WM_NCCREATE, wp, lp);
    if (r) (void)st.release();
    else SetWindowLongPtrW(h, g_buttonExtra, 0);
    return r;
}

LRESULT dispatch(HWND h, UINT msg, WPARAM wp, LPARAM lp)
{
    if (msg == WM_NCCREATE) return onCreate(h, wp, lp);
    auto* st = reinterpret_cast<BtnState*>(GetWindowLongPtrW(h, g_buttonExtra));
    if (msg == WM_NCDESTROY) {
        const LRESULT r = base(h, msg, wp, lp);
        SetWindowLongPtrW(h, g_buttonExtra, 0);
        if (st) freeImage(*st);
        delete st;
        return r;
    }
    if (st && msg == SBM_SETPICT) return setPicture(*st, h, wp, lp);
    if (msg >= static_cast<UINT>(SBM_SETCOLOR) && msg <= static_cast<UINT>(SBM_GETSTRETCH)) return 0;
    return base(h, msg, wp, lp);
}

LRESULT CALLBACK SuperBtnProc(HWND h, UINT msg, WPARAM wp, LPARAM lp)
{
    try {
        return dispatch(h, msg, wp, lp);
    } catch (...) {   // std::bad_alloc only; nothing may reach user32
        return msg == WM_NCCREATE ? FALSE : base(h, msg, wp, lp);
    }
}

} // namespace

bool RegisterSuperBtn(HINSTANCE inst)
{
    if (!g_button.lpfnWndProc) {
        WNDCLASSEXW b{};
        b.cbSize = sizeof b;
        if (!GetClassInfoExW(nullptr, L"BUTTON", &b)) return false;
        g_button = b;
        g_buttonExtra = b.cbWndExtra;
    }
    WNDCLASSEXW existing{};
    existing.cbSize = sizeof existing;
    if (GetClassInfoExW(inst, L"tbSuperBtn", &existing)) return true;

    WNDCLASSEXW wc = g_button;
    wc.cbSize = sizeof wc;
    wc.style |= CS_GLOBALCLASS | CS_DBLCLKS;
    wc.lpfnWndProc = SuperBtnProc;
    wc.cbWndExtra = g_buttonExtra + static_cast<int>(sizeof(LONG_PTR));
    wc.hInstance = inst;
    wc.lpszMenuName = nullptr;
    wc.lpszClassName = L"tbSuperBtn";
    return RegisterClassExW(&wc) != 0;
}

} // namespace win32fx::detail
