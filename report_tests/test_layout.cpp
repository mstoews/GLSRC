//
// report_tests: layout and pagination (spec #3 4.2).
//
// Most cases use a small page: 7000 x 5000 twips with 500 margins, so with
// FixedMeasure (100 x 200) the printable area is 60 characters by 20 lines.
// With one subtitle and a one-line column header the body starts at
// y = 1500 and, with a footer note, ends at 4200: 13 detail lines a page.
//
#include "catch_amalgamated.hpp"

#include "report/Layout.h"
#include "report/TextRender.h"

#include <algorithm>
#include <string>
#include <vector>

using namespace rpt;

namespace {

PageSetup small()
{
    PageSetup s;
    s.width = 7000;
    s.height = 5000;
    s.left = s.top = s.right = s.bottom = 500;
    return s;
}

Report list(int details)
{
    Report r;
    r.company = "Test Company";
    r.title = "Account List";
    r.subtitles = {"As at 10/31/2026"};
    r.footerNote = "test";
    r.columns = {{"Account", 1}, {"Description", 3}, {"Balance", 2, CellType::Amount}};
    for (int i = 0; i < details; ++i)
        r.rows.push_back(Row::Detail({Cell::Text("A" + std::to_string(100 + i)), Cell::Text("Line"),
                                      Cell::Amount(100 * (i + 1))}));
    return r;
}

bool has(const Page& p, const std::string& text)
{
    return std::any_of(p.texts.begin(), p.texts.end(), [&](const TextOp& t) { return t.text == text; });
}

int yOf(const Page& p, const std::string& text)
{
    for (const TextOp& t : p.texts)
        if (t.text == text) return t.y;
    return -1;
}

const FixedMeasure kMeasure;

} // namespace

TEST_CASE("details fill pages; every page repeats the header and numbers itself", "[layout]")
{
    const auto pages = Layout(list(30), small(), kMeasure);
    REQUIRE(pages.size() == 3);
    for (size_t p = 0; p < pages.size(); ++p) {
        CHECK(has(pages[p], "Test Company"));
        CHECK(has(pages[p], "Account List"));
        CHECK(has(pages[p], "As at 10/31/2026"));
        CHECK(has(pages[p], "Account"));
        CHECK(has(pages[p], "Balance"));
        CHECK(has(pages[p], "test"));
        CHECK(has(pages[p], "Page " + std::to_string(p + 1) + " of 3"));
    }
    CHECK(has(pages[0], "A100"));
    CHECK(has(pages[0], "A112"));    // 13 on the first page
    CHECK(has(pages[1], "A113"));
    CHECK(has(pages[2], "A129"));
    CHECK(yOf(pages[0], "A100") == 1500);
    CHECK(yOf(pages[1], "A113") == 1500);
}

TEST_CASE("a heading is never the last line of a page", "[layout]")
{
    Report r = list(12);                      // lines 1-12 of page 1
    r.rows.push_back(Row::Heading("Liabilities"));
    r.rows.push_back(Row::Detail({Cell::Text("B1")}));
    const auto pages = Layout(r, small(), kMeasure);
    REQUIRE(pages.size() == 2);
    CHECK(has(pages[1], "Liabilities"));
    CHECK(yOf(pages[1], "Liabilities") == 1500);
    CHECK(has(pages[1], "B1"));
}

TEST_CASE("a total stays with the row before it", "[layout]")
{
    Report r = list(13);                      // exactly one page of details
    r.rows.push_back(Row::Total({Cell::Text("Total"), Cell::Empty(), Cell::Amount(9100)}, true));
    const auto pages = Layout(r, small(), kMeasure);
    REQUIRE(pages.size() == 2);
    CHECK_FALSE(has(pages[0], "A112"));       // moved down with the total
    CHECK(has(pages[1], "A112"));
    CHECK(has(pages[1], "Total"));
    CHECK(has(pages[1], "91.00"));
    // A rule above the amount and a double rule below it.
    CHECK(pages[1].rules.size() == 1 + 3);    // header rule + single + double
}

TEST_CASE("page breaks, and blanks at the top of a page", "[layout]")
{
    Report r = list(2);
    r.rows.push_back(Row::PageBreak());
    r.rows.push_back(Row::Blank());
    r.rows.push_back(Row::Detail({Cell::Text("C1")}));
    r.rows.push_back(Row::PageBreak());
    r.rows.push_back(Row::PageBreak());       // a second break doesn't make an empty page
    r.rows.push_back(Row::Detail({Cell::Text("C2")}));
    const auto pages = Layout(r, small(), kMeasure);
    REQUIRE(pages.size() == 3);
    CHECK(yOf(pages[1], "C1") == 1500);       // the blank was dropped
    CHECK(has(pages[2], "C2"));
}

TEST_CASE("text is cut to its column; numbers that don't fit print ###", "[layout]")
{
    Report r;
    // 6000 - 144 gap = 5856: the code column gets 4392 twips (43 characters), the amount 1464 (14).
    r.columns = {{"Code", 3}, {"Amount", 1, CellType::Amount}};
    r.rows.push_back(Row::Detail({Cell::Text(std::string(50, 'x')), Cell::Amount(123456789012345)}));
    r.rows.push_back(Row::Detail({Cell::Text("short"), Cell::Amount(-100)}, 2));
    const auto pages = Layout(r, small(), kMeasure);
    REQUIRE(pages.size() == 1);
    const TextOp* cut = nullptr;
    for (const TextOp& t : pages[0].texts)
        if (!t.text.empty() && t.text[0] == 'x') cut = &t;
    REQUIRE(cut);
    CHECK(cut->text.size() == 43);
    CHECK(has(pages[0], "###"));              // "1,234,567,890,123.45" is 20 characters
    CHECK(has(pages[0], "-1.00"));
    for (const TextOp& t : pages[0].texts)    // two indent levels move the first column
        if (t.text == "short") {
            CHECK(t.x == 500 + 2 * kIndentStep);
            CHECK(t.width == 4392 - 2 * kIndentStep);
        }
}

TEST_CASE("dates that don't fit print ###, never a shorter date", "[layout]")
{
    Report r;
    // 5856 twips over 1 + 9: the first column gets 585 (5 characters), so
    // "10/15/2026" (10) doesn't fit; in a 10-character column it does.
    r.columns = {{"Date", 1}, {"Text", 9}};
    r.rows.push_back(Row::Detail({Cell::Date("20261015"), Cell::Text("narrow")}));
    auto pages = Layout(r, small(), kMeasure);
    REQUIRE(pages.size() == 1);
    CHECK(has(pages[0], "###"));
    CHECK_FALSE(has(pages[0], "10/15"));

    r.columns = {{"Date", 2}, {"Text", 3}};   // 2342 twips: 23 characters
    pages = Layout(r, small(), kMeasure);
    CHECK(has(pages[0], "10/15/2026"));
}

TEST_CASE("UTF-8 text is cut between code points", "[layout]")
{
    const std::string s = "\xC3\xA9t\xC3\xA9";   // "été", 3 code points
    CHECK(FitText(kMeasure, Role::Body, s, 300) == s);
    CHECK(FitText(kMeasure, Role::Body, s, 200) == "\xC3\xA9t");
    CHECK(FitText(kMeasure, Role::Body, s, 100) == "\xC3\xA9");
    CHECK(FitText(kMeasure, Role::Body, s, 50).empty());
}

TEST_CASE("notes wrap at spaces", "[layout]")
{
    Report r;
    r.rows.push_back(Row::Note("These accounts' balances don't agree with their period amounts: "
                               "101000, 102101, 111101, 144910, 148000, 221002."));
    const auto pages = Layout(r, small(), kMeasure);
    REQUIRE(pages.size() == 1);
    int lines = 0;
    for (const TextOp& t : pages[0].texts) {
        if (t.text.rfind("Page", 0) == 0) continue;
        CHECK(static_cast<int>(t.text.size()) <= 60);
        ++lines;
    }
    CHECK(lines == 2);   // 117 characters on 60-character lines
}

TEST_CASE("slip mode: each slip starts a page and long slips repeat the column headers", "[layout]")
{
    Report r;
    r.company = "Test Company";
    r.title = "Voucher Slips";
    r.slipMode = true;
    r.columns = {{"No.", 1}, {"Account", 2}, {"Debit", 2, CellType::Amount}};
    r.rows.push_back(Row::Slip({{"Slip Number", "100"}, {"Date", "10/15/2026"}, {"Reference", "Cash"}}));
    for (int i = 0; i < 20; ++i)
        r.rows.push_back(Row::Detail({Cell::Integer(i + 1), Cell::Text("L" + std::to_string(i)), Cell::Amount(100)}));
    r.rows.push_back(Row::GroupFooter({Cell::Text("Total"), Cell::Empty(), Cell::Amount(2000)}));
    r.rows.push_back(Row::Slip({{"Slip Number", "101"}}));
    r.rows.push_back(Row::Detail({Cell::Integer(1), Cell::Text("M0"), Cell::Amount(5)}));
    const auto pages = Layout(r, small(), kMeasure);
    REQUIRE(pages.size() == 3);
    CHECK(has(pages[0], "Slip Number: 100"));
    CHECK(has(pages[0], "Date: 10/15/2026"));
    CHECK(has(pages[0], "Account"));
    CHECK(has(pages[1], "Account"));          // the continuation repeats them
    CHECK_FALSE(has(pages[1], "Slip Number: 100"));
    CHECK(has(pages[1], "Total"));
    CHECK(has(pages[2], "Slip Number: 101"));
    CHECK(has(pages[2], "M0"));
}

TEST_CASE("printed-on and page number move above a long company name", "[layout]")
{
    Report r = list(1);
    r.printedOn = "11/09/2026";
    const auto beside = Layout(r, small(), kMeasure);
    CHECK(yOf(beside[0], "Printed on 11/09/2026") == yOf(beside[0], "Test Company"));
    CHECK(yOf(beside[0], "A100") == 1500);

    r.company = std::string(40, 'C');
    const auto above = Layout(r, small(), kMeasure);
    CHECK(yOf(above[0], "Printed on 11/09/2026") == 500);
    CHECK(yOf(above[0], "Page 1 of 1") == 500);
    CHECK(yOf(above[0], r.company) == 700);
    CHECK(yOf(above[0], "A100") == 1700);     // the header grew by a line
}

TEST_CASE("paper sizes", "[layout]")
{
    const PageSetup a4 = PageSetup::A4();
    CHECK(a4.width == 11906);
    CHECK(a4.height == 16838);
    CHECK(a4.printableWidth() == 11906 - 2160);
    const PageSetup land = PageSetup::Letter(Orientation::Landscape);
    CHECK(land.width == 15840);
    CHECK(land.height == 12240);
}

TEST_CASE("A footer's label runs on over empty columns instead of being cut (S7-M6)", "[layout]")
{
    Report r;
    r.title = "Spill";
    r.columns = {{"No.", 0.4, CellType::Integer}, {"Description", 2.0}, {"Debit", 1.0, CellType::Amount}};
    r.rows = {Row::Detail({Cell::Integer(100), Cell::Text("Transfer"), Cell::Amount(25000)}),
              Row::GroupFooter({Cell::Text("Total 101000 PETTY CASH"), {}, Cell::Amount(25000)}),
              Row::GroupFooter({Cell::Text("Total with a neighbour"), Cell::Text("x"), Cell::Amount(25000)}),
              Row::Total({Cell::Text("Total"), {}, Cell::Amount(25000)}, true)};
    const FixedMeasure m;
    const auto pages = Layout(r, PageSetup::A4(), m);
    REQUIRE(pages.size() == 1);
    bool whole = false, cut = false;
    for (const TextOp& t : pages[0].texts) {
        whole = whole || t.text == "Total 101000 PETTY CASH";
        cut = cut || (t.text.rfind("Total with", 0) == 0 && t.text != "Total with a neighbour");
    }
    CHECK(whole);   // the empty Description column took it
    CHECK(cut);     // a filled neighbour stops it: cut as before
}
