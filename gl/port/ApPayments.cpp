//
// ApPayments.cpp - see ApPayments.h.
//
// Translation notes (plan per-port checklist; decisions are spec #5 §8):
//   - D8: Create took the open and forced invoices due by the date (Force
//     meant nothing: a forced invoice not yet due was skipped); the port pays
//     forced invoices whatever their due date. Hold keeps one out.
//   - D9: Create gave a cheque number per vendor run but wrote no cheque; Set
//     Cheque Numbers (8074) zapped AP_REQST, renumbered and rebuilt it later.
//     The port writes the cheque at once, one per vendor, numbered from the
//     starting number (refusing a number already used), drawn on the bank
//     chosen. One run uses one bank, so a cheque never covers two.
//   - The payment's journal rows (CreateGLEntry) were dated the invoice's
//     date in its period; they are dated the payment date, in that date's
//     open period (refused if it isn't open), with the cheque number.
//   - The amount paid is what the invoice owes: its amount less the W/H
//     withheld (the original paid the full amount, W/H included).
//   - The list showed nothing until Set Date; it is filled at once (due by
//     today), and after each change. View All shows every open item.
//   - Create's message on an empty list was AR's (4074): the port's says what
//     happened. Print (the cheque list, AP_REQST.RPT) comes with the AP
//     reports (S5-M7) and is hidden until then. The status line (5013, a
//     tbStatusBar, now a win32fx class) says what the screen did last.
//   - The status pane sat partly below the dialog: it is moved to the bottom
//     of the client area, a line high.
//   - Hold / Force (1066): its two auto check boxes could both be ticked
//     (Force won); the port unticks the other, and refuses an invoice already
//     in a payment (4010), as the original's SetupWindow did for "R" and "P".
//
#include "ApPayments.h"

#include "Accounts.h"   // TrimText
#include "Ap.h"
#include "Globals.h"
#include "MainDialog.h"
#include "VoucherLists.h"   // FormatAmount

#include "cb4sqlite.h"
#include "report/Format.h"   // rpt::FormatDate

#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

#include "LegacyIds.h"   // last: see LegacyIds.h

namespace {

// Dialog 1045's controls.
constexpr int kGrid = 1075;        // ID_DISTLIST
constexpr int kDueBy = 4075;       // ID_DUEDATE: "Due Date"
constexpr int kPayDate = 3035;     // ID_DATE: "Payment Date"
constexpr int kFirstCheque = 1087; // ID_APCONTROL: "Starting Cheque No"
constexpr int kBank = 2015;        // ID_DESCRIPT: the bank combo box
constexpr int kBankAccount = 2013; // ID_ACCOUNTS
constexpr int kBankDept = 2014;    // ID_DEPART
constexpr int kStatus = 5013;      // IDD_STATUSBOX
constexpr int kPrintList = 3068;   // ID_PRINT
constexpr int kViewAll = 1121;     // ID_VIEW
constexpr int kSetDate = 4080;     // ID_SETDATE

// Dialog 1066's.
constexpr int kHoldVendor = 4001;
constexpr int kHoldInvoice = 4012;
constexpr int kHold = 4107;        // ID_HOLD
constexpr int kForce = 4106;       // ID_FORCE

std::string today()
{
    SYSTEMTIME t{};
    GetLocalTime(&t);
    char buf[16] = {};
    std::snprintf(buf, sizeof buf, "%04u%02u%02u", t.wYear, t.wMonth, t.wDay);
    return buf;
}

std::wstring amount(std::int64_t cents) { return FormatAmount(static_cast<double>(cents) / 100.0, false); }

// ---- TAPHoldForce (1066) ------------------------------------------------------

class HoldForceDialog : public MainDialog {
public:
    explicit HoldForceDialog(const PaymentItem& item) : item_(item) {}
    std::string chosen;   // H, O or F after Accept

protected:
    bool OnInitDialog() override
    {
        MainDialog::OnInitDialog();
        SetField(kHoldVendor, item_.item.vendor + "  " + item_.vendorName);
        SetField(kHoldInvoice, item_.item.invoiceNo);
        SetCheck(kHold, item_.item.paymentCtl == ap_payment::kHeld);
        SetCheck(kForce, item_.item.paymentCtl == ap_payment::kForced);
        return true;
    }

    bool OnCommand(WORD id, WORD code, HWND ctl) override
    {
        if (code == BN_CLICKED && (id == kHold || id == kForce)) {   // one or the other
            if (IsChecked(id)) SetCheck(id == kHold ? kForce : kHold, false);
            return true;
        }
        if (code == BN_CLICKED && id == IDOK) {   // TAPHoldForce::Ok
            chosen = IsChecked(kForce) ? ap_payment::kForced : IsChecked(kHold) ? ap_payment::kHeld : ap_payment::kOpen;
            End(IDOK);
            return true;
        }
        if (code == BN_CLICKED && id == IDCANCEL) {
            End(IDCANCEL);
            return true;
        }
        return MainDialog::OnCommand(id, code, ctl);
    }

private:
    PaymentItem item_;
};

// ---- TAPCreatePayment (1045) -------------------------------------------------

class CreateChequesDialog : public MainDialog {
public:
    explicit CreateChequesDialog(bool closeAtOnce) : closeAtOnce_(closeAtOnce) {}

protected:
    bool OnInitDialog() override
    {
        MainDialog::OnInitDialog();   // Create (IDOK) and Close pictures
        SetBitmap(kViewAll);
        ShowWindow(GetDlgItem(hwnd(), kPrintList), SW_HIDE);   // S5-M7
        FitStatus();
        SetWindowTextW(hwnd(), L"Create Cheques");
        EnsureApTables(cb);
        setat_ = LoadApAccounts(cb);
        const HWND box = GetDlgItem(hwnd(), kBank);   // SetCashAccounts
        for (size_t i = 0; i < setat_.banks.size(); ++i) {
            const ApBank& b = setat_.banks[i];
            const std::wstring text = fx::Utf8ToWide(b.shortName + " - " + TrimText(b.name));
            const LRESULT at = SendMessageW(box, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(text.c_str()));
            SendMessageW(box, CB_SETITEMDATA, static_cast<WPARAM>(at), static_cast<LPARAM>(i));
        }
        SetField(kDueBy, today());
        SetField(kPayDate, today());
        FloatSetDlgItemValue(hwnd(), kFirstCheque, static_cast<double>(NextChequeNumber(cb)));   // SetChequeNo

        grid_ = fx::Grid(GetDlgItem(hwnd(), kGrid));
        if (grid_.IsGrid()) {
            const int ch = grid_.CharWidth();
            grid_.SetColumns({{L"Vendor", 16 * ch},
                              {L"Ctl", 5 * ch},
                              {L"Code", 7 * ch},
                              {L"Invoice", 10 * ch},
                              {L"Due Date", 12 * ch},
                              {L"Amount", 12 * ch, fx::GridAlign::Right},
                              {L"VAT", 10 * ch, fx::GridAlign::Right},
                              {L"W/H", 10 * ch, fx::GridAlign::Right},
                              {L"Owed", 12 * ch, fx::GridAlign::Right}});
            grid_.SetStretchColumn(1);
        }
        Fill(false);
        if (closeAtOnce_) PostMessageW(hwnd(), WM_COMMAND, IDCANCEL, 0);
        return true;
    }

    bool OnCommand(WORD id, WORD code, HWND ctl) override
    {
        if (id == kGrid) {
            if (code == GRN_PICK) Pick(grid_.ActiveRow());
            return true;
        }
        if (id == kBank) {   // GetBankAccount
            if (code == CBN_SELCHANGE) {
                const ApAccount a = Bank();
                SetField(kBankAccount, a.account);
                SetField(kBankDept, a.dept);
            }
            return true;
        }
        if (code != BN_CLICKED) return MainDialog::OnCommand(id, code, ctl);
        switch (id) {
        case IDOK: Create(); return true;
        case kSetDate: Fill(false); return true;
        case kViewAll: Fill(true); return true;
        default: return MainDialog::OnCommand(id, code, ctl);   // IDCANCEL -> Cancel()
        }
    }

private:
    ApAccount Bank() const
    {
        const HWND box = GetDlgItem(hwnd(), kBank);
        const LRESULT sel = SendMessageW(box, CB_GETCURSEL, 0, 0);
        const LRESULT i = sel == CB_ERR ? -1 : SendMessageW(box, CB_GETITEMDATA, static_cast<WPARAM>(sel), 0);
        return i >= 0 && static_cast<size_t>(i) < setat_.banks.size() ? setat_.banks[static_cast<size_t>(i)].account
                                                                        : ApAccount{};
    }

    void Status(const std::wstring& text) { SetFieldW(kStatus, text); }

    // The template puts the status pane (y 173, 14 high) partly below the
    // dialog's 182: it goes along the bottom of the client area, a line of
    // the dialog's font high.
    void FitStatus()
    {
        const HWND pane = GetDlgItem(hwnd(), kStatus);
        if (!pane) return;
        RECT client{};
        GetClientRect(hwnd(), &client);
        HDC dc = GetDC(hwnd());
        HGDIOBJ old = SelectObject(dc, reinterpret_cast<HFONT>(SendMessageW(hwnd(), WM_GETFONT, 0, 0)));
        TEXTMETRICW tm{};
        GetTextMetricsW(dc, &tm);
        SelectObject(dc, old);
        ReleaseDC(hwnd(), dc);
        const int h = tm.tmHeight + 6;
        MoveWindow(pane, 0, client.bottom - h, client.right, h, TRUE);
    }

    void Fill(bool all)   // DistList
    {
        all_ = all;
        shown_ = PaymentItems(cb, GetDate(kDueBy), all);
        if (!grid_.IsGrid()) return;
        grid_.SetRedraw(false);
        grid_.Clear();
        std::int64_t owed = 0;
        int row = 0;
        for (const PaymentItem& p : shown_) {
            ++row;
            grid_.SetData(1, row, fx::Utf8ToWide(p.vendorName));
            grid_.SetData(2, row, fx::Utf8ToWide(p.item.paymentCtl));
            grid_.SetData(3, row, fx::Utf8ToWide(p.item.vendor));
            grid_.SetData(4, row, fx::Utf8ToWide(p.item.invoiceNo));
            grid_.SetData(5, row, fx::Utf8ToWide(rpt::FormatDate(p.item.dueDate)));
            grid_.SetData(6, row, amount(p.item.amount));
            grid_.SetData(7, row, amount(p.item.vat));
            grid_.SetData(8, row, amount(p.item.wh));
            grid_.SetData(9, row, amount(p.owed));
            owed += p.owed;
        }
        grid_.SetRedraw(true);
        if (all)
            Status(fx::Format(L"All %zu open item(s).", shown_.size()));
        else if (shown_.empty())
            Status(GetString(4028));   // "No invoice were found for this period."
        else
            Status(fx::Format(L"%zu invoice(s) due by %s, or forced: %s owed.", shown_.size(),
                              fx::Utf8ToWide(rpt::FormatDate(GetDate(kDueBy))).c_str(), amount(owed).c_str()));
    }

    void Pick(int row)   // TAPCreatePayment::SelectCell -> TAPHoldForce
    {
        if (row < 1 || row > static_cast<int>(shown_.size())) return;
        const PaymentItem& p = shown_[static_cast<size_t>(row - 1)];
        const std::string c = p.item.paymentCtl;
        if (c != ap_payment::kHeld && c != ap_payment::kOpen && c != ap_payment::kForced) {
            Message(GetString(4010), MB_ICONINFORMATION);   // "Only unrequested invoices may be modified."
            return;
        }
        HoldForceDialog dlg(p);
        if (dlg.DoModal(1066, hwnd()) != IDOK || dlg.chosen == c) return;
        SetPaymentControl(cb, p.item.batchno, dlg.chosen);
        Fill(all_);
    }

    void Create()   // TAPCreatePayment::Ok
    {
        double first = 0;
        FloatGetDlgItemValue(hwnd(), kFirstCheque, &first);
        const long firstNumber = static_cast<long>(std::lround(first));
        if (firstNumber <= 0) {   // ID_REQUESTNO
            Message(L"Enter the starting cheque number.", MB_ICONINFORMATION);
            SetFocus(GetDlgItem(hwnd(), kFirstCheque));
            return;
        }
        const ApAccount bank = Bank();
        if (bank.empty()) {
            Message(L"Choose the bank the cheques are drawn on.", MB_ICONINFORMATION);
            SetFocus(GetDlgItem(hwnd(), kBank));
            return;
        }
        const std::string dueBy = GetDate(kDueBy), payDate = GetDate(kPayDate);
        if (dueBy.empty() || payDate.empty()) {
            Message(GetString(8090), MB_ICONINFORMATION);   // "Double click on the date field to set the date."
            return;
        }
        if (Message(GetString(4009), MB_ICONQUESTION | MB_YESNO) != IDYES) return;   // "...create a list of cheques?"
        CreateChequesResult r;
        try {
            r = CreateCheques(cb, dueBy, payDate, bank, firstNumber);
        } catch (const std::exception& e) {
            Message(fx::Format(L"Creating the cheques failed, and nothing was changed.\n\n%s",
                               fx::Utf8ToWide(e.what()).c_str()),
                    MB_ICONSTOP);
            return;
        }
        switch (r.status) {
        case CreateChequesStatus::NothingDue:
            Message(L"No open invoice is due by that date, and none is forced.", MB_ICONINFORMATION);
            return;
        case CreateChequesStatus::BadBank: Message(GetString(4007), MB_ICONINFORMATION); return;   // "...account not found."
        case CreateChequesStatus::BadNumber:
            Message(fx::Format(L"Cheque %ld has already been used. Choose another starting number.", r.clash),
                    MB_ICONEXCLAMATION);
            SetFocus(GetDlgItem(hwnd(), kFirstCheque));
            return;
        case CreateChequesStatus::ClosedPeriod:
            Message(L"The payment date isn't in an open period (Company Profile > Period Dates).", MB_ICONEXCLAMATION);
            SetFocus(GetDlgItem(hwnd(), kPayDate));
            return;
        case CreateChequesStatus::Created: break;
        }
        std::wstring text;
        std::int64_t total = 0;
        for (const ChequeMade& c : r.cheques) {
            text += fx::Format(L"Cheque %ld  %s %s  %s  (%d invoice(s))\n", c.number, fx::Utf8ToWide(c.vendor).c_str(),
                               fx::Utf8ToWide(c.vendorName).c_str(), amount(c.amount).c_str(), c.items);
            total += c.amount;
        }
        text += fx::Format(L"\n%zu cheque(s), %s, in period %d. Print them with Print Cheques.", r.cheques.size(),
                           amount(total).c_str(), r.period);
        Message(text, MB_ICONINFORMATION);
        FloatSetDlgItemValue(hwnd(), kFirstCheque, static_cast<double>(NextChequeNumber(cb)));
        Fill(all_);
        Status(fx::Format(L"%zu cheque(s) created: %s.", r.cheques.size(), amount(total).c_str()));
    }

    bool closeAtOnce_;
    bool all_ = false;
    ApAccounts setat_;
    fx::Grid grid_;
    std::vector<PaymentItem> shown_;   // row - 1 -> item
};

} // namespace

INT_PTR RunApCreateCheques(HWND frame, bool closeAtOnce)
{
    CreateChequesDialog dlg(closeAtOnce);
    return dlg.DoModal(1045, frame);
}
