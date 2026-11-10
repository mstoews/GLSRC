//
// Format.cpp - see Format.h.
//
#include "report/Format.h"

#include <cmath>
#include <cstdlib>

namespace rpt {

namespace {

std::string_view trim(std::string_view s) noexcept
{
    while (!s.empty() && (s.front() == ' ' || s.front() == '\t')) s.remove_prefix(1);
    while (!s.empty() && (s.back() == ' ' || s.back() == '\t')) s.remove_suffix(1);
    return s;
}

// "1234567" -> "1,234,567"
std::string groupThousands(std::uint64_t n)
{
    std::string digits = std::to_string(n);
    std::string out;
    const size_t len = digits.size();
    for (size_t i = 0; i < len; ++i) {
        if (i > 0 && (len - i) % 3 == 0) out += ',';
        out += digits[i];
    }
    return out;
}

std::uint64_t magnitude(std::int64_t v) noexcept
{
    // -INT64_MIN overflows; go through unsigned arithmetic instead.
    return v < 0 ? std::uint64_t{0} - static_cast<std::uint64_t>(v) : static_cast<std::uint64_t>(v);
}

bool validDate(std::string_view s, int& y, int& m, int& d)
{
    if (s.size() != 8) return false;
    for (char c : s)
        if (c < '0' || c > '9') return false;
    y = std::atoi(std::string(s.substr(0, 4)).c_str());
    m = std::atoi(std::string(s.substr(4, 2)).c_str());
    d = std::atoi(std::string(s.substr(6, 2)).c_str());
    return m >= 1 && m <= 12 && d >= 1 && d <= 31;
}

constexpr const char* kMonths[] = {"January", "February", "March",     "April",   "May",      "June",
                                   "July",    "August",   "September", "October", "November", "December"};

} // namespace

std::int64_t ToCents(double amount) noexcept
{
    return static_cast<std::int64_t>(std::llround(amount * 100.0));
}

std::string FormatAmount(std::int64_t cents, NegativeStyle style, bool blankZero)
{
    if (cents == 0 && blankZero) return {};
    const std::uint64_t m = magnitude(cents);
    std::string body = groupThousands(m / 100) + '.';
    const auto frac = static_cast<unsigned>(m % 100);
    body += static_cast<char>('0' + frac / 10);
    body += static_cast<char>('0' + frac % 10);
    if (cents >= 0) return body;
    return style == NegativeStyle::Parentheses ? "(" + body + ")" : "-" + body;
}

std::string FormatInteger(std::int64_t n, bool blankZero)
{
    if (n == 0 && blankZero) return {};
    const std::string body = groupThousands(magnitude(n));
    return n < 0 ? "-" + body : body;
}

std::string FormatDate(std::string_view yyyymmdd)
{
    const std::string_view s = trim(yyyymmdd);
    int y = 0, m = 0, d = 0;
    if (!validDate(s, y, m, d)) return std::string(s);
    return std::string(s.substr(4, 2)) + '/' + std::string(s.substr(6, 2)) + '/' + std::string(s.substr(0, 4));
}

std::string FormatDateLong(std::string_view yyyymmdd)
{
    const std::string_view s = trim(yyyymmdd);
    int y = 0, m = 0, d = 0;
    if (!validDate(s, y, m, d)) return FormatDate(s);
    return std::string(kMonths[m - 1]) + ' ' + std::to_string(d) + ", " + std::to_string(y);
}

std::string FormatCell(const Cell& cell, const Column& column, NegativeStyle style)
{
    switch (cell.type) {
    case CellType::Empty:   return {};
    case CellType::Text:    return cell.text;
    case CellType::Amount:  return FormatAmount(cell.value, style, column.blankZero);
    case CellType::Integer: return FormatInteger(cell.value, column.blankZero);
    case CellType::Date:    return FormatDate(cell.text);
    }
    return {};
}

} // namespace rpt
