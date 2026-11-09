#pragma once
//
// Format.h - how numbers and dates print (spec #3 4.3).
//
// Dates: the database stores yyyymmdd (dBase D fields, and the text dates of
// gl_dist). Reports print MM/DD/YYYY: the order of the screens' tbDate
// controls (DS_MMDDYY), with a four-digit year so a printed date is never
// ambiguous. Statement subtitles use the long form, "October 31, 2026".
//
#include "report/Report.h"

#include <cstdint>
#include <string>
#include <string_view>

namespace rpt {

// Rounds to whole cents. Amounts are stored as doubles with 2 decimals.
std::int64_t ToCents(double amount) noexcept;

// 123456789 -> "1,234,567.89"; negatives "-1,234.56" or "(1,234.56)".
// blankZero: 0 -> "".
std::string FormatAmount(std::int64_t cents, NegativeStyle style = NegativeStyle::Minus, bool blankZero = false);

// 1234567 -> "1,234,567". blankZero: 0 -> "".
std::string FormatInteger(std::int64_t n, bool blankZero = false);

// "20261031" -> "10/31/2026". Blank -> "". Anything else is returned trimmed.
std::string FormatDate(std::string_view yyyymmdd);

// "20261031" -> "October 31, 2026". Not a valid date -> FormatDate's result.
std::string FormatDateLong(std::string_view yyyymmdd);

// A cell as it prints in `column` (Empty -> "").
std::string FormatCell(const Cell& cell, const Column& column, NegativeStyle style);

} // namespace rpt
