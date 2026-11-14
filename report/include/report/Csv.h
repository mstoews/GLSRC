//
// Csv.h - a report's rows as CSV, for Excel (spec #7 Q4, plan S7-M5).
//
// One line of column headers, then a line per row: detail, group header and
// footer, subtotal and total rows in their columns; a heading, group header
// or note's text in the first column; a slip's fields as label, value lines.
// Blank lines and page breaks are left out, and so is indentation.
//
// UTF-8 with a byte order mark (so Excel reads it as UTF-8), CRLF line ends.
// Amounts are plain numbers with two decimals ("-1234.50": no thousands
// separator, no parentheses), integers plain, dates yyyy-mm-dd. A field is
// quoted when it holds a comma, a quote, a line break or edge spaces.
//
#pragma once

#include "report/Report.h"

#include <string>

namespace rpt {

std::string ToCsv(const Report& report);

// One field, quoted if it needs to be.
std::string CsvField(const std::string& text);

} // namespace rpt
