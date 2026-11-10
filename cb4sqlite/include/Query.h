#pragma once
//
// Query - a read-only SQL query on the open company database (spec #3 7).
//
// Reports read with SQL, which joins, orders and sums more simply than Data4
// tag walks; every write stays with Data4 (spec #2 5.2). The statement must
// be read-only (sqlite3_stmt_readonly): anything else throws.
//
//   cb4sqlite::Query q(cb, "SELECT ACCOUNT, CURR_BAL FROM glaccts "
//                          "WHERE _deleted = 0 AND TYPE = ?1 ORDER BY ACCOUNT, DEPT");
//   q.bindText(1, "B");
//   while (q.next()) use(q.text(0), q.number(1));
//
// Migrated character columns are space-padded, so text() trims trailing
// blanks. number() reads a missing number (NULL, or blank text) as 0, as
// Field4 does. Columns are numbered from 0, parameters from 1 (SQLite's
// conventions). Errors throw cb4sqlite::Error with the SQL in the message.
//
#include <string>
#include <string_view>

struct sqlite3_stmt;

namespace cb4sqlite {

class Code4;

class Query {
public:
    Query(Code4& code, std::string_view sql);
    ~Query();
    Query(const Query&) = delete;
    Query& operator=(const Query&) = delete;

    Query& bindText(int param, std::string_view value);
    Query& bindInt(int param, long long value);
    Query& bindDouble(int param, double value);
    Query& bindNull(int param);

    // Steps to the next row: true if there is one.
    bool next();
    // Back to before the first row; bindings are kept.
    void reset();

    int         columns() const noexcept;
    bool        isNull(int col) const;
    std::string text(int col) const;        // right-trimmed; NULL -> ""
    double      number(int col) const;      // NULL or blank -> 0.0
    long long   integer(int col) const;     // NULL or blank -> 0

private:
    [[noreturn]] void fail(const char* what) const;
    void checkBind(int rc) const;
    void checkColumn(int col) const;

    sqlite3_stmt* stmt_ = nullptr;
    std::string   sql_;
    bool          row_ = false;
};

} // namespace cb4sqlite
