//
// report_tests: number and date formatting (spec #3 4.3).
//
#include "catch_amalgamated.hpp"

#include "report/Format.h"

#include <cstdint>
#include <limits>

using namespace rpt;

TEST_CASE("amounts group thousands and keep two decimals", "[format]")
{
    CHECK(FormatAmount(0) == "0.00");
    CHECK(FormatAmount(5) == "0.05");
    CHECK(FormatAmount(123) == "1.23");
    CHECK(FormatAmount(100000) == "1,000.00");
    CHECK(FormatAmount(123456789) == "1,234,567.89");
    CHECK(FormatAmount(-123456) == "-1,234.56");
    CHECK(FormatAmount(-123456, NegativeStyle::Parentheses) == "(1,234.56)");
    CHECK(FormatAmount(-5, NegativeStyle::Parentheses) == "(0.05)");
    CHECK(FormatAmount(0, NegativeStyle::Minus, true).empty());
    CHECK(FormatAmount(1, NegativeStyle::Minus, true) == "0.01");
    CHECK(FormatAmount(std::numeric_limits<std::int64_t>::min()) == "-92,233,720,368,547,758.08");
}

TEST_CASE("cents round to the nearest cent", "[format]")
{
    CHECK(ToCents(0.0) == 0);
    CHECK(ToCents(12.34) == 1234);
    CHECK(ToCents(1234567.89) == 123456789);
    CHECK(ToCents(0.1 + 0.2) == 30);
    CHECK(ToCents(-250.0) == -25000);
    CHECK(ToCents(-0.004) == 0);
    CHECK(ToCents(13050.0) == 1305000);
}

TEST_CASE("integers group thousands", "[format]")
{
    CHECK(FormatInteger(0) == "0");
    CHECK(FormatInteger(0, true).empty());
    CHECK(FormatInteger(999) == "999");
    CHECK(FormatInteger(1000) == "1,000");
    CHECK(FormatInteger(-1234567) == "-1,234,567");
}

TEST_CASE("dates print MM/DD/YYYY, or long on statements", "[format]")
{
    CHECK(FormatDate("20261031") == "10/31/2026");
    CHECK(FormatDate("19950302  ") == "03/02/1995");   // gl_dist's C(10) text date
    CHECK(FormatDate("        ").empty());
    CHECK(FormatDate("") == "");
    CHECK(FormatDate("not a date") == "not a date");
    CHECK(FormatDate("20261301") == "20261301");        // month 13
    CHECK(FormatDateLong("20261031") == "October 31, 2026");
    CHECK(FormatDateLong("19940401") == "April 1, 1994");
    CHECK(FormatDateLong("") == "");
}

TEST_CASE("cells format by type and column", "[format]")
{
    Column amount("Debit", 1, CellType::Amount, {}, true);
    Column text("Account", 1);
    CHECK(FormatCell(Cell::Amount(0), amount, NegativeStyle::Minus).empty());
    CHECK(FormatCell(Cell::Amount(-100), amount, NegativeStyle::Parentheses) == "(1.00)");
    CHECK(FormatCell(Cell::Integer(1500), text, NegativeStyle::Minus) == "1,500");
    CHECK(FormatCell(Cell::Date("20261015"), text, NegativeStyle::Minus) == "10/15/2026");
    CHECK(FormatCell(Cell::Text("102101"), text, NegativeStyle::Minus) == "102101");
    CHECK(FormatCell(Cell::Empty(), amount, NegativeStyle::Minus).empty());
    CHECK(amount.alignment() == Align::Right);
    CHECK(text.alignment() == Align::Left);
    CHECK(Column("x", 1).aligned(Align::Center).alignment() == Align::Center);
}
