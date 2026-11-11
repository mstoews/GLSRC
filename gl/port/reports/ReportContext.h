//
// ReportContext.h - what every report knows about the company (spec #3 4.4,
// D2; plan S3-M1 task 5). Portable: gl_tests checks it on the fixture.
//
// The original passed two values to every Crystal report (ap_repts.cpp
// PrintReport): the company name, from the first gl_prof record, and the
// current period, the lowest-numbered open period in gl_pds. The port keeps
// both (D2): with no open period the current period is 12 and
// `periodOpen` is false, so the reports that print a current period can say
// there is none. It adds the fiscal periods themselves, for the options
// dialogs' period lists and the statements' "For the period ended ..." lines.
//
#pragma once

#include "cb4sqlite.h"

#include <string>
#include <vector>

struct FiscalPeriod {
    int number = 0;            // 1-12
    std::string description;   // pd_desc, e.g. "Oct 26"
    std::string endDate;       // periode, yyyymmdd ("" if blank)
    bool open = false;         // status "0"
};

struct ReportContext {
    std::string company;              // gl_prof.company of the first record, trimmed
    int currentPeriod = 12;           // D2
    bool periodOpen = false;          // false: no period is open
    std::vector<FiscalPeriod> periods;   // gl_pds in period order
    std::string printedOn;            // today, "11/10/2026": rpt::Report::printedOn

    // The period with this number, or null.
    const FiscalPeriod* period(int number) const;
};

// Reads gl_prof and gl_pds. `todayYyyymmdd` gives printedOn (the caller
// passes the date, so tests can fix it). Missing tables give an empty company
// and no periods, not an error.
ReportContext LoadReportContext(cb4sqlite::Code4& code, const std::string& todayYyyymmdd);

// "Current period 7 (Oct 26)": the subtitle of the reports that print one.
std::string CurrentPeriodLine(const ReportContext& ctx);
