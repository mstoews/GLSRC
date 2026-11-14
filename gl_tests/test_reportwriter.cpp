//
// gl_tests: the report writer's core (spec #7 4, 5, 7, D2-D4, D9; plan S7-M0)
// on the dev fixture. Each source is checked against its owning module's own
// figures (plan P2): sales lines against Ar.h's voucher lines, GL lines
// against gl_btchd, AR open items against LoadArOpenItems.
//
#include "GlTestDb.h"

#include "Accounts.h"   // TrimText
#include "Ap.h"
#include "Ar.h"
#include "Assets.h"
#include "reports/ApReports.h"
#include "reports/ArReports.h"
#include "reports/AssetReports.h"
#include "reports/TrialBalances.h"
#include "ReportWriter.h"

#include "report/Format.h"   // rpt::ToCents
#include "report/Csv.h"
#include "report/TextRender.h"

#include <map>

#include <string>
#include <vector>

using namespace gltest;
using namespace rw;

namespace {

int rowsOf(const rpt::Report& r, rpt::RowKind kind)
{
    int n = 0;
    for (const rpt::Row& row : r.rows) n += row.kind == kind;
    return n;
}

const rpt::Row* lastOf(const rpt::Report& r, rpt::RowKind kind)
{
    for (auto it = r.rows.rbegin(); it != r.rows.rend(); ++it)
        if (it->kind == kind) return &*it;
    return nullptr;
}

std::vector<std::string> headers(const rpt::Report& r)
{
    std::vector<std::string> out;
    for (const rpt::Row& row : r.rows)
        if (row.kind == rpt::RowKind::GroupHeader) out.push_back(row.text);
    return out;
}

// The text of column `col` in each Detail row.
std::vector<std::string> column(const rpt::Report& r, size_t col)
{
    std::vector<std::string> out;
    for (const rpt::Row& row : r.rows)
        if (row.kind == rpt::RowKind::Detail)
            out.push_back(row.cells[col].type == rpt::CellType::Text || row.cells[col].type == rpt::CellType::Date
                              ? row.cells[col].text
                              : std::to_string(row.cells[col].value));
    return out;
}

ReportDefinition parsed(const std::string& text)
{
    const ParseResult p = ParseDefinition(text);
    INFO(p.error << " (line " << p.line << ")");
    REQUIRE(p.definition);
    return *p.definition;
}

rpt::Report built(cb4sqlite::Code4& code, const ReportContext& ctx, const std::string& text, const Answers& a = {})
{
    const BuildResult b = BuildReport(code, ctx, parsed(text), a);
    INFO(b.refusal);
    REQUIRE(b.report);
    return *b.report;
}

// Codes on the fixture's sales lines, which have none: voucher -> (product, med rep).
void codeSalesLines(cb4sqlite::Code4& code)
{
    const std::map<long, std::pair<const char*, const char*>> codes = {
        {1, {"P100", "MR001"}}, {2, {"P200", "MR002"}}, {3, {"P100", "MR001"}},
        {4, {"P200", "MR002"}}, {5, {"P300", "MR001"}}, {6, {"P100", "MR001"}}};
    Data4 d(&code, "ar_btchd");
    Field4 batch(d, "batchno"), product(d, "product"), medrep(d, "medrep");
    for (d.top(); !d.eof(); d.skip()) {
        auto it = codes.find(std::atol(batch.str()));
        if (it == codes.end()) continue;
        product.assign(it->second.first);
        medrep.assign(it->second.second);
        d.flush();
    }
    d.close();
}

const char* kSalesByProduct = R"(writer 1
name "Sales by product"
description "Net sales and VAT by product, posted vouchers"
source sales_lines 1
column invoice
column qty total
column sales total
column vat total
group product
filter state in Posted Transferred
)";

} // namespace

TEST_CASE("The definition text: round trips and refusals (S7-M0)", "[reportwriter]")
{
    ReportDefinition d;
    d.name = "Every \"setting\"";
    d.description = "With a \\ and quotes";
    d.title = "A title";
    d.subtitle = "A subtitle";
    d.landscape = true;
    d.source = "gl_lines";
    d.sourceVersion = 1;
    d.columns = {{"account", "", 0, false, false}, {"debit", "Dr", 1.5, true, true}, {"net", "", 0, true, false}};
    d.sort = {{"date", true}, {"voucher", false}};
    d.groups = {{"account", true}, {"dept", false}};
    d.filters = {{"period", Test::Between, {}, true, "Periods"},
                 {"reference", Test::Contains, {"two words"}, false, {}},
                 {"state", Test::In, {"Open", "Posted"}, false, {}},
                 {"cheque", Test::Blank, {}, false, {}},
                 {"account", Test::Equal, {"410001"}, false, {}},
                 {"dept", Test::NotEqual, {""}, false, {}},
                 {"med_rep", Test::NotBlank, {}, false, {}},
                 {"product", Test::In, {}, true, ""}};
    d.grandTotal = false;
    d.count = true;
    const std::string text = WriteDefinition(d);
    const ReportDefinition back = parsed(text);
    CHECK(WriteDefinition(back) == text);
    CHECK(back.name == d.name);
    CHECK(back.description == d.description);
    CHECK(back.landscape);
    REQUIRE(back.columns.size() == 3);
    CHECK(back.columns[1].label == "Dr");
    CHECK(back.columns[1].width == 1.5);
    CHECK(back.columns[1].total);
    CHECK(back.columns[1].blankZero);
    CHECK(back.sort[0].descending);
    CHECK(back.groups[0].newPage);
    REQUIRE(back.filters.size() == 8);
    CHECK(back.filters[0].ask);
    CHECK(back.filters[0].question == "Periods");
    CHECK(back.filters[1].values == std::vector<std::string>{"two words"});
    CHECK(back.filters[5].values == std::vector<std::string>{""});
    CHECK(back.filters[7].ask);
    CHECK_FALSE(back.grandTotal);
    CHECK(back.count);

    // Defaults: the description and title are the name; comments and blank lines are skipped.
    const ReportDefinition small = parsed("# a comment\nwriter 1\n\nname Small\nsource ar_open_items 1\ncolumn owed\n");
    CHECK(small.description == "Small");
    CHECK(small.title == "Small");
    CHECK(small.grandTotal);
    CHECK_FALSE(small.landscape);

    // Refusals, with their line numbers.
    auto refused = [](const std::string& t, int line, const char* words) {
        const ParseResult p = ParseDefinition(t);
        INFO(t);
        CHECK_FALSE(p.definition);
        CHECK(p.line == line);
        CHECK(p.error.find(words) != std::string::npos);
    };
    refused("name X\n", 1, "writer 1");
    refused("writer 2\nname X\n", 1, "newer version");
    refused("writer 1\nname X\nsource s 1\ncolour red\n", 4, "Unknown setting");
    refused("writer 1\nname \"X\n", 2, "quotation");
    refused("writer 1\nname X\nsource s 1\nfilter a between 1\n", 4, "takes 2");
    refused("writer 1\nname X\nsource s 1\nfilter a between ? 5\n", 4, "\"?\"");
    refused("writer 1\nname X\nsource s 1\nfilter a in\n", 4, "one value or more");
    refused("writer 1\nname X\nsource s 1\nfilter a like b\n", 4, "Unknown test");
    refused("writer 1\nname X\nsource s 1\nsort a\nsort b\nsort c\nsort d\n", 7, "three");
    refused("writer 1\nname X\nsource s 1\ngroup a\ngroup b\ngroup c\n", 6, "two groups");
    refused("writer 1\nname X\nsource s 1\ncolumn a width 0\n", 4, "width");
    refused("writer 1\nname X\nsource s 1\ncolumn a bold\n", 4, "Unknown column option");
    refused("writer 1\nsource s 1\n", 2, "no name");
    refused("writer 1\nname X\n", 2, "no source");
    refused("writer 1\nname X\nsource s one\n", 3, "version");
    refused("writer 1\nname X\nsource s 1\ntotal maybe\n", 4, "yes or no");
}

TEST_CASE("The sources agree with their modules (S7-M0, P2)", "[reportwriter]")
{
    FixtureDb db;
    EnsureArTables(db.code);
    const ReportContext ctx = LoadReportContext(db.code, "20261110");
    REQUIRE(Sources().size() == 14);
    for (const Source& s : Sources()) {
        INFO(s.key);
        CHECK(FindSource(s.key) == &s);
        for (const SourceRow& row : s.rows(db.code, ctx)) CHECK(row.size() == s.fields.size());
        for (const Field& f : s.fields)
            if (!f.nameField.empty()) CHECK(s.fieldIndex(f.nameField) >= 0);
    }
    CHECK_FALSE(FindSource("nope"));

    // Sales lines: the product lines of every voucher, their net and VAT as the ledger has them.
    const Source& sales = *FindSource("sales_lines");
    const auto rows = sales.rows(db.code, ctx);
    std::int64_t lines = 0, postedSales = 0, postedVat = 0, ledgerSales = 0, ledgerVat = 0;
    for (const ArVoucher& v : LoadArVouchers(db.code))
        for (const ArLine& l : LoadArLines(db.code, v.number)) {
            if (l.role == ArLineRole::Sales) ++lines;
            if (v.mode != ar_mode::kPosted) continue;
            if (l.role == ArLineRole::Sales) ledgerSales += l.credit;
            if (l.role == ArLineRole::Vat) ledgerVat += l.credit;
        }
    CHECK(static_cast<std::int64_t>(rows.size()) == lines);
    const int state = sales.fieldIndex("state"), net = sales.fieldIndex("sales"), vat = sales.fieldIndex("vat"),
              amount = sales.fieldIndex("amount");
    for (const SourceRow& r : rows) {
        CHECK(r[static_cast<size_t>(amount)].number == r[static_cast<size_t>(net)].number + r[static_cast<size_t>(vat)].number);
        if (r[static_cast<size_t>(state)].text == "Posted") {
            postedSales += r[static_cast<size_t>(net)].number;
            postedVat += r[static_cast<size_t>(vat)].number;
        }
    }
    CHECK(postedSales == ledgerSales);
    CHECK(postedVat == ledgerVat);
    CHECK(postedSales == 380000);   // vouchers 3, 4, 5
    CHECK(postedVat == 30000);

    // GL voucher lines: every gl_btchd line, its debits and credits.
    const Source& gl = *FindSource("gl_lines");
    const auto glRows = gl.rows(db.code, ctx);
    cb4sqlite::Query q(db.code, "SELECT count(*), sum(debit), sum(credit) FROM gl_btchd WHERE _deleted = 0");
    REQUIRE(q.next());
    CHECK(static_cast<std::int64_t>(glRows.size()) == q.integer(0));
    std::int64_t dr = 0, cr = 0;
    for (const SourceRow& r : glRows) {
        dr += r[static_cast<size_t>(gl.fieldIndex("debit"))].number;
        cr += r[static_cast<size_t>(gl.fieldIndex("credit"))].number;
        CHECK_FALSE(r[static_cast<size_t>(gl.fieldIndex("state"))].text == "?");
    }
    CHECK(dr == rpt::ToCents(q.number(1)));
    CHECK(cr == rpt::ToCents(q.number(2)));

    // AR open items: LoadArOpenItems, with the customers' names.
    const Source& open = *FindSource("ar_open_items");
    const auto openRows = open.rows(db.code, ctx);
    const auto items = LoadArOpenItems(db.code);
    REQUIRE(openRows.size() == items.size());
    for (size_t i = 0; i < items.size(); ++i) {
        CHECK(openRows[i][static_cast<size_t>(open.fieldIndex("voucher"))].number == items[i].batchno);
        CHECK(openRows[i][static_cast<size_t>(open.fieldIndex("owed"))].number == items[i].remainder);
        CHECK_FALSE(openRows[i][static_cast<size_t>(open.fieldIndex("customer_name"))].text.empty());
    }
}

TEST_CASE("BuildReport: groups, totals, sort, pages (S7-M0)", "[reportwriter]")
{
    FixtureDb db;
    EnsureArTables(db.code);
    codeSalesLines(db.code);
    const ReportContext ctx = LoadReportContext(db.code, "20261110");

    // Sales by product, posted vouchers: P100 voucher 3, P200 voucher 4, P300 voucher 5.
    const rpt::Report r = built(db.code, ctx, kSalesByProduct);
    CHECK(r.title == "Sales by product");
    CHECK(r.footerNote == "Sales by product");
    CHECK(r.subtitles == std::vector<std::string>{"State is one of Posted, Transferred"});
    REQUIRE(r.columns.size() == 4);
    CHECK(r.columns[0].header == "Invoice");
    CHECK(r.columns[2].header == "Sales (net)");
    CHECK(r.columns[2].type == rpt::CellType::Amount);
    CHECK(headers(r) == std::vector<std::string>{"Product: P100  Analgesic tablets 100mg",
                                                 "Product: P200  Cough syrup 200ml",
                                                 "Product: P300  Topical ointment 30g"});
    CHECK(column(r, 0) == std::vector<std::string>{"S-1003", "S-1004", "S-1005"});
    CHECK(rowsOf(r, rpt::RowKind::GroupFooter) == 3);
    const rpt::Row* total = lastOf(r, rpt::RowKind::Total);
    REQUIRE(total);
    CHECK(total->final);
    CHECK(total->cells[0].text == "Total");
    CHECK(total->cells[2].value == 380000);
    CHECK(total->cells[3].value == 30000);
    CHECK(total->cells[1].value == 500);   // 1 + 4 units (voucher 5's amount was typed: no quantity)
    for (const rpt::Row& row : r.rows) {
        if (row.kind == rpt::RowKind::Detail) CHECK(row.indent == 1);
        if (row.kind == rpt::RowKind::Detail || row.kind == rpt::RowKind::GroupFooter || row.kind == rpt::RowKind::Total)
            CHECK(row.cells.size() == r.columns.size());
    }
    const rpt::Row* footer = lastOf(r, rpt::RowKind::GroupFooter);
    CHECK(footer->cells[0].text == "Total P300");
    CHECK(footer->cells[2].value == 80000);

    // Two groups, a page per med rep; counts.
    const rpt::Report two = built(db.code, ctx, std::string(kSalesByProduct) + "group med_rep page\ncount yes\n");
    // Groups are med rep within product: the first group's order is product's.
    CHECK(rowsOf(two, rpt::RowKind::GroupHeader) == 6);   // 3 products, each with its one med rep
    CHECK(rowsOf(two, rpt::RowKind::PageBreak) == 2);     // before the 2nd and 3rd med-rep groups
    CHECK(lastOf(two, rpt::RowKind::Total)->cells[0].text == "Total (3 rows)");

    // Every state but cancelled by default: 6 lines (voucher 6 left out); asking for the cancelled.
    const std::string base = "writer 1\nname S\nsource sales_lines 1\ncolumn voucher\ncolumn amount total\n";
    CHECK(column(built(db.code, ctx, base), 0).size() == 6);
    CHECK(column(built(db.code, ctx, base + "filter state = cancelled\n"), 0) == std::vector<std::string>{"6"});

    // Sort descending, then ascending on ties.
    const rpt::Report sorted = built(db.code, ctx,
                                     "writer 1\nname O\nsource ar_open_items 1\ncolumn voucher\ncolumn owed total\n"
                                     "sort owed desc\nsort voucher\n");
    CHECK(column(sorted, 0) == std::vector<std::string>{"3", "4", "5"});   // 2,200.00, 600.00, 0.00
    CHECK(lastOf(sorted, rpt::RowKind::Total)->cells[1].value == 280000);

    // No total row when asked for none, or nothing is totalled.
    CHECK_FALSE(lastOf(built(db.code, ctx, base + "total no\n"), rpt::RowKind::Total));
    CHECK_FALSE(lastOf(built(db.code, ctx, "writer 1\nname N\nsource ar_open_items 1\ncolumn voucher\n"),
                       rpt::RowKind::Total));
}

TEST_CASE("BuildReport: filters, asked values, refusals (S7-M0)", "[reportwriter]")
{
    FixtureDb db;
    EnsureArTables(db.code);
    const ReportContext ctx = LoadReportContext(db.code, "20261110");
    const std::string head = "writer 1\nname F\nsource ar_open_items 1\ncolumn voucher\n";
    auto vouchers = [&](const std::string& filter, const Answers& a = {}) {
        return column(built(db.code, ctx, head + filter, a), 0);
    };
    using V = std::vector<std::string>;
    // The fixture: 3 owes 2,200.00 (C001, due 11/04, never received); 4 owes 600.00 (C002, due 11/07,
    // received 10/20); 5 is settled (C003, Northside Pharmacy, received 10/15).
    CHECK(vouchers("filter owed between 500 2500\n") == V{"3", "4"});
    CHECK(vouchers("filter owed between 500.00 \"2,200.00\"\n") == V{"3", "4"});
    CHECK(vouchers("filter state = settled\n") == V{"5"});
    CHECK(vouchers("filter state <> Settled\n") == V{"3", "4"});
    CHECK(vouchers("filter customer in C001 c003\n") == V{"3", "5"});
    CHECK(vouchers("filter customer_name contains PHARM\n") == V{"5"});
    CHECK(vouchers("filter date_paid blank\n") == V{"3"});
    CHECK(vouchers("filter date_paid notblank\n") == V{"4", "5"});
    CHECK(vouchers("filter due_date between 11/01/2026 20261130\n") == V{"3", "4"});
    CHECK(vouchers("filter period = 7\nfilter owed = 0\n") == V{"5"});
    CHECK(vouchers("filter received blank\n") == V{"3"});

    // The filters are the subtitles, dates as dates.
    const rpt::Report sub = built(db.code, ctx, head + "subtitle \"For the auditors\"\nfilter due_date between 20261101 20261130\n");
    CHECK(sub.subtitles == std::vector<std::string>{"For the auditors", "Due date from 11/01/2026 to 11/30/2026"});

    // Asked values: refused without an answer; answered, they filter and show.
    const std::string asked = head + "filter owed between ? ? \"Owed from and to\"\n";
    BuildResult b = BuildReport(db.code, ctx, parsed(asked));
    CHECK_FALSE(b.report);
    CHECK(b.refusal.find("Owed from and to") != std::string::npos);
    CHECK(vouchers("filter owed between ? ?\n", {{0, {"0", "1000"}}}) == V{"4", "5"});
    b = BuildReport(db.code, ctx, parsed(asked), {{0, {"0"}}});   // one answer for two
    CHECK_FALSE(b.report);
    b = BuildReport(db.code, ctx, parsed(asked), {{0, {"none", "1000"}}});
    CHECK_FALSE(b.report);
    CHECK(b.refusal == "Owed: \"none\" isn't a number.");
    b = BuildReport(db.code, ctx, parsed(head + "filter date = 31/12/2026\n"));
    CHECK(b.refusal.find("isn't a date") != std::string::npos);

    // A field the source doesn't have (D9): noted and left out; the report still runs.
    const rpt::Report gone = built(db.code, ctx, head + "column colour\ngroup shape\nsort size\nfilter weight = 1\n");
    CHECK(gone.columns.size() == 1);
    CHECK(rowsOf(gone, rpt::RowKind::Note) == 4);
    CHECK(rowsOf(gone, rpt::RowKind::Detail) == 3);
    CHECK(gone.rows[0].text == "\"colour\" isn't a field of AR open items any more: the column is left out.");

    // Refusals: an unknown source, no column, too many rows.
    b = BuildReport(db.code, ctx, parsed("writer 1\nname X\nsource nope 1\ncolumn a\n"));
    CHECK(b.refusal.find("\"nope\"") != std::string::npos);
    b = BuildReport(db.code, ctx, parsed("writer 1\nname X\nsource ar_open_items 1\ncolumn colour\n"));
    CHECK(b.refusal == "The report has no column to print.");
    b = BuildReport(db.code, ctx, parsed(head), {}, 2);
    CHECK_FALSE(b.report);
    CHECK(b.refusal.find("more than 2 rows") != std::string::npos);
    CHECK(BuildReport(db.code, ctx, parsed(head), {}, 3).report);

    // Nothing matches: a note, no total.
    const rpt::Report none = built(db.code, ctx, head + "column owed total\nfilter owed between 1 2\n");
    CHECK(rowsOf(none, rpt::RowKind::Detail) == 0);
    CHECK(none.rows.back().text == "No rows match the report's filters.");
    CHECK_FALSE(lastOf(none, rpt::RowKind::Total));
}

TEST_CASE("gl_rptdef: save, replace, load, delete (S7-M0)", "[reportwriter]")
{
    EmptyDb empty;
    EnsureReportWriterTables(empty.code);
    EnsureReportWriterTables(empty.code);   // twice: nothing changes
    CHECK(LoadDefinitions(empty.code).empty());
    CHECK_FALSE(FindDefinition(empty.code, 1));

    FixtureDb db;
    CHECK(LoadDefinitions(db.code).size() == 3);   // the fixture's examples (S7-M1)
    ReportDefinition sales = parsed(kSalesByProduct);
    sales.name = "Sales by product, mine";
    const long one = SaveDefinition(db.code, 0, sales, "ADMIN", "20261114093000");
    CHECK(one == 4);
    ReportDefinition other = parsed("writer 1\nname \"AR by customer\"\nsource ar_open_items 1\ncolumn owed total\n");
    const long two = SaveDefinition(db.code, 0, other, "CLERK", "20261114100000");
    CHECK(two == 5);

    auto all = LoadDefinitions(db.code);
    REQUIRE(all.size() == 5);
    CHECK(all[0].name == "AR by customer");   // name order
    const StoredDefinition& mine = all[4];
    CHECK(mine.name == "Sales by product, mine");
    CHECK(mine.description == "Net sales and VAT by product, posted vouchers");
    CHECK(mine.source == "sales_lines");
    CHECK(mine.sourceVersion == 1);
    CHECK(mine.madeBy == "ADMIN");
    CHECK(mine.madeOn == "20261114093000");
    CHECK(parsed(mine.text).columns.size() == 4);
    CHECK(WriteDefinition(parsed(mine.text)) == WriteDefinition(sales));

    // Replace 2: the same row, changed by another, made by the first.
    other.columns.push_back({"customer", "", 0, false, false});
    CHECK(SaveDefinition(db.code, two, other, "ADMIN", "20261114110000") == two);
    const auto again = FindDefinition(db.code, two);
    REQUIRE(again);
    CHECK(again->madeBy == "CLERK");
    CHECK(again->changedBy == "ADMIN");
    CHECK(again->changedOn == "20261114110000");
    CHECK(parsed(again->text).columns.size() == 2);
    CHECK(LoadDefinitions(db.code).size() == 5);

    CHECK(DefinitionNameTaken(db.code, "  sales BY product, MINE "));
    CHECK_FALSE(DefinitionNameTaken(db.code, "Sales by product, mine", one));
    CHECK(DefinitionNameTaken(db.code, "Sales by product"));   // the fixture's
    CHECK_FALSE(DefinitionNameTaken(db.code, "Something else"));

    // A stored definition builds.
    EnsureArTables(db.code);
    const ReportContext ctx = LoadReportContext(db.code, "20261110");
    CHECK(BuildReport(db.code, ctx, parsed(FindDefinition(db.code, one)->text)).report);

    CHECK(DeleteDefinition(db.code, one));
    CHECK_FALSE(DeleteDefinition(db.code, one));
    CHECK_FALSE(FindDefinition(db.code, one));
    CHECK(LoadDefinitions(db.code).size() == 4);
    CHECK(SaveDefinition(db.code, 0, sales, "ADMIN", "20261114120000") == 6);   // the highest + 1
}

TEST_CASE("The fixture's example reports (S7-M1)", "[reportwriter]")
{
    FixtureDb db;
    EnsureArTables(db.code);
    const ReportContext ctx = LoadReportContext(db.code, "20261110");
    const auto stored = LoadDefinitions(db.code);
    REQUIRE(stored.size() == 3);
    for (const StoredDefinition& s : stored) {
        INFO(s.name);
        const ReportDefinition d = parsed(s.text);
        CHECK(WriteDefinition(d) == s.text);   // as WriteDefinition writes it
        CHECK(d.name == s.name);
        CHECK(d.source == s.source);
        CHECK(s.madeBy == "ADMIN");
        bool asks = false;
        for (const Filter& f : d.filters) asks = asks || f.ask;
        CHECK(BuildReport(db.code, ctx, d).report.has_value() == !asks);   // an asked report needs its answers
    }
    auto byName = [&](const char* name, const Answers& a = {}) {
        for (const StoredDefinition& s : stored)
            if (s.name == name) return built(db.code, ctx, s.text, a);
        FAIL(name);
        return rpt::Report{};
    };

    // Sales by product: the fixture's codes, the posted vouchers' 3,800.00 and 300.00.
    const rpt::Report sales = byName("Sales by product");
    CHECK(headers(sales) == std::vector<std::string>{"Product: P100  Analgesic tablets 100mg",
                                                     "Product: P200  Cough syrup 200ml",
                                                     "Product: P300  Topical ointment 30g"});
    CHECK(lastOf(sales, rpt::RowKind::Total)->cells[3].value == 380000);
    CHECK(lastOf(sales, rpt::RowKind::Total)->cells[4].value == 30000);

    // GL lines by account asks for the periods (S7-M3). Periods 7 to 7: every
    // period-7 line, balanced as gl_btchd is, and debit - credit beside them.
    const rpt::Report gl = byName("GL lines by account", {{0, {"7", "7"}}});
    CHECK(gl.orientation == rpt::Orientation::Landscape);
    CHECK(gl.subtitles == std::vector<std::string>{"Period from 7 to 7"});
    cb4sqlite::Query q(db.code, "SELECT count(*), sum(debit), sum(credit) FROM gl_btchd WHERE _deleted = 0 AND "
                                "CAST(trim(period) AS INTEGER) = 7");
    REQUIRE(q.next());
    CHECK(rowsOf(gl, rpt::RowKind::Detail) == q.integer(0));
    CHECK(lastOf(gl, rpt::RowKind::Total)->cells[4].value == rpt::ToCents(q.number(1)));
    CHECK(lastOf(gl, rpt::RowKind::Total)->cells[5].value == rpt::ToCents(q.number(2)));
    CHECK(lastOf(gl, rpt::RowKind::Total)->cells[6].value == rpt::ToCents(q.number(1)) - rpt::ToCents(q.number(2)));
    // A second run, periods 8 to 8: the fixture has no lines there.
    const rpt::Report gl8 = byName("GL lines by account", {{0, {"8", "8"}}});
    CHECK(gl8.subtitles == std::vector<std::string>{"Period from 8 to 8"});
    CHECK(rowsOf(gl8, rpt::RowKind::Detail) == 0);
    CHECK(lastOf(gl8, rpt::RowKind::Note)->text == "No rows match the report's filters.");

    // Open items by customer: what's owed, 2,800.00, as the AR reports say.
    const rpt::Report open = byName("Open items by customer");
    CHECK(headers(open) == std::vector<std::string>{"Customer: C001  City General Hospital", "Customer: C002  Reyes, Ana Dra."});
    CHECK(lastOf(open, rpt::RowKind::Total)->cells[4].value == 280000);
}

TEST_CASE("The designer's checks: CheckDefinition, CopyName, MissingFields, DescribeFilter (S7-M2)", "[reportwriter]")
{
    FixtureDb db;
    ReportDefinition d = parsed("writer 1\nname Mine\nsource ar_open_items 1\ncolumn owed total\n");

    // CheckDefinition: a name (40 at most, not another's), a source, a column.
    CHECK_FALSE(CheckDefinition(db.code, d));
    d.name = "  ";
    auto p = CheckDefinition(db.code, d);
    REQUIRE(p);
    CHECK(p->setting == DefinitionSetting::Name);
    d.name = std::string(41, 'x');
    CHECK(CheckDefinition(db.code, d)->text.find("40") != std::string::npos);
    d.name = " sales BY product";   // the fixture's report 1
    p = CheckDefinition(db.code, d);
    REQUIRE(p);
    CHECK(p->text == "Another report is already called \"sales BY product\".");
    CHECK_FALSE(CheckDefinition(db.code, d, 1));   // report 1 itself, renamed in place
    d.name = "Mine";
    d.source = "nope";
    CHECK(CheckDefinition(db.code, d)->setting == DefinitionSetting::Source);
    d.source = "ar_open_items";
    d.columns.clear();
    CHECK(CheckDefinition(db.code, d)->setting == DefinitionSetting::Columns);

    // CopyName: "Copy of", then numbered; 40 characters at most.
    CHECK(CopyName(db.code, "Sales by product") == "Copy of Sales by product");
    ReportDefinition copy = parsed(kSalesByProduct);
    copy.name = "Copy of Sales by product";
    SaveDefinition(db.code, 0, copy, "ADMIN", "20261114130000");
    CHECK(CopyName(db.code, "Sales by product") == "Copy of Sales by product (2)");
    copy.name = "Copy of Sales by product (2)";
    SaveDefinition(db.code, 0, copy, "ADMIN", "20261114130100");
    CHECK(CopyName(db.code, "Sales by product") == "Copy of Sales by product (3)");
    const std::string longName = CopyName(db.code, std::string(40, 'a'));
    CHECK(longName.size() == 40);
    CHECK(longName.rfind("Copy of aaa", 0) == 0);

    // MissingFields: each field the source lacks, once, columns first.
    ReportDefinition gone = parsed("writer 1\nname G\nsource ar_open_items 1\ncolumn owed\ncolumn colour\nsort size\n"
                                   "group colour\nfilter weight = 1\nfilter owed = 0\n");
    CHECK(MissingFields(gone) == std::vector<std::string>{"colour", "size", "weight"});
    gone.source = "nope";
    CHECK(MissingFields(gone).empty());   // an unknown source is CheckDefinition's

    // DescribeFilter: as the subtitles say it; asked ones; unknown fields; incomplete values.
    const Source& open = *FindSource("ar_open_items");
    CHECK(DescribeFilter(open, Filter{"due_date", Test::Between, {"20261101", "11/30/2026"}, false, {}}) ==
          "Due date from 11/01/2026 to 11/30/2026");
    CHECK(DescribeFilter(open, Filter{"state", Test::In, {"Owed", "Part received"}, false, {}}) ==
          "State is one of Owed, Part received");
    CHECK(DescribeFilter(open, Filter{"date_paid", Test::Blank, {}, false, {}}) == "Last received is blank");
    CHECK(DescribeFilter(open, Filter{"period", Test::Between, {}, true, "Periods"}) == "Period: asked when run (Periods)");
    CHECK(DescribeFilter(open, Filter{"period", Test::Between, {}, true, ""}) == "Period: asked when run");
    CHECK(DescribeFilter(open, Filter{"colour", Test::Equal, {"red"}, false, {}}) == "colour (not a field of AR open items)");
    CHECK(DescribeFilter(open, Filter{"owed", Test::Between, {"1"}, false, {}}) == "Owed between ?");
}

TEST_CASE("FilterProblem: a filter's values suit its field (S7-M2)", "[reportwriter]")
{
    FixtureDb db;
    const Source& sales = *FindSource("sales_lines");
    CHECK(FilterProblem(sales, Filter{"state", Test::In, {"Posted", "Transferred"}, false, {}}).empty());
    CHECK(FilterProblem(sales, Filter{"date", Test::In, {"Posted"}, false, {}}) == "Date: \"Posted\" isn't a date (mm/dd/yyyy).");
    CHECK(FilterProblem(sales, Filter{"sales", Test::Between, {"0", "lots"}, false, {}}) == "Sales (net): \"lots\" isn't a number.");
    CHECK(FilterProblem(sales, Filter{"period", Test::Equal, {"7.5"}, false, {}}) == "Period: \"7.5\" isn't a number.");
    CHECK(FilterProblem(sales, Filter{"period", Test::Between, {"7"}, false, {}}) == "\"between\" takes 2 value(s).");
    CHECK(FilterProblem(sales, Filter{"period", Test::In, {}, false, {}}) == "\"in\" takes one value or more.");
    CHECK(FilterProblem(sales, Filter{"date", Test::Between, {"10/01/2026", "20261031"}, false, {}}).empty());
    CHECK(FilterProblem(sales, Filter{"date", Test::Between, {}, true, "Dates"}).empty());   // asked: checked when run
    CHECK(FilterProblem(sales, Filter{"colour", Test::Equal, {"x"}, false, {}}).empty());     // missing: noted when run

    ReportDefinition d = parsed("writer 1\nname Bad\nsource sales_lines 1\ncolumn sales\nfilter date in Posted\n");
    const auto p = CheckDefinition(db.code, d);
    REQUIRE(p);
    CHECK(p->setting == DefinitionSetting::Filters);
    CHECK(p->text == "A filter doesn't suit its field: Date: \"Posted\" isn't a date (mm/dd/yyyy).");
}

TEST_CASE("Calculated columns: sum, difference, count, percentages (S7-M3)", "[reportwriter]")
{
    FixtureDb db;
    EnsureArTables(db.code);
    const ReportContext ctx = LoadReportContext(db.code, "20261110");

    // The format: each kind round-trips; refusals.
    const std::string text = "writer 1\nname C\nsource ar_open_items 1\ncolumn calc sum amount vat\n"
                             "column calc diff amount received total label Owing\ncolumn calc count total\n"
                             "column calc pct owed total total\ncolumn calc pct owed group\n";
    const ReportDefinition d = parsed(text);
    REQUIRE(d.columns.size() == 5);
    CHECK(d.columns[0].calc == Calc::Sum);
    CHECK(d.columns[0].field2 == "vat");
    CHECK(d.columns[1].calc == Calc::Difference);
    CHECK(d.columns[1].label == "Owing");
    CHECK(d.columns[2].calc == Calc::Count);
    CHECK(d.columns[3].calc == Calc::Percent);
    CHECK_FALSE(d.columns[3].ofGroup);
    CHECK(d.columns[3].total);
    CHECK(d.columns[4].ofGroup);
    CHECK(WriteDefinition(parsed(WriteDefinition(d))) == WriteDefinition(d));
    for (const char* bad : {"column calc sum amount\n", "column calc pct owed\n", "column calc avg owed\n"}) {
        const ParseResult p = ParseDefinition(std::string("writer 1\nname X\nsource s 1\n") + bad);
        INFO(bad);
        CHECK_FALSE(p.definition);
        CHECK(p.line == 4);
    }

    // Labels.
    const Source& open = *FindSource("ar_open_items");
    CHECK(ColumnLabel(open, d.columns[0]) == "Amount + VAT");
    CHECK(ColumnLabel(open, d.columns[1]) == "Amount - Received");
    CHECK(ColumnLabel(open, d.columns[2]) == "Count");
    CHECK(ColumnLabel(open, d.columns[3]) == "Owed % of total");
    CHECK(ColumnLabel(open, d.columns[4]) == "Owed % of group");

    // Amount - Received is what's owed; the count totals the rows; percentages of the 2,800.00 owed.
    const rpt::Report r = built(db.code, ctx,
                                "writer 1\nname C\nsource ar_open_items 1\ncolumn voucher\n"
                                "column calc diff amount received total\ncolumn owed total\ncolumn calc count total\n"
                                "column calc pct owed total total\n");
    CHECK(r.columns[1].header == "Amount - Received");
    CHECK(r.columns[3].header == "Count");
    CHECK(r.columns[3].type == rpt::CellType::Integer);
    std::vector<std::int64_t> diff, owed, pct;
    for (const rpt::Row& row : r.rows)
        if (row.kind == rpt::RowKind::Detail) {
            diff.push_back(row.cells[1].value);
            owed.push_back(row.cells[2].value);
            CHECK(row.cells[3].value == 1);
            pct.push_back(row.cells[4].value);
        }
    CHECK(diff == owed);
    CHECK(pct == std::vector<std::int64_t>{7857, 2143, 0});   // 2,200 and 600 of 2,800.00
    const rpt::Row* total = lastOf(r, rpt::RowKind::Total);
    CHECK(total->cells[1].value == 280000);
    CHECK(total->cells[3].value == 3);
    CHECK(total->cells[4].value == 10000);   // 100.00

    // Of the group: sales by med rep, each line's share of its med rep's sales.
    const rpt::Report g = built(db.code, ctx,
                                "writer 1\nname G\nsource sales_lines 1\ncolumn invoice\ncolumn sales total\n"
                                "column calc pct sales group total\ncolumn calc pct sales total total\ngroup med_rep\n"
                                "filter state in Posted Transferred\n");
    std::vector<std::int64_t> ofGroup, ofTotal, footers, footerShare;
    for (const rpt::Row& row : g.rows) {
        if (row.kind == rpt::RowKind::Detail) {
            ofGroup.push_back(row.cells[2].value);
            ofTotal.push_back(row.cells[3].value);
        }
        if (row.kind == rpt::RowKind::GroupFooter) {
            footers.push_back(row.cells[2].value);
            footerShare.push_back(row.cells[3].value);
        }
    }
    // MR001: S-1003 2,000.00 and S-1005 800.00 (2,800.00); MR002: S-1004 1,000.00. Of 3,800.00 in all.
    CHECK(ofGroup == std::vector<std::int64_t>{7143, 2857, 10000});
    CHECK(ofTotal == std::vector<std::int64_t>{5263, 2105, 2632});
    CHECK(footers == std::vector<std::int64_t>{10000, 10000});
    CHECK(footerShare == std::vector<std::int64_t>{7368, 2632});
    const rpt::Row* gt = lastOf(g, rpt::RowKind::Total);
    CHECK(gt->cells[2].type == rpt::CellType::Empty);   // a share of each group: nothing in the grand total
    CHECK(gt->cells[3].value == 10000);

    // Problems: a calculation on text; a missing second field; too many questions.
    ReportDefinition bad = parsed("writer 1\nname \"Bad calc\"\nsource ar_open_items 1\ncolumn calc diff customer owed\n");
    CHECK(ColumnProblem(open, bad.columns[0]).find("both fields must be amounts") != std::string::npos);
    auto p = CheckDefinition(db.code, bad);
    REQUIRE(p);
    CHECK(p->setting == DefinitionSetting::Columns);
    const rpt::Report noted = built(db.code, ctx, "writer 1\nname N\nsource ar_open_items 1\ncolumn voucher\n"
                                                  "column calc diff customer owed\ncolumn calc sum owed colour\n");
    CHECK(noted.columns.size() == 1);
    CHECK(rowsOf(noted, rpt::RowKind::Note) == 2);
    CHECK(MissingFields(parsed("writer 1\nname M\nsource ar_open_items 1\ncolumn calc sum owed colour\n")) ==
          std::vector<std::string>{"colour"});
    std::string many = "writer 1\nname Many\nsource ar_open_items 1\ncolumn owed\n";
    for (int i = 0; i < 5; ++i) many += "filter owed between ? ?\n";
    p = CheckDefinition(db.code, parsed(many));
    REQUIRE(p);
    CHECK(p->text == "A report can ask at most 4 questions when it runs.");
}

TEST_CASE("An asked range no row matches still lays out a page with its note (S7-M3)", "[reportwriter]")
{
    FixtureDb db;
    EnsureArTables(db.code);
    const ReportContext ctx = LoadReportContext(db.code, "20261110");
    const rpt::Report r = built(db.code, ctx,
                                "writer 1\nname \"Sales for periods\"\nsource sales_lines 1\ncolumn invoice\n"
                                "column sales total\ncolumn calc pct sales total total\nfilter period between ? ? \"Periods\"\n",
                                {{0, {"1", "1"}}});
    CHECK(rowsOf(r, rpt::RowKind::Detail) == 0);
    REQUIRE(lastOf(r, rpt::RowKind::Note));
    const rpt::FixedMeasure m;
    const std::vector<rpt::Page> pages = rpt::Layout(r, rpt::PageSetup::A4(), m);
    REQUIRE(pages.size() == 1);
    const std::string text = rpt::RenderText(pages, rpt::PageSetup::A4(), m);
    CHECK(text.find("No rows match") != std::string::npos);
    CHECK(text.find("Sales for periods") != std::string::npos);
}

TEST_CASE("The other sources agree with their modules' reports (S7-M4, P2)", "[reportwriter]")
{
    FixtureDb db;
    EnsureArTables(db.code);
    EnsureApTables(db.code);
    const ReportContext ctx = LoadReportContext(db.code, "20261110");
    auto rowsOfSource = [&](const char* key) {
        const Source* s = FindSource(key);
        REQUIRE(s);
        return s->rows(db.code, ctx);
    };
    auto col = [](const char* source, const char* field) {
        const int i = FindSource(source)->fieldIndex(field);
        REQUIRE(i >= 0);
        return static_cast<size_t>(i);
    };
    auto sum = [&](const char* source, const char* field) {
        std::int64_t total = 0;
        for (const SourceRow& r : rowsOfSource(source)) total += r[col(source, field)].number;
        return total;
    };
    auto grand = [](const rpt::Report& r, size_t cell) {
        const rpt::Row* t = lastOf(r, rpt::RowKind::Total);
        REQUIRE(t);
        return t->cells[cell].value;
    };

    // Every source builds with every field as a column.
    for (const Source& s : Sources()) {
        INFO(s.key);
        ReportDefinition d;
        d.name = d.title = s.name;
        d.source = s.key;
        d.sourceVersion = s.version;
        for (const Field& f : s.fields) d.columns.push_back({f.key});
        CHECK(BuildReport(db.code, ctx, d).report);
        CHECK_FALSE(s.rows(db.code, ctx).empty());
    }

    // Accounts: the Trial Balance at the current period.
    const rpt::Report tb = TrialBalance(db.code, ctx, {});
    std::int64_t debit = 0, credit = 0;
    for (const SourceRow& r : rowsOfSource("accounts")) {
        const std::int64_t b = r[col("accounts", "balance")].number;
        (b > 0 ? debit : credit) += b > 0 ? b : -b;
        std::int64_t periods = 0;
        for (int p = 1; p <= 12; ++p) periods += r[col("accounts", ("period_" + std::to_string(p)).c_str())].number;
        CHECK(r[col("accounts", "year_balance")].number == r[col("accounts", "opening")].number + periods);
    }
    CHECK(debit == grand(tb, 3));
    CHECK(credit == grand(tb, 4));

    // Posted ledger lines: gl_dist, all of it.
    cb4sqlite::Query q(db.code, "SELECT count(*), sum(debit), sum(credit) FROM gl_dist WHERE _deleted = 0");
    REQUIRE(q.next());
    CHECK(static_cast<std::int64_t>(rowsOfSource("posted_lines").size()) == q.integer(0));
    CHECK(sum("posted_lines", "debit") == rpt::ToCents(q.number(1)));
    CHECK(sum("posted_lines", "credit") == rpt::ToCents(q.number(2)));

    // Fixed assets: the Fixed Asset Summary totals the assets in use.
    const rpt::Report fa = FixedAssetListing(db.code, ctx);
    std::int64_t cost = 0, book = 0;
    for (const SourceRow& r : rowsOfSource("fixed_assets")) {
        CHECK(r[col("fixed_assets", "book_value")].number ==
              r[col("fixed_assets", "cost")].number - r[col("fixed_assets", "accumulated")].number);
        if (r[col("fixed_assets", "state")].text != "In use") continue;
        cost += r[col("fixed_assets", "cost")].number;
        book += r[col("fixed_assets", "book_value")].number;
    }
    CHECK(cost == grand(fa, 6));
    CHECK(book == grand(fa, 10));
    CHECK(rowsOfSource("fixed_assets").size() == LoadAssets(db.code).size());

    // Vendors and customers: the lists.
    CHECK(rowsOfSource("vendors").size() == LoadVendors(db.code).size());
    CHECK(rowsOfSource("customers").size() == LoadCustomers(db.code).size());
    CHECK(rowsOfSource("customers")[0][col("customers", "sales_account")].text ==
          TrimText(LoadCustomers(db.code)[0].sales.account) +
              (TrimText(LoadCustomers(db.code)[0].sales.dept).empty() ? "" : "-" + TrimText(LoadCustomers(db.code)[0].sales.dept)));

    // AP open items: what the Detailed Vendor Open Payables says is owed.
    const rpt::Report payables = ApOpenPayables(db.code, ctx);
    if (lastOf(payables, rpt::RowKind::Total)) CHECK(sum("ap_open_items", "owed") == grand(payables, 5));
    else CHECK(sum("ap_open_items", "owed") == 0);
    CHECK(rowsOfSource("ap_open_items").size() == LoadOpenItems(db.code).size());

    // AR receipts: the Receipts Register.
    const rpt::Report receipts = ArReceiptsRegister(db.code, ctx);
    CHECK(sum("ar_receipts", "amount") == grand(receipts, 6));
    CHECK(sum("ar_receipts", "discount") == grand(receipts, 7));

    // Purchase lines: the expense lines of both kinds of voucher, as LoadApLines gives them.
    std::int64_t lines = 0, expense = 0;
    for (ApKind kind : {ApKind::Invoice, ApKind::Cash})
        for (const ApVoucher& v : LoadApVouchers(db.code, kind))
            for (const ApLine& l : LoadApLines(db.code, kind, v.number))
                if (l.role == ApLineRole::Expense) {
                    ++lines;
                    expense += l.debit - l.credit;
                }
    CHECK(static_cast<std::int64_t>(rowsOfSource("purchase_lines").size()) == lines);
    CHECK(sum("purchase_lines", "expense") == expense);

    // The master lists.
    for (const auto& [key, table] : std::vector<std::pair<const char*, const char*>>{
             {"med_reps", "gl_medrp"}, {"products", "gl_itemm"}, {"doctors", "gl_drcod"}}) {
        INFO(key);
        cb4sqlite::Query n(db.code, std::string("SELECT count(*) FROM ") + table + " WHERE _deleted = 0");
        REQUIRE(n.next());
        CHECK(static_cast<std::int64_t>(rowsOfSource(key).size()) == n.integer(0));
    }
}

TEST_CASE("Purchase lines read the original's cancellation without migrating it (S7-M4)", "[reportwriter]")
{
    FixtureDb db;   // no EnsureApTables: voucher 5 is still MODE "2", "Cancelled"
    const ReportContext ctx = LoadReportContext(db.code, "20261110");
    const Source& s = *FindSource("purchase_lines");
    const size_t voucher = static_cast<size_t>(s.fieldIndex("voucher")), state = static_cast<size_t>(s.fieldIndex("state"));
    int five = 0;
    for (const SourceRow& r : s.rows(db.code, ctx))
        if (r[voucher].number == 5) {
            ++five;
            CHECK(r[state].text == "Cancelled");
        }
    CHECK(five > 0);
    cb4sqlite::Query q(db.code, "SELECT trim(mode) FROM ap_btch WHERE batchno = 5 AND _deleted = 0");
    REQUIRE(q.next());
    CHECK(q.text(0) == "2");   // left as it was
}

TEST_CASE("Export and import: round trip, refusals, a taken name (S7-M5)", "[reportwriter]")
{
    // Exported from one copy of the fixture...
    FixtureDb from;
    EnsureArTables(from.code);
    const ReportContext ctxFrom = LoadReportContext(from.code, "20261110");
    std::string exported;
    for (const StoredDefinition& s : LoadDefinitions(from.code))
        if (s.name == "Sales by product") exported = s.text;   // what Export writes
    REQUIRE_FALSE(exported.empty());
    const BuildResult before = BuildReport(from.code, ctxFrom, parsed(exported));
    REQUIRE(before.report);

    // ...imported into another, whose report of that name is gone.
    FixtureDb to;
    EnsureArTables(to.code);
    const ReportContext ctxTo = LoadReportContext(to.code, "20261110");
    for (const StoredDefinition& s : LoadDefinitions(to.code))
        if (s.name == "Sales by product") REQUIRE(DeleteDefinition(to.code, s.number));
    ImportResult r = ImportDefinition(to.code, "\xEF\xBB\xBF" + exported, "CLERK", "20261114120000");
    INFO(r.refusal);
    REQUIRE(r.number > 0);
    CHECK(r.name == "Sales by product");
    CHECK_FALSE(r.renamed);
    const auto stored = FindDefinition(to.code, r.number);
    REQUIRE(stored);
    CHECK(stored->text == exported);
    CHECK(stored->madeBy == "CLERK");
    const BuildResult after = BuildReport(to.code, ctxTo, parsed(stored->text));
    REQUIRE(after.report);
    CHECK(lastOf(*after.report, rpt::RowKind::Total)->cells[3].value ==
          lastOf(*before.report, rpt::RowKind::Total)->cells[3].value);   // the same figures
    CHECK(rpt::ToCsv(*after.report) == rpt::ToCsv(*before.report));

    // Imported again: the name is taken, so it's numbered, with the title following.
    r = ImportDefinition(to.code, exported, "", "20261114120001");
    REQUIRE(r.number > 0);
    CHECK(r.name == "Sales by product (2)");
    CHECK(r.renamed);
    CHECK(parsed(FindDefinition(to.code, r.number)->text).title == "Sales by product (2)");

    // Refusals save nothing.
    const size_t count = LoadDefinitions(to.code).size();
    r = ImportDefinition(to.code, "hello", "", "x");
    CHECK(r.number == 0);
    CHECK(r.refusal.find("isn't a report") != std::string::npos);
    r = ImportDefinition(to.code, "writer 2\nname \"Later\"\nsource sales_lines 1\ncolumn invoice\n", "", "x");
    CHECK(r.refusal.find("newer version of the program") != std::string::npos);
    r = ImportDefinition(to.code, "writer 1\nname \"Elsewhere\"\nsource payroll 1\ncolumn pay\n", "", "x");
    CHECK(r.refusal == "The report reads from “payroll”, a source this program doesn't have.");
    r = ImportDefinition(to.code, "writer 1\nname \"Newer\"\nsource sales_lines 9\ncolumn invoice\n", "", "x");
    CHECK(r.refusal.find("newer version of the “Sales lines” source") != std::string::npos);
    r = ImportDefinition(to.code, "writer 1\nname \"Empty\"\nsource sales_lines 1\n", "", "x");
    CHECK_FALSE(r.refusal.empty());   // CheckDefinition: no column
    CHECK(LoadDefinitions(to.code).size() == count);
}

TEST_CASE("The Report Writer right: added once, last; who may design (S7-M5)", "[reportwriter]")
{
    FixtureDb db;
    auto rows = [&] {
        std::vector<std::pair<long, long>> out;   // COUNT, ID in "menu" order
        cb4sqlite::Query q(db.code, "SELECT \"COUNT\", id FROM accessm WHERE _deleted = 0 ORDER BY ifnull(\"COUNT\", 0), _rowid");
        while (q.next()) out.push_back({static_cast<long>(q.integer(0)), static_cast<long>(q.integer(1))});
        return out;
    };
    auto code = [&](const char* level) {
        cb4sqlite::Query q(db.code, "SELECT trim(menucode) FROM accessl WHERE trim(level) = ?1");
        q.bindText(1, level);
        REQUIRE(q.next());
        return q.text(0);
    };

    // The fixture has it, last; the clerk (level B) is denied it, the administrator isn't.
    auto all = rows();
    REQUIRE_FALSE(all.empty());
    CHECK(all.back().second == kDesignRightId);
    CHECK_FALSE(EnsureReportWriterAccess(db.code));   // there already
    CHECK(rows() == all);
    CHECK(MayDesignReports(db.code, code("A")));
    CHECK_FALSE(MayDesignReports(db.code, code("B")));
    CHECK(MayDesignReports(db.code, ""));   // no login: nothing is greyed

    // A company that predates it: removed, then added back last, once, with
    // the existing codes' places unchanged (plan P5).
    {
        cb4sqlite::Transaction tx(db.code);
        Data4 m(&db.code, "accessm");
        Field4 id(m, "id");
        for (m.top(); !m.eof(); m.skip())
            if (std::atol(id.str()) == kDesignRightId) m.deleteRec();
        m.pack();
        m.close();
        tx.commit();
    }
    const auto without = rows();
    REQUIRE(without.size() == all.size() - 1);
    CHECK(MayDesignReports(db.code, code("B")));   // no row: no right to deny
    EnsureReportWriterTables(db.code);
    const auto with = rows();
    REQUIRE(with.size() == without.size() + 1);
    CHECK(std::equal(without.begin(), without.end(), with.begin()));
    CHECK(with.back().second == kDesignRightId);
    CHECK(with.back().first == without.back().first + 1);
    CHECK_FALSE(EnsureReportWriterAccess(db.code));
    CHECK(rows().size() == with.size());
    // A code that doesn't reach the new row grants it; '1' at its place denies it.
    const std::string shorter(with.size() - 1, '0');
    CHECK(MayDesignReports(db.code, shorter));
    CHECK_FALSE(MayDesignReports(db.code, shorter + "1"));
}

TEST_CASE("The row limit names the questions to narrow; CSV of a grouped report (S7-M5)", "[reportwriter]")
{
    FixtureDb db;
    EnsureArTables(db.code);
    const ReportContext ctx = LoadReportContext(db.code, "20261110");
    const std::string asking = "writer 1\nname \"GL\"\nsource gl_lines 1\ncolumn voucher\ncolumn debit total\n"
                               "group account\nfilter period between ? ? \"Periods\"\n";
    BuildResult b = BuildReport(db.code, ctx, parsed(asking), {{0, {"1", "12"}}}, 2);
    CHECK_FALSE(b.report);
    CHECK(b.refusal == "The report has more than 2 rows, too many to lay out. Answer its questions (Periods) more "
                       "narrowly and run it again.");
    b = BuildReport(db.code, ctx, parsed("writer 1\nname \"GL\"\nsource gl_lines 1\ncolumn voucher\n"), {}, 20000);
    CHECK(b.report);
    b = BuildReport(db.code, ctx, parsed("writer 1\nname \"GL\"\nsource gl_lines 1\ncolumn voucher\n"), {}, 2);
    CHECK(b.refusal.find("Narrow its filters") != std::string::npos);
    CHECK(BuildReport(db.code, ctx, parsed("writer 1\nname \"GL\"\nsource gl_lines 1\ncolumn voucher\n"), {}, 1)
              .refusal.find("more than 1 rows") != std::string::npos);

    // The grouped report as CSV: a line per header, detail, footer and total.
    const rpt::Report r = built(db.code, ctx, asking, {{0, {"7", "7"}}});
    const std::string csv = rpt::ToCsv(r);
    CHECK(csv.rfind("\xEF\xBB\xBFVoucher,Debit\r\n", 0) == 0);
    size_t lines = 0;
    for (size_t at = 0; (at = csv.find("\r\n", at)) != std::string::npos; at += 2) ++lines;
    size_t printed = 1;   // the headers
    for (const rpt::Row& row : r.rows)
        printed += row.kind == rpt::RowKind::Blank || row.kind == rpt::RowKind::PageBreak ? 0 : 1;
    CHECK(lines == printed);
    CHECK(csv.find("\r\nAccount: 101000") != std::string::npos);
    CHECK(csv.find("\r\nTotal,1575.00\r\n") != std::string::npos);
}
