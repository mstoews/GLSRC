//
// gl_tests: the fixed-asset reports (spec #4 7, D11; plan S4-M5) on the dev
// fixture's register:
//
//   EQ  E001 30,000.00 (500.00 a month, 6,500.00), E002 5,000.00 (100.00,
//       100.00), E004 disposed 2026-08-15 (1,000.00, fully depreciated)
//   XX  E005 2,000.00 (83.33 a month, nothing yet); its accounts are 999999,
//       which isn't in the chart
//
//   In use: 37,000.00 cost, 683.33 a month, 6,600.00 accumulated, 30,400.00
//   book value. The ledger at period 7: 151000 35,000.00 Dr, 161000
//   6,600.00 Cr, so EQ's accounts agree.
//
#include "GlTestDb.h"

#include "Assets.h"
#include "reports/AssetReports.h"
#include "reports/ReportContext.h"

#include <algorithm>
#include <string>
#include <vector>

using namespace gltest;

namespace {

ReportContext context(Code4& code) { return LoadReportContext(code, "20261111"); }

std::vector<const rpt::Row*> rowsOf(const rpt::Report& r, rpt::RowKind kind)
{
    std::vector<const rpt::Row*> out;
    for (const rpt::Row& x : r.rows)
        if (x.kind == kind) out.push_back(&x);
    return out;
}

// The note containing `text`, or "<none>".
std::string note(const rpt::Report& r, const std::string& text)
{
    for (const rpt::Row* x : rowsOf(r, rpt::RowKind::Note))
        if (x->text.find(text) != std::string::npos) return x->text;
    return "<none>";
}

} // namespace

TEST_CASE("1019 Fixed Asset Summary: the register by type, totals of the assets in use", "[reports][assets]")
{
    FixtureDb db;
    const rpt::Report r = FixedAssetListing(db.code, context(db.code));
    CHECK(r.title == "General Ledger Fixed Asset Summary");
    CHECK(r.footerNote == "Report File: gl_fa.rpt");
    CHECK(r.orientation == rpt::Orientation::Landscape);
    CHECK(r.subtitles == std::vector<std::string>{"Current period 7 (Oct 26)"});
    REQUIRE(r.columns.size() == 11);

    const auto groups = rowsOf(r, rpt::RowKind::GroupHeader);
    REQUIRE(groups.size() == 2);
    CHECK(groups[0]->text == "EQ  Office equipment");
    CHECK(groups[1]->text == "XX  Bad accounts (test)");

    const auto details = rowsOf(r, rpt::RowKind::Detail);
    REQUIRE(details.size() == 8);   // 4 assets, each with its reference under it
    CHECK(details[0]->cells[0].text == "E001");
    CHECK(details[0]->cells[1].text == "20250401");
    CHECK(details[0]->cells[4].value == 13);
    CHECK(details[0]->cells[6].value == 3000000);
    CHECK(details[0]->cells[8].value == 50000);
    CHECK(details[0]->cells[9].value == 650000);
    CHECK(details[0]->cells[10].value == 2350000);
    CHECK(details[1]->cells[3].text == "Head office");
    CHECK(details[4]->cells[0].text == "E004");
    CHECK(details[4]->cells[2].text == "20260815");   // date sold
    CHECK(details[4]->cells[3].text == "Old printer (disposed)");

    const auto footers = rowsOf(r, rpt::RowKind::GroupFooter);
    REQUIRE(footers.size() == 2);
    CHECK(footers[0]->cells[0].text == "Total EQ");
    CHECK(footers[0]->cells[6].value == 3500000);   // E004 isn't totalled
    CHECK(footers[0]->cells[8].value == 60000);
    CHECK(footers[0]->cells[9].value == 660000);
    CHECK(footers[0]->cells[10].value == 2840000);
    CHECK(footers[1]->cells[6].value == 200000);

    const auto totals = rowsOf(r, rpt::RowKind::Total);
    REQUIRE(totals.size() == 1);
    CHECK(totals[0]->cells[0].text == "3 assets");
    CHECK(totals[0]->cells[6].value == 3700000);
    CHECK(totals[0]->cells[8].value == 68333);
    CHECK(totals[0]->cells[9].value == 660000);
    CHECK(totals[0]->cells[10].value == 3040000);
    CHECK(note(r, "disposed asset") == "1 disposed asset is listed but not totalled.");
}

TEST_CASE("1019's check against the ledger (D11)", "[reports][assets]")
{
    FixtureDb db;
    const rpt::Report r = FixedAssetListing(db.code, context(db.code));
    CHECK(note(r, "151000") == "151000 OFFICE EQUIPMENT (cost): register 35,000.00, ledger 35,000.00. Agrees.");
    CHECK(note(r, "161000") ==
          "161000 ACCUM. DEPRECIATION - OFFICE EQUIPMENT (accumulated depreciation): register 6,600.00, ledger "
          "6,600.00. Agrees.");
    CHECK(note(r, "999999 (cost") == "999999 (cost, type XX): not in the chart of accounts. Register 2,000.00.");
    CHECK(note(r, "999999 (accumulated") != "<none>");

    // A run not yet posted: the register is ahead of the ledger.
    REQUIRE(RunDepreciation(db.code, 7, 0).status == RunStatus::Created);
    const rpt::Report after = FixedAssetListing(db.code, context(db.code));
    CHECK(note(after, "161000") ==
          "161000 ACCUM. DEPRECIATION - OFFICE EQUIPMENT (accumulated depreciation): register 7,200.00, ledger "
          "6,600.00. Differs by 600.00.");
    CHECK(note(after, "151000").find("Agrees") != std::string::npos);
    CHECK(note(after, "A difference can be") != "<none>");
}

TEST_CASE("1020 Fixed Asset Types: each type's three accounts", "[reports][assets]")
{
    FixtureDb db;
    const rpt::Report r = FixedAssetTypesListing(db.code, context(db.code));
    CHECK(r.title == "General Ledger Fixed Asset Types");   // the .RPT said "Leger"
    CHECK(r.footerNote == "Report Name: gl_fatyp.rpt");
    CHECK(r.subtitles.empty());
    const auto groups = rowsOf(r, rpt::RowKind::GroupHeader);
    REQUIRE(groups.size() == 2);
    CHECK(groups[0]->text == "EQ  Office equipment");
    const auto details = rowsOf(r, rpt::RowKind::Detail);
    REQUIRE(details.size() == 6);
    CHECK(details[0]->cells[1].text == "Asset");
    CHECK(details[0]->cells[2].text == "151000");
    CHECK(details[1]->cells[2].text == "761000");
    CHECK(details[2]->cells[1].text == "Accumulated Depreciation");
    CHECK(details[2]->cells[2].text == "161000");
    CHECK(details[2]->cells[4].text == "ACCUM. DEPRECIATION - OFFICE EQUIPMENT");
    CHECK(r.rows.back().text == "2 types");
}
