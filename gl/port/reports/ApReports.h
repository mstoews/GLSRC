//
// ApReports.h - the Accounts Payable reports (spec #5 7, D14). Portable:
// gl_tests checks them. S5-M6 adds the cheques; S5-M7 the rest.
//
//   4091  Cheques (AP_CHEQ.RPT, spec #5 6.2): the cheques made and not yet
//         printed, one a page: the cheque (number, date, payee and address,
//         the amount in figures and words, the bank), then the stub, the
//         invoices it pays. On plain paper (Q6 assumed): the original's
//         layout was for pre-printed stock on an Epson EX-1000.
//
#pragma once

#include "ReportContext.h"

#include "report/Report.h"

#include "cb4sqlite.h"

rpt::Report ApCheques(cb4sqlite::Code4& code, const ReportContext& ctx);
