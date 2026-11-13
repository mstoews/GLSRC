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
//   - The AR half (ARUpdate, spec #6 4.4, S6-M3) runs after AP, into its own
//     vouchers, by Ar.h TransferAr: the same per-period vouchers, described
//     "Accounts Receivable GL Update". The original wrote GL_SUM.TYPE "AP"
//     for AR's rows, used the GL's current period and today's date, and
//     marked cancelled vouchers transferred (2.6 #13).
//   - The original's boxes were BS_CHECKBOX, toggled by hand; the port checks
//     both at the start and toggles them the same way. Each half is its own
//     transaction: if AR's fails, AP's stays done, and the message says so.
//   - The status line (4095) showed each account as it went; the port shows
//     the result. Cancelling a transfer's GL voucher (Transactions > Cancel
//     Voucher) puts its rows back (Ap.h UndoApTransfer).
//
#include "ApTransfer.h"

#include "Ap.h"
#include "Ar.h"
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
        SetCheck(kAr, true);
        EnsureApTables(cb);
        EnsureArTables(cb);
        int ap = 0, ar = 0;
        for (const JournalRow& r : LoadJournal(cb))
            if (r.mode == "1" || r.mode.empty()) ++ap;
        for (const JournalRow& r : LoadArJournal(cb))
            if (r.mode == "1" || r.mode.empty()) ++ar;
        SetFieldW(kStatus, fx::Format(L"Waiting for the General Ledger: %d AP and %d AR journal row(s).", ap, ar));
        if (closeAtOnce_) PostMessageW(hwnd(), WM_COMMAND, IDCANCEL, 0);
        return true;
    }

    bool OnCommand(WORD id, WORD code, HWND ctl) override
    {
        if (code != BN_CLICKED) return MainDialog::OnCommand(id, code, ctl);
        switch (id) {
        case kAp: SetCheck(kAp, !IsChecked(kAp)); return true;   // TGLAPUpdate::APCheck
        case kAr: SetCheck(kAr, !IsChecked(kAr)); return true;   // TGLAPUpdate::ARCheck
        case IDOK: Ok(); return true;
        default: return MainDialog::OnCommand(id, code, ctl);
        }
    }

private:
    void Ok()   // TGLAPUpdate::Ok
    {
        if (!IsChecked(kAp) && !IsChecked(kAr)) {
            Cancel();
            return;
        }
        const long iniBatchno = std::atol(fx::WideToUtf8(GetIniString(L"GL.INI", L"PERIOD", L"BATCHNO")).c_str());
        std::wstring text;
        bool any = false;
        for (const bool ar : {false, true}) {
            if (!IsChecked(ar ? kAr : kAp)) continue;
            const wchar_t* name = ar ? L"Accounts Receivable" : L"Accounts Payable";
            ApTransferResult r;
            try {
                r = ar ? TransferAr(cb, iniBatchno) : TransferAp(cb, iniBatchno);
            } catch (const std::exception& e) {
                text += fx::Format(L"%s: the transfer failed, and nothing of it was changed.\n%s\n\n", name,
                                   fx::Utf8ToWide(e.what()).c_str());
                continue;
            }
            text += fx::Format(L"%s:\n", name) + Describe(r) + L"\n";
            any = any || !r.vouchers.empty();
        }
        if (any) text += L"Post the voucher(s) with Transactions > Post Voucher > General Ledger Voucher.";
        Message(text, MB_ICONINFORMATION);
        Cancel();   // cb.closeAll(); cb.init(); ShutDownWindow()
    }

    static std::wstring Describe(const ApTransferResult& r)
    {
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
        return text.empty() ? std::wstring(L"There is nothing to transfer.\n") : text;
    }

    bool closeAtOnce_;
};

} // namespace

INT_PTR RunApTransfer(HWND frame, bool closeAtOnce)
{
    TransferDialog dlg(closeAtOnce);
    return dlg.DoModal(1030, frame);
}
