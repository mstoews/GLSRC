//
// gl_tests: the original voucher reports (spec #3 2.2, D4, D5, D7, D8, D17;
// plan S3-M6) on the dev fixture's four vouchers, all period 7, 10/15/2026:
//
//   100  posted      "Cash to checking"            102101 Dr 250 / 101000 Cr 250
//   101  open        "October sales"               111101 Dr 1,150 / 410001-10 Cr 600 / -20 Cr 550
//   102  open        "VAT, out of balance"         148000 Dr 100 / 260007 Cr 90   (difference 10)
//   103  adjustment  "Opening balance correction"  144910 Dr 75 / 111101 Cr 75
//
//   All lines: debits 1,575, credits 1,565.
//   6078 (unposted: 101-103): 111101 8,200 + 1,150 - 75 = 9,275; 144910 0 + 75;
//   148000 310 + 100 = 410; 260007 -450 - 90 = -540; 410001-10 -600; -20 -550.
//   Six accounts, seven lines; 102101 and 101000 (voucher 100) aren't there.
//

#include "GlTestDb.h"

#include "reports/ReportContext.h"
#include "reports/VoucherReports.h"

#include <algorithm>
#include <string>

using namespace gltest;

namespace {

ReportContext context(Code4& code) { return LoadReportContext(code, "20261110"); }

int count(const rpt::Report& r, rpt::RowKind kind)
{
    return static_cast<int>(std::count_if(r.rows.begin(), r.rows.end(), [&](const rpt::Row& x) { return x.kind == kind; }));
}

const rpt::Row& lastTotal(const rpt::Report& r)
{
    const auto it = std::find_if(r.rows.rbegin(), r.rows.rend(), [](const rpt::Row& x) { return x.kind == rpt::RowKind::Total; });
    REQUIRE(it != r.rows.rend());
    return *it;
}

// The value of a slip field, from the n-th Slip row.
std::string field(const rpt::Report& r, int slip, const std::string& label)
{
    int n = 0;
    for (const rpt::Row& x : r.rows) {
        if (x.kind != rpt::RowKind::Slip) continue;
        if (n++ != slip) continue;
        for (const auto& [k, v] : x.fields)
            if (k == label) return v;
    }
    return "<none>";
}

const rpt::Row* rowWithText(const rpt::Report& r, size_t col, const std::string& text)
{
    for (const rpt::Row& x : r.rows)
        if (x.cells.size() > col && x.cells[col].type == rpt::CellType::Text && x.cells[col].text == text) return &x;
    return nullptr;
}

std::vector<std::string> groupHeaders(const rpt::Report& r)
{
    std::vector<std::string> out;
    for (const rpt::Row& x : r.rows)
        if (x.kind == rpt::RowKind::GroupHeader) out.push_back(x.text);
    return out;
}

// Marks voucher 101's line 1 (111101 Dr 1,150) deleted, without packing.
void deleteLine(Code4& code)
{
    Data4 d(&code, "gl_btchd");
    Field4 batchno(d, "batchno"), journal(d, "journal");
    for (d.top(); !d.eof(); d.skip())
        if (batchno.num() == 101 && journal.num() == 1) {
            d.deleteRec();
            d.flush();
            break;
        }
    d.close();
}

} // namespace

TEST_CASE("6074 Voucher Slips: one slip per voucher, every mode", "[reports][vouchers]")
{
    FixtureDb db;
    const rpt::Report r = VoucherSlips(db.code, context(db.code));
    CHECK(r.title == "General Ledger Batch Entry Slip");
    CHECK(r.footerNote == "glslip.rpt");
    CHECK(r.slipMode);
    REQUIRE(count(r, rpt::RowKind::Slip) == 4);
    CHECK(field(r, 0, "Slip Number") == "100");
    CHECK(field(r, 0, "Status") == "Posted");
    CHECK(field(r, 0, "Cheque") == "000123");
    CHECK(field(r, 0, "Date of Entry") == "10/15/2026");
    CHECK(field(r, 1, "Slip Description") == "October sales");
    CHECK(field(r, 1, "Status") == "Open");
    CHECK(field(r, 3, "Status") == "Adjustment");
    CHECK(field(r, 2, "Period") == "7");
    CHECK(count(r, rpt::RowKind::Detail) == 9);
    CHECK(count(r, rpt::RowKind::Total) == 4);
    // Slip 102's total: 100 / 90.
    int totals = 0;
    for (const rpt::Row& x : r.rows)
        if (x.kind == rpt::RowKind::Total && totals++ == 2) {
            CHECK(x.cells[4].text == "Total Batch Entry");
            CHECK(x.cells[6].value == 10000);
            CHECK(x.cells[7].value == 9000);
        }
}

TEST_CASE("8082 Voucher Detail: groups, totals, and 102's signed difference (D8)", "[reports][vouchers]")
{
    FixtureDb db;
    const rpt::Report r = VoucherDetail(db.code, context(db.code));
    CHECK(r.title == "General Ledger - Slip Detail Listing");
    CHECK(r.orientation == rpt::Orientation::Landscape);
    const auto heads = groupHeaders(r);
    REQUIRE(heads.size() == 4);
    CHECK(heads[0] == "Slip 100  Cash to checking  (Posted, period 7)");
    CHECK(heads[3] == "Slip 103  Opening balance correction  (Adjustment, period 7)");
    CHECK(count(r, rpt::RowKind::GroupFooter) == 4);
    const rpt::Row* diff = rowWithText(r, 4, "Difference (debits less credits)");
    REQUIRE(diff);
    CHECK(diff->cells[10].value == 1000);   // only 102, debits over credits by 10
    CHECK(std::count_if(r.rows.begin(), r.rows.end(), [](const rpt::Row& x) {
              return x.cells.size() > 4 && x.cells[4].text == "Difference (debits less credits)";
          }) == 1);
    const rpt::Row& t = lastTotal(r);
    CHECK(t.cells[10].value == 157500);
    CHECK(t.cells[11].value == 156500);
    // A line with its doctor, med rep and product (101's line 2).
    const rpt::Row* sales = rowWithText(r, 7, "DR001");
    REQUIRE(sales);
    CHECK(sales->cells[8].text == "MR001");
    CHECK(sales->cells[9].text == "P100");
    CHECK(sales->cells[6].type == rpt::CellType::Date);
}

TEST_CASE("8082: a credit excess shows as a negative difference", "[reports][vouchers]")
{
    FixtureDb db;
    deleteLine(db.code);   // 101 loses its debit: Dr 0, Cr 1,150
    const rpt::Report r = VoucherDetail(db.code, context(db.code));
    // The original printed nothing for a credit excess; D8 prints it signed.
    std::vector<std::int64_t> diffs;
    for (const rpt::Row& x : r.rows)
        if (x.cells.size() > 4 && x.cells[4].text == "Difference (debits less credits)") diffs.push_back(x.cells[10].value);
    CHECK(diffs == std::vector<std::int64_t>{-115000, 1000});
    CHECK(lastTotal(r).cells[10].value == 157500 - 115000);   // the deleted line is left out
}

TEST_CASE("6075 Voucher Summary", "[reports][vouchers]")
{
    FixtureDb db;
    const rpt::Report r = VoucherSummary(db.code, context(db.code));
    CHECK(r.title == "General Ledger - Slip Summary Report");
    REQUIRE(count(r, rpt::RowKind::Detail) == 4);
    const rpt::Row* sales = rowWithText(r, 4, "October sales");
    REQUIRE(sales);
    CHECK(sales->cells[1].value == 101);
    CHECK(sales->cells[0].value == 7);
    CHECK(sales->cells[5].value == 115000);
    CHECK(sales->cells[6].value == 115000);
    CHECK(rowWithText(r, 2, "000123") != nullptr);   // 100's cheque
    CHECK(lastTotal(r).cells[5].value == 157500);
    CHECK(lastTotal(r).cells[6].value == 156500);
}

TEST_CASE("6078 Preposting Distribution List: unposted lines only (D4)", "[reports][vouchers]")
{
    FixtureDb db;
    const rpt::Report r = PrepostingDistribution(db.code, context(db.code));
    const auto heads = groupHeaders(r);
    REQUIRE(heads.size() == 6);
    CHECK(heads[0] == "111101  ACCOUNTS RECEIVABLE - TRADE");
    CHECK(heads[5] == "410001-20  SALES REVENUE - GENERAL");
    CHECK(std::none_of(heads.begin(), heads.end(), [](const std::string& h) { return h.find("102101") == 0; }));

    std::vector<std::int64_t> after;
    for (const rpt::Row& x : r.rows)
        if (x.kind == rpt::RowKind::GroupFooter) after.push_back(x.cells[8].value);
    CHECK(after == std::vector<std::int64_t>{927500, 7500, 41000, -54000, -60000, -55000});
    const rpt::Row& t = lastTotal(r);
    CHECK(t.cells[3].text == "7 lines");
    CHECK(t.cells[6].value == 132500);   // 1,150 + 100 + 75
    CHECK(t.cells[7].value == 131500);   // 600 + 550 + 90 + 75
}

TEST_CASE("8136 Pre-Posting Report: the unposted vouchers, with their status (D5)", "[reports][vouchers]")
{
    FixtureDb db;
    const rpt::Report r = PrePostingReport(db.code, context(db.code));
    CHECK(r.title == "General Ledger Pre-Posting Report");
    const auto heads = groupHeaders(r);
    REQUIRE(heads.size() == 3);
    CHECK(heads[0].find("Slip 101") == 0);
    CHECK(heads[0].find("(Open, period 7)") != std::string::npos);
    CHECK(heads[2].find("(Adjustment, period 7)") != std::string::npos);
    CHECK(lastTotal(r).cells[10].value == 132500);
}

TEST_CASE("D7: one voucher's slip; a missing voucher says so", "[reports][vouchers]")
{
    FixtureDb db;
    const rpt::Report r = VoucherSlip(db.code, context(db.code), 102);
    CHECK(r.footerNote == "gl_slips.rpt");
    REQUIRE(count(r, rpt::RowKind::Slip) == 1);
    CHECK(field(r, 0, "Slip Number") == "102");
    CHECK(count(r, rpt::RowKind::Detail) == 2);
    const rpt::Report none = VoucherSlip(db.code, context(db.code), 999);
    CHECK(count(none, rpt::RowKind::Slip) == 0);
    CHECK(none.rows.back().kind == rpt::RowKind::Note);
}

TEST_CASE("VoucherStatus", "[reports][vouchers]")
{
    CHECK(VoucherStatus("1") == "Open");
    CHECK(VoucherStatus("3") == "Adjustment");
    CHECK(VoucherStatus("0") == "Posted");
    CHECK(VoucherStatus("2") == "Posted adjustment");
    CHECK(VoucherStatus("9") == "Closed year");
}
