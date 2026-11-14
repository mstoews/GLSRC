//
// Csv.cpp - see report/Csv.h.
//
#include "report/Csv.h"

#include <cstdio>
#include <cstdlib>
#include <vector>

namespace rpt {

namespace {

std::string amountText(std::int64_t cents)
{
    const std::uint64_t a = cents < 0 ? 0 - static_cast<std::uint64_t>(cents) : static_cast<std::uint64_t>(cents);
    char buf[32];
    std::snprintf(buf, sizeof buf, "%s%llu.%02llu", cents < 0 ? "-" : "", static_cast<unsigned long long>(a / 100),
                  static_cast<unsigned long long>(a % 100));
    return buf;
}

std::string cellText(const Cell& c)
{
    switch (c.type) {
    case CellType::Empty: return {};
    case CellType::Text: return c.text;
    case CellType::Amount: return amountText(c.value);
    case CellType::Integer: return std::to_string(c.value);
    case CellType::Date:
        return c.text.size() == 8 ? c.text.substr(0, 4) + "-" + c.text.substr(4, 2) + "-" + c.text.substr(6, 2) : c.text;
    }
    return {};
}

void line(std::string& out, const std::vector<std::string>& fields)
{
    for (size_t i = 0; i < fields.size(); ++i) {
        if (i) out += ',';
        out += CsvField(fields[i]);
    }
    out += "\r\n";
}

} // namespace

std::string CsvField(const std::string& text)
{
    const bool quote = text.find_first_of(",\"\r\n") != std::string::npos ||
                       (!text.empty() && (text.front() == ' ' || text.back() == ' '));
    if (!quote) return text;
    std::string out = "\"";
    for (char ch : text) {
        if (ch == '"') out += '"';
        out += ch;
    }
    return out + "\"";
}

std::string ToCsv(const Report& report)
{
    std::string out = "\xEF\xBB\xBF";
    const size_t n = report.columns.size();
    std::vector<std::string> fields;
    for (const Column& c : report.columns)
        fields.push_back(c.header2.empty() ? c.header : c.header.empty() ? c.header2 : c.header + " " + c.header2);
    line(out, fields);
    auto spanning = [&](const std::string& text) {
        std::vector<std::string> f(n ? n : 1);
        f[0] = text;
        line(out, f);
    };
    for (const Row& r : report.rows) {
        switch (r.kind) {
        case RowKind::Blank:
        case RowKind::PageBreak: break;
        case RowKind::Heading:
        case RowKind::Note: spanning(r.text); break;
        case RowKind::GroupHeader:
            if (r.cells.empty()) {
                spanning(r.text);
                break;
            }
            [[fallthrough]];
        case RowKind::Detail:
        case RowKind::GroupFooter:
        case RowKind::Subtotal:
        case RowKind::Total: {
            std::vector<std::string> f(n > r.cells.size() ? n : r.cells.size());
            for (size_t i = 0; i < r.cells.size(); ++i) f[i] = cellText(r.cells[i]);
            line(out, f);
            break;
        }
        case RowKind::Slip:
            for (const auto& [label, value] : r.fields) line(out, {label, value});
            break;
        }
    }
    return out;
}

} // namespace rpt
