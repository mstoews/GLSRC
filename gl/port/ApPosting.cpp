//
// ApPosting.cpp - see ApPosting.h.
//
// Translation notes (plan per-port checklist; decisions are spec #5 §8):
//   - The three dialogs share Spec #2's voucher-list shape (VoucherPosting.cpp):
//     a 4-column grid filled from the AP headers by mode, refilled after each
//     action from tables opened for it (the operations run in transactions
//     of their own).
//   - Post (TAPPostBatch): Ok posted every open voucher that balanced with an
//     exact double compare (2.4) and wrote AP_DIST rows held ("H"); the port
//     compares cents, refuses a voucher with an account not in the chart or
//     no lines, and writes the open item "O" (D11). A double-click posted one
//     voucher but wrote GL_DIST rows (AppendGLDist) and no AP_JSUM: now it
//     posts as Ok does.
//   - Cancel (TAPCancelBatch): zeroed the voucher and its lines, wrote
//     "Cancelled" over them and set mode "2" (transferred, too); the port sets
//     "X" and keeps the lines (D3).
//   - Recover is new (D7): the original's "recover" was re-adding a line to a
//     posted voucher in entry, which deleted AR's journal rows (2.6 #3).
//   - The captions: 1043's and 1015's are the GL's, set here for AP.
//
#include "ApPosting.h"

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

const wchar_t* kindName(ApKind k) { return k == ApKind::Cash ? L"Cash" : L"Purchase"; }

class ApListDialog : public MainDialog {
public:
    ApListDialog(ApKind kind, bool closeAtOnce) : kind_(kind), closeAtOnce_(closeAtOnce) {}

protected:
    virtual std::vector<std::string> Modes() const = 0;
    virtual std::wstring Caption() const = 0;
    virtual void Pick(long number) = 0;

    bool OnInitDialog() override
    {
        MainDialog::OnInitDialog();
        SetWindowTextW(hwnd(), Caption().c_str());
        EnsureApTables(cb);
        grid_ = fx::Grid(GetDlgItem(hwnd(), kGrid));
        if (grid_.IsGrid()) {
            const int ch = grid_.CharWidth();
            grid_.SetColumns({{L"Vch", 7 * ch},
                              {L"Vendor / Reference", 30 * ch},
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
        const std::vector<std::string> modes = Modes();
        int row = 0;
        for (const ApVoucher& v : LoadApVouchers(cb, kind_)) {
            if (std::find(modes.begin(), modes.end(), v.mode) == modes.end()) continue;
            ++row;
            grid_.SetData(1, row, std::to_wstring(v.number));
            grid_.SetData(2, row, fx::Utf8ToWide(v.vendor + "  " + v.reference));
            grid_.SetData(3, row, fx::Utf8ToWide(v.invoiceNo));
            grid_.SetData(4, row, FormatAmount(static_cast<double>(v.payable) / 100.0, false));
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

    ApKind kind_;
    fx::Grid grid_;
    std::vector<long> numbers_;   // row - 1 -> voucher number

private:
    bool closeAtOnce_;
};

// ---- TAPPostBatch (1047) -----------------------------------------------------

class ApPostDialog : public ApListDialog {
public:
    using ApListDialog::ApListDialog;

protected:
    std::vector<std::string> Modes() const override { return {ap_mode::kOpen}; }
    std::wstring Caption() const override { return fx::Format(L"Post %s Vouchers", kindName(kind_)); }

    bool OnCommand(WORD id, WORD code, HWND ctl) override
    {
        if (id == IDOK && code == BN_CLICKED) {
            PostAll();
            return true;
        }
        return ApListDialog::OnCommand(id, code, ctl);
    }

    void PostAll()   // TAPPostBatch::Ok
    {
        if (numbers_.empty()) {
            Message(L"There are no open vouchers to post.", MB_ICONINFORMATION);
            return;
        }
        if (Message(GetString(8130), MB_ICONQUESTION | MB_YESNO) != IDYES) return;   // "...post all vouchers?"
        Run(0);
    }

    void Pick(long number) override   // TAPPostBatch::HandleListBox
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
            r = PostApVouchers(cb, kind_, number);
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
            Message(fx::Format(L"%zu voucher(s) posted.", r.posted.size()), MB_ICONINFORMATION);
            Cancel();   // as TAPPostBatch::Ok: close
            return;
        }
        Fill();
    }
};

// ---- TAPCancelBatch (1043) ---------------------------------------------------

class ApCancelDialog : public ApListDialog {
public:
    using ApListDialog::ApListDialog;

protected:
    std::vector<std::string> Modes() const override { return {ap_mode::kOpen}; }
    std::wstring Caption() const override { return fx::Format(L"Cancel %s Voucher", kindName(kind_)); }

    void Pick(long number) override   // TAPCancelBatch::HandleListBox
    {
        if (Message(fx::Format(L"Cancel voucher %ld?\n\nIt is kept, marked cancelled, and can't be changed or posted.",
                               number),
                    MB_ICONQUESTION | MB_YESNO) != IDYES)
            return;
        try {
            if (CancelApVoucher(cb, kind_, number) == ApCancelResult::Cancelled) ShowNumber(number);
        } catch (const std::exception& e) {
            Failed(L"Cancel", e);
            return;
        }
        Fill();
    }
};

// ---- Recover (D7, 1015) -------------------------------------------------------

class ApRecoverDialog : public ApListDialog {
public:
    using ApListDialog::ApListDialog;

protected:
    std::vector<std::string> Modes() const override { return {ap_mode::kPosted}; }
    std::wstring Caption() const override { return fx::Format(L"Recover %s Voucher", kindName(kind_)); }

    void Pick(long number) override
    {
        if (Message(fx::Format(L"Recover voucher %ld?\n\nIts journal and open item are removed and it is reopened "
                               L"for editing.",
                               number),
                    MB_YESNO | MB_ICONQUESTION) != IDYES)
            return;
        ApRecoverResult r;
        try {
            WaitCursor wait;
            r = RecoverApVoucher(cb, kind_, number);
        } catch (const std::exception& e) {
            Failed(L"Recover", e);
            return;
        }
        switch (r) {
        case ApRecoverResult::Recovered: ShowNumber(number); break;
        case ApRecoverResult::Transferred:
            Message(fx::Format(L"Voucher %ld's journal has been transferred to the General Ledger.\n\nCancel the "
                               L"transfer's GL voucher first (Transactions > Cancel Voucher > General Ledger Voucher).",
                               number),
                    MB_ICONEXCLAMATION);
            break;
        case ApRecoverResult::Paid:
            Message(fx::Format(L"Voucher %ld's invoice is in a payment: it can't be recovered.", number),
                    MB_ICONEXCLAMATION);
            break;
        case ApRecoverResult::NotPosted: break;
        }
        Fill();
    }
};

} // namespace

INT_PTR RunApPost(HWND frame, ApKind kind, bool closeAtOnce)
{
    ApPostDialog dlg(kind, closeAtOnce);
    return dlg.DoModal(1047, frame);
}

INT_PTR RunApCancel(HWND frame, ApKind kind, bool closeAtOnce)
{
    ApCancelDialog dlg(kind, closeAtOnce);
    return dlg.DoModal(1043, frame);
}

INT_PTR RunApRecover(HWND frame, ApKind kind, bool closeAtOnce)
{
    ApRecoverDialog dlg(kind, closeAtOnce);
    return dlg.DoModal(1015, frame);
}
