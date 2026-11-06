//
// Schema.cpp - table layout and _cb_* metadata (spec 4.3).
//

#include "Schema.h"
#include "Code4.h"
#include "Internal.h"

#include <set>

namespace cb4sqlite {

using detail::Stmt;

std::string normaliseTableName(const char* name)
{
    std::string_view s(name ? name : "");
    if (const size_t slash = s.find_last_of("\\/:"); slash != std::string_view::npos)
        s.remove_prefix(slash + 1);
    if (s.size() > 4 && detail::iequals(s.substr(s.size() - 4), ".dbf"))
        s.remove_suffix(4);
    return detail::lower(s);
}

std::string quoteIdent(const std::string& ident)
{
    std::string out = "\"";
    for (char c : ident) {
        if (c == '"') out += '"';
        out += c;
    }
    return out + "\"";
}

static sqlite3* requireDb(Code4& code, const char* method)
{
    if (!code.handle())
        throw Error(method, "no database open");
    return code.handle();
}

void ensureMetaTables(Code4& code)
{
    sqlite3* db = requireDb(code, "ensureMetaTables");
    detail::exec(db,
        "CREATE TABLE IF NOT EXISTS _cb_fields ("
        " table_name TEXT NOT NULL, field_no INTEGER NOT NULL, field_name TEXT NOT NULL,"
        " type TEXT NOT NULL, len INTEGER NOT NULL, dec INTEGER NOT NULL DEFAULT 0,"
        " PRIMARY KEY (table_name, field_no));"
        "CREATE TABLE IF NOT EXISTS _cb_tags ("
        " table_name TEXT NOT NULL, tag_name TEXT NOT NULL, key_expr TEXT NOT NULL,"
        " PRIMARY KEY (table_name, tag_name));",
        "ensureMetaTables");
}

void createTable(Code4& code, const char* tableName, const std::vector<FieldDef>& fields)
{
    static const char* M = "createTable";
    sqlite3* db = requireDb(code, M);
    const std::string table = normaliseTableName(tableName);
    if (table.empty())
        throw Error(M, "empty table name");
    if (fields.empty())
        throw Error(M, "table '" + table + "' has no fields");

    std::string sql = "CREATE TABLE " + quoteIdent(table) +
                      " (_rowid INTEGER PRIMARY KEY, _deleted INTEGER NOT NULL DEFAULT 0";
    std::set<std::string> seen;
    std::vector<FieldDef> defs = fields;
    for (FieldDef& f : defs) {
        if (f.name.empty() || !seen.insert(detail::lower(f.name)).second)
            throw Error(M, "empty or duplicate field name '" + f.name + "' in " + table);
        const char* sqlType = nullptr;
        switch (f.type) {
            case r4str:   sqlType = "TEXT"; break;
            case r4date:  sqlType = "TEXT"; f.len = 8; break;
            case r4log:   sqlType = "INTEGER"; f.len = 1; break;
            case r4num:
            case r4float: sqlType = (f.dec == 0) ? "INTEGER" : "REAL"; break;
            default:
                throw Error(M, std::string("unsupported field type '") + f.type + "' for " + f.name);
        }
        if (f.len <= 0 || f.dec < 0 || (detail::isNumericType(f.type) && f.dec >= f.len))
            throw Error(M, "bad width/decimals for field " + f.name);
        sql += ", " + quoteIdent(f.name) + " " + sqlType;
    }
    sql += ")";

    ensureMetaTables(code);
    detail::exec(db, "SAVEPOINT cb_create", M);
    try {
        detail::exec(db, sql, M);
        Stmt ins(db, "INSERT INTO _cb_fields (table_name, field_no, field_name, type, len, dec)"
                     " VALUES (?1, ?2, ?3, ?4, ?5, ?6)", M);
        int no = 1;
        for (const FieldDef& f : defs) {
            sqlite3_reset(ins.get());
            ins.bindText(1, table);
            ins.bindInt(2, no++);
            ins.bindText(3, f.name);
            ins.bindText(4, std::string(1, f.type));
            ins.bindInt(5, f.len);
            ins.bindInt(6, f.dec);
            ins.step(M);
        }
        detail::exec(db, "RELEASE cb_create", M);
    } catch (...) {
        sqlite3_exec(db, "ROLLBACK TO cb_create; RELEASE cb_create", nullptr, nullptr, nullptr);
        throw;
    }
}

void createTag(Code4& code, const char* tableName, const char* tag, const char* keyExpr)
{
    static const char* M = "createTag";
    sqlite3* db = requireDb(code, M);
    const std::string table = normaliseTableName(tableName);
    const std::string name  = detail::lower(tag ? tag : "");
    if (name.empty() || !keyExpr || !*keyExpr)
        throw Error(M, "empty tag name or key expression");

    TableInfo info;
    if (!loadTableInfo(db, table, info))
        throw Error(M, "no such table: " + table);

    detail::exec(db, "SAVEPOINT cb_tag", M);
    try {
        Stmt ins(db, "INSERT INTO _cb_tags (table_name, tag_name, key_expr) VALUES (?1, ?2, ?3)", M);
        ins.bindText(1, table);
        ins.bindText(2, name);
        ins.bindText(3, keyExpr);
        ins.step(M);
        detail::exec(db, "CREATE INDEX " + quoteIdent(table + "__" + name) + " ON " + quoteIdent(table) +
                         " ((" + std::string(keyExpr) + "), _rowid)", M);
        detail::exec(db, "RELEASE cb_tag", M);
    } catch (...) {
        sqlite3_exec(db, "ROLLBACK TO cb_tag; RELEASE cb_tag", nullptr, nullptr, nullptr);
        throw;
    }
}

bool loadTableInfo(sqlite3* db, const std::string& tableName, TableInfo& out)
{
    static const char* M = "loadTableInfo";
    const std::string table = tableName;   // tableName may alias out.name
    out = TableInfo{};
    out.name = table;
    {
        Stmt q(db, "SELECT count(*) FROM sqlite_master WHERE type = 'table'"
                   " AND name IN (?1, '_cb_fields', '_cb_tags')", M);
        q.bindText(1, table);
        q.step(M);
        if (sqlite3_column_int(q.get(), 0) != 3)
            return false;
    }
    {
        Stmt q(db, "SELECT field_name, type, len, dec FROM _cb_fields"
                   " WHERE table_name = ?1 ORDER BY field_no", M);
        q.bindText(1, table);
        while (q.step(M)) {
            FieldDef f;
            f.name = reinterpret_cast<const char*>(sqlite3_column_text(q.get(), 0));
            const auto* t = reinterpret_cast<const char*>(sqlite3_column_text(q.get(), 1));
            f.type = (t && *t) ? t[0] : r4str;
            f.len  = sqlite3_column_int(q.get(), 2);
            f.dec  = sqlite3_column_int(q.get(), 3);
            out.fields.push_back(std::move(f));
        }
    }
    if (out.fields.empty())
        return false;
    {
        Stmt q(db, "SELECT tag_name, key_expr FROM _cb_tags WHERE table_name = ?1 ORDER BY rowid", M);
        q.bindText(1, table);
        while (q.step(M)) {
            TagDef t;
            t.name    = reinterpret_cast<const char*>(sqlite3_column_text(q.get(), 0));
            t.keyExpr = reinterpret_cast<const char*>(sqlite3_column_text(q.get(), 1));
            out.tags.push_back(std::move(t));
        }
    }
    return true;
}

} // namespace cb4sqlite
