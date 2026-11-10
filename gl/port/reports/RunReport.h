//
// RunReport.h - running a report: the port's PrintReport (spec #3 2.1, 4;
// plan S3-M1 task 5).
//
// The original's PrintReport(name, hwnd) (gl/src/ap_repts.cpp:112) opened a
// Crystal job, set the company name and current period, and showed Crystal's
// preview. RunReport does the same steps with the native engine:
//
//   1. the report's options dialog, if it has one (Cancel stops here);
//   2. a wait cursor; the ReportContext (company, current period, periods);
//   3. the builder, whose exceptions are shown as the other screens show them;
//   4. a preview (report/win32/Preview.h), whose Options... repeats 1-3.
//
// A builder that leaves the report's company or "Printed on" line empty gets
// the context's.
//
#pragma once

#include "ReportContext.h"

#include "report/Report.h"

#include "win32fx.h"

#include <functional>

using ReportOptionsFn = std::function<bool(HWND owner)>;   // false: cancelled; null: no options
using ReportBuilder = std::function<rpt::Report(const ReportContext&)>;

void RunReport(fx::MdiFrame& frame, ReportOptionsFn options, ReportBuilder build);

// The context for the open company database, printed today.
ReportContext CurrentReportContext();

// The sample report (SampleReport.h): CM_SAMPLE_REPORT, gl.exe /samplereport.
void RunSampleReport(fx::MdiFrame& frame);

// F1 Trial Balance (6076) and F2 Distribution Trial Balance (1075), spec #3
// 5.2-5.3: their options dialog, then the preview. The options are kept for
// the session.
void RunTrialBalance(fx::MdiFrame& frame);
void RunDistributionTrialBalance(fx::MdiFrame& frame);

// F3 Income Statement (1015), F4 Balance Sheet (1016), F5 Cash Flow
// Statement (1018), spec #3 5.4, D13: options, then the preview; the options
// are kept for the session, per statement. The layout tables are created on
// the first run (StatementLayout.h).
void RunIncomeStatement(fx::MdiFrame& frame);
void RunBalanceSheet(fx::MdiFrame& frame);
void RunCashFlowStatement(fx::MdiFrame& frame);

// /stress: opens each report in a preview `times` times (the sample report
// with three copies, F1, F2 with voucher lines, F3 with every column, F4,
// F5), pages through it at every zoom, and closes it. Returns the GDI + USER
// object growth, or -1 if a preview couldn't be opened.
int StressReports(int times);
