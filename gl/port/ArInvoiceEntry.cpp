//
// ArInvoiceEntry.cpp - see ArInvoiceEntry.h.
//
// Translation notes (plan per-port checklist; decisions are spec #6 §8):
//   - D4: one Add was one product line. TARInvoiceEntry::Ok and
//     ModifyCurrentSlip appended a sales line, and added the line's VAT and
//     amount to a cumulative VAT and AR line, in 3-decimal doubles; an amount
//     typed with price 0 balanced on the first line only, and Modify and
//     Delete never recomputed (2.6 #5). The port keeps the screen: quantity
//     and price, or the amount, make a product line; Ar.h recomputes the VAT
//     and AR lines from all the product lines, in cents.
//   - D5: "No" to keeping the voucher removes only a voucher made in this
//     session (the original's RollBack deleted the voucher shown).
//   - D6: the duplicate check sought AR_DIST, so it only caught posted
//     invoices; Ar.h DuplicateArInvoice checks every voucher not cancelled.
//   - D13: the analysis codes go to their own columns (SaveArVoucher). The
//     combo boxes map each entry to its code (CB_SETITEMDATA); the code shows
//     beside each (5015, 1008, 1003), as the original's SelectMedRep etc.
//   - D16: modal; the original was modeless.
//   - 2.6 #6: the period is the one chosen, on every line (the original
//     overwrote it with the GL's current period after the first).
//   - 2.6 #7: only open vouchers are shown and changed. The original's Add on
//     a posted or cancelled voucher reopened it; Top / Bottom / Next /
//     Previous here skip the others, and picking one in the list says so.
//   - No "Enter Sample Entry?" (spec 4.2): a line needs an amount.
//   - Picking a product fills its average price (FAVPRICE), as SelectProduct.
//   - A line's description is the product's name, or its account's when no
//     product is picked (GetSalesDescription); 1053 has no description field.
//   - The line's account (2013 / 2014) starts as the header's sales account
//     (8079 / 8080, the customer's AR_ACCT), and can be changed per line
//     (spec 4.1); the original credited every line to the header's.
//   - The due date is the invoice date + terms when the customer is shown or
//     the due date is blank; a typed due date is kept (the original
//     overwrote it on every Add).
//   - A voucher shown takes its VAT account from its own VAT line.
//   - The template is gl_new.rc's (prune_rc.py), with Memo (1100) and Print
//     (ID_PRINT). Memo opens the voucher's memo (TARMemo, dialog 1060 and its
//     menu MEMO), as the original: Save keeps the text, Quit and Esc don't;
//     Enter makes a new line (the template's editor lacked ES_WANTRETURN, so
//     Enter saved). Print is hidden until the AR reports (S6-M6).
//   - Debit and Credit (2030, 2031) show the line picked, read-only: the
//     amounts are computed. The invoice total (1999) is read-only.
//
#include "ArInvoiceEntry.h"

#include "Accounts.h"
#include "AccountList.h"
#include "Ar.h"
#include "Assets.h"   // ValidateAccount
#include "Globals.h"
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

// Dialog 1053's controls (ar_btche.cpp's and ar_id.h's names).
constexpr int kBatchNo = 3031;      // ID_BATCHNO
constexpr int kPeriod = 1091;       // ID_PERIOD
constexpr int kPeriodDesc = 1084;
constexpr int kCustomer = 4001;     // ID_VENDORNO
constexpr int kShortName = 4002;    // ID_VENDORSHORT
constexpr int kSalesAccount = 8079; // ID_AR_ACCT: the customer's sales account
constexpr int kSalesDept = 8080;    // ID_AR_DEPT
constexpr int kArAccount = 1087;    // ID_APCONTROL: the AR control account
constexpr int kArDept = 1086;       // ID_APDEPART
constexpr int kVatAccount = 8081;   // ID_VATACCT
constexpr int kVatDept = 8082;      // ID_VATDEPT
constexpr int kInvDate = 4074;      // ID_INVDATE
constexpr int kDueDate = 4075;      // ID_DUEDATE
constexpr int kVatRate = 4065;      // ID_VATPERCENTAGE
constexpr int kTerms = 6086;        // ID_TERMS
constexpr int kInvoiceNo = 4012;    // ID_INVOICENO
constexpr int kOrderNo = 6022;      // ID_ORDERNO
constexpr int kOrderDate = 1006;
constexpr int kVin = 1004;          // ID_VIN_NUM
constexpr int kUnits = 1005;        // ID_UNITS
constexpr int kReference = 3036;    // ID_REFERENCE: "Document"
constexpr int kMedReps = 1097;      // ID_MEDREP
constexpr int kDoctors = 1098;      // ID_DRCODES
constexpr int kProducts = 1096;     // ID_PRODUCTS
constexpr int kRepCode = 5015;      // ID_REPS
constexpr int kDoctorCode = 1008;
constexpr int kProductCode = 1003;
constexpr int kQuantity = 1037;
constexpr int kPrice = 1038;
constexpr int kAmount = 1030;       // ID_INVOICE
constexpr int kTotal = 1999;        // the invoice total
constexpr int kLineNo = 3033;       // ID_LINENO
constexpr int kAccount = 2013;      // ID_ACCOUNTS: the line's sales account
constexpr int kDept = 2014;         // ID_DEPART
constexpr int kDebit = 2030;        // ID_DEBIT
constexpr int kCredit = 2031;       // ID_CREDIT
constexpr int kCustomers = 3043;    // ID_SEARCHCUST: "Vendors"
constexpr int kLists = 1121;        // ID_VIEW: "List" (the account list)
constexpr int kVoucherList = 1074;  // ID_SLIPLIST: "Vouchers"
constexpr int kMemo = 1100;         // "Memo"
constexpr int kPrintButton = 3068;  // ID_PRINT
constexpr LRESULT kNoCode = -1;     // the blank combo entry

std::string today()
{
    SYSTEMTIME t{};
    GetLocalTime(&t);
    char buf[16] = {};
    std::snprintf(buf, sizeof buf, "%04u%02u%02u", t.wYear, t.wMonth, t.wDay);
    return buf;
}

// TARMemo on dialog 1060: the voucher's memo, with the MEMO menu (File: Save,
// Quit; Edit: Cut, Copy, Paste, Clear).
class MemoDialog : public MainDialog {
public:
    explicit MemoDialog(std::string& memo) : memo_(memo) {}

protected:
    bool OnInitDialog() override
    {
        MainDialog::OnInitDialog();
        const HWND edit = GetDlgItem(hwnd(), kEditor);
        RECT rc{};
        GetClientRect(hwnd(), &rc);
        MoveWindow(edit, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, TRUE);
        SetWindowLongPtrW(edit, GWL_STYLE, GetWindowLongPtrW(edit, GWL_STYLE) | ES_WANTRETURN);
        SetField(kEditor, memo_);
        SetFocus(edit);
        return false;   // the focus is set
    }

    bool OnCommand(WORD id, WORD code, HWND ctl) override
    {
        const HWND edit = GetDlgItem(hwnd(), kEditor);
        if (ctl == edit) return MainDialog::OnCommand(id, code, ctl);   // EN_* notifications
        switch (id) {
        case IDOK:   // File > Save (TARMemo::Ok)
            memo_ = GetField(kEditor);
            End(IDOK);
            return true;
        case IDCANCEL:   // File > Quit, Esc: not MainDialog::Cancel, the voucher screen's tables stay open
            End(IDCANCEL);
            return true;
        case kCut: SendMessageW(edit, WM_CUT, 0, 0); return true;
        case kCopy: SendMessageW(edit, WM_COPY, 0, 0); return true;
        case kPaste: SendMessageW(edit, WM_PASTE, 0, 0); return true;
        case kClear:
            SendMessageW(edit, EM_SETSEL, 0, -1);
            SendMessageW(edit, WM_CLEAR, 0, 0);
            return true;
        default: return MainDialog::OnCommand(id, code, ctl);
        }
    }

private:
    static constexpr int kEditor = 3046;   // ID_EDITOR
    static constexpr int kCut = 3076, kCopy = 3077, kPaste = 3078, kClear = 3090;
    std::string& memo_;
};

class SalesEntry : public MainDialog {
public:
    explicit SalesEntry(bool closeAtOnce) : closeAtOnce_(closeAtOnce) {}

protected:
    bool OnInitDialog() override;
    bool OnCommand(WORD id, WORD code, HWND ctl) override;
    bool OnDestroy() override;
    void Cancel() override;

private:
    // ---- the combo boxes and periods (as AP's entry) --------------------
    void FillCombo(int id, Data4& t, const char* tag, const Field4& name, const Field4& code,
                   std::vector<std::string>& codes);
    std::string ComboCode(int id, const std::vector<std::string>& codes) const;
    void SelectCode(int id, const std::vector<std::string>& codes, const std::string& code, int codeField);
    void FillPeriods();
    int Period() const;
    void SelectPeriod(int period);
    void ShowPeriodDescription();
    void ProductPicked();

    // ---- the screen ----------------------------------------------------
    bool FocusIn(int id) const;
    long Number() const;
    int GetInt(int id) const;
    void SetInt(int id, int value) { IntSetDlgItemValue(hwnd(), id, value); }
    std::int64_t GetHundredths(int id) const;
    void SetHundredths(int id, std::int64_t v) { FloatSetDlgItemValue(hwnd(), id, static_cast<double>(v) / 100.0); }
    ApAccount Field(int account, int dept) const { return {StrTrim(GetField(account)), StrTrim(GetField(dept))}; }
    int ProductCount() const;

    bool ShowCustomer(const std::string& number, bool dueFromTerms);
    void EnterCustomer();
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
    void Memo();

    bool closeAtOnce_;
    ArAccounts setat_;
    bool transaction_ = false;   // a voucher was made in this session and not yet confirmed
    long made_ = 0;              // its number

    std::unique_ptr<AccountsTable> glaccts_;
    std::unique_ptr<RepTable> medreps_, doctors_;
    std::unique_ptr<ProductTable> products_;
    std::vector<std::string> medrepCodes_, doctorCodes_, productCodes_;

    std::vector<ArLine> shown_;   // the lines of the voucher shown
    AccountList accountList_;
    TableList customerList_, voucherList_;
    LineList lineList_;
};

bool SalesEntry::OnInitDialog()
{
    MainDialog::OnInitDialog();   // Close, Add, New, Top, Bottom, Next, Previous, Delete, Voucher pictures
    SetBitmap(kLists);            // 1121
    SendDlgItemMessageW(hwnd(), kCustomers, SBM_SETPICT, SUPERBTN_PICT_BITMAP, 4054);   // "LOOK": Search's picture
    ShowWindow(GetDlgItem(hwnd(), kPrintButton), SW_HIDE);                              // S6-M6
    for (int id : {kDebit, kCredit, kTotal}) EnableWindow(GetDlgItem(hwnd(), id), FALSE);
    for (int id : {kRepCode, kDoctorCode, kProductCode, kShortName})
        SendDlgItemMessageW(hwnd(), id, EM_SETREADONLY, TRUE, 0);

    EnsureArTables(cb);
    setat_ = LoadArAccounts(cb);

    glaccts_ = std::make_unique<AccountsTable>(cb);
    medreps_ = std::make_unique<RepTable>(cb, RepTable::Kind::MedicalRep);
    doctors_ = std::make_unique<RepTable>(cb, RepTable::Kind::Doctor);
    products_ = std::make_unique<ProductTable>(cb);
    FillPeriods();
    // LoadDoctors, LoadMedReps (rpname order), LoadProducts (the original
    // added them in fpcode order to a sorted box: by name).
    if (doctors_->isValid()) FillCombo(kDoctors, *doctors_, "drcode", doctors_->rpname, doctors_->code, doctorCodes_);
    if (medreps_->isValid()) FillCombo(kMedReps, *medreps_, "rpname", medreps_->rpname, medreps_->code, medrepCodes_);
    if (products_->isValid())
        FillCombo(kProducts, *products_, "ffulln", products_->ffulln, products_->fpcode, productCodes_);

    if (glaccts_->isValid()) accountList_.Create(hwnd(), *glaccts_, false);
    TableList::Spec customers;   // TARCustomerList
    customers.section = L"AR_CUST";
    customers.table = "ar_cust";
    customers.tag = "vendorno";
    customers.caption = L"Accounts Receivable Customers";
    customers.headers = {L"Customer", L"Short Name", L"Name"};
    customers.stretchColumn = 3;
    customerList_.Create(hwnd(), std::move(customers), [this](int row) {
        SetField(kCustomer, customerList_.RowText(row, 1));
        if (ShowCustomer(customerList_.RowText(row, 1), true)) SetFocus(GetDlgItem(hwnd(), kInvoiceNo));
    }, true, false);
    TableList::Spec vouchers;   // TARBatchList on ar_btch
    vouchers.section = L"AR_BTCH";
    vouchers.table = "ar_btch";
    vouchers.tag = "batchno";
    vouchers.caption = L"Sales Invoice Vouchers";
    vouchers.headers = {L"Voucher", L"Customer", L"Invoice"};
    vouchers.fields = {1, 6, 7};   // BATCHNO, VENDORNO, INVOICENO
    vouchers.stretchColumn = 3;
    voucherList_.Create(hwnd(), std::move(vouchers), [this](int row) {
        PickVoucher(std::atol(voucherList_.RowText(row, 1).c_str()));
    }, true, false);
    lineList_.Create(hwnd(), [this](int line) { PickLine(line); });

    Clear();
    if (closeAtOnce_) PostMessageW(hwnd(), WM_COMMAND, IDCANCEL, 0);
    return false;   // the focus is set
}

bool SalesEntry::OnCommand(WORD id, WORD code, HWND ctl)
{
    if (id == kPeriod) {
        if (code == CBN_SELCHANGE) ShowPeriodDescription();
        return true;
    }
    if (id == kProducts || id == kMedReps || id == kDoctors) {   // SelectProduct / SelectMedRep / SelectDoctor
        if (code == CBN_SELCHANGE) {
            if (id == kProducts) ProductPicked();
            else if (id == kMedReps) SetField(kRepCode, ComboCode(kMedReps, medrepCodes_));
            else SetField(kDoctorCode, ComboCode(kDoctors, doctorCodes_));
        }
        return true;
    }
    if (code != BN_CLICKED) return MainDialog::OnCommand(id, code, ctl);
    switch (id) {
    case IDOK:
        if (FocusIn(kCustomer)) EnterCustomer();   // Enter on the customer (spec #6 4.2)
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
    case kCustomers: Toggle(customerList_, StrTrim(GetField(kCustomer))); return true;
    case kLists: accountList_.Seek(GetField(kAccount)); return true;
    case kVoucherList: Toggle(voucherList_, {}); return true;
    case kMemo: Memo(); return true;
    case ID_ACCTLIST:   // the account list chose an account
        SetFocus(GetDlgItem(hwnd(), kQuantity));
        return true;
    default: return MainDialog::OnCommand(id, code, ctl);   // IDCANCEL -> Cancel()
    }
}

bool SalesEntry::OnDestroy()
{
    // Each window saves its position while it still exists.
    for (HWND w : {accountList_.hwnd(), customerList_.hwnd(), voucherList_.hwnd(), lineList_.hwnd()})
        if (w) DestroyWindow(w);
    return false;
}

void SalesEntry::Cancel()
{
    if (!AskToKeep()) return;
    MainDialog::Cancel();   // cb.closeAll(); cb.init(); close
}

// ---- the combo boxes and periods ---------------------------------------------

void SalesEntry::FillCombo(int id, Data4& t, const char* tag, const Field4& name, const Field4& code,
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

std::string SalesEntry::ComboCode(int id, const std::vector<std::string>& codes) const
{
    const HWND box = GetDlgItem(hwnd(), id);
    const LRESULT sel = SendMessageW(box, CB_GETCURSEL, 0, 0);
    if (sel == CB_ERR) return {};
    const LRESULT data = SendMessageW(box, CB_GETITEMDATA, static_cast<WPARAM>(sel), 0);
    return data >= 0 && static_cast<size_t>(data) < codes.size() ? codes[static_cast<size_t>(data)] : std::string();
}

void SalesEntry::SelectCode(int id, const std::vector<std::string>& codes, const std::string& code, int codeField)
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
    SetField(codeField, ComboCode(id, codes));
}

void SalesEntry::FillPeriods()   // ::SetPeriods
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

int SalesEntry::Period() const
{
    const HWND box = GetDlgItem(hwnd(), kPeriod);
    const LRESULT sel = SendMessageW(box, CB_GETCURSEL, 0, 0);
    return sel == CB_ERR ? 0 : static_cast<int>(SendMessageW(box, CB_GETITEMDATA, static_cast<WPARAM>(sel), 0));
}

void SalesEntry::SelectPeriod(int period)
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

void SalesEntry::ShowPeriodDescription()
{
    const int period = Period();
    std::string desc;
    for (const OpenPeriod& p : OpenPeriods(cb))
        if (p.number == period) desc = p.description;
    SetField(kPeriodDesc, desc);
}

void SalesEntry::ProductPicked()   // TARInvoiceEntry::SelectProduct: the code and the average price
{
    const std::string code = ComboCode(kProducts, productCodes_);
    SetField(kProductCode, code);
    if (code.empty() || !products_->isValid()) return;
    products_->SetIndex("fpcode");
    for (products_->top(); !products_->eof(); products_->skip())
        if (TrimText(products_->fpcode.str()) == code) {
            FloatSetDlgItemValue(hwnd(), kPrice, std::atof(products_->favprice.str()));
            return;
        }
}

// ---- the screen ------------------------------------------------------------------

bool SalesEntry::FocusIn(int id) const
{
    const HWND control = GetDlgItem(hwnd(), id), focus = GetFocus();
    return focus && (focus == control || GetParent(focus) == control);
}

long SalesEntry::Number() const
{
    LONG n = 0;
    IntGetDlgItemValue(hwnd(), kBatchNo, &n);
    return n;
}

int SalesEntry::GetInt(int id) const
{
    LONG n = 0;
    IntGetDlgItemValue(hwnd(), id, &n);
    return static_cast<int>(n);
}

std::int64_t SalesEntry::GetHundredths(int id) const   // an amount in cents, or a quantity in hundredths
{
    double v = 0;
    FloatGetDlgItemValue(hwnd(), id, &v);
    return std::llround(v * 100.0);
}

int SalesEntry::ProductCount() const
{
    int n = 0;
    for (const ArLine& l : shown_)
        if (l.role == ArLineRole::Sales) ++n;
    return n;
}

// TARInvoiceEntry::SetVendor: false if there is no such customer.
bool SalesEntry::ShowCustomer(const std::string& number, bool dueFromTerms)
{
    const std::optional<Customer> c = FindCustomer(cb, number);
    if (!c) return false;
    SetField(kCustomer, c->number);
    SetField(kShortName, c->shortName);
    SetField(kArAccount, c->ar.account);
    SetField(kArDept, c->ar.dept);
    SetField(kSalesAccount, c->sales.account);
    SetField(kSalesDept, c->sales.dept);
    SetField(kVatAccount, c->vat.account);
    SetField(kVatDept, c->vat.dept);
    SetField(kVin, c->vin);
    SetInt(kVatRate, c->vatRate);
    SetInt(kTerms, c->terms);
    if (dueFromTerms) {
        const std::string due = AddDays(GetDate(kInvDate), c->terms);
        if (!due.empty()) SetField(kDueDate, due);
    }
    if (GetInt(kLineNo) > ProductCount()) {   // a new line: on the customer's sales account
        SetField(kAccount, c->sales.account);
        SetField(kDept, c->sales.dept);
    }
    return true;
}

void SalesEntry::EnterCustomer()
{
    const std::string number = StrTrim(GetField(kCustomer));
    if (number.empty()) {
        Toggle(customerList_, {});
        return;
    }
    if (ShowCustomer(number, true)) {
        SetFocus(GetDlgItem(hwnd(), kInvoiceNo));
        return;
    }
    Message(fx::Format(L"There is no customer %s.\n\nAdd them in Accounts > Sales Customers, or pick one in the "
                       L"customer list.",
                       fx::Utf8ToWide(number).c_str()),
            MB_ICONINFORMATION);
    customerList_.Seek(number);
}

void SalesEntry::ShowVoucher(long number)   // TARInvoiceEntry::CurrentBatch
{
    const std::optional<ArVoucher> v = FindArVoucher(cb, number);
    if (!v) return;
    shown_ = LoadArLines(cb, number);
    SetInt(kBatchNo, static_cast<int>(v->number));
    ShowCustomer(v->customer, false);   // the customer's name and accounts; the voucher's own values follow
    SetField(kCustomer, v->customer);
    SetField(kInvoiceNo, v->invoiceNo);
    SetField(kInvDate, v->invoiceDate);
    SetField(kDueDate, v->dueDate);
    SetField(kOrderNo, v->orderNo);
    SetField(kOrderDate, v->orderDate);
    SetField(kVin, v->vin);
    SetField(kUnits, v->units);
    SetField(kReference, v->reference);
    SetField(kArAccount, v->account.account);
    SetField(kArDept, v->account.dept);
    SetInt(kVatRate, v->vatRate);
    SetInt(kTerms, v->terms);
    SelectPeriod(v->period);
    SetHundredths(kTotal, v->total);
    for (const ArLine& l : shown_)   // the voucher's own VAT account
        if (l.role == ArLineRole::Vat) {
            SetField(kVatAccount, l.account.account);
            SetField(kVatDept, l.account.dept);
        }
    std::vector<VoucherLine> rows;
    double debit = 0, credit = 0;
    for (const ArLine& l : shown_) {
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

void SalesEntry::PickVoucher(long number)   // the voucher list
{
    const std::optional<ArVoucher> v = FindArVoucher(cb, number);
    if (!v) return;
    if (v->mode != ar_mode::kOpen) {
        const wchar_t* state = v->mode == ar_mode::kCancelled     ? L"cancelled"
                               : v->mode == ar_mode::kTransferred ? L"transferred to the General Ledger"
                                                                  : L"posted";
        Message(fx::Format(L"Voucher %ld is %s: it can't be changed here.", number, state), MB_ICONINFORMATION);
        return;
    }
    if (!AskToKeep()) return;
    ShowVoucher(number);
}

void SalesEntry::PickLine(int line)   // TARDistList::SelectCell, then SetLineItem
{
    const ArLine* found = nullptr;
    for (const ArLine& l : shown_)
        if (l.line == line) found = &l;
    if (!found || found->role != ArLineRole::Sales) {   // a computed line, the total or a blank row
        if (found)
            Message(L"The VAT and AR lines are computed from the product lines. Change a product line, or the VAT "
                    L"rate, and Add.",
                    MB_ICONINFORMATION);
        return;
    }
    SetInt(kLineNo, line);   // the product lines come first, so this is also its place among them
    SetField(kAccount, found->account.account);
    SetField(kDept, found->account.dept);
    if (found->price != 0) {
        SetHundredths(kQuantity, found->qty);
        SetHundredths(kPrice, found->price);
        SetHundredths(kAmount, LineAmount(found->qty, found->price, found->gross));
    } else {
        SetField(kQuantity, "");
        if (found->qty) SetHundredths(kQuantity, found->qty);
        SetField(kPrice, "");
        SetHundredths(kAmount, found->gross);
    }
    SetHundredths(kDebit, 0);
    SetHundredths(kCredit, found->credit);
    SelectCode(kMedReps, medrepCodes_, found->medrep, kRepCode);
    SelectCode(kProducts, productCodes_, found->product, kProductCode);
    SelectCode(kDoctors, doctorCodes_, found->drcode, kDoctorCode);
    SetFocus(GetDlgItem(hwnd(), kQuantity));
}

void SalesEntry::ClearLine()
{
    SetInt(kLineNo, ProductCount() + 1);
    for (int id : {kQuantity, kPrice, kAmount, kDebit, kCredit}) SetField(id, "");   // blank: typing doesn't land before "0.00"
    SetField(kAccount, GetField(kSalesAccount));
    SetField(kDept, GetField(kSalesDept));
    SelectCode(kMedReps, medrepCodes_, {}, kRepCode);
    SelectCode(kProducts, productCodes_, {}, kProductCode);
    SelectCode(kDoctors, doctorCodes_, {}, kDoctorCode);
}

void SalesEntry::Clear()   // TARInvoiceEntry::Clear
{
    transaction_ = false;
    made_ = 0;
    shown_.clear();
    SetInt(kBatchNo, static_cast<int>(NextArVoucherNumber(cb)));
    for (int id : {kCustomer, kShortName, kInvoiceNo, kOrderNo, kOrderDate, kVin, kUnits, kReference, kDueDate, kTotal})
        SetField(id, "");
    SetField(kArAccount, setat_.ar.account);   // GL_SETAT, until a customer is shown (D12)
    SetField(kArDept, setat_.ar.dept);
    SetField(kSalesAccount, setat_.sales.account);
    SetField(kSalesDept, setat_.sales.dept);
    SetField(kVatAccount, setat_.vatPayable.account);
    SetField(kVatDept, setat_.vatPayable.dept);
    SetField(kInvDate, today());
    SetInt(kVatRate, 0);
    SetInt(kTerms, 0);
    lineList_.Clear();
    ClearLine();
    SetFocus(GetDlgItem(hwnd(), kCustomer));
}

bool SalesEntry::Refuse(int id, const std::wstring& text)
{
    Message(text, MB_ICONEXCLAMATION);
    if (id) SetFocus(GetDlgItem(hwnd(), id));
    return false;
}

void SalesEntry::Add()   // TARInvoiceEntry::Ok
{
    auto required = [this](int id, const wchar_t* name) {
        if (!StrTrim(GetField(id)).empty()) return true;
        return Refuse(id, fx::Format(GetString(4072).c_str(), name));   // "The field %s must be completed..."
    };
    const std::string customer = StrTrim(GetField(kCustomer));
    if (customer.empty()) {   // the original beeped and showed the list
        Toggle(customerList_, {});
        return;
    }
    if (!FindCustomer(cb, customer)) {
        EnterCustomer();
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
    const ApAccount ar = Field(kArAccount, kArDept), vat = Field(kVatAccount, kVatDept);
    if (!ValidateAccount(cb, ar.account, ar.dept)) {
        Refuse(kArAccount, GetString(6091));   // the AR account
        return;
    }
    const int vatRate = GetInt(kVatRate);
    if (vatRate < 0 || vatRate > 99) {
        Refuse(kVatRate, L"The VAT rate must be a whole percentage from 0 to 99.");
        return;
    }
    if (vatRate > 0 && vat.empty()) {
        Refuse(kVatAccount, L"The VAT rate is above zero but there is no VAT account: enter the output VAT account.");
        return;
    }
    if (vatRate > 0 && !ValidateAccount(cb, vat.account, vat.dept)) {
        Refuse(kVatAccount, GetString(6090));   // the VAT account
        return;
    }
    SaleEntry entry;
    entry.account = Field(kAccount, kDept);
    if (!ValidateAccount(cb, entry.account.account, entry.account.dept)) {   // ValidGLAccount, per line
        Message(GetString(6089), MB_ICONINFORMATION);   // the sales account
        accountList_.Seek(entry.account.account);
        return;
    }
    entry.qty = GetHundredths(kQuantity);
    entry.price = GetHundredths(kPrice);
    entry.gross = GetHundredths(kAmount);
    if (entry.qty < 0 || entry.price < 0) {
        Refuse(entry.qty < 0 ? kQuantity : kPrice, L"The quantity and price can't be negative.");
        return;
    }
    if (entry.price > 0 && entry.qty == 0) {
        Refuse(kQuantity, L"Enter the quantity sold at this price.");
        return;
    }
    if (LineAmount(entry.qty, entry.price, entry.gross) <= 0) {
        Refuse(entry.price > 0 ? kQuantity : kAmount,
               L"Enter a quantity and unit price, or the line's amount, VAT included.");
        return;
    }
    const long number = Number();
    const bool exists = FindArVoucher(cb, number).has_value();
    if (const long dup = DuplicateArInvoice(cb, customer, GetField(kInvoiceNo), exists ? number : 0)) {
        Refuse(kInvoiceNo, fx::Format(L"Invoice number has been used previously.\n\nCustomer %s's invoice %s is on "
                                      L"sales voucher %ld.",
                                      fx::Utf8ToWide(customer).c_str(),
                                      fx::Utf8ToWide(StrTrim(GetField(kInvoiceNo))).c_str(), dup));
        return;
    }

    const int lineNo = GetInt(kLineNo);
    entry.line = lineNo <= ProductCount() ? lineNo : 0;
    entry.medrep = ComboCode(kMedReps, medrepCodes_);
    entry.product = ComboCode(kProducts, productCodes_);
    entry.drcode = ComboCode(kDoctors, doctorCodes_);
    if (!entry.product.empty()) {   // GetSalesDescription: the product's name (else the account's, Ar.h)
        const HWND box = GetDlgItem(hwnd(), kProducts);
        const LRESULT sel = SendMessageW(box, CB_GETCURSEL, 0, 0);
        wchar_t name[128] = {};
        if (sel != CB_ERR && SendMessageW(box, CB_GETLBTEXTLEN, static_cast<WPARAM>(sel), 0) < 128)
            SendMessageW(box, CB_GETLBTEXT, static_cast<WPARAM>(sel), reinterpret_cast<LPARAM>(name));
        entry.descript = StrTrim(fx::WideToUtf8(name));
    }

    ArVoucher h;
    h.number = exists ? number : 0;
    h.customer = customer;
    h.invoiceNo = StrTrim(GetField(kInvoiceNo));
    h.orderNo = StrTrim(GetField(kOrderNo));
    h.orderDate = GetDate(kOrderDate);
    h.vin = StrTrim(GetField(kVin));
    h.units = StrTrim(GetField(kUnits));
    h.reference = StrTrim(GetField(kReference));
    h.date = invDate;
    h.invoiceDate = invDate;
    h.dueDate = GetDate(kDueDate);
    h.period = period;
    h.terms = GetInt(kTerms);
    h.vatRate = vatRate;
    h.account = ar;
    const long saved = SaveSaleLine(cb, h, entry, vat);
    if (saved == 0) {
        Message(fx::Format(L"Voucher %ld is no longer open: it can't be changed.", number), MB_ICONINFORMATION);
        return;
    }
    if (!exists) {   // CreateAPBatchEntry
        transaction_ = true;
        made_ = saved;
    }
    ShowVoucher(saved);
    voucherList_.Redraw();
    SetFocus(GetDlgItem(hwnd(), kQuantity));
}

void SalesEntry::DeleteLine()   // TARInvoiceEntry::Delete
{
    const int line = GetInt(kLineNo);
    if (line < 1 || line > ProductCount()) return;   // no product line picked
    if (Message(GetString(6098), MB_YESNO | MB_ICONQUESTION) != IDYES) return;   // "...delete the current line entry."
    if (DeleteSaleLine(cb, Number(), line, Field(kVatAccount, kVatDept))) ShowVoucher(Number());
}

bool SalesEntry::AskToKeep()   // the ID_COMMIT question (D5)
{
    if (!transaction_) return true;
    switch (Message(GetString(1056), MB_YESNOCANCEL | MB_ICONQUESTION)) {   // "...save the current transaction?"
    case IDYES: transaction_ = false; return true;
    case IDNO:  // CM_ROLLBACK: only the voucher made in this session
        DeleteArVoucher(cb, made_);
        transaction_ = false;
        voucherList_.Redraw();
        Clear();
        return true;
    default: return false;
    }
}

void SalesEntry::Navigate(WORD id)   // Top / Bottom / Next / Previous over the open vouchers
{
    std::vector<long> open;
    for (const ArVoucher& v : LoadArVouchers(cb))
        if (v.mode == ar_mode::kOpen) open.push_back(v.number);
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

void SalesEntry::Memo()   // TARInvoiceEntry::Memo
{
    const std::optional<ArVoucher> v = FindArVoucher(cb, Number());
    if (!v) {
        Message(GetString(6088), MB_ICONINFORMATION);   // "Add the new invoice before adding a memo."
        return;
    }
    std::string memo = v->memo;
    MemoDialog dlg(memo);
    if (dlg.DoModal(1060, hwnd()) == IDOK) SaveArMemo(cb, v->number, memo);
}

void SalesEntry::Toggle(TableList& list, const std::string& seek)
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

INT_PTR RunArInvoiceEntry(HWND frame, bool closeAtOnce)
{
    SalesEntry dlg(closeAtOnce);
    return dlg.DoModal(1053, frame);
}
