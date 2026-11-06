//
// Internal.cpp - statement wrapper and dBase text-image conversions.
//

#include "Internal.h"

#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>

namespace cb4sqlite::detail {

// --- Stmt --------------------------------------------------------------------

Stmt::Stmt(sqlite3* db, const std::string& sql, const char* method)
    : db_(db), method_(method)
{
    if (sqlite3_prepare_v2(db, sql.c_str(), static_cast<int>(sql.size()), &s_, nullptr) != SQLITE_OK)
        throwSqlite(db, method, sql.c_str());
}

bool Stmt::step(const char* method)
{
    const int rc = sqlite3_step(s_);
    if (rc == SQLITE_ROW)  return true;
    if (rc == SQLITE_DONE) return false;
    throwSqlite(db_, method, sqlite3_sql(s_));
}

void Stmt::bindText(int i, std::string_view v)
{
    check(sqlite3_bind_text(s_, i, v.data(), static_cast<int>(v.size()), SQLITE_TRANSIENT));
}

void Stmt::check(int rc)
{
    if (rc != SQLITE_OK)
        throwSqlite(db_, method_, "bind");
}

void exec(sqlite3* db, const std::string& sql, const char* method)
{
    char* err = nullptr;
    if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &err) != SQLITE_OK) {
        std::string msg = (err ? err : "unknown error");
        sqlite3_free(err);
        throw Error(method, msg + " [" + sql + "]");
    }
}

void throwSqlite(sqlite3* db, const char* method, const char* what)
{
    throw Error(method, std::string(db ? sqlite3_errmsg(db) : "no database") + " [" + what + "]");
}

// --- strings -----------------------------------------------------------------

std::string fitText(std::string_view text, size_t n)
{
    if (text.size() > n) {
        size_t cut = n;
        // Back up over UTF-8 continuation bytes so a sequence is never split.
        while (cut > 0 && (static_cast<unsigned char>(text[cut]) & 0xC0) == 0x80)
            --cut;
        text = text.substr(0, cut);
    }
    std::string out(text);
    out.resize(n, ' ');
    return out;
}

std::string_view trimRight(std::string_view s) noexcept
{
    while (!s.empty() && (s.back() == ' ' || s.back() == '\0'))
        s.remove_suffix(1);
    return s;
}

std::string_view trim(std::string_view s) noexcept
{
    s = trimRight(s);
    while (!s.empty() && s.front() == ' ')
        s.remove_prefix(1);
    return s;
}

bool iequals(std::string_view a, std::string_view b) noexcept
{
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); ++i)
        if (std::tolower(static_cast<unsigned char>(a[i])) != std::tolower(static_cast<unsigned char>(b[i])))
            return false;
    return true;
}

std::string lower(std::string_view s)
{
    std::string out(s);
    for (char& c : out)
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return out;
}

// --- dBase text images ---------------------------------------------------------

bool isNumericType(char type) noexcept
{
    return type == r4num || type == r4float;
}

std::string blankImage(const FieldDef& f)
{
    return std::string(static_cast<size_t>(f.len), ' ');
}

std::string imageFromDouble(const FieldDef& f, double v, int dec)
{
    if (dec < 0) dec = 0;
    char buf[512];
    const int n = std::snprintf(buf, sizeof buf, "%*.*f", f.len, dec, v);
    if (n < 0 || n > f.len || !std::isfinite(v))
        return std::string(static_cast<size_t>(f.len), '*');   // dBase overflow marker
    return std::string(buf, static_cast<size_t>(n));
}

std::string imageFromLong(const FieldDef& f, long v, bool zerosInFront)
{
    if (isNumericType(f.type))
        return imageFromDouble(f, static_cast<double>(v), f.dec);
    char buf[64];
    const int n = zerosInFront ? std::snprintf(buf, sizeof buf, "%0*ld", f.len, v)
                               : std::snprintf(buf, sizeof buf, "%*ld", f.len, v);
    if (n < 0 || n > f.len)
        return std::string(static_cast<size_t>(f.len), '*');
    return std::string(buf, static_cast<size_t>(n));
}

std::string imageFromText(const FieldDef& f, std::string_view text)
{
    // Stop at an embedded NUL, as the original C strings did.
    if (const size_t z = text.find('\0'); z != std::string_view::npos)
        text = text.substr(0, z);

    if (isNumericType(f.type)) {
        const std::string t(trim(text));
        if (t.empty())
            return blankImage(f);
        char* end = nullptr;
        const double v = std::strtod(t.c_str(), &end);
        if (end == t.c_str())
            return blankImage(f);                 // not a number: dBase reads it as 0
        return imageFromDouble(f, v, f.dec);
    }
    if (f.type == r4log) {
        const std::string_view t = trim(text);
        if (t.empty()) return " ";
        switch (t.front()) {
            case 'T': case 't': case 'Y': case 'y': return "T";
            case 'F': case 'f': case 'N': case 'n': return "F";
            default: return " ";
        }
    }
    return fitText(text, static_cast<size_t>(f.len));   // C and D
}

std::string imageFromColumn(const FieldDef& f, sqlite3_stmt* s, int col)
{
    const int t = sqlite3_column_type(s, col);
    if (t == SQLITE_NULL)
        return blankImage(f);
    if (isNumericType(f.type))
        return imageFromDouble(f, sqlite3_column_double(s, col), f.dec);
    if (f.type == r4log)
        return sqlite3_column_int64(s, col) ? "T" : "F";
    const auto* p = reinterpret_cast<const char*>(sqlite3_column_text(s, col));
    const int   n = sqlite3_column_bytes(s, col);
    return fitText(std::string_view(p ? p : "", static_cast<size_t>(n)), static_cast<size_t>(f.len));
}

void bindImage(Stmt& s, int i, const FieldDef& f, const std::string& image)
{
    if (isNumericType(f.type)) {
        const std::string t(trim(image));
        if (t.empty() || t.front() == '*') { s.bindNull(i); return; }
        const double v = std::strtod(t.c_str(), nullptr);
        if (f.dec == 0 && std::fabs(v) < 9.0e15)
            s.bindInt(i, static_cast<long long>(std::llround(v)));
        else
            s.bindDouble(i, v);
        return;
    }
    if (f.type == r4log) {
        if (image == "T")      s.bindInt(i, 1);
        else if (image == "F") s.bindInt(i, 0);
        else                   s.bindNull(i);
        return;
    }
    if (f.type == r4date && trim(image).empty()) {
        s.bindNull(i);
        return;
    }
    // Character: always stored right-padded to the declared width (spec 4.3),
    // even if trim() shortened the buffered copy.
    s.bindText(i, fitText(image, static_cast<size_t>(f.len)));
}

} // namespace cb4sqlite::detail
