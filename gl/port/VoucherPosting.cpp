//
// VoucherPosting.cpp - see VoucherPosting.h.
//
// Translation notes (plan per-port checklist; corrections are spec D3, D12):
//   - The three dialogs shared the same SetupWindow: a 4-column tbSpread
//     (voucher, description, debit, credit) filled from gl_btch by mode. The
//     port has one base class, VoucherListDialog. The description column
//     showed gl_btch.descript, which the real table lacks; it is reference.
//   - The original set the fourth column's header on column 5, so Credit had
//     none; the port heads all four.
//   - Each dialog opened its tables in SetupWindow and kept them; the port
//     reopens gl_btch for each fill, because the operations run in
//     transactions on tables of their own (Vouchers.h) and a rollback leaves
//     open tables stale (spec 5.2).
//   - 1061's caption said "Post Accounts Receivable Voucher"; the port sets
//     "Post General Ledger Voucher" (D3). 1043's wrong caption was already
//     replaced at run time with string 6143, as here.
//   - Post listed only open vouchers ("1"), though Ok also posted
//     adjustments ("3"), and a picked adjustment got "out of balance"
//     (8116). The port lists both and posts a picked adjustment as one.
//     Cancel lists both too, so an adjustment can be cancelled.
//   - Recover reversed a voucher on a double-click without asking; the port
//     asks first.
//   - The "voucher number" static (3031) showed each voucher as it was
//     posted; the port shows the last one posted, recovered or cancelled.
//
#include "VoucherPosting.h"

#include "Globals.h"
#include "MainDialog.h"
#include "VoucherLists.h"   // FormatAmount
#include "Vouchers.h"

#include <exception>
#include <string>
#include <utility>
#include <vector>

#include "LegacyIds.h"   // last: see LegacyIds.h

namespace {

constexpr int kGrid = ID_DISTLIST;   // 1075
constexpr int kNumber = ID_BATCHNO;  // 3031

// HourGlassOn / HourGlassOff.
struct WaitCursor {
    HCURSOR old = SetCursor(LoadCursorW(nullptr, IDC_WAIT));
    WaitCursor() = default;
    WaitCursor(const WaitCursor&) = delete;
    WaitCursor& operator=(const WaitCursor&) = delete;
    ~WaitCursor() { SetCursor(old); }
};

class VoucherListDialog : public MainDialog {
public:
    explicit VoucherListDialog(bool closeAtOnce) : closeAtOnce_(closeAtOnce) {}

protected:
    // The modes listed, the column headers and the caption (empty: keep).
    virtual std::vector<std::string> Modes() const = 0;
    virtual std::vector<std::wstring> Headers() const = 0;
    virtual std::wstring Caption() const { return {}; }
    // A double-clicked / Enter'd voucher.
    virtual void Pick(long number) = 0;

    bool OnInitDialog() override
    {
        MainDialog::OnInitDialog();
        if (const std::wstring c = Caption(); !c.empty()) SetWindowTextW(hwnd(), c.c_str());
        grid_ = fx::Grid(GetDlgItem(hwnd(), kGrid));
        if (grid_.IsGrid()) {
            const int ch = grid_.CharWidth();
            const std::vector<std::wstring> h = Headers();
            // SetupWindow: widths 6 / (rest) / 10 / 10 characters.
            grid_.SetColumns({{h[0], 7 * ch},
                              {h[1], 30 * ch},
                              {h[2], 14 * ch, fx::GridAlign::Right},
                              {h[3], 14 * ch, fx::GridAlign::Right}});
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
        VoucherHeaders h(cb);
        int row = 0;
        for (const VoucherHeader& v : ListVouchers(h, Modes())) {
            ++row;
            grid_.SetData(1, row, std::to_wstring(v.number));
            grid_.SetData(2, row, fx::Utf8ToWide(v.description));
            grid_.SetData(3, row, FormatAmount(v.debit, false));
            grid_.SetData(4, row, FormatAmount(v.credit, false));
            numbers_.push_back(v.number);
        }
        grid_.SetRedraw(true);
    }

    void ShowNumber(long number) { SetFieldW(kNumber, std::to_wstring(number)); }

    // The database failed, or /failpost: nothing was written (D1).
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

// ---- TGLPostBatch (1061) --------------------------------------------------

class PostVouchers : public VoucherListDialog {
public:
    using VoucherListDialog::VoucherListDialog;

protected:
    std::vector<std::string> Modes() const override { return {voucher_mode::kOpen, voucher_mode::kAdjustment}; }
    std::vector<std::wstring> Headers() const override
    {
        return {L"Vch", L"Description", L"Debit Amount", L"Credit Amount"};
    }
    std::wstring Caption() const override { return L"Post General Ledger Voucher"; }   // D3

    bool OnCommand(WORD id, WORD code, HWND ctl) override
    {
        if (id == IDOK) {
            PostAllVouchers();
            return true;
        }
        return VoucherListDialog::OnCommand(id, code, ctl);
    }

    // TGLPostBatch::Ok.
    void PostAllVouchers()
    {
        PostResult r;
        try {
            WaitCursor wait;
            r = PostAll(cb);
        } catch (const std::exception& e) {
            Failed(L"Posting", e);
            return;
        }
        if (!r.posted.empty()) ShowNumber(r.posted.back());
        Refusals(r);
        if (r.refused.empty()) {
            Message(GetString(8115), MB_ICONINFORMATION);   // "All Voucher entries have been sucessfully posted."
            Cancel();                                        // cb.closeAll(); ShutDownWindow()
            return;
        }
        Fill();   // Redraw
    }

    // TGLPostBatch::HandleListBox.
    void Pick(long number) override
    {
        if (Message(fx::Format(GetString(8129).c_str(), std::to_wstring(number).c_str()), MB_YESNO | MB_ICONQUESTION) !=
            IDYES)
            return;
        PostResult r;
        try {
            r = PostVoucher(cb, number);
        } catch (const std::exception& e) {
            Failed(L"Posting", e);
            return;
        }
        if (!r.posted.empty()) ShowNumber(number);
        Refusals(r);
        Fill();   // SSDelRow
    }

    void Refusals(const PostResult& r)
    {
        for (const VoucherRefusal& v : r.refused) {
            if (v.reason == PostRefusal::OutOfBalance) {
                // "Voucher number %s is out of balance and can not be posted."
                Message(fx::Format(GetString(8116).c_str(), std::to_wstring(v.number).c_str()), MB_ICONEXCLAMATION);
            } else {
                // "Unable to find GL Account", naming it and the voucher.
                Message(fx::Format(L"%s: %s %s\n\nVoucher %ld was not posted.", GetString(3081).c_str(),
                                   fx::Utf8ToWide(v.account).c_str(), fx::Utf8ToWide(v.dept).c_str(), v.number),
                        MB_ICONEXCLAMATION);
            }
        }
    }
};

// ---- TGLRecoverBatch (1015) -----------------------------------------------

class RecoverVoucherDialog : public VoucherListDialog {
public:
    using VoucherListDialog::VoucherListDialog;

protected:
    std::vector<std::string> Modes() const override { return {voucher_mode::kPosted}; }
    std::vector<std::wstring> Headers() const override
    {
        return {GetString(6043), GetString(6044), GetString(6069), GetString(6070)};   // Btch, Description, Debit, Credit
    }

    // TGLRecoverBatch::HandleDoubleClick.
    void Pick(long number) override
    {
        if (Message(fx::Format(L"Recover voucher %ld?\n\nIts postings are reversed and it is reopened for editing.",
                               number),
                    MB_YESNO | MB_ICONQUESTION) != IDYES)
            return;
        std::string account, dept;
        RecoverResult r;
        try {
            WaitCursor wait;
            r = RecoverVoucher(cb, number, &account, &dept);
        } catch (const std::exception& e) {
            Failed(L"Recover", e);
            return;
        }
        if (r == RecoverResult::Recovered) ShowNumber(number);
        if (r == RecoverResult::UnknownAccount)
            Message(fx::Format(L"%s: %s %s\n\nVoucher %ld was not recovered.", GetString(3081).c_str(),
                               fx::Utf8ToWide(account).c_str(), fx::Utf8ToWide(dept).c_str(), number),
                    MB_ICONEXCLAMATION);
        Fill();
    }
};

// ---- TGLCancelBatch (1043) ------------------------------------------------

class CancelVoucherDialog : public VoucherListDialog {
public:
    using VoucherListDialog::VoucherListDialog;

protected:
    std::vector<std::string> Modes() const override { return {voucher_mode::kOpen, voucher_mode::kAdjustment}; }
    std::vector<std::wstring> Headers() const override
    {
        return {L"Vch", L"Description", L"Debit Amount", L"Credit Amount"};
    }
    std::wstring Caption() const override { return GetString(6143); }   // "Cancel General Ledger Voucher"

    // TGLCancelBatch::SelectSlip.
    void Pick(long number) override
    {
        // "Are you sure you want to delete this Voucher?"
        if (Message(GetString(8099), MB_ICONQUESTION | MB_YESNO) != IDYES) return;
        try {
            if (CancelVoucher(cb, number) == CancelResult::Cancelled) ShowNumber(number);
        } catch (const std::exception& e) {
            Failed(L"Cancel", e);
            return;
        }
        Fill();
    }
};

} // namespace

INT_PTR RunPostVouchers(HWND frame)
{
    PostVouchers dlg(false);
    return dlg.DoModal(1061, frame);
}

INT_PTR RunRecoverVoucher(HWND frame)
{
    RecoverVoucherDialog dlg(false);
    return dlg.DoModal(1015, frame);
}

INT_PTR RunCancelVoucher(HWND frame)
{
    CancelVoucherDialog dlg(false);
    return dlg.DoModal(1043, frame);
}

int StressVoucherPosting(int times)
{
    auto objects = [] {
        HANDLE self = GetCurrentProcess();
        return static_cast<int>(GetGuiResources(self, GR_GDIOBJECTS) + GetGuiResources(self, GR_USEROBJECTS));
    };
    auto openAll = [] {
        PostVouchers post(true);
        RecoverVoucherDialog recover(true);
        CancelVoucherDialog cancel(true);
        return post.DoModal(1061, nullptr) != -1 && recover.DoModal(1015, nullptr) != -1 &&
               cancel.DoModal(1043, nullptr) != -1;
    };
    if (!openAll()) return -1;   // first open: class registration, cached bitmaps
    const int before = objects();
    for (int i = 0; i < times; ++i)
        if (!openAll()) return -1;
    return objects() - before;
}
