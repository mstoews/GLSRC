//
// StatusBar.cpp - see StatusBar.h.
//

#include "StatusBar.h"

#include <CommCtrl.h>

#include <vector>

#pragma comment(lib, "comctl32.lib")

namespace win32fx {

HWND StatusBar::Create(HWND parent, UINT id, HINSTANCE inst)
{
    hwnd_ = CreateWindowExW(0, STATUSCLASSNAMEW, nullptr, WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP, 0, 0, 0, 0,
                            parent, reinterpret_cast<HMENU>(static_cast<UINT_PTR>(id)), inst, nullptr);
    return hwnd_;
}

void StatusBar::SetParts(std::initializer_list<int> rightEdges)
{
    std::vector<int> parts(rightEdges);
    SendMessageW(hwnd_, SB_SETPARTS, parts.size(), reinterpret_cast<LPARAM>(parts.data()));
}

void StatusBar::SetText(int part, const wchar_t* text)
{
    SendMessageW(hwnd_, SB_SETTEXTW, static_cast<WPARAM>(part), reinterpret_cast<LPARAM>(text ? text : L""));
}

} // namespace win32fx
