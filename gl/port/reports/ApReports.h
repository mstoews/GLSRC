//
// ApReports.h - the Accounts Payable reports (spec #5 7, D14; plans S5-M6,
// S5-M7). Portable: gl_tests checks them. Accounts come from GL_SETAT, not
// the original's literals; "the current period" is the GL's (ReportContext).
// No options, as the originals: straight to the preview.
//
//   1078  Invoice Pre-Posting (APBTCHSM.RPT): the open invoice vouchers, each
//         with its lines and totals.
//   1081  Cash Pre-Posting (AP_CASHB.RPT): the open cash vouchers, the same.
//   8077  AP Voucher Detail (AP_BTCHD.RPT) and 8078 Cash Voucher Detail
//         (AP_CASHD.RPT): the current period's vouchers, every state, with
//         their lines.
//   1065  Voucher Status (AP_BSTAT.RPT): a line per invoice voucher of the
//         current period: vendor, invoice, date, state, amount.
//   1049  Invoice Posting Journal (AP_POSTJ.RPT): the posted invoice
//         vouchers' expense, VAT and W/H lines (the AP line left out by its
//         role, not the original's hard-coded 2210000).
//   1060  Vendor List (AP_VEND.RPT): in vendor order (the original grouped by
//         address line 1), with accounts, terms and rates.
//   1061  Open Payables Detail (AP_DETL.RPT): the unpaid invoices (H, O, F;
//         the original counted printed and cleared ones as open), by vendor,
//         with what each owes.
//   1062  Aged Payables 30-60-90 (AP_OVERD.RPT; 1063, AP_AGEDC.RPT, the
//         same report, merged): the unpaid invoices aged by due date as at
//         the report's date; the buckets add to the open total.
//   1064  Cash Requirement (AP_AGEDR.RPT): the unpaid invoices by days to
//         their due date: up to 10 (overdue included), 11-20, 21-30, over
//         30 (the original counted day 20 twice).
//   1077  AP Register (AP_BALA.RPT): every open item with its payment.
//   1058  Prepayments (AP_PREPY.RPT): the unpaid invoices with their order
//         numbers.
//   4091  Cheques (AP_CHEQ.RPT, spec #5 6.2): the cheques made and not yet
//         printed, one a page (S5-M6).
//   4092  Payment Detail (AP_REQSD.RPT): per cheque, the invoices it pays
//         with amount, VAT, W/H and net payment.
//   8073  Cash Cheque Summary (AP_CHEQC.RPT): the current period's cash
//         vouchers paid by cheque, the amount less W/H.
//   8067  VAT: AP (AP_VATAP.RPT): the VAT lines of the current period's
//         invoice and cash vouchers.
//   8072  VAT: GL (AP_VATGL.RPT): the General Ledger voucher lines of the
//         current period on the VAT account (GL_SETAT VATR; the original
//         named 148000).
//   8071  Withholding Tax (AP_REB.RPT): the W/H lines of the current period's
//         invoice and cash vouchers, by W/H account (its selection was
//         commented out, so it printed every GL line).
//   8068  AP Journal Summary (AP_JSUM.RPT): the journal waiting for the GL,
//         by account.
//   8069  AP Journal Detail (AP_JSUMD.RPT): the current period's journal, by
//         account.
//   8075  AP Journal: Cash (AP_JSUMC.RPT) and 8076 AP Journal: Invoices and
//         Payments (AP_JSUMA.RPT): the current period's rows of each type.
//
// The menu labels that named the wrong report (4091 twice, 4092 "Customer
// Statement Detail", 1060 twice, 8069 "Transfer Summary") are corrected by
// prune_rc.py; 8068, 1049 and 1065, on no menu, join Reporting > Accounts
// Payable. RunApReport (RunReport.h) runs them by command.
//
#pragma once

#include "ReportContext.h"

#include "report/Report.h"

#include "cb4sqlite.h"

rpt::Report ApInvoicePrePosting(cb4sqlite::Code4& code, const ReportContext& ctx);
rpt::Report ApCashPrePosting(cb4sqlite::Code4& code, const ReportContext& ctx);
rpt::Report ApInvoiceVoucherDetail(cb4sqlite::Code4& code, const ReportContext& ctx);
rpt::Report ApCashVoucherDetail(cb4sqlite::Code4& code, const ReportContext& ctx);
rpt::Report ApVoucherStatus(cb4sqlite::Code4& code, const ReportContext& ctx);
rpt::Report ApPostingJournal(cb4sqlite::Code4& code, const ReportContext& ctx);
rpt::Report ApVendorList(cb4sqlite::Code4& code, const ReportContext& ctx);
rpt::Report ApOpenPayables(cb4sqlite::Code4& code, const ReportContext& ctx);
rpt::Report ApAgedPayables(cb4sqlite::Code4& code, const ReportContext& ctx);
rpt::Report ApCashRequirement(cb4sqlite::Code4& code, const ReportContext& ctx);
rpt::Report ApRegister(cb4sqlite::Code4& code, const ReportContext& ctx);
rpt::Report ApPrepayments(cb4sqlite::Code4& code, const ReportContext& ctx);
rpt::Report ApCheques(cb4sqlite::Code4& code, const ReportContext& ctx);
rpt::Report ApPaymentDetail(cb4sqlite::Code4& code, const ReportContext& ctx);
rpt::Report ApCashChequeSummary(cb4sqlite::Code4& code, const ReportContext& ctx);
rpt::Report ApVatAp(cb4sqlite::Code4& code, const ReportContext& ctx);
rpt::Report ApVatGl(cb4sqlite::Code4& code, const ReportContext& ctx);
rpt::Report ApWithholding(cb4sqlite::Code4& code, const ReportContext& ctx);
rpt::Report ApJournalSummary(cb4sqlite::Code4& code, const ReportContext& ctx);
rpt::Report ApJournalDetail(cb4sqlite::Code4& code, const ReportContext& ctx);
rpt::Report ApJournalCash(cb4sqlite::Code4& code, const ReportContext& ctx);
rpt::Report ApJournalInvoices(cb4sqlite::Code4& code, const ReportContext& ctx);

// The report's "as at" date for the aged reports: ReportContext::printedOn
// ("11/13/2026") as yyyymmdd; "" if it isn't a date.
std::string ApAsAtDate(const ReportContext& ctx);
