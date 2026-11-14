//
// ReportWriter.cpp - see ReportWriter.h.
//
// Translation notes (plan per-port checklist; decisions are spec #7 §8):
//   - D1: the original's custom reports were Crystal .RPT files registered
//     in GL_REPT (TGLAddReport took a description, a name and a path); the
//     writer replaces Crystal's designer, not the file format.
//   - D3: each source reads its tables the way its owning module does:
//     sales lines through Ar.h (LoadArVouchers, LoadArLines: the roles and
//     typed amounts of ar_linex, so a line's sales and VAT are the ledger's);
//     AR open items through LoadArOpenItems; GL voucher lines with Spec #2's
//     modes and gl_btchd's text PERIOD.
//   - Totals are the builder's, in cents (the engine has no formulas).
//
#include "ReportWriter.h"

#include "Accounts.h"   // TrimText
#include "Ar.h"

#include "report/Format.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <utility>

namespace rw {

namespace {

// ---- small helpers ---------------------------------------------------------------------

std::string lower(std::string s)
{
    for (char& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return s;
}

std::string norm(const std::string& s) { return lower(TrimText(s)); }

// a / b rounded half away from zero, in integers.
std::int64_t divRound(std::int64_t a, std::int64_t b)
{
    if (b == 0) return 0;
    const bool neg = (a < 0) != (b < 0);
    const std::int64_t ua = a < 0 ? -a : a, ub = b < 0 ? -b : b;
    const std::int64_t q = (ua + ub / 2) / ub;
    return neg ? -q : q;
}

bool hasTable(cb4sqlite::Code4& code, const char* name)
{
    cb4sqlite::Query q(code, "SELECT count(*) FROM sqlite_master WHERE type = 'table' AND name = ?1");
    q.bindText(1, name);
    return q.next() && q.integer(0) > 0;
}

Value text(const std::string& s) { return {TrimText(s), 0}; }
Value number(std::int64_t n) { return {{}, n}; }

// "1,234.56" / "-12" -> cents; false if it isn't a number.
bool parseCents(const std::string& in, std::int64_t& out)
{
    std::string s;
    for (char c : TrimText(in))
        if (c != ',') s += c;
    if (s.empty()) return false;
    char* end = nullptr;
    const double v = std::strtod(s.c_str(), &end);
    if (!end || *end) return false;
    out = rpt::ToCents(v);
    return true;
}

bool parseInteger(const std::string& in, std::int64_t& out)
{
    const std::string s = TrimText(in);
    if (s.empty()) return false;
    char* end = nullptr;
    const long long v = std::strtoll(s.c_str(), &end, 10);
    if (!end || *end) return false;
    out = v;
    return true;
}

// yyyymmdd, or mm/dd/yyyy -> yyyymmdd; false otherwise.
bool parseDate(const std::string& in, std::string& out)
{
    const std::string s = TrimText(in);
    auto digits = [](const std::string& t) { return !t.empty() && t.find_first_not_of("0123456789") == std::string::npos; };
    std::string d;
    if (s.size() == 8 && digits(s)) d = s;
    else if (s.size() == 10 && s[2] == '/' && s[5] == '/' && digits(s.substr(0, 2)) && digits(s.substr(3, 2)) &&
             digits(s.substr(6, 4)))
        d = s.substr(6, 4) + s.substr(0, 2) + s.substr(3, 2);
    else
        return false;
    const int month = std::atoi(d.substr(4, 2).c_str()), day = std::atoi(d.substr(6, 2).c_str());
    if (month < 1 || month > 12 || day < 1 || day > 31) return false;
    out = d;
    return true;
}

std::string display(const Field& f, const Value& v)
{
    switch (f.type) {
    case FieldType::Amount: return rpt::FormatAmount(v.number);
    case FieldType::Integer: return std::to_string(v.number);
    case FieldType::Date: return v.text.empty() ? std::string("(no date)") : rpt::FormatDate(v.text);
    case FieldType::Text: break;
    }
    return v.text.empty() ? std::string("(blank)") : v.text;
}

// -1, 0, 1, by the field's type.
int compareValues(const Field& f, const Value& a, const Value& b)
{
    if (f.type == FieldType::Amount || f.type == FieldType::Integer) return a.number < b.number ? -1 : a.number > b.number;
    if (f.type == FieldType::Date) return a.text < b.text ? -1 : a.text > b.text;
    const std::string x = norm(a.text), y = norm(b.text);
    return x < y ? -1 : x > y;
}

// ---- the text format -------------------------------------------------------------------

const char* testWord(Test t)
{
    switch (t) {
    case Test::Equal: return "=";
    case Test::NotEqual: return "<>";
    case Test::Between: return "between";
    case Test::In: return "in";
    case Test::Contains: return "contains";
    case Test::Blank: return "blank";
    case Test::NotBlank: return "notblank";
    }
    return "=";
}

std::optional<Test> testOf(const std::string& w)
{
    const std::string s = lower(w);
    if (s == "=") return Test::Equal;
    if (s == "<>") return Test::NotEqual;
    if (s == "between") return Test::Between;
    if (s == "in") return Test::In;
    if (s == "contains") return Test::Contains;
    if (s == "blank") return Test::Blank;
    if (s == "notblank") return Test::NotBlank;
    return std::nullopt;
}

// How many values a test takes; -1: one or more.
int valueCount(Test t)
{
    switch (t) {
    case Test::Between: return 2;
    case Test::In: return -1;
    case Test::Blank:
    case Test::NotBlank: return 0;
    default: return 1;
    }
}

struct Token {
    std::string text;
    bool quoted = false;
};

// Splits a line into words and "quoted text"; false on an unclosed quote.
bool tokenize(const std::string& line, std::vector<Token>& out)
{
    size_t i = 0;
    while (i < line.size()) {
        if (std::isspace(static_cast<unsigned char>(line[i]))) {
            ++i;
            continue;
        }
        Token t;
        if (line[i] == '"') {
            t.quoted = true;
            ++i;
            bool closed = false;
            while (i < line.size()) {
                if (line[i] == '\\' && i + 1 < line.size()) {
                    t.text += line[i + 1];
                    i += 2;
                } else if (line[i] == '"') {
                    ++i;
                    closed = true;
                    break;
                } else {
                    t.text += line[i++];
                }
            }
            if (!closed) return false;
        } else {
            while (i < line.size() && !std::isspace(static_cast<unsigned char>(line[i]))) t.text += line[i++];
        }
        out.push_back(t);
    }
    return true;
}

std::string quote(const std::string& s)
{
    std::string out = "\"";
    for (char c : s) {
        if (c == '"' || c == '\\') out += '\\';
        out += c;
    }
    return out + "\"";
}

// A value as a word when it can be one, else quoted.
std::string word(const std::string& s)
{
    if (s.empty() || s == "?" || s.find_first_of(" \t\"\\#") != std::string::npos) return quote(s);
    return s;
}

std::string widthText(double w)
{
    char buf[32];
    std::snprintf(buf, sizeof buf, "%g", w);
    return buf;
}

} // namespace

int Source::fieldIndex(const std::string& k) const
{
    for (size_t i = 0; i < fields.size(); ++i)
        if (fields[i].key == k) return static_cast<int>(i);
    return -1;
}

ParseResult ParseDefinition(const std::string& textIn)
{
    ParseResult r;
    ReportDefinition d;
    bool sawWriter = false, sawName = false, sawSource = false;
    bool sawDescription = false, sawTitle = false;
    std::vector<std::string> lines;
    {
        std::string cur;
        for (char c : textIn) {
            if (c == '\n') {
                lines.push_back(cur);
                cur.clear();
            } else if (c != '\r') {
                cur += c;
            }
        }
        if (!cur.empty()) lines.push_back(cur);
    }
    auto fail = [&](int line, std::string why) {
        r.line = line;
        r.error = std::move(why);
        return r;
    };
    auto yesNo = [](const std::string& w, bool& out) {
        const std::string s = lower(w);
        if (s == "yes") out = true;
        else if (s == "no") out = false;
        else return false;
        return true;
    };
    for (size_t n = 0; n < lines.size(); ++n) {
        const int ln = static_cast<int>(n + 1);
        const std::string t = TrimText(lines[n]);
        if (t.empty() || t[0] == '#') continue;
        std::vector<Token> tok;
        if (!tokenize(t, tok)) return fail(ln, "A quotation mark isn't closed.");
        const std::string key = lower(tok[0].text);
        if (!sawWriter) {
            if (key != "writer" || tok.size() != 2) return fail(ln, "The first line must be \"writer 1\".");
            std::int64_t v = 0;
            if (!parseInteger(tok[1].text, v)) return fail(ln, "The format's version must be a number.");
            if (v > kFormatVersion)
                return fail(ln, "This report was made by a newer version of the program (format " + std::to_string(v) +
                                    "); this one reads format " + std::to_string(kFormatVersion) + ".");
            sawWriter = true;
            continue;
        }
        auto oneText = [&](std::string& into, bool& seen) -> bool {
            if (tok.size() != 2) return false;
            into = tok[1].text;
            seen = true;
            return true;
        };
        if (key == "name") {
            if (!oneText(d.name, sawName)) return fail(ln, "\"name\" takes one text.");
        } else if (key == "description") {
            if (!oneText(d.description, sawDescription)) return fail(ln, "\"description\" takes one text.");
        } else if (key == "title") {
            if (!oneText(d.title, sawTitle)) return fail(ln, "\"title\" takes one text.");
        } else if (key == "subtitle") {
            bool seen = false;
            if (!oneText(d.subtitle, seen)) return fail(ln, "\"subtitle\" takes one text.");
        } else if (key == "orientation") {
            const std::string o = tok.size() == 2 ? lower(tok[1].text) : "";
            if (o == "landscape") d.landscape = true;
            else if (o == "portrait") d.landscape = false;
            else return fail(ln, "\"orientation\" is portrait or landscape.");
        } else if (key == "source") {
            std::int64_t v = 0;
            if (tok.size() != 3 || !parseInteger(tok[2].text, v)) return fail(ln, "\"source\" takes a source and its version.");
            d.source = tok[1].text;
            d.sourceVersion = static_cast<int>(v);
            sawSource = true;
        } else if (key == "column") {
            if (tok.size() < 2) return fail(ln, "\"column\" needs a field.");
            ColumnDef c;
            size_t i = 2;
            if (lower(tok[1].text) == "calc" && !tok[1].quoted) {
                const std::string kind = tok.size() > 2 ? lower(tok[2].text) : "";
                if (kind == "sum" || kind == "diff") {
                    if (tok.size() < 5) return fail(ln, "\"calc " + kind + "\" takes two fields.");
                    c.calc = kind == "sum" ? Calc::Sum : Calc::Difference;
                    c.field = tok[3].text;
                    c.field2 = tok[4].text;
                    i = 5;
                } else if (kind == "count") {
                    c.calc = Calc::Count;
                    i = 3;
                } else if (kind == "pct") {
                    const std::string of = tok.size() > 4 ? lower(tok[4].text) : "";
                    if (of != "group" && of != "total") return fail(ln, "\"calc pct\" takes a field, then group or total.");
                    c.calc = Calc::Percent;
                    c.field = tok[3].text;
                    c.ofGroup = of == "group";
                    i = 5;
                } else {
                    return fail(ln, "\"calc\" is sum, diff, count or pct.");
                }
            } else {
                c.field = tok[1].text;
            }
            for (; i < tok.size(); ++i) {
                const std::string o = lower(tok[i].text);
                if (o == "total") c.total = true;
                else if (o == "blank") c.blankZero = true;
                else if (o == "label" && i + 1 < tok.size()) c.label = tok[++i].text;
                else if (o == "width" && i + 1 < tok.size()) {
                    char* end = nullptr;
                    c.width = std::strtod(tok[++i].text.c_str(), &end);
                    if (!end || *end || c.width <= 0) return fail(ln, "A column's width must be a number above 0.");
                } else return fail(ln, "Unknown column option \"" + tok[i].text + "\".");
            }
            d.columns.push_back(c);
        } else if (key == "sort") {
            if (tok.size() < 2 || tok.size() > 3) return fail(ln, "\"sort\" takes a field, then asc or desc.");
            SortKey s;
            s.field = tok[1].text;
            if (tok.size() == 3) {
                const std::string o = lower(tok[2].text);
                if (o == "desc") s.descending = true;
                else if (o != "asc") return fail(ln, "\"sort\" takes a field, then asc or desc.");
            }
            if (d.sort.size() == kMaxSort) return fail(ln, "At most three sort fields.");
            d.sort.push_back(s);
        } else if (key == "group") {
            if (tok.size() < 2 || tok.size() > 3) return fail(ln, "\"group\" takes a field, then page.");
            GroupDef g;
            g.field = tok[1].text;
            if (tok.size() == 3) {
                if (lower(tok[2].text) != "page") return fail(ln, "\"group\" takes a field, then page.");
                g.newPage = true;
            }
            if (d.groups.size() == kMaxGroups) return fail(ln, "At most two groups.");
            d.groups.push_back(g);
        } else if (key == "filter") {
            if (tok.size() < 3) return fail(ln, "\"filter\" takes a field, a test and its values.");
            Filter f;
            f.field = tok[1].text;
            const std::optional<Test> test = testOf(tok[2].text);
            if (!test) return fail(ln, "Unknown test \"" + tok[2].text + "\".");
            f.test = *test;
            std::vector<Token> rest(tok.begin() + 3, tok.end());
            const int want = valueCount(f.test);
            if (want != 0 && !rest.empty() && !rest[0].quoted && rest[0].text == "?") {
                f.ask = true;
                const size_t marks = want < 0 ? 1 : static_cast<size_t>(want);
                for (size_t i = 0; i < marks; ++i)
                    if (i >= rest.size() || rest[i].quoted || rest[i].text != "?")
                        return fail(ln, "An asked \"" + std::string(testWord(f.test)) + "\" takes " +
                                            std::to_string(marks) + " \"?\".");
                rest.erase(rest.begin(), rest.begin() + static_cast<long>(marks));
                if (rest.size() > 1) return fail(ln, "An asked filter ends with its question.");
                if (rest.size() == 1) f.question = rest[0].text;
            } else {
                for (const Token& v : rest) f.values.push_back(v.text);
                if (want >= 0 && static_cast<int>(f.values.size()) != want)
                    return fail(ln, "\"" + std::string(testWord(f.test)) + "\" takes " + std::to_string(want) + " value(s).");
                if (want < 0 && f.values.empty()) return fail(ln, "\"in\" takes one value or more.");
            }
            d.filters.push_back(f);
        } else if (key == "total") {
            if (tok.size() != 2 || !yesNo(tok[1].text, d.grandTotal)) return fail(ln, "\"total\" is yes or no.");
        } else if (key == "count") {
            if (tok.size() != 2 || !yesNo(tok[1].text, d.count)) return fail(ln, "\"count\" is yes or no.");
        } else if (key == "writer") {
            return fail(ln, "\"writer\" is only the first line.");
        } else {
            return fail(ln, "Unknown setting \"" + tok[0].text + "\".");
        }
    }
    if (!sawWriter) return fail(1, "The first line must be \"writer 1\".");
    if (!sawName || TrimText(d.name).empty()) return fail(static_cast<int>(lines.size()), "The report has no name.");
    if (!sawSource) return fail(static_cast<int>(lines.size()), "The report has no source.");
    if (!sawDescription) d.description = d.name;
    if (!sawTitle) d.title = d.name;
    r.definition = d;
    return r;
}

std::string WriteDefinition(const ReportDefinition& d)
{
    std::string s = "writer " + std::to_string(kFormatVersion) + "\n";
    s += "name " + quote(d.name) + "\n";
    if (!d.description.empty() && d.description != d.name) s += "description " + quote(d.description) + "\n";
    if (!d.title.empty() && d.title != d.name) s += "title " + quote(d.title) + "\n";
    if (!d.subtitle.empty()) s += "subtitle " + quote(d.subtitle) + "\n";
    if (d.landscape) s += "orientation landscape\n";
    s += "source " + word(d.source) + " " + std::to_string(d.sourceVersion) + "\n";
    for (const ColumnDef& c : d.columns) {
        switch (c.calc) {
        case Calc::None: s += "column " + word(c.field); break;
        case Calc::Sum: s += "column calc sum " + word(c.field) + " " + word(c.field2); break;
        case Calc::Difference: s += "column calc diff " + word(c.field) + " " + word(c.field2); break;
        case Calc::Count: s += "column calc count"; break;
        case Calc::Percent: s += "column calc pct " + word(c.field) + (c.ofGroup ? " group" : " total"); break;
        }
        if (c.total) s += " total";
        if (c.blankZero) s += " blank";
        if (!c.label.empty()) s += " label " + quote(c.label);
        if (c.width > 0) s += " width " + widthText(c.width);
        s += "\n";
    }
    for (const SortKey& k : d.sort) s += "sort " + word(k.field) + (k.descending ? " desc" : "") + "\n";
    for (const GroupDef& g : d.groups) s += "group " + word(g.field) + (g.newPage ? " page" : "") + "\n";
    for (const Filter& f : d.filters) {
        s += "filter " + word(f.field) + " " + testWord(f.test);
        if (f.ask) {
            const int want = valueCount(f.test);
            for (int i = 0; i < (want < 0 ? 1 : want); ++i) s += " ?";
            if (!f.question.empty()) s += " " + quote(f.question);
        } else {
            for (const std::string& v : f.values) s += " " + word(v);
        }
        s += "\n";
    }
    if (!d.grandTotal) s += "total no\n";
    if (d.count) s += "count yes\n";
    return s;
}

std::string ColumnLabel(const Source& source, const ColumnDef& c)
{
    auto label = [&](const std::string& key) {
        const int i = source.fieldIndex(key);
        return i < 0 ? key : source.fields[static_cast<size_t>(i)].label;
    };
    switch (c.calc) {
    case Calc::None: return label(c.field);
    case Calc::Sum: return label(c.field) + " + " + label(c.field2);
    case Calc::Difference: return label(c.field) + " - " + label(c.field2);
    case Calc::Count: return "Count";
    case Calc::Percent: return label(c.field) + (c.ofGroup ? " % of group" : " % of total");
    }
    return c.field;
}

std::string ColumnProblem(const Source& source, const ColumnDef& c)
{
    auto numeric = [&](const std::string& key) {
        const int i = source.fieldIndex(key);
        return i >= 0 && (source.fields[static_cast<size_t>(i)].type == FieldType::Amount ||
                          source.fields[static_cast<size_t>(i)].type == FieldType::Integer);
    };
    auto known = [&](const std::string& key) { return source.fieldIndex(key) >= 0; };
    switch (c.calc) {
    case Calc::None:
    case Calc::Count: return {};
    case Calc::Sum:
    case Calc::Difference:
        if (known(c.field) && known(c.field2) && (!numeric(c.field) || !numeric(c.field2)))
            return "\"" + ColumnLabel(source, c) + "\" adds or subtracts numbers: both fields must be amounts or numbers.";
        return {};
    case Calc::Percent:
        if (known(c.field) && !numeric(c.field))
            return "\"" + ColumnLabel(source, c) + "\" is a percentage of a number: its field must be an amount or a number.";
        return {};
    }
    return {};
}

// ---- building ----------------------------------------------------------------------------

namespace {

struct ResolvedFilter {
    int index;
    const Filter* filter;
    std::vector<std::string> values;
    // parsed values
    std::vector<std::int64_t> numbers;
    std::vector<std::string> texts;   // dates as yyyymmdd; text normalized
};

// Parses a filter's values for its field; "" when fine, else why not.
std::string prepare(const Field& f, ResolvedFilter& rf)
{
    for (const std::string& v : rf.values) {
        if (f.type == FieldType::Amount || f.type == FieldType::Integer) {
            std::int64_t n = 0;
            const bool ok = f.type == FieldType::Amount ? parseCents(v, n) : parseInteger(v, n);
            if (!ok) return f.label + ": \"" + v + "\" isn't a number.";
            rf.numbers.push_back(n);
        } else if (f.type == FieldType::Date) {
            std::string d;
            if (!parseDate(v, d)) return f.label + ": \"" + v + "\" isn't a date (mm/dd/yyyy).";
            rf.texts.push_back(d);
        } else {
            rf.texts.push_back(norm(v));
        }
    }
    return {};
}

bool matches(const Field& f, const Value& v, const ResolvedFilter& rf)
{
    const Test t = rf.filter->test;
    const bool numeric = f.type == FieldType::Amount || f.type == FieldType::Integer;
    const std::string s = f.type == FieldType::Text ? norm(v.text) : v.text;
    auto eq = [&](size_t i) { return numeric ? v.number == rf.numbers[i] : s == rf.texts[i]; };
    switch (t) {
    case Test::Equal: return eq(0);
    case Test::NotEqual: return !eq(0);
    case Test::Between:
        if (numeric) return v.number >= rf.numbers[0] && v.number <= rf.numbers[1];
        return s >= rf.texts[0] && s <= rf.texts[1];
    case Test::In:
        for (size_t i = 0; i < (numeric ? rf.numbers.size() : rf.texts.size()); ++i)
            if (eq(i)) return true;
        return false;
    case Test::Contains:
        if (numeric) return std::to_string(v.number).find(std::to_string(rf.numbers[0])) != std::string::npos;
        return s.find(rf.texts[0]) != std::string::npos;
    case Test::Blank: return numeric ? v.number == 0 : TrimText(v.text).empty();
    case Test::NotBlank: return numeric ? v.number != 0 : !TrimText(v.text).empty();
    }
    return true;
}

std::string describe(const Field& f, const Filter& flt, const std::vector<std::string>& values)
{
    auto shown = [&](const std::string& v) {
        std::string d;
        return f.type == FieldType::Date && parseDate(v, d) ? rpt::FormatDate(d) : TrimText(v);
    };
    std::string out = f.label + " ";
    switch (flt.test) {
    case Test::Equal: return out + "is " + shown(values[0]);
    case Test::NotEqual: return out + "is not " + shown(values[0]);
    case Test::Between: return out + "from " + shown(values[0]) + " to " + shown(values[1]);
    case Test::In: {
        out += "is one of ";
        for (size_t i = 0; i < values.size(); ++i) out += (i ? ", " : "") + shown(values[i]);
        return out;
    }
    case Test::Contains: return out + "contains \"" + TrimText(values[0]) + "\"";
    case Test::Blank: return out + "is blank";
    case Test::NotBlank: return out + "isn't blank";
    }
    return out;
}

rpt::CellType cellType(FieldType t)
{
    switch (t) {
    case FieldType::Amount: return rpt::CellType::Amount;
    case FieldType::Integer: return rpt::CellType::Integer;
    case FieldType::Date: return rpt::CellType::Date;
    case FieldType::Text: break;
    }
    return rpt::CellType::Text;
}

rpt::Cell cellOf(const Field& f, const Value& v)
{
    switch (f.type) {
    case FieldType::Amount: return rpt::Cell::Amount(v.number);
    case FieldType::Integer: return rpt::Cell::Integer(v.number);
    case FieldType::Date: return v.text.empty() ? rpt::Cell::Empty() : rpt::Cell::Date(v.text);
    case FieldType::Text: break;
    }
    return rpt::Cell::Text(v.text);
}

} // namespace

BuildResult BuildReport(cb4sqlite::Code4& code, const ReportContext& ctx, const ReportDefinition& def,
                        const Answers& answers, size_t rowLimit)
{
    BuildResult out;
    const Source* src = FindSource(def.source);
    if (!src) {
        out.refusal = "The report's source \"" + def.source + "\" doesn't exist in this version of the program.";
        return out;
    }
    std::vector<std::string> notes;
    auto missing = [&](const std::string& field, const char* what) {
        notes.push_back("\"" + field + "\" isn't a field of " + src->name + " any more: the " + what + " is left out.");
    };

    // Columns: a field's, or a calculation's (spec #7 5.3).
    struct Col {
        const ColumnDef* def;
        int a = -1, b = -1;   // the fields' indexes
        FieldType type = FieldType::Text;
    };
    std::vector<Col> cols;
    for (const ColumnDef& c : def.columns) {
        Col col{&c};
        const bool needsA = c.calc != Calc::Count, needsB = c.calc == Calc::Sum || c.calc == Calc::Difference;
        col.a = needsA ? src->fieldIndex(c.field) : -1;
        col.b = needsB ? src->fieldIndex(c.field2) : -1;
        if (needsA && col.a < 0) {
            missing(c.field, "column");
            continue;
        }
        if (needsB && col.b < 0) {
            missing(c.field2, "column");
            continue;
        }
        if (c.calc != Calc::None) {
            if (const std::string why = ColumnProblem(*src, c); !why.empty()) {
                notes.push_back(why + " The column is left out.");
                continue;
            }
        }
        switch (c.calc) {
        case Calc::None: col.type = src->fields[static_cast<size_t>(col.a)].type; break;
        case Calc::Sum:
        case Calc::Difference:
            col.type = src->fields[static_cast<size_t>(col.a)].type == FieldType::Amount ||
                               src->fields[static_cast<size_t>(col.b)].type == FieldType::Amount
                           ? FieldType::Amount
                           : FieldType::Integer;
            break;
        case Calc::Count: col.type = FieldType::Integer; break;
        case Calc::Percent: col.type = FieldType::Amount; break;   // hundredths of a percent print as 12.34
        }
        cols.push_back(col);
    }
    if (cols.empty()) {
        out.refusal = "The report has no column to print.";
        return out;
    }

    // Filters: the definition's, then the source's defaults for fields it doesn't filter.
    std::vector<ResolvedFilter> filters;
    std::vector<std::string> filterLines;
    for (size_t k = 0; k < def.filters.size(); ++k) {
        const Filter& f = def.filters[k];
        const int i = src->fieldIndex(f.field);
        if (i < 0) {
            missing(f.field, "filter");
            continue;
        }
        ResolvedFilter rf{i, &f, f.values, {}, {}};
        if (f.ask) {
            auto a = answers.find(k);
            const int want = valueCount(f.test);
            if (a == answers.end() || a->second.empty() || (want > 0 && static_cast<int>(a->second.size()) != want)) {
                out.refusal = "The report asks: " + (f.question.empty() ? src->fields[static_cast<size_t>(i)].label : f.question) +
                              ". Answer it to run the report.";
                return out;
            }
            rf.values = a->second;
        }
        if (const std::string why = prepare(src->fields[static_cast<size_t>(i)], rf); !why.empty()) {
            out.refusal = why;
            return out;
        }
        filterLines.push_back(describe(src->fields[static_cast<size_t>(i)], f, rf.values));
        filters.push_back(std::move(rf));
    }
    for (const Filter& f : src->defaults) {
        const int i = src->fieldIndex(f.field);
        if (i < 0) continue;
        bool own = false;
        for (const Filter& d : def.filters) own = own || d.field == f.field;
        if (own) continue;
        ResolvedFilter rf{i, &f, f.values, {}, {}};
        if (prepare(src->fields[static_cast<size_t>(i)], rf).empty()) filters.push_back(std::move(rf));
    }

    // Sort keys: the groups, then the sort fields.
    struct Key {
        int index;
        bool descending;
    };
    std::vector<Key> keys;
    std::vector<std::pair<int, const GroupDef*>> groups;
    for (const GroupDef& g : def.groups) {
        const int i = src->fieldIndex(g.field);
        if (i < 0) {
            missing(g.field, "group");
            continue;
        }
        groups.emplace_back(i, &g);
        keys.push_back({i, false});
    }
    for (const SortKey& s : def.sort) {
        const int i = src->fieldIndex(s.field);
        if (i < 0) missing(s.field, "sort");
        else keys.push_back({i, s.descending});
    }

    // The rows.
    std::vector<SourceRow> rows;
    for (SourceRow& row : src->rows(code, ctx)) {
        bool keep = true;
        for (const ResolvedFilter& f : filters)
            if (!matches(src->fields[static_cast<size_t>(f.index)], row[static_cast<size_t>(f.index)], f)) {
                keep = false;
                break;
            }
        if (!keep) continue;
        rows.push_back(std::move(row));
        if (rows.size() > rowLimit) {
            out.refusal = "The report has more than " + std::to_string(rowLimit) +
                          " rows. Narrow its filters (a period, a date or an account range) and run it again.";
            return out;
        }
    }
    std::stable_sort(rows.begin(), rows.end(), [&](const SourceRow& a, const SourceRow& b) {
        for (const Key& k : keys) {
            const int c = compareValues(src->fields[static_cast<size_t>(k.index)], a[static_cast<size_t>(k.index)],
                                        b[static_cast<size_t>(k.index)]);
            if (c != 0) return k.descending ? c > 0 : c < 0;
        }
        return false;
    });

    // The report.
    rpt::Report r;
    r.title = TrimText(def.title).empty() ? def.name : def.title;
    if (!TrimText(def.subtitle).empty()) r.subtitles.push_back(def.subtitle);
    for (const std::string& l : filterLines) r.subtitles.push_back(l);
    r.footerNote = def.name;
    r.orientation = def.landscape ? rpt::Orientation::Landscape : rpt::Orientation::Portrait;
    for (const Col& c : cols) {
        const double fieldWidth = c.a >= 0 ? src->fields[static_cast<size_t>(c.a)].width : 0.8;
        r.columns.emplace_back(c.def->label.empty() ? ColumnLabel(*src, *c.def) : c.def->label,
                               c.def->width > 0 ? c.def->width : fieldWidth, cellType(c.type), std::string(),
                               c.def->blankZero);
    }
    for (const std::string& n : notes) r.rows.push_back(rpt::Row::Note(n));

    const size_t ncol = cols.size();
    const size_t levels = groups.size();
    auto totalled = [&](size_t k) {
        return cols[k].def->total && (cols[k].type == FieldType::Amount || cols[k].type == FieldType::Integer);
    };
    // A row's number in column k (a percentage column's: its field's).
    auto raw = [&](size_t k, const SourceRow& row) -> std::int64_t {
        const Col& c = cols[k];
        switch (c.def->calc) {
        case Calc::Sum: return row[static_cast<size_t>(c.a)].number + row[static_cast<size_t>(c.b)].number;
        case Calc::Difference: return row[static_cast<size_t>(c.a)].number - row[static_cast<size_t>(c.b)].number;
        case Calc::Count: return 1;
        default: return row[static_cast<size_t>(c.a)].number;
        }
    };
    auto percent = [](std::int64_t x, std::int64_t base) { return base == 0 ? 0 : divRound(x * 10000, base); };
    // The percentage columns' bases: the report's totals, and each row's innermost group's.
    std::vector<std::int64_t> reportBase(ncol, 0);
    std::vector<std::vector<std::int64_t>> groupBase(ncol);
    for (size_t k = 0; k < ncol; ++k) {
        if (cols[k].def->calc != Calc::Percent) continue;
        for (const SourceRow& row : rows) reportBase[k] += raw(k, row);
        groupBase[k].assign(rows.size(), reportBase[k]);
        if (!cols[k].def->ofGroup || levels == 0) continue;
        size_t start = 0;
        for (size_t i = 1; i <= rows.size(); ++i) {
            bool same = i < rows.size();
            for (size_t l = 0; same && l < levels; ++l) {
                const int g = groups[l].first;
                same = compareValues(src->fields[static_cast<size_t>(g)], rows[i - 1][static_cast<size_t>(g)],
                                     rows[i][static_cast<size_t>(g)]) == 0;
            }
            if (same) continue;
            std::int64_t sum = 0;
            for (size_t j = start; j < i; ++j) sum += raw(k, rows[j]);
            for (size_t j = start; j < i; ++j) groupBase[k][j] = sum;
            start = i;
        }
    }
    bool anyTotal = false;
    for (size_t k = 0; k < ncol; ++k) anyTotal = anyTotal || totalled(k);

    struct Sums {
        std::vector<std::int64_t> v;
        std::int64_t n = 0;
    };
    // `level`: the footer's group level; -1 for the grand total.
    auto totalCells = [&](const Sums& s, std::string label, int level) {
        std::vector<rpt::Cell> cells(ncol);
        if (def.count) label += " (" + std::to_string(s.n) + (s.n == 1 ? " row)" : " rows)");
        for (size_t k = 0; k < ncol; ++k) {
            if (!totalled(k)) continue;
            if (cols[k].def->calc == Calc::Percent) {
                // Of the report: the group's share; of a group: 100.00 on its own footer, nothing above it.
                if (!cols[k].def->ofGroup || levels == 0) cells[k] = rpt::Cell::Amount(percent(s.v[k], reportBase[k]));
                else if (level == static_cast<int>(levels) - 1) cells[k] = rpt::Cell::Amount(percent(s.v[k], s.v[k]));
                continue;
            }
            cells[k] = cols[k].type == FieldType::Amount ? rpt::Cell::Amount(s.v[k]) : rpt::Cell::Integer(s.v[k]);
        }
        // The label goes in the first column not totalled; with none, it is left out.
        for (size_t k = 0; k < ncol; ++k)
            if (!totalled(k)) {
                cells[k] = rpt::Cell::Text(label);
                break;
            }
        return cells;
    };
    auto addTo = [&](Sums& s, const SourceRow& row) {
        if (s.v.empty()) s.v.assign(ncol, 0);
        for (size_t k = 0; k < ncol; ++k)
            if (totalled(k)) s.v[k] += raw(k, row);
        ++s.n;
    };

    std::vector<Sums> groupSums(levels);
    std::vector<const SourceRow*> groupRow(levels, nullptr);   // the open group's first row
    Sums grand;
    grand.v.assign(ncol, 0);
    auto headerText = [&](size_t level, const SourceRow& row) {
        const int i = groups[level].first;
        const Field& f = src->fields[static_cast<size_t>(i)];
        std::string t = f.label + ": " + display(f, row[static_cast<size_t>(i)]);
        if (!f.nameField.empty()) {
            const int n = src->fieldIndex(f.nameField);
            if (n >= 0 && !row[static_cast<size_t>(n)].text.empty()) t += "  " + row[static_cast<size_t>(n)].text;
        }
        return t;
    };
    auto closeFrom = [&](size_t level) {   // closes the open groups from the deepest to `level`
        for (size_t l = levels; l-- > level;) {
            if (!groupRow[l]) continue;
            if (anyTotal || def.count) {
                const int i = groups[l].first;
                rpt::Row row = rpt::Row::GroupFooter(totalCells(
                    groupSums[l], "Total " + display(src->fields[static_cast<size_t>(i)], (*groupRow[l])[static_cast<size_t>(i)]),
                    static_cast<int>(l)));
                row.indent = static_cast<int>(l);
                r.rows.push_back(std::move(row));
            }
            groupRow[l] = nullptr;
            groupSums[l] = Sums{};
        }
    };
    bool first = true;
    for (size_t at = 0; at < rows.size(); ++at) {
        const SourceRow& row = rows[at];
        size_t from = levels;   // the first level whose value changes
        for (size_t l = 0; l < levels; ++l) {
            const int i = groups[l].first;
            if (!groupRow[l] || compareValues(src->fields[static_cast<size_t>(i)], (*groupRow[l])[static_cast<size_t>(i)],
                                               row[static_cast<size_t>(i)]) != 0) {
                from = l;
                break;
            }
        }
        if (from < levels) {
            closeFrom(from);
            bool page = false;
            for (size_t l = from; l < levels; ++l) page = page || groups[l].second->newPage;
            if (page && !first) r.rows.push_back(rpt::Row::PageBreak());
            for (size_t l = from; l < levels; ++l) {
                rpt::Row h = rpt::Row::GroupHeader(headerText(l, row));
                h.indent = static_cast<int>(l);
                r.rows.push_back(std::move(h));
                groupRow[l] = &row;
            }
        }
        std::vector<rpt::Cell> cells;
        for (size_t k = 0; k < ncol; ++k) {
            const Col& c = cols[k];
            switch (c.def->calc) {
            case Calc::None: cells.push_back(cellOf(src->fields[static_cast<size_t>(c.a)], row[static_cast<size_t>(c.a)])); break;
            case Calc::Percent: cells.push_back(rpt::Cell::Amount(percent(raw(k, row), groupBase[k][at]))); break;
            default:
                cells.push_back(c.type == FieldType::Amount ? rpt::Cell::Amount(raw(k, row)) : rpt::Cell::Integer(raw(k, row)));
                break;
            }
        }
        r.rows.push_back(rpt::Row::Detail(std::move(cells), static_cast<int>(levels)));
        for (size_t l = 0; l < levels; ++l) addTo(groupSums[l], row);
        addTo(grand, row);
        first = false;
    }
    closeFrom(0);
    if (rows.empty()) r.rows.push_back(rpt::Row::Note("No rows match the report's filters."));
    else if (def.grandTotal && (anyTotal || def.count)) r.rows.push_back(rpt::Row::Total(totalCells(grand, "Total", -1), true));
    out.report = std::move(r);
    return out;
}

// ---- the sources (spec #7 4) -------------------------------------------------------------

namespace {

std::map<std::string, std::string> namesOf(cb4sqlite::Code4& code, const char* table, const char* codeField,
                                           const char* nameField)
{
    std::map<std::string, std::string> m;
    if (!hasTable(code, table)) return m;
    cb4sqlite::Query q(code, std::string("SELECT trim(") + codeField + "), trim(" + nameField + ") FROM " + table +
                                 " WHERE _deleted = 0");
    while (q.next()) m[q.text(0)] = q.text(1);
    return m;
}

std::string lookup(const std::map<std::string, std::string>& m, const std::string& k)
{
    auto it = m.find(TrimText(k));
    return it == m.end() ? std::string() : it->second;
}

const char* arStateName(const std::string& mode)
{
    if (mode == ar_mode::kOpen) return "Open";
    if (mode == ar_mode::kPosted) return "Posted";
    if (mode == ar_mode::kTransferred) return "Transferred";
    if (mode == ar_mode::kCancelled) return "Cancelled";
    return "?";
}

const char* glStateName(const std::string& mode)
{
    if (mode == "1") return "Open";
    if (mode == "0") return "Posted";
    if (mode == "3") return "Adjustment";
    if (mode == "2") return "Posted adjustment";
    if (mode == "9") return "Closed year";
    return "?";
}

const char* receiptStateName(const std::string& c)
{
    if (c == ar_receipt::kOpen) return "Owed";
    if (c == ar_receipt::kPartial) return "Part received";
    if (c == ar_receipt::kSettled) return "Settled";
    return "";
}

std::string accountText(const ApAccount& a) { return a.dept.empty() ? a.account : a.account + "-" + a.dept; }

std::map<std::string, std::string> customerNames(cb4sqlite::Code4& code)
{
    std::map<std::string, std::string> m;
    if (!hasTable(code, "ar_cust")) return m;
    for (const Customer& c : LoadCustomers(code)) m[TrimText(c.number)] = TrimText(c.name);
    return m;
}

Source salesLines()
{
    Source s;
    s.key = "sales_lines";
    s.name = "Sales lines";
    s.version = 1;
    s.description = "Each product line of the sales invoice vouchers, with its customer, product, med rep and doctor; "
                    "its amount split into sales and VAT as the ledger has it.";
    s.fields = {
        {"voucher", "Voucher", FieldType::Integer, 0.7},
        {"state", "State", FieldType::Text, 0.9},
        {"date", "Date", FieldType::Date, 1.0},
        {"period", "Period", FieldType::Integer, 0.5},
        {"customer", "Customer", FieldType::Text, 0.8, "customer_name"},
        {"customer_name", "Customer name", FieldType::Text, 2.2},
        {"invoice", "Invoice", FieldType::Text, 0.9},
        {"product", "Product", FieldType::Text, 0.8, "product_name"},
        {"product_name", "Product name", FieldType::Text, 2.2},
        {"med_rep", "Med rep", FieldType::Text, 0.8, "med_rep_name"},
        {"med_rep_name", "Med rep name", FieldType::Text, 1.8},
        {"doctor", "Doctor", FieldType::Text, 0.8, "doctor_name"},
        {"doctor_name", "Doctor name", FieldType::Text, 1.8},
        {"description", "Description", FieldType::Text, 2.2},
        {"account", "Sales account", FieldType::Text, 1.0},
        {"qty", "Quantity", FieldType::Amount, 0.9},
        {"price", "Unit price", FieldType::Amount, 1.0},
        {"amount", "Amount", FieldType::Amount, 1.2},
        {"sales", "Sales (net)", FieldType::Amount, 1.2},
        {"vat", "VAT", FieldType::Amount, 1.0},
    };
    s.defaults = {Filter{"state", Test::NotEqual, {"Cancelled"}, false, {}}};
    s.rows = [](cb4sqlite::Code4& code, const ReportContext&) {
        std::vector<SourceRow> out;
        if (!hasTable(code, "ar_btch")) return out;
        const auto customers = customerNames(code);
        const auto products = namesOf(code, "gl_itemm", "fpcode", "ffulln");
        const auto reps = namesOf(code, "gl_medrp", "repcode", "rpname");
        const auto doctors = namesOf(code, "gl_drcod", "drcode", "rpname");
        for (const ArVoucher& v : LoadArVouchers(code))
            for (const ArLine& l : LoadArLines(code, v.number)) {
                if (l.role != ArLineRole::Sales) continue;
                const std::int64_t gross = l.price != 0 ? LineAmount(l.qty, l.price, l.gross) : l.gross;
                out.push_back({number(v.number), text(arStateName(v.mode)),
                               text(v.invoiceDate.empty() ? v.date : v.invoiceDate), number(v.period), text(v.customer),
                               text(lookup(customers, v.customer)), text(v.invoiceNo), text(l.product),
                               text(lookup(products, l.product)), text(l.medrep), text(lookup(reps, l.medrep)),
                               text(l.drcode), text(lookup(doctors, l.drcode)), text(l.descript),
                               text(accountText(l.account)), number(l.qty), number(l.price), number(gross),
                               number(l.credit - l.debit), number(gross - (l.credit - l.debit))});
            }
        return out;
    };
    return s;
}

Source glLines()
{
    Source s;
    s.key = "gl_lines";
    s.name = "GL voucher lines";
    s.version = 1;
    s.description = "Each line of the General Ledger vouchers, open and posted, with its voucher's state and the "
                    "account's name.";
    s.fields = {
        {"voucher", "Voucher", FieldType::Integer, 0.7},
        {"line", "Line", FieldType::Integer, 0.4},
        {"state", "State", FieldType::Text, 0.9},
        {"date", "Date", FieldType::Date, 1.0},
        {"period", "Period", FieldType::Integer, 0.5},
        {"reference", "Reference", FieldType::Text, 1.8},
        {"description", "Description", FieldType::Text, 2.2},
        {"cheque", "Cheque", FieldType::Text, 0.8},
        {"account", "Account", FieldType::Text, 0.8, "account_name"},
        {"dept", "Dept", FieldType::Text, 0.5},
        {"account_name", "Account name", FieldType::Text, 2.2},
        {"med_rep", "Med rep", FieldType::Text, 0.8},
        {"product", "Product", FieldType::Text, 0.8},
        {"doctor", "Doctor", FieldType::Text, 0.8},
        {"debit", "Debit", FieldType::Amount, 1.2},
        {"credit", "Credit", FieldType::Amount, 1.2},
        {"net", "Net (Dr - Cr)", FieldType::Amount, 1.2},
    };
    s.rows = [](cb4sqlite::Code4& code, const ReportContext&) {
        std::vector<SourceRow> out;
        if (!hasTable(code, "gl_btchd")) return out;
        cb4sqlite::Query q(
            code,
            "SELECT d.batchno, d.journal, coalesce(trim(h.mode), trim(d.mode)), d.date, trim(d.period), d.reference, "
            "d.descript, d.cheque, trim(d.account), coalesce(trim(d.dept), ''), coalesce(a.descript, ''), d.medrep, "
            "d.prod, d.drcode, d.debit, d.credit "
            "FROM gl_btchd d "
            "LEFT JOIN (SELECT batchno, max(mode) AS mode FROM gl_btch WHERE _deleted = 0 GROUP BY batchno) h "
            "ON h.batchno = d.batchno "
            "LEFT JOIN (SELECT trim(account) AS acct, coalesce(trim(dept), '') AS dp, max(descript) AS descript "
            "FROM glaccts WHERE _deleted = 0 GROUP BY 1, 2) a "
            "ON a.acct = trim(d.account) AND a.dp = coalesce(trim(d.dept), '') "
            "WHERE d._deleted = 0 ORDER BY d.batchno, d.journal, d._rowid");
        while (q.next()) {
            const std::int64_t dr = rpt::ToCents(q.number(14)), cr = rpt::ToCents(q.number(15));
            std::int64_t period = 0;
            parseInteger(q.text(4), period);
            out.push_back({number(q.integer(0)), number(q.integer(1)), text(glStateName(TrimText(q.text(2)))),
                           text(q.text(3)), number(period), text(q.text(5)), text(q.text(6)), text(q.text(7)),
                           text(q.text(8)), text(q.text(9)), text(q.text(10)), text(q.text(11)), text(q.text(12)),
                           text(q.text(13)), number(dr), number(cr), number(dr - cr)});
        }
        return out;
    };
    return s;
}

Source arOpenItems()
{
    Source s;
    s.key = "ar_open_items";
    s.name = "AR open items";
    s.version = 1;
    s.description = "Each posted sales invoice: what it was for, what has been received, and what is owed.";
    s.fields = {
        {"voucher", "Voucher", FieldType::Integer, 0.7},
        {"customer", "Customer", FieldType::Text, 0.8, "customer_name"},
        {"customer_name", "Customer name", FieldType::Text, 2.2},
        {"invoice", "Invoice", FieldType::Text, 0.9},
        {"reference", "Reference", FieldType::Text, 1.8},
        {"date", "Date", FieldType::Date, 1.0},
        {"due_date", "Due date", FieldType::Date, 1.0},
        {"date_paid", "Last received", FieldType::Date, 1.0},
        {"period", "Period", FieldType::Integer, 0.5},
        {"state", "State", FieldType::Text, 1.1},
        {"ar_account", "AR account", FieldType::Text, 0.9},
        {"amount", "Amount", FieldType::Amount, 1.2},
        {"vat", "VAT", FieldType::Amount, 1.0},
        {"received", "Received", FieldType::Amount, 1.2},
        {"owed", "Owed", FieldType::Amount, 1.2},
    };
    s.rows = [](cb4sqlite::Code4& code, const ReportContext&) {
        std::vector<SourceRow> out;
        if (!hasTable(code, "ar_dist")) return out;
        const auto customers = customerNames(code);
        for (const ArOpenItem& i : LoadArOpenItems(code))
            out.push_back({number(i.batchno), text(i.customer), text(lookup(customers, i.customer)), text(i.invoiceNo),
                           text(i.reference), text(i.date), text(i.dueDate), text(i.datePaid), number(i.period),
                           text(receiptStateName(i.paymentCtl)), text(accountText(i.ar)), number(i.amount),
                           number(i.vat), number(i.received), number(i.remainder)});
        return out;
    };
    return s;
}

} // namespace

const std::vector<Source>& Sources()
{
    static const std::vector<Source> all = {salesLines(), glLines(), arOpenItems()};
    return all;
}

const Source* FindSource(const std::string& key)
{
    for (const Source& s : Sources())
        if (s.key == key) return &s;
    return nullptr;
}

// ---- gl_rptdef ------------------------------------------------------------------------

namespace {

StoredDefinition readStored(const cb4sqlite::Query& q)
{
    StoredDefinition d;
    d.number = static_cast<long>(q.integer(0));
    d.name = TrimText(q.text(1));
    d.description = TrimText(q.text(2));
    d.source = TrimText(q.text(3));
    d.sourceVersion = static_cast<int>(q.integer(4));
    d.text = q.text(5);
    d.madeBy = TrimText(q.text(6));
    d.madeOn = TrimText(q.text(7));
    d.changedBy = TrimText(q.text(8));
    d.changedOn = TrimText(q.text(9));
    return d;
}

const char* kStoredColumns = "SELECT number, name, descript, source, srcver, deftext, madeby, madeon, changedby, "
                             "changedon FROM gl_rptdef WHERE _deleted = 0";

std::string clip(const std::string& s, size_t n) { return s.size() > n ? s.substr(0, n) : s; }

} // namespace

void EnsureReportWriterTables(cb4sqlite::Code4& code)
{
    if (hasTable(code, "gl_rptdef")) return;
    cb4sqlite::Transaction tx(code);
    cb4sqlite::createTable(code, "gl_rptdef",
                           {{"NUMBER", 'N', 6},     {"NAME", 'C', 40},     {"DESCRIPT", 'C', 60},
                            {"SOURCE", 'C', 20},    {"SRCVER", 'N', 3},    {"DEFTEXT", 'M', 10},
                            {"MADEBY", 'C', 10},    {"MADEON", 'C', 14},   {"CHANGEDBY", 'C', 10},
                            {"CHANGEDON", 'C', 14}});
    cb4sqlite::createTag(code, "gl_rptdef", "number", "\"NUMBER\"");
    tx.commit();
}

std::vector<StoredDefinition> LoadDefinitions(cb4sqlite::Code4& code)
{
    std::vector<StoredDefinition> out;
    if (!hasTable(code, "gl_rptdef")) return out;
    cb4sqlite::Query q(code, std::string(kStoredColumns) + " ORDER BY lower(trim(name)), number");
    while (q.next()) out.push_back(readStored(q));
    return out;
}

std::optional<StoredDefinition> FindDefinition(cb4sqlite::Code4& code, long n)
{
    if (!hasTable(code, "gl_rptdef")) return std::nullopt;
    cb4sqlite::Query q(code, std::string(kStoredColumns) + " AND number = ?1 ORDER BY _rowid LIMIT 1");
    q.bindInt(1, n);
    if (!q.next()) return std::nullopt;
    return readStored(q);
}

long SaveDefinition(cb4sqlite::Code4& code, long n, const ReportDefinition& def, const std::string& user,
                    const std::string& stamp)
{
    EnsureReportWriterTables(code);
    cb4sqlite::Transaction tx(code);
    Data4 d(&code, "gl_rptdef");
    Field4 num(d, "number");
    bool found = false;
    if (n > 0)
        for (d.top(); !d.eof(); d.skip())
            if (!d.deleted() && std::atol(num.str()) == n) {
                found = true;
                break;
            }
    if (!found) {
        long next = 1;
        {
            cb4sqlite::Query q(code, "SELECT coalesce(max(number), 0) + 1 FROM gl_rptdef WHERE _deleted = 0");
            if (q.next()) next = static_cast<long>(q.integer(0));
        }
        n = next;
        d.appendStart();
        d.blank();
        num.assignLong(n);
        Field4(d, "madeby").assign(clip(user, 10).c_str());
        Field4(d, "madeon").assign(stamp.c_str());
    }
    Field4(d, "name").assign(clip(TrimText(def.name), 40).c_str());
    Field4(d, "descript").assign(clip(TrimText(def.description.empty() ? def.name : def.description), 60).c_str());
    Field4(d, "source").assign(clip(def.source, 20).c_str());
    Field4(d, "srcver").assignLong(def.sourceVersion);
    Field4(d, "deftext").assign(WriteDefinition(def).c_str());
    Field4(d, "changedby").assign(clip(user, 10).c_str());
    Field4(d, "changedon").assign(stamp.c_str());
    if (found) d.flush();
    else d.append();
    d.close();
    tx.commit();
    return n;
}

bool DeleteDefinition(cb4sqlite::Code4& code, long n)
{
    if (!hasTable(code, "gl_rptdef")) return false;
    cb4sqlite::Transaction tx(code);
    Data4 d(&code, "gl_rptdef");
    Field4 num(d, "number");
    bool any = false;
    for (d.top(); !d.eof(); d.skip())
        if (!d.deleted() && std::atol(num.str()) == n) {
            d.deleteRec();
            any = true;
        }
    if (any) d.pack();
    d.close();
    tx.commit();
    return any;
}

bool DefinitionNameTaken(cb4sqlite::Code4& code, const std::string& name, long except)
{
    const std::string want = norm(name);
    for (const StoredDefinition& d : LoadDefinitions(code))
        if (d.number != except && norm(d.name) == want) return true;
    return false;
}

// ---- the designer's checks --------------------------------------------------------------

std::optional<DefinitionProblem> CheckDefinition(cb4sqlite::Code4& code, const ReportDefinition& def, long except)
{
    const std::string name = TrimText(def.name);
    if (name.empty()) return DefinitionProblem{DefinitionSetting::Name, "Give the report a name."};
    if (name.size() > 40) return DefinitionProblem{DefinitionSetting::Name, "A report's name can be 40 characters at most."};
    if (DefinitionNameTaken(code, name, except))
        return DefinitionProblem{DefinitionSetting::Name, "Another report is already called \"" + name + "\"."};
    if (!FindSource(def.source)) return DefinitionProblem{DefinitionSetting::Source, "Choose the report's source."};
    if (def.columns.empty())
        return DefinitionProblem{DefinitionSetting::Columns, "Add at least one column: pick a field and Add."};
    const Source& src = *FindSource(def.source);
    for (const ColumnDef& c : def.columns)
        if (const std::string why = ColumnProblem(src, c); !why.empty())
            return DefinitionProblem{DefinitionSetting::Columns, why};
    size_t asked = 0;
    for (const Filter& f : def.filters) asked += f.ask ? 1 : 0;
    if (asked > kMaxQuestions)
        return DefinitionProblem{DefinitionSetting::Filters,
                                 "A report can ask at most " + std::to_string(kMaxQuestions) + " questions when it runs."};
    for (const Filter& f : def.filters)
        if (const std::string why = FilterProblem(src, f); !why.empty())
            return DefinitionProblem{DefinitionSetting::Filters, "A filter doesn't suit its field: " + why};
    return std::nullopt;
}

std::string FilterProblem(const Source& source, const Filter& filter)
{
    const int i = source.fieldIndex(filter.field);
    if (i < 0 || filter.ask) return {};   // a missing field is noted when run (D9); asked values are checked then
    const int want = valueCount(filter.test);
    if (want >= 0 && static_cast<int>(filter.values.size()) != want)
        return std::string("\"") + testWord(filter.test) + "\" takes " + std::to_string(want) + " value(s).";
    if (want < 0 && filter.values.empty()) return "\"in\" takes one value or more.";
    ResolvedFilter rf{i, &filter, filter.values, {}, {}};
    return prepare(source.fields[static_cast<size_t>(i)], rf);
}

std::string CopyName(cb4sqlite::Code4& code, const std::string& name)
{
    std::string base = "Copy of " + TrimText(name);
    if (base.size() > 40) base = base.substr(0, 40);
    if (!DefinitionNameTaken(code, base)) return base;
    for (int n = 2;; ++n) {
        const std::string tail = " (" + std::to_string(n) + ")";
        std::string candidate = base.size() + tail.size() > 40 ? base.substr(0, 40 - tail.size()) : base;
        candidate += tail;
        if (!DefinitionNameTaken(code, candidate)) return candidate;
    }
}

std::vector<std::string> MissingFields(const ReportDefinition& def)
{
    std::vector<std::string> out;
    const Source* src = FindSource(def.source);
    auto check = [&](const std::string& field) {
        if (src && src->fieldIndex(field) >= 0) return;
        if (std::find(out.begin(), out.end(), field) == out.end()) out.push_back(field);
    };
    if (!src) return out;
    for (const ColumnDef& c : def.columns) {
        if (c.calc != Calc::Count) check(c.field);
        if (c.calc == Calc::Sum || c.calc == Calc::Difference) check(c.field2);
    }
    for (const SortKey& k : def.sort) check(k.field);
    for (const GroupDef& g : def.groups) check(g.field);
    for (const Filter& f : def.filters) check(f.field);
    return out;
}

std::string DescribeFilter(const Source& source, const Filter& filter)
{
    const int i = source.fieldIndex(filter.field);
    if (i < 0) return filter.field + " (not a field of " + source.name + ")";
    const Field& f = source.fields[static_cast<size_t>(i)];
    if (filter.ask) return f.label + ": asked when run" + (filter.question.empty() ? std::string() : " (" + filter.question + ")");
    const int want = valueCount(filter.test);
    if ((want > 0 && static_cast<int>(filter.values.size()) != want) || (want < 0 && filter.values.empty()))
        return f.label + " " + testWord(filter.test) + " ?";
    return describe(f, filter, filter.values);
}

} // namespace rw
