//
// VoucherEntry.cpp - see VoucherEntry.h.
//
// Translation notes (plan per-port checklist; corrections are Spec #2
// design D11, record-level ones in Vouchers.cpp):
//   - The header description, a new voucher's number, its totals, line
//     replacement and the voucher's mode: see Vouchers.cpp.
//   - Picking a line loaded its fields but left the medical rep, product and
//     doctor combo boxes as they were, so saving it rewrote those codes from
//     whatever was showing; the port selects the line's own. Loading a
//     voucher selects its period in the period combo box (the original kept
//     the period shown), when that period is still open.
//   - CheckGLAccount seeked account+dept as a prefix ("1000" found "100000");
//     the port requires that account and dept (SeekAccount). CheckGLDept was
//     never called.
//   - The combo boxes map each entry to its code (CB_SETITEMDATA) instead of
//     seeking the table by the name shown, which picked the first of two
//     reps or products with the same name.
//   - IDOK (the tool box's OK button, and Enter) is Add/Mod. The original had
//     no IDOK handler, so OWL's TDialog::Ok closed the dialog without the
//     save-or-discard question.
//   - Print (ID_PRINT) printed the voucher slip with Crystal Reports, but
//     selected voucher 548 through a misnamed formula and went straight to
//     the printer. It now previews the voucher's slip (spec #3 D7) in a
//     window over this dialog; a voucher not yet saved gets a message.
//   - The windows are destroyed with the dialog, each saving its position
//     (the original's Cancel called their SavePosition, then cb.closeAll()).
//
#include "VoucherEntry.h"

#include "AccountList.h"
#include "Accounts.h"
#include "Globals.h"
#include "Ini.h"
#include "MainDialog.h"
#include "MasterTables.h"
#include "TableList.h"
#include "VoucherLists.h"
#include "Vouchers.h"
#include "reports/RunReport.h"
#include "reports/VoucherReports.h"
#include "stubs/Stubs.h"

#include "cb4sqlite.h"
#include "report/win32/Preview.h"

#include <cstdlib>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "LegacyIds.h"   // last: see LegacyIds.h

namespace {

// Dialog 1013's controls (gl/header/gl.h names, and gl_btche.cpp's own).
constexpr int kBatchNo = 3031;     // ID_BATCHNO
constexpr int kPeriod = 1091;      // ID_PERIOD
constexpr int kPeriodDesc = 1084;
constexpr int kDate = 3035;        // ID_DATE
constexpr int kDesc = 2999;        // ID_DESC: the voucher's description
constexpr int kShow = 2801;        // IDLISTBOX: the Show / Hide check box
constexpr int kReference = 3036;   // ID_REFERENCE: "Document"
constexpr int kJDescript = 3037;   // ID_JDESCRIPT: "Acct. Name"
constexpr int kCheque = 6070;      // ID_CHEQUE
constexpr int kMedReps = 6042;     // ID_MEDREPS
constexpr int kProducts = 1096;    // ID_PRODUCTS
constexpr int kDoctors = 1098;     // ID_DRCODES
constexpr int kLineNo = 3033;      // ID_LINENO
constexpr int kAccount = 2013;     // ID_ACCOUNTS
constexpr int kDept = 2014;        // ID_DEPART
constexpr int kDebit = 2030;       // ID_DEBIT
constexpr int kCredit = 2031;      // ID_CREDIT
constexpr int kAdd = 4095;         // ID_ADD: "Add/Mod"
constexpr int kAccountsList = 1121;   // ID_VIEW: "List"
constexpr int kVoucherList = 1074;    // ID_SLIPLIST: "Voucher"
constexpr int kPrint = 3068;          // ID_PRINT
constexpr LRESULT kNoCode = -1;       // the blank combo entry

long iniBatchno()
{
    return std::atol(fx::WideToUtf8(GetIniString(L"GL.INI", L"PERIOD", L"BATCHNO")).c_str());
}

class VoucherEntry : public MainDialog {
public:
    VoucherEntry(bool prior, bool closeAtOnce, long start = 0) : prior_(prior), closeAtOnce_(closeAtOnce), start_(start) {}

protected:
    bool OnInitDialog() override;
    bool OnCommand(WORD id, WORD code, HWND ctl) override;
    bool OnDestroy() override;
    void Cancel() override;

private:
    // ---- the combo boxes ------------------------------------------------
    void FillCombo(int id, Data4& t, const char* tag, const Field4& name, const Field4& code,
                   std::vector<std::string>& codes);
    std::string ComboCode(int id, const std::vector<std::string>& codes) const;
    void SelectCode(int id, const std::vector<std::string>& codes, const std::string& code);
    void FillPeriods();
    int Period() const;
    void SelectPeriod(int period);
    void ShowPeriodDescription();

    // ---- the commands ---------------------------------------------------
    void Add();                    // TGLBatchEntry::Add
    void DeleteLine();             // TGLBatchEntry::Delete
    void New();                    // TGLBatchEntry::New
    bool AskToKeep();              // the ID_COMMIT question; false: stay
    void Clear();                  // TGLBatchEntry::Clear
    void Navigate(WORD id);        // Next / Previous / Top / Bottom
    void ShowVoucher(long number); // TGLBatchEntry::CurrentBatch
    void PickLine(int line);       // TGLDistList::SelectCell
    void ToggleToolBox();          // TGLBatchEntry::ShowToolBox
    void ToggleVoucherList();      // TGLBatchEntry::ViewBatch
    bool EnterError(int id);       // TMainDialog::szEnterError
    void PrintSlip();              // Print (ID_PRINT), spec #3 D7
    long Number() const;

    bool prior_;
    bool closeAtOnce_;
    long start_;                   // the voucher to show on opening (the voucher browser); 0: a new one
    std::string mode_;
    bool transaction_ = false;     // bTransaction: a voucher was created and not yet confirmed

    std::unique_ptr<AccountsTable> glaccts_;
    std::unique_ptr<VoucherHeaders> headers_;
    std::unique_ptr<VoucherLines> lines_;
    std::unique_ptr<RepTable> medreps_, doctors_;
    std::unique_ptr<ProductTable> products_;
    std::vector<std::string> medrepCodes_, doctorCodes_, productCodes_;

    std::vector<VoucherLine> shown_;   // the lines of the voucher shown
    AccountList accountList_;
    TableList voucherList_;
    LineList lineList_;
    ToolBox toolBox_;
};

bool VoucherEntry::OnInitDialog()
{
    MainDialog::OnInitDialog();   // Close, New, Top, Bottom, Next, Previous, Delete, Voucher pictures
    for (const UINT id : {static_cast<UINT>(kAdd), static_cast<UINT>(kAccountsList), static_cast<UINT>(kPrint)})
        SetBitmap(id);
    mode_ = prior_ ? voucher_mode::kAdjustment : voucher_mode::kOpen;   // SetMode("1") / TGLPrior: SetMode("3")
    if (prior_) SetWindowTextW(hwnd(), L"Prior Period Adjustment Entry");

    glaccts_ = std::make_unique<AccountsTable>(cb);
    headers_ = std::make_unique<VoucherHeaders>(cb);
    lines_ = std::make_unique<VoucherLines>(cb);
    medreps_ = std::make_unique<RepTable>(cb, RepTable::Kind::MedicalRep);
    doctors_ = std::make_unique<RepTable>(cb, RepTable::Kind::Doctor);
    products_ = std::make_unique<ProductTable>(cb);
    const Data4 gldept(&cb, "gldept");
    const std::pair<const Data4*, const wchar_t*> required[] = {   // CheckFile
        {glaccts_.get(), L"GLACCTS"}, {&gldept, L"GLDEPT"},        {products_.get(), L"GL_ITEMM"},
        {medreps_.get(), L"GL_MEDRP"}, {doctors_.get(), L"GL_DRCOD"}, {headers_.get(), L"GL_BTCH"},
        {lines_.get(), L"GL_BTCHD"}};
    for (const auto& [table, name] : required)
        if (!table->isValid()) {
            Message(std::wstring(L"The company database has no ") + name + L" table.", MB_ICONINFORMATION);
            End(IDCANCEL);
            return true;
        }

    FillPeriods();
    // LoadDoctors (drcode order), LoadMedReps (rpname order), LoadProducts (ffulln order).
    FillCombo(kDoctors, *doctors_, "drcode", doctors_->rpname, doctors_->code, doctorCodes_);
    FillCombo(kMedReps, *medreps_, "rpname", medreps_->rpname, medreps_->code, medrepCodes_);
    FillCombo(kProducts, *products_, "ffulln", products_->ffulln, products_->fpcode, productCodes_);

    accountList_.Create(hwnd(), *glaccts_, false);
    TableList::Spec spec;   // TGLViewBatchList
    spec.section = L"GL_BTCH";
    spec.table = "gl_btch";
    spec.tag = "batchno";
    spec.caption = GetString(6071);                                     // "General Ledger Batch List"
    spec.headers = {GetString(6077), L" ", GetString(6079)};            // batchno, mode, reference
    voucherList_.Create(
        hwnd(), std::move(spec),
        [this](int row) { ShowVoucher(std::atol(voucherList_.RowText(row, 1).c_str())); }, true, false);
    lineList_.Create(hwnd(), [this](int line) { PickLine(line); });
    toolBox_.Create(hwnd());

    IntSetDlgItemValue(hwnd(), kBatchNo, NextVoucherNumber(*headers_, iniBatchno()));
    IntSetDlgItemValue(hwnd(), kLineNo, 1);
    SelectCode(kMedReps, medrepCodes_, {});
    SelectCode(kProducts, productCodes_, {});
    SelectCode(kDoctors, doctorCodes_, {});
    SetFocus(GetDlgItem(hwnd(), kPeriod));
    if (start_ > 0) ShowVoucher(start_);
    if (closeAtOnce_) PostMessageW(hwnd(), WM_COMMAND, IDCANCEL, 0);
    return false;   // focus set
}

bool VoucherEntry::OnCommand(WORD id, WORD code, HWND ctl)
{
    if (id == kPeriod) {   // TGLBatchEntry::SetPeriods
        if (code == CBN_SELCHANGE) ShowPeriodDescription();
        return true;
    }
    if (code != BN_CLICKED) return MainDialog::OnCommand(id, code, ctl);
    switch (id) {
    case kAdd:
    case IDOK: Add(); return true;
    case ID_DELETE: DeleteLine(); return true;
    case ID_NEW: New(); return true;
    case ID_TOP:
    case ID_BOTTOM:
    case ID_NEXT:
    case ID_PREVIOUS: Navigate(id); return true;
    case kAccountsList: accountList_.Seek(GetField(kAccount)); return true;   // ViewAccts
    case kVoucherList: ToggleVoucherList(); return true;
    case kShow: ToggleToolBox(); return true;
    case kPrint: PrintSlip(); return true;
    case ID_ACCTLIST:   // the account list chose an account (TAccountList::HandleListBox)
        SetFocus(GetDlgItem(hwnd(), kCheque));
        return true;
    default: return MainDialog::OnCommand(id, code, ctl);   // IDCANCEL -> Cancel()
    }
}

bool VoucherEntry::OnDestroy()
{
    // Each window saves its position while it still exists.
    for (HWND w : {accountList_.hwnd(), voucherList_.hwnd(), lineList_.hwnd(), toolBox_.hwnd()})
        if (w) DestroyWindow(w);
    return false;
}

void VoucherEntry::Cancel()
{
    if (!AskToKeep()) return;
    MainDialog::Cancel();   // cb.closeAll(); cb.init(); close
}

// ---- the combo boxes --------------------------------------------------------

void VoucherEntry::FillCombo(int id, Data4& t, const char* tag, const Field4& name, const Field4& code,
                             std::vector<std::string>& codes)
{
    const HWND box = GetDlgItem(hwnd(), id);
    SendMessageW(box, CB_RESETCONTENT, 0, 0);
    codes.clear();
    t.SetIndex(tag);
    for (t.top(); !t.eof(); t.skip()) {
        const LRESULT i = SendMessageW(box, CB_ADDSTRING, 0,
                                       reinterpret_cast<LPARAM>(fx::Utf8ToWide(TrimText(name.str())).c_str()));
        SendMessageW(box, CB_SETITEMDATA, static_cast<WPARAM>(i), static_cast<LPARAM>(codes.size()));
        codes.push_back(TrimText(code.str()));
    }
    const LRESULT blank = SendMessageW(box, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L" "));
    SendMessageW(box, CB_SETITEMDATA, static_cast<WPARAM>(blank), kNoCode);
}

std::string VoucherEntry::ComboCode(int id, const std::vector<std::string>& codes) const
{
    const HWND box = GetDlgItem(hwnd(), id);
    const LRESULT sel = SendMessageW(box, CB_GETCURSEL, 0, 0);
    if (sel == CB_ERR) return {};
    const LRESULT data = SendMessageW(box, CB_GETITEMDATA, static_cast<WPARAM>(sel), 0);
    return data >= 0 && static_cast<size_t>(data) < codes.size() ? codes[static_cast<size_t>(data)] : std::string();
}

void VoucherEntry::SelectCode(int id, const std::vector<std::string>& codes, const std::string& code)
{
    const HWND box = GetDlgItem(hwnd(), id);
    const LRESULT count = SendMessageW(box, CB_GETCOUNT, 0, 0);
    LRESULT pick = CB_ERR;
    for (LRESULT i = 0; i < count && pick == CB_ERR; ++i) {
        const LRESULT data = SendMessageW(box, CB_GETITEMDATA, static_cast<WPARAM>(i), 0);
        const bool blank = data < 0 || static_cast<size_t>(data) >= codes.size();
        if (code.empty() ? blank : (!blank && codes[static_cast<size_t>(data)] == code)) pick = i;
    }
    SendMessageW(box, CB_SETCURSEL, static_cast<WPARAM>(pick), 0);
}

void VoucherEntry::FillPeriods()   // ::SetPeriods
{
    const HWND box = GetDlgItem(hwnd(), kPeriod);
    SendMessageW(box, CB_RESETCONTENT, 0, 0);
    LRESULT last = CB_ERR;
    for (const OpenPeriod& p : OpenPeriods(cb)) {
        last = SendMessageW(box, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(std::to_wstring(p.number).c_str()));
        SendMessageW(box, CB_SETITEMDATA, static_cast<WPARAM>(last), p.number);
    }
    SendMessageW(box, CB_SETCURSEL, static_cast<WPARAM>(last), 0);   // the last added: the lowest open period
    ShowPeriodDescription();
}

int VoucherEntry::Period() const   // TGLBatchEntry::GetPeriod
{
    const HWND box = GetDlgItem(hwnd(), kPeriod);
    const LRESULT sel = SendMessageW(box, CB_GETCURSEL, 0, 0);
    return sel == CB_ERR ? 0 : static_cast<int>(SendMessageW(box, CB_GETITEMDATA, static_cast<WPARAM>(sel), 0));
}

void VoucherEntry::SelectPeriod(int period)
{
    const HWND box = GetDlgItem(hwnd(), kPeriod);
    const LRESULT count = SendMessageW(box, CB_GETCOUNT, 0, 0);
    for (LRESULT i = 0; i < count; ++i)
        if (SendMessageW(box, CB_GETITEMDATA, static_cast<WPARAM>(i), 0) == period) {
            SendMessageW(box, CB_SETCURSEL, static_cast<WPARAM>(i), 0);
            ShowPeriodDescription();
            return;
        }
}

void VoucherEntry::ShowPeriodDescription()
{
    const int period = Period();
    std::string desc;
    for (const OpenPeriod& p : OpenPeriods(cb))
        if (p.number == period) desc = p.description;
    SetField(kPeriodDesc, desc);
}

// ---- the commands -----------------------------------------------------------

long VoucherEntry::Number() const
{
    LONG n = 0;
    IntGetDlgItemValue(hwnd(), kBatchNo, &n);
    return n;
}

void VoucherEntry::PrintSlip()
{
    // D7: the voucher shown, as a slip, previewed over this dialog. The
    // original printed voucher 548 (its formula named the wrong field).
    const long number = Number();
    if (!SeekVoucher(*headers_, number)) {
        MessageBoxW(hwnd(),
                    fx::Format(L"Voucher %ld isn't saved yet. Add a line (Add/Mod) to save it, then print its slip.",
                               number)
                        .c_str(),
                    szApp, MB_OK | MB_ICONINFORMATION);
        return;
    }
    try {
        if (!rpt::win32::ShowPreviewWindow(hwnd(), VoucherSlip(cb, CurrentReportContext(), number)))
            MessageBoxW(hwnd(), L"The slip's preview could not be opened.", szApp, MB_OK | MB_ICONSTOP);
    } catch (const std::exception& e) {
        MessageBoxW(hwnd(), (L"The slip could not be prepared:\n\n" + fx::Utf8ToWide(e.what())).c_str(), szApp,
                    MB_OK | MB_ICONSTOP);
    }
}

bool VoucherEntry::EnterError(int id)
{
    if (!StrTrim(GetField(id)).empty()) return false;
    // TMainDialog::szEnterError named the field with the control ID's string:
    // 2999 "Description", 3036 "Document".
    Message(fx::Format(GetString(4072).c_str(), GetString(static_cast<UINT>(id)).c_str()), MB_ICONINFORMATION);
    SetFocus(GetDlgItem(hwnd(), id));
    return true;
}

void VoucherEntry::Add()
{
    if (EnterError(kDesc) || EnterError(kReference)) return;
    const std::string account = StrTrim(GetField(kAccount));
    const std::string dept = StrTrim(GetField(kDept));
    if (!SeekAccount(*glaccts_, account, dept)) {   // CheckGLAccount
        MessageBeep(MB_ICONEXCLAMATION);
        accountList_.Seek(account);
        return;
    }
    const std::string date = GetDate(kDate);
    if (date.empty()) {
        Message(GetString(8090), MB_OK | MB_ICONEXCLAMATION);   // "Double click on the date field to set the date."
        return;
    }
    double debit = 0, credit = 0;
    FloatGetDlgItemValue(hwnd(), kDebit, &debit);
    FloatGetDlgItemValue(hwnd(), kCredit, &credit);
    if (debit == 0 && credit == 0) {
        Message(GetString(8092), MB_OK | MB_ICONEXCLAMATION);   // "Enter a debit or credit amount!"
        return;
    }
    if (debit > 0 && credit > 0) {
        Message(GetString(8093), MB_OK);   // "...either a debit or credit amount but not both!"
        return;
    }
    const int period = Period();
    if (period == 0) {   // no open period (the original saved period 0)
        Message(fx::Format(GetString(4072).c_str(), L"Pd"), MB_ICONINFORMATION);
        SetFocus(GetDlgItem(hwnd(), kPeriod));
        return;
    }

    VoucherHeader header;
    header.mode = mode_;
    header.description = GetField(kDesc);
    header.date = date;
    header.period = period;
    header.cheque = GetField(kCheque);
    VoucherLine line;
    LONG lineNo = 0;
    IntGetDlgItemValue(hwnd(), kLineNo, &lineNo);
    line.line = static_cast<int>(lineNo);
    line.period = period;
    line.date = date;
    line.reference = GetField(kReference);
    line.descript = GetField(kJDescript);
    line.cheque = header.cheque;
    line.account = account;
    line.dept = dept;
    line.medrep = ComboCode(kMedReps, medrepCodes_);
    line.prod = ComboCode(kProducts, productCodes_);
    line.drcode = ComboCode(kDoctors, doctorCodes_);
    line.debit = debit;
    line.credit = credit;

    SaveKind kind{};
    const long number = SaveLine(*headers_, *lines_, Number(), header, line, iniBatchno(), &kind);
    if (kind == SaveKind::NewVoucher) transaction_ = true;   // AppendNewEntry
    FloatSetDlgItemValue(hwnd(), kDebit, 0.0);
    FloatSetDlgItemValue(hwnd(), kCredit, 0.0);
    ShowVoucher(number);
    voucherList_.Redraw();
}

void VoucherEntry::ShowVoucher(long number)   // TGLBatchEntry::CurrentBatch (and TGLPrior's)
{
    // TGLPrior::CurrentBatch checked the mode before the base class touched
    // the voucher (LoadVoucher renumbers its lines).
    if (prior_ && SeekVoucher(*headers_, number) && ReadHeader(*headers_).mode != voucher_mode::kAdjustment) {
        MessageBoxW(hwnd(), L"Only prior period adjustment entries may be modified in this program.", L"Adjustment",
                    MB_OK);
        return;
    }
    VoucherHeader header;
    std::vector<VoucherLine> lines;
    const LoadResult r = LoadVoucher(*headers_, *lines_, number, header, lines);
    if (r == LoadResult::NotFound || r == LoadResult::Posted) return;   // the original showed nothing
    if (r == LoadResult::NoLines) {
        Message(GetString(6087), MB_ICONINFORMATION);   // "Unable to find corresponding detail voucher."
        return;
    }
    shown_ = lines;
    IntSetDlgItemValue(hwnd(), kBatchNo, header.number);
    IntSetDlgItemValue(hwnd(), kLineNo, static_cast<LONG>(lines.size()) + 1);
    SetField(kDesc, header.description);
    SetField(kDate, header.date);
    SetField(kCheque, header.cheque);
    for (int id : {kAccount, kJDescript, kDept, kDebit, kCredit}) SetField(id, "");
    SelectPeriod(header.period);
    lineList_.Show(lines, header.debit, header.credit);
}

void VoucherEntry::PickLine(int line)   // TGLDistList::SelectCell
{
    const VoucherLine* found = nullptr;
    for (const VoucherLine& l : shown_)
        if (l.line == line) found = &l;
    if (!found) {   // the total row or a blank one: CM_CURRENTBATCH
        ShowVoucher(Number());
        return;
    }
    SetField(kDate, found->date);
    SetField(kReference, found->reference);
    SetField(kJDescript, found->descript);
    SetField(kCheque, found->cheque);
    SetField(kAccount, found->account);
    SetField(kDept, found->dept);
    FloatSetDlgItemValue(hwnd(), kDebit, found->debit);
    FloatSetDlgItemValue(hwnd(), kCredit, found->credit);
    IntSetDlgItemValue(hwnd(), kLineNo, found->line);
    SelectCode(kMedReps, medrepCodes_, found->medrep);
    SelectCode(kProducts, productCodes_, found->prod);
    SelectCode(kDoctors, doctorCodes_, found->drcode);
    SetFocus(GetDlgItem(hwnd(), kPeriod));
}

void VoucherEntry::DeleteLine()
{
    double debit = 0, credit = 0;
    FloatGetDlgItemValue(hwnd(), kDebit, &debit);
    FloatGetDlgItemValue(hwnd(), kCredit, &credit);
    if (debit <= 0 && credit <= 0) return;   // the original's condition: a line is shown
    LONG lineNo = 0;
    IntGetDlgItemValue(hwnd(), kLineNo, &lineNo);
    if (Message(GetString(6098), MB_YESNO | MB_ICONQUESTION) != IDYES) return;   // "...delete the current line entry."
    if (::DeleteLine(*headers_, *lines_, Number(), static_cast<int>(lineNo))) ShowVoucher(Number());
}

bool VoucherEntry::AskToKeep()
{
    if (!transaction_) return true;
    switch (Message(GetString(1056), MB_YESNOCANCEL | MB_ICONQUESTION)) {   // "...save the current transaction?"
    case IDYES: transaction_ = false; return true;
    case IDNO:  // CM_ROLLBACK
        DeleteVoucher(*headers_, *lines_, Number());
        transaction_ = false;
        voucherList_.Redraw();
        return true;
    default: return false;
    }
}

void VoucherEntry::New()
{
    if (AskToKeep()) Clear();
}

void VoucherEntry::Clear()
{
    transaction_ = false;
    shown_.clear();
    IntSetDlgItemValue(hwnd(), kBatchNo, NextVoucherNumber(*headers_, iniBatchno()));
    IntSetDlgItemValue(hwnd(), kLineNo, 1);
    for (int id : {kReference, kJDescript, kDesc, kCheque, kAccount}) SetField(id, "");
    FloatSetDlgItemValue(hwnd(), kCredit, 0.0);
    FloatSetDlgItemValue(hwnd(), kDebit, 0.0);
    lineList_.Clear();
    SelectCode(kProducts, productCodes_, {});
    SelectCode(kMedReps, medrepCodes_, {});
    SelectCode(kDoctors, doctorCodes_, {});
    SetFocus(GetDlgItem(hwnd(), kDesc));
}

void VoucherEntry::Navigate(WORD id)
{
    VoucherHeaders& h = *headers_;
    auto posted = [&] { return IsPostedMode(TrimText(h.mode.str())); };
    switch (id) {
    case ID_NEXT:
        if (!SeekVoucher(h, Number())) return;
        h.skip(1);
        if (h.eof()) h.bottom();
        break;
    case ID_PREVIOUS:
        if (!SeekVoucher(h, Number())) return;
        if (h.skip(-1) == r4bof || h.bof()) h.top();
        break;
    case ID_TOP:   // the first voucher not yet posted
        if (h.recCount() == 0) return;
        for (h.top(); !h.eof() && posted(); h.skip()) {
        }
        break;
    default:       // ID_BOTTOM: the last one not yet posted
        if (h.recCount() == 0) return;
        h.bottom();
        while (!h.bof() && posted())
            if (h.skip(-1) == r4bof) break;
        break;
    }
    if (h.eof() || h.bof()) return;
    ShowVoucher(ReadHeader(h).number);
}

void VoucherEntry::ToggleVoucherList()   // TGLBatchEntry::ViewBatch
{
    const HWND w = voucherList_.hwnd();
    if (!w) return;
    if (IsWindowVisible(w)) {
        ShowWindow(w, SW_HIDE);
    } else {
        voucherList_.Redraw();
        ShowWindow(w, SW_SHOWNOACTIVATE);
    }
}

void VoucherEntry::ToggleToolBox()   // TGLBatchEntry::ShowToolBox
{
    // The dialog narrows by its column of buttons (one button's width and 10
    // px, scaled) while the tool box shows, and widens by as much again after.
    // Measured each time, not remembered, so it holds after a DPI change.
    const HWND show = GetDlgItem(hwnd(), kShow);
    RECT rc{}, button{};
    GetWindowRect(hwnd(), &rc);
    GetWindowRect(GetDlgItem(hwnd(), ID_NEW), &button);
    const int column = button.right - button.left + MulDiv(10, static_cast<int>(GetDpiForWindow(hwnd())), 96);
    if (SendMessageW(show, BM_GETCHECK, 0, 0) != BST_CHECKED) {
        ShowWindow(toolBox_.hwnd(), SW_SHOWNOACTIVATE);
        SendMessageW(show, BM_SETCHECK, BST_CHECKED, 0);
        MoveWindow(hwnd(), rc.left, rc.top, rc.right - rc.left - column, rc.bottom - rc.top, TRUE);
        SetWindowTextW(show, L"&Hide");
    } else {
        SendMessageW(show, BM_SETCHECK, BST_UNCHECKED, 0);
        ShowWindow(toolBox_.hwnd(), SW_HIDE);
        MoveWindow(hwnd(), rc.left, rc.top, rc.right - rc.left + column, rc.bottom - rc.top, TRUE);
        SetWindowTextW(show, GetString(6066).c_str());   // "&Show"
    }
}

} // namespace

INT_PTR RunVoucherEntry(HWND frame, long voucher)
{
    VoucherEntry dlg(false, false, voucher);
    return dlg.DoModal(1013, frame);
}

INT_PTR RunPriorPeriodEntry(HWND frame, long voucher)
{
    VoucherEntry dlg(true, false, voucher);
    return dlg.DoModal(1013, frame);
}

int StressVoucherEntry(int times)
{
    auto objects = [] {
        HANDLE self = GetCurrentProcess();
        return static_cast<int>(GetGuiResources(self, GR_GDIOBJECTS) + GetGuiResources(self, GR_USEROBJECTS));
    };
    {
        VoucherEntry warmUp(false, true);   // first open: class registration, cached bitmaps
        if (warmUp.DoModal(1013, nullptr) == -1) return -1;
    }
    const int before = objects();
    for (int i = 0; i < times; ++i) {
        VoucherEntry dlg(i % 2 != 0, true);
        if (dlg.DoModal(1013, nullptr) == -1) return -1;
    }
    return objects() - before;
}
