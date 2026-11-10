//
// ReportContext.cpp - see ReportContext.h.
//
#include "ReportContext.h"

#include "report/Format.h"

#include <utility>

namespace {

bool hasTable(cb4sqlite::Code4& code, const char* name)
{
    cb4sqlite::Query q(code, "SELECT count(*) FROM sqlite_master WHERE type = 'table' AND name = ?1");
    q.bindText(1, name);
    return q.next() && q.integer(0) > 0;
}

} // namespace

const FiscalPeriod* ReportContext::period(int number) const
{
    for (const FiscalPeriod& p : periods)
        if (p.number == number) return &p;
    return nullptr;
}

ReportContext LoadReportContext(cb4sqlite::Code4& code, const std::string& todayYyyymmdd)
{
    ReportContext ctx;
    ctx.printedOn = rpt::FormatDate(todayYyyymmdd);   // Layout adds "Printed on "

    // The first record, as GetCompany's top() reads it: Data4 walks deleted
    // rows too, so neither does this skip them.
    if (hasTable(code, "gl_prof")) {
        cb4sqlite::Query q(code, "SELECT company FROM gl_prof ORDER BY _rowid LIMIT 1");
        if (q.next()) ctx.company = q.text(0);
    }

    if (hasTable(code, "gl_pds")) {
        // Deleted rows count, as for GetCurrentPeriod and OpenPeriods (Data4
        // walks), so the reports' current period is the status bar's.
        cb4sqlite::Query q(code, "SELECT period, pd_desc, periode, status FROM gl_pds ORDER BY period, _rowid");
        while (q.next()) {
            FiscalPeriod p;
            p.number = static_cast<int>(q.integer(0));
            p.description = q.text(1);
            p.endDate = q.text(2);
            p.open = q.text(3) == "0";
            if (p.number < 1 || p.number > 12 || ctx.period(p.number)) continue;   // the first of a duplicate
            ctx.periods.push_back(std::move(p));
        }
    }
    for (const FiscalPeriod& p : ctx.periods) {
        if (p.open) {   // the lowest-numbered open period (D2)
            ctx.currentPeriod = p.number;
            ctx.periodOpen = true;
            break;
        }
    }
    return ctx;
}
