//
// TransactionInquiry.cpp - see TransactionInquiry.h.
//
// Translation notes (plan per-port checklist; corrections are spec D13):
//   - SetData read the account and dept with WM_GETTEXT and sizeof(char*),
//     so 3 characters of each, and seeked glaccts on that prefix: the
//     opening balance and description came from the first account starting
//     with those 3 characters. The port looks up exactly the account and
//     dept typed (SeekAccount) and says 3081 if there is none.
//   - The query ACCOUNT = '<a>' .AND. DEPT = '<d>' .AND. MODE = '0' was a
//     dBase left match: a blank dept listed every dept's lines, and the
//     opening balance belonged to one account while the lines came from
//     several. The port lists exactly that account and dept.
//   - "Opening Account Balance" was written into the float Debit cell of
//     row 1 and then overwritten by the amount; the port writes it in the
//     Description column, like "Account Balance".
//   - The amounts were gcvt() text; the port shows them as the grids do
//     (FormatAmount). The layout is kept: the opening balance on row 1, the
//     lines from row 4, a blank row, then the balance; each balance in Debit
//     when positive, else in Credit without its sign.
//   - SetupWindow showed the first account's number and dept in the fields
//     and its description in ID_DESCRIPT, which 1029 doesn't have; the port
//     shows the account and dept.
//   - Ok (Enter) did nothing, and still does; Print was an empty handler and
//     1029 has no Print button.
//
#include "TransactionInquiry.h"

#include "AccountList.h"
#include "Accounts.h"
#include "Globals.h"
#include "MainDialog.h"
#include "VoucherLists.h"   // FormatAmount
#include "Vouchers.h"

#include <cmath>
#include <memory>
#include <string>

#include "LegacyIds.h"   // last: see LegacyIds.h

namespace {

constexpr int kSearch = 2000;   // gl_accts.h's ID_SEARCH, which gl_trans.cpp's [ID_FIRST + ID_SEARCH] used
constexpr int kGrid = ID_DISTLIST;

class TransactionInquiry : public MainDialog {
public:
    explicit TransactionInquiry(bool closeAtOnce) : closeAtOnce_(closeAtOnce) {}

protected:
    bool OnInitDialog() override
    {
        MainDialog::OnInitDialog();
        glaccts_ = std::make_unique<AccountsTable>(cb);
        if (!glaccts_->isValid()) {
            Message(L"The company database has no GLACCTS table.", MB_ICONINFORMATION);
            End(IDCANCEL);
            return true;
        }
        list_.Create(hwnd(), *glaccts_);
        SetBitmap(kSearch);
        SetBitmap(ID_SETDATA);
        grid_ = fx::Grid(GetDlgItem(hwnd(), kGrid));
        if (grid_.IsGrid()) {
            const int ch = grid_.CharWidth();
            grid_.SetColumns({{L"Pd", 5 * ch},
                              {L"Btch No.", 9 * ch},
                              {L"Description", 30 * ch},
                              {L"Debit", 14 * ch, fx::GridAlign::Right},
                              {L"Credit", 14 * ch, fx::GridAlign::Right}});
            grid_.SetStretchColumn(3);
        }
        glaccts_->top();   // SetupWindow: the first account
        if (!glaccts_->eof()) {
            SetField(ID_ACCOUNTS, *glaccts_->accounts);
            SetField(ID_DEPART, *glaccts_->dept);
        }
        if (closeAtOnce_) PostMessageW(hwnd(), WM_COMMAND, IDCANCEL, 0);
        return true;
    }

    bool OnCommand(WORD id, WORD code, HWND ctl) override
    {
        if (code != BN_CLICKED) return MainDialog::OnCommand(id, code, ctl);   // EN_* from the fields
        switch (id) {
        case kSearch:   // Search: show or hide the account list
            if (list_.hwnd()) ShowWindow(list_.hwnd(), IsWindowVisible(list_.hwnd()) ? SW_HIDE : SW_SHOWNOACTIVATE);
            return true;
        case ID_SETDATA:    // Update: SendMessage(ID_ACCTLIST)
        case ID_ACCTLIST:   // the account list chose an account
            SetData();
            return true;
        case IDOK:          // TGLTransInq::Ok {}
            return true;
        default:
            return MainDialog::OnCommand(id, code, ctl);
        }
    }

    bool OnDestroy() override
    {
        if (list_.hwnd()) DestroyWindow(list_.hwnd());   // saves its position while the window still exists
        return false;
    }

private:
    // A balance in Debit when positive, else in Credit without its sign.
    void Balance(int row, const wchar_t* label, double amount)
    {
        grid_.SetData(3, row, label);
        grid_.SetData(amount > 0 ? 4 : 5, row, FormatAmount(std::fabs(amount), false));
    }

    // TGLTransInq::SetData / SetBatchList.
    void SetData()
    {
        const AccountActivity a = AccountTransactions(cb, GetField(ID_ACCOUNTS), GetField(ID_DEPART));
        if (grid_.IsGrid()) grid_.Clear();
        if (!a.found) {
            SetField(ID_JDESCRIPT, "");
            Message(GetString(3081), MB_ICONINFORMATION);   // "Unable to find GL Account"
            SetFocus(GetDlgItem(hwnd(), ID_ACCOUNTS));
            return;
        }
        SetField(ID_JDESCRIPT, a.descript);
        if (!grid_.IsGrid()) return;
        grid_.SetRedraw(false);
        Balance(1, L"Opening Account Balance", a.openBal);
        grid_.SetData(1, 3, L"");   // rows 2 and 3 blank, as before
        int row = 4;
        for (const PostedLine& l : a.lines) {
            grid_.SetData(1, row, std::to_wstring(l.period));
            grid_.SetData(2, row, std::to_wstring(l.number));
            grid_.SetData(3, row, fx::Utf8ToWide(l.descript));
            grid_.SetData(4, row, FormatAmount(l.debit));
            grid_.SetData(5, row, FormatAmount(l.credit));
            ++row;
        }
        Balance(row + 1, L"Account Balance", a.balance);
        grid_.SetRedraw(true);
    }

    bool closeAtOnce_;
    std::unique_ptr<AccountsTable> glaccts_;
    AccountList list_;
    fx::Grid grid_;
};

} // namespace

INT_PTR RunTransactionInquiry(HWND frame)
{
    TransactionInquiry dlg(false);
    return dlg.DoModal(1029, frame);
}

int StressTransactionInquiry(int times)
{
    auto objects = [] {
        HANDLE self = GetCurrentProcess();
        return static_cast<int>(GetGuiResources(self, GR_GDIOBJECTS) + GetGuiResources(self, GR_USEROBJECTS));
    };
    {
        TransactionInquiry warmUp(true);   // first open: class registration, cached bitmaps
        if (warmUp.DoModal(1029, nullptr) == -1) return -1;
    }
    const int before = objects();
    for (int i = 0; i < times; ++i) {
        TransactionInquiry dlg(true);
        if (dlg.DoModal(1029, nullptr) == -1) return -1;
    }
    return objects() - before;
}
