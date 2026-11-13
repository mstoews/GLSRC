//
// Toolbar.cpp - see Toolbar.h.
//

#include "Toolbar.h"
#include "controls/CustomControls.h"

#pragma comment(lib, "comctl32.lib")

namespace win32fx {

CommandBar::~CommandBar()
{
    if (hwnd_ && IsWindow(hwnd_)) DestroyWindow(hwnd_);
    if (images_) ImageList_Destroy(images_);
}

HWND CommandBar::Create(HWND parent, UINT id, HINSTANCE inst, int iconPx)
{
    inst_ = inst;
    parent_ = parent;
    id_ = id;
    iconPx96_ = iconPx;
    // CCS_NORESIZE | CCS_NOPARENTALIGN: MdiFrame::Dock places and sizes it.
    hwnd_ = CreateWindowExW(0, TOOLBARCLASSNAMEW, nullptr,
                            WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | TBSTYLE_FLAT | TBSTYLE_TOOLTIPS |
                                CCS_NODIVIDER | CCS_NORESIZE | CCS_NOPARENTALIGN,
                            0, 0, 0, 0, parent, reinterpret_cast<HMENU>(static_cast<UINT_PTR>(id)), inst, nullptr);
    if (!hwnd_) return nullptr;
    SendMessageW(hwnd_, TB_BUTTONSTRUCTSIZE, sizeof(TBBUTTON), 0);
    iconPx_ = MulDiv(iconPx, static_cast<int>(GetDpiForWindow(parent)), 96);
    if (images_) ImageList_Destroy(images_);
    images_ = ImageList_Create(iconPx_, iconPx_, ILC_COLOR32 | ILC_MASK, 16, 4);
    SendMessageW(hwnd_, TB_SETIMAGELIST, 0, reinterpret_cast<LPARAM>(images_));
    // No text under the icons: the strings show only as tooltips.
    SendMessageW(hwnd_, TB_SETMAXTEXTROWS, 0, 0);
    return hwnd_;
}

void CommandBar::AddButton(int picture, WORD command, const std::wstring& tip)
{
    buttons_.push_back({picture, command, tip});
    addButton(buttons_.back());
}

void CommandBar::AddSeparator()
{
    buttons_.push_back({0, 0, {}});
    addButton(buttons_.back());
}

void CommandBar::addButton(const Button& button)
{
    if (!hwnd_) return;
    TBBUTTON b{};
    if (button.picture == 0) {
        b.fsStyle = BTNS_SEP;
    } else {
        b.iBitmap = I_IMAGENONE;
        if (HICON icon = LoadModernIcon(inst_, MAKEINTRESOURCEW(button.picture), iconPx_)) {
            if (images_) b.iBitmap = ImageList_AddIcon(images_, icon);
            DestroyIcon(icon);
        }
        b.idCommand = button.command;
        b.fsState = TBSTATE_ENABLED;
        b.fsStyle = BTNS_BUTTON;
        b.iString = reinterpret_cast<INT_PTR>(button.tip.c_str());   // buttons_ is a list: it stays put
    }
    SendMessageW(hwnd_, TB_ADDBUTTONSW, 1, reinterpret_cast<LPARAM>(&b));
}

HWND CommandBar::Rebuild()
{
    if (hwnd_ && IsWindow(hwnd_)) DestroyWindow(hwnd_);
    hwnd_ = nullptr;
    if (!Create(parent_, id_, inst_, iconPx96_)) return nullptr;
    for (const Button& b : buttons_) addButton(b);
    return hwnd_;
}

int CommandBar::Height() const
{
    if (!hwnd_) return 0;
    const int pad = MulDiv(2, static_cast<int>(GetDpiForWindow(hwnd_)), 96);
    return HIWORD(SendMessageW(hwnd_, TB_GETBUTTONSIZE, 0, 0)) + 2 * pad;
}

} // namespace win32fx
