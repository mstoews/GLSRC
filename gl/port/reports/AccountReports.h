//
// AccountReports.h - the original account reports (spec #3 2.2, D2, D3, D6;
// plan S3-M5). Portable: gl_tests checks them.
//
// Each lists every account (not deleted) in account + dept order, at the
// current period (D2), with column totals, as its Crystal report did; the
// footer names the .RPT the original ran. No original report took options,
// so these have none.
//
//   6080  Trial Balance by Department (GLDEPT.RPT): opening balance, the
//         current period's amount, "YTD" and the current balance. YTD is the
//         original's formula, CURR_BAL - OPENBAL - PERIOD<p>, kept as written
//         (D3): it is the year to date *before* the current period.
//   10071 Chart of Accounts Inquiry (GL_CHART.RPT): opening balance,
//         "Accumulated This Year" (PERIOD1-12) and the current balance.
//   10073 Budget Inquiry (GL_BUDGT.RPT, D6): "Current Year" (PERIOD1-12),
//         the current period's amount, its budget (BUDGET<p>) and the
//         difference, amount less budget.
//
// With no open period the current period is 12 (D2), and 6080 and 10073,
// which print one, say so in a note.
//
#pragma once

#include "ReportContext.h"

#include "report/Report.h"

rpt::Report TrialBalanceByDept(cb4sqlite::Code4& code, const ReportContext& ctx);
rpt::Report ChartOfAccountsReport(cb4sqlite::Code4& code, const ReportContext& ctx);
rpt::Report BudgetReport(cb4sqlite::Code4& code, const ReportContext& ctx);
