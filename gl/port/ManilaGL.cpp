//
// ManilaGL.cpp - see ManilaGL.h.
//
#include "ManilaGL.h"

#include "ApInvoiceEntry.h"
#include "ApPayments.h"
#include "ApPosting.h"
#include "ApTransfer.h"
#include "ApVendors.h"
#include "ArCustomers.h"
#include "ArInvoiceEntry.h"
#include "ArPosting.h"
#include "ArReceipts.h"
#include "Company.h"
#include "ChartOfAccounts.h"
#include "FixedAssetTypes.h"
#include "FixedAssets.h"
#include "DepreciationRun.h"
#include "FixedAssetDisposal.h"
#include "MasterData.h"
#include "VoucherBrowser.h"
#include "VoucherEntry.h"
#include "VoucherPosting.h"
#include "TransactionInquiry.h"
#include "Backup.h"
#include "YearEnd.h"
#include "CompanyProfile.h"
#include "Globals.h"
#include "Login.h"
#include "PortIds.h"
#include "reports/ReportList.h"
#include "reports/RunReport.h"
#include "reports/StatementLayoutDialog.h"
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
    1084,  // APPostAdj        ID_APPOSTADJ
    4046,  // APDelVendors     ID_DELVENDOR
    4047,  // APModVendors     ID_MODVENDOR
    8079,  // AccountsPayable  CM_ACCOUNT_PAYABLE (an empty handler in gl.cpp)
};
const WORD kArCommands[] = {
    8080,  // AccountsReceive  CM_ACCOUNT_RECEIVABLE (an empty handler in gl.cpp)
};
// TManilaGL::GLPrePosting; no legacy header names it.
constexpr WORD kPrePostingReport = 8136;
// TGlWindow::GLFinFA / GLFinDep (gl_xls.cpp): ID_FINFA / ID_FINDEP have no value in the headers.
constexpr WORD kFixedAssetListing = 1019;
constexpr WORD kFixedAssetTypes = 1020;

// The reports Spec #3 doesn't port (spec #3 1.2): they go with their modules.
// Every General Ledger report is ported (S3-M1 to S3-M7); 1025 left the menu
// (D13), and 6077 (GLAccounts), on no menu or toolbar, isn't ported (D11).
// The fixed-asset reports 1019 and 1020 are ported with Spec #4 (S4-M5), the
// AP reports with Spec #5 (S5-M7, RunApReport; 1082 and 1990, the adjustment
// reports, aren't: there are no adjustments, spec #5 7).
// The AR reports with Spec #6 (S6-M6, RunArReport); 8137 (ar_cust.rpt) existed
// nowhere and nothing called it: not ported (spec #6 7).
const WORD kArReports[] = {
    8137,
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

// ---- the top toolbar --------------------------------------------------------

WORD toolbarCommand(WORD id)
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
    default:            return id;            // ID_PRINT, ID_OPEN, ID_SAVE, ID_APBATCH
    }
}

// TToolBar::SetupWindow's buttons, in its order; `gap` where it left a wider
// space (pos += 1.3), now a separator. The spreadsheet buttons it placed hidden
// (cut, copy, paste, border, font, alignment, number, grids) are gone: no
// spreadsheet window was ever ported to show them.
struct ToolButton {
    WORD id;   // the original button ID: its bitmap, and so its icon P<id>
    UINT tip;
    bool gap;
};
const ToolButton kToolButtons[] = {
    {ID_NEW, IDS_TIP_NEW, false},         {ID_OPEN, IDS_TIP_OPEN, false},
    {ID_SAVE, IDS_TIP_SAVE, false},       {ID_PRINT, IDS_TIP_PRINT, true},
    {ID_GLBATCH, IDS_TIP_GLBATCH, false}, {ID_APBATCH, IDS_TIP_APBATCH, false},
    {ID_CASH, IDS_TIP_CASH, false},       {ID_ARBATCHENT, IDS_TIP_ARBATCH, true},
    {ID_GLINQ, IDS_TIP_GLINQ, false},     {ID_GLPOST, IDS_TIP_GLPOST, false},
    {ID_GLADD, IDS_TIP_GLADD, true},      {ID_CANCEL, IDS_TIP_EXIT, false},
};

} // namespace

// ---- the frame ----------------------------------------------------------------

HWND ManilaGL::Create(HINSTANCE inst)
{
    // TGlWindow::GetClassName: "GLWindow"; GetWindowClass: icon "GL".
    return CreateFrame(inst, L"GLWindow", szApp, HGLMenu, kWindowMenuPos, LoadIconW(inst, L"GL"));
}

bool ManilaGL::OnCreate(LPCREATESTRUCTW cs)
{
    MdiFrame::OnCreate(cs);

    // TGlWindow::SetupWindow: the toolbar (1032) and the module bar (2057),
    // now the standard controls (UI refresh spec 4.3, 4.4). The status bar is
    // docked first: it sits at the very bottom.
    status_.Create(hwnd(), IDC_FRAME_STATUS, hInst);
    Dock(status_, fx::DockSide::Bottom, 0);
    toolbar_.Create(hwnd(), IDC_FRAME_TOOLBAR, hInst);
    for (const ToolButton& b : kToolButtons) {
        toolbar_.AddButton(b.id, toolbarCommand(b.id), fx::LoadResString(hInst, b.tip));
        if (b.gap) toolbar_.AddSeparator();
    }
    Dock(toolbar_, fx::DockSide::Top, toolbar_.Height());

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

void ManilaGL::OnDpiChanged(UINT)
{
    // UI refresh spec 5.2: the toolbar is made again at the new icon size (a
    // new window, so it is docked again); the status bar gets the new font. The
    // frame then takes its new size, and OnSize sets the status parts.
    Undock(toolbar_);
    if (toolbar_.Rebuild()) Dock(toolbar_, fx::DockSide::Top, toolbar_.Height());
    status_.UpdateFont();
}

bool ManilaGL::OnSize(UINT type, int cx, int cy)
{
    MdiFrame::OnSize(type, cx, cy);
    if (!status_.hwnd()) return true;
    // TGlWindow::WMSize's layout: company on the left (200 px at 96 DPI),
    // period on the right (100 px), menu help between them.
    const int dpi = static_cast<int>(GetDpiForWindow(hwnd()));
    const int company = MulDiv(200, dpi, 96), period = MulDiv(100, dpi, 96);
    status_.SetParts({company, (std::max)(company, cx - period), -1});
    // The original also reread the company and period here, on every resize;
    // RefreshStatus() runs after the screens that change them instead.
    return true;
}

void ManilaGL::RefreshStatus()
{
    SetStatus(ID_STATUSHELP, GetCompany());       // TGlWindow::GLCreateStatusBar
    SetStatus(ID_PERIODFIELD, GetCurrentPeriod());
}

void ManilaGL::SetStatus(int field, const std::wstring& text)
{
    // The original's three fields of 2057 -> the status bar's parts.
    const int part = field == ID_STATUSHELP ? 0 : field == ID_MENUHELP ? 1 : 2;
    status_.SetText(part, text.c_str());
}

bool ManilaGL::OnCommand(WORD id, WORD code, HWND ctl)
{
    // Toolbar clicks are commands, as TToolBar::Mssg sent them (lParam 0).
    if (ctl && ctl == toolbar_.hwnd()) ctl = nullptr;
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
    case ID_FATYPE:                        // TManilaGL::GLFixedTypes: Fixed Assets > Asset Types (spec #4)
        RunAssetTypes(hwnd());
        return true;
    case ID_FAADD:                         // TManilaGL::GLFixedAdd: Fixed Assets > Add/Modify
        RunFixedAssets(hwnd());
        return true;
    case ID_FASLIP:                        // TManilaGL::GLFixedSlip: Fixed Assets > Create Voucher
        RunCreateDepreciationVoucher(hwnd());
        return true;
    case ID_DISPOSAL:                      // TManilaGL::GLFixedDisposal: Fixed Assets > Disposal
        RunAssetDisposal(hwnd());
        return true;
    case kFixedAssetListing:               // Fixed Assets > Fixed assets listing (GL_FA.RPT, spec #4 7)
        RunFixedAssetListing(*this);
        return true;
    case kFixedAssetTypes:                 // ... > Fixed asset types (GL_FATYP.RPT)
        RunFixedAssetTypes(*this);
        return true;
    case 4045:                             // TManilaGL::APAddVendors (ID_ADDVENDOR): Accounts > Purchase Vendors (spec #5 S5-M1)
        RunApVendors(hwnd());
        return true;
    case 4059:                             // TManilaGL::ARAddVendors (ID_AR_CUSTOMER): Accounts > Sales Customers (spec #6 S6-M1)
        RunArCustomers(hwnd());
        return true;
    case 5045:                             // TManilaGL::ARBatch (ID_ARBATCH): Transactions > Sales Invoice Vouchers (spec #6 S6-M2)
        RunArInvoiceEntry(hwnd());
        return true;
    case 4044:                             // TManilaGL::APBatch (ID_APBATCH): Transactions > Purchase Invoice Vouchers, toolbar (S5-M2)
        RunApInvoiceEntry(hwnd());
        return true;
    case 8074:                             // TManilaGL::APChequeAP: Confirm Printed Cheques (S5-M6, D9)
        RunApConfirmCheques(hwnd());
        return true;
    case 6085:                             // TManilaGL::APPayment: Bank Reconciliation (S5-M6)
        RunApBankReconciliation(hwnd());
        return true;
    case 1066:                             // TManilaGL::APPurgeAccts: Maintenance > Purge Paid Payables (S5-M6)
        RunApPurge(hwnd());
        return true;
    case 1057:                             // TManilaGL::APCreateGLSlip: Receipts/Payments > Create Cheques (S5-M5)
        RunApCreateCheques(hwnd());
        return true;
    case 1079:                             // TManilaGL::APCash: Transactions > Cash Invoice Vouchers, toolbar (S5-M4)
        RunApCashEntry(hwnd());
        return true;
    case 1088:                             // TManilaGL::APPostCash: Post Voucher > Cash Vouchers (S5-M4)
        RunApPost(hwnd(), ApKind::Cash);
        return true;
    case 1085:                             // TManilaGL::APCancelCash: Cancel Voucher > Cash Invoice Vouchers (S5-M4)
        RunApCancel(hwnd(), ApKind::Cash);
        return true;
    case 1048:                             // TManilaGL::APPostBatch: Post Voucher > Purchase Vouchers (S5-M3)
        RunApPost(hwnd(), ApKind::Invoice);
        return true;
    case 3064:                             // TManilaGL::APCancelBatch: Cancel Voucher > Purchase Vouchers (S5-M3)
        RunApCancel(hwnd(), ApKind::Invoice);
        return true;
    case CM_AP_RECOVER:                    // Transactions > Recover Purchase Voucher (spec #5 D7, S5-M3)
        RunApRecover(hwnd(), ApKind::Invoice);
        return true;
    case 4049:                             // TManilaGL::ARPostBatch (ID_ARPOST): Post Voucher > Sales Voucher (spec #6 S6-M3)
        RunArPost(hwnd());
        return true;
    case 4053:                             // TManilaGL::ARCancelBatch (ID_ARCANCEL): Cancel Voucher > Sales Invoice Voucher (S6-M3)
        RunArCancel(hwnd());
        return true;
    case CM_AR_RECOVER:                    // Transactions > Recover Sales Voucher (spec #6 D7, S6-M3)
        RunArRecover(hwnd());
        return true;
    case 4054:                             // TManilaGL::ARCreateSlip (ID_ARPAYMENT): Clear Receipts (spec #6 S6-M4)
        RunArReceipts(hwnd());
        return true;
    case 4062:                             // TManilaGL::ARPurgeAccts (ID_ARPURGE): Maintenance > Purge Received Receivables (S6-M5)
        RunArPurge(hwnd());
        return true;
    case 1028:                             // TGlWindow::GLUpdateAP: Maintenance > Update AR and AP (S5-M3)
        RunApTransfer(hwnd());
        return true;
    case ID_MODBATCH:                      // TGlWindow::GLModifyBatch: Transactions > GL Voucher, GLSUB, toolbar
    case ID_BATCHENT:                      // TGlWindow::GLBatchEnt (the same dialog)
        RunVoucherEntry(hwnd());
        return true;
    case CM_VOUCHER_LIST:                  // Transactions > Voucher List (VoucherBrowser.h)
        OpenVoucherBrowser(*this);
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
        RunReportList();                   // no preview active: the report list (1022)
        return true;
    case CM_REPORT_LIST:                   // Reporting > Custom Report, GLSUB (D9; 3068 in the original)
        RunReportList();
        return true;
    case CM_STATEMENT_LAYOUT:              // Accounts > Statement Layout (spec #3 6.4)
        RunStatementLayout(hwnd());
        return true;
    case CM_SAMPLE_REPORT:                 // the report engine's test report (gl.exe /samplereport)
        RunSampleReport(*this);
        return true;
    case ID_TB:                            // Reporting > General Ledger > Trial Balance (spec #3 F1, D14)
        RunTrialBalance(*this);
        return true;
    case ID_DISTLIST:                      // Reporting > General Ledger > Distribution List (F2, D14)
        RunDistributionTrialBalance(*this);
        return true;
    case ID_FININCOME:                     // Reporting > Financial Statements > Income statement (F3, D13)
        RunIncomeStatement(*this);
        return true;
    case ID_FINBS:                         // ... > Balance sheet (F4)
        RunBalanceSheet(*this);
        return true;
    case ID_FINRE:                         // ... > Cash flow statement (F5; was retained earnings, D13)
        RunCashFlowStatement(*this);
        return true;
    case ID_TBBYDEPT:                      // Reporting > General Ledger > Trial Balance by Dept. (GLDEPT.RPT)
        RunTrialBalanceByDept(*this);
        return true;
    case ID_GLCHARTINQ:                    // Accounts > Chart of Accounts Report, the toolbar's inquiry button
        RunChartOfAccountsReport(*this);
        return true;
    case ID_GLBUDGETINQ:                   // Accounts > Budget Report (GL_BUDGT.RPT, D6)
        RunBudgetReport(*this);
        return true;
    case ID_SLIP:                          // Reporting > General Ledger > Voucher Slips (GLSLIP.RPT)
        RunVoucherSlips(*this);
        return true;
    case CM_SLIPDETAIL:                    // ... > Voucher Detail (GLBTCHD.RPT, D8)
        RunVoucherDetail(*this);
        return true;
    case ID_SLIPSUM:                       // ... > Voucher Summary (GL_BTCHS.RPT)
        RunVoucherSummary(*this);
        return true;
    case ID_PREPOST:                       // ... > Preposting Distribution List (GLPRE.RPT, D4)
        RunPrepostingDistribution(*this);
        return true;
    case kPrePostingReport:                // Transactions > Voucher Reports > GL Pre-Posting Report (D5)
        RunPrePostingReport(*this);
        return true;
    case ID_REPLIST:                       // Reporting > General Ledger > Medical Representatives (GL_MEDRP.RPT)
        RunMedicalRepsListing(*this);
        return true;
    case ID_PRODUCTLIST:                   // ... > Product code summary (GL_PRODL.RPT)
        RunProductCodesListing(*this);
        return true;
    case ID_DOCTOR_RPT:                    // ... > Doctor Codes (GL_DRCDS.RPT, D12)
        RunDoctorCodesListing(*this);
        return true;
    default:
        break;
    }
    if (!ctl && RunApReport(*this, id)) return true;   // the AP reports, 4091 Print Cheques among them (spec #5 7)
    if (!ctl && RunArReport(*this, id)) return true;   // the AR reports (spec #6 7)
    if (ctl || id >= 0xFF00) return false;   // child-window IDs: DefFrameProc
    NotPorted(id);
    return true;
}

void ManilaGL::RunReportList()
{
    // The chosen report runs as its menu item does, once the list has closed.
    if (const WORD id = ShowReportList(hwnd())) OnCommand(id, 0, nullptr);
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
    if (contains(kArReports, id)) {
        ReportingStubs::Show(hwnd(), item.c_str(), L"Accounts Receivable", 6);
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
    case WM_INITMENUPOPUP: {   // TManilaGL::WMInitMenuPopup
        HMENU menu = GetMenu(hwnd());
        ApplyMenuAccess(menu);
        // Statement Layout is behind the Chart of Accounts' access (spec #3 6.4).
        if (menu)
            EnableMenuItem(menu, CM_STATEMENT_LAYOUT,
                           MF_BYCOMMAND | ((GetMenuState(menu, ID_ACCTADD, MF_BYCOMMAND) & MF_GRAYED) ? MF_GRAYED : MF_ENABLED));
        // Recover Purchase / Sales Voucher are behind Recover General Ledger Voucher's access.
        if (menu)
            for (UINT id : {CM_AP_RECOVER, CM_AR_RECOVER})
                EnableMenuItem(menu, id,
                               MF_BYCOMMAND | ((GetMenuState(menu, ID_GLREPBATCH, MF_BYCOMMAND) & MF_GRAYED) ? MF_GRAYED : MF_ENABLED));
        // The voucher list opens Voucher Entry: behind its access.
        if (menu)
            EnableMenuItem(menu, CM_VOUCHER_LIST,
                           MF_BYCOMMAND | ((GetMenuState(menu, ID_MODBATCH, MF_BYCOMMAND) & MF_GRAYED) ? MF_GRAYED : MF_ENABLED));
        return false;        // DefFrameProc too
    }
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
    SetStatus(ID_MENUHELP, fx::LoadResString(hInst, str));
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
