//
// ArPosting.cpp - see ArPosting.h.
//
// Translation notes (plan per-port checklist; decisions are spec #6 §8):
//   - The three dialogs share AP's voucher-list shape (ApPosting.cpp): a
//     4-column grid filled from the AR headers by mode, refilled after each
//     action (the operations run in transactions of their own).
//   - Post (TARPostBatch): posted every open voucher that balanced with an
//     exact double compare (2.4); an invoice already in AR_DIST had its total
//     added to ADJUSTMENT. The port compares cents, refuses a voucher with an
//     account not in the chart or no lines, and writes one open item "O"
//     per voucher (D6 keeps invoice numbers apart at entry).
//   - Cancel (TARCancelBatch): ran on AP's dialog 1043, zeroed the voucher's
//     total and lines (not its VAT), wrote "Cancelled" over the reference and
//     set "0", as posted (2.6 #7, #14). The port uses AR's own 1054, sets
//     "X" and keeps the lines (D3).
//   - Recover is new (D7): RecoverPostedInvoice was dead code that would
//     have crashed (2.6 #8).
//   - 1061 is gl.rc's: gl_new.rc's copy is cut short (no Close button and
//     no closing brace), and prune_rc.py drops it.
//
#include "ArPosting.h"

#include "Ar.h"
#include "Globals.h"
#include "MainDialog.h"
#include "VoucherLists.h"   // FormatAmount

#include <algorithm>
#include <exception>
#include <string>
#include <vector>

#include "LegacyIds.h"   // last: see LegacyIds.h

namespace {

constexpr int kGrid = ID_DISTLIST;   // 1075
constexpr int kNumber = ID_BATCHNO;  // 3031

struct WaitCursor {
    HCURSOR old = SetCursor(LoadCursorW(nullptr, IDC_WAIT));
    WaitCursor() = default;
    WaitCursor(const WaitCursor&) = delete;
    WaitCursor& operator=(const WaitCursor&) = delete;
    ~WaitCursor() { SetCursor(old); }
};

class ArListDialog : public MainDialog {
public:
    explicit ArListDialog(bool closeAtOnce) : closeAtOnce_(closeAtOnce) {}

protected:
    virtual const char* Mode() const = 0;
    virtual const wchar_t* Caption() const { return nullptr; }   // nullptr: the template's
    virtual void Pick(long number) = 0;

    bool OnInitDialog() override
    {
        MainDialog::OnInitDialog();
        if (const wchar_t* c = Caption()) SetWindowTextW(hwnd(), c);
        EnsureArTables(cb);
        grid_ = fx::Grid(GetDlgItem(hwnd(), kGrid));
        if (grid_.IsGrid()) {
            const int ch = grid_.CharWidth();
            grid_.SetColumns({{L"Vch", 7 * ch},
                              {L"Customer / Reference", 30 * ch},
                              {L"Invoice", 11 * ch},
                              {L"Amount", 14 * ch, fx::GridAlign::Right}});
            grid_.SetStretchColumn(2);
        }
        Fill();
        if (closeAtOnce_) PostMessageW(hwnd(), WM_COMMAND, IDCANCEL, 0);
        return true;
    }

    bool OnCommand(WORD id, WORD code, HWND ctl) override
    {
        if (id == kGrid) {
            if (code == GRN_PICK) {
                const int row = grid_.ActiveRow();
                if (row >= 1 && row <= static_cast<int>(numbers_.size())) Pick(numbers_[static_cast<size_t>(row - 1)]);
            }
            return true;
        }
        return MainDialog::OnCommand(id, code, ctl);
    }

    void Fill()
    {
        numbers_.clear();
        if (!grid_.IsGrid()) return;
        grid_.SetRedraw(false);
        grid_.Clear();
        int row = 0;
        for (const ArVoucher& v : LoadArVouchers(cb)) {
            if (v.mode != Mode()) continue;
            ++row;
            grid_.SetData(1, row, std::to_wstring(v.number));
            grid_.SetData(2, row, fx::Utf8ToWide(v.customer + "  " + v.reference));
            grid_.SetData(3, row, fx::Utf8ToWide(v.invoiceNo));
            grid_.SetData(4, row, FormatAmount(static_cast<double>(v.total) / 100.0, false));
            numbers_.push_back(v.number);
        }
        grid_.SetRedraw(true);
    }

    void ShowNumber(long number) { SetFieldW(kNumber, std::to_wstring(number)); }

    void Failed(const wchar_t* what, const std::exception& e)
    {
        Message(fx::Format(L"%s failed, and nothing was changed.\n\n%s", what, fx::Utf8ToWide(e.what()).c_str()),
                MB_ICONSTOP);
        Fill();
    }

    fx::Grid grid_;
    std::vector<long> numbers_;   // row - 1 -> voucher number

private:
    bool closeAtOnce_;
};

// ---- TARPostBatch (1061) -----------------------------------------------------

class ArPostDialog : public ArListDialog {
public:
    using ArListDialog::ArListDialog;

protected:
    const char* Mode() const override { return ar_mode::kOpen; }

    bool OnCommand(WORD id, WORD code, HWND ctl) override
    {
        if (id == IDOK && code == BN_CLICKED) {
            PostAll();
            return true;
        }
        return ArListDialog::OnCommand(id, code, ctl);
    }

    void PostAll()   // TARPostBatch::Ok
    {
        if (numbers_.empty()) {
            Message(L"There are no open sales vouchers to post.", MB_ICONINFORMATION);
            return;
        }
        if (Message(GetString(8130), MB_ICONQUESTION | MB_YESNO) != IDYES) return;   // "...post all vouchers?"
        Run(0);
    }

    void Pick(long number) override   // TARPostBatch::HandleListBox
    {
        if (Message(fx::Format(GetString(8129).c_str(), std::to_wstring(number).c_str()), MB_YESNO | MB_ICONQUESTION) !=
            IDYES)
            return;
        Run(number);
    }

    void Run(long number)
    {
        ApPostResult r;
        try {
            WaitCursor wait;
            r = PostArVouchers(cb, number);
        } catch (const std::exception& e) {
            Failed(L"Posting", e);
            return;
        }
        if (!r.posted.empty()) ShowNumber(r.posted.back());
        for (const ApRefusal& f : r.refused) {
            switch (f.reason) {
            case ApPostRefusal::OutOfBalance:   // "The voucher number %s is out of balance and can not be posted."
                Message(fx::Format(GetString(1007).c_str(), std::to_wstring(f.number).c_str()), MB_ICONEXCLAMATION);
                break;
            case ApPostRefusal::NoLines:
                Message(fx::Format(L"Voucher %ld has no lines and was not posted.", f.number), MB_ICONEXCLAMATION);
                break;
            case ApPostRefusal::UnknownAccount:
                Message(fx::Format(L"%s: %s %s\n\nVoucher %ld was not posted.", GetString(3081).c_str(),
                                   fx::Utf8ToWide(f.account.account).c_str(), fx::Utf8ToWide(f.account.dept).c_str(),
                                   f.number),
                        MB_ICONEXCLAMATION);
                break;
            }
        }
        if (number == 0 && r.refused.empty()) {
            Message(fx::Format(L"%zu sales voucher(s) posted.", r.posted.size()), MB_ICONINFORMATION);
            Cancel();   // as TARPostBatch::Ok: close
            return;
        }
        Fill();
    }
};

// ---- TARCancelBatch (1054) ---------------------------------------------------

class ArCancelDialog : public ArListDialog {
public:
    using ArListDialog::ArListDialog;

protected:
    const char* Mode() const override { return ar_mode::kOpen; }

    void Pick(long number) override   // TARCancelBatch::HandleListBox
    {
        if (Message(fx::Format(L"Cancel sales voucher %ld?\n\nIt is kept, marked cancelled, and can't be changed or "
                               L"posted.",
                               number),
                    MB_ICONQUESTION | MB_YESNO) != IDYES)
            return;
        try {
            if (CancelArVoucher(cb, number) == ApCancelResult::Cancelled) ShowNumber(number);
        } catch (const std::exception& e) {
            Failed(L"Cancel", e);
            return;
        }
        Fill();
    }
};

// ---- Recover (D7, 1015) -------------------------------------------------------

class ArRecoverDialog : public ArListDialog {
public:
    using ArListDialog::ArListDialog;

protected:
    const char* Mode() const override { return ar_mode::kPosted; }
    const wchar_t* Caption() const override { return L"Recover Sales Voucher"; }

    void Pick(long number) override
    {
        if (Message(fx::Format(L"Recover sales voucher %ld?\n\nIts journal and open item are removed and it is "
                               L"reopened for editing.",
                               number),
                    MB_YESNO | MB_ICONQUESTION) != IDYES)
            return;
        ArRecoverResult r;
        try {
            WaitCursor wait;
            r = RecoverArVoucher(cb, number);
        } catch (const std::exception& e) {
            Failed(L"Recover", e);
            return;
        }
        switch (r) {
        case ArRecoverResult::Recovered: ShowNumber(number); break;
        case ArRecoverResult::Transferred:
            Message(fx::Format(L"Voucher %ld's journal has been transferred to the General Ledger.\n\nCancel the "
                               L"transfer's GL voucher first (Transactions > Cancel Voucher > General Ledger Voucher).",
                               number),
                    MB_ICONEXCLAMATION);
            break;
        case ArRecoverResult::Received:
            Message(fx::Format(L"A receipt has been entered against voucher %ld's invoice: it can't be recovered.",
                               number),
                    MB_ICONEXCLAMATION);
            break;
        case ArRecoverResult::NotPosted: break;
        }
        Fill();
    }
};

} // namespace

INT_PTR RunArPost(HWND frame, bool closeAtOnce)
{
    ArPostDialog dlg(closeAtOnce);
    return dlg.DoModal(1061, frame);
}

INT_PTR RunArCancel(HWND frame, bool closeAtOnce)
{
    ArCancelDialog dlg(closeAtOnce);
    return dlg.DoModal(1054, frame);
}

INT_PTR RunArRecover(HWND frame, bool closeAtOnce)
{
    ArRecoverDialog dlg(closeAtOnce);
    return dlg.DoModal(1015, frame);
}
