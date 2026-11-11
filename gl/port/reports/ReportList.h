//
// ReportList.h - the report list, dialog 1022 (spec #3 D9; plan S3-M7 task 3).
//
// The original's Custom Report (TGLCustomReport, gl_rept.cpp) listed the
// gl_rept table's Crystal reports and ran the chosen .RPT. No Crystal report
// can run in the port, so the same dialog lists the port's own GL reports,
// and Print (or a double-click, or Enter) runs the chosen one as its menu
// item does: its options dialog, if it has one, then its preview.
//
//   - Add/Mod, Delete and Modify are hidden: the list isn't editable, and
//     gl_rept is neither read nor changed.
//   - The original's SetupWindow called cb.closeAll(), closing every open
//     table under the other windows; the port doesn't.
//
// File > Print and the toolbar Print button (3068) open it when no preview is
// active; Reporting > Custom Report and GLSUB's Custom Report
// (CM_REPORT_LIST) always do.
//
#pragma once

#include <Windows.h>

struct ListedReport {
    WORD command;          // the menu command that runs it
    const wchar_t* name;   // as the list shows it
};

// The reports the list offers, in its order: the finance set, then the
// original reports in menu order.
const ListedReport* ReportListEntries(int& count) noexcept;

// Shows dialog 1022 over `owner`. Returns the chosen report's command, or 0
// when closed without choosing.
WORD ShowReportList(HWND owner);
