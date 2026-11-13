//
// ApVendors.cpp - see ApVendors.h.
//
// Translation notes (plan per-port checklist; decisions are spec #5 §8):
//   - D12: Set Data seeked a boolean (`seek(GetField(ID_VENDORNO) == 0)`), so
//     it never found the typed vendor; Set Data, and Enter in the number,
//     show it. New vendors default the AP account to GL_SETAT AP and the VAT
//     account to VATR (the add-vendor popup of invoice entry used VATP).
//   - D12: Delete deleted the current record, whatever was shown, and left
//     its vouchers pointing at nothing; it now deletes the vendor shown,
//     refusing one with vouchers, open items or cheques (Ap.h DeleteVendor).
//   - TAPVendorAdd::Ok checked only the number and names, and kept the last
//     type when neither radio button was set; the port also asks for the
//     type, checks the rates, and checks the accounts against the chart
//     (Ap.h CheckVendor). A VAT rate needs a VAT account.
//   - The template's labels don't match its fields: 4009 (ID_VENDORCONTACT)
//     is labelled "TIN No."; 1039 (the VIN) "VAT No."; 5111, beside "VIN
//     No.", is read by nothing. The port labels 4009 "Contact", keeps "VAT
//     No." on the VIN, and hides 5111 and its label. ID_VENDORTEL (4008) and
//     ID_VENDORSTATUS (4011) aren't in the template; STATUS is kept as stored.
//   - Pictures: "DEFAULT" (New) and "LOOK" (Search) have no modern icon; the
//     port uses New's (4057) and the Chart of Accounts' Search (4054), so no
//     icon is added. Set Data has its own (4076).
//   - The template gives Delete the default-button style (32785) as well as
//     Add, so the dialog took Delete as its default and Enter asked to
//     delete the vendor. Add (IDOK) is made the default, as on the other
//     screens, so Enter in the number shows the vendor and elsewhere adds.
//   - Next / Previous stop at the last / first vendor, as on the Spec #2
//     screens. Modify keeps the vendor shown; adding clears, as the original.
//
#include "ApVendors.h"

#include "Accounts.h"   // TrimText
#include "Ap.h"
#include "ApInvoiceEntry.h"
#include "ApPosting.h"
#include "ApTransfer.h"
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

// Dialog 1041's controls (gl/header/ap.h names).
constexpr int kNumber = 4001;     // ID_VENDORNO
constexpr int kShort = 4002;      // ID_VENDORSHORT
constexpr int kName = 4003;       // ID_VENDORNAME
constexpr int kAddress1 = 4004;   // ID_VENDORADDRESS1 ("Street")
constexpr int kAddress2 = 4005;   // ("City")
constexpr int kAddress3 = 4006;   // ("State/Country")
constexpr int kPostal = 4007;     // ID_VENDORPOST
constexpr int kContact = 4009;    // ID_VENDORCONTACT (labelled "TIN No." in the template)
constexpr int kPhone = 6069;      // ID_PHONE
constexpr int kFax = 6068;        // ID_FAX
constexpr int kMedrep = 1097;     // ID_MEDREP
constexpr int kVin = 1039;        // ID_VIN_NUM ("VAT No.")
constexpr int kUnused = 5111;     // beside "VIN No.": read by nothing
constexpr int kIndividual = 4079; // ID_INDIVIDUAL
constexpr int kCompany = 4078;    // ID_COMPANY
constexpr int kVatRate = 4065;    // ID_VATPERCENTAGE
constexpr int kTerms = 6086;      // ID_TERMS
constexpr int kWhRate = 4073;     // ID_REBATE
constexpr int kApAccount = 1087;  // ID_APCONTROL
constexpr int kApDept = 1086;     // ID_APDEPART
constexpr int kVatAccount = 8081; // ID_VATACCT
constexpr int kVatDept = 8082;    // ID_VATDEPT
constexpr int kDiscDays = 1037;   // ID_DIS_DAYS
constexpr int kDiscPercent = 1038;   // ID_DIS_PERC
constexpr int kNew = 4082;        // CM_NEW (this template's New; ID_NEW is 4057 elsewhere)
constexpr int kSearch = 3043;     // ID_SEARCHCUST
constexpr int kSetData = 4076;    // ID_SETDATA

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

class VendorsDialog : public MainDialog {
public:
    explicit VendorsDialog(bool closeAtOnce) : closeAtOnce_(closeAtOnce) {}

protected:
    bool OnInitDialog() override
    {
        MainDialog::OnInitDialog();   // Close/Add/Top/Bottom/Next/Previous/Delete pictures
        auto picture = [this](int button, int resource) {
            SendDlgItemMessageW(hwnd(), button, SBM_SETPICT, SUPERBTN_PICT_BITMAP, static_cast<LPARAM>(resource));
        };
        picture(kNew, ID_NEW);   // "DEFAULT" in the original
        picture(kSearch, 4054);  // "LOOK"
        SetBitmap(kSetData);

        // The template's labels (translation notes).
        if (HWND l = findLabel(hwnd(), L"TIN No.")) SetWindowTextW(l, L"Contact");
        if (HWND l = findLabel(hwnd(), L"VIN No.")) ShowWindow(l, SW_HIDE);
        ShowWindow(GetDlgItem(hwnd(), kUnused), SW_HIDE);
        // Delete has the default-button style too (32785, as Add), so Enter
        // sent Delete: Add is the default (translation notes).
        SendDlgItemMessageW(hwnd(), ID_DELETE, BM_SETSTYLE, BS_PUSHBUTTON, TRUE);
        SendMessageW(hwnd(), DM_SETDEFID, IDOK, 0);

        EnsureApTables(cb);
        defaults_ = LoadApAccounts(cb);
        table_ = std::make_unique<Data4>(&cb, "ap_vend");
        table_->SetIndex("vendorno");
        numberField_.init(*table_, "vendorno");
        table_->top();

        TableList::Spec spec;   // TCustomerList on ap_vend
        spec.section = L"AP_VEND";
        spec.table = "ap_vend";
        spec.tag = "vendorno";
        spec.caption = L"Accounts Payable Vendors";
        spec.headers = {L"Vendor", L"Short Name", L"Name"};
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
            if (FocusIn(kNumber)) ShowTyped();   // Enter on the number (D12)
            else Add();
            return true;
        case kSetData: ShowTyped(); return true;
        case ID_DELETE: Delete(); return true;
        case kNew: Clear(); return true;
        case kSearch: list_.Seek(GetField(kNumber)); return true;
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

    void ShowTyped()   // Set Data (D12)
    {
        const std::string number = StrTrim(GetField(kNumber));
        if (number.empty()) return;
        if (Show(number)) return;
        if (FocusIn(kNumber)) {
            SetFocus(GetDlgItem(hwnd(), kShort));   // a new vendor: go on typing
        } else {
            Message(fx::Format(L"There is no vendor %s.", fx::Utf8ToWide(number).c_str()), MB_ICONINFORMATION);
        }
    }

    void SetNumber(int id, int value) { FloatSetDlgItemValue(hwnd(), id, static_cast<double>(value)); }

    int GetNumber(int id) const
    {
        double v = 0;
        if (!FloatGetDlgItemValue(hwnd(), id, &v)) return 0;
        return static_cast<int>(std::lround(v));
    }

    // TAPVendorAdd::SetData: false if there is no such vendor.
    bool Show(const std::string& number)
    {
        const std::optional<Vendor> found = FindVendor(cb, number);
        if (!found) return false;
        const Vendor& v = *found;
        SeekCode(*table_, numberField_, v.number);   // Next / Previous go on from here
        shown_ = v;
        const std::pair<int, const std::string*> texts[] = {
            {kNumber, &v.number},     {kShort, &v.shortName},  {kName, &v.name},       {kAddress1, &v.address1},
            {kAddress2, &v.address2}, {kAddress3, &v.address3}, {kPostal, &v.postal},  {kContact, &v.contact},
            {kPhone, &v.phone},       {kFax, &v.fax},          {kMedrep, &v.medrep},   {kVin, &v.vin},
            {kApAccount, &v.ap.account}, {kApDept, &v.ap.dept}, {kVatAccount, &v.vat.account},
            {kVatDept, &v.vat.dept}};
        for (const auto& [id, text] : texts) SetField(id, *text);
        SetNumber(kVatRate, v.vatRate);
        SetNumber(kWhRate, v.whRate);
        SetNumber(kTerms, v.terms);
        SetNumber(kDiscDays, v.discountDays);
        SetNumber(kDiscPercent, v.discountPercent);
        SetCheck(kCompany, v.type == "C");
        SetCheck(kIndividual, v.type == "I");
        return true;
    }

    void Clear()   // TAPVendorAdd::New, with the default accounts (D12)
    {
        shown_.reset();
        for (int id : {kNumber, kShort, kName, kAddress1, kAddress2, kAddress3, kPostal, kContact, kPhone, kFax,
                       kMedrep, kVin, kVatRate, kWhRate, kTerms, kDiscDays, kDiscPercent})
            SetField(id, "");
        SetField(kApAccount, defaults_.ap.account);
        SetField(kApDept, defaults_.ap.dept);
        SetField(kVatAccount, defaults_.vatInput.account);
        SetField(kVatDept, defaults_.vatInput.dept);
        SetCheck(kCompany, false);
        SetCheck(kIndividual, false);
        SetFocus(GetDlgItem(hwnd(), kNumber));
    }

    Vendor Read() const   // TAPVendorAdd::AddLine
    {
        // Fields the screen doesn't show are kept from the vendor shown.
        Vendor v = shown_ && TrimText(shown_->number) == StrTrim(GetField(kNumber)) ? *shown_ : Vendor{};
        v.number = StrTrim(GetField(kNumber));
        v.shortName = StrTrim(GetField(kShort));
        v.name = StrTrim(GetField(kName));
        v.address1 = StrTrim(GetField(kAddress1));
        v.address2 = StrTrim(GetField(kAddress2));
        v.address3 = StrTrim(GetField(kAddress3));
        v.postal = StrTrim(GetField(kPostal));
        v.contact = StrTrim(GetField(kContact));
        v.phone = GetField(kPhone);
        v.fax = GetField(kFax);
        v.medrep = StrTrim(GetField(kMedrep));
        v.vin = StrTrim(GetField(kVin));
        v.ap = {StrTrim(GetField(kApAccount)), StrTrim(GetField(kApDept))};
        v.vat = {StrTrim(GetField(kVatAccount)), StrTrim(GetField(kVatDept))};
        v.vatRate = GetNumber(kVatRate);
        v.whRate = GetNumber(kWhRate);
        v.terms = GetNumber(kTerms);
        v.discountDays = GetNumber(kDiscDays);
        v.discountPercent = GetNumber(kDiscPercent);
        v.type = IsChecked(kIndividual) ? "I" : IsChecked(kCompany) ? "C" : "";
        return v;
    }

    void Refuse(const VendorProblem& p)
    {
        static const std::pair<VendorField, std::pair<int, const wchar_t*>> fields[] = {
            {VendorField::Number, {kNumber, L"Vendor Number"}}, {VendorField::ShortName, {kShort, L"Short Name"}},
            {VendorField::Name, {kName, L"Vendor Name"}},       {VendorField::Type, {kCompany, L"Individual / Company"}},
            {VendorField::VatRate, {kVatRate, L"VAT %"}},       {VendorField::WhRate, {kWhRate, L"W/H Tax percentage"}},
            {VendorField::ApAccount, {kApAccount, L"AP Acct"}}, {VendorField::VatAccount, {kVatAccount, L"VAT Acct"}}};
        int control = kNumber;
        const wchar_t* name = L"";
        for (const auto& [f, c] : fields)
            if (f == p.field) {
                control = c.first;
                name = c.second;
            }
        std::wstring text;
        switch (p.kind) {
        case VendorProblem::Kind::Required: text = fx::Format(GetString(8101).c_str(), name); break;
        case VendorProblem::Kind::NoType: text = L"Choose Individual or Company: the withholding tax account depends on it."; break;
        case VendorProblem::Kind::BadRate: text = fx::Format(L"%s must be a whole percentage from 0 to 99.", name); break;
        case VendorProblem::Kind::BadAccount:
            text = fx::Format(L"The %s isn't in the chart of accounts.", p.field == VendorField::ApAccount
                                                                            ? L"AP account"
                                                                            : L"VAT account");
            break;
        case VendorProblem::Kind::VatAccountNeeded:
            text = L"A vendor with a VAT rate needs a VAT account: invoices book the VAT to it.";
            break;
        }
        Message(text, MB_ICONEXCLAMATION);
        SetFocus(GetDlgItem(hwnd(), control));
    }

    void Add()   // TAPVendorAdd::Ok
    {
        const Vendor v = Read();
        if (const std::optional<VendorProblem> p = CheckVendor(cb, v)) {
            Refuse(*p);
            return;
        }
        const bool exists = FindVendor(cb, v.number).has_value();
        if (exists && Message(GetString(6037), MB_ICONQUESTION | MB_YESNO) != IDYES) return;   // "...update it?"
        SaveVendor(cb, v);
        list_.Redraw();
        if (exists) Show(v.number);   // modified: keep it shown
        else Clear();                 // added: CM_NEW, as the original
    }

    void Delete()   // TAPVendorAdd::Delete, refusing a vendor in use (D12)
    {
        const std::string number = StrTrim(GetField(kNumber));
        if (number.empty()) return;
        if (!FindVendor(cb, number)) {
            Message(fx::Format(L"There is no vendor %s.", fx::Utf8ToWide(number).c_str()), MB_ICONINFORMATION);
            return;
        }
        if (Message(GetString(6034), MB_ICONQUESTION | MB_YESNO) != IDYES) return;   // "...delete this vendor?"
        std::string usedBy;
        switch (DeleteVendor(cb, number, &usedBy)) {
        case DeleteVendorResult::Deleted:
            list_.Redraw();
            table_->top();
            Clear();
            break;
        case DeleteVendorResult::InUse:
            Message(fx::Format(L"Vendor %s can't be deleted: %s is theirs.\n\n"
                               L"A vendor stays while their vouchers, open items or cheques are on file.",
                               fx::Utf8ToWide(number).c_str(), fx::Utf8ToWide(usedBy).c_str()),
                    MB_ICONEXCLAMATION);
            break;
        case DeleteVendorResult::NotFound: break;
        }
    }

    bool closeAtOnce_;
    ApAccounts defaults_;
    std::unique_ptr<Data4> table_;
    Field4 numberField_;
    TableList list_;
    std::optional<Vendor> shown_;
};

} // namespace

INT_PTR RunApVendors(HWND frame, bool closeAtOnce)
{
    VendorsDialog dlg(closeAtOnce);
    return dlg.DoModal(1041, frame);
}

int StressAp(int times)
{
    auto objects = [] {
        HANDLE self = GetCurrentProcess();
        return static_cast<int>(GetGuiResources(self, GR_GDIOBJECTS) + GetGuiResources(self, GR_USEROBJECTS));
    };
    auto openAll = [] {
        return RunApVendors(nullptr, true) != -1 && RunApInvoiceEntry(nullptr, true) != -1 &&
               RunApPost(nullptr, ApKind::Invoice, true) != -1 && RunApCancel(nullptr, ApKind::Invoice, true) != -1 &&
               RunApRecover(nullptr, ApKind::Invoice, true) != -1 && RunApTransfer(nullptr, true) != -1 &&
               RunApCashEntry(nullptr, true) != -1 && RunApPost(nullptr, ApKind::Cash, true) != -1 &&
               RunApCancel(nullptr, ApKind::Cash, true) != -1;
    };
    if (!openAll()) return -1;   // first open: class registration, cached bitmaps
    const int before = objects();
    for (int i = 0; i < times; ++i)
        if (!openAll()) return -1;
    return objects() - before;
}
