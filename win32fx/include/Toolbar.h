//
// Toolbar.h - the top toolbar (spec 3.5): the original TToolBar.
//
// The legacy toolbars are modeless child dialogs built from a template
// (dialog 1032 for the top bar, 2057 for the module bar) whose buttons are
// tbSuperBtn controls with bitmaps. DialogBar is that: a child Dialog that
// sends every WM_COMMAND from its buttons on to its parent, the frame, which
// the original did one handler at a time (TToolBar::Mssg). A port that needs
// a different ID sent overrides MapCommand.
//
//     Toolbar bar;
//     bar.CreateBar(hInst, 1032, frame);
//     bar.SetBackground(L"STATUSBAR");
//     bar.SetLayout(10, 30, 25);
//     bar.PlaceButton(ID_NEW, 0, true);      // TToolBar::SetPic
//     frame.Dock(bar, win32fx::DockSide::Top, bar.height());
//
#pragma once

#include "Dialog.h"
#include "HandleGuard.h"

namespace win32fx {

class DialogBar : public Dialog {
public:
    // A modeless child dialog from `templateId` (WS_CHILD template), shown.
    HWND CreateBar(HINSTANCE inst, UINT templateId, HWND parent);

    // Button grid used by PlaceButton: `space` px from the left, slots
    // `width` x `height` px, top 2 px. Original values: 10, 30, 25.
    void SetLayout(int spacePx, int widthPx, int heightPx) noexcept;

    // The original SetPic: moves button `id` to slot `slot` (fractional
    // slots make the gaps between groups), shows or hides it, and gives it
    // the bitmap resource with the same ID. Returns the SBM_SETPICT result.
    LRESULT PlaceButton(int id, double slot, bool show);

    int height() const noexcept { return barHeight_; }
    void setHeight(int h) noexcept { barHeight_ = h; }

protected:
    // The ID to send to the parent for a click on `id`; 0 to send nothing.
    virtual WORD MapCommand(WORD id) { return id; }

    bool OnCommand(WORD id, WORD code, HWND ctl) override;

    HINSTANCE inst_ = nullptr;
    int space_ = 10, width_ = 30, btnHeight_ = 25;
    int barHeight_ = 30;   // TOOLBAR_HEIGHT
};

class Toolbar : public DialogBar {
public:
    // Paints bitmap resource `name` stretched across the bar, as
    // TToolBar::WMPaint did with "STATUSBAR". Null: the dialog background.
    void SetBackground(const wchar_t* bitmapName);

protected:
    // Paints the background (WM_ERASEBKGND); an override must call this first.
    bool OnMessage(UINT msg, WPARAM wp, LPARAM lp, LRESULT& result) override;

private:
    BitmapGuard background_;
    int bmpWidth_ = 0, bmpHeight_ = 0;
};

} // namespace win32fx
