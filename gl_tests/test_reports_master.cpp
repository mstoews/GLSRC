//
// gl_tests: the master-data listings (spec #3 2.2, D12; plan S3-M7) on the
// dev fixture's three medical reps (MR001-MR003), three doctors (DR001-DR003)
// and three products (P100, P200, P300). Each party has address lines 1 and 3
// ("1 Clinic Road", "Vancouver BC"), terms 30 and no credit limit.
//
#include "GlTestDb.h"

#include "reports/MasterReports.h"
#include "reports/ReportContext.h"

#include <algorithm>
#include <string>
#include <vector>

using namespace gltest;

namespace {

ReportContext context(Code4& code) { return LoadReportContext(code, "20261111"); }

std::vector<const rpt::Row*> details(const rpt::Report& r)
{
    std::vector<const rpt::Row*> out;
    for (const rpt::Row& x : r.rows)
        if (x.kind == rpt::RowKind::Detail) out.push_back(&x);
    return out;
}

std::string text(const rpt::Row& row, size_t col) { return col < row.cells.size() ? row.cells[col].text : "<none>"; }

const rpt::Row& lastRow(const rpt::Report& r)
{
    REQUIRE(!r.rows.empty());
    return r.rows.back();
}

// Marks the record whose `keyField` is `key` deleted, without packing.
void deleteRecord(Code4& code, const char* table, const char* keyField, const char* key)
{
    Data4 d(&code, table);
    Field4 f(d, keyField);
    for (d.top(); !d.eof(); d.skip())
        if (std::string(f.str()).rfind(key, 0) == 0) {
            d.deleteRec();
            d.flush();
            break;
        }
    d.close();
}

} // namespace

TEST_CASE("6039 Medical Representatives: one party per rep, address lines under the first", "[reports][master]")
{
    FixtureDb db;
    const rpt::Report r = MedicalRepsListing(db.code, context(db.code));
    CHECK(r.title == "Medical Representative Summary Report");
    CHECK(r.footerNote == "Report File: gl_medrp.rpt");
    CHECK(r.orientation == rpt::Orientation::Landscape);
    CHECK(r.subtitles == std::vector<std::string>{"Current period 7 (Oct 26)"});
    REQUIRE(r.columns.size() == 12);

    const auto rows = details(r);
    REQUIRE(rows.size() == 6);   // three reps, each with address line 3 on a line of its own
    CHECK(text(*rows[0], 0) == "MR001");
    CHECK(text(*rows[0], 1) == "TANAKA");
    CHECK(text(*rows[0], 2) == "Hana Tanaka");
    CHECK(text(*rows[0], 3) == "1 Clinic Road");
    CHECK(text(*rows[0], 4) == "V6B1A1");
    CHECK(text(*rows[0], 5) == "(604) 555 - 0199");
    CHECK(text(*rows[0], 9) == "A");
    CHECK(rows[0]->cells[10].type == rpt::CellType::Integer);
    CHECK(rows[0]->cells[10].value == 30);
    CHECK(rows[0]->cells[11].type == rpt::CellType::Amount);
    CHECK(rows[0]->cells[11].value == 0);
    // The address continuation: only the address column.
    CHECK(text(*rows[1], 3) == "Vancouver BC");
    CHECK(std::count_if(rows[1]->cells.begin(), rows[1]->cells.end(),
                        [](const rpt::Cell& c) { return c.type != rpt::CellType::Empty; }) == 1);
    CHECK(text(*rows[2], 0) == "MR002");
    CHECK(text(*rows[4], 0) == "MR003");

    const rpt::Row& t = lastRow(r);
    CHECK(t.kind == rpt::RowKind::Note);
    CHECK(t.text == "3 medical representatives");
}

TEST_CASE("6041 Doctor Codes: titled Doctor Codes (D12)", "[reports][master]")
{
    FixtureDb db;
    const rpt::Report r = DoctorCodesListing(db.code, context(db.code));
    CHECK(r.title == "Doctor Codes Summary Report");
    CHECK(r.footerNote == "Report File: gl_drcds.rpt");
    CHECK(r.orientation == rpt::Orientation::Landscape);
    REQUIRE(r.columns.size() == 10);
    CHECK(r.columns[7].header == "Territory");
    CHECK(r.columns[0].header2 == "Code");

    const auto rows = details(r);
    REQUIRE(rows.size() == 6);
    CHECK(text(*rows[0], 0) == "DR001");
    CHECK(text(*rows[0], 2) == "Dr. Alex Morgan");
    CHECK(text(*rows[0], 7) == "NORTH");
    CHECK(text(*rows[3], 3) == "Vancouver BC");
    CHECK(text(*rows[4], 0) == "DR003");
    CHECK(lastRow(r).text == "3 doctors");
}

TEST_CASE("6038 Product code summary: code order, lot size and price", "[reports][master]")
{
    FixtureDb db;
    const rpt::Report r = ProductCodesListing(db.code, context(db.code));
    CHECK(r.title == "General Ledger Product Codes");
    CHECK(r.footerNote == "Report File : gl_prodl.rpt");
    CHECK(r.orientation == rpt::Orientation::Portrait);
    CHECK(r.subtitles.empty());   // GL_PRODL.RPT prints no current period
    REQUIRE(r.columns.size() == 6);

    const auto rows = details(r);
    REQUIRE(rows.size() == 3);
    CHECK(text(*rows[0], 0) == "P100");
    CHECK(text(*rows[0], 1) == "ANALGESIC");
    CHECK(text(*rows[0], 2) == "Analgesic tablets 100mg");
    CHECK(text(*rows[0], 4) == "10");
    CHECK(rows[0]->cells[5].value == 425);   // FAVPRICE 4.25
    CHECK(text(*rows[1], 0) == "P200");
    CHECK(text(*rows[1], 4) == "24");
    CHECK(rows[1]->cells[5].value == 650);
    CHECK(text(*rows[2], 0) == "P300");
    CHECK(lastRow(r).text == "3 products");
}

TEST_CASE("Master listings leave out deleted records", "[reports][master]")
{
    FixtureDb db;
    deleteRecord(db.code, "gl_medrp", "repcode", "MR002");
    deleteRecord(db.code, "gl_itemm", "fpcode", "P100");
    const rpt::Report reps = MedicalRepsListing(db.code, context(db.code));
    const auto rows = details(reps);
    REQUIRE(rows.size() == 4);
    CHECK(text(*rows[2], 0) == "MR003");
    CHECK(lastRow(reps).text == "2 medical representatives");
    const rpt::Report products = ProductCodesListing(db.code, context(db.code));
    CHECK(details(products).size() == 2);
    CHECK(lastRow(products).text == "2 products");
}
