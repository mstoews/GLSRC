//
// ReportList.cpp - see ReportList.h.
//
// Translation notes (plan per-port checklist):
//   - TGLCustomReport filled a tbSpread from gl_rept ("No.", "Description");
//     the port's fx::Grid has the same two columns, filled from
//     ReportListEntries.
//   - HandlePrint ran the active row's .RPT through Crystal; Print here
//     returns the row's command, which the frame runs as if it had been
//     chosen from the menu, after the dialog has closed.
//   - Spec #7 (S7-M1): the user reports (gl_rptdef) follow the built-in ones,
//     a "Made by" column telling them apart. Delete (1118) removes a user
//     report after asking, and refuses a built-in one.
//   - S7-M2: Add/Mod (4095) is "Add...": the designer on a new report. Modify
//     (2026) opens the chosen user report in it (the original's appended a
//     duplicate record instead of changing one); Copy, a button added in code
//     (IDC_RW_COPY), opens a copy. Built-in reports can't be modified or
//     copied, and say so. The six buttons are stacked in code in the
//     template's column: Print, Add, Modify, Copy, Delete, Close.
//
#include "ReportList.h"

#include "../Globals.h"
#include "../ReportWriter.h"
#include "ReportDesigner.h"

#include "win32fx.h"

#include <algorithm>
#include <exception>
#include <iterator>
#include <string>
#include <vector>

#include "../PortIds.h"     // CM_AR_STATEMENTS

#include "../LegacyIds.h"   // last: see LegacyIds.h

namespace {

constexpr int kGridId = ID_DISTLIST;   // 1075
constexpr int kAddModId = 4095;
constexpr int kDeleteId = 1118;
constexpr int kModifyId = 2026;
constexpr int kPrintId = ID_PRINT;     // 3068

const ListedReport kReports[] = {
    {ID_TB, L"Trial Balance"},
    {ID_DISTLIST, L"Distribution List (distribution trial balance)"},
    {1015, L"Income Statement"},                    // ID_FININCOME
    {1016, L"Balance Sheet"},                       // ID_FINBS
    {1018, L"Cash Flow Statement"},                 // ID_FINRE (D13)
    {ID_TBBYDEPT, L"Trial Balance by Dept."},
    {10071, L"Chart of Accounts Report"},           // ID_GLCHARTINQ
    {10073, L"Budget Report"},                      // ID_GLBUDGETINQ
    {ID_SLIP, L"Voucher Slips"},
    {CM_SLIPDETAIL, L"Voucher Detail"},
    {ID_SLIPSUM, L"Voucher Summary"},
    {ID_PREPOST, L"Preposting Distribution List"},
    {8136, L"General Ledger Pre-Posting Report"},   // TManilaGL::GLPrePosting
    {ID_REPLIST, L"Medical Representatives"},
    {ID_PRODUCTLIST, L"Product code summary"},
    {ID_DOCTOR_RPT, L"Doctor Codes"},
    {1019, L"Fixed Assets Listing"},                // GLFinFA (spec #4 7)
    {1020, L"Fixed Asset Types"},                   // GLFinDep
    {1078, L"AP Invoice Pre-Posting"},              // spec #5 7
    {1081, L"AP Cash Pre-Posting"},
    {8077, L"AP Voucher Detail"},
    {8078, L"AP Cash Voucher Detail"},
    {1065, L"AP Voucher Status"},
    {1049, L"AP Invoice Posting Journal"},
    {1060, L"Vendor List"},
    {1061, L"Open Payables Detail"},
    {1062, L"Aged Payables 30-60-90"},
    {1064, L"Cash Requirement"},
    {1077, L"Accounts Payable Register"},
    {1058, L"Prepayments"},
    {4091, L"Cheques"},
    {4092, L"Payment Detail"},
    {8073, L"Cash Cheque Summary"},
    {8067, L"VAT: AP"},
    {8072, L"VAT: General Ledger"},
    {8071, L"Withholding Tax"},
    {8068, L"AP Journal Summary"},
    {8069, L"AP Journal Detail"},
    {8075, L"AP Journal: Cash"},
    {8076, L"AP Journal: Invoices and Payments"},
    {4063, L"AR Sales Pre-Posting"},              // spec #6 7
    {4073, L"AR Sales Voucher Detail"},
    {4070, L"AR Voucher Status"},
    {4056, L"Customer List"},
    {4058, L"Customer Summary"},
    {4057, L"Open Receivables Detail"},
    {4060, L"Accounts Receivable Register"},
    {4068, L"Aged Receivables 30-60-90"},
    {4061, L"Collections Forecast"},
    {4076, L"Receipts Register"},
    {4071, L"VAT: Sales"},
    {4072, L"VAT: General Ledger (output VAT)"},
    {4074, L"AR Journal Summary"},
    {4075, L"AR Journal Detail"},
    {CM_AR_STATEMENTS, L"Customer Statements"},
};

class ReportListDialog : public fx::Dialog {
public:
    ReportChoice chosen;

protected:
    bool OnInitDialog() override
    {
        Buttons();

        grid_ = fx::Grid(GetDlgItem(hwnd(), kGridId));
        if (!grid_.IsGrid()) return true;
        const int digit = grid_.CharWidth();
        // The first column is always left-aligned.
        grid_.SetColumns({{L"No.", digit * 5}, {L"Description", digit * 20}, {L"Made by", digit * 11}});
        Fill(1);
        SetFocus(grid_.hwnd());
        return false;   // the focus is set
    }

    bool OnCommand(WORD id, WORD code, HWND) override
    {
        if (id == kPrintId || (id == kGridId && code == GRN_PICK)) {
            const Entry* e = At(grid_.ActiveRow());
            if (!e) {
                MessageBeep(MB_OK);
                return true;
            }
            chosen = {e->command, e->definition};
            End(IDOK);
            return true;
        }
        if (code == BN_CLICKED) {
            switch (id) {
            case kDeleteId: Delete(); return true;
            case kAddModId: Design(0, false); return true;
            case kModifyId:
                if (const long n = Chosen(L"changed")) Design(n, false);
                return true;
            case IDC_RW_COPY:
                if (const long n = Chosen(L"copied")) Design(n, true);
                return true;
            default: break;
            }
        }
        return false;   // IDCANCEL: closes (fx::Dialog)
    }

private:
    struct Entry {
        WORD command = 0;
        long definition = 0;
        std::wstring name, madeBy;
    };

    fx::Grid grid_;
    std::vector<Entry> entries_;

    const Entry* At(int row) const
    {
        return row >= 1 && row <= static_cast<int>(entries_.size()) ? &entries_[static_cast<size_t>(row - 1)] : nullptr;
    }

    // The built-in reports, then the user reports in name order.
    void Fill(int activeRow)
    {
        entries_.clear();
        for (const ListedReport& r : kReports) entries_.push_back({r.command, 0, r.name, {}});
        try {
            for (const rw::StoredDefinition& d : rw::LoadDefinitions(cb))
                entries_.push_back({0, d.number, fx::Utf8ToWide(d.name),
                                    fx::Utf8ToWide(d.madeBy.empty() ? std::string("(you)") : d.madeBy)});
        } catch (const std::exception& e) {
            MessageBoxW(hwnd(), (L"The user reports could not be read:\n\n" + fx::Utf8ToWide(e.what())).c_str(), szApp,
                        MB_OK | MB_ICONEXCLAMATION);
        }
        grid_.SetRedraw(false);
        grid_.Clear();
        int row = 1;
        for (const Entry& e : entries_) {
            grid_.SetData(1, row, std::to_wstring(row));
            grid_.SetData(2, row, e.name);
            grid_.SetData(3, row, e.madeBy);
            ++row;
        }
        grid_.SetRedraw(true);
        // Description takes the rest of the list's width, once the rows (and
        // so the vertical scroll bar) are in: no horizontal scroll bar.
        RECT rc{};
        GetClientRect(grid_.list(), &rc);
        grid_.SetColumnWidth(2, rc.right - grid_.ColumnWidth(1) - grid_.ColumnWidth(3));
        grid_.SetActiveCell(1, std::min(std::max(activeRow, 1), static_cast<int>(entries_.size())));
    }

    // The template's five buttons and Copy, stacked in its button column:
    // Print, Add, Modify, Copy, Delete, Close.
    void Buttons()
    {
        HWND add = GetDlgItem(hwnd(), kAddModId);
        RECT r{};
        GetWindowRect(add, &r);
        MapWindowPoints(nullptr, hwnd(), reinterpret_cast<POINT*>(&r), 2);
        const LONG style = GetWindowLongW(add, GWL_STYLE);
        HWND copy = CreateWindowExW(0, L"tbSuperBtn", L"C&opy...", static_cast<DWORD>(style) | WS_VISIBLE, r.left, r.top,
                                    r.right - r.left, r.bottom - r.top, hwnd(),
                                    reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_RW_COPY)), hInst, nullptr);
        SendMessageW(copy, WM_SETFONT, SendMessageW(hwnd(), WM_GETFONT, 0, 0), TRUE);
        SetWindowTextW(add, L"&Add...");
        SetWindowTextW(GetDlgItem(hwnd(), kModifyId), L"&Modify...");
        RECT unit{0, 4, 0, 17};   // dialog units: the first button's top, and the step
        MapDialogRect(hwnd(), &unit);
        int y = unit.top;
        HWND after = HWND_TOP;
        for (int id : {kPrintId, kAddModId, kModifyId, static_cast<int>(IDC_RW_COPY), kDeleteId, static_cast<int>(IDCANCEL)}) {
            HWND b = GetDlgItem(hwnd(), id);
            SetWindowPos(b, after, r.left, y, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE);   // and the tab order
            after = b;
            y += unit.bottom;
        }
    }

    // The chosen user report's number; 0, with a message, for none or a built-in one.
    long Chosen(const wchar_t* verb)
    {
        const Entry* e = At(grid_.ActiveRow());
        if (!e) {
            MessageBeep(MB_OK);
            return 0;
        }
        if (!e->definition) {
            MessageBoxW(hwnd(),
                        (L"\u201C" + e->name + L"\u201D is built into the program: it can't be " + verb +
                         L". Add makes a report of your own.")
                            .c_str(),
                        szApp, MB_OK | MB_ICONINFORMATION);
            return 0;
        }
        return e->definition;
    }

    // The designer on a new report (number 0, not a copy), a user report, or a copy of one.
    void Design(long number, bool copy)
    {
        const long saved = RunReportDesigner(hwnd(), number, copy);
        int row = grid_.ActiveRow();
        Fill(row);
        const long show = saved > 0 ? saved : number;
        for (size_t i = 0; i < entries_.size(); ++i)
            if (show && entries_[i].definition == show) row = static_cast<int>(i) + 1;
        grid_.SetActiveCell(1, row);
        SetFocus(grid_.hwnd());
    }

    void Delete()
    {
        const int row = grid_.ActiveRow();
        const Entry* e = At(row);
        if (!e) {
            MessageBeep(MB_OK);
            return;
        }
        if (!e->definition) {
            MessageBoxW(hwnd(), (L"\u201C" + e->name + L"\u201D is built into the program: it can't be deleted.").c_str(),
                        szApp, MB_OK | MB_ICONINFORMATION);
            return;
        }
        const std::wstring ask = L"Delete the report \u201C" + e->name +
                                 L"\u201D?\n\nOnly the report is deleted: the books aren't touched. It can't be undone.";
        if (MessageBoxW(hwnd(), ask.c_str(), szApp, MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) != IDYES) return;
        try {
            rw::DeleteDefinition(cb, e->definition);
        } catch (const std::exception& ex) {
            MessageBoxW(hwnd(), (L"The report could not be deleted:\n\n" + fx::Utf8ToWide(ex.what())).c_str(), szApp,
                        MB_OK | MB_ICONSTOP);
        }
        Fill(row);
        SetFocus(grid_.hwnd());
    }
};

} // namespace

const ListedReport* ReportListEntries(int& count) noexcept
{
    count = static_cast<int>(std::size(kReports));
    return kReports;
}

ReportChoice ShowReportList(HWND owner)
{
    ReportListDialog dlg;
    return dlg.DoModal(hInst, 1022, owner) == IDOK ? dlg.chosen : ReportChoice{};
}
