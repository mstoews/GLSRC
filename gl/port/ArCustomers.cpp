//
// ArCustomers.cpp - see ArCustomers.h.
//
// Translation notes (plan per-port checklist; decisions are spec #6 §8):
//   - TARVendorAdd::SetupWindow opened its customer table on "ap_vend": the
//     screen maintained the vendors, not AR_CUST. The port works on ar_cust.
//   - D12: TARVendorOptions::SetupWindow's tests were crossed (ar_vendo.cpp:
//     117-128): it filled the sales account when ACCOUNT was set, and the AR
//     account when AR_ACCT was; and it put the VAT dept in 1083, which the
//     template doesn't have. Each field now shows its own field.
//   - D12, D15: Options read and wrote WITHTAX / WITHDEPT, which AR_CUST
//     doesn't have. The W/H account and its label are hidden.
//   - D12: Options needed the customer to be on file first (string 4119), and
//     a new customer was added with blank accounts, so every new customer
//     was saved unusable. The screen keeps the customer it shows, Options
//     edits it whether it's on file or not, and New fills the accounts from
//     GL_SETAT AR, SALE and VATP. Add checks everything (Ar.h CheckCustomer)
//     and names what's missing. Options' OK on a customer on file still
//     saves its fields at once, after asking (string 1093), as the original.
//   - TARVendorAdd::Update wrote the contact from ID_VENDORCONTACT, which is
//     on Options, not on 1052, so every Add/Mod blanked it. The contact is
//     Options' field only.
//   - Options' account check showed string 4017 ("Select a menu item from
//     the menu list..."); the port says which account isn't in the chart.
//   - D12: Delete deleted the current record whatever was shown, and said
//     "Vendor code not found."; it now deletes the customer shown, refusing
//     one with vouchers, open items or receipts (Ar.h DeleteCustomer).
//   - Find (Set Data) is as the original: it shows the typed customer, or
//     string 6017. Enter on the number does the same, and for a new number
//     moves on to the short name, as on the vendor screen (spec #5 D12).
//   - Next / Previous stop at the last / first customer. Modify keeps the
//     customer shown; adding clears, as the vendor screen.
//
#include "ArCustomers.h"

#include "Accounts.h"   // TrimText
#include "Ar.h"
#include "ArInvoiceEntry.h"
#include "Globals.h"
#include "MainDialog.h"
#include "MasterTables.h"   // SeekCode
#include "TableList.h"

#include "cb4sqlite.h"

#include <cmath>
#include <memory>
#include <optional>
#include <string>
#include <utility>

#include "LegacyIds.h"   // last: see LegacyIds.h

namespace {

// Dialog 1052's controls (gl/header/ap.h and ar_vendo.cpp names).
constexpr int kNumber = 4001;      // ID_VENDORNO
constexpr int kShort = 4002;       // ID_VENDORSHORT
constexpr int kName = 4003;        // ID_VENDORNAME
constexpr int kAddress1 = 4004;    // ID_VENDORADDRESS1 ("Street")
constexpr int kAddress2 = 4005;    // ("City")
constexpr int kAddress3 = 4006;    // ("State/Country")
constexpr int kPostal = 4007;      // ID_VENDORPOST
constexpr int kPhone = 6069;       // ID_PHONE
constexpr int kFax = 6068;         // ID_FAX
constexpr int kShip1 = 1078;       // ID_S_ADDRESS1
constexpr int kShip2 = 1079;       // ID_S_ADDRESS2
constexpr int kShip3 = 1080;       // ID_S_ADDRESS3
constexpr int kShipPostal = 1081;  // ID_S_POSTAL
constexpr int kShipPhone = 1076;   // ID_S_PHONE
constexpr int kShipFax = 1077;     // ID_S_FAX
constexpr int kOptions = 3001;     // IDMAINOPTIONS
constexpr int kList = 1121;        // ID_VIEW
constexpr int kFind = 4076;        // ID_SETDATA

// Dialog 1063's.
constexpr int kVatRate = 4081;     // ID_VAT
constexpr int kTerms = 6086;       // ID_TERMS
constexpr int kArAccount = 1087;   // ID_APCONTROL ("AR Acct")
constexpr int kArDept = 1086;      // ID_APDEPART
constexpr int kSalesAccount = 8079;   // ID_AR_ACCT ("Sales Acct")
constexpr int kSalesDept = 8080;   // ID_AR_DEPT
constexpr int kVatAccount = 8081;  // ID_VATACCT
constexpr int kVatDept = 8082;     // ID_VATDEPT
constexpr int kContact = 4009;     // ID_VENDORCONTACT
constexpr int kVin = 1005;         // ID_VIN_NUM ("VAT No.")
constexpr int kWhAccount = 8083;   // ID_WITHTAX: AR_CUST has no such field (D15)
constexpr int kWhDept = 8084;      // ID_WITHDEPT

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

const wchar_t* fieldName(CustomerField f)
{
    switch (f) {
    case CustomerField::Number: return L"Customer No.";
    case CustomerField::ShortName: return L"Short Name";
    case CustomerField::Name: return L"Customer Name";
    case CustomerField::VatRate: return L"Vat Percentage";
    case CustomerField::ArAccount: return L"AR account";
    case CustomerField::SalesAccount: return L"sales account";
    case CustomerField::VatAccount: return L"VAT account";
    default: return L"";
    }
}

bool onOptions(CustomerField f)
{
    return f == CustomerField::VatRate || f == CustomerField::ArAccount || f == CustomerField::SalesAccount ||
           f == CustomerField::VatAccount;
}

std::wstring problemText(const CustomerProblem& p, const std::wstring& required)
{
    switch (p.kind) {
    case CustomerProblem::Kind::Required: return fx::Format(required.c_str(), fieldName(p.field));
    case CustomerProblem::Kind::BadRate: return L"The VAT percentage must be a whole number from 0 to 99.";
    case CustomerProblem::Kind::BadAccount:
        return fx::Format(L"The %s isn't in the chart of accounts.", fieldName(p.field));
    case CustomerProblem::Kind::VatAccountNeeded:
        return L"A customer with a VAT percentage needs a VAT account: sales invoices book the VAT to it.";
    }
    return L"";
}

// ---- TARVendorOptions (1063) ----------------------------------------------------

class OptionsDialog : public MainDialog {
public:
    // `onFile`: the customer is in ar_cust, so OK saves these fields at once.
    OptionsDialog(Customer& c, bool onFile, std::optional<CustomerField> focus)
        : c_(c), onFile_(onFile), focus_(focus) {}

protected:
    bool OnInitDialog() override
    {
        MainDialog::OnInitDialog();   // OK and Close pictures
        // No W/H on AR (D15): the account, its dept and its label go.
        for (int id : {kWhAccount, kWhDept}) ShowWindow(GetDlgItem(hwnd(), id), SW_HIDE);
        if (HWND l = findLabel(hwnd(), L"W/H Tax")) ShowWindow(l, SW_HIDE);

        // Each field from its own field (D12; the original's were crossed).
        const std::pair<int, const std::string*> texts[] = {
            {kArAccount, &c_.ar.account},       {kArDept, &c_.ar.dept},       {kSalesAccount, &c_.sales.account},
            {kSalesDept, &c_.sales.dept},       {kVatAccount, &c_.vat.account}, {kVatDept, &c_.vat.dept},
            {kContact, &c_.contact},            {kVin, &c_.vin}};
        for (const auto& [id, text] : texts) SetField(id, TrimText(*text));
        FloatSetDlgItemValue(hwnd(), kVatRate, static_cast<double>(c_.vatRate));
        FloatSetDlgItemValue(hwnd(), kTerms, static_cast<double>(c_.terms));
        SetFocus(GetDlgItem(hwnd(), focus_ ? controlOf(*focus_) : kVatRate));
        return false;   // the focus is set
    }

    bool OnCommand(WORD id, WORD code, HWND ctl) override
    {
        if (code == BN_CLICKED && id == IDOK) {
            Ok();
            return true;
        }
        if (code == BN_CLICKED && id == IDCANCEL) {   // not MainDialog::Cancel: the customer screen's tables stay open
            End(IDCANCEL);
            return true;
        }
        return MainDialog::OnCommand(id, code, ctl);
    }

private:
    static int controlOf(CustomerField f)
    {
        switch (f) {
        case CustomerField::ArAccount: return kArAccount;
        case CustomerField::SalesAccount: return kSalesAccount;
        case CustomerField::VatAccount: return kVatAccount;
        default: return kVatRate;
        }
    }

    int number(int id) const
    {
        double v = 0;
        if (!FloatGetDlgItemValue(hwnd(), id, &v)) return 0;
        return static_cast<int>(std::lround(v));
    }

    void Ok()   // TARVendorOptions::Ok
    {
        Customer c = c_;
        c.ar = {StrTrim(GetField(kArAccount)), StrTrim(GetField(kArDept))};
        c.sales = {StrTrim(GetField(kSalesAccount)), StrTrim(GetField(kSalesDept))};
        c.vat = {StrTrim(GetField(kVatAccount)), StrTrim(GetField(kVatDept))};
        c.contact = StrTrim(GetField(kContact));
        c.vin = StrTrim(GetField(kVin));
        c.vatRate = number(kVatRate);
        c.terms = number(kTerms);
        if (const std::optional<CustomerProblem> p = CheckCustomer(cb, c); p && onOptions(p->field)) {
            Message(problemText(*p, GetString(8101)), MB_ICONEXCLAMATION);
            SetFocus(GetDlgItem(hwnd(), controlOf(p->field)));
            return;
        }
        if (onFile_) {   // as the original: saved at once, after asking
            if (Message(GetString(1093), MB_ICONQUESTION | MB_YESNO) != IDYES) return;
            // Only Options' fields: the main screen's edits wait for its Add.
            Customer saved = *FindCustomer(cb, c.number);
            saved.ar = c.ar;
            saved.sales = c.sales;
            saved.vat = c.vat;
            saved.contact = c.contact;
            saved.vin = c.vin;
            saved.vatRate = c.vatRate;
            saved.terms = c.terms;
            SaveCustomer(cb, saved);
        }
        c_ = c;
        End(IDOK);
    }

    Customer& c_;
    bool onFile_;
    std::optional<CustomerField> focus_;
};

// ---- TARVendorAdd (1052) ---------------------------------------------------------

class CustomersDialog : public MainDialog {
public:
    explicit CustomersDialog(bool closeAtOnce) : closeAtOnce_(closeAtOnce) {}

protected:
    bool OnInitDialog() override
    {
        MainDialog::OnInitDialog();   // Close/Add/New/Top/Bottom/Next/Previous/Delete pictures
        SetBitmap(kOptions);          // TARVendorAdd::SetupWindow: ID_NEW, IDMAINOPTIONS, ID_VIEW, ID_SETDATA
        SetBitmap(kList);
        SetBitmap(kFind);
        SendMessageW(hwnd(), DM_SETDEFID, IDOK, 0);

        EnsureArTables(cb);
        defaults_ = LoadArAccounts(cb);
        table_ = std::make_unique<Data4>(&cb, "ar_cust");   // the original opened "ap_vend" (translation notes)
        table_->SetIndex("vendorno");
        numberField_.init(*table_, "vendorno");
        table_->top();

        TableList::Spec spec;   // TARCustomerList
        spec.section = L"AR_CUST";
        spec.table = "ar_cust";
        spec.tag = "vendorno";
        spec.caption = L"Accounts Receivable Customers";
        spec.headers = {L"Customer", L"Short Name", L"Name"};
        spec.stretchColumn = 3;
        list_.Create(hwnd(), std::move(spec), [this](int row) { Show(list_.RowText(row, 1)); });

        Clear();
        if (closeAtOnce_) PostMessageW(hwnd(), WM_COMMAND, IDCANCEL, 0);
        return false;   // the focus is set
    }

    bool OnDestroy() override
    {
        if (list_.hwnd()) DestroyWindow(list_.hwnd());   // saves its position while the window still exists
        return false;
    }

    bool OnCommand(WORD id, WORD code, HWND ctl) override
    {
        if (code != BN_CLICKED) return MainDialog::OnCommand(id, code, ctl);
        switch (id) {
        case IDOK:
            if (FocusIn(kNumber)) ShowTyped(true);   // Enter on the number
            else Add();
            return true;
        case kFind: ShowTyped(false); return true;
        case ID_DELETE: Delete(); return true;
        case ID_NEW: Clear(); return true;
        case kOptions: Options(); return true;
        case kList: list_.Seek(GetField(kNumber)); return true;
        default:
            if (Navigate(id)) return true;
            return MainDialog::OnCommand(id, code, ctl);   // IDCANCEL -> Cancel()
        }
    }

private:
    bool FocusIn(int id) const
    {
        const HWND control = GetDlgItem(hwnd(), id), focus = GetFocus();
        return focus && (focus == control || GetParent(focus) == control);
    }

    bool Navigate(WORD id)
    {
        Data4& t = *table_;
        switch (id) {
        case ID_TOP: t.top(); break;
        case ID_BOTTOM: t.bottom(); break;
        case ID_NEXT:
            if (t.skip(1) == r4eof || t.eof()) t.bottom();
            break;
        case ID_PREVIOUS:
            if (t.skip(-1) == r4bof || t.bof()) t.top();
            break;
        default: return false;
        }
        if (!t.eof()) Show(TrimText(numberField_.str()));
        return true;
    }

    void ShowTyped(bool enter)   // TARVendorAdd::Find
    {
        const std::string number = StrTrim(GetField(kNumber));
        if (number.empty()) return;
        if (Show(number)) return;
        if (enter) {   // a new customer: the default accounts, and go on typing
            NewCustomer();
            current_.number = number;
            SetFocus(GetDlgItem(hwnd(), kShort));
        } else {
            Message(GetString(6017), MB_ICONINFORMATION);   // "Customer number not found."
        }
    }

    // TARVendorAdd::SetData: false if there is no such customer.
    bool Show(const std::string& number)
    {
        const std::optional<Customer> found = FindCustomer(cb, number);
        if (!found) return false;
        current_ = *found;
        onFile_ = true;
        SeekCode(*table_, numberField_, current_.number);   // Next / Previous go on from here
        const Customer& c = current_;
        const std::pair<int, const std::string*> texts[] = {
            {kNumber, &c.number},         {kShort, &c.shortName},        {kName, &c.name},
            {kAddress1, &c.address1},     {kAddress2, &c.address2},      {kAddress3, &c.address3},
            {kPostal, &c.postal},         {kPhone, &c.phone},            {kFax, &c.fax},
            {kShip1, &c.shipAddress1},    {kShip2, &c.shipAddress2},     {kShip3, &c.shipAddress3},
            {kShipPostal, &c.shipPostal}, {kShipPhone, &c.shipPhone},    {kShipFax, &c.shipFax}};
        for (const auto& [id, text] : texts) SetField(id, *text);
        return true;
    }

    void NewCustomer()   // the default accounts (D12)
    {
        current_ = Customer{};
        current_.ar = defaults_.ar;
        current_.sales = defaults_.sales;
        current_.vat = defaults_.vatPayable;
        onFile_ = false;
    }

    void Clear()   // TARVendorAdd::New
    {
        NewCustomer();
        for (int id : {kNumber, kShort, kName, kAddress1, kAddress2, kAddress3, kPostal, kPhone, kFax, kShip1, kShip2,
                       kShip3, kShipPostal, kShipPhone, kShipFax})
            SetField(id, "");
        SetFocus(GetDlgItem(hwnd(), kNumber));
    }

    // TARVendorAdd::Update: the main screen's fields over the customer kept,
    // which holds Options' fields.
    Customer Read() const
    {
        Customer c = current_;
        c.number = StrTrim(GetField(kNumber));
        c.shortName = StrTrim(GetField(kShort));
        c.name = StrTrim(GetField(kName));
        c.address1 = StrTrim(GetField(kAddress1));
        c.address2 = StrTrim(GetField(kAddress2));
        c.address3 = StrTrim(GetField(kAddress3));
        c.postal = StrTrim(GetField(kPostal));
        c.phone = GetField(kPhone);
        c.fax = GetField(kFax);
        c.shipAddress1 = StrTrim(GetField(kShip1));
        c.shipAddress2 = StrTrim(GetField(kShip2));
        c.shipAddress3 = StrTrim(GetField(kShip3));
        c.shipPostal = StrTrim(GetField(kShipPostal));
        c.shipPhone = GetField(kShipPhone);
        c.shipFax = GetField(kShipFax);
        return c;
    }

    // Typing another number over a customer shown starts a new one, with
    // that customer's options as they were: the same as the original, which
    // added it with the fields of the record it was on.
    bool SameAsShown(const Customer& c) const { return onFile_ && TrimText(current_.number) == c.number; }

    void Options(std::optional<CustomerField> focus = {})   // TARVendorAdd::Options
    {
        Customer c = Read();
        const bool onFile = SameAsShown(c) && FindCustomer(cb, c.number).has_value();
        OptionsDialog dlg(c, onFile, focus);
        if (dlg.DoModal(1063, hwnd()) == IDOK) {
            current_.ar = c.ar;
            current_.sales = c.sales;
            current_.vat = c.vat;
            current_.contact = c.contact;
            current_.vin = c.vin;
            current_.vatRate = c.vatRate;
            current_.terms = c.terms;
        }
    }

    void Add()   // TARVendorAdd::Ok
    {
        const Customer c = Read();
        if (const std::optional<CustomerProblem> p = CheckCustomer(cb, c)) {
            if (onOptions(p->field)) {   // the account fields are on Options: open it there
                Message(problemText(*p, GetString(8101)) + L"\n\nSet it under Options.", MB_ICONEXCLAMATION);
                Options(p->field);
            } else {
                Message(problemText(*p, GetString(8101)), MB_ICONEXCLAMATION);
                SetFocus(GetDlgItem(hwnd(), p->field == CustomerField::Number      ? kNumber
                                                : p->field == CustomerField::ShortName ? kShort
                                                                                        : kName));
            }
            return;
        }
        const bool exists = FindCustomer(cb, c.number).has_value();
        if (exists && Message(GetString(1093), MB_ICONQUESTION | MB_YESNO) != IDYES) return;   // "...modify the entry?"
        SaveCustomer(cb, c);
        list_.Redraw();
        if (exists) Show(c.number);   // modified: keep it shown
        else Clear();                 // added
    }

    void Delete()   // TARVendorAdd::Delete, refusing a customer in use (D12)
    {
        const std::string number = StrTrim(GetField(kNumber));
        if (number.empty()) {
            Message(GetString(6018), MB_ICONINFORMATION);   // "Enter a customer number."
            return;
        }
        if (!FindCustomer(cb, number)) {
            Message(GetString(6017), MB_ICONINFORMATION);
            return;
        }
        if (Message(GetString(6094), MB_ICONQUESTION | MB_YESNO) != IDYES) return;   // "...delete the current entry?"
        std::string usedBy;
        switch (DeleteCustomer(cb, number, &usedBy)) {
        case DeleteCustomerResult::Deleted:
            list_.Redraw();
            table_->top();
            Clear();
            break;
        case DeleteCustomerResult::InUse:
            Message(fx::Format(L"Customer %s can't be deleted: %s is theirs.\n\n"
                               L"A customer stays while their vouchers, open items or receipts are on file.",
                               fx::Utf8ToWide(number).c_str(), fx::Utf8ToWide(usedBy).c_str()),
                    MB_ICONEXCLAMATION);
            break;
        case DeleteCustomerResult::NotFound: break;
        }
    }

    bool closeAtOnce_;
    ArAccounts defaults_;
    std::unique_ptr<Data4> table_;
    Field4 numberField_;
    TableList list_;
    Customer current_;
    bool onFile_ = false;   // current_ is the customer shown, as loaded
};

} // namespace

INT_PTR RunArCustomers(HWND frame, bool closeAtOnce)
{
    CustomersDialog dlg(closeAtOnce);
    return dlg.DoModal(1052, frame);
}

int StressAr(int times)
{
    auto objects = [] {
        HANDLE self = GetCurrentProcess();
        return static_cast<int>(GetGuiResources(self, GR_GDIOBJECTS) + GetGuiResources(self, GR_USEROBJECTS));
    };
    auto openAll = [] { return RunArCustomers(nullptr, true) != -1 && RunArInvoiceEntry(nullptr, true) != -1; };
    if (!openAll()) return -1;   // first open: class registration, cached bitmaps
    const int before = objects();
    for (int i = 0; i < times; ++i)
        if (!openAll()) return -1;
    return objects() - before;
}
