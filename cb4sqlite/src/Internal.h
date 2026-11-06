#pragma once
//
// Internal.h - helpers shared by the cb4sqlite sources. Not a public header.
//

#include "cb4sqlite_compat.h"
#include "Errors.h"
#include "Schema.h"

#include "../sqlite3.h"

#include <string>
#include <string_view>

namespace cb4sqlite::detail {

// Owns one prepared statement.
class Stmt {
public:
    Stmt(sqlite3* db, const std::string& sql, const char* method);
    ~Stmt() { sqlite3_finalize(s_); }
    Stmt(const Stmt&) = delete;
    Stmt& operator=(const Stmt&) = delete;

    sqlite3_stmt* get() const noexcept { return s_; }
    bool step(const char* method);           // true = row, false = done; throws on error
    void bindText(int i, std::string_view v);
    void bindInt(int i, long long v)  { check(sqlite3_bind_int64(s_, i, v)); }
    void bindDouble(int i, double v)  { check(sqlite3_bind_double(s_, i, v)); }
    void bindNull(int i)              { check(sqlite3_bind_null(s_, i)); }
    void bindValue(int i, const sqlite3_value* v) { check(sqlite3_bind_value(s_, i, v)); }

private:
    void check(int rc);
    sqlite3*      db_;
    sqlite3_stmt* s_ = nullptr;
    const char*   method_;
};

// Runs SQL with no result rows. Throws Error on failure.
void exec(sqlite3* db, const std::string& sql, const char* method);

// [[noreturn]] wrapper that adds sqlite3_errmsg.
[[noreturn]] void throwSqlite(sqlite3* db, const char* method, const char* what);

// --- dBase text images ------------------------------------------------------
bool        isNumericType(char type) noexcept;          // N or F
std::string blankImage(const FieldDef& f);
std::string imageFromText(const FieldDef& f, std::string_view text);
std::string imageFromDouble(const FieldDef& f, double v, int dec);
std::string imageFromLong(const FieldDef& f, long v, bool zerosInFront);
std::string imageFromColumn(const FieldDef& f, sqlite3_stmt* s, int col);
void        bindImage(Stmt& s, int i, const FieldDef& f, const std::string& image);

// Truncates to at most n bytes without splitting a UTF-8 sequence, then pads
// with spaces to exactly n bytes.
std::string fitText(std::string_view text, size_t n);
std::string_view trimRight(std::string_view s) noexcept;
std::string_view trim(std::string_view s) noexcept;
bool        iequals(std::string_view a, std::string_view b) noexcept;
std::string lower(std::string_view s);

} // namespace cb4sqlite::detail
