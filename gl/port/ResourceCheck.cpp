//
// ResourceCheck.cpp - see ResourceCheck.h.
//
//   gl.exe /checkres           report in a message box
//   gl.exe /checkres /quiet    exit code = failures
//                              (from cmd: start /wait gl.exe /checkres /quiet
//                               then echo %errorlevel%)
//
#include "ResourceCheck.h"

#include "PortIds.h"

#include "win32fx.h"

#include <cwchar>
#include <string>
#include <vector>

namespace {

struct Report {
    int failures = 0;
    std::wstring log;
    void check(bool ok, const std::wstring& what)
    {
        log += ok ? L"ok      " : L"FAILED  ";
        log += what;
        log += L"\n";
        if (!ok) ++failures;
    }
};

INT_PTR CALLBACK ProbeProc(HWND, UINT, WPARAM, LPARAM) { return FALSE; }

struct DialogCase {
    int id;
    const wchar_t* what;
    bool hasGrid;   // contains a tbSpread grid (fx::Grid, Spec #2 S2-M0): checked to hold its ListView
};

// Spec 6.2 as corrected in M4. The three named in the M4 exit criteria come first.
const DialogCase kDialogs[] = {
    {1010, L"Chart of Accounts (TGLAcctsAdd / TGLAcctsDelete)", false},
    {1158, L"Login (TGLAccess)", false},
    {1021, L"Company Profile (TGLProfile)", false},
    {1008, L"About", false},
    {1028, L"Set Period Dates (TGLSetPeriodDates)", false},
    {1032, L"Toolbar (TToolBar)", false},
    {1033, L"Account Setup (TGLSetAccounts)", false},
    {1034, L"Cash Accounts (TAPCashAccts)", false},
    {1035, L"List Window Parameters (TSetQuery)", false},
    {2057, L"Module bar (TVToolBar)", false},
    {1159, L"User Access Update (TGLAdministration)", false},
    {1011, L"Cash Accounts list (TAPCashAcctsList)", true},
    {1160, L"Menu Access (TAccessMenus)", true},
    // Spec #2 S2-M2.
    {1023, L"Department Codes (TGLDeptCodes)", false},
    {1006, L"Medical Reps (TGLMedReps)", false},
    {1040, L"Doctor Codes (TGLDrCodes)", false},
    {1007, L"Product Codes (TGLProd)", false},
    // Spec #2 S2-M3.
    {1013, L"Voucher Entry (TGLBatchEntry, TGLPrior)", false},
    // Spec #2 S2-M4.
    {1061, L"Post General Ledger Voucher (TGLPostBatch)", true},
    {1015, L"Recover General Ledger Voucher (TGLRecoverBatch)", true},
    {1043, L"Cancel General Ledger Voucher (TGLCancelBatch)", true},
    // Spec #2 S2-M5.
    {1029, L"Transaction Inquiry (TGLTransInq)", true},
    {1017, L"Budget Data (TGLBudget)", true},
    // Spec #3 S3-M7.
    {1022, L"Report list (TGLCustomReport, D9)", true},
    // Spec #4 S4-M1.
    {4002, L"Asset Types (TGLTypes)", false},
    // Spec #4 S4-M2.
    {4004, L"Fixed Assets Add/Modify (TGLFixedAssets)", false},
    // Spec #4 S4-M3.
    {4003, L"Create Voucher (TGLCreateDeprSlip)", false},
    // Spec #4 S4-M4.
    {1001, L"Disposal (TGLFixedDelete)", false},
    // The port's own (gl/res/port.rc, spec #3 3).
    {IDD_SAMPLE_OPTIONS, L"Sample Report options (port.rc, S3-M1)", false},
    {IDD_TB_OPTIONS, L"Trial balance options (port.rc, S3-M2)", false},
    {IDD_STMT_OPTIONS, L"Financial statement options (port.rc, S3-M3)", false},
};

void checkDialogs(HINSTANCE inst, Report& r)
{
    // A hidden owner/parent: 1032 and 2057 are WS_CHILD templates.
    HWND parent = CreateWindowExW(0, L"STATIC", L"resource check", WS_POPUP, 0, 0, 400, 300, nullptr,
                                  nullptr, inst, nullptr);
    for (const DialogCase& d : kDialogs) {
        const std::wstring name = fx::Format(L"dialog %d, %s", d.id, d.what);
        const bool present = FindResourceW(inst, MAKEINTRESOURCEW(d.id), MAKEINTRESOURCEW(5)) != nullptr;  // RT_DIALOG
        if (!present) {
            r.check(false, name + L": template missing");
            continue;
        }
        HWND dlg = CreateDialogParamW(inst, MAKEINTRESOURCEW(d.id), parent, ProbeProc, 0);
        r.check(dlg != nullptr, name);
        if (dlg && d.hasGrid) r.check(fx::Grid(GetDlgItem(dlg, 1075)).IsGrid(), name + L": tbSpread grid (ID_DISTLIST)");
        if (dlg) DestroyWindow(dlg);
    }
    if (parent) DestroyWindow(parent);
}

void checkMenus(HINSTANCE inst, Report& r)
{
    for (const wchar_t* name : {L"MANILA", L"GLE", L"GLSUB"}) {
        fx::MenuGuard menu(LoadMenuW(inst, name));
        r.check(static_cast<bool>(menu), fx::Format(L"menu %s", name));
        if (menu && wcscmp(name, L"MANILA") == 0) {
            wchar_t text[32] = {};
            GetMenuStringW(menu.get(), 7, text, 32, MF_BYPOSITION);
            r.check(std::wstring(text) == L"&Window", L"MANILA: Window menu at position 7");
        }
        // Spec #3 D9: Custom Report has its own command; File > Print keeps 3068.
        if (menu && wcscmp(name, L"GLE") != 0) {
            wchar_t text[32] = {};
            GetMenuStringW(menu.get(), CM_REPORT_LIST, text, 32, MF_BYCOMMAND);
            r.check(std::wstring(text) == L"&Custom Report...", fx::Format(L"%s: Custom Report is CM_REPORT_LIST", name));
        }
    }
}

BOOL CALLBACK collectName(HMODULE, LPCWSTR, LPWSTR name, LONG_PTR param)
{
    auto* names = reinterpret_cast<std::vector<std::wstring>*>(param);
    names->push_back(IS_INTRESOURCE(name) ? fx::Format(L"#%u", static_cast<unsigned>(reinterpret_cast<ULONG_PTR>(name)))
                                          : std::wstring(name));
    return TRUE;
}

LPCWSTR resName(const std::wstring& n)
{
    return n[0] == L'#' ? MAKEINTRESOURCEW(std::stoi(n.substr(1))) : n.c_str();
}

void checkImages(HINSTANCE inst, Report& r)
{
    std::vector<std::wstring> bitmaps;
    EnumResourceNamesW(inst, MAKEINTRESOURCEW(2), collectName, reinterpret_cast<LONG_PTR>(&bitmaps));  // RT_BITMAP
    int loaded = 0;
    for (const auto& n : bitmaps) {
        fx::BitmapGuard bmp(static_cast<HBITMAP>(LoadImageW(inst, resName(n), IMAGE_BITMAP, 0, 0, 0)));
        if (bmp) {
            ++loaded;
        } else {
            r.check(false, L"bitmap " + n);
        }
    }
    r.check(bitmaps.size() == 83 && loaded == 83, fx::Format(L"%d of %d bitmaps load (83 expected)", loaded,
                                                              static_cast<int>(bitmaps.size())));
    for (const wchar_t* n : {L"STATUSBAR", L"PLUS", L"MINUS"}) {
        fx::BitmapGuard bmp(static_cast<HBITMAP>(LoadImageW(inst, n, IMAGE_BITMAP, 0, 0, 0)));
        r.check(static_cast<bool>(bmp), fx::Format(L"bitmap %s", n));
    }
    for (const wchar_t* n : {L"GL", L"SHEET"}) {
        r.check(LoadIconW(inst, n) != nullptr, fx::Format(L"icon %s", n));
    }
    r.check(LoadIconW(inst, MAKEINTRESOURCEW(1111)) != nullptr, L"icon 1111");
    r.check(LoadAcceleratorsW(inst, L"GL") != nullptr, L"accelerators GL");
}

void checkStrings(HINSTANCE inst, Report& r)
{
    r.check(fx::LoadResString(inst, 4024) == L"MM/DD/YY", L"string 4024 = MM/DD/YY (tbDate display format)");
    r.check(fx::LoadResString(inst, 1021) == L"Are you sure you wish to delete this account?", L"string 1021");
    r.check(!fx::LoadResString(inst, 8095).empty(), L"string 8095");
}

} // namespace

int RunResourceCheck(HINSTANCE inst, bool quiet)
{
    Report r;
    checkDialogs(inst, r);
    checkMenus(inst, r);
    checkImages(inst, r);
    checkStrings(inst, r);
    if (!quiet) {
        r.log += r.failures ? fx::Format(L"\n%d check(s) FAILED", r.failures) : std::wstring(L"\nAll checks passed");
        MessageBoxW(nullptr, r.log.c_str(), L"gl.exe resource check", r.failures ? MB_ICONERROR : MB_ICONINFORMATION);
    }
    return r.failures;
}
