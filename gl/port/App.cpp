//
// App.cpp - gl.exe entry point and the original module globals (spec 2.4,
// 3.7). Port of WinMain / TAcctModule in gl/src/gl.cpp (plan M5 task 1).
//
// Startup, in order:
//   1. Register the Toolbox controls: every dialog template needs them.
//   2. /checkres: the resource check (plan M4; ResourceCheck.cpp), then exit.
//   3. One instance only: a second gl.exe brings the first to the front
//      (the original's hPrevInstance check).
//   4. GLDR.INI [GL_DIR] DIR -> the company database; refuse a network path
//      (Ini.cpp). Open it as the global cb.
//   5. Load MANILA, GLE and GLSUB.
//   6. Create the ManilaGL frame. It shows the login when GL.INI [ACCESS]
//      PSW=T (TGlWindow::SetupWindow posted ID_SYSACCESS the same way).
//
//   gl.exe /checkres [/quiet]   loads every Spec #1 resource and reports
//   gl.exe /stress [/quiet]     opens and closes the Chart of Accounts 100
//                               times, each master-data screen 25 times and
//                               Voucher Entry 25 times, on the company
//                               database (plan risk P6); the exit code is the
//                               GDI + USER object growth
//
// Debug builds report CRT heap leaks at exit (_CrtDumpMemoryLeaks, to the
// debugger's output window).
//
// The GL accelerator table in gl.rc is not loaded: the original never loaded
// it (it maps Tab to Print).
//
#include <Windows.h>

#include "win32fx.h"

#include "ChartOfAccounts.h"
#include "MasterData.h"
#include "VoucherEntry.h"
#include "Globals.h"
#include "Ini.h"
#include "ManilaGL.h"
#include "ResourceCheck.h"

#include "cb4sqlite.h"

#include <string>

#ifdef _DEBUG
#include <crtdbg.h>
#endif

#include "LegacyIds.h"   // last: see LegacyIds.h (declares szMsg)

// The original globals (gl/src/gl.cpp). Login.cpp defines gl_acces.cpp's.
HINSTANCE hInst = nullptr;
HMENU HGLMenu = nullptr;
HMENU HGLExMenu = nullptr;
HMENU HGLSubMenu = nullptr;
Code4 cb;
WCHAR szApp[20] = L"TIAS";
WCHAR CurDir[MAX_PATH] = L"";
char szMsg[256] = "";

namespace {

bool activatePrevious()
{
    HWND prev = FindWindowW(L"GLWindow", nullptr);
    if (!prev) return false;
    prev = GetLastActivePopup(prev);
    ShowWindow(prev, SW_SHOWNORMAL);
    SetForegroundWindow(prev);
    return true;
}

int fail(const std::wstring& text)
{
    MessageBoxW(nullptr, text.c_str(), szApp, MB_OK | MB_ICONSTOP);
    return 1;
}

} // namespace

int WINAPI wWinMain(_In_ HINSTANCE inst, _In_opt_ HINSTANCE, _In_ LPWSTR cmdLine, _In_ int show)
{
    hInst = inst;
#ifdef _DEBUG
    _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
    if (!fx::RegisterCustomControls(inst)) return fail(L"The custom controls could not be registered.");
    const std::wstring args = cmdLine ? cmdLine : L"";
    if (args.find(L"/checkres") != std::wstring::npos)
        return RunResourceCheck(inst, args.find(L"/quiet") != std::wstring::npos);
    if (activatePrevious()) return 0;

    const CompanyDbLocation db = FindCompanyDb();
    if (!db.error.empty()) return fail(db.error);
    if (db.path.size() >= MAX_PATH) return fail(L"The company database path is too long:\n" + db.path);
    CurDir[db.path.copy(CurDir, MAX_PATH - 1)] = L'\0';
    if (cb.open(std::wstring_view(db.path)) != r4success)
        return fail(L"The company database could not be opened:\n" + db.path);

    if (args.find(L"/stress") != std::wstring::npos) {
        const int accounts = StressChartOfAccounts(100);
        const int master = StressMasterData(25);
        const int vouchers = StressVoucherEntry(25);
        const int growth = (accounts < 0 || master < 0 || vouchers < 0) ? -1 : accounts + master + vouchers;
        cb.close();
        if (args.find(L"/quiet") == std::wstring::npos)
            MessageBoxW(nullptr,
                        fx::Format(L"Chart of Accounts opened and closed 100 times, and Department, Medical Rep, "
                                   L"Doctor and Product Codes and Voucher Entry 25 times each.\nGDI + USER objects: %+d.",
                                   growth).c_str(),
                        szApp, MB_OK | (growth == 0 ? MB_ICONINFORMATION : MB_ICONEXCLAMATION));
        return growth;
    }

    // ~TAcctModule destroyed these; the frame detaches MANILA before it goes.
    fx::MenuGuard manila(LoadMenuW(inst, L"MANILA"));
    fx::MenuGuard gle(LoadMenuW(inst, L"GLE"));
    fx::MenuGuard glsub(LoadMenuW(inst, L"GLSUB"));
    if (!manila || !gle || !glsub) return fail(L"The menus could not be loaded.");
    HGLMenu = manila.get();
    HGLExMenu = gle.get();
    HGLSubMenu = glsub.get();

    int rc = 1;
    {
        ManilaGL frame;
        if (frame.Create(inst)) {
            ShowWindow(frame, show);
            UpdateWindow(frame);
            rc = fx::RunMessageLoop(frame, frame.mdiClient());
        }
    }
    cb.close();
    return rc;
}
