//
// gl_tests: the original account reports 6080, 10071, 10073 (spec #3 2.2,
// D2, D3, D6; plan S3-M5) on the dev fixture, current period 7. Order and
// totals are checked against SQL sums over glaccts; a few rows by hand:
//
//   102101  opening 12,500, period 7 +250, curr 12,750: 6080's YTD
//           (curr - open - period) 0; accumulated this year 250.
//   102201  opening 20,000, period 7 +700, curr 24,950: YTD 4,250.
//   411000  current year -16,000, period 7 -6,000, budget 7 -10,000:
//           difference 4,000.
//

#include "GlTestDb.h"

#include "reports/AccountReports.h"
#include "reports/ReportContext.h"

#include <algorithm>
#include <string>

using namespace gltest;

namespace {

ReportContext context(Code4& code) { return LoadReportContext(code, "20261110"); }

std::vector<const rpt::Row*> details(const rpt::Report& r)
{
    std::vector<const rpt::Row*> out;
    for (const rpt::Row& x : r.rows)
        if (x.kind == rpt::RowKind::Detail) out.push_back(&x);
    return out;
}

const rpt::Row* account(const rpt::Report& r, const std::string& a, const std::string& dept = "")
{
    for (const rpt::Row* x : details(r))
        if (x->cells[0].text == a && x->cells[1].text == dept) return x;
    return nullptr;
}

const rpt::Row& total(const rpt::Report& r)
{
    const auto it = std::find_if(r.rows.begin(), r.rows.end(), [](const rpt::Row& x) { return x.kind == rpt::RowKind::Total; });
    REQUIRE(it != r.rows.end());
    return *it;
}

std::int64_t sqlCents(Code4& code, const std::string& expr)
{
    cb4sqlite::Query q(code, "SELECT round(sum(" + expr + ") * 100) FROM glaccts WHERE _deleted = 0");
    REQUIRE(q.next());
    return q.integer(0);
}

bool hasNote(const rpt::Report& r, const std::string& text)
{
    return std::any_of(r.rows.begin(), r.rows.end(), [&](const rpt::Row& x) {
        return x.kind == rpt::RowKind::Note && x.text.find(text) != std::string::npos;
    });
}

void closeAllPeriods(Code4& code)
{
    Data4 pds(&code, "gl_pds");
    Field4 st(pds, "status");
    for (pds.top(); !pds.eof(); pds.skip()) {
        st.assign("1");
        pds.flush();
    }
    pds.close();
}

const std::string kPeriods = "coalesce(period1,0)+coalesce(period2,0)+coalesce(period3,0)+coalesce(period4,0)+"
                             "coalesce(period5,0)+coalesce(period6,0)+coalesce(period7,0)+coalesce(period8,0)+"
                             "coalesce(period9,0)+coalesce(period10,0)+coalesce(period11,0)+coalesce(period12,0)";

} // namespace

TEST_CASE("6080 Trial Balance by Department", "[reports][accounts]")
{
    FixtureDb db;
    const rpt::Report r = TrialBalanceByDept(db.code, context(db.code));
    CHECK(r.title == "Trial Balance by Department");
    CHECK(r.subtitles[0] == "Current period 7 (Oct 26)");
    CHECK(r.footerNote == "Report Name : gldept");
    CHECK(r.orientation == rpt::Orientation::Landscape);
    const auto rows = details(r);
    REQUIRE(rows.size() == 25);   // every account, zero or not
    CHECK(rows.front()->cells[0].text == "101000");
    CHECK(rows.back()->cells[0].text == "999000");
    for (size_t i = 1; i < rows.size(); ++i)   // account, then dept
        CHECK(rows[i - 1]->cells[0].text + rows[i - 1]->cells[1].text <= rows[i]->cells[0].text + rows[i]->cells[1].text);

    const rpt::Row* checking = account(r, "102101");
    REQUIRE(checking);
    CHECK(checking->cells[3].value == 1250000);
    CHECK(checking->cells[4].value == 25000);
    CHECK(checking->cells[5].value == 0);   // YTD as the original: curr - open - period
    CHECK(checking->cells[6].value == 1275000);
    CHECK(account(r, "102201")->cells[5].value == 425000);
    CHECK(account(r, "731000", "20")->cells[4].value == 0);

    const rpt::Row& t = total(r);
    CHECK(t.final);
    CHECK(t.cells[3].value == sqlCents(db.code, "openbal"));
    CHECK(t.cells[4].value == sqlCents(db.code, "coalesce(period7,0)"));
    CHECK(t.cells[5].value == sqlCents(db.code, "curr_bal - openbal - coalesce(period7,0)"));
    CHECK(t.cells[6].value == sqlCents(db.code, "curr_bal"));
    CHECK_FALSE(hasNote(r, "No period is open"));
}

TEST_CASE("10071 Chart of Accounts Inquiry", "[reports][accounts]")
{
    FixtureDb db;
    const rpt::Report r = ChartOfAccountsReport(db.code, context(db.code));
    CHECK(r.title == "Chart of Accounts Inquiry");
    CHECK(r.footerNote == "gl_chart.rpt");
    CHECK(r.orientation == rpt::Orientation::Portrait);
    CHECK(details(r).size() == 25);
    CHECK(account(r, "102101")->cells[4].value == 25000);
    CHECK(account(r, "102201")->cells[4].value == 495000);
    CHECK(account(r, "161000")->cells[4].value == -60000);   // the undistributed depreciation counts
    const rpt::Row& t = total(r);
    CHECK(t.cells[3].value == sqlCents(db.code, "openbal"));
    CHECK(t.cells[4].value == sqlCents(db.code, kPeriods));
    CHECK(t.cells[5].value == sqlCents(db.code, "curr_bal"));
}

TEST_CASE("10073 Budget Inquiry", "[reports][accounts]")
{
    FixtureDb db;
    const rpt::Report r = BudgetReport(db.code, context(db.code));
    CHECK(r.title == "Budget Inquiry");
    CHECK(r.footerNote == "gl_budgt.rpt");
    const rpt::Row* sales = account(r, "411000");
    REQUIRE(sales);
    CHECK(sales->cells[3].value == -1600000);
    CHECK(sales->cells[4].value == -600000);
    CHECK(sales->cells[5].value == -1000000);
    CHECK(sales->cells[6].value == 400000);
    const rpt::Row& t = total(r);
    CHECK(t.cells[0].text == "Chart of Accounts Total");
    CHECK(t.cells[3].value == sqlCents(db.code, kPeriods));
    CHECK(t.cells[4].value == sqlCents(db.code, "coalesce(period7,0)"));
    CHECK(t.cells[5].value == sqlCents(db.code, "coalesce(budget7,0)"));
    CHECK(t.cells[6].value == sqlCents(db.code, "coalesce(period7,0) - coalesce(budget7,0)"));
}

TEST_CASE("6080 and 10073 with no open period: period 12, and a note (D2)", "[reports][accounts]")
{
    FixtureDb db;
    closeAllPeriods(db.code);
    const ReportContext ctx = context(db.code);
    const rpt::Report tb = TrialBalanceByDept(db.code, ctx);
    CHECK(tb.subtitles[0] == "Current period 12 (Mar 27)");
    CHECK(hasNote(tb, "No period is open"));
    CHECK(total(tb).cells[4].value == sqlCents(db.code, "coalesce(period12,0)"));
    CHECK(hasNote(BudgetReport(db.code, ctx), "No period is open"));
    CHECK_FALSE(hasNote(ChartOfAccountsReport(db.code, ctx), "No period is open"));   // it prints no period
}
