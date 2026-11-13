//
// ApInvoiceEntry.cpp - see ApInvoiceEntry.h.
//
// Translation notes (plan per-port checklist; decisions are spec #5 §8):
//   - D4: one Add posted one amount (ID_INVOICE, labelled "Total") on one
//     expense account; GetAPExpense split it, the expense line was appended
//     and the VAT, W/H and AP lines replaced by account, with doubles and an
//     exact balance check. The port keeps the screen: the amount (relabelled
//     "Amount") and the line fields make an expense line; Ap.h recomputes the
//     voucher's VAT, W/H and AP lines from all its expense lines, in cents.
//     Modify and Delete recompute too (the original's didn't, 2.6 #4).
//   - D5: "No" to keeping the voucher removes only a voucher made in this
//     session (the original's RollBack deleted the voucher shown, whatever
//     it was, lines saved before included).
//   - D6: the duplicate check sought VENDORNO+INVOICENO on the batchno tag, so
//     it never matched; Ap.h DuplicateInvoice checks invoice and cash
//     vouchers, cancelled ones excepted.
//   - D13: the analysis codes go to their own columns (SaveApVoucher). The
//     combo boxes map each entry to its code (CB_SETITEMDATA), as Spec #2's
//     Voucher Entry.
//   - The due date is the invoice date + terms when the vendor is shown or
//     the due date is blank; a typed due date is kept (the original
//     overwrote it on every Add).
//   - The VAT account defaults to the vendor's (VATR for new vendors, D12);
//     the original read GL_SETAT VATP here and the vendor's after. A voucher
//     shown takes its VAT and W/H accounts from its own lines.
//   - Only open vouchers are shown and changed: Top / Bottom / Next /
//     Previous skip the others, as the original's skipped posted ones, and
//     picking a posted, transferred or cancelled voucher in the list says so.
//   - The original offered to add a missing vendor in a copy of the Vendors
//     dialog (TAPVendorBatchAdd); the port names the vendor and points to
//     Accounts > Purchase Vendors, showing the vendor list.
//   - Account and Dept are typed (the template made them read-only, filled
//     from the account list only); the list still fills them.
//   - Debit and Credit (2030, 2031) show the line picked, read-only: the
//     amounts are computed. Print (ID_PRINT) is hidden until the AP reports
//     (S5-M7). Company and Individual are plain radio buttons the code
//     checks, as in the original.
//
#include "ApInvoiceEntry.h"

#include "Accounts.h"
#include "AccountList.h"
#include "Ap.h"
#include "Assets.h"   // ValidateAccount
#include "Globals.h"
#include "Ini.h"
#include "MainDialog.h"
#include "MasterTables.h"
#include "TableList.h"
#include "VoucherLists.h"
#include "Vouchers.h"   // OpenPeriods, VoucherLine

#include "cb4sqlite.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "LegacyIds.h"   // last: see LegacyIds.h

namespace {

// Dialog 1042's controls (ap_btche.cpp's names).
constexpr int kBatchNo = 3031;     // ID_BATCHNO
constexpr int kPeriod = 1091;      // ID_PERIOD
constexpr int kPeriodDesc = 1084;
constexpr int kVendor = 4001;      // ID_VENDORNO
constexpr int kVendorName = 4003;  // ID_VENDORNAME
constexpr int kApAccount = 1087;   // ID_APCONTROL
constexpr int kApDept = 1086;      // ID_APDEPART
constexpr int kVatAccount = 1034;  // ID_VATACCT (this file's)
constexpr int kVatDept = 1035;     // ID_VATDEPT
constexpr int kVatRate = 4065;     // ID_VATPERCENTAGE
constexpr int kWhRate = 4073;      // ID_REBATE
constexpr int kTerms = 6086;       // ID_TERMS
constexpr int kCompany = 4078;     // ID_COMPANY
constexpr int kIndividual = 4079;  // ID_INDIVIDUAL
constexpr int kInvoiceNo = 4012;   // ID_INVOICENO
constexpr int kInvDate = 4074;     // ID_INVDATE
constexpr int kDueDate = 4075;     // ID_DUEDATE
constexpr int kAmount = 1030;      // ID_INVOICE ("Total")
constexpr int kOrderNo = 2017;     // ID_CURRBAL ("Order Number")
constexpr int kDescript = 3037;    // ID_JDESCRIPT: the line's description
constexpr int kReference = 3036;   // ID_REFERENCE: "Document", the voucher's reference
constexpr int kMedReps = 6042;     // ID_MEDREPS
constexpr int kProducts = 1096;    // ID_PRODUCTS
constexpr int kDoctors = 1098;     // ID_DRCODES
constexpr int kLineNo = 3033;      // ID_LINENO
constexpr int kAccount = 2013;     // ID_ACCOUNTS
constexpr int kDept = 2014;        // ID_DEPART
constexpr int kDebit = 2030;       // ID_DEBIT
constexpr int kCredit = 2031;      // ID_CREDIT
constexpr int kVendors = 3043;     // ID_SEARCHCUST: "Vendors"
constexpr int kLists = 1121;       // ID_VIEW: "Lists" (the account list)
constexpr int kVoucherList = 1074; // ID_SLIPLIST: "Voucher"
constexpr int kPrintButton = 3068; // ID_PRINT
constexpr LRESULT kNoCode = -1;    // the blank combo entry

// A static whose text is `text`, among the dialog's children (the labels are -1).
HWND findLabel(HWND dlg, const wchar_t* text)
{
    struct Find {
        const wchar_t* text;
        HWND found;
    } f{text, nullptr};
    EnumChildWindows(
        dlg,
        [](HWND h, LPARAM lp) -> BOOL {
            auto* f = reinterpret_cast<Find*>(lp);
            wchar_t cls[16] = {}, buf[64] = {};
            GetClassNameW(h, cls, 16);
            GetWindowTextW(h, buf, 64);
            if (_wcsicmp(cls, L"Static") == 0 && wcscmp(buf, f->text) == 0) {
                f->found = h;
                return FALSE;
            }
            return TRUE;
        },
        reinterpret_cast<LPARAM>(&f));
    return f.found;
}

std::string today()
{
    SYSTEMTIME t{};
    GetLocalTime(&t);
    char buf[16] = {};
    std::snprintf(buf, sizeof buf, "%04u%02u%02u", t.wYear, t.wMonth, t.wDay);
    return buf;
}

class InvoiceEntry : public MainDialog {
public:
    explicit InvoiceEntry(bool closeAtOnce) : closeAtOnce_(closeAtOnce) {}

protected:
    bool OnInitDialog() override;
    bool OnCommand(WORD id, WORD code, HWND ctl) override;
    bool OnDestroy() override;
    void Cancel() override;

private:
    // ---- the combo boxes and periods (Spec #2's Voucher Entry) ---------
    void FillCombo(int id, Data4& t, const char* tag, const Field4& name, const Field4& code,
                   std::vector<std::string>& codes);
    std::string ComboCode(int id, const std::vector<std::string>& codes) const;
    void SelectCode(int id, const std::vector<std::string>& codes, const std::string& code);
    void FillPeriods();
    int Period() const;
    void SelectPeriod(int period);
    void ShowPeriodDescription();

    // ---- the screen ----------------------------------------------------
    bool FocusIn(int id) const;
    long Number() const;
    int GetInt(int id) const;
    void SetInt(int id, int value) { IntSetDlgItemValue(hwnd(), id, value); }
    std::int64_t GetAmount(int id) const;
    bool Individual() const { return IsChecked(kIndividual); }
    void SetType(bool individual);
    ApAccount Field(int account, int dept) const { return {StrTrim(GetField(account)), StrTrim(GetField(dept))}; }
    VoucherAccounts Accounts() const;
    int ExpenseCount() const;

    bool ShowVendor(const std::string& number, bool dueFromTerms);
    void EnterVendor();
    void ShowVoucher(long number);
    void PickLine(int line);
    void PickVoucher(long number);
    void ClearLine();
    void Clear();
    bool Refuse(int id, const std::wstring& text);
    void Add();
    void DeleteLine();
    bool AskToKeep();
    void Navigate(WORD id);
    void Toggle(TableList& list, const std::string& seek);

    bool closeAtOnce_;
    TaxMode mode_ = TaxMode::F;
    ApAccounts setat_;
    bool transaction_ = false;   // a voucher was made in this session and not yet confirmed
    long made_ = 0;              // its number

    std::unique_ptr<AccountsTable> glaccts_;
    std::unique_ptr<RepTable> medreps_, doctors_;
    std::unique_ptr<ProductTable> products_;
    std::vector<std::string> medrepCodes_, doctorCodes_, productCodes_;

    std::vector<ApLine> shown_;   // the lines of the voucher shown
    AccountList accountList_;
    TableList vendorList_, voucherList_;
    LineList lineList_;
};

bool InvoiceEntry::OnInitDialog()
{
    MainDialog::OnInitDialog();   // Close, Add, New, Top, Bottom, Next, Previous, Delete, Voucher pictures
    SetBitmap(kLists);            // 1121
    SendDlgItemMessageW(hwnd(), kVendors, SBM_SETPICT, SUPERBTN_PICT_BITMAP, 4054);   // "LOOK": Search's picture
    ShowWindow(GetDlgItem(hwnd(), kPrintButton), SW_HIDE);                             // S5-M7
    if (HWND l = findLabel(hwnd(), L"Total")) SetWindowTextW(l, L"Amount");
    EnableWindow(GetDlgItem(hwnd(), kDebit), FALSE);
    // The template makes Account and Dept read-only (tbES_STATIC): the
    // original filled them from the account list only. They're typed, as on
    // Spec #2's Voucher Entry; the list still fills them.
    SendDlgItemMessageW(hwnd(), kAccount, EM_SETREADONLY, FALSE, 0);
    SendDlgItemMessageW(hwnd(), kDept, EM_SETREADONLY, FALSE, 0);
    EnableWindow(GetDlgItem(hwnd(), kCredit), FALSE);

    EnsureApTables(cb);
    setat_ = LoadApAccounts(cb);
    mode_ = ParseTaxMode(fx::WideToUtf8(GetIniString(L"GL.INI", L"WITHHOLDING", L"TAX")));

    glaccts_ = std::make_unique<AccountsTable>(cb);
    medreps_ = std::make_unique<RepTable>(cb, RepTable::Kind::MedicalRep);
    doctors_ = std::make_unique<RepTable>(cb, RepTable::Kind::Doctor);
    products_ = std::make_unique<ProductTable>(cb);
    FillPeriods();
    // LoadDoctors (drcode order), LoadMedReps (rpname order), LoadProducts (ffulln order).
    if (doctors_->isValid()) FillCombo(kDoctors, *doctors_, "drcode", doctors_->rpname, doctors_->code, doctorCodes_);
    if (medreps_->isValid()) FillCombo(kMedReps, *medreps_, "rpname", medreps_->rpname, medreps_->code, medrepCodes_);
    if (products_->isValid())
        FillCombo(kProducts, *products_, "ffulln", products_->ffulln, products_->fpcode, productCodes_);

    if (glaccts_->isValid()) accountList_.Create(hwnd(), *glaccts_, false);
    TableList::Spec vendors;   // TCustomerList "Vendor List"
    vendors.section = L"AP_VEND";
    vendors.table = "ap_vend";
    vendors.tag = "vendorno";
    vendors.caption = L"Vendor List";
    vendors.headers = {L"Vendor", L"Short Name", L"Name"};
    vendorList_.Create(hwnd(), std::move(vendors), [this](int row) {
        SetField(kVendor, vendorList_.RowText(row, 1));
        ShowVendor(vendorList_.RowText(row, 1), true);
    }, true, false);
    TableList::Spec vouchers;   // TBatchList on ap_btch
    vouchers.section = L"AP_BTCH";
    vouchers.table = "ap_btch";
    vouchers.tag = "batchno";
    vouchers.caption = L"Purchase Invoice Vouchers";
    vouchers.headers = {L"Voucher", L"Vendor", L"Invoice"};
    vouchers.fields = {1, 6, 8};   // BATCHNO, VENDORNO, INVOICENO
    vouchers.stretchColumn = 3;
    voucherList_.Create(hwnd(), std::move(vouchers), [this](int row) {
        PickVoucher(std::atol(voucherList_.RowText(row, 1).c_str()));
    }, true, false);
    lineList_.Create(hwnd(), [this](int line) { PickLine(line); });

    Clear();
    if (closeAtOnce_) PostMessageW(hwnd(), WM_COMMAND, IDCANCEL, 0);
    return false;   // the focus is set
}

bool InvoiceEntry::OnCommand(WORD id, WORD code, HWND ctl)
{
    if (id == kPeriod) {
        if (code == CBN_SELCHANGE) ShowPeriodDescription();
        return true;
    }
    if (code != BN_CLICKED) return MainDialog::OnCommand(id, code, ctl);
    switch (id) {
    case IDOK:
        if (FocusIn(kVendor)) EnterVendor();   // Enter on the vendor (spec #5 4.2)
        else Add();
        return true;
    case ID_DELETE: DeleteLine(); return true;
    case ID_NEW:
        if (AskToKeep()) Clear();
        return true;
    case ID_TOP:
    case ID_BOTTOM:
    case ID_NEXT:
    case ID_PREVIOUS: Navigate(id); return true;
    case kVendors: Toggle(vendorList_, StrTrim(GetField(kVendor))); return true;
    case kLists: accountList_.Seek(GetField(kAccount)); return true;
    case kVoucherList: Toggle(voucherList_, {}); return true;
    case kCompany:
    case kIndividual: SetType(id == kIndividual); return true;
    case ID_ACCTLIST:   // the account list chose an account (TAccountList::HandleListBox)
        SetFocus(GetDlgItem(hwnd(), kDescript));
        return true;
    default: return MainDialog::OnCommand(id, code, ctl);   // IDCANCEL -> Cancel()
    }
}

bool InvoiceEntry::OnDestroy()
{
    // Each window saves its position while it still exists.
    for (HWND w : {accountList_.hwnd(), vendorList_.hwnd(), voucherList_.hwnd(), lineList_.hwnd()})
        if (w) DestroyWindow(w);
    return false;
}

void InvoiceEntry::Cancel()
{
    if (!AskToKeep()) return;
    MainDialog::Cancel();   // cb.closeAll(); cb.init(); close
}

// ---- the combo boxes and periods ---------------------------------------------

void InvoiceEntry::FillCombo(int id, Data4& t, const char* tag, const Field4& name, const Field4& code,
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

std::string InvoiceEntry::ComboCode(int id, const std::vector<std::string>& codes) const
{
    const HWND box = GetDlgItem(hwnd(), id);
    const LRESULT sel = SendMessageW(box, CB_GETCURSEL, 0, 0);
    if (sel == CB_ERR) return {};
    const LRESULT data = SendMessageW(box, CB_GETITEMDATA, static_cast<WPARAM>(sel), 0);
    return data >= 0 && static_cast<size_t>(data) < codes.size() ? codes[static_cast<size_t>(data)] : std::string();
}

void InvoiceEntry::SelectCode(int id, const std::vector<std::string>& codes, const std::string& code)
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

void InvoiceEntry::FillPeriods()   // ::SetPeriods
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

int InvoiceEntry::Period() const
{
    const HWND box = GetDlgItem(hwnd(), kPeriod);
    const LRESULT sel = SendMessageW(box, CB_GETCURSEL, 0, 0);
    return sel == CB_ERR ? 0 : static_cast<int>(SendMessageW(box, CB_GETITEMDATA, static_cast<WPARAM>(sel), 0));
}

void InvoiceEntry::SelectPeriod(int period)
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

void InvoiceEntry::ShowPeriodDescription()
{
    const int period = Period();
    std::string desc;
    for (const OpenPeriod& p : OpenPeriods(cb))
        if (p.number == period) desc = p.description;
    SetField(kPeriodDesc, desc);
}

// ---- the screen ------------------------------------------------------------------

bool InvoiceEntry::FocusIn(int id) const
{
    const HWND control = GetDlgItem(hwnd(), id), focus = GetFocus();
    return focus && (focus == control || GetParent(focus) == control);
}

long InvoiceEntry::Number() const
{
    LONG n = 0;
    IntGetDlgItemValue(hwnd(), kBatchNo, &n);
    return n;
}

int InvoiceEntry::GetInt(int id) const
{
    LONG n = 0;
    IntGetDlgItemValue(hwnd(), id, &n);
    return static_cast<int>(n);
}

std::int64_t InvoiceEntry::GetAmount(int id) const
{
    double v = 0;
    FloatGetDlgItemValue(hwnd(), id, &v);
    return std::llround(v * 100.0);
}

void InvoiceEntry::SetType(bool individual)   // TAPInvoiceEntry::Company / Individual
{
    SetCheck(kIndividual, individual);
    SetCheck(kCompany, !individual);
}

VoucherAccounts InvoiceEntry::Accounts() const
{
    VoucherAccounts a;
    a.vat = Field(kVatAccount, kVatDept);
    a.wh = Individual() ? setat_.whIndividual : setat_.whCompany;   // GL_SETAT WTXI / WTXC
    a.credit = Field(kApAccount, kApDept);
    return a;
}

int InvoiceEntry::ExpenseCount() const
{
    int n = 0;
    for (const ApLine& l : shown_)
        if (l.role == ApLineRole::Expense) ++n;
    return n;
}

// TAPInvoiceEntry::SetVendor: false if there is no such vendor.
bool InvoiceEntry::ShowVendor(const std::string& number, bool dueFromTerms)
{
    const std::optional<Vendor> v = FindVendor(cb, number);
    if (!v) return false;
    SetField(kVendor, v->number);
    SetField(kVendorName, v->name);
    SetField(kApAccount, v->ap.account);
    SetField(kApDept, v->ap.dept);
    SetField(kVatAccount, v->vat.account);
    SetField(kVatDept, v->vat.dept);
    SetInt(kVatRate, v->vatRate);
    SetInt(kWhRate, v->whRate);
    SetInt(kTerms, v->terms);
    SetType(v->individual());
    if (dueFromTerms) {
        const std::string due = AddDays(GetDate(kInvDate), v->terms);
        if (!due.empty()) SetField(kDueDate, due);
    }
    return true;
}

void InvoiceEntry::EnterVendor()
{
    const std::string number = StrTrim(GetField(kVendor));
    if (number.empty()) {
        Toggle(vendorList_, {});
        return;
    }
    if (ShowVendor(number, true)) {
        SetFocus(GetDlgItem(hwnd(), kInvoiceNo));
        return;
    }
    Message(fx::Format(L"There is no vendor %s.\n\nAdd them in Accounts > Purchase Vendors, or pick one in the "
                       L"vendor list.",
                       fx::Utf8ToWide(number).c_str()),
            MB_ICONINFORMATION);
    vendorList_.Seek(number);
}

void InvoiceEntry::ShowVoucher(long number)   // TAPInvoiceEntry::CurrentBatch
{
    const std::optional<ApVoucher> v = FindApVoucher(cb, ApKind::Invoice, number);
    if (!v) return;
    shown_ = LoadApLines(cb, ApKind::Invoice, number);
    SetInt(kBatchNo, static_cast<int>(v->number));
    ShowVendor(v->vendor, false);   // the vendor's name and type; the voucher's own values follow
    SetField(kVendor, v->vendor);
    if (!v->vendorName.empty()) SetField(kVendorName, v->vendorName);
    SetField(kInvoiceNo, v->invoiceNo);
    SetField(kInvDate, v->invoiceDate);
    SetField(kDueDate, v->dueDate);
    SetField(kOrderNo, v->orderNo);
    SetField(kReference, v->reference);
    SetField(kApAccount, v->account.account);
    SetField(kApDept, v->account.dept);
    SetInt(kVatRate, v->vatRate);
    SetInt(kWhRate, v->whRate);
    SetInt(kTerms, v->terms);
    SelectPeriod(v->period);
    for (const ApLine& l : shown_) {   // the voucher's own VAT and W/H accounts
        if (l.role == ApLineRole::Vat) {
            SetField(kVatAccount, l.account.account);
            SetField(kVatDept, l.account.dept);
        }
        if (l.role == ApLineRole::WithHolding && !l.account.empty()) {
            if (l.account == setat_.whIndividual && !(l.account == setat_.whCompany)) SetType(true);
            if (l.account == setat_.whCompany && !(l.account == setat_.whIndividual)) SetType(false);
        }
    }
    std::vector<VoucherLine> rows;
    double debit = 0, credit = 0;
    for (const ApLine& l : shown_) {
        VoucherLine r;
        r.line = l.line;
        r.descript = l.descript;
        r.account = l.account.account;
        r.dept = l.account.dept;
        r.debit = static_cast<double>(l.debit) / 100.0;
        r.credit = static_cast<double>(l.credit) / 100.0;
        debit += r.debit;
        credit += r.credit;
        rows.push_back(r);
    }
    lineList_.Show(rows, debit, credit);
    ClearLine();
}

void InvoiceEntry::PickVoucher(long number)   // the voucher list
{
    const std::optional<ApVoucher> v = FindApVoucher(cb, ApKind::Invoice, number);
    if (!v) return;
    if (v->mode != ap_mode::kOpen) {
        const wchar_t* state = v->mode == ap_mode::kCancelled     ? L"cancelled"
                               : v->mode == ap_mode::kTransferred ? L"transferred to the General Ledger"
                                                                  : L"posted";
        Message(fx::Format(L"Voucher %ld is %s: it can't be changed here.", number, state), MB_ICONINFORMATION);
        return;
    }
    if (!AskToKeep()) return;
    ShowVoucher(number);
}

void InvoiceEntry::PickLine(int line)   // TAPDistList::SelectCell
{
    const ApLine* found = nullptr;
    for (const ApLine& l : shown_)
        if (l.line == line) found = &l;
    if (!found || found->role != ApLineRole::Expense) {   // a computed line, the total or a blank row
        if (found)
            Message(L"The VAT, withholding tax and AP lines are computed from the expense lines. Change an expense "
                    L"line, or the rates, and Add.",
                    MB_ICONINFORMATION);
        return;
    }
    SetInt(kLineNo, line);   // the expense lines come first, so this is also its place among them
    SetField(kAccount, found->account.account);
    SetField(kDept, found->account.dept);
    SetField(kDescript, found->descript);
    FloatSetDlgItemValue(hwnd(), kAmount, static_cast<double>(found->gross) / 100.0);
    FloatSetDlgItemValue(hwnd(), kDebit, static_cast<double>(found->debit) / 100.0);
    FloatSetDlgItemValue(hwnd(), kCredit, 0.0);
    SelectCode(kMedReps, medrepCodes_, found->medrep);
    SelectCode(kProducts, productCodes_, found->product);
    SelectCode(kDoctors, doctorCodes_, found->drcode);
    SetFocus(GetDlgItem(hwnd(), kAmount));
}

void InvoiceEntry::ClearLine()
{
    SetInt(kLineNo, ExpenseCount() + 1);
    for (int id : {kAccount, kDept, kDescript, kAmount, kDebit, kCredit}) SetField(id, "");   // blank: typing doesn't land before "0.00"
    SelectCode(kMedReps, medrepCodes_, {});
    SelectCode(kProducts, productCodes_, {});
    SelectCode(kDoctors, doctorCodes_, {});
}

void InvoiceEntry::Clear()   // TAPInvoiceEntry::Clear
{
    transaction_ = false;
    made_ = 0;
    shown_.clear();
    SetInt(kBatchNo, static_cast<int>(NextApVoucherNumber(cb, ApKind::Invoice)));
    for (int id : {kVendor, kVendorName, kInvoiceNo, kOrderNo, kReference, kDueDate}) SetField(id, "");
    SetField(kApAccount, setat_.ap.account);
    SetField(kApDept, setat_.ap.dept);
    SetField(kVatAccount, setat_.vatInput.account);
    SetField(kVatDept, setat_.vatInput.dept);
    SetField(kInvDate, today());
    SetInt(kVatRate, 0);
    SetInt(kWhRate, 0);
    SetInt(kTerms, 0);
    SetCheck(kCompany, false);
    SetCheck(kIndividual, false);
    lineList_.Clear();
    ClearLine();
    SetFocus(GetDlgItem(hwnd(), kVendor));
}

bool InvoiceEntry::Refuse(int id, const std::wstring& text)
{
    Message(text, MB_ICONEXCLAMATION);
    if (id) SetFocus(GetDlgItem(hwnd(), id));
    return false;
}

void InvoiceEntry::Add()   // TAPInvoiceEntry::Ok
{
    auto required = [this](int id, const wchar_t* name) {
        if (!StrTrim(GetField(id)).empty()) return true;
        return Refuse(id, fx::Format(GetString(4072).c_str(), name));   // "The field %s must be completed..."
    };
    const std::string vendor = StrTrim(GetField(kVendor));
    if (vendor.empty()) {
        Toggle(vendorList_, {});
        return;
    }
    if (!FindVendor(cb, vendor)) {
        EnterVendor();
        return;
    }
    if (!required(kInvoiceNo, L"Invoice No.")) return;
    const std::string invDate = GetDate(kInvDate);
    if (invDate.empty()) {
        Refuse(kInvDate, GetString(8090));   // "Double click on the date field to set the date."
        return;
    }
    if (GetDate(kDueDate).empty()) SetField(kDueDate, AddDays(invDate, GetInt(kTerms)));
    const int period = Period();
    if (period == 0) {
        Refuse(kPeriod, fx::Format(GetString(4072).c_str(), L"Period"));
        return;
    }
    if (!required(kReference, L"Document")) return;
    if (!IsChecked(kCompany) && !IsChecked(kIndividual)) {
        Refuse(kCompany, L"Choose Company or Individual: the withholding tax account depends on it.");
        return;
    }
    const VoucherAccounts accounts = Accounts();
    if (!ValidateAccount(cb, accounts.credit.account, accounts.credit.dept)) {
        Refuse(kApAccount, L"The AP account isn't in the chart of accounts.");
        return;
    }
    const int vatRate = GetInt(kVatRate), whRate = GetInt(kWhRate);
    if (vatRate < 0 || vatRate > 99 || whRate < 0 || whRate > 99) {
        Refuse(vatRate < 0 || vatRate > 99 ? kVatRate : kWhRate, L"A rate must be a whole percentage from 0 to 99.");
        return;
    }
    if (vatRate > 0 && accounts.vat.empty()) {
        Refuse(kVatAccount, GetString(8157));   // "...VAT percentage is greater than zero but no VAT GL account..."
        return;
    }
    if (vatRate > 0 && !ValidateAccount(cb, accounts.vat.account, accounts.vat.dept)) {
        Refuse(kVatAccount, L"The VAT account isn't in the chart of accounts.");
        return;
    }
    if (whRate > 0 && !ValidateAccount(cb, accounts.wh.account, accounts.wh.dept)) {
        Refuse(kWhRate, fx::Format(L"The %s withholding tax account (%s in Company Profile > Account Setup) isn't "
                                   L"set, or isn't in the chart of accounts.",
                                   Individual() ? L"individual" : L"company", Individual() ? L"WTXI" : L"WTXC"));
        return;
    }
    ExpenseEntry entry;
    entry.account = Field(kAccount, kDept);
    if (!ValidateAccount(cb, entry.account.account, entry.account.dept)) {   // ValidateGL, corrected
        MessageBeep(MB_ICONEXCLAMATION);
        accountList_.Seek(entry.account.account);
        return;
    }
    entry.gross = GetAmount(kAmount);
    if (entry.gross <= 0) {
        Refuse(kAmount, L"Enter the line's amount, VAT included.");
        return;
    }
    const long number = Number();
    const bool exists = FindApVoucher(cb, ApKind::Invoice, number).has_value();
    if (const auto dup = DuplicateInvoice(cb, vendor, GetField(kInvoiceNo),
                                          exists ? std::optional<ApVoucherRef>(ApVoucherRef{ApKind::Invoice, number})
                                                 : std::nullopt)) {
        Refuse(kInvoiceNo, fx::Format(L"%s\n\nVendor %s's invoice %s is on %s voucher %ld.",
                                      GetString(1039).c_str(), fx::Utf8ToWide(vendor).c_str(),
                                      fx::Utf8ToWide(StrTrim(GetField(kInvoiceNo))).c_str(),
                                      dup->kind == ApKind::Cash ? L"cash" : L"invoice", dup->number));
        return;
    }

    const int lineNo = GetInt(kLineNo);
    entry.line = lineNo <= ExpenseCount() ? lineNo : 0;
    entry.descript = StrTrim(GetField(kDescript));
    entry.medrep = ComboCode(kMedReps, medrepCodes_);
    entry.product = ComboCode(kProducts, productCodes_);
    entry.drcode = ComboCode(kDoctors, doctorCodes_);

    ApVoucher h;
    h.kind = ApKind::Invoice;
    h.number = exists ? number : 0;
    h.vendor = vendor;
    h.vendorName = StrTrim(GetField(kVendorName));
    h.invoiceNo = StrTrim(GetField(kInvoiceNo));
    h.orderNo = StrTrim(GetField(kOrderNo));
    h.reference = StrTrim(GetField(kReference));
    h.date = invDate;
    h.invoiceDate = invDate;
    h.dueDate = GetDate(kDueDate);
    h.period = period;
    h.terms = GetInt(kTerms);
    h.vatRate = vatRate;
    h.whRate = whRate;
    h.account = accounts.credit;
    const long saved = SaveExpenseLine(cb, h, entry, mode_, accounts);
    if (saved == 0) {
        Message(fx::Format(L"Voucher %ld is no longer open: it can't be changed.", number), MB_ICONINFORMATION);
        return;
    }
    if (!exists) {   // AppendNewEntry
        transaction_ = true;
        made_ = saved;
    }
    ShowVoucher(saved);
    voucherList_.Redraw();
    SetFocus(GetDlgItem(hwnd(), kAmount));
}

void InvoiceEntry::DeleteLine()   // TAPInvoiceEntry::Delete
{
    const int line = GetInt(kLineNo);
    if (line < 1 || line > ExpenseCount()) return;   // no expense line picked
    if (Message(GetString(6098), MB_YESNO | MB_ICONQUESTION) != IDYES) return;   // "...delete the current line entry."
    if (DeleteExpenseLine(cb, ApKind::Invoice, Number(), line, mode_, Accounts())) ShowVoucher(Number());
}

bool InvoiceEntry::AskToKeep()   // the ID_COMMIT question (D5)
{
    if (!transaction_) return true;
    switch (Message(GetString(1056), MB_YESNOCANCEL | MB_ICONQUESTION)) {   // "...save the current transaction?"
    case IDYES: transaction_ = false; return true;
    case IDNO:  // CM_ROLLBACK: only the voucher made in this session
        DeleteApVoucher(cb, ApKind::Invoice, made_);
        transaction_ = false;
        voucherList_.Redraw();
        Clear();
        return true;
    default: return false;
    }
}

void InvoiceEntry::Navigate(WORD id)   // Top / Bottom / Next / Previous over the open vouchers
{
    std::vector<long> open;
    for (const ApVoucher& v : LoadApVouchers(cb, ApKind::Invoice))
        if (v.mode == ap_mode::kOpen) open.push_back(v.number);
    if (open.empty()) {
        Message(GetString(8153), MB_ICONINFORMATION);   // "No invoices have yet been entered."
        return;
    }
    const long current = Number();
    long to = 0;
    switch (id) {
    case ID_TOP: to = open.front(); break;
    case ID_BOTTOM: to = open.back(); break;
    case ID_NEXT:
        to = open.back();
        for (long n : open)
            if (n > current) {
                to = n;
                break;
            }
        break;
    default:   // ID_PREVIOUS
        to = open.front();
        for (long n : open)
            if (n < current) to = n;
        break;
    }
    if (to == current && !shown_.empty()) return;
    if (!AskToKeep()) return;
    ShowVoucher(to);
}

void InvoiceEntry::Toggle(TableList& list, const std::string& seek)
{
    const HWND w = list.hwnd();
    if (!w) return;
    if (IsWindowVisible(w)) {
        ShowWindow(w, SW_HIDE);
    } else {
        list.Redraw();
        list.Seek(seek);
    }
}

} // namespace

INT_PTR RunApInvoiceEntry(HWND frame, bool closeAtOnce)
{
    InvoiceEntry dlg(closeAtOnce);
    return dlg.DoModal(1042, frame);
}
