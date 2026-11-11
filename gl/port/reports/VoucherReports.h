//
// VoucherReports.h - the original voucher reports (spec #3 2.2, D4, D5, D7,
// D8, D17; plan S3-M6). Portable: gl_tests checks them.
//
// They read gl_btch (headers) and gl_btchd (lines), skipping deleted rows,
// in voucher-number order, and list every voucher whatever its mode or
// period (D17), except 6078 and 8136, which take only the unposted ones
// (modes "1" open and "3" adjustment). Every line shows its period.
//
//   6074  Voucher Slips (GLSLIP.RPT): one slip per voucher: its number, date,
//         description, period, cheque and status, then its lines and
//         "Total Batch Entry".
//   8082  Voucher Detail (GLBTCHD.RPT): every line grouped by voucher, with
//         date, doctor, med rep and product; per voucher the debit and
//         credit totals and the Difference, debits less credits, printed
//         only when not zero (D8: the original hid a credit excess).
//   6075  Voucher Summary (GL_BTCHS.RPT): one line per voucher: period,
//         number, cheque, date, description and its lines' totals.
//   6078  Preposting Distribution List (GLPRE.RPT, D4): per account + dept
//         with unposted lines: the current balance, those lines, their
//         count and totals, and the balance after posting them.
//   8136  Pre-Posting Report (D5): the unposted vouchers in 8082's layout.
//   (1013) Voucher Entry's Print (GL_SLIPS.RPT, D7): one voucher's slip.
//
#pragma once

#include "ReportContext.h"

#include "report/Report.h"

rpt::Report VoucherSlips(cb4sqlite::Code4& code, const ReportContext& ctx);
rpt::Report VoucherDetail(cb4sqlite::Code4& code, const ReportContext& ctx);
rpt::Report VoucherSummary(cb4sqlite::Code4& code, const ReportContext& ctx);
rpt::Report PrepostingDistribution(cb4sqlite::Code4& code, const ReportContext& ctx);
rpt::Report PrePostingReport(cb4sqlite::Code4& code, const ReportContext& ctx);

// One voucher's slip (D7). A voucher that doesn't exist gives a report with
// a note saying so.
rpt::Report VoucherSlip(cb4sqlite::Code4& code, const ReportContext& ctx, long voucher);

// "Open", "Adjustment", "Posted", "Posted adjustment", "Closed year", or the
// mode itself.
std::string VoucherStatus(const std::string& mode);
