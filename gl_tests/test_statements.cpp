//
// gl_tests: the statement layout and F3-F5 (spec #3 5.4-5.6, 6; plan S3-M3)
// on the dev fixture, period 7 (Oct 26). Worked out by hand from
// make_dev_fixture.py's FINANCE_* journals:
//
// F3 Income Statement      Month   Year to date   Prior YTD   Budget YTD
//   Gross sales (411000)   6,000      16,000         63,000      70,000
//   Cost of sales          2,400       6,400         25,200      28,000
//   Gross profit           3,600       9,600         37,800      42,000
//   Personnel (731000)     2,000       5,500         22,400      24,500
//   General and admin.         0       1,200          8,400       8,400
//   Other operating            0         600          3,500       4,200
//   Total operating exp.   2,000       7,300         34,300      37,100
//   Operating income       1,600       2,300          3,500       4,900
//   Non-operating income       0          50            280         350
//   Income before tax      1,600       2,350          3,780       5,250
//   Income tax               900         900          4,900       5,600
//   Net income               700       1,450         (1,120)       (350)   variance 1,800
//   Department 10: personnel 2,000 / 4,000; net income (2,000) / (4,000).
//
// F4 Balance Sheet                  End of Oct 26   Start of year
//   Cash and bank (101000, 102101, 102201)  37,950     33,000
//   Total current assets                    46,460     41,510
//   Property and equipment                  35,000     30,000
//   Accumulated depreciation                (6,600)    (6,000)
//   Total assets                            74,860     65,510
//   Total liabilities                       12,670      4,670
//   Net income for the year to date          1,450          0
//   Total equity                            62,290     60,840
//   Unclassified accounts (999000)            (100)         0
//   Total liabilities and equity            74,860     65,510
//
// F5 Cash Flow Statement               Month   Year to date
//   Net income                           700      1,450
//   Depreciation                           0        600
//   Net cash from operating activities   700      2,050
//   Property and equipment                 0     (5,000)
//   Borrowings                             0      8,000
//   Unclassified accounts (999000)         0       (100)
//   Net increase in cash                 700      4,950
//   Cash at beginning                 37,250     33,000
//   Cash at end                       37,950     37,950   (F4's cash)
//

#include "GlTestDb.h"

#include "reports/Balances.h"
#include "reports/ReportContext.h"
#include "reports/StatementLayout.h"
#include "reports/Statements.h"

#include <algorithm>
#include <string>

using namespace gltest;

namespace {

ReportContext context(Code4& code) { return LoadReportContext(code, "20261110"); }

const rpt::Row* row(const rpt::Report& r, const std::string& caption)
{
    for (const rpt::Row& x : r.rows)
        if (!x.cells.empty() && x.cells[0].type == rpt::CellType::Text && x.cells[0].text == caption) return &x;
    return nullptr;
}

// The row's amount in column c (1-based among the amount columns), in cents.
std::int64_t amount(const rpt::Report& r, const std::string& caption, size_t c)
{
    const rpt::Row* x = row(r, caption);
    REQUIRE(x);
    REQUIRE(x->cells.size() > c);
    return x->cells[c].value;
}

bool hasNote(const rpt::Report& r, const std::string& text)
{
    return std::any_of(r.rows.begin(), r.rows.end(), [&](const rpt::Row& x) {
        return x.kind == rpt::RowKind::Note && x.text.find(text) != std::string::npos;
    });
}

bool hasHeading(const rpt::Report& r, const std::string& text)
{
    return std::any_of(r.rows.begin(), r.rows.end(),
                       [&](const rpt::Row& x) { return x.kind == rpt::RowKind::Heading && x.text == text; });
}

int problems(const std::vector<LayoutProblem>& ps, bool errors)
{
    return static_cast<int>(std::count_if(ps.begin(), ps.end(), [&](const LayoutProblem& p) { return p.error == errors; }));
}

bool problemContains(const std::vector<LayoutProblem>& ps, const std::string& text)
{
    return std::any_of(ps.begin(), ps.end(), [&](const LayoutProblem& p) { return p.text.find(text) != std::string::npos; });
}

} // namespace

// ---- the layout -------------------------------------------------------------

TEST_CASE("ParseFormula", "[layout]")
{
    using Terms = std::vector<std::pair<int, int>>;
    CHECK(ParseFormula("50-60") == Terms{{1, 50}, {-1, 60}});
    CHECK(ParseFormula(" 170 + 180 - 190 + 200 ") == Terms{{1, 170}, {1, 180}, {-1, 190}, {1, 200}});
    CHECK(ParseFormula("-20") == Terms{{-1, 20}});
    CHECK_FALSE(ParseFormula(""));
    CHECK_FALSE(ParseFormula("50-"));
    CHECK_FALSE(ParseFormula("50--60"));
    CHECK_FALSE(ParseFormula("50 60"));
    CHECK_FALSE(ParseFormula("50*2"));
}

TEST_CASE("EnsureLayoutTables seeds the default once", "[layout]")
{
    FixtureDb db;
    EnsureLayoutTables(db.code);
    EnsureLayoutTables(db.code);   // idempotent
    for (const char* s : {kIncomeStatement, kBalanceSheet, kCashFlow}) {
        const auto loaded = LoadLayout(db.code, s);
        const auto expected = DefaultLayout(s);
        REQUIRE(loaded.size() == expected.size());
        for (size_t i = 0; i < loaded.size(); ++i) {
            CHECK(loaded[i].lineNo == expected[i].lineNo);
            CHECK(loaded[i].kind == expected[i].kind);
            CHECK(loaded[i].caption == expected[i].caption);
            CHECK(loaded[i].sign == expected[i].sign);
            CHECK(loaded[i].formula == expected[i].formula);
            CHECK(loaded[i].ranges.size() == expected[i].ranges.size());
        }
        CHECK(LayoutDescription(loaded) == "Layout: default");
    }
    CHECK(DefaultLayout(kIncomeStatement).size() == 23);
    const auto other = LoadLayout(db.code, kIncomeStatement);
    CHECK(other[14].ranges.size() == 2);   // 150 Other operating expenses: 600000-699999, 760000-799999
}

TEST_CASE("SaveLayout replaces a statement's lines; ResetLayout restores the default", "[layout]")
{
    FixtureDb db;
    EnsureLayoutTables(db.code);
    auto lines = LoadLayout(db.code, kBalanceSheet);
    lines.back().caption = "Total liabilities and shareholders' equity";
    lines.pop_back();
    SaveLayout(db.code, kBalanceSheet, lines, "2026-11-11 10:00");
    auto again = LoadLayout(db.code, kBalanceSheet);
    CHECK(again.size() == DefaultLayout(kBalanceSheet).size() - 1);
    CHECK(LayoutDescription(again) == "Layout: modified 2026-11-11 10:00");
    CHECK(LoadLayout(db.code, kIncomeStatement).size() == 23);   // other statements untouched
    ResetLayout(db.code, kBalanceSheet);
    CHECK(LoadLayout(db.code, kBalanceSheet).size() == DefaultLayout(kBalanceSheet).size());
    CHECK(LayoutDescription(LoadLayout(db.code, kBalanceSheet)) == "Layout: default");
}

TEST_CASE("CheckLayout: the defaults on the fixture warn only about 999000", "[layout]")
{
    FixtureDb db;
    const auto accounts = LoadBalances(db.code);
    CHECK(CheckLayout(kIncomeStatement, DefaultLayout(kIncomeStatement), accounts).empty());
    for (const char* s : {kBalanceSheet, kCashFlow}) {
        const auto ps = CheckLayout(s, DefaultLayout(s), accounts);
        CHECK(problems(ps, true) == 0);
        REQUIRE(problems(ps, false) == 1);
        CHECK(problemContains(ps, "on no line"));
        CHECK(problemContains(ps, "999000"));
    }
}

TEST_CASE("CheckLayout: errors", "[layout]")
{
    const std::vector<AccountBalances> none;
    auto is = DefaultLayout(kIncomeStatement);
    is[4].formula = "20+30+400";           // 50: no line 400
    is[6].formula = "50-230";              // 70: a later line
    is[1].ranges = {{"41000", "419999"}};   // not 6 digits
    is[2].ranges = {{"429999", "420000"}};  // backwards
    is[3].ranges.clear();                  // an A line without ranges
    is[22].formula.clear();                // a T without a formula
    is[8].lineNo = 100;                    // duplicate (90 -> 100)
    is[9].kind = 'X';
    const auto ps = CheckLayout(kIncomeStatement, is, none);
    CHECK(problems(ps, false) == 0);
    CHECK(problemContains(ps, "line 400"));
    CHECK(problemContains(ps, "line 230"));
    CHECK(problemContains(ps, "41000-419999"));
    CHECK(problemContains(ps, "runs backwards"));
    CHECK(problemContains(ps, "no account ranges"));
    CHECK(problemContains(ps, "needs a formula"));
    CHECK(problemContains(ps, "100 appears twice"));
    CHECK(problemContains(ps, "\"X\""));

    auto cf = DefaultLayout(kCashFlow);
    cf.back().ranges = {{"100000", "102999"}};   // E differs from O
    CHECK(problemContains(CheckLayout(kCashFlow, cf, none), "different ranges"));
    cf.pop_back();                               // no E
    CHECK(problemContains(CheckLayout(kCashFlow, cf, none), "exactly one"));
}

TEST_CASE("CheckLayout: accounts on two lines, and cash on a CF account line", "[layout]")
{
    FixtureDb db;
    const auto accounts = LoadBalances(db.code);
    auto bs = DefaultLayout(kBalanceSheet);
    bs[2].ranges.push_back({"102000", "102999"});   // Receivables also takes 102xxx
    auto ps = CheckLayout(kBalanceSheet, bs, accounts);
    CHECK(problemContains(ps, "more than one line"));
    CHECK(problemContains(ps, "102101"));

    auto cf = DefaultLayout(kCashFlow);
    cf[2].ranges.push_back({"101000", "101999"});   // Depreciation also takes petty cash
    ps = CheckLayout(kCashFlow, cf, accounts);
    CHECK(problemContains(ps, "cash accounts are also on an account line"));
    CHECK(problemContains(ps, "101000"));
}

// ---- F3 ---------------------------------------------------------------------

TEST_CASE("F3 Income Statement: month and year to date", "[statements][f3]")
{
    FixtureDb db;
    const rpt::Report r = IncomeStatement(db.code, context(db.code), {});
    CHECK(r.title == "Income Statement");
    REQUIRE(r.subtitles.size() == 1);
    CHECK(r.subtitles[0] == "For the month and year to date ended October 31, 2026");
    CHECK(r.footerNote == "Income Statement - Layout: default");
    CHECK(r.negatives == rpt::NegativeStyle::Parentheses);
    CHECK(r.orientation == rpt::Orientation::Portrait);
    REQUIRE(r.columns.size() == 3);
    CHECK(r.columns[1].header2 == "Oct 26");

    CHECK(amount(r, "Gross sales", 1) == 600000);
    CHECK(amount(r, "Gross sales", 2) == 1600000);
    CHECK(amount(r, "Cost of sales", 2) == 640000);
    CHECK(amount(r, "Gross profit", 1) == 360000);
    CHECK(amount(r, "Gross profit", 2) == 960000);
    CHECK(amount(r, "Personnel", 2) == 550000);
    CHECK(amount(r, "General and administrative", 2) == 120000);
    CHECK(amount(r, "Other operating expenses", 2) == 60000);
    CHECK(amount(r, "Total operating expenses", 1) == 200000);
    CHECK(amount(r, "Total operating expenses", 2) == 730000);
    CHECK(amount(r, "Operating income", 2) == 230000);
    CHECK(amount(r, "Non-operating income", 2) == 5000);
    CHECK(amount(r, "Income before income tax", 2) == 235000);
    CHECK(amount(r, "Income tax", 1) == 90000);
    CHECK(amount(r, "Net income", 1) == 70000);
    CHECK(amount(r, "Net income", 2) == 145000);
    const rpt::Row* net = row(r, "Net income");
    CHECK(net->kind == rpt::RowKind::Total);
    CHECK(net->final);

    // Zero lines and empty headings are hidden; every income account is classified.
    CHECK(row(r, "Sales returns") == nullptr);
    CHECK(row(r, "Variable selling") == nullptr);
    CHECK(row(r, "Unclassified accounts") == nullptr);
    CHECK(hasHeading(r, "Revenue"));
    CHECK_FALSE(hasNote(r, "on no line"));
}

TEST_CASE("F3: prior year, budget, variance, landscape", "[statements][f3]")
{
    FixtureDb db;
    StatementOptions opt;
    opt.comparePrior = true;
    opt.compareBudget = true;
    const rpt::Report r = IncomeStatement(db.code, context(db.code), opt);
    CHECK(r.orientation == rpt::Orientation::Landscape);
    REQUIRE(r.columns.size() == 6);   // caption, month, YTD, prior, budget, variance
    CHECK(amount(r, "Gross sales", 3) == 6300000);
    CHECK(amount(r, "Gross sales", 4) == 7000000);
    CHECK(amount(r, "Gross sales", 5) == -5400000);   // 16,000 - 70,000
    CHECK(amount(r, "Total operating expenses", 3) == 3430000);
    CHECK(amount(r, "Total operating expenses", 4) == 3710000);
    CHECK(amount(r, "Net income", 3) == -112000);
    CHECK(amount(r, "Net income", 4) == -35000);
    CHECK(amount(r, "Net income", 5) == 180000);
}

TEST_CASE("F3 for one department", "[statements][f3]")
{
    FixtureDb db;
    StatementOptions opt;
    opt.dept = "10";
    const rpt::Report r = IncomeStatement(db.code, context(db.code), opt);
    REQUIRE(r.subtitles.size() == 2);
    CHECK(r.subtitles[1] == "Department 10 Research & Development");
    CHECK(amount(r, "Personnel", 1) == 200000);
    CHECK(amount(r, "Personnel", 2) == 400000);
    CHECK(amount(r, "Net income", 1) == -200000);
    CHECK(amount(r, "Net income", 2) == -400000);
    CHECK(row(r, "Gross sales") == nullptr);   // 410001-10 has no activity
}

TEST_CASE("F3 options: zero lines and the accounts under each line", "[statements][f3]")
{
    FixtureDb db;
    StatementOptions opt;
    opt.showZeroLines = true;
    opt.showAccounts = true;
    const rpt::Report r = IncomeStatement(db.code, context(db.code), opt);
    CHECK(row(r, "Sales returns") != nullptr);
    const rpt::Row* salaries = row(r, "731000-20  SALARIES - GENERAL");
    REQUIRE(salaries);
    CHECK(salaries->indent == 2);
    CHECK(salaries->cells[2].value == 150000);
    CHECK(row(r, "410001-10  SALES REVENUE - R&D") != nullptr);   // zero, but shown with the option
}

// ---- F4 ---------------------------------------------------------------------

TEST_CASE("F4 Balance Sheet balances; its net income is F3's year to date", "[statements][f4]")
{
    FixtureDb db;
    const rpt::Report r = BalanceSheet(db.code, context(db.code), {});
    CHECK(r.title == "Balance Sheet");
    CHECK(r.subtitles[0] == "As at October 31, 2026");
    CHECK(amount(r, "Cash and bank", 1) == 3795000);
    CHECK(amount(r, "Cash and bank", 2) == 3300000);
    CHECK(amount(r, "Total current assets", 1) == 4646000);
    CHECK(amount(r, "Accumulated depreciation", 1) == -660000);
    CHECK(amount(r, "Total assets", 1) == 7486000);
    CHECK(amount(r, "Total assets", 2) == 6551000);
    CHECK(amount(r, "Total liabilities", 1) == 1267000);
    CHECK(amount(r, "Total liabilities", 2) == 467000);
    CHECK(amount(r, "Net income for the year to date", 1) ==
          amount(IncomeStatement(db.code, context(db.code), {}), "Net income", 2));
    CHECK(amount(r, "Total equity", 1) == 6229000);
    CHECK(amount(r, "Total equity", 2) == 6084000);
    CHECK(amount(r, "Unclassified accounts", 1) == -10000);
    CHECK(amount(r, "Total liabilities and equity", 1) == amount(r, "Total assets", 1));
    CHECK(amount(r, "Total liabilities and equity", 2) == amount(r, "Total assets", 2));
    CHECK(row(r, "Total liabilities and equity")->final);
    CHECK(hasNote(r, "999000"));   // the layout warning
    CHECK_FALSE(hasNote(r, "doesn't balance"));
}

TEST_CASE("F4: an account the layout misses is Unclassified, and the sheet still balances", "[statements][f4]")
{
    FixtureDb db;
    EnsureLayoutTables(db.code);
    auto lines = LoadLayout(db.code, kBalanceSheet);
    lines.erase(std::remove_if(lines.begin(), lines.end(), [](const LayoutLine& l) { return l.lineNo == 70; }), lines.end());
    for (LayoutLine& l : lines)
        if (l.lineNo == 100) l.formula = "60+80+90";
    SaveLayout(db.code, kBalanceSheet, lines, "2026-11-11 10:00");

    const rpt::Report r = BalanceSheet(db.code, context(db.code), {});
    CHECK(r.footerNote == "Balance Sheet - Layout: modified 2026-11-11 10:00");
    CHECK(amount(r, "Total assets", 1) == 3986000);              // 74,860 less 35,000
    CHECK(amount(r, "Unclassified accounts", 1) == -3510000);    // 151000 and 999000
    CHECK(amount(r, "Total liabilities and equity", 1) == 3986000);
    CHECK(hasNote(r, "151000"));
}

TEST_CASE("F4: an unbalanced ledger gets a note", "[statements][f4]")
{
    FixtureDb db;
    {
        Data4 gl(&db.code, "glaccts");
        gl.SetIndex("accounts");
        Field4 acc(gl, "account"), p7(gl, "period7"), bal(gl, "curr_bal");
        for (gl.top(); !gl.eof(); gl.skip()) {
            std::string a = acc.str();
            a.erase(a.find_last_not_of(' ') + 1);
            if (a == "102201") {
                p7.assignDouble(p7.num() + 10.0);
                bal.assignDouble(bal.num() + 10.0);
                gl.flush();
                break;
            }
        }
        gl.close();
    }
    const rpt::Report r = BalanceSheet(db.code, context(db.code), {});
    CHECK(hasNote(r, "doesn't balance"));
    CHECK(hasNote(r, "10.00"));
}

// ---- F5 ---------------------------------------------------------------------

TEST_CASE("F5 Cash Flow Statement reconciles for the month and the year", "[statements][f5]")
{
    FixtureDb db;
    const rpt::Report r = CashFlowStatement(db.code, context(db.code), {});
    CHECK(r.title == "Cash Flow Statement");
    CHECK(amount(r, "Net income", 1) == 70000);
    CHECK(amount(r, "Net income", 2) == 145000);
    CHECK(amount(r, "Depreciation and amortisation", 2) == 60000);
    CHECK(amount(r, "Net cash from operating activities", 1) == 70000);
    CHECK(amount(r, "Net cash from operating activities", 2) == 205000);
    CHECK(amount(r, "Property and equipment", 2) == -500000);
    CHECK(amount(r, "Net cash used in investing activities", 2) == -500000);
    CHECK(amount(r, "Borrowings", 2) == 800000);
    CHECK(amount(r, "Unclassified accounts", 2) == -10000);
    CHECK(amount(r, "Net increase (decrease) in cash", 1) == 70000);
    CHECK(amount(r, "Net increase (decrease) in cash", 2) == 495000);
    CHECK(amount(r, "Cash at beginning of period", 1) == 3725000);
    CHECK(amount(r, "Cash at beginning of period", 2) == 3300000);
    CHECK(amount(r, "Cash at end of period", 1) == 3795000);
    CHECK(amount(r, "Cash at end of period", 2) == 3795000);
    CHECK(row(r, "Cash at end of period")->final);
    CHECK(row(r, "Unreconciled difference") == nullptr);
    CHECK(hasHeading(r, "Financing activities"));    // Borrowings is under it
    CHECK_FALSE(hasHeading(r, "Nothing"));
    // Closing cash is F4's cash.
    CHECK(amount(r, "Cash at end of period", 1) == amount(BalanceSheet(db.code, context(db.code), {}), "Cash and bank", 1));
}

TEST_CASE("F5 at an earlier period, and an unreconciled difference", "[statements][f5]")
{
    FixtureDb db;
    StatementOptions opt;
    opt.period = 4;   // the loan
    rpt::Report r = CashFlowStatement(db.code, context(db.code), opt);
    CHECK(amount(r, "Borrowings", 1) == 800000);
    CHECK(amount(r, "Cash at end of period", 1) ==
          amount(r, "Cash at beginning of period", 1) + amount(r, "Net increase (decrease) in cash", 1));

    // Period 1: the year's opening cash is 33,000, and nothing was invested,
    // so the investing heading goes; opening and closing cash always print.
    opt.period = 1;
    r = CashFlowStatement(db.code, context(db.code), opt);
    CHECK(amount(r, "Cash at beginning of period", 1) == 3300000);
    CHECK_FALSE(hasHeading(r, "Investing activities"));
    CHECK(row(r, "Net cash used in investing activities") != nullptr);   // totals always print

    // A non-cash account on two lines counts twice: the gap shows.
    EnsureLayoutTables(db.code);
    const auto original = LoadLayout(db.code, kCashFlow);
    auto lines = original;
    lines[2].ranges.push_back({"150000", "159999"});   // line 30 also takes 151000
    SaveLayout(db.code, kCashFlow, lines, "2026-11-11 10:00");
    r = CashFlowStatement(db.code, context(db.code), {});
    CHECK(amount(r, "Unreconciled difference", 1) == 0);         // 151000 didn't move in October
    CHECK(amount(r, "Unreconciled difference", 2) == 500000);    // its 5,000 purchase, counted twice
    CHECK(hasNote(r, "more than one line"));

    // A cash account on an account line only warns: cash counts only on the O / E lines.
    lines = original;
    lines[2].ranges.push_back({"102200", "102299"});
    SaveLayout(db.code, kCashFlow, lines, "2026-11-11 10:00");
    r = CashFlowStatement(db.code, context(db.code), {});
    CHECK(row(r, "Unreconciled difference") == nullptr);
    CHECK(hasNote(r, "cash accounts are also on an account line"));

    // Cash ranges that leave out 102xxx: those accounts become unclassified
    // non-cash accounts, so the statement still reconciles.
    lines = original;
    lines.back().ranges = {{"100000", "101999"}};
    lines[lines.size() - 2].ranges = {{"100000", "101999"}};
    SaveLayout(db.code, kCashFlow, lines, "2026-11-11 10:00");
    r = CashFlowStatement(db.code, context(db.code), {});
    CHECK(row(r, "Unreconciled difference") == nullptr);
    CHECK(amount(r, "Cash at end of period", 2) == 25000);   // 101000 only
}
