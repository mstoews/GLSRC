//
// report_tests: a golden rendering (spec #3 11, plan S3-M0 exit criteria).
//
// A three-page statement-style report on the small test page (60 x 20
// characters), rendered with TextRender. It pins the page header (printed-on
// date and page numbers beside the company), headings, indents, parentheses
// for negatives, subtotal rules, the final total's double rule kept with the
// row before it, a blank row, a note, and the footer note.
//
// If a deliberate layout change alters it, check the new rendering by eye,
// then paste it in as the expected text.
//
#include "catch_amalgamated.hpp"

#include "report/Layout.h"
#include "report/TextRender.h"

#include <string>

using namespace rpt;

namespace {

Report sample()
{
    Report r;
    r.company = "Test Company";
    r.title = "Income Statement";
    r.subtitles = {"For the month and year to date ended October 31, 2026"};
    r.printedOn = "11/09/2026";
    r.footerNote = "Income Statement - Layout: default";
    r.negatives = NegativeStyle::Parentheses;
    r.columns = {{"", 3}, {"Month", 1, CellType::Amount}, {"Year to date", 1.2, CellType::Amount}};
    auto line = [&](const char* cap, long long m, long long y, int ind = 1) {
        r.rows.push_back(Row::Detail({Cell::Text(cap), Cell::Amount(m), Cell::Amount(y)}, ind));
    };
    r.rows.push_back(Row::Heading("Revenue"));
    line("Gross sales", 115000, 980000);
    line("Sales returns", -2000, -15000);
    r.rows.push_back(Row::Subtotal({Cell::Text("Net sales"), Cell::Amount(113000), Cell::Amount(965000)}));
    line("Cost of sales", 60000, 510000, 0);
    r.rows.push_back(Row::Subtotal({Cell::Text("Gross profit"), Cell::Amount(53000), Cell::Amount(455000)}));
    r.rows.push_back(Row::Blank());
    r.rows.push_back(Row::Heading("Operating expenses"));
    for (int i = 0; i < 22; ++i)
        line(("Expense line " + std::to_string(i + 1)).c_str(), 1000 * (i + 1), 7000 * (i + 1));
    r.rows.push_back(
        Row::Subtotal({Cell::Text("Total operating expenses"), Cell::Amount(253000), Cell::Amount(1771000)}));
    r.rows.push_back(Row::Total({Cell::Text("Net income"), Cell::Amount(-200000), Cell::Amount(-1316000)}, true));
    r.rows.push_back(Row::Note("Unclassified accounts: 999999 is on no line of the layout."));
    return r;
}

} // namespace

TEST_CASE("golden: a three-page statement", "[layout][golden]")
{
    PageSetup s;
    s.width = 7000;
    s.height = 5000;
    s.left = s.top = s.right = s.bottom = 500;
    const FixedMeasure m;
    const std::string expected = R"(--- page 1 ---
Printed on 11/09/2026   Test Company            Page 1 of 3
                      Income Statement
   For the month and year to date ended October 31, 2026
                                       Month   Year to date
------------------------------------------------------------
Revenue
  Gross sales                       1,150.00       9,800.00
  Sales returns                      (20.00)       (150.00)
                                  ----------- --------------
Net sales                           1,130.00       9,650.00
Cost of sales                         600.00       5,100.00
                                  ----------- --------------
Gross profit                          530.00       4,550.00

Operating expenses
  Expense line 1                       10.00          70.00
  Expense line 2                       20.00         140.00
  Expense line 3                       30.00         210.00
  Expense line 4                       40.00         280.00
  Expense line 5                       50.00         350.00
Income Statement - Layout: default
--- page 2 ---
Printed on 11/09/2026   Test Company            Page 2 of 3
                      Income Statement
   For the month and year to date ended October 31, 2026
                                       Month   Year to date
------------------------------------------------------------
  Expense line 6                       60.00         420.00
  Expense line 7                       70.00         490.00
  Expense line 8                       80.00         560.00
  Expense line 9                       90.00         630.00
  Expense line 10                     100.00         700.00
  Expense line 11                     110.00         770.00
  Expense line 12                     120.00         840.00
  Expense line 13                     130.00         910.00
  Expense line 14                     140.00         980.00
  Expense line 15                     150.00       1,050.00
  Expense line 16                     160.00       1,120.00
  Expense line 17                     170.00       1,190.00
  Expense line 18                     180.00       1,260.00

Income Statement - Layout: default
--- page 3 ---
Printed on 11/09/2026   Test Company            Page 3 of 3
                      Income Statement
   For the month and year to date ended October 31, 2026
                                       Month   Year to date
------------------------------------------------------------
  Expense line 19                     190.00       1,330.00
  Expense line 20                     200.00       1,400.00
  Expense line 21                     210.00       1,470.00
  Expense line 22                     220.00       1,540.00
                                  ----------- --------------
Total operating expenses            2,530.00      17,710.00
                                  ----------- --------------
Net income                        (2,000.00)    (13,160.00)
                                  =========== ==============
Unclassified accounts: 999999 is on no line of the layout.






Income Statement - Layout: default
)";
    CHECK(RenderText(Layout(sample(), s, m), s, m) == expected);
}
