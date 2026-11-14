//
// gl_tests: the report writer's core (spec #7 4, 5, 7, D2-D4, D9; plan S7-M0)
// on the dev fixture. Each source is checked against its owning module's own
// figures (plan P2): sales lines against Ar.h's voucher lines, GL lines
// against gl_btchd, AR open items against LoadArOpenItems.
//
#include "GlTestDb.h"

#include "Accounts.h"   // TrimText
#include "Ar.h"
#include "ReportWriter.h"

#include "report/Format.h"   // rpt::ToCents

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
    REQUIRE(Sources().size() == 3);
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
    CHECK(LoadDefinitions(db.code).empty());   // no table yet
    const ReportDefinition sales = parsed(kSalesByProduct);
    const long one = SaveDefinition(db.code, 0, sales, "ADMIN", "20261114093000");
    CHECK(one == 1);
    ReportDefinition other = parsed("writer 1\nname \"AR by customer\"\nsource ar_open_items 1\ncolumn owed total\n");
    const long two = SaveDefinition(db.code, 0, other, "CLERK", "20261114100000");
    CHECK(two == 2);

    auto all = LoadDefinitions(db.code);
    REQUIRE(all.size() == 2);
    CHECK(all[0].name == "AR by customer");   // name order
    CHECK(all[1].name == "Sales by product");
    CHECK(all[1].description == "Net sales and VAT by product, posted vouchers");
    CHECK(all[1].source == "sales_lines");
    CHECK(all[1].sourceVersion == 1);
    CHECK(all[1].madeBy == "ADMIN");
    CHECK(all[1].madeOn == "20261114093000");
    CHECK(parsed(all[1].text).columns.size() == 4);
    CHECK(WriteDefinition(parsed(all[1].text)) == WriteDefinition(sales));

    // Replace 2: the same row, changed by another, made by the first.
    other.columns.push_back({"customer", "", 0, false, false});
    CHECK(SaveDefinition(db.code, two, other, "ADMIN", "20261114110000") == two);
    const auto again = FindDefinition(db.code, two);
    REQUIRE(again);
    CHECK(again->madeBy == "CLERK");
    CHECK(again->changedBy == "ADMIN");
    CHECK(again->changedOn == "20261114110000");
    CHECK(parsed(again->text).columns.size() == 2);
    CHECK(LoadDefinitions(db.code).size() == 2);

    CHECK(DefinitionNameTaken(db.code, "  sales BY product "));
    CHECK_FALSE(DefinitionNameTaken(db.code, "Sales by product", one));
    CHECK_FALSE(DefinitionNameTaken(db.code, "Something else"));

    // A stored definition builds.
    EnsureArTables(db.code);
    const ReportContext ctx = LoadReportContext(db.code, "20261110");
    CHECK(BuildReport(db.code, ctx, parsed(FindDefinition(db.code, one)->text)).report);

    CHECK(DeleteDefinition(db.code, one));
    CHECK_FALSE(DeleteDefinition(db.code, one));
    CHECK_FALSE(FindDefinition(db.code, one));
    CHECK(LoadDefinitions(db.code).size() == 1);
    CHECK(SaveDefinition(db.code, 0, sales, "ADMIN", "20261114120000") == 3);   // the highest + 1
}
