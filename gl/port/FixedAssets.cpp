//
// FixedAssets.cpp - see FixedAssets.h.
//
// Translation notes (plan per-port checklist; decisions are spec #4 §8):
//   - D9: Enter on a typed code shows the asset (TGLFixedAssets::Search was
//     empty), and Enter on a typed type shows its accounts.
//   - The figures are Assets.h's (D3, D4, D6), recomputed as the inputs
//     change. The original computed them only on Add/Mod (UpdateData) and
//     showed accumulated = monthly x months used and "net book value" = cost
//     - salvage - that (spec #4 2.4 #3); the port shows the stored
//     accumulated depreciation and cost - accumulated.
//   - Accumulated depreciation (6040, read-only in the template) can be typed
//     until the run has depreciated the asset (D6): an asset brought in part
//     way through its life. Months used (6032) is fixed from then on too.
//   - CheckInput, never called in the original, is CheckAsset (8101 naming
//     the field); the original's other checks are kept: salvage <= cost
//     (9097), months used <= life (9098). The three account checks (6056-
//     6058) are the type's, made on the Asset Types screen; an asset only
//     names the type, which must exist.
//   - "Full Year" (1036) and the time control (1035) are hidden (D8); the
//     declining rate (7024, 7025) shows only for Declining, as in the
//     original.
//   - A disposed asset (spec #4 D7) can't be changed: Add/Mod says so, and
//     the caption shows the date sold. (Disposal comes in S4-M4.)
//   - The asset list (TAssetList) showed FACODE, DEP_METH and REFERENCE
//     under "Code" / "Method" / "Description"; the port shows DESCRIPT under
//     "Description" (fields 1, 2, 4). Both lists start hidden and open on
//     their buttons, as the original's did once picked from.
//   - Next / Previous stop at the last / first record, as on the Spec #2
//     screens.
//
#include "FixedAssets.h"

#include "Accounts.h"   // TrimText
#include "Assets.h"
#include "DepreciationRun.h"
#include "FixedAssetTypes.h"
#include "Globals.h"
#include "MainDialog.h"
#include "MasterTables.h"   // SeekCode
#include "TableList.h"

#include "report/Format.h"   // rpt::ToCents, FormatDate

#include "cb4sqlite.h"

#include <iterator>
#include <memory>
#include <optional>
#include <string>
#include <utility>

#include "LegacyIds.h"   // last: see LegacyIds.h

namespace {

constexpr int kCode = 6019;        // ID_FACODE
constexpr int kDate = 3035;        // ID_DATE
constexpr int kTime = 1035;        // a tbTime, never set
constexpr int kMethod = 1034;      // ID_DEPRECIATION
constexpr int kFullYear = 1036;    // ID_FULLYEAR
constexpr int kDescript = 2015;    // ID_DESCRIPT
constexpr int kReference = 3036;   // ID_REFERENCE
constexpr int kType = 6021;        // ID_ASSET_TYPE
constexpr int kCost = 6031;        // ID_ASSET_COST
constexpr int kSalvage = 6036;     // ID_SALVAGE
constexpr int kAccum = 6040;
constexpr int kBookValue = 6041;
constexpr int kLife = 6022;        // ID_LIFETIME (months)
constexpr int kYears = 6033;       // ID_YEARS_LIFE
constexpr int kUsed = 6032;        // ID_LIFE_RMD: months used
constexpr int kRate = 7024;        // ID_DECLINING
constexpr int kRateLabel = 7025;   // "Declining Average"
constexpr int kMonthly = 6037;     // ID_MONTHLY_DEP
constexpr int kAssetsButton = 1074;   // ID_SLIPLIST: "Assets"
constexpr int kTypesButton = 1103;    // ID_FA: "Types"

// The type's accounts, read-only: account, dept, description.
constexpr int kAccountFields[3][3] = {{6023, 6024, 1078}, {6025, 6026, 6027}, {1076, 6030, 6028}};

class FixedAssetsDialog : public MainDialog {
public:
    explicit FixedAssetsDialog(bool closeAtOnce) : closeAtOnce_(closeAtOnce) {}

protected:
    bool OnInitDialog() override
    {
        MainDialog::OnInitDialog();
        SetBitmap(kTypesButton);
        SetBitmap(kAssetsButton);
        EnsureAssetTables(cb);
        yearEnd_ = FiscalYearEnd(cb);
        table_ = std::make_unique<Data4>(&cb, "gl_fa");
        table_->SetIndex("facode");
        codeField_.init(*table_, "facode");
        table_->top();

        SendDlgItemMessageW(hwnd(), kMethod, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Straight"));
        SendDlgItemMessageW(hwnd(), kMethod, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Declining"));
        SendDlgItemMessageW(hwnd(), kMethod, CB_SETCURSEL, 0, 0);
        for (int id : {kFullYear, kTime}) ShowWindow(GetDlgItem(hwnd(), id), SW_HIDE);   // D8
        ShowRate(false);
        GetWindowTextW(hwnd(), caption_, static_cast<int>(std::size(caption_)));

        TableList::Spec assets;
        assets.section = L"GL_FA";
        assets.table = "gl_fa";
        assets.tag = "facode";
        assets.caption = GetString(6059);                                         // "General Ledger Fixed Assets"
        assets.headers = {GetString(6060), GetString(6093), GetString(6044)};   // Code, Method, Description
        assets.fields = {1, 2, 4};                                              // FACODE, DEP_METH, DESCRIPT
        assetList_.Create(hwnd(), std::move(assets), [this](int row) { Show(assetList_.RowText(row, 1)); }, true,
                          false);
        TableList::Spec types;
        types.section = L"GL_FATYP";
        types.table = "gl_fatyp";
        types.tag = "fatype";
        types.caption = GetString(6061);                       // "General Ledger Fixed Asset Types"
        types.headers = {GetString(6060), GetString(6044)};   // Code, Description
        types.stretchColumn = 2;
        typeList_.Create(hwnd(), std::move(types), [this](int row) { ShowType(typeList_.RowText(row, 1)); }, true,
                         false);

        Clear();
        if (closeAtOnce_) PostMessageW(hwnd(), WM_COMMAND, IDCANCEL, 0);
        return false;   // Clear set the focus
    }

    bool OnDestroy() override
    {
        for (TableList* l : {&assetList_, &typeList_})
            if (l->hwnd()) DestroyWindow(l->hwnd());   // saves its position while the window still exists
        return false;
    }

    bool OnCommand(WORD id, WORD code, HWND ctl) override
    {
        if (id == kMethod && code == CBN_SELCHANGE) {
            ShowRate(Method() == DepMethod::Declining);
            Recompute();
            return true;
        }
        if (code == EN_CHANGE && IsInput(id)) {
            Recompute();
            return false;
        }
        if (id == kType && code == EN_KILLFOCUS) {
            ShowType(GetField(kType));
            return false;
        }
        if (code != BN_CLICKED) return MainDialog::OnCommand(id, code, ctl);
        switch (id) {
        case IDOK:
            if (FocusIn(kCode)) ShowTyped();   // Enter on the code (D9)
            else if (FocusIn(kType)) NextField(kType);
            else Add();
            return true;
        case ID_NEW: Clear(); return true;
        case kAssetsButton: assetList_.Seek(GetField(kCode)); return true;   // TGLFixedAssets::AssetList
        case kTypesButton: typeList_.Seek(GetField(kType)); return true;     // TGLFixedAssets::Types
        default:
            if (Navigate(id)) return true;
            return MainDialog::OnCommand(id, code, ctl);   // IDCANCEL -> Cancel()
        }
    }

private:
    // ---- controls ---------------------------------------------------------

    bool FocusIn(int id) const
    {
        const HWND control = GetDlgItem(hwnd(), id), focus = GetFocus();
        return focus && (focus == control || GetParent(focus) == control);
    }

    void NextField(int id)
    {
        SendMessageW(hwnd(), WM_NEXTDLGCTL, reinterpret_cast<WPARAM>(GetDlgItem(hwnd(), id)), FALSE);   // tab on
        SendMessageW(hwnd(), WM_NEXTDLGCTL, 0, FALSE);
    }

    static bool IsInput(WORD id)
    {
        return id == kCost || id == kSalvage || id == kLife || id == kUsed || id == kRate || id == kAccum;
    }

    void ShowRate(bool on)
    {
        for (int id : {kRate, kRateLabel}) ShowWindow(GetDlgItem(hwnd(), id), on ? SW_SHOW : SW_HIDE);
    }

    void SetReadOnly(int id, bool on) { SendDlgItemMessageW(hwnd(), id, EM_SETREADONLY, on ? TRUE : FALSE, 0); }

    std::int64_t Cents(int id) const
    {
        double v = 0;
        FloatGetDlgItemValue(hwnd(), id, &v);
        return rpt::ToCents(v);
    }
    void SetCents(int id, std::int64_t cents)
    {
        updating_ = true;
        FloatSetDlgItemValue(hwnd(), id, static_cast<double>(cents) / 100.0);
        updating_ = false;
    }
    int Int(int id) const
    {
        LONG v = 0;
        IntGetDlgItemValue(hwnd(), id, &v);
        return static_cast<int>(v);
    }

    DepMethod Method() const
    {
        return SendDlgItemMessageW(hwnd(), kMethod, CB_GETCURSEL, 0, 0) == 1 ? DepMethod::Declining : DepMethod::Straight;
    }

    // ---- the record -------------------------------------------------------

    Asset Input() const
    {
        Asset a;
        a.code = StrTrim(GetField(kCode));
        a.type = StrTrim(GetField(kType));
        a.description = StrTrim(GetField(kDescript));
        a.reference = StrTrim(GetField(kReference));
        a.datePurchased = GetDate(kDate);
        a.method = Method();
        a.cost = Cents(kCost);
        a.salvage = Cents(kSalvage);
        a.accumulated = Cents(kAccum);
        a.life = Int(kLife);
        a.monthsUsed = Int(kUsed);
        double rate = 0;
        FloatGetDlgItemValue(hwnd(), kRate, &rate);
        a.percent = static_cast<int>(rate + 0.5);
        return a;
    }

    // The start-of-year book value for the declining method: from the
    // history for a depreciated asset, else cost less what is typed.
    std::int64_t StartOfYear(const Asset& a) const
    {
        return history_ ? StartOfYearBookValue(cb, a, yearEnd_) : a.cost - a.accumulated;
    }

    void Recompute()
    {
        if (updating_) return;
        const Asset a = Input();
        SetCents(kMonthly, MonthlyAmount(a, StartOfYear(a)));
        SetCents(kBookValue, BookValue(a));
        updating_ = true;
        FloatSetDlgItemValue(hwnd(), kYears, a.life / 12.0);
        updating_ = false;
    }

    void ShowTyped()
    {
        const std::string code = StrTrim(GetField(kCode));
        if (code.empty()) return;
        if (!Show(code)) NextField(kCode);   // a new asset: go on typing
    }

    // TGLFixedAssets::SetData: false if there is no such asset.
    bool Show(const std::string& code)
    {
        const std::optional<Asset> a = FindAsset(cb, code);
        if (!a) return false;
        SeekCode(*table_, codeField_, a->code);   // Next / Previous go on from here
        history_ = HasDepreciation(cb, a->code);
        updating_ = true;
        SetField(kCode, a->code);
        SetField(kDate, a->datePurchased);
        SetField(kDescript, a->description);
        SetField(kReference, a->reference);
        SendDlgItemMessageW(hwnd(), kMethod, CB_SETCURSEL, a->method == DepMethod::Declining ? 1 : 0, 0);
        ShowRate(a->method == DepMethod::Declining);
        FloatSetDlgItemValue(hwnd(), kCost, a->cost / 100.0);
        FloatSetDlgItemValue(hwnd(), kSalvage, a->salvage / 100.0);
        FloatSetDlgItemValue(hwnd(), kAccum, a->accumulated / 100.0);
        FloatSetDlgItemValue(hwnd(), kRate, a->percent);
        IntSetDlgItemValue(hwnd(), kLife, a->life);
        IntSetDlgItemValue(hwnd(), kUsed, a->monthsUsed);
        updating_ = false;
        ShowType(a->type);
        SetReadOnly(kAccum, history_);   // D6
        SetReadOnly(kUsed, history_);
        disposed_ = a->disposed() ? a->dateSold : std::string();
        SetCaption();
        Recompute();
        return true;
    }

    // TGLFixedAssets::UpdateType: the type and its accounts; blank accounts
    // for a type that isn't there.
    void ShowType(const std::string& type)
    {
        const std::optional<AssetType> t = FindAssetType(cb, type);
        if (t) SetField(kType, t->code);
        const AssetAccount* acct[3] = {t ? &t->asset : nullptr, t ? &t->expense : nullptr, t ? &t->accum : nullptr};
        for (int i = 0; i < 3; ++i) {
            SetField(kAccountFields[i][0], acct[i] ? acct[i]->account : std::string());
            SetField(kAccountFields[i][1], acct[i] ? acct[i]->dept : std::string());
            SetField(kAccountFields[i][2], acct[i] ? acct[i]->descript : std::string());
        }
    }

    void SetCaption()
    {
        std::wstring c = caption_;
        if (!disposed_.empty()) c += L" - disposed " + fx::Utf8ToWide(rpt::FormatDate(disposed_));
        SetWindowTextW(hwnd(), c.c_str());
    }

    void Clear()   // TGLFixedAssets::New
    {
        updating_ = true;
        for (int id : {kCode, kDate, kDescript, kReference, kType, kCost, kSalvage, kAccum, kBookValue, kLife, kYears,
                       kUsed, kRate, kMonthly})
            SetField(id, "");
        updating_ = false;
        ShowType("");
        SendDlgItemMessageW(hwnd(), kMethod, CB_SETCURSEL, 0, 0);
        ShowRate(false);
        history_ = false;
        SetReadOnly(kAccum, false);
        SetReadOnly(kUsed, false);
        disposed_.clear();
        SetCaption();
        SetFocus(GetDlgItem(hwnd(), kCode));
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
        if (!t.eof()) Show(TrimText(codeField_.str()));
        return true;
    }

    // ---- Add/Mod ----------------------------------------------------------

    static int ControlOf(AssetField f)
    {
        switch (f) {
        case AssetField::Code: return kCode;
        case AssetField::Type: return kType;
        case AssetField::Description: return kDescript;
        case AssetField::Date: return kDate;
        case AssetField::Cost: return kCost;
        case AssetField::Salvage: return kSalvage;
        case AssetField::Life: return kLife;
        case AssetField::MonthsUsed: return kUsed;
        case AssetField::Rate: return kRate;
        case AssetField::Accumulated: return kAccum;
        default: return kCode;
        }
    }

    static const wchar_t* NameOf(AssetField f)
    {
        switch (f) {
        case AssetField::Code: return L"Fixed Asset Code";
        case AssetField::Type: return L"Fixed Asset Type";
        case AssetField::Description: return L"Description";
        case AssetField::Date: return L"Date Purchased";
        case AssetField::Cost: return L"Asset Cost";
        case AssetField::Life: return L"Life of Assets (Months)";
        case AssetField::Rate: return L"Declining Average";
        default: return L"";
        }
    }

    void Report(const AssetProblem& p, const Asset& a)
    {
        std::wstring text;
        switch (p.kind) {
        case AssetProblem::Kind::Required: text = fx::Format(GetString(8101).c_str(), NameOf(p.field)); break;
        case AssetProblem::Kind::NoSuchType:
            text = fx::Format(L"There is no fixed asset type %s.\n\nChoose one from the Types list.",
                              fx::Utf8ToWide(a.type).c_str());
            break;
        case AssetProblem::Kind::SalvageOverCost: text = GetString(9097); break;
        case AssetProblem::Kind::UsedOverLife: text = GetString(9098); break;
        case AssetProblem::Kind::AccumOverDepreciable:
            text = L"The accumulated depreciation can't be more than the cost less the salvage value.";
            break;
        }
        Message(text, MB_ICONINFORMATION);
        SetFocus(GetDlgItem(hwnd(), ControlOf(p.field)));
    }

    void Add()   // TGLFixedAssets::Ok / Update
    {
        Asset a = Input();
        const std::optional<Asset> existing = FindAsset(cb, a.code);
        if (existing && existing->disposed()) {
            Message(fx::Format(L"Asset %s was disposed of on %s. It can't be changed.",
                               fx::Utf8ToWide(existing->code).c_str(),
                               fx::Utf8ToWide(rpt::FormatDate(existing->dateSold)).c_str()),
                    MB_ICONINFORMATION);
            return;
        }
        if (const std::optional<AssetProblem> p = CheckAsset(cb, a)) {
            Report(*p, a);
            return;
        }
        if (existing) {
            a.mode = existing->mode;
            a.dateSold = existing->dateSold;
            if (HasDepreciation(cb, a.code)) {   // D6: the run's figures stand
                a.accumulated = existing->accumulated;
                a.monthsUsed = existing->monthsUsed;
            }
            // "The current asset has already been added. Do you want to modify the entry."
            if (Message(GetString(8102), MB_ICONQUESTION | MB_YESNO) != IDYES) return;
        }
        a.monthly = MonthlyAmount(a, existing && HasDepreciation(cb, a.code) ? StartOfYearBookValue(cb, a, yearEnd_)
                                                                             : a.cost - a.accumulated);
        SaveAsset(cb, a);
        assetList_.Redraw();
        if (existing) Show(a.code);   // TGLFixedAssets::Update showed the record again
        else Clear();                 // TGLFixedAssets::Ok started a new one
    }

    bool closeAtOnce_;
    std::unique_ptr<Data4> table_;
    Field4 codeField_;
    TableList assetList_, typeList_;
    std::string yearEnd_;
    std::string disposed_;   // the date sold of the asset shown, if disposed
    bool history_ = false;   // the asset shown has been depreciated by a run
    bool updating_ = false;  // filling the fields: no recomputing
    wchar_t caption_[128] = {};
};

} // namespace

INT_PTR RunFixedAssets(HWND frame, bool closeAtOnce)
{
    FixedAssetsDialog dlg(closeAtOnce);
    return dlg.DoModal(4004, frame);
}

int StressFixedAssets(int times)
{
    auto objects = [] {
        HANDLE self = GetCurrentProcess();
        return static_cast<int>(GetGuiResources(self, GR_GDIOBJECTS) + GetGuiResources(self, GR_USEROBJECTS));
    };
    auto openAll = [] {
        return RunAssetTypes(nullptr, true) != -1 && RunFixedAssets(nullptr, true) != -1 &&
               RunCreateDepreciationVoucher(nullptr, true) != -1;
    };
    if (!openAll()) return -1;   // first open: class registration, cached bitmaps
    const int before = objects();
    for (int i = 0; i < times; ++i)
        if (!openAll()) return -1;
    return objects() - before;
}
