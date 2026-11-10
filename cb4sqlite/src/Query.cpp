//
// Query.cpp - see Query.h.
//
#include "Query.h"

#include "Code4.h"
#include "Internal.h"

#include <cstdlib>
#include <string>

namespace cb4sqlite {

namespace {

constexpr const char* kMethod = "Query";

bool blank(std::string_view s) noexcept
{
    return detail::trim(s).empty();
}

} // namespace

Query::Query(Code4& code, std::string_view sql) : sql_(sql)
{
    if (!code.isOpen()) throw Error(kMethod, "no database is open, for: " + sql_);
    const int rc = sqlite3_prepare_v2(code.handle(), sql_.c_str(), static_cast<int>(sql_.size()), &stmt_, nullptr);
    if (rc != SQLITE_OK || !stmt_) {
        const std::string msg = sqlite3_errmsg(code.handle());
        sqlite3_finalize(stmt_);
        stmt_ = nullptr;
        throw Error(kMethod, msg + ", in: " + sql_);
    }
    if (!sqlite3_stmt_readonly(stmt_)) {
        sqlite3_finalize(stmt_);
        stmt_ = nullptr;
        throw Error(kMethod, "only read-only statements are allowed, not: " + sql_);
    }
}

Query::~Query()
{
    sqlite3_finalize(stmt_);
}

void Query::fail(const char* what) const
{
    throw Error(kMethod, std::string(what) + ": " + sqlite3_errmsg(sqlite3_db_handle(stmt_)) + ", in: " + sql_);
}

void Query::checkBind(int rc) const
{
    if (rc != SQLITE_OK) fail("bind");
}

void Query::checkColumn(int col) const
{
    if (!row_) throw Error(kMethod, "no current row, in: " + sql_);
    if (col < 0 || col >= sqlite3_column_count(stmt_))
        throw Error(kMethod, "column " + std::to_string(col) + " out of range, in: " + sql_);
}

Query& Query::bindText(int param, std::string_view value)
{
    checkBind(sqlite3_bind_text(stmt_, param, value.data(), static_cast<int>(value.size()), SQLITE_TRANSIENT));
    return *this;
}

Query& Query::bindInt(int param, long long value)
{
    checkBind(sqlite3_bind_int64(stmt_, param, value));
    return *this;
}

Query& Query::bindDouble(int param, double value)
{
    checkBind(sqlite3_bind_double(stmt_, param, value));
    return *this;
}

Query& Query::bindNull(int param)
{
    checkBind(sqlite3_bind_null(stmt_, param));
    return *this;
}

bool Query::next()
{
    const int rc = sqlite3_step(stmt_);
    if (rc == SQLITE_ROW) return row_ = true;
    row_ = false;
    if (rc == SQLITE_DONE) return false;
    fail("step");
}

void Query::reset()
{
    sqlite3_reset(stmt_);
    row_ = false;
}

int Query::columns() const noexcept
{
    return sqlite3_column_count(stmt_);
}

bool Query::isNull(int col) const
{
    checkColumn(col);
    return sqlite3_column_type(stmt_, col) == SQLITE_NULL;
}

std::string Query::text(int col) const
{
    checkColumn(col);
    const auto* p = reinterpret_cast<const char*>(sqlite3_column_text(stmt_, col));
    if (!p) return {};
    const std::string_view s(p, static_cast<size_t>(sqlite3_column_bytes(stmt_, col)));
    return std::string(detail::trimRight(s));
}

double Query::number(int col) const
{
    checkColumn(col);
    switch (sqlite3_column_type(stmt_, col)) {
    case SQLITE_NULL:
        return 0.0;
    case SQLITE_INTEGER:
    case SQLITE_FLOAT:
        return sqlite3_column_double(stmt_, col);
    default: {
        const std::string s = text(col);
        return blank(s) ? 0.0 : std::strtod(s.c_str(), nullptr);
    }
    }
}

long long Query::integer(int col) const
{
    checkColumn(col);
    switch (sqlite3_column_type(stmt_, col)) {
    case SQLITE_NULL:
        return 0;
    case SQLITE_INTEGER:
        return sqlite3_column_int64(stmt_, col);
    case SQLITE_FLOAT:
        return static_cast<long long>(sqlite3_column_double(stmt_, col));
    default: {
        const std::string s = text(col);
        return blank(s) ? 0 : std::strtoll(s.c_str(), nullptr, 10);
    }
    }
}

} // namespace cb4sqlite
