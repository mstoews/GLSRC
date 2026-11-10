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
//   - TGLBudget (dialog 1017, Accounts > Budget Data; Spec #2 S2-M5) is
//     TGLAcctsAdd with a budget grid and its own Ok: the Budget mode here.
//     Corrections (spec D8, D13):
//       - its Ok assigned budget1-12 to the current record without writing
//         it; they were saved only when the record moved or the tables
//         closed. The port writes them at once, then shows what was saved;
//       - it saved into whatever record was current, which a later Set Data
//         on an unknown account could leave on another one; the port saves
//         into the account the grid shows;
//       - the 12 amounts are edited in the grid (D8), as in the original,
//         whose grid was the tbSpread with editable float cells.
//     Update read fields 33-44, 8-19 and 21-32 by number; in the real
//     layout they are budget1-12, period1-12 and previous1-12, which
//     ReadBudget (Accounts.h) opens by name. Its caption was already set at
//     run time from string 8097, as here (D3).
//
#include "ChartOfAccounts.h"

#include "AccountList.h"
#include "Accounts.h"
#include "Globals.h"
#include "MainDialog.h"
#include "VoucherLists.h"   // FormatAmount

#include "cb4sqlite.h"

#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#include "LegacyIds.h"   // last: see LegacyIds.h

// gl_accts.cpp / gl_accts.h's own IDs (not in gl.h).
#define ID_SEEK    3005
#define ID_FOREIGN 2001
#define ID_RETAINEDERROR 16011   // gl_strg.h

namespace {

enum class Mode { Add, Delete, Budget };

class ChartOfAccounts : public MainDialog {
public:
    explicit ChartOfAccounts(Mode mode, bool closeAtOnce = false)
        : deleteMode_(mode == Mode::Delete), budget_(mode == Mode::Budget), closeAtOnce_(closeAtOnce)
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
    void SetupBudget();
    void ShowBudget();
    void SaveBudgetData();

    bool deleteMode_;
    bool budget_;
    bool closeAtOnce_;
    fx::Grid grid_;            // Budget: 1017's tbSpread
    std::string shownAccount_, shownDept_;   // Budget: the account the grid shows
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
    if (budget_) SetupBudget();
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
        if (budget_)
            SaveBudgetData();   // TGLBudget::Ok
        else if (deleteMode_)
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
    if (budget_) ShowBudget();   // TGLBudget::Update
    SetFocus(GetDlgItem(hwnd(), IDOK));
}

// ---- TGLBudget (dialog 1017) ----------------------------------------------

void ChartOfAccounts::SetupBudget()   // TGLBudget::SetupWindow
{
    SetWindowTextW(hwnd(), GetString(8097).c_str());   // "General Ledger - Budget Update"
    grid_ = fx::Grid(GetDlgItem(hwnd(), ID_DISTLIST));
    if (!grid_.IsGrid()) return;
    // Column 1 six characters wide, the other three sharing the rest.
    RECT rc{};
    GetClientRect(grid_.hwnd(), &rc);
    const int first = 7 * grid_.CharWidth();
    const int rest = (std::max)(10 * grid_.CharWidth(),
                                (static_cast<int>(rc.right) - first - GetSystemMetrics(SM_CXVSCROLL)) / 3);
    grid_.SetColumns({{L"Period", first, fx::GridAlign::Right},
                      {L"Budget", rest, fx::GridAlign::Right},
                      {L"Current Year", rest, fx::GridAlign::Right},
                      {L"Previous Year", rest, fx::GridAlign::Right}});
    // SSSetTypeFloat(FS_SEPARATOR, 15, 2): the tbFloat mask and style of 1017's own amounts.
    grid_.SetColumnEditable(2, L"9999999999999.99", 9216);
    grid_.SetMaxRows(12);
}

void ChartOfAccounts::ShowBudget()   // the current record
{
    shownAccount_ = TrimText(glaccts->accounts->str());
    shownDept_ = TrimText(glaccts->dept->str());
    if (!grid_.IsGrid()) return;
    if (grid_.IsEditing()) grid_.EndEdit(false);
    grid_.SetRedraw(false);
    const std::vector<BudgetRow> rows = ReadBudget(*glaccts);
    for (int i = 1; i <= 12; ++i) {
        const BudgetRow& r = rows[static_cast<size_t>(i - 1)];
        grid_.SetData(1, i, std::to_wstring(i));
        grid_.SetData(2, i, FormatAmount(r.budget, false));
        grid_.SetData(3, i, FormatAmount(r.current, false));
        grid_.SetData(4, i, FormatAmount(r.previous, false));
    }
    grid_.SetRedraw(true);
}

void ChartOfAccounts::SaveBudgetData()   // TGLBudget::Ok
{
    if (grid_.IsGrid() && grid_.IsEditing()) grid_.EndEdit(true);
    if (shownAccount_.empty() || !SeekAccount(*glaccts, shownAccount_, shownDept_)) {
        Message(GetString(3081), MB_ICONINFORMATION);   // "Unable to find GL Account": nothing shown yet
        return;
    }
    // "Update the current budget information?"
    if (Message(GetString(8098), MB_ICONQUESTION | MB_YESNO) != IDYES) return;
    std::vector<double> budget;
    for (int i = 1; i <= 12; ++i) budget.push_back(ParseAmount(fx::WideToUtf8(grid_.GetData(2, i))));
    SaveBudget(*glaccts, budget);
    Update();   // shows what was saved
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
    ChartOfAccounts dlg(Mode::Add);
    return dlg.DoModal(1010, frame);
}

INT_PTR RunDeleteAccounts(HWND frame)
{
    ChartOfAccounts dlg(Mode::Delete);
    return dlg.DoModal(1010, frame);
}

INT_PTR RunBudgetData(HWND frame)
{
    ChartOfAccounts dlg(Mode::Budget);
    return dlg.DoModal(1017, frame);
}

int StressChartOfAccounts(int times)
{
    auto objects = [] {
        HANDLE self = GetCurrentProcess();
        return static_cast<int>(GetGuiResources(self, GR_GDIOBJECTS) + GetGuiResources(self, GR_USEROBJECTS));
    };
    {
        ChartOfAccounts warmUp(Mode::Add, true);   // first open: class registration, cached bitmaps
        warmUp.DoModal(1010, nullptr);
        ChartOfAccounts budget(Mode::Budget, true);
        if (budget.DoModal(1017, nullptr) == -1) return -1;
    }
    const int before = objects();
    for (int i = 0; i < times; ++i) {
        // Add, Delete and Budget Data (1017) in turn.
        const Mode mode = i % 3 == 0 ? Mode::Add : i % 3 == 1 ? Mode::Delete : Mode::Budget;
        ChartOfAccounts dlg(mode, true);
        if (dlg.DoModal(mode == Mode::Budget ? 1017 : 1010, nullptr) == -1) return -1;
    }
    return objects() - before;
}
