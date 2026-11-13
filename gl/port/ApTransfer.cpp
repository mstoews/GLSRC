//
// ApTransfer.cpp - see ApTransfer.h.
//
// Translation notes (plan per-port checklist; decisions are spec #5 §8):
//   - D2: APUpdate netted every waiting AP_JSUM row into one GL voucher in the
//     GL's current period, whatever the rows' periods, dated today; the port
//     makes one per period, dated its end, and leaves a closed period's rows
//     waiting (reported). The voucher numbers come from Spec #2's SaveLine.
//   - D10: UpdateApBatch marked the posted invoice vouchers "2" whether or not
//     their rows went; cash vouchers were never marked. The port marks a
//     voucher of either kind when all its rows are transferred.
//   - The AR half (ARUpdate) is Spec #6: its box is disabled. The original's
//     boxes were BS_CHECKBOX, toggled by hand; the port checks the AP box at
//     the start and toggles it the same way.
//   - The status line (4095) showed each account as it went; the port shows
//     the result. Cancelling a transfer's GL voucher (Transactions > Cancel
//     Voucher) puts its rows back (Ap.h UndoApTransfer).
//
#include "ApTransfer.h"

#include "Ap.h"
#include "Globals.h"
#include "Ini.h"
#include "MainDialog.h"

#include <cstdlib>
#include <exception>
#include <string>

#include "LegacyIds.h"   // last: see LegacyIds.h

namespace {

constexpr int kAp = 1001;       // "Update Account Payable"
constexpr int kAr = 1002;       // "Update Accounts Receivable"
constexpr int kStatus = 4095;   // the status line

std::wstring money(std::int64_t cents)
{
    const std::int64_t a = cents < 0 ? -cents : cents;
    std::wstring whole = std::to_wstring(a / 100);
    for (int i = static_cast<int>(whole.size()) - 3; i > 0; i -= 3) whole.insert(static_cast<size_t>(i), L",");
    wchar_t frac[4];
    swprintf(frac, 4, L"%02lld", static_cast<long long>(a % 100));
    return (cents < 0 ? L"-" : L"") + whole + L"." + frac;
}

class TransferDialog : public MainDialog {
public:
    explicit TransferDialog(bool closeAtOnce) : closeAtOnce_(closeAtOnce) {}

protected:
    bool OnInitDialog() override
    {
        MainDialog::OnInitDialog();
        SetCheck(kAp, true);
        EnableWindow(GetDlgItem(hwnd(), kAr), FALSE);   // Spec #6
        EnsureApTables(cb);
        int waiting = 0;
        for (const JournalRow& r : LoadJournal(cb))
            if (r.mode == "1" || r.mode.empty()) ++waiting;
        SetFieldW(kStatus, fx::Format(L"%d AP journal row(s) waiting for the General Ledger.", waiting));
        if (closeAtOnce_) PostMessageW(hwnd(), WM_COMMAND, IDCANCEL, 0);
        return true;
    }

    bool OnCommand(WORD id, WORD code, HWND ctl) override
    {
        if (code != BN_CLICKED) return MainDialog::OnCommand(id, code, ctl);
        switch (id) {
        case kAp: SetCheck(kAp, !IsChecked(kAp)); return true;   // TGLAPUpdate::APCheck
        case IDOK: Ok(); return true;
        default: return MainDialog::OnCommand(id, code, ctl);
        }
    }

private:
    void Ok()   // TGLAPUpdate::Ok
    {
        if (!IsChecked(kAp)) {
            Cancel();
            return;
        }
        ApTransferResult r;
        try {
            const long iniBatchno = std::atol(fx::WideToUtf8(GetIniString(L"GL.INI", L"PERIOD", L"BATCHNO")).c_str());
            r = TransferAp(cb, iniBatchno);
        } catch (const std::exception& e) {
            Message(fx::Format(L"The transfer failed, and nothing was changed.\n\n%s", fx::Utf8ToWide(e.what()).c_str()),
                    MB_ICONSTOP);
            return;
        }
        std::wstring text;
        for (const ApTransferVoucher& v : r.vouchers) {
            if (v.glVoucher)
                text += fx::Format(L"Period %d: GL voucher %ld, %d line(s), %s each way.\n", v.period, v.glVoucher,
                                   v.lines, money(v.debit).c_str());
            else
                text += fx::Format(L"Period %d: the rows net to nothing; no voucher.\n", v.period);
        }
        for (int p : r.closedPeriods)
            text += fx::Format(L"Period %d is closed: its rows are left waiting.\n", p);
        for (int p : r.undatedPeriods)
            text += fx::Format(L"Period %d has no end date in Company Profile > Period Dates: its rows are left "
                               L"waiting.\n",
                               p);
        if (text.empty()) text = L"There is nothing to transfer.\n";
        else if (!r.vouchers.empty()) text += L"\nPost the voucher(s) with Transactions > Post Voucher > General Ledger Voucher.";
        Message(text, MB_ICONINFORMATION);
        Cancel();   // cb.closeAll(); cb.init(); ShutDownWindow()
    }

    bool closeAtOnce_;
};

} // namespace

INT_PTR RunApTransfer(HWND frame, bool closeAtOnce)
{
    TransferDialog dlg(closeAtOnce);
    return dlg.DoModal(1030, frame);
}
