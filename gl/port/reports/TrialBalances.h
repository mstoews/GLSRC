//
// TrialBalances.h - F1 Trial Balance (6076) and F2 Distribution Trial
// Balance (1075), spec #3 5.2, 5.3, D14. Portable: gl_tests checks them.
//
// F1: each account's balance at the end of a period, in Debit when positive
//     and Credit when negative; the two totals, and a note if they differ.
// F2: per account, the opening balance (end of from - 1), the debits and
//     credits of its gl_dist lines in from..to, and the closing balance
//     (opening + the period movements). A row whose lines don't account for
//     its movement is marked "*" and explained in a note. With voucher lines,
//     each account's gl_dist lines follow it: the original Distribution List.
//
// Both end with the consistency note (Balances.h) when an account fails it.
//
#pragma once

#include "ReportContext.h"

#include "report/Report.h"

struct TrialBalanceOptions {
    int period = 0;              // 0: the context's current period
    bool includeZeros = false;   // accounts whose balance is zero
};

struct DistTrialBalanceOptions {
    int from = 0, to = 0;        // 0: the context's current period
    bool voucherLines = false;   // each account's gl_dist lines beneath it
    bool includeInactive = false;   // accounts with no balance and no activity
};

rpt::Report TrialBalance(cb4sqlite::Code4& code, const ReportContext& ctx, const TrialBalanceOptions& opt);
rpt::Report DistributionTrialBalance(cb4sqlite::Code4& code, const ReportContext& ctx,
                                     const DistTrialBalanceOptions& opt);
