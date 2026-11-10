//
// gl_tests: Transaction Inquiry and Budget Data (plan S2-M5) on the dev
// fixture. Voucher 100 is posted: 102101 Dr 250 / 101000 Cr 250 in period 7;
// 102101's opening balance is 12,500. Voucher 101 (open) has a 600 credit to
// 410001 dept 10 and 550 to dept 20.
//

#include "GlTestDb.h"

#include "Accounts.h"
#include "Vouchers.h"

#include <string>
#include <vector>

using namespace gltest;
using Catch::Approx;

TEST_CASE("AccountTransactions: the posted lines and the balance", "[inquiry]")
{
    FixtureDb db;
    const AccountActivity a = AccountTransactions(db.code, "102101", "");
    REQUIRE(a.found);
    CHECK(a.descript == "CHECKING ACCOUNT");
    CHECK(a.openBal == Approx(12500.0));
    REQUIRE(a.lines.size() == 1);
    CHECK(a.lines[0].number == 100);
    CHECK(a.lines[0].period == 7);
    CHECK(a.lines[0].descript == "Transfer from petty cash");
    CHECK(a.lines[0].debit == Approx(250.0));
    CHECK(a.balance == Approx(12750.0));

    const AccountActivity petty = AccountTransactions(db.code, " 101000 ", "");   // typed with spaces
    REQUIRE(petty.lines.size() == 1);
    CHECK(petty.credit == Approx(250.0));
    CHECK(petty.balance == Approx(250.0));   // 500 - 250
}

TEST_CASE("AccountTransactions: only that account and dept, and only posted lines", "[inquiry]")
{
    FixtureDb db;
    CHECK(AccountTransactions(db.code, "410001", "10").found);
    CHECK(AccountTransactions(db.code, "410001", "10").lines.empty());   // 101 is still open
    CHECK_FALSE(AccountTransactions(db.code, "410001", "").found);       // no blank-dept 410001
    CHECK_FALSE(AccountTransactions(db.code, "4100", "10").found);       // no prefix match
    CHECK_FALSE(AccountTransactions(db.code, "999999", "").found);

    REQUIRE(PostVoucher(db.code, 101).posted == std::vector<long>{101});
    const AccountActivity d10 = AccountTransactions(db.code, "410001", "10");
    REQUIRE(d10.lines.size() == 1);
    CHECK(d10.lines[0].credit == Approx(600.0));
    CHECK(d10.balance == Approx(-600.0));
    CHECK(AccountTransactions(db.code, "410001", "20").lines.size() == 1);

    // Recovered lines are no longer posted (D12).
    REQUIRE(RecoverVoucher(db.code, 100) == RecoverResult::Recovered);
    CHECK(AccountTransactions(db.code, "102101", "").lines.empty());
}

TEST_CASE("ParseAmount reads grid and tbFloat text", "[budget]")
{
    CHECK(ParseAmount("1,234.50") == Approx(1234.5));
    CHECK(ParseAmount("  -12.00 ") == Approx(-12.0));
    CHECK(ParseAmount("") == 0.0);
}

TEST_CASE("ReadBudget / SaveBudget: budget1-12 beside the period columns", "[budget]")
{
    FixtureDb db;
    {
        AccountsTable gl(db.code);
        REQUIRE(SeekAccount(gl, "102101", ""));
        const std::vector<BudgetRow> rows = ReadBudget(gl);
        REQUIRE(rows.size() == 12);
        CHECK(rows[6].current == Approx(250.0));   // period7
        CHECK(rows[11].previous == Approx(12500.0));   // previous12, the opening balance
        CHECK(rows[0].budget == 0.0);

        std::vector<double> budget;
        for (int i = 1; i <= 12; ++i) budget.push_back(i * 100.0 + 0.004);
        SaveBudget(gl, budget);
    }
    AccountsTable gl(db.code);
    REQUIRE(SeekAccount(gl, "102101", ""));
    const std::vector<BudgetRow> rows = ReadBudget(gl);
    for (size_t i = 0; i < 12; ++i) CHECK(rows[i].budget == Approx((static_cast<double>(i) + 1) * 100.0));
    CHECK(rows[6].current == Approx(250.0));   // untouched
    CHECK(gl.budget12->num() == Approx(1200.0));
    REQUIRE(SeekAccount(gl, "101000", ""));
    CHECK(ReadBudget(gl)[0].budget == 0.0);   // another account's budget is untouched
}
