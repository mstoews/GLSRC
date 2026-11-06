//
// Layer-1 tests: Code4 session and Schema metadata.
//

#include "TestDb.h"

using namespace test;
using Catch::Matchers::ContainsSubstring;

TEST_CASE("Code4::open refuses a missing file unless create is set", "[code4]")
{
    TempDb db;
    Code4 code;
    CHECK(code.open(db.path().c_str()) == e4open);
    CHECK(code.errorCode == e4open);
    CHECK_FALSE(code.isOpen());

    CHECK(code.open(db.path().c_str(), true) == r4success);
    CHECK(code.isOpen());
    CHECK(code.errorCode == 0);
    CHECK(raw(code, "SELECT count(*) FROM sqlite_master WHERE name IN ('_cb_fields','_cb_tags')") == "2");
    CHECK(raw(code, "PRAGMA journal_mode") == "delete");
    CHECK(raw(code, "PRAGMA foreign_keys") == "1");
}

TEST_CASE("Code4 close and reopen keeps tables and rows", "[code4]")
{
    TempDb db;
    {
        Code4 code;
        db.open(code);
        createAccounts(code);
        Data4 d(code, "accts");
        addSampleAccounts(d);
    }   // Data4 then Code4 destroyed: everything closed
    Code4 code;
    REQUIRE(code.open(db.path().c_str()) == r4success);
    Data4 d(&code, "accts");
    REQUIRE(d.isValid());
    CHECK(d.recCount() == 5);
}

TEST_CASE("Code4::closeAll invalidates open tables but keeps the connection", "[code4]")
{
    TempDb db;
    Code4 code;
    db.open(code);
    createAccounts(code);
    Data4 d(code, "accts");
    addSampleAccounts(d);
    Field4 account(d, "account");

    CHECK(code.closeAll() == r4success);
    CHECK_FALSE(d.isValid());
    CHECK(d.top() == e4data);
    CHECK(std::string(account.str()) == "       ");   // blanks, not a dangling buffer
    CHECK(code.isOpen());
    CHECK(code.init() == 0);
    CHECK(code.initUndo() == 0);

    Data4 again(code, "accts");                         // the legacy reopen pattern
    CHECK(again.isValid());
    CHECK(again.recCount() == 5);
}

TEST_CASE("Data4 on a missing table is invalid, not an exception", "[code4]")
{
    TempDb db;
    Code4 code;
    db.open(code);
    Data4 d(code, "nosuch");
    CHECK_FALSE(d.isValid());
    CHECK(code.errorCode == e4open);
    CHECK(d.top() == e4data);
    CHECK(d.seek("x") == e4data);
    CHECK(d.recCount() == 0);
    Field4 f(d, 1);
    CHECK_FALSE(f.isValid());
    CHECK(std::string(f.str()).empty());
    CHECK(f.assign("x") == e4fieldName);

    Code4 closed;                                       // no database at all
    Data4 none(closed, "accts");
    CHECK_FALSE(none.isValid());
}

TEST_CASE("Table names are normalised like dBase file names", "[schema]")
{
    CHECK(cb4sqlite::normaliseTableName("C:\\CO\\GLACCTS.DBF") == "glaccts");
    CHECK(cb4sqlite::normaliseTableName("data/ACCESSM") == "accessm");
    CHECK(cb4sqlite::normaliseTableName("access.psw") == "access.psw");
    CHECK(cb4sqlite::normaliseTableName("GlDept.dbf") == "gldept");
    CHECK(cb4sqlite::quoteIdent("a\"b") == "\"a\"\"b\"");
}

TEST_CASE("createTable stores dBase types, widths and SQL column types", "[schema]")
{
    TempDb db;
    Code4 code;
    db.open(code);
    createAccounts(code);

    CHECK(raw(code, "SELECT group_concat(field_name || ':' || type || len || '.' || dec, ' ')"
                    " FROM (SELECT * FROM _cb_fields WHERE table_name='accts' ORDER BY field_no)") ==
          "ACCOUNT:C7.0 DEPT:C3.0 DESCRIPT:C20.0 CURR_BAL:N12.2 COUNT:N5.0 OPENED:D8.0 ACTIVE:L1.0");
    CHECK(raw(code, "SELECT group_concat(tag_name || '=' || key_expr, '; ')"
                    " FROM (SELECT * FROM _cb_tags WHERE table_name='accts' ORDER BY rowid)") ==
          "accounts=account || dept; descrip=descript; balance=curr_bal");
    CHECK(raw(code, "SELECT type FROM pragma_table_info('accts') WHERE name='CURR_BAL'") == "REAL");
    CHECK(raw(code, "SELECT type FROM pragma_table_info('accts') WHERE name='COUNT'") == "INTEGER");
    CHECK(raw(code, "SELECT count(*) FROM sqlite_master WHERE type='index' AND name='accts__accounts'") == "1");
}

TEST_CASE("createTable and createTag reject bad definitions", "[schema]")
{
    TempDb db;
    Code4 code;
    db.open(code);
    CHECK_THROWS_AS(cb4sqlite::createTable(code, "t", {}), cb4sqlite::Error);
    CHECK_THROWS_AS(cb4sqlite::createTable(code, "t", {{"A", 'X', 5, 0}}), cb4sqlite::Error);
    CHECK_THROWS_AS(cb4sqlite::createTable(code, "t", {{"A", r4str, 0, 0}}), cb4sqlite::Error);
    CHECK_THROWS_AS(cb4sqlite::createTable(code, "t", {{"A", r4str, 5, 0}, {"a", r4str, 5, 0}}),
                    cb4sqlite::Error);
    CHECK_THROWS_WITH(cb4sqlite::createTag(code, "nosuch", "x", "a"), ContainsSubstring("no such table"));

    cb4sqlite::createTable(code, "t", {{"A", r4str, 5, 0}});
    CHECK_THROWS_AS(cb4sqlite::createTag(code, "t", "bad", "no_such_column"), cb4sqlite::Error);
    // The failed tag left no metadata behind.
    CHECK(raw(code, "SELECT count(*) FROM _cb_tags WHERE table_name='t'") == "0");
    // A failed table creation (duplicate) leaves the original intact.
    CHECK_THROWS_AS(cb4sqlite::createTable(code, "t", {{"B", r4str, 5, 0}}), cb4sqlite::Error);
    CHECK(raw(code, "SELECT count(*) FROM _cb_fields WHERE table_name='t'") == "1");
}
