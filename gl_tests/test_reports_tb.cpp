//
// gl_tests: Balances, F1 Trial Balance and F2 Distribution Trial Balance
// (spec #3 5.1-5.3, plan S3-M2) on the dev fixture.
//
// Expected figures, from make_dev_fixture.py (FINANCE_* and the Spec #1/#2
// accounts). Current period 7 (Oct 26, ends 2026-10-31).
//
//   102201 bank: opening 20,000; periods +6,000 (1) -3,500 (2) -5,100 (3)
//          +8,000 (4) -1,150 (5) +700 (7): 24,950 at the end of period 7.
//   161000 accumulated depreciation: opening -6,000; -600 in period 6 with
//          no gl_dist lines, so F2 marks it (and 761000, +600) "*".
//   101000 / 102101: voucher 100 in period 7, -250 / +250.
//   751000: a deleted gl_dist row of 1,200 that nothing may count.
//
//   F1 at period 7: 22 accounts with a balance; debit = credit = 96,160.00.
//   F1 at period 6: 21 (911000's tax is in period 7); 90,160.00.
//   F1 at period 3: 17; 81,510.00.
//   F2 for period 7: debits = credits = 11,550 (voucher 100's 250 and
//          journals 9-12: 6,000 + 2,400 + 900 + 2,000); nothing marked.
//   F2 for periods 1-7: debits = credits = 43,400; 161000 and 761000 marked;
//          102201: opening 20,000, debits 24,050, credits 19,100, closing 24,950.
//

#include "GlTestDb.h"

#include "Accounts.h"   // TrimText
#include "reports/Balances.h"
#include "reports/ReportContext.h"
#include "reports/TrialBalances.h"
#include "Vouchers.h"   // YearEnd

#include <algorithm>
#include <string>

using namespace gltest;

namespace {

ReportContext context(Code4& code) { return LoadReportContext(code, "20261110"); }

const rpt::Row* findRow(const rpt::Report& r, const std::string& account, const std::string& dept = "")
{
    for (const rpt::Row& row : r.rows)
        if (row.kind == rpt::RowKind::Detail && row.indent == 0 && row.cells.size() > 1 &&
            row.cells[0].text == account && row.cells[1].text == dept)
            return &row;
    return nullptr;
}

const rpt::Row& totalRow(const rpt::Report& r)
{
    const auto it = std::find_if(r.rows.begin(), r.rows.end(), [](const rpt::Row& row) { return row.kind == rpt::RowKind::Total; });
    REQUIRE(it != r.rows.end());
    return *it;
}

int accountRows(const rpt::Report& r)
{
    return static_cast<int>(std::count_if(r.rows.begin(), r.rows.end(), [](const rpt::Row& row) {
        return row.kind == rpt::RowKind::Detail && row.indent == 0;
    }));
}

int voucherLineRows(const rpt::Report& r)
{
    return static_cast<int>(std::count_if(r.rows.begin(), r.rows.end(), [](const rpt::Row& row) {
        return row.kind == rpt::RowKind::Detail && row.indent == 1;
    }));
}

bool hasNoteContaining(const rpt::Report& r, const std::string& text)
{
    return std::any_of(r.rows.begin(), r.rows.end(), [&](const rpt::Row& row) {
        return row.kind == rpt::RowKind::Note && row.text.find(text) != std::string::npos;
    });
}

std::int64_t cents(const rpt::Cell& c) { return c.type == rpt::CellType::Amount ? c.value : 0; }

void setCurrBal(Code4& code, const char* account, double value)
{
    Data4 gl(&code, "glaccts");
    gl.SetIndex("accounts");
    Field4 acc(gl, "account"), bal(gl, "curr_bal");
    for (gl.top(); !gl.eof(); gl.skip()) {
        if (TrimText(acc.str()) == account) {
            bal.assignDouble(value);
            gl.flush();
            break;
        }
    }
    gl.close();
}

} // namespace

TEST_CASE("LoadBalances: every fixture account agrees with its periods", "[balances]")
{
    FixtureDb db;
    const auto accounts = LoadBalances(db.code);
    REQUIRE(accounts.size() == 25);
    std::int64_t sum = 0;
    for (const AccountBalances& a : accounts) {
        CHECK(a.consistent());
        sum += a.currBal;
    }
    CHECK(sum == 0);
    CHECK(ConsistencyNote(accounts).empty());

    const auto bank = std::find_if(accounts.begin(), accounts.end(), [](const AccountBalances& a) { return a.account == "102201"; });
    REQUIRE(bank != accounts.end());
    CHECK(bank->balanceAt(0) == 2000000);
    CHECK(bank->balanceAt(1) == 2600000);
    CHECK(bank->balanceAt(7) == 2495000);
    CHECK(bank->movement(2, 3) == -860000);
    CHECK(bank->movement(8, 12) == 0);

    const auto sales = std::find_if(accounts.begin(), accounts.end(), [](const AccountBalances& a) { return a.account == "411000"; });
    REQUIRE(sales != accounts.end());
    CHECK(sales->type == "I");
    CHECK(sales->previousMovement(1, 7) == -6300000);   // -9,000 a month
    CHECK(sales->budgetMovement(7, 7) == -1000000);

    const auto r10 = std::find_if(accounts.begin(), accounts.end(), [](const AccountBalances& a) {
        return a.account == "731000" && a.dept == "10";
    });
    REQUIRE(r10 != accounts.end());
    CHECK(r10->movement(1, 12) == 400000);
}

TEST_CASE("ConsistencyNote names an account whose CURR_BAL disagrees", "[balances]")
{
    FixtureDb db;
    setCurrBal(db.code, "999000", 123.45);
    const auto accounts = LoadBalances(db.code);
    const std::string note = ConsistencyNote(accounts);
    CHECK(note.find("999000") != std::string::npos);
    CHECK(note.find("102201") == std::string::npos);
    // F1 shows it too, and still balances from the period amounts.
    const rpt::Report r = TrialBalance(db.code, context(db.code), {});
    CHECK(hasNoteContaining(r, "999000"));
    CHECK(cents(totalRow(r).cells[3]) == cents(totalRow(r).cells[4]));
}

TEST_CASE("DistTotalsByAccount and DistLines skip deleted rows", "[balances]")
{
    FixtureDb db;
    const auto totals = DistTotalsByAccount(db.code, 1, 12);
    CHECK(totals.at({"751000", ""}).debit == 120000);   // not 2,400: the deleted duplicate
    CHECK(totals.at({"102201", ""}).debit == 2405000);
    CHECK(totals.at({"102201", ""}).credit == 1910000);
    CHECK(totals.count({"761000", ""}) == 0);

    const auto lines = DistLines(db.code, 7, 7);
    CHECK(lines.size() == 10);   // voucher 100's two, journals 9-12's eight
    for (const DistLine& l : lines) {
        CHECK(l.period == 7);
        CHECK(l.type == "GL");
    }
    CHECK(lines.front().account == "101000");   // account order
    CHECK(lines.front().batchno == 100);
    CHECK(lines.front().credit == 25000);
}

TEST_CASE("F1 Trial Balance at the current period", "[reports][f1]")
{
    FixtureDb db;
    const rpt::Report r = TrialBalance(db.code, context(db.code), {});
    CHECK(r.title == "Trial Balance");
    REQUIRE(r.subtitles.size() == 1);
    CHECK(r.subtitles[0] == "As at October 31, 2026, the end of period 7 (Oct 26)");
    CHECK(r.company == "Dev Fixture Company Ltd.");
    CHECK(accountRows(r) == 22);   // 25 less the three zero balances
    const rpt::Row& t = totalRow(r);
    CHECK(t.final);
    CHECK(cents(t.cells[3]) == 9616000);
    CHECK(cents(t.cells[4]) == 9616000);

    const rpt::Row* bank = findRow(r, "102201");
    REQUIRE(bank);
    CHECK(cents(bank->cells[3]) == 2495000);
    CHECK(bank->cells[4].type == rpt::CellType::Empty);
    const rpt::Row* loan = findRow(r, "201000");
    REQUIRE(loan);
    CHECK(loan->cells[3].type == rpt::CellType::Empty);
    CHECK(cents(loan->cells[4]) == 800000);
    const rpt::Row* salaries = findRow(r, "731000", "20");
    REQUIRE(salaries);
    CHECK(cents(salaries->cells[3]) == 150000);
    CHECK(findRow(r, "144910") == nullptr);   // zero
    CHECK_FALSE(hasNoteContaining(r, "differ"));
}

TEST_CASE("F1 Trial Balance at earlier periods, and with zero balances", "[reports][f1]")
{
    FixtureDb db;
    const ReportContext ctx = context(db.code);
    TrialBalanceOptions opt;
    opt.period = 6;
    rpt::Report r = TrialBalance(db.code, ctx, opt);
    CHECK(r.subtitles[0] == "As at September 30, 2026, the end of period 6 (Sep 26)");
    CHECK(accountRows(r) == 21);
    CHECK(cents(totalRow(r).cells[3]) == 9016000);
    CHECK(cents(totalRow(r).cells[4]) == 9016000);
    CHECK(cents(findRow(r, "102101")->cells[3]) == 1250000);   // before voucher 100 (period 7)

    opt.period = 3;
    r = TrialBalance(db.code, ctx, opt);
    CHECK(accountRows(r) == 17);
    CHECK(cents(totalRow(r).cells[3]) == 8151000);

    opt.period = 7;
    opt.includeZeros = true;
    r = TrialBalance(db.code, ctx, opt);
    CHECK(accountRows(r) == 25);
    const rpt::Row* transit = findRow(r, "144910");
    REQUIRE(transit);
    CHECK(transit->cells[3].type == rpt::CellType::Empty);
    CHECK(transit->cells[4].type == rpt::CellType::Empty);
}

TEST_CASE("F1: an unbalanced ledger gets a note", "[reports][f1]")
{
    FixtureDb db;
    {   // A one-sided change to a period column (and CURR_BAL, so it stays consistent).
        Data4 gl(&db.code, "glaccts");
        gl.SetIndex("accounts");
        Field4 acc(gl, "account"), p7(gl, "period7"), bal(gl, "curr_bal");
        for (gl.top(); !gl.eof(); gl.skip()) {
            if (TrimText(acc.str()) == "999000") {
                p7.assignDouble(10.0);
                bal.assignDouble(110.0);
                gl.flush();
                break;
            }
        }
        gl.close();
    }
    const rpt::Report r = TrialBalance(db.code, context(db.code), {});
    CHECK(cents(totalRow(r).cells[3]) - cents(totalRow(r).cells[4]) == 1000);
    CHECK(hasNoteContaining(r, "differ by 10.00"));
    CHECK_FALSE(hasNoteContaining(r, "don't agree"));
}

TEST_CASE("F2 Distribution Trial Balance for the current period", "[reports][f2]")
{
    FixtureDb db;
    const rpt::Report r = DistributionTrialBalance(db.code, context(db.code), {});
    CHECK(r.title == "Distribution Trial Balance");
    CHECK(r.subtitles[0] == "For period 7 (Oct 26), ending October 31, 2026");
    CHECK(r.orientation == rpt::Orientation::Portrait);
    CHECK(accountRows(r) == 22);
    const rpt::Row& t = totalRow(r);
    CHECK(cents(t.cells[3]) == 0);         // opening: the books balance
    CHECK(cents(t.cells[4]) == 1155000);
    CHECK(cents(t.cells[5]) == 1155000);
    CHECK(cents(t.cells[6]) == 0);
    CHECK_FALSE(hasNoteContaining(r, "* The debits"));

    const rpt::Row* checking = findRow(r, "102101");
    REQUIRE(checking);
    CHECK(cents(checking->cells[3]) == 1250000);
    CHECK(cents(checking->cells[4]) == 25000);
    CHECK(cents(checking->cells[5]) == 0);
    CHECK(cents(checking->cells[6]) == 1275000);
    CHECK(checking->cells[7].type == rpt::CellType::Empty);
    // opening + debits - credits = closing on every row.
    for (const rpt::Row& row : r.rows)
        if (row.kind == rpt::RowKind::Detail && row.indent == 0)
            CHECK(cents(row.cells[3]) + cents(row.cells[4]) - cents(row.cells[5]) == cents(row.cells[6]));
}

TEST_CASE("F2 for periods 1-7: the undistributed depreciation is marked", "[reports][f2]")
{
    FixtureDb db;
    DistTrialBalanceOptions opt;
    opt.from = 1;
    opt.to = 7;
    const rpt::Report r = DistributionTrialBalance(db.code, context(db.code), opt);
    CHECK(r.subtitles[0] == "For periods 1 (Apr 26) to 7 (Oct 26), ending October 31, 2026");
    CHECK(cents(totalRow(r).cells[4]) == 4340000);
    CHECK(cents(totalRow(r).cells[5]) == 4340000);

    const rpt::Row* bank = findRow(r, "102201");
    REQUIRE(bank);
    CHECK(cents(bank->cells[3]) == 2000000);
    CHECK(cents(bank->cells[4]) == 2405000);
    CHECK(cents(bank->cells[5]) == 1910000);
    CHECK(cents(bank->cells[6]) == 2495000);
    CHECK(bank->cells[7].type == rpt::CellType::Empty);

    const rpt::Row* accum = findRow(r, "161000");
    REQUIRE(accum);
    CHECK(accum->cells[7].text == "*");
    CHECK(cents(accum->cells[6]) == -660000);
    CHECK(findRow(r, "761000")->cells[7].text == "*");
    CHECK(cents(findRow(r, "751000")->cells[4]) == 120000);   // the deleted row isn't counted
    CHECK(hasNoteContaining(r, "2 accounts are marked"));
}

TEST_CASE("F2 with voucher lines: the original Distribution List beneath each account", "[reports][f2]")
{
    FixtureDb db;
    DistTrialBalanceOptions opt;
    opt.voucherLines = true;
    const rpt::Report r = DistributionTrialBalance(db.code, context(db.code), opt);
    CHECK(r.orientation == rpt::Orientation::Landscape);
    CHECK(voucherLineRows(r) == 10);
    // Voucher 100's lines follow 101000 and 102101.
    for (const char* account : {"101000", "102101"}) {
        const auto it = std::find_if(r.rows.begin(), r.rows.end(), [&](const rpt::Row& row) {
            return row.kind == rpt::RowKind::Detail && row.indent == 0 && row.cells[0].text == account;
        });
        REQUIRE(it != r.rows.end());
        const rpt::Row& line = *(it + 1);
        CHECK(line.indent == 1);
        CHECK(line.cells[0].type == rpt::CellType::Date);
        CHECK(line.cells[0].text == "20261015");
        CHECK(line.cells[1].text == "P7");
        CHECK(line.cells[2].text.find("No. 100 GL") == 0);
        CHECK(line.cells[2].text.find("chq 000123") != std::string::npos);
    }
}

TEST_CASE("F1 after year end (D16): the balance sheet, income folded into retained earnings", "[reports][f1][yearend]")
{
    FixtureDb db;
    TrialBalanceOptions at12;
    at12.period = 12;
    const rpt::Report before = TrialBalance(db.code, context(db.code), at12);
    REQUIRE(YearEnd(db.code).closed);
    const rpt::Report after = TrialBalance(db.code, context(db.code), at12);
    CHECK(ConsistencyNote(LoadBalances(db.code)).empty());   // the identity holds after D16

    auto signedBalance = [](const rpt::Row* row) { return row ? cents(row->cells[3]) - cents(row->cells[4]) : 0; };
    for (const char* account : {"101000", "102201", "151000", "161000", "201000", "301000", "999000"})
        CHECK(signedBalance(findRow(after, account)) == signedBalance(findRow(before, account)));
    // 340000: -16,840 and the year's income (-1,450: sales 16,000 and interest 50 less 14,600 of costs).
    CHECK(signedBalance(findRow(after, "340000")) == -1684000 - 145000);
    CHECK(findRow(after, "411000") == nullptr);   // income accounts are 0
    CHECK(cents(totalRow(after).cells[3]) == cents(totalRow(after).cells[4]));
}

TEST_CASE("F2 includes inactive accounts only when asked", "[reports][f2]")
{
    FixtureDb db;
    DistTrialBalanceOptions opt;
    opt.includeInactive = true;
    CHECK(accountRows(DistributionTrialBalance(db.code, context(db.code), opt)) == 25);
    opt.from = opt.to = 12;   // nothing happens in period 12, but balances carry
    opt.includeInactive = false;
    const rpt::Report r = DistributionTrialBalance(db.code, context(db.code), opt);
    CHECK(accountRows(r) == 22);
    CHECK(cents(totalRow(r).cells[4]) == 0);
}
