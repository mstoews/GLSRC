//
// StatementLayoutDialog.cpp - see StatementLayoutDialog.h.
//
#include "StatementLayoutDialog.h"

#include "RunReport.h"        // CurrentReportContext
#include "StatementLayout.h"
#include "Statements.h"

#include "../Globals.h"
#include "../MainDialog.h"
#include "../PortIds.h"

#include "report/win32/Preview.h"

#include "cb4sqlite.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <optional>
#include <string>
#include <vector>

namespace {

// The statements in the picker's order.
struct StatementChoice {
    const char* code;
    const wchar_t* name;
};
const StatementChoice kStatements[] = {
    {kIncomeStatement, L"Income Statement"}, {kBalanceSheet, L"Balance Sheet"}, {kCashFlow, L"Cash Flow Statement"}};

// The kinds in the Kind combo's order (spec #3 6.1).
const char kKinds[] = "HATNOES";
const wchar_t* const kKindNames[] = {L"Heading",      L"Accounts",     L"Total",     L"Net income",
                                     L"Opening cash", L"Closing cash", L"Blank line"};

const wchar_t* kindName(char k)
{
    for (int i = 0; kKinds[i]; ++i)
        if (kKinds[i] == k) return kKindNames[i];
    return L"?";
}

bool hasAccounts(char k) { return k == 'A' || k == 'O' || k == 'E'; }

std::wstring wide(const std::string& s) { return fx::Utf8ToWide(s); }

std::string stamp()
{
    SYSTEMTIME t{};
    GetLocalTime(&t);
    char buf[24] = {};
    std::snprintf(buf, sizeof buf, "%04u-%02u-%02u %02u:%02u", t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute);
    return buf;
}

class StatementLayoutDialog : public MainDialog {
public:
    explicit StatementLayoutDialog(bool closeAtOnce) : closeAtOnce_(closeAtOnce) {}

protected:
    bool OnInitDialog() override
    {
        EnsureLayoutTables(cb);
        for (const StatementChoice& s : kStatements)
            SendDlgItemMessageW(hwnd(), IDC_SL_STATEMENT, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(s.name));
        SendDlgItemMessageW(hwnd(), IDC_SL_STATEMENT, CB_SETCURSEL, 0, 0);
        for (const wchar_t* k : kKindNames)
            SendDlgItemMessageW(hwnd(), IDC_SL_KIND, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(k));
        for (const wchar_t* s : {L"+1", L"-1"})
            SendDlgItemMessageW(hwnd(), IDC_SL_SIGN, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(s));
        for (const wchar_t* s : {L"0", L"1", L"2", L"3"})
            SendDlgItemMessageW(hwnd(), IDC_SL_INDENT, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(s));

        grid_ = fx::Grid(GetDlgItem(hwnd(), IDC_SL_GRID));
        if (grid_.IsGrid()) {
            const int ch = grid_.CharWidth();
            grid_.SetColumns({{L"No.", 6 * ch},
                              {L"Kind", 11 * ch},
                              {L"Caption", 30 * ch},
                              {L"Sign", 5 * ch, fx::GridAlign::Right},
                              {L"Formula", 12 * ch},
                              {L"Ranges", 24 * ch}});
            grid_.SetStretchColumn(3);
        }
        Load(0);
        if (closeAtOnce_) PostMessageW(hwnd(), WM_COMMAND, IDCANCEL, 0);
        return true;
    }

    bool OnCommand(WORD id, WORD code, HWND ctl) override
    {
        if (id == IDC_SL_GRID) {
            if (code == GRN_SELCHANGE || code == GRN_PICK) ShowLine(grid_.ActiveRow());
            return true;
        }
        if (id == IDC_SL_STATEMENT && code == CBN_SELCHANGE) {
            ChangeStatement();
            return true;
        }
        if (id == IDC_SL_KIND && code == CBN_SELCHANGE) {
            EnableFields(SelectedKind());
            return true;
        }
        if (code != BN_CLICKED) return MainDialog::OnCommand(id, code, ctl);
        switch (id) {
        case IDOK:   // Enter in a field
        case IDC_SL_APPLY: Apply(); return true;
        case IDC_SL_ADD: AddLine(false); return true;
        case IDC_SL_INSERT: AddLine(true); return true;
        case IDC_SL_DELETE: DeleteLine(); return true;
        case IDC_SL_CHECK: Check(); return true;
        case IDC_SL_PREVIEW: Preview(); return true;
        case IDC_SL_RESET: Reset(); return true;
        case IDC_SL_SAVE: Save(); return true;
        case IDCANCEL:
            if (closeAtOnce_ || AskToSave()) End(IDCANCEL);
            return true;
        default: return MainDialog::OnCommand(id, code, ctl);
        }
    }

private:
    // ---- the layout -------------------------------------------------------

    const char* Statement() const { return kStatements[statement_].code; }

    void Load(int statement)
    {
        statement_ = statement;
        lines_ = LoadLayout(cb, Statement());
        dirty_ = false;
        Fill(lines_.empty() ? 0 : 1);
    }

    // Loads the grid and selects `row` (1-based; 0: none).
    void Fill(int row)
    {
        grid_.SetRedraw(false);
        grid_.Clear();
        int r = 1;
        for (const LayoutLine& l : lines_) {
            grid_.SetData(1, r, std::to_wstring(l.lineNo));
            grid_.SetData(2, r, kindName(l.kind));
            grid_.SetData(3, r, std::wstring(static_cast<size_t>(l.indent) * 3, L' ') + wide(l.caption));
            grid_.SetData(4, r, hasAccounts(l.kind) ? (l.sign < 0 ? L"-1" : L"+1") : L"");
            grid_.SetData(5, r, wide(l.formula));
            grid_.SetData(6, r, wide(FormatRanges(l.ranges)));
            ++r;
        }
        grid_.SetRedraw(true);
        row = std::min(row, static_cast<int>(lines_.size()));
        if (row > 0) grid_.SetActiveCell(1, row);
        ShowLine(row);
        UpdateCaption();
    }

    void UpdateCaption()
    {
        std::wstring c = L"Statement Layout";
        if (dirty_) c += L" (not saved)";
        SetWindowTextW(hwnd(), c.c_str());
    }

    char SelectedKind() const
    {
        const LRESULT i = SendDlgItemMessageW(hwnd(), IDC_SL_KIND, CB_GETCURSEL, 0, 0);
        return i >= 0 && i < 7 ? kKinds[i] : 'S';
    }

    void EnableFields(char kind)
    {
        EnableWindow(GetDlgItem(hwnd(), IDC_SL_SIGN), hasAccounts(kind));
        EnableWindow(GetDlgItem(hwnd(), IDC_SL_RANGES), hasAccounts(kind));
        EnableWindow(GetDlgItem(hwnd(), IDC_SL_FORMULA), kind == 'T');
    }

    // The fields for line `row` (1-based); 0 clears them.
    void ShowLine(int row)
    {
        selected_ = row;
        const LayoutLine blank;
        const LayoutLine& l = row >= 1 && row <= static_cast<int>(lines_.size()) ? lines_[static_cast<size_t>(row - 1)] : blank;
        SetFieldW(IDC_SL_LINENO, row > 0 ? std::to_wstring(l.lineNo) : L"");
        const size_t kind = std::string(kKinds).find(l.kind);
        SendDlgItemMessageW(hwnd(), IDC_SL_KIND, CB_SETCURSEL, kind == std::string::npos ? 0 : kind, 0);
        SendDlgItemMessageW(hwnd(), IDC_SL_SIGN, CB_SETCURSEL, l.sign < 0 ? 1 : 0, 0);
        SendDlgItemMessageW(hwnd(), IDC_SL_INDENT, CB_SETCURSEL, static_cast<WPARAM>(std::clamp(l.indent, 0, 3)), 0);
        SetField(IDC_SL_CAPTION, l.caption);
        SetField(IDC_SL_FORMULA, l.formula);
        SetField(IDC_SL_RANGES, FormatRanges(l.ranges));
        EnableFields(row > 0 ? l.kind : 'H');
    }

    // The fields as a line; nullopt (with a message) if the ranges don't parse.
    // `lineNo` 0 when the field is blank.
    std::optional<LayoutLine> ReadFields()
    {
        LayoutLine l;
        l.lineNo = std::atoi(StrTrim(GetField(IDC_SL_LINENO)).c_str());
        l.kind = SelectedKind();
        l.sign = SendDlgItemMessageW(hwnd(), IDC_SL_SIGN, CB_GETCURSEL, 0, 0) == 1 ? -1 : 1;
        l.indent = static_cast<int>(std::max<LRESULT>(0, SendDlgItemMessageW(hwnd(), IDC_SL_INDENT, CB_GETCURSEL, 0, 0)));
        l.caption = StrTrim(GetField(IDC_SL_CAPTION));
        if (l.kind == 'T') l.formula = StrTrim(GetField(IDC_SL_FORMULA));
        if (hasAccounts(l.kind)) {
            const std::optional<std::vector<AccountRange>> r = ParseRanges(GetField(IDC_SL_RANGES));
            if (!r) {
                Message(L"The ranges can't be read. Write them as 410000-419999; 420000-429999, or one account "
                        L"such as 999000.",
                        MB_ICONINFORMATION);
                SetFocus(GetDlgItem(hwnd(), IDC_SL_RANGES));
                return std::nullopt;
            }
            l.ranges = *r;
        } else {
            l.sign = 1;
        }
        return l;
    }

    void Changed(int row)
    {
        dirty_ = true;
        Fill(row);
    }

    // The row of line number `n` after sorting, 1-based.
    int RowOf(int n) const
    {
        for (size_t i = 0; i < lines_.size(); ++i)
            if (lines_[i].lineNo == n) return static_cast<int>(i) + 1;
        return 0;
    }

    void Sort()
    {
        std::stable_sort(lines_.begin(), lines_.end(),
                         [](const LayoutLine& a, const LayoutLine& b) { return a.lineNo < b.lineNo; });
    }

    // ---- the line buttons -------------------------------------------------

    void Apply()
    {
        if (selected_ < 1) {
            AddLine(false);
            return;
        }
        std::optional<LayoutLine> l = ReadFields();
        if (!l) return;
        LayoutLine& target = lines_[static_cast<size_t>(selected_ - 1)];
        if (l->lineNo == 0) l->lineNo = target.lineNo;
        l->updated = target.updated;
        target = *l;
        Sort();
        Changed(RowOf(l->lineNo));
    }

    void AddLine(bool before)
    {
        std::optional<LayoutLine> l = ReadFields();
        if (!l) return;
        if (l->lineNo == 0) {
            if (!before || selected_ < 1) {
                l->lineNo = lines_.empty() ? 10 : lines_.back().lineNo + 10;
            } else {
                const int next = lines_[static_cast<size_t>(selected_ - 1)].lineNo;
                const int prev = selected_ > 1 ? lines_[static_cast<size_t>(selected_ - 2)].lineNo : 0;
                if (next - prev < 2) {
                    Message(fx::Format(L"There is no free line number between %d and %d. Type one in No.; "
                                       L"line numbers aren't changed for you, as totals refer to them.",
                                       prev, next),
                            MB_ICONINFORMATION);
                    SetFocus(GetDlgItem(hwnd(), IDC_SL_LINENO));
                    return;
                }
                l->lineNo = prev + (next - prev) / 2;
            }
        }
        const int n = l->lineNo;
        lines_.push_back(*l);
        Sort();
        Changed(RowOf(n));
    }

    void DeleteLine()
    {
        if (selected_ < 1) return;
        const LayoutLine& l = lines_[static_cast<size_t>(selected_ - 1)];
        if (Message(fx::Format(L"Delete line %d (%s)?", l.lineNo, wide(l.caption).c_str()), MB_ICONQUESTION | MB_YESNO) != IDYES)
            return;
        const int row = selected_;
        lines_.erase(lines_.begin() + (row - 1));
        Changed(std::min(row, static_cast<int>(lines_.size())));
    }

    // ---- the layout buttons -----------------------------------------------

    std::vector<LayoutProblem> Problems() const { return CheckLayout(Statement(), lines_, LoadBalances(cb)); }

    static std::wstring List(const std::vector<LayoutProblem>& problems, bool errors)
    {
        std::wstring s;
        for (const LayoutProblem& p : problems)
            if (p.error == errors) s += L"\n  - " + wide(p.text);
        return s;
    }

    void Check()
    {
        const std::vector<LayoutProblem> p = Problems();
        if (p.empty()) {
            Message(L"No problems found.", MB_ICONINFORMATION);
            return;
        }
        std::wstring s;
        if (const std::wstring e = List(p, true); !e.empty()) s += L"Errors (Save refuses these):" + e + L"\n\n";
        if (const std::wstring w = List(p, false); !w.empty()) s += L"Warnings:" + w;
        Message(s, MB_ICONINFORMATION);
    }

    void Preview()
    {
        try {
            const ReportContext ctx = CurrentReportContext();
            StatementOptions opt;
            opt.layout = lines_;
            const std::string s = Statement();
            const rpt::Report r = s == kIncomeStatement ? IncomeStatement(cb, ctx, opt)
                                  : s == kBalanceSheet  ? BalanceSheet(cb, ctx, opt)
                                                        : CashFlowStatement(cb, ctx, opt);
            rpt::win32::ShowPreviewWindow(hwnd(), r);
        } catch (const std::exception& e) {
            Message(L"The statement could not be prepared:\n\n" + wide(e.what()), MB_ICONSTOP);
        }
    }

    void Reset()
    {
        if (Message(fx::Format(L"Replace the %s's layout with the default?\n\nThis is saved at once; changes not "
                               L"saved are lost.",
                               kStatements[statement_].name),
                    MB_ICONQUESTION | MB_YESNO) != IDYES)
            return;
        ResetLayout(cb, Statement());
        Load(statement_);
    }

    // True if saved, or nothing to save.
    bool Save()
    {
        const std::vector<LayoutProblem> p = Problems();
        if (const std::wstring e = List(p, true); !e.empty()) {
            Message(L"The layout can't be saved while it has errors:" + e, MB_ICONEXCLAMATION);
            return false;
        }
        if (const std::wstring w = List(p, false); !w.empty())
            if (Message(L"Warnings:" + w + L"\n\nSave anyway?", MB_ICONQUESTION | MB_YESNO) != IDYES) return false;
        try {
            SaveLayout(cb, Statement(), lines_, stamp());
        } catch (const std::exception& e) {
            Message(L"The layout could not be saved; nothing was changed.\n\n" + wide(e.what()), MB_ICONSTOP);
            return false;
        }
        const int row = selected_;
        Load(statement_);
        Fill(row);
        return true;
    }

    // Before leaving the statement shown: true to go on.
    bool AskToSave()
    {
        if (!dirty_) return true;
        const int a = Message(fx::Format(L"Save the changes to the %s's layout?", kStatements[statement_].name),
                              MB_ICONQUESTION | MB_YESNOCANCEL);
        if (a == IDCANCEL) return false;
        if (a == IDYES) return Save();
        return true;
    }

    void ChangeStatement()
    {
        const int want = static_cast<int>(SendDlgItemMessageW(hwnd(), IDC_SL_STATEMENT, CB_GETCURSEL, 0, 0));
        if (want == statement_ || want < 0) return;
        if (!AskToSave()) {
            SendDlgItemMessageW(hwnd(), IDC_SL_STATEMENT, CB_SETCURSEL, static_cast<WPARAM>(statement_), 0);
            return;
        }
        Load(want);
    }

    bool closeAtOnce_;
    fx::Grid grid_;
    int statement_ = 0;
    std::vector<LayoutLine> lines_;
    int selected_ = 0;   // 1-based row of the line in the fields; 0: none
    bool dirty_ = false;
};

} // namespace

INT_PTR RunStatementLayout(HWND frame, bool closeAtOnce)
{
    StatementLayoutDialog dlg(closeAtOnce);
    return dlg.DoModal(IDD_STMT_LAYOUT, frame);
}

int StressStatementLayout(int times)
{
    auto objects = [] {
        HANDLE self = GetCurrentProcess();
        return static_cast<int>(GetGuiResources(self, GR_GDIOBJECTS) + GetGuiResources(self, GR_USEROBJECTS));
    };
    if (RunStatementLayout(nullptr, true) == -1) return -1;   // first open: class registration
    const int before = objects();
    for (int i = 0; i < times; ++i)
        if (RunStatementLayout(nullptr, true) == -1) return -1;
    return objects() - before;
}
