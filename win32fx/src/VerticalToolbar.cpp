//
// VerticalToolbar.cpp - see VerticalToolbar.h.
//

#include "VerticalToolbar.h"

namespace win32fx {

void VerticalToolbar::SetText(int id, const wchar_t* text)
{
    SetDlgItemTextW(hwnd(), id, text ? text : L"");
}

void VerticalToolbar::PlaceField(int id, int x, int width)
{
    RECT rc{};
    GetClientRect(hwnd(), &rc);
    MoveWindow(GetDlgItem(hwnd(), id), x, rc.top + 4, width, btnHeight_ - space_, TRUE);   // TGlWindow::WMSize
}

} // namespace win32fx
