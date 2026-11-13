//
// ArReceipts.cpp - see ArReceipts.h.
//
// Translation notes (plan per-port checklist; decisions are spec #6 §8):
//   - D9: TARClear's OK only marked the invoice (P, R or D) with the amount
//     and bank; Create then wrote the journal, in the invoice's date and
//     period: a partial journalled 0.00, a discount credited the original
//     amount and booked its sales part to a session global (2.6 #1-#4). The
//     port records the receipt at OK (Ar.h RecordReceipt): its own row in
//     AR_RCPT, its journal dated the receipt, in its open period, and the
//     open item R or C. The list's Create button becomes Receipts (D10).
//   - A short amount with neither box ticked asks, as the original (8132):
//     Yes records it with a discount, No as a partial. Partial with the whole
//     amount is a receipt in full (the original refused it, 8134); a
//     discount with the whole amount is still refused.
//   - The discount's accounts are the invoice's own (its sales and VAT
//     lines), shown in the account boxes; the original showed the customer's.
//   - Q3, D15: the W/H box, its account and label are hidden.
//   - Reset put AMOUNTPAID to 0 and closed; nothing is marked any more, so
//     it puts the screen back as it opened.
//   - Set View (ID_SETDATA: TSetData, a customer to show) had no template
//     in either .rc; it is hidden. The status line says what the screen did.
//   - The list's columns are the original's (10); the date paid and the
//     bank are on the receipt.
//   - Receipts (D10) lists the invoice's receipts and offers to reverse the
//     latest, dated today.
//   - Purge (TARPurgeBatch, 1062; S6-M5, D11): Ok deleted every "C" item,
//     whatever its date, and double-click one; both closed every table. The
//     port lists the settled items whose last receipt is before a date (a
//     date control added in code over 3031, whose label, "Purge Voucher
//     Number :", is relabelled), deletes them with their receipts (Ar.h
//     PurgeReceived) after asking, and keeps the vouchers and journal. The
//     template's "&Post" button is "&Purge".
//
#include "ArReceipts.h"

#include "Accounts.h"   // TrimText
#include "Ar.h"
#include "Globals.h"
#include "MainDialog.h"
#include "VoucherLists.h"   // FormatAmount

#include "report/Format.h"   // rpt::FormatDate

#include <cmath>
#include <cstdio>
#include <exception>
#include <string>
#include <vector>

#include "PortIds.h"

#include "LegacyIds.h"   // last: see LegacyIds.h

namespace {

// Dialog 1156's controls.
constexpr int kGrid = ID_DISTLIST;    // 1075
constexpr int kStatus = 5013;         // IDD_STATUSBOX
constexpr int kReceipts = IDOK;       // "Create": Receipts (D10)
constexpr int kViewAll = 1121;        // ID_VIEW
constexpr int kSetView = 4076;        // ID_SETDATA

// Dialog 1155's controls (ar_payrq.cpp's names).
constexpr int kCustomer = 4001;       // ID_VENDORNO
constexpr int kInvoice = 4012;        // ID_INVOICENO
constexpr int kTotal = 2030;          // ID_DEBIT: the invoice total
constexpr int kReceived = 1031;       // ID_PAIDTODATE
constexpr int kOwed = 1032;           // ID_DUEAMOUNT
constexpr int kAmount = 1030;         // ID_AMOUNT: this receipt
constexpr int kDate = 3035;           // ID_DATE
constexpr int kBank = 2015;           // ID_DESCRIPT: the GL_SETAT cash accounts
constexpr int kBankAccount = 2013;    // ID_ACCOUNTS
constexpr int kBankDept = 2014;       // ID_DEPART
constexpr int kSalesAccount = 2016, kSalesDept = 2017, kVatAccount = 2018, kVatDept = 2019;
constexpr int kArAccount = 2020, kArDept = 2021;
constexpr int kWhAccount = 2122, kWhDept = 2123;
constexpr int kDiscount = 2023;       // ID_DISCOUNT
constexpr int kPartial = 2025;        // ID_PARTIAL
constexpr int kWithholding = 4026;    // ID_WITHHOLDING
constexpr int kReset = 1094;          // ID_RESET

std::string today()
{
    SYSTEMTIME t{};
    GetLocalTime(&t);
    char buf[16] = {};
    std::snprintf(buf, sizeof buf, "%04u%02u%02u", t.wYear, t.wMonth, t.wDay);
    return buf;
}

std::wstring amount(std::int64_t cents) { return FormatAmount(static_cast<double>(cents) / 100.0, false); }

std::wstring dateText(const std::string& yyyymmdd)
{
    const std::string d = TrimText(yyyymmdd);
    if (d.size() != 8) return fx::Utf8ToWide(d);
    return fx::Utf8ToWide(d.substr(4, 2) + "/" + d.substr(6, 2) + "/" + d.substr(2, 2));
}

// A static whose text is `text`, among the dialog's children (the labels are -1).
HWND findLabel(HWND dlg, const wchar_t* text)
{
    struct Find {
        const wchar_t* text;
        HWND found;
    } f{text, nullptr};
    EnumChildWindows(
        dlg,
        [](HWND h, LPARAM lp) -> BOOL {
            auto* f = reinterpret_cast<Find*>(lp);
            wchar_t cls[16] = {}, buf[64] = {};
            GetClassNameW(h, cls, 16);
            GetWindowTextW(h, buf, 64);
            if (_wcsicmp(cls, L"Static") == 0 && wcscmp(buf, f->text) == 0) {
                f->found = h;
                return FALSE;
            }
            return TRUE;
        },
        reinterpret_cast<LPARAM>(&f));
    return f.found;
}

// ---- TARClear (1155): one receipt ----------------------------------------------

class ReceiptDialog : public MainDialog {
public:
    explicit ReceiptDialog(const ReceiptItem& item) : item_(item) {}

    ReceiptOutcome outcome;

protected:
    bool OnInitDialog() override
    {
        MainDialog::OnInitDialog();   // Clear (IDOK) and Close pictures
        SetBitmap(kReset);
        for (int id : {kWithholding, kWhAccount, kWhDept}) ShowWindow(GetDlgItem(hwnd(), id), SW_HIDE);   // Q3
        if (HWND l = findLabel(hwnd(), L"Withholding Tax ")) ShowWindow(l, SW_HIDE);
        if (HWND l = findLabel(hwnd(), L"Vendor Number")) SetWindowTextW(l, L"Customer");
        for (int id : {kCustomer, kInvoice, kTotal, kReceived, kOwed, kSalesAccount, kSalesDept, kVatAccount, kVatDept,
                       kArAccount, kArDept})
            EnableWindow(GetDlgItem(hwnd(), id), FALSE);
        setat_ = LoadArAccounts(cb);
        const HWND box = GetDlgItem(hwnd(), kBank);   // SetCashAccounts
        for (size_t i = 0; i < setat_.banks.size(); ++i) {
            const ApBank& b = setat_.banks[i];
            const std::wstring text = fx::Utf8ToWide(b.shortName + " - " + TrimText(b.name));
            const LRESULT at = SendMessageW(box, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(text.c_str()));
            SendMessageW(box, CB_SETITEMDATA, static_cast<WPARAM>(at), static_cast<LPARAM>(i));
        }
        const ArOpenItem& i = item_.item;
        SetField(kCustomer, i.customer);
        SetField(kInvoice, i.invoiceNo);
        FloatSetDlgItemValue(hwnd(), kTotal, static_cast<double>(i.amount) / 100.0);
        FloatSetDlgItemValue(hwnd(), kReceived, static_cast<double>(i.received) / 100.0);
        FloatSetDlgItemValue(hwnd(), kOwed, static_cast<double>(i.remainder) / 100.0);
        SetField(kSalesAccount, item_.sales.account);
        SetField(kSalesDept, item_.sales.dept);
        SetField(kVatAccount, item_.vat.account);
        SetField(kVatDept, item_.vat.dept);
        SetField(kArAccount, i.ar.account);
        SetField(kArDept, i.ar.dept);
        Reset();
        return false;   // the focus is set
    }

    bool OnCommand(WORD id, WORD code, HWND ctl) override
    {
        if (id == kBank) {   // GetBankAccount
            if (code == CBN_SELCHANGE) ShowBank();
            return true;
        }
        if (code != BN_CLICKED) return MainDialog::OnCommand(id, code, ctl);
        switch (id) {
        case kDiscount:   // SetDiscount / SetPartial: the boxes toggle by hand, one at a time
        case kPartial: {
            const bool on = !IsChecked(id);
            SetCheck(kDiscount, id == kDiscount && on);
            SetCheck(kPartial, id == kPartial && on);
            return true;
        }
        case kReset: Reset(); return true;
        case IDOK: Ok(); return true;
        case IDCANCEL: End(IDCANCEL); return true;   // not MainDialog::Cancel: the list's tables stay open
        default: return MainDialog::OnCommand(id, code, ctl);
        }
    }

private:
    void ShowBank()
    {
        const HWND box = GetDlgItem(hwnd(), kBank);
        const LRESULT sel = SendMessageW(box, CB_GETCURSEL, 0, 0);
        const LRESULT i = sel == CB_ERR ? -1 : SendMessageW(box, CB_GETITEMDATA, static_cast<WPARAM>(sel), 0);
        if (i < 0 || static_cast<size_t>(i) >= setat_.banks.size()) return;
        SetField(kBankAccount, setat_.banks[static_cast<size_t>(i)].account.account);
        SetField(kBankDept, setat_.banks[static_cast<size_t>(i)].account.dept);
    }

    void SelectBank(const ApAccount& a)
    {
        const HWND box = GetDlgItem(hwnd(), kBank);
        const LRESULT count = SendMessageW(box, CB_GETCOUNT, 0, 0);
        LRESULT pick = count > 0 ? 0 : CB_ERR;   // the first bank, unless the last receipt's is there
        for (LRESULT at = 0; at < count; ++at) {
            const LRESULT i = SendMessageW(box, CB_GETITEMDATA, static_cast<WPARAM>(at), 0);
            if (i >= 0 && static_cast<size_t>(i) < setat_.banks.size() && !a.empty() &&
                setat_.banks[static_cast<size_t>(i)].account == a)
                pick = at;
        }
        SendMessageW(box, CB_SETCURSEL, static_cast<WPARAM>(pick), 0);
        ShowBank();
    }

    void Reset()   // the screen as it opened
    {
        FloatSetDlgItemValue(hwnd(), kAmount, static_cast<double>(item_.item.remainder) / 100.0);
        SetField(kDate, today());
        SetCheck(kDiscount, false);
        SetCheck(kPartial, false);
        SelectBank(item_.item.bank);
        SetFocus(GetDlgItem(hwnd(), kAmount));
    }

    void Ok()   // TARClear::Ok, recording at once (D9)
    {
        double v = 0;
        FloatGetDlgItemValue(hwnd(), kAmount, &v);
        const std::int64_t received = std::llround(v * 100.0);
        const std::int64_t owed = item_.item.remainder;
        const std::string date = GetDate(kDate);
        if (date.empty()) {
            Refuse(kDate, GetString(8090));   // "Double click on the date field to set the date."
            return;
        }
        bool discount = IsChecked(kDiscount);
        if (received > 0 && received < owed && !discount && !IsChecked(kPartial)) {
            switch (Message(GetString(8132), MB_YESNOCANCEL | MB_ICONQUESTION)) {   // "...record the payment as a discount?"
            case IDYES: discount = true; break;
            case IDNO: break;
            default: return;
            }
        }
        const ApAccount bank{StrTrim(GetField(kBankAccount)), StrTrim(GetField(kBankDept))};
        try {
            outcome = RecordReceipt(cb, item_.item.batchno, date, bank, received, discount);
        } catch (const std::exception& e) {
            Message(fx::Format(L"The receipt failed, and nothing was changed.\n\n%s", fx::Utf8ToWide(e.what()).c_str()),
                    MB_ICONSTOP);
            return;
        }
        switch (outcome.result) {
        case ReceiptResult::Recorded: End(IDOK); return;
        case ReceiptResult::Settled: Refuse(0, GetString(8127)); return;    // "The invoice has already been cleared..."
        case ReceiptResult::Zero: Refuse(kAmount, GetString(1037)); return;  // "Amount paid can not be zero."
        case ReceiptResult::TooMuch:                                         // "Total entered is greater than the amount due."
            Refuse(kAmount, GetString(8133));
            return;
        case ReceiptResult::NoDiscount:   // "Amount is not discounted."
            Refuse(kAmount, GetString(8134) + L"\n\nThe whole amount owed was received: untick Discount.");
            return;
        case ReceiptResult::ClosedPeriod:
            Refuse(kDate, L"The date isn't in an open period (Company Profile > Period Dates).");
            return;
        case ReceiptResult::UnknownBank:   // "Unable to locate the General Ledger account."
            Refuse(kBank, GetString(8131) + L"\n\nChoose the bank the money went into.");
            return;
        case ReceiptResult::NotFound: End(IDCANCEL); return;
        }
    }

    void Refuse(int id, const std::wstring& text)
    {
        Message(text, MB_ICONEXCLAMATION);
        if (id) SetFocus(GetDlgItem(hwnd(), id));
    }

    ReceiptItem item_;
    ArAccounts setat_;
};

// ---- TARCreatePayment (1156): the list -------------------------------------------

class ReceiptsDialog : public MainDialog {
public:
    explicit ReceiptsDialog(bool closeAtOnce) : closeAtOnce_(closeAtOnce) {}

protected:
    bool OnInitDialog() override
    {
        MainDialog::OnInitDialog();   // Create (IDOK) and Close pictures
        SetBitmap(kViewAll);
        SetWindowTextW(GetDlgItem(hwnd(), kReceipts), L"&Receipts");
        ShowWindow(GetDlgItem(hwnd(), kSetView), SW_HIDE);
        SetWindowTextW(hwnd(), L"Clear Receipts");
        FitStatus();
        EnsureArTables(cb);
        grid_ = fx::Grid(GetDlgItem(hwnd(), kGrid));
        if (grid_.IsGrid()) {
            const int ch = grid_.CharWidth();
            grid_.SetColumns({{L"Customer", 14 * ch},
                              {L"Ctl", 5 * ch},
                              {L"Code", 7 * ch},
                              {L"Invoice", 9 * ch},
                              {L"Due Date", 10 * ch},
                              {L"Amount", 11 * ch, fx::GridAlign::Right},
                              {L"VAT", 10 * ch, fx::GridAlign::Right},
                              {L"Inv. Total", 11 * ch, fx::GridAlign::Right},
                              {L"Received", 11 * ch, fx::GridAlign::Right},
                              {L"Owed", 11 * ch, fx::GridAlign::Right}});
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
        if (code != BN_CLICKED) return MainDialog::OnCommand(id, code, ctl);
        switch (id) {
        case kReceipts: ShowReceipts(grid_.ActiveRow()); return true;
        case kViewAll: Fill(!all_); return true;
        default: return MainDialog::OnCommand(id, code, ctl);   // IDCANCEL -> Cancel()
        }
    }

private:
    void Status(const std::wstring& text) { SetFieldW(kStatus, text); }

    void FitStatus()   // the status pane along the bottom of the client area, a line high
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
        SetWindowTextW(GetDlgItem(hwnd(), kViewAll), all ? L"View &Owed" : L"&View All");
        items_ = ReceiptItems(cb, all);
        if (!grid_.IsGrid()) return;
        grid_.SetRedraw(false);
        grid_.Clear();
        int row = 0;
        std::int64_t owed = 0;
        for (const ReceiptItem& r : items_) {
            const ArOpenItem& i = r.item;
            ++row;
            grid_.SetData(1, row, fx::Utf8ToWide(r.customerName));
            grid_.SetData(2, row, fx::Utf8ToWide(i.paymentCtl));
            grid_.SetData(3, row, fx::Utf8ToWide(i.customer));
            grid_.SetData(4, row, fx::Utf8ToWide(i.invoiceNo));
            grid_.SetData(5, row, dateText(i.dueDate));
            grid_.SetData(6, row, amount(i.amount - i.vat));
            grid_.SetData(7, row, amount(i.vat));
            grid_.SetData(8, row, amount(i.amount));
            grid_.SetData(9, row, amount(i.received));
            grid_.SetData(10, row, amount(i.remainder));
            owed += i.remainder;
        }
        grid_.SetRedraw(true);
        Status(items_.empty() ? std::wstring(all ? L"There are no posted sales invoices."
                                                 : L"No invoice has anything owed.")
                              : fx::Format(L"%d invoice(s); %s owed.", row, amount(owed).c_str()));
    }

    const ReceiptItem* At(int row) const
    {
        return row >= 1 && row <= static_cast<int>(items_.size()) ? &items_[static_cast<size_t>(row - 1)] : nullptr;
    }

    void Pick(int row)   // TARCreatePayment::SelectCell
    {
        const ReceiptItem* r = At(row);
        if (!r) return;
        if (r->item.paymentCtl == ar_receipt::kSettled || r->item.remainder <= 0) {
            Message(GetString(8127), MB_ICONINFORMATION);   // "The invoice has already been cleared..."
            return;
        }
        ReceiptDialog dlg(*r);
        if (dlg.DoModal(1155, hwnd()) != IDOK) return;
        const ReceiptOutcome& o = dlg.outcome;
        const std::wstring invoice = fx::Utf8ToWide(r->item.invoiceNo);   // Fill replaces the items
        Fill(all_);
        Status(fx::Format(L"Receipt %ld recorded for invoice %s%s.", o.number, invoice.c_str(),
                          o.discount ? fx::Format(L", with a discount of %s", amount(o.discount).c_str()).c_str()
                          : o.settled ? L": settled"
                                      : L""));
    }

    void ShowReceipts(int row)   // D10
    {
        const ReceiptItem* r = At(row);
        if (!r) {
            Message(L"Pick an invoice in the list first.", MB_ICONINFORMATION);
            return;
        }
        const long batchno = r->item.batchno;
        const std::wstring invoice = fx::Utf8ToWide(r->item.invoiceNo);
        const std::vector<Receipt> receipts = LoadReceipts(cb, batchno);
        if (receipts.empty()) {
            Message(fx::Format(L"Invoice %s has no receipts.", invoice.c_str()), MB_ICONINFORMATION);
            return;
        }
        std::wstring text = fx::Format(L"Receipts for invoice %s:\n\n", invoice.c_str());
        for (const Receipt& c : receipts) {
            text += fx::Format(L"No. %ld   %s   %s", c.number, dateText(c.date).c_str(), amount(c.amount).c_str());
            if (c.discount) text += fx::Format(L"  + discount %s", amount(c.discount).c_str());
            text += fx::Format(L"   %s", fx::Utf8ToWide(c.bank.account).c_str());
            if (!TrimText(c.reference).empty()) text += L"   " + fx::Utf8ToWide(TrimText(c.reference));
            text += L"\n";
        }
        const Receipt& last = receipts.back();
        text += fx::Format(L"\nReverse the latest, receipt %ld?\n\nIf its journal has gone to the General Ledger, "
                           L"new entries reverse it, dated today.",
                           last.number);
        if (Message(text, MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) != IDYES) return;
        ReverseResult result;
        try {
            result = ReverseReceipt(cb, batchno, today());
        } catch (const std::exception& e) {
            Message(fx::Format(L"The reversal failed, and nothing was changed.\n\n%s", fx::Utf8ToWide(e.what()).c_str()),
                    MB_ICONSTOP);
            return;
        }
        if (result == ReverseResult::ClosedPeriod) {
            Message(fx::Format(L"Receipt %ld's journal has gone to the General Ledger, and today isn't in an open "
                               L"period: it can't be reversed today.",
                               last.number),
                    MB_ICONEXCLAMATION);
            return;
        }
        Fill(all_);
        Status(fx::Format(L"Receipt %ld reversed: invoice %s is owed again.", last.number, invoice.c_str()));
    }

    bool closeAtOnce_;
    bool all_ = false;
    fx::Grid grid_;
    std::vector<ReceiptItem> items_;
};

// ---- TARPurgeBatch (1062) ----------------------------------------------------------

class PurgeDialog : public MainDialog {
public:
    explicit PurgeDialog(bool closeAtOnce) : closeAtOnce_(closeAtOnce) {}

protected:
    bool OnInitDialog() override
    {
        MainDialog::OnInitDialog();
        EnsureArTables(cb);
        SetWindowTextW(hwnd(), L"Purge Received Receivables");
        if (HWND l = findLabel(hwnd(), L"Purge Voucher Number :")) SetWindowTextW(l, L"Received before:");
        // The date, over 3031 (a static the template had for a voucher number).
        const HWND old = GetDlgItem(hwnd(), 3031);
        RECT rc{};
        GetWindowRect(old, &rc);
        MapWindowPoints(nullptr, hwnd(), reinterpret_cast<POINT*>(&rc), 2);
        ShowWindow(old, SW_HIDE);
        const int dpi = static_cast<int>(GetDpiForWindow(hwnd()));
        HWND date = CreateWindowExW(0, L"tbDate", L"", WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER, rc.left - 20,
                                    rc.top - 3, MulDiv(80, dpi, 96), MulDiv(14, dpi, 96) + 6, hwnd(),
                                    reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_AR_PURGEDATE)), hInst, nullptr);
        SendMessageW(date, WM_SETFONT, SendMessageW(hwnd(), WM_GETFONT, 0, 0), TRUE);
        SetField(IDC_AR_PURGEDATE, today());
        SetDlgItemTextW(hwnd(), IDOK, L"&Purge");
        grid_ = fx::Grid(GetDlgItem(hwnd(), kGrid));
        if (grid_.IsGrid()) {
            const int ch = grid_.CharWidth();
            grid_.SetColumns({{L"Vch", 6 * ch},
                              {L"Customer / Reference", 24 * ch},
                              {L"Invoice", 10 * ch},
                              {L"Received", 12 * ch},
                              {L"Amount", 13 * ch, fx::GridAlign::Right}});
            grid_.SetStretchColumn(2);
        }
        Fill();
        if (closeAtOnce_) PostMessageW(hwnd(), WM_COMMAND, IDCANCEL, 0);
        return true;
    }

    bool OnCommand(WORD id, WORD code, HWND ctl) override
    {
        if (id == kGrid) {
            if (code == GRN_PICK) PurgeOne(grid_.ActiveRow());
            return true;
        }
        if (id == IDC_AR_PURGEDATE) {
            if (code == EN_CHANGE || code == EN_KILLFOCUS) Fill();
            return true;
        }
        if (code == BN_CLICKED && id == IDOK) {
            PurgeAll();
            return true;
        }
        return MainDialog::OnCommand(id, code, ctl);
    }

private:
    std::string Before() const { return GetDate(IDC_AR_PURGEDATE); }

    void Fill()
    {
        items_ = ArPurgeCandidates(cb, Before());
        if (!grid_.IsGrid()) return;
        grid_.SetRedraw(false);
        grid_.Clear();
        int row = 0;
        for (const ArOpenItem& i : items_) {
            ++row;
            grid_.SetData(1, row, std::to_wstring(i.batchno));
            grid_.SetData(2, row, fx::Utf8ToWide(i.customer + "  " + TrimText(i.reference)));
            grid_.SetData(3, row, fx::Utf8ToWide(i.invoiceNo));
            grid_.SetData(4, row, fx::Utf8ToWide(rpt::FormatDate(TrimText(i.datePaid))));
            grid_.SetData(5, row, amount(i.amount));
        }
        grid_.SetRedraw(true);
    }

    void PurgeAll()   // TARPurgeBatch::Ok
    {
        const std::string before = Before();
        if (before.empty()) {
            Message(GetString(8090), MB_ICONINFORMATION);   // "Double click on the date field to set the date."
            return;
        }
        Fill();
        if (items_.empty()) {
            Message(L"No settled invoice was last received before that date.", MB_ICONINFORMATION);
            return;
        }
        if (Message(fx::Format(L"Delete the %zu settled invoice(s) last received before %s, and their receipts?\n\n"
                               L"Invoices with something still owed are kept, and so are the vouchers and their "
                               L"journal.",
                               items_.size(), fx::Utf8ToWide(rpt::FormatDate(before)).c_str()),
                    MB_YESNO | MB_ICONQUESTION) != IDYES)
            return;
        Run(before, 0);
    }

    void PurgeOne(int row)   // TARPurgeBatch::SelectItem
    {
        if (row < 1 || row > static_cast<int>(items_.size())) return;
        const ArOpenItem i = items_[static_cast<size_t>(row - 1)];
        if (Message(GetString(6028), MB_ICONQUESTION | MB_YESNO) != IDYES) return;   // "...purge this paid account?"
        Run(Before(), i.batchno);
    }

    void Run(const std::string& before, long only)
    {
        ArPurgeOutcome o;
        try {
            o = PurgeReceived(cb, before, only);
        } catch (const std::exception& e) {
            Message(fx::Format(L"The purge failed, and nothing was changed.\n\n%s", fx::Utf8ToWide(e.what()).c_str()),
                    MB_ICONSTOP);
            return;
        }
        Message(fx::Format(L"%d invoice(s) and %d receipt(s) purged.", o.items, o.receipts), MB_ICONINFORMATION);
        Fill();
    }

    bool closeAtOnce_;
    fx::Grid grid_;
    std::vector<ArOpenItem> items_;
};

} // namespace

INT_PTR RunArReceipts(HWND frame, bool closeAtOnce)
{
    ReceiptsDialog dlg(closeAtOnce);
    return dlg.DoModal(1156, frame);
}

INT_PTR RunArPurge(HWND frame, bool closeAtOnce)
{
    PurgeDialog dlg(closeAtOnce);
    return dlg.DoModal(1062, frame);
}
