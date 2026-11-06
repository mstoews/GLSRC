#pragma once
//
// Schema - how a dBase table is laid out in the company SQLite file (spec 4.3).
//
// Every migrated table T has:
//   * the SQL table "T": _rowid INTEGER PRIMARY KEY, _deleted INTEGER NOT NULL
//     DEFAULT 0, then one column per dBase field;
//   * rows in _cb_fields (table_name, field_no, field_name, type, len, dec)
//     giving the dBase type and declared width of each field, in record order;
//   * rows in _cb_tags (table_name, tag_name, key_expr), one per index tag.
//     key_expr is an SQL expression over the table's columns that produces the
//     same key the dBase tag expression did, e.g. `account || dept` for
//     ACCOUNT+DEPT. Character columns are stored right-padded to their width,
//     so concatenation reproduces the fixed-width dBase key byte for byte.
//
// Table and tag names are stored lower-case. Field names keep the case they
// were created with (dBase field names are upper-case) and are matched
// case-insensitively.
//
// createTable()/createTag() are used by the tests and by dbf2sqlite_core (M2).
//

#include <string>
#include <vector>

struct sqlite3;

namespace cb4sqlite {

class Code4;

struct FieldDef {
    std::string name;
    char        type = 'C';   // r4str, r4num, r4float, r4date, r4log
    int         len  = 0;     // declared width in bytes (dates: 8, logicals: 1)
    int         dec  = 0;     // decimals, numeric fields only
};

struct TagDef {
    std::string name;
    std::string keyExpr;
};

struct TableInfo {
    std::string           name;    // normalised SQL table name
    std::vector<FieldDef> fields;  // record order; field number = index + 1
    std::vector<TagDef>   tags;    // creation order; first one is the default tag
};

// "C:\\co\\GLACCTS.DBF" -> "glaccts"; "access.psw" -> "access.psw".
// Strips any directory and a trailing ".dbf", lower-cases the rest.
std::string normaliseTableName(const char* name);

// Double-quotes an SQL identifier.
std::string quoteIdent(const std::string& ident);

// Creates the _cb_* metadata tables if they don't exist yet.
void ensureMetaTables(Code4& code);

// Creates table + metadata. Throws Error on SQL failure or a bad definition
// (unknown type, zero width, duplicate field).
void createTable(Code4& code, const char* table, const std::vector<FieldDef>& fields);

// Registers a tag and creates a matching SQLite index on (key_expr, _rowid).
void createTag(Code4& code, const char* table, const char* tag, const char* keyExpr);

// Loads metadata. Returns false if the table or its _cb_fields rows are missing.
bool loadTableInfo(sqlite3* db, const std::string& table, TableInfo& out);

} // namespace cb4sqlite
