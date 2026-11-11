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
//   - Add/Mod (4095), Delete (1118) and Modify (2026) are hidden, and Print
//     and Close move up into their places. (Modify appended a duplicate
//     record instead of changing one.)
//
#include "ReportList.h"

#include "../Globals.h"

#include "win32fx.h"

#include <iterator>
#include <string>

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
};

class ReportListDialog : public fx::Dialog {
public:
    WORD chosen = 0;

protected:
    bool OnInitDialog() override
    {
        // Print and Close take the places of Add/Mod and Delete.
        moveInto(kPrintId, kAddModId);
        moveInto(IDCANCEL, kDeleteId);
        for (int id : {kAddModId, kDeleteId, kModifyId}) ShowWindow(GetDlgItem(hwnd(), id), SW_HIDE);

        grid_ = fx::Grid(GetDlgItem(hwnd(), kGridId));
        if (!grid_.IsGrid()) return true;
        const int digit = grid_.CharWidth();
        grid_.SetColumns({{L"No.", digit * 5}, {L"Description", digit * 20}});   // the first column is always left-aligned
        int row = 1;
        for (const ListedReport& r : kReports) {
            grid_.SetData(1, row, std::to_wstring(row));
            grid_.SetData(2, row, r.name);
            ++row;
        }
        // Description takes the rest of the list's width, once the rows (and
        // so the vertical scroll bar) are in: no horizontal scroll bar.
        RECT rc{};
        GetClientRect(grid_.list(), &rc);
        grid_.SetColumnWidth(2, rc.right - grid_.ColumnWidth(1));
        grid_.SetActiveCell(1, 1);
        SetFocus(grid_.hwnd());
        return false;   // the focus is set
    }

    bool OnCommand(WORD id, WORD code, HWND) override
    {
        if (id == kPrintId || (id == kGridId && code == GRN_PICK)) {
            const int row = grid_.ActiveRow();
            if (row < 1 || row > static_cast<int>(std::size(kReports))) {
                MessageBeep(MB_OK);
                return true;
            }
            chosen = kReports[row - 1].command;
            End(IDOK);
            return true;
        }
        return false;   // IDCANCEL: closes (fx::Dialog)
    }

private:
    fx::Grid grid_;

    void moveInto(int id, int placeOf)
    {
        HWND from = GetDlgItem(hwnd(), placeOf), to = GetDlgItem(hwnd(), id);
        if (!from || !to) return;
        RECT rc{};
        GetWindowRect(from, &rc);
        MapWindowPoints(nullptr, hwnd(), reinterpret_cast<POINT*>(&rc), 2);
        SetWindowPos(to, nullptr, rc.left, rc.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
    }
};

} // namespace

const ListedReport* ReportListEntries(int& count) noexcept
{
    count = static_cast<int>(std::size(kReports));
    return kReports;
}

WORD ShowReportList(HWND owner)
{
    ReportListDialog dlg;
    return dlg.DoModal(hInst, 1022, owner) == IDOK ? dlg.chosen : 0;
}
