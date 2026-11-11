//
// FixedAssetDisposal.cpp - see FixedAssetDisposal.h.
//
// Translation notes (plan per-port checklist; decisions are spec #4 §8):
//   - D7: the original's Delete (IDOK, string 8107) deleted and packed the
//     current record. The port's button is "Dispose": it sets DATE_SOLD and
//     MODE "D" (Assets.h DisposeAsset) and keeps the asset, then gives the
//     entry to make in Voucher Entry. A separate Delete (added in code,
//     IDC_FA_DELETE) removes an asset entered by mistake, only if the run has
//     never depreciated it.
//   - The template's "Disposal Data" fields reused Add/Modify's IDs (spec #4
//     2.4 #5): "Sale Price" (6031) showed the cost and "Gain/Loss on Sale"
//     (6037) the monthly depreciation. Here 6031 is the sale price, typed
//     (made editable in code), and 6037 the gain or loss on it (sale price -
//     book value). 6022 and 6032 are the life and the months used ("Rmd" is
//     relabelled "Used"), 6036 the salvage value.
//   - The cost, accumulated depreciation and book value, which the template
//     has no fields for, are on a line added under the box
//     (IDC_FA_SUMMARY); the dialog is made taller for it and for Delete.
//   - Date Disposed (1075, never written by the original) defaults to today.
//   - D9: the code (6019, read-only in the template) can be typed; Enter
//     shows the asset (TGLFixedAssets::Search was empty). Assets shows the
//     list; Top / Bottom / Next / Previous as on the Spec #2 screens.
//   - A disposed asset shows its date sold in the caption and can't be
//     disposed of again.
//
#include "FixedAssetDisposal.h"

#include "Accounts.h"   // TrimText
#include "Assets.h"
#include "Globals.h"
#include "MainDialog.h"
#include "MasterTables.h"   // SeekCode
#include "PortIds.h"
#include "TableList.h"

#include "report/Format.h"   // rpt::ToCents, FormatAmount, FormatDate

#include "cb4sqlite.h"

#include <cstdio>
#include <cwchar>
#include <iterator>
#include <memory>
#include <optional>
#include <string>
#include <utility>

#include "LegacyIds.h"   // last: see LegacyIds.h

namespace {

constexpr int kCode = 6019;          // ID_FACODE
constexpr int kDatePurchased = 3035; // ID_DATE
constexpr int kDateDisposed = 1075;
constexpr int kMethod = 1034;        // ID_DEPRECIATION
constexpr int kDescript = 2015;      // ID_DESCRIPT
constexpr int kReference = 3036;     // ID_REFERENCE
constexpr int kSalePrice = 6031;     // "Sale Price" (ID_ASSET_COST in Add/Modify)
constexpr int kLife = 6022;          // "Life (Months)"
constexpr int kUsed = 6032;          // "Rmd": months used
constexpr int kSalvage = 6036;       // "Salvage Amount"
constexpr int kGainLoss = 6037;      // "Gain/Loss on Sale"
constexpr int kAssetsButton = 1074;  // ID_SLIPLIST: "Assets"

std::wstring money(std::int64_t cents) { return fx::Utf8ToWide(rpt::FormatAmount(cents)); }
std::wstring wide(const std::string& s) { return fx::Utf8ToWide(TrimText(s)); }

std::string today()
{
    SYSTEMTIME t{};
    GetLocalTime(&t);
    char buf[16] = {};
    std::snprintf(buf, sizeof buf, "%04u%02u%02u", t.wYear, t.wMonth, t.wDay);
    return buf;
}

class DisposalDialog : public MainDialog {
public:
    explicit DisposalDialog(bool closeAtOnce) : closeAtOnce_(closeAtOnce) {}

protected:
    bool OnInitDialog() override
    {
        MainDialog::OnInitDialog();
        SetBitmap(kAssetsButton);
        EnsureAssetTables(cb);
        table_ = std::make_unique<Data4>(&cb, "gl_fa");
        table_->SetIndex("facode");
        codeField_.init(*table_, "facode");
        table_->top();
        GetWindowTextW(hwnd(), caption_, static_cast<int>(std::size(caption_)));

        SetDlgItemTextW(hwnd(), IDOK, L"&Dispose");
        SendDlgItemMessageW(hwnd(), kCode, EM_SETREADONLY, FALSE, 0);
        SendDlgItemMessageW(hwnd(), kSalePrice, EM_SETREADONLY, FALSE, 0);
        SendDlgItemMessageW(hwnd(), kMethod, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Straight"));
        SendDlgItemMessageW(hwnd(), kMethod, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Declining"));
        EnableWindow(GetDlgItem(hwnd(), kMethod), FALSE);
        Relabel(L"Rmd", L"Used");
        AddControls();

        TableList::Spec assets;
        assets.section = L"GL_FA";
        assets.table = "gl_fa";
        assets.tag = "facode";
        assets.caption = GetString(6059);                                         // "General Ledger Fixed Assets"
        assets.headers = {GetString(6060), GetString(6093), GetString(6044)};   // Code, Method, Description
        assets.fields = {1, 2, 4};
        list_.Create(hwnd(), std::move(assets), [this](int row) { Show(list_.RowText(row, 1)); }, true, false);

        Clear();
        if (closeAtOnce_) PostMessageW(hwnd(), WM_COMMAND, IDCANCEL, 0);
        return false;   // Clear set the focus
    }

    bool OnDestroy() override
    {
        if (list_.hwnd()) DestroyWindow(list_.hwnd());
        return false;
    }

    bool OnCommand(WORD id, WORD code, HWND ctl) override
    {
        if (id == kSalePrice && code == EN_CHANGE) {
            ShowGainLoss();
            return false;
        }
        if (code != BN_CLICKED) return MainDialog::OnCommand(id, code, ctl);
        switch (id) {
        case IDOK:
            if (FocusIn(kCode)) ShowTyped();                                     // Enter on the code (D9)
            else if (FocusIn(kSalePrice) || FocusIn(kDateDisposed)) NextField();   // Enter in a field moves on
            else Dispose();
            return true;
        case IDC_FA_DELETE: Delete(); return true;
        case kAssetsButton: list_.Seek(GetField(kCode)); return true;
        default:
            if (Navigate(id)) return true;
            return MainDialog::OnCommand(id, code, ctl);   // IDCANCEL -> Cancel()
        }
    }

private:
    // ---- layout -----------------------------------------------------------

    RECT DialogUnits(int x, int y, int cx, int cy) const
    {
        RECT r{x, y, x + cx, y + cy};
        MapDialogRect(hwnd(), &r);
        return r;
    }

    // The static over a field whose text is `from` gets `to`.
    void Relabel(const wchar_t* from, const wchar_t* to)
    {
        for (HWND c = GetWindow(hwnd(), GW_CHILD); c; c = GetWindow(c, GW_HWNDNEXT)) {
            wchar_t text[32] = {};
            GetWindowTextW(c, text, 32);
            if (wcscmp(text, from) == 0) {
                SetWindowTextW(c, to);
                // "Rmd"'s label is 15 dialog units wide: "Used" needs 22 (the next label starts at 126).
                RECT r{};
                GetWindowRect(c, &r);
                MapWindowPoints(nullptr, hwnd(), reinterpret_cast<POINT*>(&r), 2);
                const RECT w = DialogUnits(0, 0, 22, 0);
                SetWindowPos(c, nullptr, 0, 0, w.right, r.bottom - r.top, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
                return;
            }
        }
    }

    // The summary line and Delete, under the "Disposal Data" box (ends at 113
    // dialog units) and Close (100-116): the dialog grows by 22 units.
    void AddControls()
    {
        const RECT grow = DialogUnits(0, 0, 0, 22);
        RECT w{};
        GetWindowRect(hwnd(), &w);
        SetWindowPos(hwnd(), nullptr, 0, 0, w.right - w.left, w.bottom - w.top + grow.bottom,
                     SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
        const HFONT font = reinterpret_cast<HFONT>(SendMessageW(hwnd(), WM_GETFONT, 0, 0));
        const RECT s = DialogUnits(6, 120, 262, 10);
        HWND summary = CreateWindowExW(0, L"STATIC", L"", WS_CHILD | WS_VISIBLE | SS_LEFT, s.left, s.top,
                                       s.right - s.left, s.bottom - s.top, hwnd(),
                                       reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_FA_SUMMARY)), hInst, nullptr);
        const RECT b = DialogUnits(276, 118, 47, 16);
        HWND del = CreateWindowExW(0, L"tbSuperBtn", L"De&lete", WS_CHILD | WS_VISIBLE | WS_TABSTOP | 0x8010, b.left,
                                   b.top, b.right - b.left, b.bottom - b.top, hwnd(),
                                   reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_FA_DELETE)), hInst, nullptr);
        for (HWND h : {summary, del})
            if (h) SendMessageW(h, WM_SETFONT, reinterpret_cast<WPARAM>(font), FALSE);
    }

    bool FocusIn(int id) const
    {
        const HWND control = GetDlgItem(hwnd(), id), focus = GetFocus();
        return focus && (focus == control || GetParent(focus) == control);
    }

    void NextField() { SendMessageW(hwnd(), WM_NEXTDLGCTL, 0, FALSE); }

    // ---- the asset --------------------------------------------------------

    void ShowTyped()
    {
        const std::string code = StrTrim(GetField(kCode));
        if (code.empty()) return;
        if (!Show(code)) {
            Message(fx::Format(L"There is no fixed asset %s.", wide(code).c_str()), MB_ICONINFORMATION);
            SetFocus(GetDlgItem(hwnd(), kCode));
        }
    }

    bool Show(const std::string& code)   // TGLFixedAssets::SetData, for 1001's fields
    {
        const std::optional<Asset> a = FindAsset(cb, code);
        if (!a) return false;
        shown_ = a;
        SeekCode(*table_, codeField_, a->code);   // Next / Previous go on from here
        SetField(kCode, a->code);
        SetField(kDatePurchased, a->datePurchased);
        SendDlgItemMessageW(hwnd(), kMethod, CB_SETCURSEL, a->method == DepMethod::Declining ? 1 : 0, 0);
        SetField(kDescript, a->description);
        SetField(kReference, a->reference);
        IntSetDlgItemValue(hwnd(), kLife, a->life);
        IntSetDlgItemValue(hwnd(), kUsed, a->monthsUsed);
        FloatSetDlgItemValue(hwnd(), kSalvage, a->salvage / 100.0);
        SetField(kDateDisposed, a->disposed() ? a->dateSold : today());
        std::wstring line = fx::Format(L"Cost %s     Accumulated depreciation %s     Book value %s", money(a->cost).c_str(),
                                       money(a->accumulated).c_str(), money(BookValue(*a)).c_str());
        SetDlgItemTextW(hwnd(), IDC_FA_SUMMARY, line.c_str());
        std::wstring c = caption_;
        if (a->disposed()) c += L" - disposed " + fx::Utf8ToWide(rpt::FormatDate(a->dateSold));
        SetWindowTextW(hwnd(), c.c_str());
        ShowGainLoss();
        return true;
    }

    void ShowGainLoss()
    {
        if (!shown_) return;
        double sale = 0;
        if (!FloatGetDlgItemValue(hwnd(), kSalePrice, &sale)) {
            SetField(kGainLoss, "");
            return;
        }
        FloatSetDlgItemValue(hwnd(), kGainLoss, GainOnSale(*shown_, rpt::ToCents(sale)) / 100.0);
    }

    void Clear()
    {
        shown_.reset();
        for (int id : {kCode, kDatePurchased, kDescript, kReference, kSalePrice, kLife, kUsed, kSalvage, kGainLoss})
            SetField(id, "");
        SendDlgItemMessageW(hwnd(), kMethod, CB_SETCURSEL, static_cast<WPARAM>(-1), 0);
        SetField(kDateDisposed, today());
        SetDlgItemTextW(hwnd(), IDC_FA_SUMMARY, L"");
        SetWindowTextW(hwnd(), caption_);
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

    // The asset to act on: the one shown, if its code is still the one typed.
    bool Current()
    {
        const std::string code = StrTrim(GetField(kCode));
        if (code.empty()) {
            Message(fx::Format(GetString(8101).c_str(), L"Fixed Asset Code"), MB_ICONINFORMATION);
            SetFocus(GetDlgItem(hwnd(), kCode));
            return false;
        }
        if (!shown_ || TrimText(shown_->code) != code) {
            if (!Show(code)) {
                ShowTyped();
                return false;
            }
        }
        return true;
    }

    // ---- Dispose and Delete -----------------------------------------------

    void Dispose()   // TGLFixedDelete::Ok, as D7
    {
        if (!Current()) return;
        const Asset a = *shown_;
        if (a.disposed()) {
            Message(fx::Format(L"Asset %s was disposed of on %s.", wide(a.code).c_str(),
                               fx::Utf8ToWide(rpt::FormatDate(a.dateSold)).c_str()),
                    MB_ICONINFORMATION);
            return;
        }
        const std::string date = GetDate(kDateDisposed);
        if (date.empty()) {
            Message(fx::Format(GetString(8101).c_str(), L"Date Disposed"), MB_ICONINFORMATION);
            SetFocus(GetDlgItem(hwnd(), kDateDisposed));
            return;
        }
        double saleValue = 0;
        FloatGetDlgItemValue(hwnd(), kSalePrice, &saleValue);
        const std::int64_t sale = rpt::ToCents(saleValue);
        if (Message(GetString(8107), MB_ICONQUESTION | MB_YESNO) != IDYES) return;   // "...dispose of this asset?"
        if (DisposeAsset(cb, a.code, date) != DisposeResult::Disposed) return;
        Message(EntryText(a, sale, date), MB_ICONINFORMATION);
        list_.Redraw();
        Show(a.code);
    }

    // What to record in Voucher Entry (D7): the asset leaves the books.
    std::wstring EntryText(const Asset& a, std::int64_t sale, const std::string& date) const
    {
        const std::optional<AssetType> t = FindAssetType(cb, a.type);
        auto acct = [](const AssetAccount& x) {
            std::wstring s = fx::Utf8ToWide(TrimText(x.account));
            if (!TrimText(x.dept).empty()) s += L"-" + fx::Utf8ToWide(TrimText(x.dept));
            return s;
        };
        const std::int64_t gain = GainOnSale(a, sale);
        std::wstring s = fx::Format(L"Asset %s is disposed of as at %s.\n\nRecord the disposal in Voucher Entry:\n",
                                    wide(a.code).c_str(), fx::Utf8ToWide(rpt::FormatDate(date)).c_str());
        if (t) s += fx::Format(L"\n    Dr  %s  accumulated depreciation    %s", acct(t->accum).c_str(), money(a.accumulated).c_str());
        if (sale != 0) s += fx::Format(L"\n    Dr  the account the sale was paid into    %s", money(sale).c_str());
        if (gain < 0) s += fx::Format(L"\n    Dr  your loss on disposal account    %s", money(-gain).c_str());
        if (t) s += fx::Format(L"\n    Cr  %s  asset cost    %s", acct(t->asset).c_str(), money(a.cost).c_str());
        if (gain > 0) s += fx::Format(L"\n    Cr  your gain on disposal account    %s", money(gain).c_str());
        return s;
    }

    void Delete()   // new: an asset entered by mistake
    {
        if (!Current()) return;
        const std::string code = shown_->code;
        if (Message(fx::Format(L"Delete asset %s? It is removed from the register.", wide(code).c_str()),
                    MB_ICONQUESTION | MB_YESNO) != IDYES)
            return;
        switch (DeleteAsset(cb, code)) {
        case DeleteAssetResult::Deleted:
            list_.Redraw();
            table_->top();
            Clear();
            break;
        case DeleteAssetResult::HasHistory:
            Message(fx::Format(L"Asset %s has been depreciated, so it can't be deleted.\n\nUse Dispose instead.",
                               wide(code).c_str()),
                    MB_ICONINFORMATION);
            break;
        case DeleteAssetResult::NotFound: break;
        }
    }

    bool closeAtOnce_;
    std::unique_ptr<Data4> table_;
    Field4 codeField_;
    TableList list_;
    std::optional<Asset> shown_;
    wchar_t caption_[128] = {};
};

} // namespace

INT_PTR RunAssetDisposal(HWND frame, bool closeAtOnce)
{
    DisposalDialog dlg(closeAtOnce);
    return dlg.DoModal(1001, frame);
}
