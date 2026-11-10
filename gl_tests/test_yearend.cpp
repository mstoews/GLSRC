//
// gl_tests: Year End (Vouchers.cpp) and Restore (CompanyFiles.cpp), plan
// S2-M6, on the dev fixture. Its one type-R account is 340000 Retained
// Earnings (curr_bal -16,840); 410001 depts 10 and 20 and the finance set's
// 411000-911000 (S3-M2) are income accounts.
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

// The income-statement (type I) accounts' total balance.
double incomeTotal(Code4& code)
{
    AccountsTable gl(code);
    double sum = 0;
    for (gl.top(); !gl.eof(); gl.skip())
        if (std::string(gl.type->str()) == "I") sum += gl.curr_bal->num();
    return sum;
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
    // 410001's -1,150 from voucher 101, and the finance set's -1,450 (S3-M2).
    const double income = incomeTotal(db.code);
    CHECK(income == Approx(-2600.0));

    const YearEndResult r = YearEnd(db.code);
    REQUIRE(r.closed);
    REQUIRE(r.retained.size() == 1);
    CHECK(r.retained[0].account == "340000");
    CHECK(r.transferred == Approx(retained + income));   // 340000's own balance, plus the income

    CHECK(sumCurrBal(db.code) == Approx(before));   // the books still balance
    CHECK(field(db.code, "340000", "", &Tglaccts::curr_bal) == Approx(retained + income));
    CHECK(field(db.code, "410001", "10", &Tglaccts::curr_bal) == 0.0);
    CHECK(field(db.code, "410001", "20", &Tglaccts::curr_bal) == 0.0);
    CHECK(field(db.code, "111101", "", &Tglaccts::curr_bal) == Approx(receivable));   // balance sheet: kept

    // The periods roll into the previous-year columns.
    CHECK(field(db.code, "111101", "", &Tglaccts::previous7) == Approx(1150.0));
    CHECK(field(db.code, "111101", "", &Tglaccts::period7) == 0.0);
    CHECK(field(db.code, "101000", "", &Tglaccts::previous7) == Approx(-250.0));
    CHECK(field(db.code, "101000", "", &Tglaccts::previous12) == 0.0);   // period12 was blank

    // Spec #3 D16: the new year opens at the closing balances (until S3-M2,
    // openbal was left alone: 102101 kept 12,500).
    CHECK(field(db.code, "102101", "", &Tglaccts::openbal) == Approx(12750.0));
    CHECK(field(db.code, "340000", "", &Tglaccts::openbal) == Approx(retained + income));
    CHECK(field(db.code, "411000", "", &Tglaccts::openbal) == 0.0);
    AccountsTable gl(db.code);
    for (gl.top(); !gl.eof(); gl.skip()) CHECK(gl.openbal->num() == Approx(gl.curr_bal->num()));
}

TEST_CASE("YearEnd D16: the year's gl_dist lines move to gl_distb", "[yearend]")
{
    FixtureDb db;
    auto rows = [&](const char* table, bool live) {
        Data4 d(&db.code, table);
        int n = 0;
        for (d.top(); !d.eof(); d.skip())
            if (!live || !d.deleted()) ++n;
        d.close();
        return n;
    };
    const int lines = rows("gl_dist", true);
    CHECK(lines == 27);   // voucher 100's 2, the finance journals' 25 (and one deleted row)
    REQUIRE(rows("gl_distb", false) == 0);

    const YearEndResult r = YearEnd(db.code);
    REQUIRE(r.closed);
    CHECK(r.archived == lines);
    CHECK(rows("gl_dist", false) == 0);   // the deleted row is packed away too
    CHECK(rows("gl_distb", false) == lines);

    // The archived lines keep their content (journal 9, October's cash sales).
    Data4 b(&db.code, "gl_distb");
    Field4 batchno(b, "batchno"), account(b, "account"), period(b, "period"), credit(b, "credit"), ref(b, "reference");
    bool found = false;
    for (b.top(); !b.eof(); b.skip()) {
        if (batchno.num() == 9 && TrimText(account.str()) == "411000") {
            found = true;
            CHECK(TrimText(period.str()) == "7");
            CHECK(credit.num() == Approx(6000.0));
            CHECK(TrimText(ref.str()) == "October cash sales");
        }
    }
    b.close();
    CHECK(found);
}

TEST_CASE("YearEnd closes the year's posted vouchers (mode 9)", "[yearend]")
{
    FixtureDb db;
    // Before: voucher 100 is posted, and 102101's inquiry shows its line.
    const AccountActivity before = AccountTransactions(db.code, "102101", "");
    REQUIRE(before.lines.size() == 1);
    CHECK(before.balance == Approx(12750.0));

    const YearEndResult r = YearEnd(db.code);
    REQUIRE(r.closed);
    CHECK(r.closedVouchers == 1);   // 100; 101-103 are open

    VoucherHeaders h(db.code);
    REQUIRE(SeekVoucher(h, 100));
    CHECK(ReadHeader(h).mode == voucher_mode::kClosedYear);
    CHECK(IsPostedMode(voucher_mode::kClosedYear));   // Voucher Entry won't load it, Cancel won't delete it
    CHECK(ListVouchers(h, {voucher_mode::kPosted}).empty());   // nothing for Recover
    CHECK(ListVouchers(h, {voucher_mode::kOpen, voucher_mode::kAdjustment}).size() == 3);

    // After: the new year's opening balance, and no lines counted twice.
    const AccountActivity after = AccountTransactions(db.code, "102101", "");
    CHECK(after.openBal == Approx(12750.0));
    CHECK(after.lines.empty());
    CHECK(after.balance == Approx(12750.0));
}

TEST_CASE("YearEnd: refused, it writes nothing (no archive, openbal kept)", "[yearend]")
{
    FixtureDb db;
    setType(db.code, "340000", "B");   // no R account
    REQUIRE_FALSE(YearEnd(db.code).closed);
    CHECK(field(db.code, "102101", "", &Tglaccts::openbal) == Approx(12500.0));
    Data4 d(&db.code, "gl_distb");
    d.top();
    CHECK(d.eof());
    d.close();
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
