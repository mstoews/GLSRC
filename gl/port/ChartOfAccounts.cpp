//
// ChartOfAccounts.cpp - port of gl/src/gl_accts.cpp. See ChartOfAccounts.h.
//
// Translation notes (plan per-port checklist):
//   - glaccts and gldept belong to the dialog (AccountsTable/DeptTable, which
//     free the Field4s the legacy headers leak). CheckFile("GLACCTS"/"GLDEPT")
//     becomes a check that both tables opened.
//   - Response table -> OnCommand. The CM_FIRST and ID_FIRST entries for
//     ID_GLINQ (Seek and Search) did the same thing and are one case here.
//   - The account type radio buttons no longer write glaccts->type into the
//     current record's buffer. That buffer belonged to whatever record was
//     last shown, and the next seek wrote it back, so choosing a type before
//     adding a new account changed the type of the account shown before.
//     Modify() writes the chosen type instead, which is how the type of an
//     existing account changed in the original.
//   - Account lookups use SeekAccount (Accounts.h): account padded to its
//     width + dept, and an exact match. The original's prefix seek on the
//     concatenated control texts could delete or modify a different account
//     ("1" found "101000"). Set Data keeps the original's nearest-match seek.
//   - szEnterError named the field with GetString(control ID); string 3037 is
//     "Accounts Receivable Voucher Entry", so the description uses 6044
//     ("Description"). A blank account on Delete showed string 1025 ("Print
//     current period Manufacturing Statement."); it gets the blank-field
//     message instead.
//   - TGLAcctsDelete::Ok showed the seek key in a debugging MessageBox; dropped.
//   - SetupWindow's EPM_SETMASK "???????" on the tbFloat ID_CURRBAL, with a
//     callback that returned every character unchanged, is left out: on a
//     tbFloat it would replace the number mask.
//   - SetData's strcpy into a buffer one byte shorter than the account (and
//     without the dept) is a std::string.
//   - TGLBudget (dialog 1017, Accounts > Budget) is not in Spec #1.
//
#include "ChartOfAccounts.h"

#include "AccountList.h"
#include "Accounts.h"
#include "Globals.h"
#include "MainDialog.h"

#include "cb4sqlite.h"

#include <memory>
#include <string>

#include "LegacyIds.h"   // last: see LegacyIds.h

// gl_accts.cpp / gl_accts.h's own IDs (not in gl.h).
#define ID_SEEK    3005
#define ID_FOREIGN 2001
#define ID_RETAINEDERROR 16011   // gl_strg.h

namespace {

class ChartOfAccounts : public MainDialog {
public:
    explicit ChartOfAccounts(bool deleteMode, bool closeAtOnce = false)
        : deleteMode_(deleteMode), closeAtOnce_(closeAtOnce)
    {
    }

protected:
    bool OnInitDialog() override;
    bool OnCommand(WORD id, WORD code, HWND ctl) override;
    bool OnDestroy() override;

private:
    void Ok();
    void Modify();
    void Delete();
    void SetData();
    void Update();
    void Search();
    void ShowForeign();
    void SetType(int id);
    void ClearFields();
    std::string Type() const;
    AccountInput ReadInput() const;
    bool EnterError(int id, UINT nameString);

    bool deleteMode_;
    bool closeAtOnce_;
    std::unique_ptr<AccountsTable> glaccts;
    std::unique_ptr<DeptTable> gldept;
    AccountList list;
};

bool ChartOfAccounts::OnInitDialog()
{
    MainDialog::OnInitDialog();
    glaccts = std::make_unique<AccountsTable>(cb);
    gldept = std::make_unique<DeptTable>(cb);
    if (!glaccts->isValid() || !gldept->isValid()) {   // CheckFile
        Message(std::wstring(L"The company database has no ") + (glaccts->isValid() ? L"GLDEPT" : L"GLACCTS") +
                    L" table.",
                MB_ICONINFORMATION);
        End(IDCANCEL);
        return true;
    }
    ShowWindow(GetDlgItem(hwnd(), ID_DESCRIPT), SW_HIDE);   // the foreign description
    if (deleteMode_) {                                     // TGLAcctsDelete::SetupWindow
        SetFieldW(IDOK, GetString(8095));
        SetWindowTextW(hwnd(), GetString(8096).c_str());
    }
    list.Create(hwnd(), *glaccts);
    SetCheck(ID_FOREIGN, false);
    SetBitmap(ID_GLINQ);
    SetBitmap(ID_SETDATA);
    SetBitmap(IDOK);
    SetBitmap(IDCANCEL);
    SetFocus(GetDlgItem(hwnd(), ID_ACCOUNTS));
    if (closeAtOnce_) PostMessageW(hwnd(), WM_COMMAND, IDCANCEL, 0);
    return false;   // focus set
}

bool ChartOfAccounts::OnCommand(WORD id, WORD code, HWND ctl)
{
    // Edit-control notifications (EN_*) share the WM_COMMAND IDs; only
    // clicks, menu-style commands and the list's ID_ACCTLIST act.
    if (code != BN_CLICKED) return MainDialog::OnCommand(id, code, ctl);
    switch (id) {
    case ID_BS:
    case ID_IS:
    case ID_RE:
        SetType(id);
        return true;
    case ID_GLINQ:   // Search, and Seek (CM_FIRST + ID_GLINQ)
    case ID_SEEK:    // GetSeek
        Search();
        return true;
    case ID_SETDATA:
        SetData();
        return true;
    case IDOK:
        if (deleteMode_)
            Delete();   // TGLAcctsDelete::Ok: the same steps as Delete
        else
            Ok();
        return true;
    case ID_ACCTLIST:   // CM_FIRST + ID_ACCTLIST: the list chose a record
        Update();
        return true;
    case ID_DELETE:
        Delete();
        return true;
    case ID_FOREIGN:
        ShowForeign();
        return true;
    default:
        return MainDialog::OnCommand(id, code, ctl);   // IDCANCEL -> Cancel()
    }
}

bool ChartOfAccounts::OnDestroy()
{
    if (list.hwnd()) DestroyWindow(list.hwnd());   // saves its position while the window still exists
    return false;
}

std::string ChartOfAccounts::Type() const
{
    if (IsChecked(ID_BS)) return "B";
    if (IsChecked(ID_IS)) return "I";
    if (IsChecked(ID_RE)) return "R";
    return "N";
}

void ChartOfAccounts::SetType(int id)   // BS / IS / RE
{
    SetCheck(ID_BS, id == ID_BS);
    SetCheck(ID_IS, id == ID_IS);
    SetCheck(ID_RE, id == ID_RE);
    SetFocus(GetDlgItem(hwnd(), IDOK));
}

AccountInput ChartOfAccounts::ReadInput() const
{
    AccountInput in;
    in.account = GetField(ID_ACCOUNTS);
    in.dept = GetField(ID_DEPART);
    in.descript = GetField(ID_JDESCRIPT);
    in.fdescript = GetField(ID_DESCRIPT);
    in.type = Type();
    in.currBal = GetField(ID_CURRBAL);
    in.openBal = GetField(ID_OPENBAL);
    return in;
}

// TMainDialog::szEnterError: string 4072, "The field %s must be completed before continuing."
bool ChartOfAccounts::EnterError(int id, UINT nameString)
{
    SetFocus(GetDlgItem(hwnd(), id));
    if (!TrimText(GetField(id)).empty()) return false;
    std::wstring text = GetString(ID_BLANKFIELD);
    const size_t at = text.find(L"%s");
    if (at != std::wstring::npos) text.replace(at, 2, GetString(nameString));
    Message(text, MB_ICONEXCLAMATION);
    SetFocus(GetDlgItem(hwnd(), id));
    return true;
}

void ChartOfAccounts::Ok()
{
    if (EnterError(ID_ACCOUNTS, 2013)) return;    // "GL Account"
    if (EnterError(ID_JDESCRIPT, 6044)) return;   // "Description"

    const AccountInput in = ReadInput();
    const bool found = SeekAccount(*glaccts, in.account, in.dept);
    if (!found && in.type == "R") {
        Message(GetString(ID_RETAINEDERROR), MB_OK);   // only one retained earnings account
        return;
    }
    if (found) {
        Modify();
        return;
    }
    if (in.type == "N") {
        Message(GetString(4071), MB_ICONINFORMATION);   // choose an account type
        return;
    }

    AppendAccount(*glaccts, in);

    ClearFields();
    SetField(ID_DESCRIPT, "");
    FloatSetDlgItemValue(hwnd(), ID_CURRBAL, 0);
    FloatSetDlgItemValue(hwnd(), ID_OPENBAL, 0);
    list.Redraw();
    SetFocus(GetDlgItem(hwnd(), ID_ACCOUNTS));
}

void ChartOfAccounts::Modify()   // on the record Ok() found
{
    ModifyAccount(*glaccts, ReadInput());
    list.Redraw();
    Search();   // SendMessage(WM_COMMAND, ID_GLINQ)
}

void ChartOfAccounts::Delete()
{
    if (Message(GetString(1021), MB_ICONQUESTION | MB_YESNO) != IDYES) return;
    if (EnterError(ID_ACCOUNTS, 2013)) return;
    if (!SeekAccount(*glaccts, GetField(ID_ACCOUNTS), GetField(ID_DEPART))) {
        Message(GetString(1026), MB_OK);   // not found
        return;
    }
    if (HasActivity(*glaccts)) {
        Message(GetString(1027), MB_OK);   // must be empty to be deleted
        return;
    }
    DeleteAccount(*glaccts);
    ClearFields();
    SetField(ID_CURRBAL, "");
    SetField(ID_OPENBAL, "");
    SetFocus(GetDlgItem(hwnd(), ID_ACCOUNTS));
    list.Redraw();
}

void ChartOfAccounts::ClearFields()
{
    SetField(ID_ACCOUNTS, "");
    SetField(ID_DEPART, "");
    SetField(ID_DEPTCODE, "");   // not on 1010; kept from the original
    SetField(ID_JDESCRIPT, "");
}

void ChartOfAccounts::SetData()
{
    // SetIndex("accounts") - already active per Tglaccts ctor; retained from original for defensive symmetry
    glaccts->SetIndex("accounts");
    const std::string account = GetField(ID_ACCOUNTS);
    if (TrimText(account).empty())
        glaccts->top();
    else
        glaccts->seek(AccountKey(account, GetField(ID_DEPART), glaccts->accounts->len()).c_str());   // nearest
    Update();
}

void ChartOfAccounts::Update()   // show the current record
{
    SetField(ID_ACCOUNTS, *glaccts->accounts);
    SetField(ID_DEPART, *glaccts->dept);
    SetField(ID_JDESCRIPT, *glaccts->descript);
    SetField(ID_DESCRIPT, *glaccts->fdescript);
    SetField(ID_OPENBAL, *glaccts->previous12);
    SetField(ID_CURRBAL, *glaccts->curr_bal);
    const std::string type = glaccts->type->str();
    if (type == "B") SetType(ID_BS);
    else if (type == "R") SetType(ID_RE);
    else if (type == "I") SetType(ID_IS);
    SetFocus(GetDlgItem(hwnd(), IDOK));
}

void ChartOfAccounts::Search()   // Search / Seek / GetSeek
{
    list.Seek(GetField(ID_ACCOUNTS));
}

void ChartOfAccounts::ShowForeign()
{
    const bool show = !IsChecked(ID_FOREIGN);
    ShowWindow(GetDlgItem(hwnd(), ID_DESCRIPT), show ? SW_SHOW : SW_HIDE);
    SetCheck(ID_FOREIGN, show);
}

} // namespace

INT_PTR RunChartOfAccounts(HWND frame)
{
    ChartOfAccounts dlg(false);
    return dlg.DoModal(1010, frame);
}

INT_PTR RunDeleteAccounts(HWND frame)
{
    ChartOfAccounts dlg(true);
    return dlg.DoModal(1010, frame);
}

int StressChartOfAccounts(int times)
{
    auto objects = [] {
        HANDLE self = GetCurrentProcess();
        return static_cast<int>(GetGuiResources(self, GR_GDIOBJECTS) + GetGuiResources(self, GR_USEROBJECTS));
    };
    {
        ChartOfAccounts warmUp(false, true);   // first open: class registration, cached bitmaps
        warmUp.DoModal(1010, nullptr);
    }
    const int before = objects();
    for (int i = 0; i < times; ++i) {
        ChartOfAccounts dlg(i % 2 != 0, true);
        if (dlg.DoModal(1010, nullptr) == -1) return -1;
    }
    return objects() - before;
}
