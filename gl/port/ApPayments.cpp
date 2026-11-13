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
//   - S5-M6. Confirm Printed Cheques (8074, IDD_AP_CONFIRM in port.rc)
//     replaces Set Cheque Numbers (1157: TPrintCheques), which zapped
//     AP_REQST, renumbered every "P" item from a starting number, and
//     marked them all "V" on Verify; the template has room for one number,
//     so the port's own dialog lists the cheques waiting, confirms a range,
//     and renumbers a spoiled one (D9).
//   - Bank Reconciliation (6085, 1046; TAPPayment): listed the "P" items,
//     so a printed cheque never showed, and double-click cleared it in
//     1051 (TAPSetChequeNumber), which then closed every table. The port
//     lists the cheques not cleared, made or printed; 1051 clears a printed
//     one with its date, or voids it (a Void button added in code: 1051's
//     caption, "Clear Cancel Cheque", promised it).
//   - Purge (1066, 1048; TAPPurgeBatch) deleted the "P" items: the cheques
//     made and not yet printed. The port deletes the cleared items paid
//     before a date (a date control added in code over 3031, whose label,
//     "Posting Voucher Number :", is relabelled), and their cheques (D8).
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

#include "PortIds.h"

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


// ---- Confirm Printed Cheques (IDD_AP_CONFIRM) -----------------------------------

class ConfirmDialog : public MainDialog {
public:
    explicit ConfirmDialog(bool closeAtOnce) : closeAtOnce_(closeAtOnce) {}

protected:
    bool OnInitDialog() override
    {
        MainDialog::OnInitDialog();
        EnsureApTables(cb);
        grid_ = fx::Grid(GetDlgItem(hwnd(), IDC_APC_GRID));
        if (grid_.IsGrid()) {
            const int ch = grid_.CharWidth();
            grid_.SetColumns({{L"Cheque", 10 * ch},
                              {L"Vendor", 20 * ch},
                              {L"Date", 13 * ch},
                              {L"Amount", 13 * ch, fx::GridAlign::Right}});
            grid_.SetStretchColumn(2);
        }
        Fill(true);
        if (closeAtOnce_) PostMessageW(hwnd(), WM_COMMAND, IDCANCEL, 0);
        return true;
    }

    bool OnCommand(WORD id, WORD code, HWND ctl) override
    {
        if (id == IDC_APC_GRID) return true;
        if (code != BN_CLICKED) return MainDialog::OnCommand(id, code, ctl);
        switch (id) {
        case IDOK: Confirm(); return true;
        case IDC_APC_RENUMBER: Renumber(); return true;
        default: return MainDialog::OnCommand(id, code, ctl);
        }
    }

private:
    void Fill(bool resetRange)
    {
        shown_ = LoadChequesIn(cb, ap_payment::kPaid);
        if (grid_.IsGrid()) {
            grid_.SetRedraw(false);
            grid_.Clear();
            int row = 0;
            for (const ChequeToPrint& c : shown_) {
                ++row;
                grid_.SetData(1, row, std::to_wstring(c.cheque.number));
                grid_.SetData(2, row, fx::Utf8ToWide(c.cheque.vendor + "  " + TrimText(c.payee.name)));
                grid_.SetData(3, row, fx::Utf8ToWide(rpt::FormatDate(c.cheque.date)));
                grid_.SetData(4, row, amount(c.cheque.amount));
            }
            grid_.SetRedraw(true);
        }
        if (resetRange && !shown_.empty()) {
            SetDlgItemInt(hwnd(), IDC_APC_FROM, static_cast<UINT>(shown_.front().cheque.number), FALSE);
            SetDlgItemInt(hwnd(), IDC_APC_TO, static_cast<UINT>(shown_.back().cheque.number), FALSE);
        }
    }

    void Confirm()   // TPrintCheques::Verify, for a range
    {
        BOOL ok1 = FALSE, ok2 = FALSE;
        const long from = static_cast<long>(GetDlgItemInt(hwnd(), IDC_APC_FROM, &ok1, FALSE));
        const long to = static_cast<long>(GetDlgItemInt(hwnd(), IDC_APC_TO, &ok2, FALSE));
        if (!ok1 || !ok2 || from > to) {
            Message(L"Enter the first and last cheque numbers that printed correctly.", MB_ICONINFORMATION);
            return;
        }
        // "Confirm that all cheques have been printed correctly." / "Set Cheque Numbers"
        if (Message(fx::Format(L"Have cheques %ld to %ld printed correctly?\n\nThey can't be renumbered after this.",
                               from, to),
                    MB_YESNO | MB_ICONQUESTION) != IDYES)
            return;
        const int n = ConfirmPrinted(cb, from, to);
        Message(fx::Format(L"%d cheque(s) confirmed printed.", n), MB_ICONINFORMATION);
        Fill(true);
    }

    void Renumber()
    {
        const int row = grid_.ActiveRow();
        if (row < 1 || row > static_cast<int>(shown_.size())) {
            Message(L"Select the spoiled cheque in the list first.", MB_ICONINFORMATION);
            return;
        }
        BOOL ok = FALSE;
        const long to = static_cast<long>(GetDlgItemInt(hwnd(), IDC_APC_NEWNO, &ok, FALSE));
        const long from = shown_[static_cast<size_t>(row - 1)].cheque.number;
        if (!ok || to <= 0) {
            Message(L"Enter the new cheque number.", MB_ICONINFORMATION);
            SetFocus(GetDlgItem(hwnd(), IDC_APC_NEWNO));
            return;
        }
        switch (RenumberCheque(cb, from, to)) {
        case RenumberResult::Renumbered:
            Message(fx::Format(L"Cheque %ld is now cheque %ld. Print it again.", from, to), MB_ICONINFORMATION);
            SetDlgItemTextW(hwnd(), IDC_APC_NEWNO, L"");
            Fill(true);
            break;
        case RenumberResult::NumberUsed:
            Message(fx::Format(L"Cheque number %ld has already been used.", to), MB_ICONEXCLAMATION);
            break;
        default: Message(fx::Format(L"Cheque %ld can't be renumbered.", from), MB_ICONEXCLAMATION); break;
        }
    }

    bool closeAtOnce_;
    fx::Grid grid_;
    std::vector<ChequeToPrint> shown_;
};

// ---- Clear Cancel Cheque (1051): TAPSetChequeNumber -------------------------------

class ClearDialog : public MainDialog {
public:
    explicit ClearDialog(const ChequeToPrint& c) : c_(c) {}
    enum class Action { None, Clear, Void } action = Action::None;
    std::string date;

protected:
    bool OnInitDialog() override
    {
        MainDialog::OnInitDialog();
        SetField(6070, std::to_string(c_.cheque.number));
        FloatSetDlgItemValue(hwnd(), 1030, static_cast<double>(c_.cheque.amount) / 100.0);
        SetField(2013, c_.cheque.bank.account);
        SetField(2014, c_.cheque.bank.dept);
        SetField(3035, today());
        for (int id : {6070, 1030, 2013, 2014}) SendDlgItemMessageW(hwnd(), id, EM_SETREADONLY, TRUE, 0);
        SetDlgItemTextW(hwnd(), IDOK, L"C&lear");
        SetWindowTextW(hwnd(), fx::Format(L"Clear or Void Cheque %ld", c_.cheque.number).c_str());
        if (HWND l = findLabel(L"Invoice Amount")) SetWindowTextW(l, L"Amount");
        if (HWND l = findLabel(L"Payment Date")) SetWindowTextW(l, L"Date");
        // Three buttons where the template has two: Clear, Void, Close.
        RECT ok{}, close{};
        GetWindowRect(GetDlgItem(hwnd(), IDOK), &ok);
        GetWindowRect(GetDlgItem(hwnd(), IDCANCEL), &close);
        MapWindowPoints(nullptr, hwnd(), reinterpret_cast<POINT*>(&ok), 2);
        MapWindowPoints(nullptr, hwnd(), reinterpret_cast<POINT*>(&close), 2);
        RECT client{};
        GetClientRect(hwnd(), &client);
        const int gap = MulDiv(6, static_cast<int>(GetDpiForWindow(hwnd())), 96);
        const int w = (client.right - 4 * gap) / 3, h = ok.bottom - ok.top;
        MoveWindow(GetDlgItem(hwnd(), IDOK), gap, ok.top, w, h, TRUE);
        HWND voidButton = CreateWindowExW(0, L"BUTTON", L"&Void", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
                                          2 * gap + w, ok.top, w, h, hwnd(),
                                          reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_AP_VOID)), hInst, nullptr);
        SendMessageW(voidButton, WM_SETFONT, SendMessageW(hwnd(), WM_GETFONT, 0, 0), TRUE);
        SetWindowPos(voidButton, GetDlgItem(hwnd(), IDOK), 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
        MoveWindow(GetDlgItem(hwnd(), IDCANCEL), 3 * gap + 2 * w, ok.top, w, h, TRUE);
        return true;
    }

    bool OnCommand(WORD id, WORD code, HWND ctl) override
    {
        if (code == BN_CLICKED && (id == IDOK || id == IDC_AP_VOID)) {
            date = GetDate(3035);
            if (date.empty()) {
                Message(GetString(8090), MB_ICONINFORMATION);
                return true;
            }
            action = id == IDOK ? Action::Clear : Action::Void;
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
    HWND findLabel(const wchar_t* text) const
    {
        for (HWND h = GetWindow(hwnd(), GW_CHILD); h; h = GetWindow(h, GW_HWNDNEXT)) {
            wchar_t buf[64] = {};
            GetWindowTextW(h, buf, 64);
            if (wcscmp(buf, text) == 0) return h;
        }
        return nullptr;
    }

    ChequeToPrint c_;
};

// ---- Bank Reconciliation (1046): TAPPayment -------------------------------------

class BankRecDialog : public MainDialog {
public:
    explicit BankRecDialog(bool closeAtOnce) : closeAtOnce_(closeAtOnce) {}

protected:
    bool OnInitDialog() override
    {
        MainDialog::OnInitDialog();
        EnsureApTables(cb);
        grid_ = fx::Grid(GetDlgItem(hwnd(), kGrid));
        if (grid_.IsGrid()) {
            const int ch = grid_.CharWidth();
            grid_.SetColumns({{L"Cheque", 7 * ch},
                              {L"Vendor", 8 * ch},
                              {L"State", 8 * ch},
                              {L"Bank", 8 * ch},
                              {L"Amount", 12 * ch, fx::GridAlign::Right}});
            grid_.SetStretchColumn(2);
        }
        Fill();
        if (closeAtOnce_) PostMessageW(hwnd(), WM_COMMAND, IDCANCEL, 0);
        return true;
    }

    bool OnCommand(WORD id, WORD code, HWND ctl) override
    {
        if (id == kGrid) {
            if (code == GRN_PICK) Pick(grid_.ActiveRow());
            return true;
        }
        return MainDialog::OnCommand(id, code, ctl);
    }

private:
    void Fill()
    {
        shown_ = LoadChequesIn(cb, "");   // every cheque not cleared
        if (!grid_.IsGrid()) return;
        grid_.SetRedraw(false);
        grid_.Clear();
        int row = 0;
        for (const ChequeToPrint& c : shown_) {
            ++row;
            grid_.SetData(1, row, std::to_wstring(c.cheque.number));
            grid_.SetData(2, row, fx::Utf8ToWide(c.cheque.vendor));
            grid_.SetData(3, row, c.state == ap_payment::kPrinted ? L"Printed" : c.state == ap_payment::kPaid ? L"Made" : L"");
            grid_.SetData(4, row, fx::Utf8ToWide(c.cheque.bank.account));
            grid_.SetData(5, row, amount(c.cheque.amount));
        }
        grid_.SetRedraw(true);
    }

    void Pick(int row)   // TAPPayment::SelectCell -> TAPSetChequeNumber
    {
        if (row < 1 || row > static_cast<int>(shown_.size())) return;
        const ChequeToPrint c = shown_[static_cast<size_t>(row - 1)];
        ClearDialog dlg(c);
        if (dlg.DoModal(1051, hwnd()) != IDOK) return;
        if (dlg.action == ClearDialog::Action::Clear) {
            switch (ClearCheque(cb, c.cheque.number, dlg.date)) {
            case ClearResult::Cleared: break;
            case ClearResult::NotPrinted:
                Message(fx::Format(L"Cheque %ld hasn't been confirmed printed (Confirm Printed Cheques).", c.cheque.number),
                        MB_ICONINFORMATION);
                break;
            default: Message(fx::Format(L"Cheque %ld can't be cleared.", c.cheque.number), MB_ICONEXCLAMATION); break;
            }
        } else {
            if (Message(fx::Format(L"Void cheque %ld (%s, %s)?\n\nIts invoices become open again and its payment is "
                                   L"reversed.",
                                   c.cheque.number, fx::Utf8ToWide(c.cheque.vendor).c_str(), amount(c.cheque.amount).c_str()),
                        MB_YESNO | MB_ICONQUESTION) != IDYES)
                return;
            const VoidOutcome v = VoidCheque(cb, c.cheque.number, dlg.date);
            switch (v.result) {
            case VoidResult::Voided:
                Message(fx::Format(L"Cheque %ld voided: %d invoice(s) open again; %d journal row(s) removed, %d reversed.",
                                   c.cheque.number, v.itemsReopened, v.rowsRemoved, v.rowsReversed),
                        MB_ICONINFORMATION);
                break;
            case VoidResult::ClosedPeriod:
                Message(L"The date isn't in an open period, and the payment has gone to the General Ledger: it can't be "
                        L"reversed then.",
                        MB_ICONEXCLAMATION);
                break;
            default: Message(fx::Format(L"Cheque %ld can't be voided.", c.cheque.number), MB_ICONEXCLAMATION); break;
            }
        }
        Fill();
    }

    bool closeAtOnce_;
    fx::Grid grid_;
    std::vector<ChequeToPrint> shown_;
};

// ---- Purge Paid Payables (1048): TAPPurgeBatch ----------------------------------

class PurgeDialog : public MainDialog {
public:
    explicit PurgeDialog(bool closeAtOnce) : closeAtOnce_(closeAtOnce) {}

protected:
    bool OnInitDialog() override
    {
        MainDialog::OnInitDialog();
        EnsureApTables(cb);
        SetWindowTextW(hwnd(), L"Purge Paid Payables");
        for (HWND h = GetWindow(hwnd(), GW_CHILD); h; h = GetWindow(h, GW_HWNDNEXT)) {
            wchar_t buf[64] = {};
            GetWindowTextW(h, buf, 64);
            if (wcscmp(buf, L"Posting Voucher Number :") == 0) SetWindowTextW(h, L"Cleared and paid before:");
        }
        // The date, over 3031 (a static the template had for a voucher number).
        const HWND old = GetDlgItem(hwnd(), 3031);
        RECT rc{};
        GetWindowRect(old, &rc);
        MapWindowPoints(nullptr, hwnd(), reinterpret_cast<POINT*>(&rc), 2);
        ShowWindow(old, SW_HIDE);
        const int h = MulDiv(14, static_cast<int>(GetDpiForWindow(hwnd())), 96) + 6;
        HWND date = CreateWindowExW(0, L"tbDate", L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER, rc.left + 10,
                                    rc.top - 3, MulDiv(80, static_cast<int>(GetDpiForWindow(hwnd())), 96), h, hwnd(),
                                    reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_AP_PURGEDATE)), hInst, nullptr);
        SendMessageW(date, WM_SETFONT, SendMessageW(hwnd(), WM_GETFONT, 0, 0), TRUE);
        SetField(IDC_AP_PURGEDATE, today());
        SetDlgItemTextW(hwnd(), IDOK, L"&Purge");
        grid_ = fx::Grid(GetDlgItem(hwnd(), kGrid));
        if (grid_.IsGrid()) {
            const int ch = grid_.CharWidth();
            grid_.SetColumns({{L"Vendor", 8 * ch},
                              {L"Invoice", 12 * ch},
                              {L"Cheque", 8 * ch},
                              {L"Paid", 11 * ch},
                              {L"Amount", 13 * ch, fx::GridAlign::Right}});
            grid_.SetStretchColumn(2);
        }
        Fill();
        if (closeAtOnce_) PostMessageW(hwnd(), WM_COMMAND, IDCANCEL, 0);
        return true;
    }

    bool OnCommand(WORD id, WORD code, HWND ctl) override
    {
        if (id == kGrid) return true;
        if (code == BN_CLICKED && id == IDOK) {
            Purge();
            return true;
        }
        return MainDialog::OnCommand(id, code, ctl);
    }

private:
    void Fill()
    {
        const std::string before = GetDate(IDC_AP_PURGEDATE);
        const std::vector<OpenItem> items = PurgeCandidates(cb, before);
        if (grid_.IsGrid()) {
            grid_.SetRedraw(false);
            grid_.Clear();
            int row = 0;
            for (const OpenItem& i : items) {
                ++row;
                grid_.SetData(1, row, fx::Utf8ToWide(i.vendor));
                grid_.SetData(2, row, fx::Utf8ToWide(i.invoiceNo));
                grid_.SetData(3, row, std::to_wstring(i.cheque));
                grid_.SetData(4, row, fx::Utf8ToWide(rpt::FormatDate(i.datePaid)));
                grid_.SetData(5, row, amount(i.paid));
            }
            grid_.SetRedraw(true);
        }
        SetFieldW(1001, fx::Format(L"%zu cleared invoice(s) paid before %s.", items.size(),
                                   fx::Utf8ToWide(rpt::FormatDate(before)).c_str()));
    }

    void Purge()
    {
        const std::string before = GetDate(IDC_AP_PURGEDATE);
        if (before.empty()) {
            Message(GetString(8090), MB_ICONINFORMATION);
            return;
        }
        const size_t n = PurgeCandidates(cb, before).size();
        Fill();
        if (n == 0) {
            Message(L"No cleared invoice was paid before that date.", MB_ICONINFORMATION);
            return;
        }
        if (Message(fx::Format(L"Delete the %zu cleared invoice(s) paid before %s, and their cheques?\n\nOpen, unpaid "
                               L"and uncleared items are kept.",
                               n, fx::Utf8ToWide(rpt::FormatDate(before)).c_str()),
                    MB_YESNO | MB_ICONQUESTION) != IDYES)
            return;
        const PurgeOutcome o = PurgePaid(cb, before);
        Message(fx::Format(L"%d invoice(s) and %d cheque(s) purged.", o.items, o.cheques), MB_ICONINFORMATION);
        Fill();
    }

    bool closeAtOnce_;
    fx::Grid grid_;
};

} // namespace

INT_PTR RunApCreateCheques(HWND frame, bool closeAtOnce)
{
    CreateChequesDialog dlg(closeAtOnce);
    return dlg.DoModal(1045, frame);
}

INT_PTR RunApConfirmCheques(HWND frame, bool closeAtOnce)
{
    ConfirmDialog dlg(closeAtOnce);
    return dlg.DoModal(IDD_AP_CONFIRM, frame);
}

INT_PTR RunApBankReconciliation(HWND frame, bool closeAtOnce)
{
    BankRecDialog dlg(closeAtOnce);
    return dlg.DoModal(1046, frame);
}

INT_PTR RunApPurge(HWND frame, bool closeAtOnce)
{
    PurgeDialog dlg(closeAtOnce);
    return dlg.DoModal(1048, frame);
}
