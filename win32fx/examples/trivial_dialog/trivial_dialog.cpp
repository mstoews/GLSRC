//
// trivial_dialog.cpp - win32fx consumed from outside gl.sln (plan M3 task 8,
// acceptance 6.7 #9), the way the AP / AR projects will consume it.
//
//   trivial_dialog.exe                    the dialog, modal
//   trivial_dialog.exe /selftest          the checks below, results in a message box
//   trivial_dialog.exe /selftest /quiet   the same, no message box; exit code = failures
//                                         (from cmd: start /wait trivial_dialog.exe /selftest /quiet
//                                          then echo %errorlevel%)
//
// The self-test is the plan's two-instance dispatch test (risk P2): two live
// instances of one dialog class, and two MDI children of one window class,
// each receive their own messages. It also drives the custom controls with
// WM_SETTEXT / WM_CHAR, and the tbSpread grid (fx::Grid, Spec #2 plan M0)
// through its API, its keyboard and its custom draw.
//
#include <Windows.h>

#include "win32fx.h"
#include "resource.h"

#include <string>
#include <utility>

namespace {

class TrivialDialog : public fx::Dialog {
public:
    explicit TrivialDialog(const wchar_t* name, bool editableGrid = true) : name_(name), editable_(editableGrid) {}
    int pings = 0;
    int picks = 0, changes = 0, selChanges = 0;   // GRN_* from IDC_GRID

protected:
    bool OnInitDialog() override
    {
        SetDlgItemTextW(hwnd(), IDC_NAME, name_);
        fx::Grid grid(GetDlgItem(hwnd(), IDC_GRID));
        const int ch = grid.CharWidth();
        grid.SetColumns({{L"Account", 9 * ch}, {L"Description", 20 * ch}, {L"Amount", 12 * ch, fx::GridAlign::Right}});
        grid.SetStretchColumn(2);
        const wchar_t* rows[][3] = {{L"101000", L"Petty cash", L"500.00"},
                                    {L"102101", L"Checking account", L"12,500.00"},
                                    {L"340000", L"Retained earnings", L"-16,840.00"}};
        for (int r = 0; r < 3; ++r)
            for (int c = 0; c < 3; ++c) grid.SetData(c + 1, r + 1, rows[r][c]);
        if (editable_) grid.SetColumnEditable(3, L"9999999999.99", 0x2000);   // FS_SEPARATOR
        return true;
    }

    bool OnCommand(WORD id, WORD code, HWND) override
    {
        if (id == IDC_GRID) {
            if (code == GRN_PICK) ++picks;
            if (code == GRN_CELLCHANGED) ++changes;
            if (code == GRN_SELCHANGE) ++selChanges;
            return true;
        }
        if (id != IDC_PING || code != BN_CLICKED) return false;
        SetDlgItemInt(hwnd(), IDC_PINGS, static_cast<UINT>(++pings), FALSE);
        return true;
    }

    BEGIN_MSG_MAP(TrivialDialog)
        ON_MSG(WM_APP, OnWhoAmI)
    END_MSG_MAP()

    LRESULT OnWhoAmI(WPARAM, LPARAM) { return reinterpret_cast<LRESULT>(this); }

private:
    const wchar_t* name_;
    bool editable_;
};

class ProbeChild : public fx::MdiChild {
public:
    int commands = 0;

protected:
    bool OnCommand(WORD id, WORD, HWND) override
    {
        if (id != 42) return false;
        ++commands;
        return true;
    }
};

class ProbeFrame : public fx::MdiFrame {
public:
    HWND lastActivated = nullptr;
    void OnMdiChildActivate(HWND child) override { lastActivated = child; }

protected:
    bool OnDestroy() override { return true; }   // not the main window: no WM_QUIT
};

struct Results {
    int failures = 0;
    std::wstring log;
    void check(bool ok, const wchar_t* what)
    {
        log += ok ? L"ok      " : L"FAILED  ";
        log += what;
        log += L"\n";
        if (!ok) ++failures;
    }
};

void typeText(HWND ctl, const wchar_t* text)
{
    SendMessageW(ctl, EM_SETSEL, 0, -1);
    SendMessageW(ctl, EM_REPLACESEL, FALSE, reinterpret_cast<LPARAM>(L""));
    for (const wchar_t* p = text; *p; ++p) SendMessageW(ctl, WM_CHAR, *p, 0);
}

void dialogInstances(HINSTANCE inst, Results& r)
{
    TrivialDialog a(L"A"), b(L"B");
    HWND ha = a.CreateModeless(inst, IDD_TRIVIAL, nullptr);
    HWND hb = b.CreateModeless(inst, IDD_TRIVIAL, nullptr);
    r.check(ha && hb && ha != hb, L"two modeless instances of one Dialog class");
    if (!ha || !hb) return;

    const auto ping = [](HWND dlg) {
        SendMessageW(dlg, WM_COMMAND, MAKEWPARAM(IDC_PING, BN_CLICKED),
                     reinterpret_cast<LPARAM>(GetDlgItem(dlg, IDC_PING)));
    };
    ping(ha);
    ping(ha);
    ping(hb);
    r.check(a.pings == 2 && b.pings == 1, L"WM_COMMAND reaches the instance it was sent to");
    r.check(SendMessageW(ha, WM_APP, 0, 0) == reinterpret_cast<LRESULT>(&a) &&
                SendMessageW(hb, WM_APP, 0, 0) == reinterpret_cast<LRESULT>(&b),
            L"message map runs on its own instance, result via DWLP_MSGRESULT");
    r.check(fx::Dialog::FromHandle(ha) == &a && fx::Dialog::FromHandle(hb) == &b, L"Dialog::FromHandle");

    ShowWindow(hb, SW_SHOW);
    SetActiveWindow(hb);
    SendMessageW(GetDlgItem(hb, IDC_PING), BM_CLICK, 0, 0);
    r.check(a.pings == 2 && b.pings == 2, L"a click on B's tbSuperBtn reaches B only");

    // Custom controls, on dialog A.
    HWND account = GetDlgItem(ha, IDC_ACCOUNT), phone = GetDlgItem(ha, IDC_PHONE);
    HWND date = GetDlgItem(ha, IDC_DATE), amount = GetDlgItem(ha, IDC_AMOUNT), qty = GetDlgItem(ha, IDC_QUANTITY);
    r.check(fx::GetText(account).empty() && fx::GetText(amount).empty(), L"template text is the mask, not shown");
    wchar_t mask[32] = {};
    SendMessageW(account, EPM_GETMASK, 32, reinterpret_cast<LPARAM>(mask));
    r.check(std::wstring(mask) == L"NNNNNNN", L"EPM_GETMASK returns the template mask");
    const LRESULT code = SendMessageW(account, WM_GETDLGCODE, 0, 0);
    r.check((code & (DLGC_WANTCHARS | DLGC_WANTARROWS)) == (DLGC_WANTCHARS | DLGC_WANTARROWS),
            L"tbPic WM_GETDLGCODE has DLGC_WANTCHARS | DLGC_WANTARROWS");

    typeText(account, L"AB12345X");
    r.check(fx::GetText(account) == L"AB12345", L"tbPic NNNNNNN: at most 7 letters/digits");
    typeText(account, L"A-1");
    r.check(fx::GetText(account) == L"A1", L"tbPic NNNNNNN: '-' rejected");
    typeText(phone, L"5551234567");
    r.check(fx::GetText(phone) == L"(555) 123 - 4567", L"tbPic phone mask: literals inserted");

    SetWindowTextW(date, L"19950131");
    r.check(fx::GetText(date) == L"01/31/95", L"tbDate shows YYYYMMDD as MM/DD/YY");
    r.check(fx::DateGetYmd(date) == L"19950131", L"DateGetYmd reads it back as YYYYMMDD");
    typeText(date, L"123199");
    r.check(fx::GetText(date) == L"12/31/99" && fx::DateGetYmd(date) == L"19991231", L"tbDate typing inserts '/'");
    typeText(date, L"023099");
    r.check(fx::DateGetYmd(date).empty(), L"DateGetYmd: Feb 30 is not a date");

    SetWindowTextW(amount, L"     1234.5");
    r.check(fx::GetText(amount) == L"1,234.50", L"tbFloat formats a padded cb4sqlite value");
    double v = 0;
    r.check(FloatGetValue(amount, &v) && v == 1234.5, L"FloatGetValue ignores separators");
    typeText(amount, L"12.345");
    r.check(fx::GetText(amount) == L"12.34", L"tbFloat .99: third decimal rejected");
    FloatSetDlgItemValue(ha, IDC_AMOUNT, -0.5);
    r.check(fx::GetText(amount) == L"-0.50", L"FloatSetValue");

    IntSetDlgItemValue(ha, IDC_QUANTITY, 42);
    LONG n = 0;
    r.check(IntGetDlgItemValue(ha, IDC_QUANTITY, &n) && n == 42, L"IntSetValue / IntGetValue");
    typeText(qty, L"12345");
    r.check(fx::GetText(qty) == L"1234", L"tbInteger 9999: fifth digit rejected");
    typeText(qty, L"1.5");
    r.check(fx::GetText(qty) == L"15", L"tbInteger: '.' rejected");

    DestroyWindow(ha);
    DestroyWindow(hb);
    r.check(a.hwnd() == nullptr && b.hwnd() == nullptr, L"modeless dialogs detach on WM_NCDESTROY");
}

void key(HWND ctl, WPARAM vk)
{
    SendMessageW(ctl, WM_KEYDOWN, vk, 1);
    SendMessageW(ctl, WM_KEYUP, vk, 0xC0000001);
}

void gridChecks(HINSTANCE inst, Results& r)
{
    TrivialDialog d(L"grid", false);   // column 3 is made editable after the pick checks
    HWND dlg = d.CreateModeless(inst, IDD_TRIVIAL, nullptr);
    if (!dlg) {
        r.check(false, L"grid: dialog created");
        return;
    }
    ShowWindow(dlg, SW_SHOW);
    SetActiveWindow(dlg);
    fx::Grid grid(GetDlgItem(dlg, IDC_GRID));
    r.check(grid.IsGrid() && grid.list() != nullptr, L"tbSpread from a dialog template holds a ListView");
    r.check(grid.Columns() == 3 && grid.Rows() == 3, L"grid: SetColumns / SetData built 3 x 3");
    r.check(grid.GetData(1, 2) == L"102101" && grid.GetData(3, 3) == L"-16,840.00", L"grid: GetData round-trip");
    r.check(grid.GetData(4, 1).empty() && grid.GetData(1, 4).empty() && grid.GetData(0, 1).empty(),
            L"grid: GetData outside the grid is empty");

    grid.SetData(1, 5, L"999999");
    r.check(grid.Rows() == 5 && grid.GetData(1, 5) == L"999999" && grid.GetData(1, 4).empty(),
            L"grid: SetData past the last row adds rows");
    grid.SetMaxRows(3);
    r.check(grid.Rows() == 3 && grid.GetData(1, 3) == L"340000", L"grid: SetMaxRows removes rows from the end");

    {
        RECT rc{};
        GetClientRect(grid.list(), &rc);
        const int total = grid.ColumnWidth(1) + grid.ColumnWidth(2) + grid.ColumnWidth(3);
        r.check(total == rc.right - rc.left, L"grid: the stretch column takes the remaining width");
    }

    const int sel = d.selChanges;
    grid.SetActiveCell(1, 2);
    r.check(grid.ActiveRow() == 2 && grid.ActiveCol() == 1 && d.selChanges == sel + 1,
            L"grid: SetActiveCell selects the row, GRN_SELCHANGE sent");

    SetFocus(grid.list());
    key(grid.list(), VK_RETURN);   // column 1 isn't editable: a pick
    r.check(d.picks == 1 && grid.ActiveRow() == 2, L"grid: Enter on a row sends GRN_PICK");
    r.check((SendMessageW(grid.list(), WM_GETDLGCODE, VK_RETURN, 0) & DLGC_WANTARROWS) != 0,
            L"grid: the list keeps the arrow keys in a dialog");

    // Custom draw: what the list would paint for row 2, column 2.
    grid.SetColor(fx::Grid::kAll, fx::Grid::kAll, RGB(250, 250, 250), RGB(0, 0, 0));
    grid.SetColor(fx::Grid::kAll, 2, RGB(192, 220, 192), RGB(0, 0, 255));
    const auto paint = [&](int row) {
        NMLVCUSTOMDRAW cd{};
        cd.nmcd.hdr.hwndFrom = grid.list();
        cd.nmcd.hdr.idFrom = 1;
        cd.nmcd.hdr.code = NM_CUSTOMDRAW;
        cd.nmcd.dwDrawStage = CDDS_ITEMPREPAINT | CDDS_SUBITEM;
        cd.nmcd.dwItemSpec = static_cast<DWORD_PTR>(row - 1);
        cd.iSubItem = 1;
        SendMessageW(grid.hwnd(), WM_NOTIFY, 1, reinterpret_cast<LPARAM>(&cd));
        return std::make_pair(cd.clrTextBk, cd.clrText);
    };
    r.check(paint(2) == std::make_pair(RGB(192, 220, 192), RGB(0, 0, 255)), L"grid: a row colour is painted");
    r.check(paint(1) == std::make_pair(RGB(250, 250, 250), RGB(0, 0, 0)), L"grid: other rows get the default colour");
    grid.SetColor(2, 1, RGB(255, 255, 255), RGB(255, 0, 0));
    r.check(paint(1) == std::make_pair(RGB(255, 255, 255), RGB(255, 0, 0)), L"grid: a cell colour beats the row's");

    // Editing column 3 (tbFloat 9999999999.99 with separators).
    grid.SetColumnEditable(3, L"9999999999.99", 0x2000);   // FS_SEPARATOR
    r.check(!grid.BeginEdit(1, 1) && !grid.IsEditing(), L"grid: a column that isn't editable doesn't edit");
    r.check(grid.BeginEdit(3, 1) && grid.IsEditing(), L"grid: BeginEdit on an editable cell");
    HWND editor = GetDlgItem(grid.list(), 2);
    r.check(editor && fx::GetText(editor) == L"500.00", L"grid: the editor starts with the cell's value");
    typeText(editor, L"1234.5");
    key(editor, VK_RETURN);
    r.check(!grid.IsEditing() && grid.GetData(3, 1) == L"1,234.50", L"grid: Enter commits, in the mask's format");
    r.check(d.changes == 1 && grid.LastChangedRow() == 1 && grid.LastChangedCol() == 3,
            L"grid: GRN_CELLCHANGED with the changed cell");
    grid.BeginEdit(3, 1);
    typeText(GetDlgItem(grid.list(), 2), L"99");
    key(GetDlgItem(grid.list(), 2), VK_ESCAPE);
    r.check(!grid.IsEditing() && grid.GetData(3, 1) == L"1,234.50" && d.changes == 1, L"grid: Esc cancels the edit");
    grid.BeginEdit(3, 1);
    typeText(GetDlgItem(grid.list(), 2), L"7");
    key(GetDlgItem(grid.list(), 2), VK_DOWN);
    r.check(grid.IsEditing() && grid.ActiveRow() == 2 && grid.GetData(3, 1) == L"7.00",
            L"grid: Down commits and edits the cell below");
    grid.EndEdit(false);
    r.check(!grid.IsEditing() && grid.GetData(3, 2) == L"12,500.00", L"grid: EndEdit(false) leaves the cell");

    grid.Clear();
    r.check(grid.Rows() == 0 && grid.Columns() == 3, L"grid: Clear removes the rows, keeps the columns");

    HWND made = fx::Grid::Create(dlg, 1099, 0, 0, 100, 50);
    r.check(made && fx::Grid(made).IsGrid(), L"Grid::Create makes one in code");

    DestroyWindow(dlg);
    r.check(d.hwnd() == nullptr, L"grid: dialog destroyed");
}

void mdiInstances(HINSTANCE inst, Results& r)
{
    ProbeFrame frame;
    r.check(frame.CreateFrame(inst, L"TrivialProbeFrame", L"probe", nullptr, -1) != nullptr, L"MdiFrame created");
    if (!frame.hwnd()) return;
    ProbeChild one, two;
    HWND h1 = one.CreateChild(frame, L"TrivialProbeChild", L"one");
    HWND h2 = two.CreateChild(frame, L"TrivialProbeChild", L"two");
    r.check(h1 && h2 && h1 != h2, L"two MDI children of one window class");
    SendMessageW(h1, WM_COMMAND, 42, 0);
    SendMessageW(h2, WM_COMMAND, 42, 0);
    SendMessageW(h2, WM_COMMAND, 42, 0);
    r.check(one.commands == 1 && two.commands == 2, L"WM_COMMAND reaches the MDI child it was sent to");
    r.check(fx::Window::FromHandle(h1) == &one && fx::Window::FromHandle(h2) == &two,
            L"Window::FromHandle (lpParam arrived through MDICREATESTRUCT)");
    SendMessageW(frame.mdiClient(), WM_MDIACTIVATE, reinterpret_cast<WPARAM>(h1), 0);
    r.check(frame.lastActivated == h1 && frame.activeChild() == h1, L"OnMdiChildActivate hook");
    DestroyWindow(frame);
    r.check(one.hwnd() == nullptr && two.hwnd() == nullptr && frame.hwnd() == nullptr,
            L"frame and children detach when destroyed");
}

} // namespace

int WINAPI wWinMain(_In_ HINSTANCE inst, _In_opt_ HINSTANCE, _In_ LPWSTR cmdLine, _In_ int)
{
    if (!fx::RegisterCustomControls(inst)) return 100;
    const std::wstring args = cmdLine ? cmdLine : L"";
    if (args.find(L"/selftest") == std::wstring::npos) {
        TrivialDialog dlg(L"modal");
        return static_cast<int>(dlg.DoModal(inst, IDD_TRIVIAL, nullptr));
    }
    Results r;
    dialogInstances(inst, r);
    gridChecks(inst, r);
    mdiInstances(inst, r);
    if (args.find(L"/quiet") == std::wstring::npos) {
        r.log += r.failures ? fx::Format(L"\n%d check(s) FAILED", r.failures) : std::wstring(L"\nAll checks passed");
        MessageBoxW(nullptr, r.log.c_str(), L"win32fx self-test", r.failures ? MB_ICONERROR : MB_ICONINFORMATION);
    }
    return r.failures;
}
