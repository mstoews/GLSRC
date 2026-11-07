//
// App.cpp - gl.exe entry point and the original module globals (spec 2.4, 3.7).
//
// M3 smoke stub (plan M3 exit criteria): an MdiFrame with the MANILA menu and
// a status bar. File > New opens an empty MDI child, so the Window menu, the
// OWL CM_* commands and the OnMdiChildActivate hook can be tried; every other
// menu command just names itself in the status bar. Plan M5 replaces this
// with the real WinMain (ManilaGL frame, login, company profile). The legacy
// reference is gl/src/gl.cpp.
//
//   gl.exe /checkres [/quiet]   loads every Spec #1 resource (plan M4; ResourceCheck.cpp)
//
#include <Windows.h>

#include "win32fx.h"

#include "ResourceCheck.h"

#include "cb4sqlite.h"

#include <string>

#include "legacy_headers_begin.h"
#include "db_accts.h"
#include "legacy_headers_end.h"

// The original globals (gl/src/gl.cpp), kept as plain globals (spec 3.7).
// Other port files declare them extern; win32fx never touches them.
HINSTANCE hInst = nullptr;
HMENU HGLMenu = nullptr;
Code4 cb;
WCHAR szApp[20] = L"TIAS";

namespace {

constexpr WORD ID_FILENEW = 111;     // MANILA File > New
constexpr UINT ID_STATUSBAR = 100;
constexpr int WINDOW_MENU_POS = 7;   // TManilaGL: ChildMenuPos = 7

class SmokeChild : public fx::MdiChild {
protected:
    void OnFinalMessage() override { delete this; }   // owns itself, like an OWL child
};

class SmokeFrame : public fx::MdiFrame {
protected:
    bool OnCreate(LPCREATESTRUCTW cs) override
    {
        MdiFrame::OnCreate(cs);
        status_.Create(hwnd(), ID_STATUSBAR, hInst);
        status_.SetText(0, L"M3 smoke test: win32fx MdiFrame, MANILA menu, status bar");
        Dock(status_, fx::DockSide::Bottom, 0);
        return true;
    }

    bool OnCommand(WORD id, WORD code, HWND ctl) override
    {
        if (MdiFrame::OnCommand(id, code, ctl)) return true;
        if (id == ID_FILENEW) {
            auto* child = new SmokeChild;
            if (!child->CreateChild(*this, L"GLSmokeChild", fx::Format(L"Child %d", ++children_).c_str()))
                delete child;
            return true;
        }
        if (!ctl && id < 0xFF00) {   // a menu command (child-window IDs go to DefFrameProc)
            status_.SetText(0, fx::Format(L"Command %u is not part of the M3 smoke build", id).c_str());
            return true;
        }
        return false;
    }

    void OnMdiChildActivate(HWND child) override
    {
        status_.SetText(0, child ? (L"Active: " + fx::GetText(child)).c_str() : L"No MDI children");
    }

private:
    fx::StatusBar status_;
    int children_ = 0;
};

} // namespace

int WINAPI wWinMain(_In_ HINSTANCE inst, _In_opt_ HINSTANCE, _In_ LPWSTR cmdLine, _In_ int show)
{
    hInst = inst;
    if (!fx::RegisterCustomControls(inst)) return 1;
    const std::wstring args = cmdLine ? cmdLine : L"";
    if (args.find(L"/checkres") != std::wstring::npos)
        return RunResourceCheck(inst, args.find(L"/quiet") != std::wstring::npos);

    HGLMenu = LoadMenuW(inst, L"MANILA");   // destroyed with the frame
    if (!HGLMenu) return 1;

    SmokeFrame frame;
    if (!frame.CreateFrame(inst, L"TManilaGL", szApp, HGLMenu, WINDOW_MENU_POS, LoadIconW(inst, L"GL"))) return 1;
    ShowWindow(frame, show);
    UpdateWindow(frame);
    return fx::RunMessageLoop(frame, frame.mdiClient());
}
