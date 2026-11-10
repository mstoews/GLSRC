//
// Statements.h - F3 Income Statement (1015), F4 Balance Sheet (1016) and
// F5 Cash Flow Statement (1018), spec #3 5.4-5.6, 6.2, D13. Portable:
// gl_tests checks them.
//
// Each evaluates its layout (StatementLayout.h) column by column, in cents:
//
//   F3  Month (period p) and Year to date (1..p): each account line is the
//       accounts' movement x sign; optionally Prior year to date
//       (PREVIOUS1..p) and Budget to date (BUDGET1..p) with Variance (year to
//       date less budget). One department, or all.
//   F4  End of period p and Start of year: balances x sign.
//   F5  Month and Year to date: minus the change in balance x sign, so an
//       increase in an asset uses cash; opening and closing cash from the
//       O / E lines.
//
// Net income (kind N) is minus the income accounts' movement over the
// column's periods (F4: 1..p at the end, nothing at the start of the year).
// A total (T) adds and subtracts earlier lines' printed amounts. Accounts the
// statement covers but no account line holds are printed as "Unclassified
// accounts" just before the statement's last total, which includes them, so
// the totals stay complete. F4 checks that the ledger balances, F5 that
// opening cash plus the net change is closing cash ("Unreconciled
// difference"); layout warnings and the balances' consistency note end the
// report as notes.
//
#pragma once

#include "ReportContext.h"

#include "report/Report.h"

#include <string>

struct StatementOptions {
    int period = 0;              // 0: the current period
    std::string dept;            // F3: "" for all departments
    bool comparePrior = false;   // F3
    bool compareBudget = false;  // F3
    bool showAccounts = false;   // each account line's accounts beneath it
    bool showZeroLines = false;  // account lines that are zero in every column
};

rpt::Report IncomeStatement(cb4sqlite::Code4& code, const ReportContext& ctx, const StatementOptions& opt);
rpt::Report BalanceSheet(cb4sqlite::Code4& code, const ReportContext& ctx, const StatementOptions& opt);
rpt::Report CashFlowStatement(cb4sqlite::Code4& code, const ReportContext& ctx, const StatementOptions& opt);
