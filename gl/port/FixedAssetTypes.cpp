//
// FixedAssetTypes.cpp - see FixedAssetTypes.h.
//
// Translation notes (plan per-port checklist; decisions are spec #4 §8):
//   - D9: a type typed and Enter shows it (TGLFixedAssets::Search, inherited,
//     was empty), and Delete refuses a type an asset uses (the original
//     deleted it, and the run then failed with string 8120).
//   - TGLTypes::Ok checked the three accounts first and then returned without
//     a word on a blank type; the port asks for the type and the description
//     first (string 8101, which TGLFixedAssets::szCheckField used), then
//     checks the accounts (6056-6058).
//   - TGLTypes::Delete seeked the typed type and deleted it, then packed (as
//     Spec #2's DeleteRecord does); kept, on the type typed.
//   - The list (TAssetTypesList) showed the first three fields of gl_fatyp
//     under "Code" / "General Ledger Fixed Asset Types" (string 6061, the
//     caption) / "Description": the third column held FAYEARS. The port shows
//     two columns, "Code" and "Description" (6060, 6044), over FATYPE and
//     FADESC; GL.INI [GL_FATYP] keeps its position, as the original's
//     GetClassName said.
//   - Next / Previous stop at the last / first record, as on the Spec #2
//     screens; the original showed a blank record past either end.
//   - FAYEARS and FAPERCENT are on no screen, as in the original.
//
#include "FixedAssetTypes.h"

#include "Accounts.h"   // TrimText
#include "Assets.h"
#include "Globals.h"
#include "MainDialog.h"
#include "MasterTables.h"   // SeekCode
#include "TableList.h"

#include "cb4sqlite.h"

#include <memory>
#include <optional>
#include <string>
#include <utility>

#include "LegacyIds.h"   // last: see LegacyIds.h

namespace {

constexpr int kType = 6021;       // ID_ASSET_TYPE
constexpr int kDescript = 2015;   // ID_DESCRIPT
constexpr int kTypesButton = 1103;   // ID_FA: "Types"

// The three account rows of the " Accounts " box: account, dept, description.
struct AccountRow {
    int account, dept, descript;
    UINT invalid;   // the message for an account not in the chart
};
constexpr AccountRow kAsset{6023, 6024, 1078, 6056};     // ID_ASSET_ACCOUNT / _DEPT / ID_ASSET_DESC; "Invalid Asset Account"
constexpr AccountRow kExpense{6025, 6026, 6027, 6057};   // ID_EXPENSE_...; "Invalid Expense Account"
constexpr AccountRow kAccum{1076, 6030, 6028, 6058};     // ID_ACCUM_...; "Invalid Accumulated Depreciation Account"

class AssetTypesDialog : public MainDialog {
public:
    explicit AssetTypesDialog(bool closeAtOnce) : closeAtOnce_(closeAtOnce) {}

protected:
    bool OnInitDialog() override
    {
        MainDialog::OnInitDialog();   // New/Top/.../Delete/Add/Close pictures
        SetBitmap(kTypesButton);
        EnsureAssetTables(cb);
        table_ = std::make_unique<Data4>(&cb, "gl_fatyp");
        table_->SetIndex("fatype");
        typeField_.init(*table_, "fatype");
        table_->top();

        TableList::Spec spec;
        spec.section = L"GL_FATYP";
        spec.table = "gl_fatyp";
        spec.tag = "fatype";
        spec.caption = GetString(6061);                       // "General Ledger Fixed Asset Types"
        spec.headers = {GetString(6060), GetString(6044)};   // "Code", "Description"
        spec.stretchColumn = 2;
        list_.Create(hwnd(), std::move(spec), [this](int row) { Pick(row); });
        if (closeAtOnce_) PostMessageW(hwnd(), WM_COMMAND, IDCANCEL, 0);
        SetFocus(GetDlgItem(hwnd(), kType));
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
            if (FocusIn(kType)) ShowTyped();   // Enter on the type (D9)
            else Add();
            return true;
        case ID_DELETE: Delete(); return true;
        case ID_NEW: Clear(); return true;
        case kTypesButton: list_.Seek(GetField(kType)); return true;   // TGLFixedAssets::Types
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
        if (!t.eof()) Show(TrimText(typeField_.str()));
        return true;
    }

    void Pick(int row)   // TAssetTypesList::HandleListBox -> CM_UPDATETYPES
    {
        Show(list_.RowText(row, 1));
    }

    void ShowTyped()
    {
        const std::string type = StrTrim(GetField(kType));
        if (type.empty()) return;
        if (!Show(type)) SetFocus(GetDlgItem(hwnd(), kDescript));   // a new type: go on typing
    }

    // TGLFixedAssets::UpdateType with bTypeList: false if there is no such type.
    bool Show(const std::string& type)
    {
        const std::optional<AssetType> t = FindAssetType(cb, type);
        if (!t) return false;
        SeekCode(*table_, typeField_, t->code);   // Next / Previous go on from here
        SetField(kType, t->code);
        SetField(kDescript, t->description);
        const std::pair<AccountRow, const AssetAccount*> rows[] = {{kAsset, &t->asset}, {kExpense, &t->expense},
                                                                    {kAccum, &t->accum}};
        for (const auto& [row, a] : rows) {
            SetField(row.account, a->account);
            SetField(row.dept, a->dept);
            SetField(row.descript, a->descript);
        }
        return true;
    }

    void Clear()   // TGLFixedAssets::New
    {
        for (int id : {kType, kDescript, kAsset.account, kAsset.dept, kAsset.descript, kExpense.account,
                       kExpense.dept, kExpense.descript, kAccum.account, kAccum.dept, kAccum.descript})
            SetField(id, "");
        SetFocus(GetDlgItem(hwnd(), kType));
    }

    // String 8101 naming the field, then the focus on it.
    bool Missing(int id, const wchar_t* name)
    {
        if (!StrTrim(GetField(id)).empty()) return false;
        Message(fx::Format(GetString(8101).c_str(), name), MB_ICONINFORMATION);
        SetFocus(GetDlgItem(hwnd(), id));
        return true;
    }

    AssetAccount Read(const AccountRow& row) const
    {
        return {StrTrim(GetField(row.account)), StrTrim(GetField(row.dept)), StrTrim(GetField(row.descript))};
    }

    void Add()   // TGLTypes::Ok / Modify
    {
        if (Missing(kType, L"Fixed Asset Type") || Missing(kDescript, L"Description")) return;
        AssetType t;
        t.code = StrTrim(GetField(kType));
        t.description = StrTrim(GetField(kDescript));
        const std::pair<AccountRow, AssetAccount*> rows[] = {{kAsset, &t.asset}, {kExpense, &t.expense},
                                                              {kAccum, &t.accum}};
        for (const auto& [row, a] : rows) {
            *a = Read(row);
            if (!ValidateAccount(cb, a->account, a->dept)) {
                Message(GetString(row.invalid), MB_ICONEXCLAMATION);
                SetFocus(GetDlgItem(hwnd(), row.account));
                return;
            }
        }
        if (FindAssetType(cb, t.code) && Message(GetString(8108), MB_ICONQUESTION | MB_YESNO) != IDYES)
            return;   // "...already been added. Do you want to modify the entry."
        SaveAssetType(cb, t);
        list_.Redraw();
        Clear();   // TGLTypes::Ok cleared after adding (New)
    }

    void Delete()   // TGLTypes::Delete, refusing a type in use (D9)
    {
        if (Missing(kType, L"Fixed Asset Type")) return;
        const std::string type = StrTrim(GetField(kType));
        if (Message(GetString(8109), MB_ICONQUESTION | MB_YESNO) != IDYES) return;   // "...delete this asset type?"
        std::string usedBy;
        switch (DeleteAssetType(cb, type, &usedBy)) {
        case DeleteTypeResult::Deleted:
            list_.Redraw();
            Clear();
            table_->top();
            break;
        case DeleteTypeResult::InUse:
            Message(fx::Format(L"Asset type %s can't be deleted: asset %s uses it.\n\n"
                               L"Give the assets another type first.",
                               fx::Utf8ToWide(type).c_str(), fx::Utf8ToWide(usedBy).c_str()),
                    MB_ICONEXCLAMATION);
            break;
        case DeleteTypeResult::NotFound:
            Message(fx::Format(L"There is no asset type %s.", fx::Utf8ToWide(type).c_str()), MB_ICONINFORMATION);
            break;
        }
    }

    bool closeAtOnce_;
    std::unique_ptr<Data4> table_;
    Field4 typeField_;
    TableList list_;
};

} // namespace

INT_PTR RunAssetTypes(HWND frame, bool closeAtOnce)
{
    AssetTypesDialog dlg(closeAtOnce);
    return dlg.DoModal(4002, frame);
}
