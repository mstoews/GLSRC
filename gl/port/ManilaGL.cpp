//
// ManilaGL.cpp - see ManilaGL.h.
//
#include "ManilaGL.h"

#include "Company.h"
#include "ChartOfAccounts.h"
#include "MasterData.h"
#include "VoucherEntry.h"
#include "VoucherPosting.h"
#include "TransactionInquiry.h"
#include "Backup.h"
#include "YearEnd.h"
#include "CompanyProfile.h"
#include "Globals.h"
#include "Login.h"
#include "PortIds.h"
#include "reports/RunReport.h"
#include "stubs/Stubs.h"

#include "report/win32/Preview.h"
#include "report/win32/PrinterSetup.h"

#include <CommCtrl.h>
#pragma comment(lib, "comctl32.lib")   // SetWindowSubclass

#include <algorithm>
#include <cstring>
#include <iterator>
#include <string>

#include "LegacyIds.h"   // last: see LegacyIds.h

namespace {

constexpr int kWindowMenuPos = 7;            // TManilaGL: ChildMenuPos = 7
constexpr wchar_t kGleProp[] = L"GL.UseGle";

// Which stub each menu command outside Spec #1 goes to. The command IDs and
// handler names are TGlWindow's and TManilaGL's response tables
// (gl/header/gl_main.h, gl_man.h). A handler whose body is a PrintReport()
// call (gl/src/ap_repts.cpp) is Reporting, whichever module it belongs to;
// the other handlers in gl_man.h's "Accounts Payable" / "Accounts
// Receivable" sections are AP / AR. Everything else is General Ledger.
const WORD kApCommands[] = {
    1048,  // APPostBatch      ID_APPOSTBATCH
    1057,  // APCreateGLSlip   ID_GLSLIP
    1066,  // APPurgeAccts     ID_PURGEACCTS
    1079,  // APCash           ID_APCASH
    1084,  // APPostAdj        ID_APPOSTADJ
    1085,  // APCancelCash     ID_CASHCANCEL
    1088,  // APPostCash       ID_APPOSTCASH
    3064,  // APCancelBatch    ID_BATCHCANCEL
    4044,  // APBatch          ID_APBATCH
    4045,  // APAddVendors     ID_ADDVENDOR
    4046,  // APDelVendors     ID_DELVENDOR
    4047,  // APModVendors     ID_MODVENDOR
    6085,  // APPayment        ID_PAYMENT
    8074,  // APChequeAP       CM_CHEQUE_AP
    8079,  // AccountsPayable  CM_ACCOUNT_PAYABLE (an empty handler in gl.cpp)
};
const WORD kArCommands[] = {
    4049,  // ARPostBatch      ID_ARPOST
    4053,  // ARCancelBatch    ID_ARCANCEL
    4054,  // ARCreateSlip     ID_ARPAYMENT
    4059,  // ARAddVendors     ID_AR_CUSTOMER
    4062,  // ARPurgeAccts     ID_ARPURGE
    5045,  // ARBatch          ID_ARBATCH
    8080,  // AccountsReceive  CM_ACCOUNT_RECEIVABLE (an empty handler in gl.cpp)
};
const WORD kReportingCommands[] = {
    // General Ledger reports
    1015, 1016, 1018, 1025,   // Financial Statements popup (income, balance sheet, retained earnings, manufacturing)
    1019, 1020,               // GLFinFA, GLFinDep: fixed-asset listings
    1075,                     // GLPrtDistList
    // 2001 (CMDialogPrint, printer setup) is ported: S3-M1.
    3068,                     // GLCustomReport ID_PRINT (also the toolbar Print button), without a preview open
    6038, 6039, 6041,         // GLProductList, GLMedRepsList, GLDoctorRpt
    6074, 6075, 6076, 6077, 6078, 6080,   // slips, slip summary, trial balances, accounts, pre-posting
    8082, 8136,               // GLPrtSlipDetail, GLPrePosting
    10071, 10073,             // GLChartInquiry, GLBudgetInquiry
    // Accounts Payable reports
    1049, 1058, 1060, 1061, 1062, 1063, 1064, 1065, 1077, 1078, 1081, 1082, 1990,
    4091, 4092,
    8067, 8068, 8069, 8071, 8072, 8073, 8075, 8076, 8077, 8078,
    // Accounts Receivable reports
    4056, 4057, 4060, 4061, 4063, 4068, 4069, 4070, 4071, 4072, 4073, 4074, 4075, 4076, 8137,
};

template <size_t N>
bool contains(const WORD (&list)[N], WORD id)
{
    return std::find(std::begin(list), std::end(list), id) != std::end(list);
}

int findPopup(HMENU menu, const wchar_t* text)
{
    const int n = GetMenuItemCount(menu);
    for (int i = 0; i < n; ++i) {
        wchar_t buf[64] = {};
        GetMenuStringW(menu, static_cast<UINT>(i), buf, 64, MF_BYPOSITION);
        if (wcscmp(buf, text) == 0) return i;
    }
    return -1;
}

} // namespace

// ---- the top toolbar --------------------------------------------------------

WORD ManilaGL::TopToolbar::MapCommand(WORD id)
{
    // TToolBar (gl/header/gl_tool.h, gl/src/gl_toolb.cpp): button -> the command it sent.
    switch (id) {
    case ID_GLADD:      return ID_ACCTADD;
    case ID_GLBATCH:    return ID_MODBATCH;
    case ID_ARBATCHENT: return ID_ARBATCH;
    case ID_CASH:       return ID_APCASH;
    case ID_GLPOST:     return ID_GLPOSTBATCH;
    case ID_GLINQ:      return ID_GLCHARTINQ;
    case ID_CANCEL:     return fx::CM_EXIT;   // GLQuit: Mssg(24340)
    case ID_NEW:        return 111;
    default:            return id;            // ID_PRINT, ID_OPEN, ID_SAVE, ID_APBATCH, the edit buttons
    }
}

// ---- the frame ----------------------------------------------------------------

HWND ManilaGL::Create(HINSTANCE inst)
{
    // TGlWindow::GetClassName: "GLWindow"; GetWindowClass: icon "GL".
    return CreateFrame(inst, L"GLWindow", szApp, HGLMenu, kWindowMenuPos, LoadIconW(inst, L"GL"));
}

bool ManilaGL::OnCreate(LPCREATESTRUCTW cs)
{
    MdiFrame::OnCreate(cs);

    // TGlWindow::SetupWindow: the toolbar (1032) and the module bar (2057).
    toolbar_.CreateBar(hInst, 1032, hwnd());
    toolbar_.SetBackground(L"STATUSBAR");
    toolbar_.setHeight(TOOLBAR_HEIGHT);
    toolbar_.SetLayout(10, 30, 25);
    double pos = 0;   // TToolBar::SetupWindow
    toolbar_.PlaceButton(ID_NEW, pos, true);        pos += 1;
    toolbar_.PlaceButton(ID_OPEN, pos, true);       pos += 1;
    toolbar_.PlaceButton(ID_SAVE, pos, true);       pos += 1;
    toolbar_.PlaceButton(ID_PRINT, pos, true);      pos += 1.3;
    toolbar_.PlaceButton(ID_GLBATCH, pos, true);    pos += 1;
    toolbar_.PlaceButton(ID_APBATCH, pos, true);    pos += 1;
    toolbar_.PlaceButton(ID_CASH, pos, true);       pos += 1;
    toolbar_.PlaceButton(ID_ARBATCHENT, pos, true); pos += 1.3;
    toolbar_.PlaceButton(ID_GLINQ, pos, true);      pos += 1;
    toolbar_.PlaceButton(ID_GLPOST, pos, true);     pos += 1;
    toolbar_.PlaceButton(ID_GLADD, pos, true);      pos += 1.3;
    toolbar_.PlaceButton(ID_CANCEL, pos, true);
    pos = 4.3;   // the spreadsheet buttons, hidden until a sheet is active (not in Spec #1)
    for (const int id : {ID_CUT, ID_COPY, ID_PASTE}) {
        toolbar_.PlaceButton(id, pos, false);
        pos += 1;
    }
    pos += 0.3;
    for (const int id : {ID_BORDER, ID_FONT, ID_ALIGNMENT, ID_NUMBER, ID_GRIDS}) {
        toolbar_.PlaceButton(id, pos, false);
        pos += 1;
    }
    Dock(toolbar_, fx::DockSide::Top, toolbar_.height());

    // TVToolBar: SetPic(ID_GENERAL / 2210 / ID_RECEIVABLE) finds no buttons in
    // gl_new.rc's 2057 (spec 6.2), so only the status fields are left.
    vbar_.CreateBar(hInst, 2057, hwnd());
    vbar_.SetBackground(L"STATUSBAR");
    vbar_.setHeight(TOOLBAR_HEIGHT);
    Dock(vbar_, fx::DockSide::Bottom, vbar_.height());

    // TGLClient::WMRButtonDown: a right-click on the client opens GLSUB.
    SetWindowSubclass(mdiClient(), ClientProc, 1, reinterpret_cast<DWORD_PTR>(this));

    // TManilaGL::SetupWindow. ID_GL isn't in gl_new.rc's MANILA; kept for parity.
    CheckMenuItem(HGLMenu, ID_GL, MF_BYCOMMAND | MF_CHECKED);

    // TGlWindow::SetupWindow: the login, if GL.INI asks for one.
    if (LoginRequired()) {
        bMenuSetup = FALSE;
        PostMessageW(hwnd(), WM_COMMAND, ID_SYSACCESS, 0);
    } else {
        strcpy_s(szUser, "NOLOGIN");   // fits; the original wrote strcmp(szUser, "NOLOGIN"), a no-op
        bMenuSetup = TRUE;
    }
    RefreshStatus();
    return true;
}

bool ManilaGL::OnSize(UINT type, int cx, int cy)
{
    MdiFrame::OnSize(type, cx, cy);
    if (!vbar_.hwnd()) return true;
    // TGlWindow::WMSize: company on the left (8 x 25 px), period on the right
    // (100 px), menu help between them.
    constexpr int kHeight = 25, kSpace = 2;
    vbar_.PlaceField(ID_STATUSHELP, kSpace, 8 * kHeight);
    vbar_.PlaceField(ID_PERIODFIELD, cx - 102, 100);
    const int left = kSpace + 8 * kHeight + 1;
    vbar_.PlaceField(ID_MENUHELP, left, (std::max)(0, (cx - 102) - left - 10));
    // The original also reread the company and period here, on every resize;
    // RefreshStatus() runs after the screens that change them instead.
    return true;
}

void ManilaGL::RefreshStatus()
{
    vbar_.SetText(ID_STATUSHELP, GetCompany().c_str());       // TGlWindow::GLCreateStatusBar
    vbar_.SetText(ID_PERIODFIELD, GetCurrentPeriod().c_str());
}

bool ManilaGL::OnCommand(WORD id, WORD code, HWND ctl)
{
    if (MdiFrame::OnCommand(id, code, ctl)) return true;   // Window menu, CM_EXIT
    switch (id) {
    case ID_SYSACCESS:                     // TManilaGL::GLSysAccess (posted at startup)
        if (RunLogin(hwnd()) != IDOK) PostMessageW(hwnd(), WM_COMMAND, fx::CM_EXIT, 0);
        return true;
    case ID_ACCESS:                        // Maintenance > User Access
        RunUserAdministration(hwnd());
        return true;
    case ID_COMPANYPROFILE:                // TGlWindow::GLCompanySet
        RunCompanyProfile(hwnd());
        RefreshStatus();                   // the original updated a status bar it never created
        return true;
    case ID_ACCTADD:                       // TGlWindow::GLAcctAdd: Accounts > Chart of Accounts, GLSUB
        RunChartOfAccounts(hwnd());
        return true;
    case ID_ACCTDEL:                       // TGlWindow::GLAcctDel: GLSUB > Delete Accounts
        RunDeleteAccounts(hwnd());
        return true;
    case ID_DEPTCODES:                     // TGlWindow::GLDeptCodes: Accounts > Department Codes, GLSUB
        RunDepartmentCodes(hwnd());
        return true;
    case ID_MEDREPS:                       // TManilaGL::GLMedicalReps
        RunMedicalReps(hwnd());
        return true;
    case ID_DOCTOR:                        // TManilaGL::GLDoctor
        RunDoctorCodes(hwnd());
        return true;
    case ID_PRODCODE:                      // TManilaGL::GLProductCode
        RunProductCodes(hwnd());
        return true;
    case ID_MODBATCH:                      // TGlWindow::GLModifyBatch: Transactions > GL Voucher, GLSUB, toolbar
    case ID_BATCHENT:                      // TGlWindow::GLBatchEnt (the same dialog)
        RunVoucherEntry(hwnd());
        return true;
    case CM_PRIOR:                         // TGlWindow::GLPrior: Transactions > Prior Period Entry
        RunPriorPeriodEntry(hwnd());
        return true;
    case ID_GLTRANSINQ:                    // TGlWindow::GLTransInquiry: General Ledger Voucher Inquiry
        RunTransactionInquiry(hwnd());
        return true;
    case ID_ACCTBUDGET:                    // TGlWindow::GLAcctBudget: Budget Data
        RunBudgetData(hwnd());
        return true;
    case ID_YEAREND:                       // TGlWindow::GLYearEnd: Year End Consolidation
        RunYearEnd(hwnd());
        return true;
    case ID_BACKUP:                        // TGlWindow::GLBackup: Backup Databases
        RunBackup(hwnd());
        return true;
    case ID_RESTORE:                       // TGlWindow::GLRestore: Restore Databases
        if (RunRestore(hwnd())) RefreshStatus();   // the restored company's name and period
        return true;
    case ID_REINDEX:                       // TGlWindow::GLReindex: Reindex Databases
        RunReindex(hwnd());
        return true;
    case ID_GLPOSTBATCH:                   // TGlWindow::GLPostBatch: Post Voucher > General Ledger, GLSUB, toolbar
        RunPostVouchers(hwnd());
        return true;
    case ID_GLREPBATCH:                    // TGlWindow::GLRepBatch: Recover General Ledger Voucher
        RunRecoverVoucher(hwnd());
        return true;
    case ID_BATCHDELETE:                   // TGlWindow::GLDeleteBatch: Cancel Voucher > General Ledger
        RunCancelVoucher(hwnd());
        return true;
    case ID_ABOUT: {                       // TGlWindow::CMAbout
        fx::Dialog about;
        about.DoModal(hInst, 1008, hwnd());
        return true;
    }
    case ID_GL:                            // the toolbar's GL button (not in gl_new.rc's 1032)
        return true;
    case CM_DIALOGPRINT:                   // TGlWindow::CMDialogPrint: File > Printer Setup (spec #3 D10)
        rpt::win32::ShowPrinterSetup(hwnd());
        return true;
    case ID_PRINT:                         // File > Print, the toolbar Print button (spec #3 D9)
        if (rpt::win32::PrintActivePreview(*this)) return true;
        break;                             // no preview open: the report list comes in S3-M7
    case CM_SAMPLE_REPORT:                 // the report engine's test report (gl.exe /samplereport)
        RunSampleReport(*this);
        return true;
    default:
        break;
    }
    if (ctl || id >= 0xFF00) return false;   // child-window IDs: DefFrameProc
    NotPorted(id);
    return true;
}

std::wstring ManilaGL::MenuText(WORD id) const
{
    wchar_t buf[128] = {};
    HMENU menu = GetMenu(hwnd());
    if (!menu || GetMenuStringW(menu, id, buf, 128, MF_BYCOMMAND) <= 0) return {};
    std::wstring text;
    for (const wchar_t* p = buf; *p; ++p)
        if (*p != L'&') text += *p;
    while (!text.empty() && (text.back() == L'.' || text.back() == L' ')) text.pop_back();
    return text;
}

void ManilaGL::NotPorted(WORD id)
{
    const std::wstring item = MenuText(id);
    if (contains(kReportingCommands, id)) {
        ReportingStubs::Show(hwnd(), item.c_str());
    } else if (contains(kApCommands, id)) {
        ApStubs::Show(hwnd(), item.c_str());
    } else if (contains(kArCommands, id)) {
        ArStubs::Show(hwnd(), item.c_str());
    } else {
        std::wstring text = item.empty() ? fx::Format(L"Command %u", id) : item;
        text += L"\n\nThis General Ledger function is not part of Spec #1.";
        MessageBoxW(hwnd(), text.c_str(), szApp, MB_OK | MB_ICONINFORMATION);
    }
}

bool ManilaGL::OnMessage(UINT msg, WPARAM wp, LPARAM lp, LRESULT&)
{
    switch (msg) {
    case WM_INITMENUPOPUP:   // TManilaGL::WMInitMenuPopup
        ApplyMenuAccess(GetMenu(hwnd()));
        return false;        // DefFrameProc too
    case WM_MENUSELECT:
        MenuHelp(wp, lp);
        return false;
    default:
        return false;
    }
}

void ManilaGL::MenuHelp(WPARAM wp, LPARAM lp)
{
    // TGlWindow::WMMenuSelect. Win16 packed the flags and menu into lParam;
    // Win32 sends the item and flags in wParam and the menu in lParam.
    const UINT item = LOWORD(wp);
    const UINT flags = HIWORD(wp);
    UINT str = item;
    if (flags == 0xFFFF && lp == 0)
        str = IDS_READY;        // "Ready..."
    else if (flags & MF_POPUP)
        str = IDS_MENUITEM;     // "Select a menu item from the menu list..."
    else if (flags & MF_SYSMENU)
        str = IDS_SYSTEM;       // "System menu..."
    vbar_.SetText(ID_MENUHELP, fx::LoadResString(hInst, str).c_str());
}

void ManilaGL::PopUpMenu(POINT screen)
{
    // TGlWindow::PopUpMenu; HGLSubMenu is loaded once in wWinMain.
    if (HMENU popup = GetSubMenu(HGLSubMenu, 0))
        TrackPopupMenu(popup, TPM_LEFTBUTTON, screen.x, screen.y, 0, hwnd(), nullptr);
}

LRESULT CALLBACK ManilaGL::ClientProc(HWND h, UINT msg, WPARAM wp, LPARAM lp, UINT_PTR id, DWORD_PTR ref)
{
    if (msg == WM_RBUTTONDOWN) {
        POINT pt{static_cast<short>(LOWORD(lp)), static_cast<short>(HIWORD(lp))};
        ClientToScreen(h, &pt);
        reinterpret_cast<ManilaGL*>(ref)->PopUpMenu(pt);
        return 0;
    }
    if (msg == WM_NCDESTROY) RemoveWindowSubclass(h, ClientProc, id);
    return DefSubclassProc(h, msg, wp, lp);
}

bool ManilaGL::OnDestroy()
{
    // Detach the menu so the frame doesn't destroy it: wWinMain owns
    // MANILA, GLE and GLSUB and destroys all three (~TAcctModule).
    SetMenu(hwnd(), nullptr);
    return MdiFrame::OnDestroy();
}

void ManilaGL::UseGleMenu(HWND child)
{
    SetPropW(child, kGleProp, reinterpret_cast<HANDLE>(1));
}

void ManilaGL::ReleaseGleMenu(HWND child)
{
    RemovePropW(child, kGleProp);
}

void ManilaGL::OnMdiChildActivate(HWND child)
{
    // The swap gl_xls.cpp's TWorksheet::WMMDIActivate made: GLE while a
    // child that asked for it is active, MANILA otherwise. The original
    // passed submenu 0 (File) as the Window menu; this passes each menu's
    // Window popup, so the child list stays in the Window menu.
    const bool gle = child && GetPropW(child, kGleProp);
    HMENU menu = gle ? HGLExMenu : HGLMenu;
    if (!menu || GetMenu(hwnd()) == menu) return;
    const int pos = gle ? findPopup(menu, L"&Window") : kWindowMenuPos;
    SendMessageW(mdiClient(), WM_MDISETMENU, reinterpret_cast<WPARAM>(menu),
                 reinterpret_cast<LPARAM>(pos >= 0 ? GetSubMenu(menu, pos) : nullptr));
    DrawMenuBar(hwnd());
}
