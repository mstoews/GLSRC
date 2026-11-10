//
// AccountReports.cpp - see AccountReports.h.
//
#include "AccountReports.h"

#include "Balances.h"

#include <algorithm>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace {

using Values = std::vector<std::int64_t>;

// "Current period 7 (Oct 26)".
std::string currentPeriodLine(const ReportContext& ctx)
{
    std::string s = "Current period " + std::to_string(ctx.currentPeriod);
    const FiscalPeriod* fp = ctx.period(ctx.currentPeriod);
    if (fp && !fp->description.empty()) s += " (" + fp->description + ")";
    return s;
}

// The common frame: header, account / dept / description, then `amounts`
// columns filled by `values`, a total row, and the closing notes.
rpt::Report listing(cb4sqlite::Code4& code, const ReportContext& ctx, const std::string& title,
                    const std::string& footer, const std::vector<rpt::Column>& amounts,
                    const std::function<Values(const AccountBalances&)>& values, const std::string& totalLabel,
                    bool printsPeriod)
{
    rpt::Report r;
    r.company = ctx.company;
    r.printedOn = ctx.printedOn;
    r.title = title;
    r.subtitles.push_back(currentPeriodLine(ctx));
    r.footerNote = footer;
    r.columns = {{"Account", 1.0}, {"Dept", 0.5}, {"Description", 3.2}};
    for (const rpt::Column& c : amounts) r.columns.push_back(c);
    if (amounts.size() > 3) r.orientation = rpt::Orientation::Landscape;

    const std::vector<AccountBalances> accounts = LoadBalances(code);
    Values total(amounts.size(), 0);
    for (const AccountBalances& a : accounts) {
        const Values v = values(a);
        std::vector<rpt::Cell> cells{rpt::Cell::Text(a.account), rpt::Cell::Text(a.dept), rpt::Cell::Text(a.descript)};
        for (size_t i = 0; i < v.size(); ++i) {
            cells.push_back(rpt::Cell::Amount(v[i]));
            total[i] += v[i];
        }
        r.rows.push_back(rpt::Row::Detail(std::move(cells)));
    }
    std::vector<rpt::Cell> t{rpt::Cell::Text(totalLabel), {}, {}};
    for (std::int64_t x : total) t.push_back(rpt::Cell::Amount(x));
    r.rows.push_back(rpt::Row::Total(std::move(t), true));

    std::vector<std::string> notes;
    if (printsPeriod && !ctx.periodOpen)
        notes.push_back("No period is open: the current period is taken as period 12.");
    if (const std::string c = ConsistencyNote(accounts); !c.empty()) notes.push_back(c);
    if (!notes.empty()) r.rows.push_back(rpt::Row::Blank());
    for (const std::string& n : notes) r.rows.push_back(rpt::Row::Note(n));
    return r;
}

rpt::Column amount(const char* header, const char* header2 = "")
{
    return {header, 1.3, rpt::CellType::Amount, header2};
}

} // namespace

rpt::Report TrialBalanceByDept(cb4sqlite::Code4& code, const ReportContext& ctx)
{
    const int p = std::clamp(ctx.currentPeriod, 1, 12);
    return listing(
        code, ctx, "Trial Balance by Department", "Report Name : gldept",
        {amount("Opening", "Balance"), amount("Current", "Period"), amount("YTD"), amount("Current", "Balance")},
        [p](const AccountBalances& a) {
            const std::int64_t periodAmount = a.movement(p, p);
            return Values{a.openbal, periodAmount, a.currBal - a.openbal - periodAmount, a.currBal};
        },
        "Total", true);
}

rpt::Report ChartOfAccountsReport(cb4sqlite::Code4& code, const ReportContext& ctx)
{
    return listing(
        code, ctx, "Chart of Accounts Inquiry", "gl_chart.rpt",
        {amount("Opening", "Balance"), amount("Accumulated", "This Year"), amount("Current", "Balance")},
        [](const AccountBalances& a) { return Values{a.openbal, a.movement(1, 12), a.currBal}; }, "Total", false);
}

rpt::Report BudgetReport(cb4sqlite::Code4& code, const ReportContext& ctx)
{
    const int p = std::clamp(ctx.currentPeriod, 1, 12);
    return listing(
        code, ctx, "Budget Inquiry", "gl_budgt.rpt",
        {amount("Current", "Year"), amount("Current", "Period"), amount("Budget", "Amount"), amount("Difference")},
        [p](const AccountBalances& a) {
            const std::int64_t month = a.movement(p, p), budget = a.budgetMovement(p, p);
            return Values{a.movement(1, 12), month, budget, month - budget};
        },
        "Chart of Accounts Total", true);
}
