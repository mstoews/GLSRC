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
//                               times (Add, Delete and Budget Data in turn),
//                               each master-data screen 25 times, and Voucher
//                               Entry, Post, Recover, Cancel and Transaction
//                               Inquiry 25 times each, and the sample report's
//                               preview 25 times (paging at every zoom), on the
//                               company database (plan risk P6); the exit code
//                               is the GDI + USER object growth
//   gl.exe /samplereport        opens the report engine's test report after
//                               startup (spec #3 S3-M1; reports/SampleReport.h)
//   gl.exe /failpost <n>        fault injection (spec #2 8 #6): posting throws
//                               after writing its nth line, so the check can
//                               see that nothing was written (Vouchers.h)
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
#include "FixedAssets.h"
#include "MasterData.h"
#include "VoucherEntry.h"
#include "VoucherPosting.h"
#include "TransactionInquiry.h"
#include "Vouchers.h"   // SetPostFault
#include "Globals.h"
#include "Ini.h"
#include "ManilaGL.h"
#include "PortIds.h"
#include "ResourceCheck.h"
#include "reports/RunReport.h"

#include "cb4sqlite.h"

#include <algorithm>
#include <cwchar>
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

// One /stress step's GDI + USER growth (-1: could not run). Growth is
// measured a second time and the smaller reading kept: now and then Windows
// creates one USER object of its own in the process, at a moment of its
// choosing (seen 0.4 to 4 s in, in whichever step was running, once per
// process, never a GDI object and never more than +1; 2026-11-11), which
// no warm-up can absorb. A leak in the step shows in both readings; that
// object can appear in only one.
template <typename Step>
int stressStep(Step step)
{
    const int first = step();
    if (first <= 0) return first;
    const int second = step();
    return second < 0 ? second : (std::min)(first, second);
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

    if (const size_t at = args.find(L"/failpost"); at != std::wstring::npos)
        SetPostFault(static_cast<int>(std::wcstol(args.c_str() + at + 9, nullptr, 10)));   // "/failpost <n>"; 0: off

    if (args.find(L"/stress") != std::wstring::npos) {
        const int accounts = stressStep([] { return StressChartOfAccounts(100); });
        const int master = stressStep([] { return StressMasterData(25); });
        const int vouchers = stressStep([] { return StressVoucherEntry(25); });
        const int posting = stressStep([] { return StressVoucherPosting(25); });
        const int inquiry = stressStep([] { return StressTransactionInquiry(25); });
        const int reports = stressStep([] { return StressReports(25); });
        const int assets = stressStep([] { return StressFixedAssets(25); });
        const int growth =
            (accounts < 0 || master < 0 || vouchers < 0 || posting < 0 || inquiry < 0 || reports < 0 || assets < 0)
                ? -1
                : accounts + master + vouchers + posting + inquiry + reports + assets;
        cb.close();
        if (args.find(L"/quiet") == std::wstring::npos)
            MessageBoxW(nullptr,
                        fx::Format(L"Chart of Accounts opened and closed 100 times, and Department, Medical Rep, "
                                   L"Doctor and Product Codes, Voucher Entry, Post, Recover, Cancel, Transaction "
                                   L"Inquiry, every report's preview and the Fixed Assets screens 25 times each.\n"
                                   L"GDI + USER objects: %+d.\n\n"
                                   L"Per step (-1: could not run): Chart of Accounts %+d, master data %+d, "
                                   L"voucher entry %+d, posting %+d, inquiry %+d, reports %+d, fixed assets %+d.",
                                   growth, accounts, master, vouchers, posting, inquiry, reports, assets).c_str(),
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
            // After the login the frame posted at creation, if there is one.
            if (args.find(L"/samplereport") != std::wstring::npos)
                PostMessageW(frame, WM_COMMAND, CM_SAMPLE_REPORT, 0);
            rc = fx::RunMessageLoop(frame, frame.mdiClient());
        }
    }
    cb.close();
    return rc;
}
