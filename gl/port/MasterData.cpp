//
// MasterData.cpp - see MasterData.h.
//
// Translation notes (plan per-port checklist; corrections are Spec #2
// design D3 / D10):
//   - TDeptList's declaration isn't in the tree; its GL.INI section is
//     "GLDEPT", the table's name, as for the other lists' GetClassName.
//   - The list headers and messages that the original copied from other
//     screens are corrected (D3):
//       Department list: "Acct" / "Dept" / "Description" over depart and
//         descript -> "Dept" / "Description".
//       Medical Reps list column 3: string 8127 ("The invoice has already
//         been cleared...") -> 6044 "Description", as the Doctor list has.
//       Product list column 2: 8106 "Description" twice -> 8126 "Name".
//       Product update prompt: 8102 ("The current asset has already been
//         added...") -> 8122 ("Record has already been added...").
//       Doctor delete prompt: 8113 ("...the current Medical Representative?")
//         -> 6052 ("Delete the record?").
//   - Data the original lost on every save (D10):
//       TGLDeptCodes::Delete deleted the *current* record, not the
//         department typed; the port seeks the typed code first.
//       Medical Reps and Doctor Codes wrote balance from control 6039, which
//         neither template has, so every save set it to 0; the port leaves
//         balance alone. Medical Reps read territory from 6038, but 1006's
//         territory control is 1076, so it was saved blank; the port reads
//         1076 there (1040's is 6038).
//       Product Codes wrote flotsize from control 6007, which 1007 lacks;
//         the port leaves it alone. TGLProd::Ok appended a record with a
//         blank code; the port asks for the code (string 4072), as Delete did.
//   - Next / Previous stop at the last / first record; the original showed a
//     blank record past either end.
//   - Department Codes redraws its list after Add/Mod, as the other three
//     screens did (the original redrew it only after Delete). Its Modify
//     handler (ID_MODIFY) had no button in 1023 and isn't ported.
//   - Each list window is destroyed with its dialog (saving its position).
//     The originals' Cancel called List->SavePosition() and cb.closeAll().
//
#include "MasterData.h"

#include "Accounts.h"   // TrimText
#include "Globals.h"
#include "MainDialog.h"
#include "MasterTables.h"
#include "TableList.h"

#include "cb4sqlite.h"

#include <memory>
#include <string>
#include <utility>

#include "LegacyIds.h"   // last: see LegacyIds.h

namespace {

// Control IDs of the four templates (gl/header/gl.h names in the comments).
constexpr int kCode = 3046;       // ID_CODE
constexpr int kShort = 4002;      // ID_SHORT
constexpr int kName = 4003;       // ID_NAME
constexpr int kAddress1 = 3073;   // ID_ADDRESS1 (Street)
constexpr int kAddress2 = 3068;   // ID_ADDRESS2 (City)
constexpr int kAddress3 = 3069;   // ID_ADDRESS3 (State/Country)
constexpr int kPostal = 3070;     // ID_POST
constexpr int kPhone = 6069;      // ID_PHONE
constexpr int kFax = 6068;        // ID_FAX
constexpr int kContact = 3072;    // ID_CONTACT
constexpr int kTerms = 6086;      // ID_TERMS
constexpr int kCreditLimit = 6040;   // ID_CREDITLMT
constexpr int kTerritoryRep = 1076;  // 1006's territory control
constexpr int kTerritoryDoctor = 6038;   // 1040's (ID_TERRITORY)
constexpr int kProdName = 6008;   // ID_SNAME
constexpr int kProdFull = 6018;   // ID_FULLN
constexpr int kProdType = 6010;   // ID_PTYPE
constexpr int kProdUnit = 6009;   // ID_UNIT
constexpr int kProdOnHand = 6006;    // ID_STANDARD ("Cost" on the template)
constexpr int kProdPrice = 6012;     // ID_AVPRICE ("Unit Price")
constexpr int kListButton = 1074;    // ID_SLIPLIST: "Med Reps" / "Doctor" / "Products"

// Shared by the four dialogs: the table check, the list, closing at once for
// /stress, and the record-navigation buttons.
class CodeDialog : public MainDialog {
public:
    explicit CodeDialog(bool closeAtOnce) : closeAtOnce_(closeAtOnce) {}

protected:
    bool OnDestroy() override
    {
        if (list_.hwnd()) DestroyWindow(list_.hwnd());   // saves its position while the window still exists
        return false;
    }

    // CheckFile: false (and the dialog closes) if the table isn't there.
    bool RequireTable(const Data4& t, const wchar_t* name)
    {
        if (t.isValid()) return true;
        Message(std::wstring(L"The company database has no ") + name + L" table.", MB_ICONINFORMATION);
        End(IDCANCEL);
        return false;
    }

    void FinishInit()
    {
        if (closeAtOnce_) PostMessageW(hwnd(), WM_COMMAND, IDCANCEL, 0);
    }

    // TMainDialog::szEnterError: string 4072 naming the field.
    bool EnterError(int id, const std::wstring& name)
    {
        if (!StrTrim(GetField(id)).empty()) return false;
        Message(fx::Format(GetString(4072).c_str(), name.c_str()), MB_ICONINFORMATION);
        SetFocus(GetDlgItem(hwnd(), id));
        return true;
    }

    // Top / Bottom / Next / Previous on `t`, then `show` the record.
    template <class Show>
    bool Navigate(Data4& t, WORD id, Show show)
    {
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
        if (!t.eof()) show();
        return true;
    }

    TableList list_;

private:
    bool closeAtOnce_;
};

// ---- Department Codes (1023) ----------------------------------------------

class DepartmentCodes : public CodeDialog {
public:
    using CodeDialog::CodeDialog;

protected:
    bool OnInitDialog() override
    {
        MainDialog::OnInitDialog();
        SetBitmap(ID_GLINQ);
        gldept_ = std::make_unique<DepartmentTable>(cb);
        if (!RequireTable(*gldept_, L"GLDEPT")) return true;
        TableList::Spec spec;
        spec.section = L"GLDEPT";
        spec.table = "gldept";
        spec.tag = "depart";
        spec.caption = GetString(6092);                        // "Department Code List"
        spec.headers = {GetString(6054), GetString(6044)};    // "Dept", "Description" (D3)
        spec.stretchColumn = 2;
        list_.Create(hwnd(), std::move(spec), [this](int row) { Pick(row); });
        FinishInit();
        return true;
    }

    bool OnCommand(WORD id, WORD code, HWND ctl) override
    {
        if (code != BN_CLICKED) return MainDialog::OnCommand(id, code, ctl);
        switch (id) {
        case IDOK: Ok(); return true;
        case ID_DELETE: Delete(); return true;
        case ID_GLINQ: list_.Seek(GetField(ID_DEPART)); return true;   // Search
        default: return MainDialog::OnCommand(id, code, ctl);         // IDCANCEL -> Cancel()
        }
    }

private:
    void Pick(int row)   // TDeptList::HandleListBox
    {
        if (!SeekCode(*gldept_, gldept_->depart, list_.RowText(row, 1))) return;
        SetField(ID_DEPART, gldept_->depart);
        SetField(ID_DESCRIPT, gldept_->descript);
    }

    void Clear()
    {
        SetField(ID_DEPART, "");
        SetField(ID_DESCRIPT, "");
    }

    void Ok()   // TGLDeptCodes::Ok
    {
        const std::string dept = StrTrim(GetField(ID_DEPART));
        const std::string descript = StrTrim(GetField(ID_DESCRIPT));
        if (dept.empty()) {
            Message(GetString(6049), MB_OK);   // "Enter a new department number."
            return;
        }
        if (descript.empty()) {
            Message(GetString(6050), MB_OK);   // "Enter a new department description."
            return;
        }
        DepartmentTable& t = *gldept_;
        const SaveResult r = SaveRecord(
            t, t.depart, dept,
            [&] {
                t.depart.assign(dept.c_str());
                t.descript.assign(descript.c_str());
            },
            [&] { return Message(GetString(6051), MB_YESNO) == IDYES; });   // "...already been entered. modify?"
        if (r == SaveResult::Added) Clear();   // the original cleared only after adding
        if (r != SaveResult::Unchanged) list_.Redraw();
    }

    void Delete()   // TGLDeptCodes::Delete, on the department typed (D10)
    {
        const std::string dept = StrTrim(GetField(ID_DEPART));
        if (dept.empty()) {
            Message(GetString(6049), MB_OK);
            return;
        }
        if (Message(GetString(6052), MB_YESNO) != IDYES) return;   // "Delete the record?"
        if (DeleteRecord(*gldept_, gldept_->depart, dept)) {
            Clear();
            SetFocus(GetDlgItem(hwnd(), IDOK));
            list_.Redraw();
        }
    }

    std::unique_ptr<DepartmentTable> gldept_;
};

// ---- Medical Reps (1006) and Doctor Codes (1040) --------------------------

class RepCodes : public CodeDialog {
public:
    RepCodes(RepTable::Kind kind, bool closeAtOnce) : CodeDialog(closeAtOnce), kind_(kind) {}

protected:
    bool OnInitDialog() override
    {
        MainDialog::OnInitDialog();   // the New/Top/.../list/Delete/Add/Close pictures
        const bool rep = kind_ == RepTable::Kind::MedicalRep;
        table_ = std::make_unique<RepTable>(cb, kind_);
        if (!RequireTable(*table_, rep ? L"GL_MEDRP" : L"GL_DRCOD")) return true;
        TableList::Spec spec;
        spec.section = rep ? L"GL_MEDRP" : L"GL_DRCOD";
        spec.table = rep ? "gl_medrp" : "gl_drcod";
        spec.tag = rep ? "repcode" : "drcode";
        if (rep) {
            spec.caption = GetString(8124);   // "General Ledger Medical Representatives"
            spec.headers = {GetString(8125), GetString(8126), GetString(6044)};   // Rep. Code, Name, Description (D3)
        } else {
            spec.caption = L"General Ledger Doctor Codes";   // literals in TGLDrCodeList::SetWndDetail
            spec.headers = {L"Dr. Code", L"Name", L"Description"};
        }
        list_.Create(hwnd(), std::move(spec), [this](int row) { Pick(row); });
        FinishInit();
        return true;
    }

    bool OnCommand(WORD id, WORD code, HWND ctl) override
    {
        if (code != BN_CLICKED) return MainDialog::OnCommand(id, code, ctl);
        switch (id) {
        case IDOK: Ok(); return true;
        case ID_DELETE: Delete(); return true;
        case ID_NEW: Clear(); return true;
        case kListButton: list_.Seek(GetField(kCode)); return true;   // RepsList
        default:
            if (Navigate(*table_, id, [this] { Show(); })) return true;
            return MainDialog::OnCommand(id, code, ctl);   // IDCANCEL -> Cancel()
        }
    }

private:
    int TerritoryId() const { return kind_ == RepTable::Kind::MedicalRep ? kTerritoryRep : kTerritoryDoctor; }

    void Pick(int row)   // TGLMedRepList::HandleListBox -> CM_UPDATE -> SetData
    {
        if (SeekCode(*table_, table_->code, list_.RowText(row, 1))) Show();
    }

    void Show()   // TGLMedReps::SetData
    {
        const RepTable& t = *table_;
        SetField(kCode, t.code);
        SetField(kShort, t.rpshort);
        SetField(kName, t.rpname);
        SetField(kAddress1, t.address1);
        SetField(kAddress2, t.address2);
        SetField(kAddress3, t.address3);
        SetField(kPostal, t.postal);
        SetField(kPhone, t.phone);
        SetField(kFax, t.fax);
        SetField(kContact, t.contact);
        SetField(kTerms, t.terms);
        SetField(kCreditLimit, t.creditlmt);
        SetField(TerritoryId(), t.territory);
    }

    void Clear()   // TGLMedReps::New
    {
        for (int id : {kCode, kShort, kName, kAddress1, kAddress2, kAddress3, kPostal, kPhone, kFax, kContact, kTerms,
                       kCreditLimit, TerritoryId()})
            SetField(id, "");
    }

    RepInput Input() const
    {
        RepInput in;
        in.code = StrTrim(GetField(kCode));
        in.shortName = GetField(kShort);
        in.name = GetField(kName);
        in.address1 = GetField(kAddress1);
        in.address2 = GetField(kAddress2);
        in.address3 = GetField(kAddress3);
        in.postal = GetField(kPostal);
        in.phone = GetField(kPhone);
        in.fax = GetField(kFax);
        in.contact = GetField(kContact);
        in.territory = GetField(TerritoryId());
        in.terms = GetField(kTerms);
        in.creditLimit = GetField(kCreditLimit);
        return in;
    }

    void Ok()   // TGLMedReps::Ok
    {
        const RepInput in = Input();
        if (in.code.empty()) return;
        RepTable& t = *table_;
        const SaveResult r = SaveRecord(
            t, t.code, in.code, [&] { AssignRep(t, in); },
            [&] { return Message(GetString(8122), MB_YESNO) == IDYES; });   // "Record has already been added..."
        t.unlock();
        Clear();
        if (r != SaveResult::Unchanged) list_.Redraw();
    }

    void Delete()   // TGLMedReps::Delete
    {
        const UINT prompt = kind_ == RepTable::Kind::MedicalRep ? 8113 : 6052;   // D3 for doctors
        if (Message(GetString(prompt), MB_ICONQUESTION | MB_YESNO) != IDYES) return;
        if (DeleteRecord(*table_, table_->code, GetField(kCode))) {
            list_.Redraw();
            Clear();
        }
    }

    RepTable::Kind kind_;
    std::unique_ptr<RepTable> table_;
};

// ---- Product Codes (1007) -------------------------------------------------

class ProductCodes : public CodeDialog {
public:
    using CodeDialog::CodeDialog;

protected:
    bool OnInitDialog() override
    {
        MainDialog::OnInitDialog();   // includes the Products button (ID_SLIPLIST)
        glprod_ = std::make_unique<ProductTable>(cb);
        if (!RequireTable(*glprod_, L"GL_ITEMM")) return true;
        TableList::Spec spec;
        spec.section = L"GL_ITEMM";
        spec.table = "gl_itemm";
        spec.tag = "fpcode";
        spec.caption = GetString(8128);   // "Product Codes" (TGLProd::SetupWindow's SetCaption over 8104)
        spec.headers = {GetString(8105), GetString(8126), GetString(8106)};   // Code, Name (D3), Description
        // TGLProdList::HandleList didn't hide the list after a pick.
        list_.Create(hwnd(), std::move(spec), [this](int row) { Pick(row); }, false);
        FinishInit();
        return true;
    }

    bool OnCommand(WORD id, WORD code, HWND ctl) override
    {
        if (code != BN_CLICKED) return MainDialog::OnCommand(id, code, ctl);
        switch (id) {
        case IDOK: Ok(); return true;
        case ID_DELETE: Delete(); return true;
        case ID_NEW: Clear(); return true;
        case kListButton: list_.Seek(GetField(kCode)); return true;   // RepsList
        default:
            if (Navigate(*glprod_, id, [this] { Show(); })) return true;
            return MainDialog::OnCommand(id, code, ctl);
        }
    }

private:
    void Pick(int row)   // TGLProdList::HandleList -> CM_UPDATE -> SetData
    {
        if (SeekCode(*glprod_, glprod_->fpcode, list_.RowText(row, 1))) Show();
    }

    void Show()   // TGLProd::SetData
    {
        const ProductTable& t = *glprod_;
        SetField(kCode, t.fpcode);
        SetField(kProdName, t.fpname);
        SetField(kProdFull, t.ffulln);
        SetField(kProdType, t.fptype);
        SetField(kProdOnHand, t.fonhand);
        SetField(kProdUnit, t.funit);
        SetField(kProdPrice, t.favprice);
    }

    void Clear()   // TGLProd::New
    {
        for (int id : {kCode, kProdName, kProdFull, kProdType, kProdOnHand, kProdUnit, kProdPrice}) SetField(id, "");
    }

    void Ok()   // TGLProd::Ok / Update
    {
        if (EnterError(kCode, GetString(8105))) return;   // D10: no blank codes
        ProductInput in;
        in.code = StrTrim(GetField(kCode));
        in.name = GetField(kProdName);
        in.fullName = GetField(kProdFull);
        in.type = GetField(kProdType);
        in.unit = GetField(kProdUnit);
        in.onHand = GetField(kProdOnHand);
        in.avPrice = GetField(kProdPrice);
        ProductTable& t = *glprod_;
        const SaveResult r = SaveRecord(
            t, t.fpcode, in.code, [&] { AssignProduct(t, in); },
            [&] { return Message(GetString(8122), MB_ICONQUESTION | MB_YESNO) == IDYES; });   // D3: was 8102
        t.unlock();
        if (r == SaveResult::Unchanged) return;
        list_.Redraw();
        Clear();
    }

    void Delete()   // TGLProd::Delete
    {
        if (EnterError(kCode, GetString(8105))) return;
        if (Message(GetString(8123), MB_ICONQUESTION | MB_YESNO) != IDYES) return;   // "...delete this product code?"
        if (DeleteRecord(*glprod_, glprod_->fpcode, GetField(kCode))) {
            list_.Redraw();
            Clear();
        }
    }

    std::unique_ptr<ProductTable> glprod_;
};

} // namespace

INT_PTR RunDepartmentCodes(HWND frame)
{
    DepartmentCodes dlg(false);
    return dlg.DoModal(1023, frame);
}

INT_PTR RunMedicalReps(HWND frame)
{
    RepCodes dlg(RepTable::Kind::MedicalRep, false);
    return dlg.DoModal(1006, frame);
}

INT_PTR RunDoctorCodes(HWND frame)
{
    RepCodes dlg(RepTable::Kind::Doctor, false);
    return dlg.DoModal(1040, frame);
}

INT_PTR RunProductCodes(HWND frame)
{
    ProductCodes dlg(false);
    return dlg.DoModal(1007, frame);
}

int StressMasterData(int times)
{
    auto objects = [] {
        HANDLE self = GetCurrentProcess();
        return static_cast<int>(GetGuiResources(self, GR_GDIOBJECTS) + GetGuiResources(self, GR_USEROBJECTS));
    };
    auto openAll = [] {
        DepartmentCodes dept(true);
        RepCodes rep(RepTable::Kind::MedicalRep, true);
        RepCodes doctor(RepTable::Kind::Doctor, true);
        ProductCodes prod(true);
        return dept.DoModal(1023, nullptr) != -1 && rep.DoModal(1006, nullptr) != -1 &&
               doctor.DoModal(1040, nullptr) != -1 && prod.DoModal(1007, nullptr) != -1;
    };
    if (!openAll()) return -1;   // first open: class registration, cached bitmaps
    const int before = objects();
    for (int i = 0; i < times; ++i)
        if (!openAll()) return -1;
    return objects() - before;
}
