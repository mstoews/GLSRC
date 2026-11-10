//
// SampleReport.h - the report engine's test report (plan S3-M1 exit:
// "a test report previews, pages, and prints"). Portable.
//
// The chart of accounts from glaccts, grouped by account type, with a
// subtotal per type and a final total, the current period from the context,
// and a closing note that wraps. Options repeat the list to make pages, drop
// zero balances, or turn the page to landscape. gl.exe /samplereport opens it
// (gl/README.md, Checks); /stress opens and closes it. It isn't on a menu,
// and isn't one of the original reports.
//
#pragma once

#include "ReportContext.h"

#include "report/Report.h"

struct SampleOptions {
    bool includeZeros = true;
    bool landscape = false;
    int repeat = 1;   // 1-50
};

rpt::Report BuildSampleReport(cb4sqlite::Code4& code, const ReportContext& ctx, const SampleOptions& opt);
