//
// Toolbar.cpp - see Toolbar.h.
//

#include "Toolbar.h"
#include "controls/CustomControls.h"

namespace win32fx {

// ---- DialogBar ----

HWND DialogBar::CreateBar(HINSTANCE inst, UINT templateId, HWND parent)
{
    inst_ = inst;
    HWND h = CreateModeless(inst, templateId, parent);
    if (h) ShowWindow(h, SW_SHOW);
    return h;
}

void DialogBar::SetLayout(int spacePx, int widthPx, int heightPx) noexcept
{
    space_ = spacePx;
    width_ = widthPx;
    btnHeight_ = heightPx;
}

LRESULT DialogBar::PlaceButton(int id, double slot, bool show)
{
    HWND btn = GetDlgItem(hwnd(), id);
    if (!btn) return 0;
    RECT rc{};
    GetClientRect(hwnd(), &rc);
    MoveWindow(btn, static_cast<int>(rc.left + space_ + slot * width_), rc.top + 2, width_, btnHeight_, TRUE);
    ShowWindow(btn, show ? SW_SHOW : SW_HIDE);
    SendMessageW(btn, SBM_SETSHADOWSIZE, 1, 0);
    return SendMessageW(btn, SBM_SETPICT, SUPERBTN_PICT_BITMAP, id);
}

bool DialogBar::OnCommand(WORD id, WORD code, HWND ctl)
{
    // The bar is never closed by Enter/Esc (IDOK/IDCANCEL without a control).
    if (!ctl) return true;
    if (code == BN_CLICKED)
        if (const WORD to = MapCommand(id)) SendMessageW(GetParent(hwnd()), WM_COMMAND, to, 0);   // TToolBar::Mssg
    return true;
}

// ---- Toolbar ----

void Toolbar::SetBackground(const wchar_t* bitmapName)
{
    background_.reset();
    bmpWidth_ = bmpHeight_ = 0;
    if (bitmapName)
        background_.reset(static_cast<HBITMAP>(LoadImageW(inst_, bitmapName, IMAGE_BITMAP, 0, 0, LR_DEFAULTCOLOR)));
    if (background_) {
        BITMAP bm{};
        GetObjectW(background_.get(), sizeof bm, &bm);
        bmpWidth_ = bm.bmWidth;
        bmpHeight_ = bm.bmHeight;
    }
    if (hwnd()) InvalidateRect(hwnd(), nullptr, TRUE);
}

bool Toolbar::OnMessage(UINT msg, WPARAM wp, LPARAM, LRESULT& result)
{
    if (msg != WM_ERASEBKGND || !background_) return false;
    // TToolBar::WMPaint: the left 10 px of the bitmap stretched across the bar.
    const HDC dc = reinterpret_cast<HDC>(wp);
    RECT rc{};
    GetClientRect(hwnd(), &rc);
    MemDcGuard mem(CreateCompatibleDC(dc));
    const HGDIOBJ old = SelectObject(mem.get(), background_.get());
    StretchBlt(dc, 0, 0, rc.right - rc.left, rc.bottom - rc.top, mem.get(), 0, 0,
               bmpWidth_ < 10 ? bmpWidth_ : 10, bmpHeight_ < barHeight_ ? bmpHeight_ : barHeight_, SRCCOPY);
    SelectObject(mem.get(), old);
    result = 1;
    return true;
}

} // namespace win32fx
