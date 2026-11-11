//
// DepreciationRun.cpp - see DepreciationRun.h.
//
// Translation notes (plan per-port checklist; decisions are spec #4 §8):
//   - D2-D5, D10: Assets.h RunDepreciation replaces TGLCreateDeprSlip::Ok:
//     every asset due, every month; one run per period; the voucher numbered
//     NextVoucherNumber, dated the period's end, with its totals set and its
//     lines numbered 1, 2, 3, ...; the salvage floor.
//   - The dialog shows what the run will do (Assets.h PlanRun) before OK, in
//     4095, the label the original wrote the voucher number into while it ran.
//     OK's question (8110) lists the assets due and those skipped: disposed,
//     fully depreciated, not yet bought, or whose type is missing (8120) or
//     points at an account that isn't in the chart.
//   - The period (4001) is the current period, as GetCurrentPeriod gave it;
//     it can't be typed over (the original's tbPic could be, to no effect).
//   - The caption's "Deprecation" is "Depreciation" (spec #4 6.4).
//   - Cancel didn't need cb.closeAll(); the port closes nothing.
//
#include "DepreciationRun.h"

#include "Accounts.h"   // TrimText
#include "Assets.h"
#include "Globals.h"
#include "Ini.h"
#include "MainDialog.h"
#include "Vouchers.h"

#include "report/Format.h"   // FormatAmount

#include "cb4sqlite.h"

#include <cstdlib>
#include <exception>
#include <string>
#include <vector>

#include "LegacyIds.h"   // last: see LegacyIds.h

namespace {

constexpr int kPeriod = 4001;
constexpr int kStatus = 4095;

std::wstring amount(std::int64_t cents) { return fx::Utf8ToWide(rpt::FormatAmount(cents)); }

const wchar_t* reasonText(SkipReason r)
{
    switch (r) {
    case SkipReason::Disposed: return L"disposed";
    case SkipReason::FullyDepreciated: return L"fully depreciated";
    case SkipReason::NoType: return L"its type isn't there";
    case SkipReason::BadAccount: return L"its type's account isn't in the chart";
    case SkipReason::NotYetBought: return L"bought after the period's end";
    }
    return L"";
}

class DepreciationRunDialog : public MainDialog {
public:
    explicit DepreciationRunDialog(bool closeAtOnce) : closeAtOnce_(closeAtOnce) {}

protected:
    bool OnInitDialog() override
    {
        MainDialog::OnInitDialog();
        SetWindowTextW(hwnd(), L"Create Current Month Depreciation Voucher");
        EnsureAssetTables(cb);
        const std::vector<OpenPeriod> open = OpenPeriods(cb);   // last first; the current period is the lowest
        if (!open.empty()) {
            period_ = open.back().number;
            std::wstring p = std::to_wstring(period_);
            if (!open.back().description.empty()) p += L" (" + fx::Utf8ToWide(open.back().description) + L")";
            SetFieldW(kPeriod, p);
        }
        SendDlgItemMessageW(hwnd(), kPeriod, EM_SETREADONLY, TRUE, 0);
        Describe();
        if (closeAtOnce_) PostMessageW(hwnd(), WM_COMMAND, IDCANCEL, 0);
        return true;
    }

    bool OnCommand(WORD id, WORD code, HWND ctl) override
    {
        if (code == BN_CLICKED && id == IDOK) {
            Ok();
            return true;
        }
        return MainDialog::OnCommand(id, code, ctl);   // IDCANCEL -> Cancel()
    }

private:
    // The label under the period: what OK will do.
    void Describe()
    {
        if (period_ == 0) {
            SetFieldW(kStatus, L"No period is open.");
            EnableWindow(GetDlgItem(hwnd(), IDOK), FALSE);
            return;
        }
        plan_ = PlanRun(cb, period_);
        std::wstring s;
        if (plan_.alreadyRun)
            s = plan_.existingVoucher ? fx::Format(L"Period %d has been run: voucher %ld.", period_, plan_.existingVoucher)
                                      : fx::Format(L"Period %d's depreciation is already booked.", period_);
        else if (plan_.due.empty())
            s = L"No asset is due.";
        else
            s = fx::Format(L"%d asset%s due: %s.", static_cast<int>(plan_.due.size()), plan_.due.size() == 1 ? L"" : L"s",
                           amount(plan_.total).c_str());
        if (!plan_.alreadyRun && !plan_.skipped.empty())
            s += fx::Format(L" %d skipped.", static_cast<int>(plan_.skipped.size()));
        status_ = s;
        SetFieldW(kStatus, s);
    }

    std::wstring Details() const
    {
        std::wstring s;
        for (const RunItem& i : plan_.due)
            s += fx::Format(L"\n    %s  %s  %s", fx::Utf8ToWide(TrimText(i.asset.code)).c_str(),
                            fx::Utf8ToWide(TrimText(i.asset.description)).c_str(), amount(i.amount).c_str());
        if (!plan_.skipped.empty()) s += L"\n\nSkipped:";
        for (const RunSkip& k : plan_.skipped)
            s += fx::Format(L"\n    %s  %s: %s", fx::Utf8ToWide(TrimText(k.asset.code)).c_str(),
                            fx::Utf8ToWide(TrimText(k.asset.description)).c_str(), reasonText(k.reason));
        return s;
    }

    void Ok()   // TGLCreateDeprSlip::Ok
    {
        Describe();   // the books may have changed since the dialog opened
        if (plan_.alreadyRun) {
            std::wstring text = status_;
            if (plan_.existingVoucher) text += L"\n\nTo run it again, cancel that voucher first (Transactions > Cancel Voucher).";
            Message(text, MB_ICONINFORMATION);
            return;
        }
        if (plan_.due.empty()) {
            Message(L"No asset is due for depreciation in this period." + Details(), MB_ICONINFORMATION);
            return;
        }
        // 8110: "Creating a new depreciation journal entry. Are you sure ...?"
        const std::wstring ask = GetString(8110) + fx::Format(L"\n\nPeriod %d, %s:", period_, amount(plan_.total).c_str()) +
                                 Details();
        if (Message(ask, MB_ICONQUESTION | MB_YESNO) != IDYES) return;
        try {
            const long ini = std::atol(fx::WideToUtf8(GetIniString(L"GL.INI", L"PERIOD", L"BATCHNO")).c_str());
            const RunResult r = RunDepreciation(cb, period_, ini);
            switch (r.status) {
            case RunStatus::Created:
                Message(fx::Format(L"Voucher %ld created: %d asset%s, %s.\n\n"
                                   L"Post it with Transactions > Post Voucher. To undo the run, cancel the voucher.",
                                   r.batchno, r.assets, r.assets == 1 ? L"" : L"s", amount(r.total).c_str()),
                        MB_ICONINFORMATION);
                End(IDOK);   // the original closed after the run
                return;
            case RunStatus::AlreadyRun:
            case RunStatus::NothingDue: Describe(); return;
            case RunStatus::NoPeriodEnd:
                Message(fx::Format(L"Period %d has no end date. Set it in Company Profile > Periods.", period_),
                        MB_ICONINFORMATION);
                return;
            }
        } catch (const std::exception& e) {
            Message(L"The voucher could not be created; nothing was changed.\n\n" + fx::Utf8ToWide(e.what()),
                    MB_ICONSTOP);
        }
    }

    bool closeAtOnce_;
    int period_ = 0;
    RunPlan plan_;
    std::wstring status_;
};

} // namespace

INT_PTR RunCreateDepreciationVoucher(HWND frame, bool closeAtOnce)
{
    DepreciationRunDialog dlg(closeAtOnce);
    return dlg.DoModal(4003, frame);
}
