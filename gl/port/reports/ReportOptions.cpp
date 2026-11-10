//
// ReportOptions.cpp - see ReportOptions.h.
//
#include "ReportOptions.h"

#include "../Globals.h"
#include "../PortIds.h"

#include "win32fx.h"

#include <algorithm>
#include <exception>
#include <string>
#include <vector>

namespace {

// Fills a period combo with the 12 periods ("7  Oct 26") and selects `select`.
void fillPeriods(HWND combo, const ReportContext& ctx, int select)
{
    SendMessageW(combo, CB_RESETCONTENT, 0, 0);
    for (int p = 1; p <= 12; ++p) {
        std::wstring text = std::to_wstring(p);
        if (const FiscalPeriod* fp = ctx.period(p)) {
            if (!fp->description.empty()) text += L"   " + fx::Utf8ToWide(fp->description);
            if (p == ctx.currentPeriod && ctx.periodOpen) text += L"   (current)";
        }
        SendMessageW(combo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(text.c_str()));
    }
    SendMessageW(combo, CB_SETCURSEL, static_cast<WPARAM>(std::clamp(select, 1, 12) - 1), 0);
}

int selectedPeriod(HWND combo)
{
    const LRESULT i = SendMessageW(combo, CB_GETCURSEL, 0, 0);
    return i == CB_ERR ? 0 : static_cast<int>(i) + 1;
}

// IDD_TB_OPTIONS for F1 (dist == nullptr) or F2.
class TrialBalanceDialog : public fx::Dialog {
public:
    TrialBalanceDialog(const ReportContext& ctx, TrialBalanceOptions* tb, DistTrialBalanceOptions* dist)
        : ctx_(ctx), tb_(tb), dist_(dist) {}

protected:
    bool OnInitDialog() override
    {
        const int current = ctx_.currentPeriod;
        if (tb_) {
            SetWindowTextW(hwnd(), L"Trial Balance");
            SetDlgItemTextW(hwnd(), IDC_TB_FROM_LABEL, L"&Period:");
            SetDlgItemTextW(hwnd(), IDC_TB_ZEROS, L"&Include accounts with a zero balance");
            for (int id : {IDC_TB_TO_LABEL, IDC_TB_TO, IDC_TB_LINES}) ShowWindow(GetDlgItem(hwnd(), id), SW_HIDE);
            fillPeriods(GetDlgItem(hwnd(), IDC_TB_FROM), ctx_, tb_->period > 0 ? tb_->period : current);
            CheckDlgButton(hwnd(), IDC_TB_ZEROS, tb_->includeZeros ? BST_CHECKED : BST_UNCHECKED);
        } else {
            SetWindowTextW(hwnd(), L"Distribution Trial Balance");
            fillPeriods(GetDlgItem(hwnd(), IDC_TB_FROM), ctx_, dist_->from > 0 ? dist_->from : current);
            fillPeriods(GetDlgItem(hwnd(), IDC_TB_TO), ctx_, dist_->to > 0 ? dist_->to : current);
            CheckDlgButton(hwnd(), IDC_TB_LINES, dist_->voucherLines ? BST_CHECKED : BST_UNCHECKED);
            CheckDlgButton(hwnd(), IDC_TB_ZEROS, dist_->includeInactive ? BST_CHECKED : BST_UNCHECKED);
        }
        return true;
    }

    bool OnCommand(WORD id, WORD, HWND) override
    {
        if (id != IDOK) return false;
        const int from = selectedPeriod(GetDlgItem(hwnd(), IDC_TB_FROM));
        const bool zeros = IsDlgButtonChecked(hwnd(), IDC_TB_ZEROS) == BST_CHECKED;
        if (tb_) {
            tb_->period = from;
            tb_->includeZeros = zeros;
        } else {
            const int to = selectedPeriod(GetDlgItem(hwnd(), IDC_TB_TO));
            if (to < from) {
                MessageBoxW(hwnd(), L"The To period comes before the From period.", szApp, MB_OK | MB_ICONEXCLAMATION);
                SetFocus(GetDlgItem(hwnd(), IDC_TB_TO));
                return true;
            }
            dist_->from = from;
            dist_->to = to;
            dist_->voucherLines = IsDlgButtonChecked(hwnd(), IDC_TB_LINES) == BST_CHECKED;
            dist_->includeInactive = zeros;
        }
        End(IDOK);
        return true;
    }

private:
    const ReportContext& ctx_;
    TrialBalanceOptions* tb_;
    DistTrialBalanceOptions* dist_;
};

// IDD_STMT_OPTIONS for F3 (with department and comparisons), F4 and F5.
class StatementDialog : public fx::Dialog {
public:
    StatementDialog(const ReportContext& ctx, const wchar_t* title, bool income, StatementOptions& opt)
        : ctx_(ctx), title_(title), income_(income), opt_(opt) {}

protected:
    bool OnInitDialog() override
    {
        SetWindowTextW(hwnd(), title_);
        fillPeriods(GetDlgItem(hwnd(), IDC_ST_PERIOD), ctx_, opt_.period > 0 ? opt_.period : ctx_.currentPeriod);
        if (income_) {
            HWND dept = GetDlgItem(hwnd(), IDC_ST_DEPT);
            SendMessageW(dept, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"All departments"));
            depts_.push_back({});
            try {
                cb4sqlite::Query q(cb, "SELECT depart, descript FROM gldept WHERE _deleted = 0 ORDER BY depart");
                while (q.next()) {
                    depts_.push_back(q.text(0));
                    const std::wstring text = fx::Utf8ToWide(q.text(0) + "   " + q.text(1));
                    SendMessageW(dept, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(text.c_str()));
                }
            } catch (const std::exception&) {
                // No gldept: all departments only.
            }
            const auto it = std::find(depts_.begin(), depts_.end(), opt_.dept);
            SendMessageW(dept, CB_SETCURSEL, it == depts_.end() ? 0 : static_cast<WPARAM>(it - depts_.begin()), 0);
            CheckDlgButton(hwnd(), IDC_ST_PRIOR, opt_.comparePrior ? BST_CHECKED : BST_UNCHECKED);
            CheckDlgButton(hwnd(), IDC_ST_BUDGET, opt_.compareBudget ? BST_CHECKED : BST_UNCHECKED);
        } else {
            for (int id : {IDC_ST_DEPT_LABEL, IDC_ST_DEPT, IDC_ST_PRIOR, IDC_ST_BUDGET})
                ShowWindow(GetDlgItem(hwnd(), id), SW_HIDE);
        }
        CheckDlgButton(hwnd(), IDC_ST_ACCOUNTS, opt_.showAccounts ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(hwnd(), IDC_ST_ZEROS, opt_.showZeroLines ? BST_CHECKED : BST_UNCHECKED);
        return true;
    }

    bool OnCommand(WORD id, WORD, HWND) override
    {
        if (id != IDOK) return false;
        opt_.period = selectedPeriod(GetDlgItem(hwnd(), IDC_ST_PERIOD));
        if (income_) {
            const LRESULT d = SendDlgItemMessageW(hwnd(), IDC_ST_DEPT, CB_GETCURSEL, 0, 0);
            opt_.dept = d > 0 && static_cast<size_t>(d) < depts_.size() ? depts_[static_cast<size_t>(d)] : std::string();
            opt_.comparePrior = IsDlgButtonChecked(hwnd(), IDC_ST_PRIOR) == BST_CHECKED;
            opt_.compareBudget = IsDlgButtonChecked(hwnd(), IDC_ST_BUDGET) == BST_CHECKED;
        }
        opt_.showAccounts = IsDlgButtonChecked(hwnd(), IDC_ST_ACCOUNTS) == BST_CHECKED;
        opt_.showZeroLines = IsDlgButtonChecked(hwnd(), IDC_ST_ZEROS) == BST_CHECKED;
        End(IDOK);
        return true;
    }

private:
    const ReportContext& ctx_;
    const wchar_t* title_;
    bool income_;
    StatementOptions& opt_;
    std::vector<std::string> depts_;   // index 0: all departments
};

class SampleOptionsDialog : public fx::Dialog {
public:
    explicit SampleOptionsDialog(SampleOptions& opt) : opt_(opt) {}

protected:
    bool OnInitDialog() override
    {
        CheckDlgButton(hwnd(), IDC_SAMPLE_ZEROS, opt_.includeZeros ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(hwnd(), IDC_SAMPLE_LANDSCAPE, opt_.landscape ? BST_CHECKED : BST_UNCHECKED);
        SendDlgItemMessageW(hwnd(), IDC_SAMPLE_REPEAT, EM_LIMITTEXT, 2, 0);
        SetDlgItemInt(hwnd(), IDC_SAMPLE_REPEAT, static_cast<UINT>(opt_.repeat), FALSE);
        return true;
    }

    bool OnCommand(WORD id, WORD, HWND) override
    {
        if (id != IDOK) return false;
        BOOL ok = FALSE;
        const UINT repeat = GetDlgItemInt(hwnd(), IDC_SAMPLE_REPEAT, &ok, FALSE);
        if (!ok || repeat < 1 || repeat > 50) {
            MessageBoxW(hwnd(), L"Enter a number from 1 to 50.", szApp, MB_OK | MB_ICONEXCLAMATION);
            HWND edit = GetDlgItem(hwnd(), IDC_SAMPLE_REPEAT);
            SetFocus(edit);
            SendMessageW(edit, EM_SETSEL, 0, -1);
            return true;
        }
        opt_.includeZeros = IsDlgButtonChecked(hwnd(), IDC_SAMPLE_ZEROS) == BST_CHECKED;
        opt_.landscape = IsDlgButtonChecked(hwnd(), IDC_SAMPLE_LANDSCAPE) == BST_CHECKED;
        opt_.repeat = static_cast<int>(repeat);
        End(IDOK);
        return true;
    }

private:
    SampleOptions& opt_;
};

} // namespace

bool ShowSampleOptions(HWND owner, SampleOptions& opt)
{
    SampleOptionsDialog dlg(opt);
    return dlg.DoModal(hInst, IDD_SAMPLE_OPTIONS, owner) == IDOK;
}

bool ShowTrialBalanceOptions(HWND owner, const ReportContext& ctx, TrialBalanceOptions& opt)
{
    TrialBalanceDialog dlg(ctx, &opt, nullptr);
    return dlg.DoModal(hInst, IDD_TB_OPTIONS, owner) == IDOK;
}

bool ShowDistTrialBalanceOptions(HWND owner, const ReportContext& ctx, DistTrialBalanceOptions& opt)
{
    TrialBalanceDialog dlg(ctx, nullptr, &opt);
    return dlg.DoModal(hInst, IDD_TB_OPTIONS, owner) == IDOK;
}

bool ShowStatementOptions(HWND owner, const ReportContext& ctx, const wchar_t* title, bool incomeStatement,
                          StatementOptions& opt)
{
    StatementDialog dlg(ctx, title, incomeStatement, opt);
    return dlg.DoModal(hInst, IDD_STMT_OPTIONS, owner) == IDOK;
}
