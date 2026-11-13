//
// Toolbar.h - the frame's command bar (UI refresh spec 4.3, D3): a common-
// controls toolbar (ToolbarWindow32) with flat icon buttons and tooltips. It
// replaced the original TToolBar, a dialog of tbSuperBtn buttons (1032).
//
// A thin wrapper, as StatusBar: the control has its own window procedure,
// subclassed only to draw the bar's edge: a 2 px white border and a 2 px gap
// on every side, so the buttons sit in the middle of the bar.
// Each button shows the modern icon for a bitmap ID (LoadModernIcon, "P<id>")
// and sends a command: a click arrives at the parent as WM_COMMAND with the
// command, BN_CLICKED and the bar's HWND. A frame that tells menu commands from
// control notifications by a null lParam should treat the bar's as null.
//
//     fx::CommandBar bar;
//     bar.Create(frame, IDC_TOOLBAR, inst);
//     bar.AddButton(ID_SAVE, ID_SAVE, L"Save");
//     bar.AddSeparator();
//     frame.Dock(bar, fx::DockSide::Top, bar.Height());
//
#pragma once

#include <Windows.h>
#include <CommCtrl.h>

#include <list>
#include <string>

namespace win32fx {

class CommandBar {
public:
    CommandBar() noexcept = default;
    CommandBar(const CommandBar&) = delete;
    CommandBar& operator=(const CommandBar&) = delete;
    ~CommandBar();

    // A child of `parent` with ID `id`; icons `iconPx` px at 96 DPI, scaled to
    // the parent's DPI. Null on failure.
    HWND Create(HWND parent, UINT id, HINSTANCE inst, int iconPx = 24);

    // A button with the icon that replaces bitmap `picture`, sending `command`,
    // with tooltip `tip`. Without such an icon the button has no picture.
    void AddButton(int picture, WORD command, const std::wstring& tip);
    void AddSeparator();

    // The bar's height for its buttons, in px: dock it with this.
    int Height() const;

    // After a DPI change (UI refresh spec 5.2): the bar is made again with its
    // icons at the parent's new DPI. Returns the new HWND: the old one is
    // destroyed, so undock it first and dock this one.
    HWND Rebuild();

    HWND hwnd() const noexcept { return hwnd_; }
    operator HWND() const noexcept { return hwnd_; }

private:
    struct Button {
        int picture;   // 0: a separator
        WORD command;
        std::wstring tip;
    };
    void addButton(const Button& b);

    HWND hwnd_ = nullptr;
    HWND parent_ = nullptr;
    UINT id_ = 0;
    int iconPx96_ = 24;
    std::list<Button> buttons_;
    HINSTANCE inst_ = nullptr;
    HIMAGELIST images_ = nullptr;   // owned: the toolbar doesn't destroy it
    int iconPx_ = 24;               // at the current DPI
};

} // namespace win32fx
