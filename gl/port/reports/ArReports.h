//
// ArReports.h - the Accounts Receivable reports (spec #6 7, D14; plan S6-M6).
// Portable: gl_tests checks them. Accounts come from GL_SETAT and the
// customer records; every report reads AR's own tables (most of the
// original .RPT files were AP reports renamed, still reading AP_JSUM,
// AP_DIST or AP_VEND: spec #6 2.6 #17). "The current period" is the GL's
// (ReportContext). No options, as the originals: straight to the preview.
//
//   4063  Sales Pre-Posting (ARBTCHSM.RPT): the open vouchers, each with its
//         lines and totals.
//   4073  Sales Voucher Detail (AR_BTCHD.RPT): the current period's vouchers,
//         every state, with their lines.
//   4070  Voucher Status (ar_bstat, eiwa): a line per voucher of the current
//         period with its state.
//   4056  Customer List (AR_VEND.RPT, the original's "Customer Detail").
//   4058  Customer Summary (new: the menu item had no handler): a line per
//         customer: invoiced, received, discounts, owed.
//   4057  Open Receivables Detail (AR_DETL.RPT): what's owed (O, R) by
//         customer; also Accounts > Customer Inquiry Report.
//   4060  AR Register (AR_BALA.RPT): every open item with its receipts.
//   4068  Aged Receivables 30-60-90 (AR_AGEDC.RPT): what's owed as at the
//         report's date, by days past due; current and not yet due in the
//         first column. The buckets add to the open total.
//   4061  Collections Forecast (AR_AGEDR.RPT; 4061 printed the aged report
//         again, 4069 this one): what's owed by days to the due date (up to
//         10, overdue included; 11-20; 21-30; over 30), each day once.
//   4076  Receipts Register (new; the original's AR_RECEI.RPT existed
//         nowhere): the current period's receipts: invoice, bank, amount,
//         discount.
//   4071  VAT: Sales (AR_VATAP.RPT): the output VAT of the current period's
//         posted vouchers, less the VAT of its receipts' discounts.
//   4072  VAT: General Ledger (AR_VATGL.RPT): the GL lines of the current
//         period on GL_SETAT VATP's account (the original named 1491000).
//   4074  AR Journal Summary (AR_JSUM.RPT): the journal waiting for the GL,
//         by account.
//   4075  AR Journal Detail (AR_JSUMD.RPT): the current period's journal, by
//         account. (The menu's labels for 4074 and 4075 were swapped.)
//   CM_AR_STATEMENTS  Customer Statements (AR_STAT.RPT, new on the menu):
//         per customer with something owed, the address and the open items,
//         one a page.
//
// RunArReport (RunReport.h) runs them by command.
//
#pragma once

#include "ReportContext.h"

#include "report/Report.h"

#include "cb4sqlite.h"

rpt::Report ArSalesPrePosting(cb4sqlite::Code4& code, const ReportContext& ctx);
rpt::Report ArSalesVoucherDetail(cb4sqlite::Code4& code, const ReportContext& ctx);
rpt::Report ArVoucherStatus(cb4sqlite::Code4& code, const ReportContext& ctx);
rpt::Report ArCustomerList(cb4sqlite::Code4& code, const ReportContext& ctx);
rpt::Report ArCustomerSummary(cb4sqlite::Code4& code, const ReportContext& ctx);
rpt::Report ArOpenReceivables(cb4sqlite::Code4& code, const ReportContext& ctx);
rpt::Report ArRegister(cb4sqlite::Code4& code, const ReportContext& ctx);
rpt::Report ArAgedReceivables(cb4sqlite::Code4& code, const ReportContext& ctx);
rpt::Report ArCollectionsForecast(cb4sqlite::Code4& code, const ReportContext& ctx);
rpt::Report ArReceiptsRegister(cb4sqlite::Code4& code, const ReportContext& ctx);
rpt::Report ArVatSales(cb4sqlite::Code4& code, const ReportContext& ctx);
rpt::Report ArVatGl(cb4sqlite::Code4& code, const ReportContext& ctx);
rpt::Report ArJournalSummary(cb4sqlite::Code4& code, const ReportContext& ctx);
rpt::Report ArJournalDetail(cb4sqlite::Code4& code, const ReportContext& ctx);
rpt::Report ArStatements(cb4sqlite::Code4& code, const ReportContext& ctx);
