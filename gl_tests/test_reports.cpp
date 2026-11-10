//
// gl_tests: the report context and the sample report (spec #3 4.4, D2;
// plan S3-M1). The fixture's company is "Dev Fixture Company Ltd."; periods
// 1-6 (Apr-Sep 26) are closed, so the current period is 7, "Oct 26", ending
// 2026-10-31. Its eleven accounts' current balances add up to zero.
//

#include "GlTestDb.h"

#include "reports/ReportContext.h"
#include "reports/SampleReport.h"

#include "report/Layout.h"
#include "report/TextRender.h"

#include <string>

using namespace gltest;

namespace {

void setAllPeriodStatus(Code4& code, const char* status)
{
    Data4 pds(&code, "gl_pds");
    REQUIRE(pds.isValid());
    Field4 st(pds, "status");
    for (pds.top(); !pds.eof(); pds.skip()) {
        st.assign(status);
        pds.flush();
    }
    pds.close();
}

int countRows(const rpt::Report& r, rpt::RowKind kind)
{
    int n = 0;
    for (const rpt::Row& row : r.rows)
        if (row.kind == kind) ++n;
    return n;
}

} // namespace

TEST_CASE("LoadReportContext: company, current period and periods (D2)", "[reports]")
{
    FixtureDb db;
    const ReportContext ctx = LoadReportContext(db.code, "20261110");
    CHECK(ctx.company == "Dev Fixture Company Ltd.");
    CHECK(ctx.printedOn == "11/10/2026");   // Layout adds "Printed on "
    CHECK(ctx.periodOpen);
    CHECK(ctx.currentPeriod == 7);
    REQUIRE(ctx.periods.size() == 12);
    for (int i = 0; i < 12; ++i) CHECK(ctx.periods[static_cast<size_t>(i)].number == i + 1);
    const FiscalPeriod* oct = ctx.period(7);
    REQUIRE(oct);
    CHECK(oct->description == "Oct 26");
    CHECK(oct->endDate == "20261031");
    CHECK(oct->open);
    CHECK_FALSE(ctx.period(6)->open);
    CHECK(ctx.period(11)->endDate == "20270228");
    CHECK(ctx.period(13) == nullptr);
}

TEST_CASE("LoadReportContext: no open period gives period 12, not open", "[reports]")
{
    FixtureDb db;
    setAllPeriodStatus(db.code, "1");
    const ReportContext ctx = LoadReportContext(db.code, "20261110");
    CHECK_FALSE(ctx.periodOpen);
    CHECK(ctx.currentPeriod == 12);
    CHECK(ctx.periods.size() == 12);
}

TEST_CASE("LoadReportContext: the lowest open period wins", "[reports]")
{
    FixtureDb db;
    setAllPeriodStatus(db.code, "0");
    const ReportContext ctx = LoadReportContext(db.code, "20261110");
    CHECK(ctx.periodOpen);
    CHECK(ctx.currentPeriod == 1);
}

TEST_CASE("BuildSampleReport: groups by type, subtotals, a final total of zero", "[reports]")
{
    FixtureDb db;
    const ReportContext ctx = LoadReportContext(db.code, "20261110");
    const rpt::Report r = BuildSampleReport(db.code, ctx, {});
    CHECK(r.company == ctx.company);
    CHECK(r.printedOn == ctx.printedOn);
    REQUIRE(r.subtitles.size() == 1);
    CHECK(r.subtitles[0] == "Current period 7, Oct 26, ending October 31, 2026");
    CHECK(r.orientation == rpt::Orientation::Portrait);
    REQUIRE(r.columns.size() == 6);

    CHECK(countRows(r, rpt::RowKind::Detail) == 11);
    CHECK(countRows(r, rpt::RowKind::GroupHeader) == 3);   // types B, I, R
    CHECK(countRows(r, rpt::RowKind::GroupFooter) == 3);
    REQUIRE(countRows(r, rpt::RowKind::Total) == 1);
    for (const rpt::Row& row : r.rows) {
        if (row.kind != rpt::RowKind::Total) continue;
        CHECK(row.final);
        CHECK(row.cells[5].value == 0);   // the current balances add up to zero
    }
    CHECK(r.rows.back().kind == rpt::RowKind::Note);
}

TEST_CASE("BuildSampleReport: options", "[reports]")
{
    FixtureDb db;
    const ReportContext ctx = LoadReportContext(db.code, "20261110");
    SampleOptions opt;
    opt.includeZeros = false;
    opt.landscape = true;
    opt.repeat = 3;
    const rpt::Report r = BuildSampleReport(db.code, ctx, opt);
    CHECK(r.orientation == rpt::Orientation::Landscape);
    // 144910 and the two 410001 departments have a zero balance.
    CHECK(countRows(r, rpt::RowKind::Detail) == 3 * 8);
    CHECK(countRows(r, rpt::RowKind::Heading) == 3);   // "Copy n of 3"

    opt.repeat = 500;   // clamped to 50
    CHECK(countRows(BuildSampleReport(db.code, ctx, opt), rpt::RowKind::Heading) == 50);
}

TEST_CASE("BuildSampleReport: with no open period the subtitle says so", "[reports]")
{
    FixtureDb db;
    setAllPeriodStatus(db.code, "1");
    const rpt::Report r = BuildSampleReport(db.code, LoadReportContext(db.code, "20261110"), {});
    REQUIRE(r.subtitles.size() == 1);
    CHECK(r.subtitles[0] == "No period is open");
}

TEST_CASE("BuildSampleReport lays out: repeated copies make pages", "[reports]")
{
    FixtureDb db;
    const ReportContext ctx = LoadReportContext(db.code, "20261110");
    rpt::FixedMeasure m;
    SampleOptions opt;
    const auto one = rpt::Layout(BuildSampleReport(db.code, ctx, opt), rpt::PageSetup::A4(), m);
    opt.repeat = 10;
    const auto ten = rpt::Layout(BuildSampleReport(db.code, ctx, opt), rpt::PageSetup::A4(), m);
    CHECK(one.size() == 1);
    CHECK(ten.size() > 2);
}
