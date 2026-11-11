//
// ManilaGL.h - the MDI frame: the original TManilaGL / TGlWindow
// (gl/src/gl.cpp, gl/header/gl_main.h, gl_man.h). Plan M5 task 3.
//
//   - MANILA menu (HGLMenu); the Window menu is submenu 7.
//   - Toolbar 1032 at the top (TToolBar), module bar 2057 at the bottom
//     (TVToolBar). The module bar's three fields are the status line:
//     3222 company name, 3223 menu help, 3224 current period.
//   - GL.INI [ACCESS] PSW=T: a login over the frame at startup; Cancel
//     closes the program. The menu-access string grays menu items on the
//     next WM_INITMENUPOPUP.
//   - A right-click on the MDI client opens the GLSUB popup (TGLClient).
//   - Menu commands outside Spec #1 go to the AP / AR / Reporting stubs, or
//     to a general "not yet ported" message (ManilaGL.cpp lists which).
//
// Menu swap (spec 10 Q5, as the original did it): an MDI child that wants
// the GLE menu calls ManilaGL::UseGleMenu(child) once it exists, and
// ManilaGL::ReleaseGleMenu(child) on WM_DESTROY. While such a child is
// active the frame shows GLE, otherwise MANILA. Only the spreadsheet window
// (gl_xls.cpp, not in Spec #1) did this in the original.
//
#pragma once

#include "win32fx.h"

#include <string>

class ManilaGL : public fx::MdiFrame {
public:
    HWND Create(HINSTANCE inst);

    // Rereads the company name and current period into the status fields.
    void RefreshStatus();

    static void UseGleMenu(HWND child);
    static void ReleaseGleMenu(HWND child);

    void OnMdiChildActivate(HWND child) override;

protected:
    bool OnCreate(LPCREATESTRUCTW cs) override;
    bool OnSize(UINT type, int cx, int cy) override;
    bool OnCommand(WORD id, WORD code, HWND ctl) override;
    bool OnDestroy() override;
    bool OnMessage(UINT msg, WPARAM wp, LPARAM lp, LRESULT& result) override;

private:
    class TopToolbar : public fx::Toolbar {
    protected:
        WORD MapCommand(WORD id) override;   // TToolBar's response table
    };

    void MenuHelp(WPARAM wp, LPARAM lp);       // TGlWindow::WMMenuSelect
    void PopUpMenu(POINT screen);              // TGlWindow::PopUpMenu
    void NotPorted(WORD id);
    void RunReportList();                      // TGLCustomReport: dialog 1022 (spec #3 D9)
    std::wstring MenuText(WORD id) const;

    static LRESULT CALLBACK ClientProc(HWND h, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR, DWORD_PTR ref);

    TopToolbar toolbar_;
    fx::VerticalToolbar vbar_;
};
