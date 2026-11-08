//
// Layer-1 tests: Code4::backupTo (spec #2 5.3).
//

#include "TestDb.h"

#include <filesystem>

using namespace test;
using Catch::Matchers::ContainsSubstring;
namespace fs = std::filesystem;

TEST_CASE("backupTo copies the database, including the changed record in a buffer", "[backup]")
{
    TempDb db, copy;
    Code4 code;
    db.open(code);
    createAccounts(code);
    Data4 d(code, "accts");
    addSampleAccounts(d);
    REQUIRE(d.SetIndex("accounts") == r4success);
    REQUIRE(d.seek("100    01") == r4success);
    Field4(d, "descript").assign("Cash edited");   // still only in the buffer

    code.backupTo(copy.path().c_str());
    CHECK_FALSE(fs::exists(copy.path() + ".part"));

    Code4 other;
    REQUIRE(other.open(copy.path().c_str()) == r4success);
    CHECK(raw(other, "SELECT count(*) FROM accts") == "5");
    CHECK(raw(other, "SELECT rtrim(descript) FROM accts WHERE account LIKE '100%' AND dept = '01 '") == "Cash edited");
    Data4 o(other, "accts");                 // tags and field info came along
    REQUIRE(o.SetIndex("accounts") == r4success);
    CHECK(walk(o) == std::vector<std::string>{"100/01", "100/02", "200/01", "300/01", "400/01"});
}

TEST_CASE("backupTo replaces an existing file", "[backup]")
{
    TempDb db, copy;
    Code4 code;
    db.open(code);
    createAccounts(code);
    {
        Data4 d(code, "accts");
        addSampleAccounts(d);
    }
    code.backupTo(copy.path().c_str());
    {
        Data4 d(code, "accts");
        addAccount(d, "900", "01", "Later", 0);
    }
    code.backupTo(copy.path().c_str());
    Code4 other;
    REQUIRE(other.open(copy.path().c_str()) == r4success);
    CHECK(raw(other, "SELECT count(*) FROM accts") == "6");
}

TEST_CASE("backupTo failures leave no file behind", "[backup]")
{
    TempDb db;
    Code4 code;
    db.open(code);
    createAccounts(code);
    const fs::path dir = fs::temp_directory_path() / "cb4sqlite_no_such_dir_xyz";
    const std::string target = (dir / "copy.db").string();
    CHECK_THROWS_AS(code.backupTo(target.c_str()), cb4sqlite::Error);
    CHECK_FALSE(fs::exists(target));
    CHECK_FALSE(fs::exists(target + ".part"));

    CHECK_THROWS_WITH(code.backupTo(""), ContainsSubstring("no file name"));
    code.begin();
    CHECK_THROWS_WITH(code.backupTo((fs::temp_directory_path() / "x.db").string().c_str()),
                      ContainsSubstring("transaction is open"));
    code.rollback();
    Code4 closed;
    CHECK_THROWS_WITH(closed.backupTo("x.db"), ContainsSubstring("no database is open"));
}
