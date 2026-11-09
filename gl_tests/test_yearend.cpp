//
// gl_tests: Year End (Vouchers.cpp) and Restore (CompanyFiles.cpp), plan
// S2-M6, on the dev fixture. Its one type-R account is 340000 Retained
// Earnings (curr_bal -16,840); 410001 depts 10 and 20 are income accounts.
//

#include "GlTestDb.h"

#include "Accounts.h"
#include "CompanyFiles.h"
#include "Vouchers.h"

#include <cmath>
#include <fstream>
#include <string>
#include <vector>

using namespace gltest;
using Catch::Approx;
namespace fs = std::filesystem;

namespace {

double sumCurrBal(Code4& code)
{
    AccountsTable gl(code);
    double sum = 0;
    for (gl.top(); !gl.eof(); gl.skip()) sum += gl.curr_bal->num();
    return sum;
}

void setType(Code4& code, const char* account, const char* type)
{
    AccountsTable gl(code);
    REQUIRE(SeekAccount(gl, account, ""));
    gl.type->assign(type);
    gl.flush();
}

double field(Code4& code, const char* account, const char* dept, Field4* Tglaccts::*f)
{
    AccountsTable gl(code);
    REQUIRE(SeekAccount(gl, account, dept));
    return (gl.*f)->num();
}

} // namespace

TEST_CASE("YearEnd refuses unless exactly one account has type R", "[yearend]")
{
    FixtureDb db;
    const double before = sumCurrBal(db.code);

    setType(db.code, "101000", "R");   // the real data's second R account
    YearEndResult two = YearEnd(db.code);
    CHECK_FALSE(two.closed);
    REQUIRE(two.retained.size() == 2);
    CHECK(two.retained[0].account == "101000");
    CHECK(two.retained[1].account == "340000");
    CHECK(field(db.code, "101000", "", &Tglaccts::period7) == Approx(-250.0));   // nothing changed
    CHECK(sumCurrBal(db.code) == Approx(before));

    setType(db.code, "101000", "B");
    setType(db.code, "340000", "B");
    YearEndResult none = YearEnd(db.code);
    CHECK_FALSE(none.closed);
    CHECK(none.retained.empty());
}

TEST_CASE("YearEnd moves the income accounts into Retained Earnings", "[yearend]")
{
    FixtureDb db;
    REQUIRE(PostVoucher(db.code, 101).posted == std::vector<long>{101});   // 410001: -600 / -550; 111101: +1150
    const double before = sumCurrBal(db.code);
    const double retained = field(db.code, "340000", "", &Tglaccts::curr_bal);
    const double receivable = field(db.code, "111101", "", &Tglaccts::curr_bal);

    const YearEndResult r = YearEnd(db.code);
    REQUIRE(r.closed);
    REQUIRE(r.retained.size() == 1);
    CHECK(r.retained[0].account == "340000");
    CHECK(r.transferred == Approx(retained - 1150.0));   // 340000's own balance, plus the income

    CHECK(sumCurrBal(db.code) == Approx(before));   // the books still balance
    CHECK(field(db.code, "340000", "", &Tglaccts::curr_bal) == Approx(retained - 1150.0));
    CHECK(field(db.code, "410001", "10", &Tglaccts::curr_bal) == 0.0);
    CHECK(field(db.code, "410001", "20", &Tglaccts::curr_bal) == 0.0);
    CHECK(field(db.code, "111101", "", &Tglaccts::curr_bal) == Approx(receivable));   // balance sheet: kept

    // The periods roll into the previous-year columns.
    CHECK(field(db.code, "111101", "", &Tglaccts::previous7) == Approx(1150.0));
    CHECK(field(db.code, "111101", "", &Tglaccts::period7) == 0.0);
    CHECK(field(db.code, "101000", "", &Tglaccts::previous7) == Approx(-250.0));
    CHECK(field(db.code, "101000", "", &Tglaccts::previous12) == 0.0);   // period12 was blank
    CHECK(field(db.code, "102101", "", &Tglaccts::openbal) == Approx(12500.0));   // openbal untouched
}

TEST_CASE("BackupFileName: <stem>-<yyyymmdd>.db", "[backup]")
{
    CHECK(BackupFileName(fs::path("data") / "manila.db", 2026, 11, 9) == fs::path("manila-20261109.db"));
    CHECK(BackupOfCurrent(fs::path("manila.db")) == fs::path("manila.db.bak"));
}

TEST_CASE("Backup then Restore: the restored file replaces the current one", "[backup]")
{
    TempFile live(tempPath("gl_restore_live"));
    TempFile backup(tempPath("gl_restore_backup"));
    TempFile bak(BackupOfCurrent(live.path()));
    fs::copy_file(masterFixture(), live.path());

    Code4 code;
    REQUIRE(OpenCompany(code, live.path()) == r4success);
    code.backupTo(backup.path().string().c_str());
    // A change after the backup: a new account.
    {
        AccountsTable gl(code);
        AccountInput in;
        in.account = "999001";
        in.descript = "After the backup";
        in.type = "B";
        AppendAccount(gl, in);
        REQUIRE(SeekAccount(gl, "999001", ""));
    }

    const RestoreResult r = RestoreDatabase(code, live.path(), backup.path());
    INFO(r.error);
    REQUIRE(r.restored);
    CHECK(r.reopened);
    CHECK(code.isOpen());
    {
        AccountsTable gl(code);
        CHECK_FALSE(SeekAccount(gl, "999001", ""));   // gone
        CHECK(SeekAccount(gl, "340000", ""));
    }
    CHECK(fs::exists(bak.path()));   // the replaced file, with the new account
    {
        Code4 old;
        REQUIRE(OpenCompany(old, bak.path()) == r4success);
        AccountsTable gl(old);
        CHECK(SeekAccount(gl, "999001", ""));
    }
    code.close();
}

TEST_CASE("Restore refuses a foreign, damaged or same file and keeps the company open", "[backup]")
{
    TempFile live(tempPath("gl_restore_live"));
    TempFile bak(BackupOfCurrent(live.path()));
    fs::copy_file(masterFixture(), live.path());
    Code4 code;
    REQUIRE(OpenCompany(code, live.path()) == r4success);

    // Not a database.
    TempFile text(tempPath("gl_restore_text"));
    { std::ofstream(text.path()) << "this is not a database, just some text that is long enough to matter\n"; }
    RestoreResult r = RestoreDatabase(code, live.path(), text.path());
    CHECK_FALSE(r.restored);
    CHECK(r.reopened);
    CHECK_FALSE(r.error.empty());

    // An SQLite file without the company tables.
    TempFile other(tempPath("gl_restore_other"));
    {
        Code4 empty;
        REQUIRE(empty.open(other.path().string().c_str(), true) == r4success);   // only the meta tables
    }
    r = RestoreDatabase(code, live.path(), other.path());
    CHECK_FALSE(r.restored);
    CHECK(r.error.find("glaccts") != std::string::npos);
    const auto size = fs::file_size(other.path());

    // A damaged copy of the company: the header page is overwritten.
    TempFile damaged(tempPath("gl_restore_damaged"));
    fs::copy_file(masterFixture(), damaged.path());
    {
        std::fstream f(damaged.path(), std::ios::in | std::ios::out | std::ios::binary);
        f.seekp(0);
        const std::string junk(100, 'x');
        f.write(junk.data(), static_cast<std::streamsize>(junk.size()));
    }
    r = RestoreDatabase(code, live.path(), damaged.path());
    CHECK_FALSE(r.restored);

    // The company itself.
    r = RestoreDatabase(code, live.path(), live.path());
    CHECK_FALSE(r.restored);

    // None of this touched anything.
    CHECK(code.isOpen());
    CHECK_FALSE(fs::exists(bak.path()));
    CHECK(fs::file_size(other.path()) == size);
    AccountsTable gl(code);
    CHECK(SeekAccount(gl, "340000", ""));
    code.close();
}
