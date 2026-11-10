//
// gl_tests: gl/port/Accounts.cpp, the Chart of Accounts record operations, on
// the migrated dev fixture. These are the 48 checks of the M6 Linux harness
// (never committed), regrouped into test cases.
//

#include "GlTestDb.h"

#include "Accounts.h"

#include <string>

using namespace gltest;

namespace {

// The fixture's accounts: 11 from Specs #1/#2, 14 more from Spec #3 S3-M2.
constexpr int kFixtureAccounts = 25;

int countLive(Code4& code)
{
    Data4 d(code, "glaccts");
    d.SetIndex("accounts");
    Relate4set r(&d);
    r.querySet(".NOT. DELETED()");
    int n = 0;
    for (int rc = r.top(); rc == r4success; rc = r.skip(1)) ++n;
    r.free();
    return n;
}

} // namespace

TEST_CASE("AccountsTable opens on the first account; SeekAccount is exact", "[accounts]")
{
    FixtureDb db;
    AccountsTable gl(db.code);
    REQUIRE(gl.isValid());
    CHECK(TrimText(gl.accounts->str()) == "101000");   // ctor: SetIndex("accounts"); top()
    CHECK(gl.accounts->len() == 7);

    CHECK(SeekAccount(gl, "410001", "20"));
    CHECK(TrimText(gl.descript->str()) == "SALES REVENUE - GENERAL");
    CHECK(SeekAccount(gl, "410001 ", "10  "));
    CHECK(std::string(gl.type->str()) == "I");
    CHECK_FALSE(SeekAccount(gl, "410001", ""));        // no 410001 without a dept
    CHECK_FALSE(SeekAccount(gl, "1", ""));             // no prefix match
    CHECK_FALSE(SeekAccount(gl, "", ""));
}

TEST_CASE("HasActivity, the Retained Earnings account and departments", "[accounts]")
{
    FixtureDb db;
    AccountsTable gl(db.code);
    CHECK(SeekAccount(gl, "102101", ""));
    CHECK(HasActivity(gl));
    CHECK(SeekAccount(gl, "144910", ""));
    CHECK_FALSE(HasActivity(gl));
    CHECK(SeekAccount(gl, "340000", ""));
    CHECK(std::string(gl.type->str()) == "R");
    CHECK(gl.previous12->num() == -16840.0);

    DeptTable dept(db.code);
    REQUIRE(dept.isValid());
    int n = 0;
    for (dept.top(); !dept.eof(); dept.skip()) ++n;
    CHECK(n == 2);
    CHECK(countLive(db.code) == kFixtureAccounts);
}

TEST_CASE("Add, modify and delete an account", "[accounts]")
{
    FixtureDb db;
    {
        AccountsTable gl(db.code);
        AccountInput in{"500100", "", "TEST EXPENSE", "FOREIGN TEXT", "I", "1,234.50", ""};
        CHECK_FALSE(SeekAccount(gl, in.account, in.dept));
        AppendAccount(gl, in);
    }
    CHECK(countLive(db.code) == kFixtureAccounts + 1);
    {
        AccountsTable gl(db.code);
        REQUIRE(SeekAccount(gl, "500100", ""));
        CHECK(TrimText(gl.descript->str()) == "TEST EXPENSE");
        CHECK(TrimText(gl.fdescript->str()) == "FOREIGN TEXT");
        CHECK(std::string(gl.type->str()) == "I");
        CHECK(gl.curr_bal->num() == 1234.5);
        CHECK(gl.openbal->num() == 0);
        CHECK(gl.previous12->num() == 0);
        CHECK(gl.budget1->num() == 0);
        CHECK(std::string(gl.budget1->str()).find_first_not_of(' ') != std::string::npos);   // zeroed, not null
        CHECK_FALSE(HasActivity(gl));
        ModifyAccount(gl, {"500100", "", "TEST EXPENSE CHANGED", "", "N", "10", "2,000.00"});   // "N" keeps the type
    }
    {
        AccountsTable gl(db.code);
        REQUIRE(SeekAccount(gl, "500100", ""));
        CHECK(TrimText(gl.descript->str()) == "TEST EXPENSE CHANGED");
        CHECK(std::string(gl.type->str()) == "I");
        CHECK(gl.curr_bal->num() == 10);
        CHECK(gl.previous12->num() == 2000);
        ModifyAccount(gl, {"500100", "", "TEST EXPENSE CHANGED", "", "B", "10", "2000"});
    }
    {
        AccountsTable gl(db.code);
        REQUIRE(SeekAccount(gl, "500100", ""));
        CHECK(std::string(gl.type->str()) == "B");
        DeleteAccount(gl);
        CHECK_FALSE(SeekAccount(gl, "500100", ""));
    }
    CHECK(countLive(db.code) == kFixtureAccounts);
    {
        Data4 d(db.code, "glaccts");
        CHECK(d.recCount() == kFixtureAccounts);   // pack removed the row
    }
    {
        AccountsTable gl(db.code);
        CHECK_FALSE(SeekAccount(gl, "500100", ""));
        CHECK(SeekAccount(gl, "101000", ""));
    }
}

TEST_CASE("closeAll while the tables are open, then their destructors", "[accounts]")
{
    FixtureDb db;
    {
        // TMainDialog::Cancel: cb.closeAll(); cb.init(); before the members go.
        AccountsTable gl(db.code);
        DeptTable dept(db.code);
        Data4 rows(&db.code, "glaccts");
        db.code.closeAll();
        db.code.init();
        CHECK_FALSE(gl.isValid());
    }
    SUCCEED("no crash, no leak (ASan / the Debug CRT check this)");
}

TEST_CASE("A database without glaccts", "[accounts]")
{
    EmptyDb db;
    AccountsTable gl(db.code);   // must not free garbage pointers
    CHECK_FALSE(gl.isValid());
    CHECK_FALSE(SeekAccount(gl, "1", ""));
}
