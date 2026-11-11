//
// gl_tests: the fixed-asset core (spec #4 4.1, 5, 9; plan S4-M0) on the dev
// fixture's register:
//
//   EQ  151000 / 761000 / 161000           XX  999999 (not in the chart)
//
//   E001  straight, 30,000.00, life 60, 13 months used, 6,500.00 accumulated  (500.00 a month)
//   E002  declining 24 %, 5,000.00, bought 2026-06-15, 1 month, 100.00        (100.00 a month)
//   E004  disposed 2026-08-15, 1,000.00, fully depreciated
//   E005  type XX, 2,000.00, nothing yet
//
//   gl_fadep: period 6 of the year ending 2027-03-31, E001 500.00 and E002
//   100.00, no voucher. Type EQ's register (E001 + E002) is cost 35,000.00
//   and accumulated 6,600.00: the ledger's 151000 and 161000 at period 7.
//
#include "GlTestDb.h"

#include "Accounts.h"   // TrimText
#include "Assets.h"
#include "Vouchers.h"

#include <algorithm>
#include <numeric>
#include <string>
#include <vector>

using namespace gltest;

namespace {

Asset straight(std::int64_t cost, std::int64_t salvage, int life, int used, std::int64_t accum)
{
    Asset a;
    a.code = "T1";
    a.type = "EQ";
    a.method = DepMethod::Straight;
    a.cost = cost;
    a.salvage = salvage;
    a.life = life;
    a.monthsUsed = used;
    a.accumulated = accum;
    return a;
}

// Runs `months` months of straight-line depreciation on `a`, as the run will:
// the due amount, then accumulated and months used move.
std::vector<std::int64_t> runMonths(Asset a, int months)
{
    std::vector<std::int64_t> out;
    for (int i = 0; i < months; ++i) {
        const std::int64_t due = DueAmount(a, a.cost);
        out.push_back(due);
        a.accumulated += due;
        ++a.monthsUsed;
    }
    return out;
}

double balance(Code4& code, const char* account)
{
    cb4sqlite::Query q(code, "SELECT openbal + coalesce(period1,0) + coalesce(period2,0) + coalesce(period3,0) + "
                             "coalesce(period4,0) + coalesce(period5,0) + coalesce(period6,0) + coalesce(period7,0) "
                             "FROM glaccts WHERE trim(account) = ?1");
    q.bindText(1, account);
    REQUIRE(q.next());
    return q.number(0);
}

} // namespace

TEST_CASE("The fixture's register and types", "[assets]")
{
    FixtureDb db;
    CHECK(EnsureAssetTables(db.code) == 0);   // every accumulated amount is set

    const auto types = LoadAssetTypes(db.code);
    REQUIRE(types.size() == 2);
    CHECK(types[0].code == "EQ");
    CHECK(types[0].asset.account == "151000");
    CHECK(types[0].expense.account == "761000");
    CHECK(types[0].accum.account == "161000");
    CHECK(types[0].accum.dept.empty());
    CHECK(types[1].code == "XX");

    const auto assets = LoadAssets(db.code);
    REQUIRE(assets.size() == 4);
    CHECK(assets[0].code == "E001");
    CHECK(assets[0].method == DepMethod::Straight);
    CHECK(assets[0].cost == 3000000);
    CHECK(assets[0].accumulated == 650000);
    CHECK(assets[0].monthsUsed == 13);
    CHECK(assets[0].life == 60);
    CHECK(assets[0].mode.empty());
    CHECK(assets[1].method == DepMethod::Declining);
    CHECK(assets[1].percent == 24);
    CHECK(assets[1].datePurchased == "20260615");
    CHECK(assets[2].code == "E004");
    CHECK(assets[2].disposed());
    CHECK(assets[2].dateSold == "20260815");
    CHECK(assets[3].type == "XX");

    // The register agrees with the ledger (D11): type EQ's assets in use.
    std::int64_t cost = 0, accum = 0;
    for (const Asset& a : assets)
        if (a.type == "EQ" && !a.disposed()) {
            cost += a.cost;
            accum += a.accumulated;
        }
    CHECK(cost == 3500000);
    CHECK(accum == 660000);
    CHECK(balance(db.code, "151000") == 35000.0);
    CHECK(balance(db.code, "161000") == -6600.0);

    CHECK(FindAsset(db.code, "e001") == std::nullopt);   // codes are exact, as the tag
    REQUIRE(FindAsset(db.code, "E002 "));
    CHECK(FindAsset(db.code, "E002 ")->description == "Laptops");
    CHECK(FindAssetType(db.code, "XX")->asset.account == "999999");
    CHECK_FALSE(FindAssetType(db.code, "ZZ"));
}

TEST_CASE("ValidateAccount: the account and dept are in the chart", "[assets]")
{
    FixtureDb db;
    CHECK(ValidateAccount(db.code, "161000", ""));
    CHECK(ValidateAccount(db.code, "161000 ", "  "));
    CHECK_FALSE(ValidateAccount(db.code, "999999", ""));
    CHECK_FALSE(ValidateAccount(db.code, "", ""));
    CHECK(ValidateAccount(db.code, "731000", "10"));        // salaries, dept 10
    CHECK_FALSE(ValidateAccount(db.code, "731000", "30"));  // no dept 30
    CHECK_FALSE(ValidateAccount(db.code, "731000", ""));    // it exists only by department
}

TEST_CASE("Straight line: the monthly amount, rounding and the last month", "[assets]")
{
    CHECK(MonthlyAmount(straight(500000, 20000, 48, 0, 0), 0) == 10000);   // 4,800 / 48 = 100.00
    CHECK(MonthlyAmount(straight(500000, 0, 0, 0, 0), 0) == 0);           // no life

    // 1,000.00 over 3 months: 333.33, 333.33, then the 333.34 that is left.
    const auto months = runMonths(straight(100000, 0, 3, 0, 0), 4);
    CHECK(months == std::vector<std::int64_t>{33333, 33333, 33334, 0});

    // E001-like: 500.00 a month, the last of 60 months exactly what is left.
    const auto all = runMonths(straight(3000000, 0, 60, 0, 0), 61);
    CHECK(all.front() == 50000);
    CHECK(std::accumulate(all.begin(), all.end(), std::int64_t{0}) == 3000000);
    CHECK(all.back() == 0);
}

TEST_CASE("The salvage floor (D4)", "[assets]")
{
    // 2,000.00 cost, 200.00 salvage, 1,750.00 accumulated: only 50.00 is left.
    Asset a = straight(200000, 20000, 48, 20, 175000);
    CHECK(Remaining(a) == 5000);
    CHECK(MonthlyAmount(a, a.cost) == 3750);
    CHECK(DueAmount(a, a.cost) == 3750);
    a.accumulated = 178000;   // 20.00 left, less than a month
    CHECK(DueAmount(a, a.cost) == 2000);
    a.accumulated = 180000;   // at salvage
    CHECK(DueAmount(a, a.cost) == 0);
    CHECK(BookValue(a) == 20000);

    Asset d = a;
    d.accumulated = 0;
    d.mode = "D";             // disposed
    CHECK(DueAmount(d, d.cost) == 0);
}

TEST_CASE("Declining balance: the start-of-year book value (D3)", "[assets]")
{
    FixtureDb db;
    const std::string yearEnd = FiscalYearEnd(db.code);
    CHECK(yearEnd == "20270331");

    // E002: bought this year, so its start-of-year value is its cost.
    const Asset e002 = *FindAsset(db.code, "E002");
    CHECK(StartOfYearBookValue(db.code, e002, yearEnd) == 500000);
    CHECK(MonthlyAmount(e002, 500000) == 10000);      // 5,000.00 x 24 % / 12
    CHECK(DueAmount(e002, 500000) == 10000);

    // E001: 6,000.00 accumulated before this year.
    const Asset e001 = *FindAsset(db.code, "E001");
    CHECK(StartOfYearBookValue(db.code, e001, yearEnd) == 3000000 - 600000);

    // The next year: this year's runs are history, so they count.
    CHECK(StartOfYearBookValue(db.code, e002, "20280331") == 500000 - 10000);
    CHECK(MonthlyAmount(e002, 490000) == 9800);       // 4,900.00 x 24 % / 12

    const auto rows = LoadDepreciation(db.code, "E002");
    REQUIRE(rows.size() == 1);
    CHECK(rows[0].period == 6);
    CHECK(rows[0].batchno == 0);
    CHECK(rows[0].amount == 10000);
}

TEST_CASE("Saving types, assets and history", "[assets]")
{
    FixtureDb db;
    AssetType fu{"FU", "Furniture", {"151000", "", "OFFICE EQUIPMENT"}, {"761000", "", "DEPRECIATION"},
                 {"161000", "", "ACCUM. DEPRECIATION"}};
    CHECK(SaveAssetType(db.code, fu));
    fu.description = "Furniture and fittings";
    CHECK_FALSE(SaveAssetType(db.code, fu));   // replaced
    CHECK(LoadAssetTypes(db.code).size() == 3);
    CHECK(FindAssetType(db.code, "FU")->description == "Furniture and fittings");

    Asset a = straight(120000, 0, 12, 0, 0);
    a.code = "E006";
    a.type = "FU";
    a.description = "Desk";
    a.datePurchased = "20261001";
    a.monthly = MonthlyAmount(a, a.cost);
    CHECK(SaveAsset(db.code, a));
    a.accumulated = 10000;
    a.monthsUsed = 1;
    CHECK_FALSE(SaveAsset(db.code, a));
    const Asset back = *FindAsset(db.code, "E006");
    CHECK(back.monthly == 10000);
    CHECK(back.accumulated == 10000);
    CHECK(back.monthsUsed == 1);
    CHECK(back.datePurchased == "20261001");
    CHECK(back.dateSold.empty());
    CHECK(back.method == DepMethod::Straight);
    CHECK(LoadAssets(db.code).size() == 5);
    // Adding left the others alone (appendStart before blank: the other order
    // flushes blanks over the record the failed lookup left current).
    for (const Asset& x : LoadAssets(db.code)) CHECK_FALSE(TrimText(x.code).empty());
    CHECK(FindAsset(db.code, "E001")->cost == 3000000);
    CHECK(FindAssetType(db.code, "EQ")->accum.account == "161000");
    CHECK(FindAssetType(db.code, "XX")->asset.account == "999999");

    AppendDepreciation(db.code, {"E006", "20270331", 7, 105, 10000});
    const auto rows = LoadDepreciation(db.code, "E006");
    REQUIRE(rows.size() == 1);
    CHECK(rows[0].batchno == 105);
    CHECK(rows[0].yearEnd == "20270331");
}

TEST_CASE("CheckAsset: what Add/Mod refuses", "[assets]")
{
    FixtureDb db;
    Asset a = straight(120000, 0, 12, 0, 0);
    a.code = "E006";
    a.description = "Desk";
    a.datePurchased = "20261001";
    CHECK_FALSE(CheckAsset(db.code, a));

    using K = AssetProblem::Kind;
    auto failsOn = [&](Asset x, AssetField f, K k) {
        const auto p = CheckAsset(db.code, x);
        REQUIRE(p);
        CHECK(p->field == f);
        CHECK(p->kind == k);
    };
    Asset x = a; x.code = " ";          failsOn(x, AssetField::Code, K::Required);
    x = a; x.type = "";                 failsOn(x, AssetField::Type, K::Required);
    x = a; x.type = "ZZ";               failsOn(x, AssetField::Type, K::NoSuchType);
    x = a; x.description = "";          failsOn(x, AssetField::Description, K::Required);
    x = a; x.datePurchased = "";        failsOn(x, AssetField::Date, K::Required);
    x = a; x.cost = 0;                  failsOn(x, AssetField::Cost, K::Required);
    x = a; x.life = 0;                  failsOn(x, AssetField::Life, K::Required);
    x = a; x.method = DepMethod::Declining; failsOn(x, AssetField::Rate, K::Required);
    x = a; x.salvage = 130000;          failsOn(x, AssetField::Salvage, K::SalvageOverCost);
    x = a; x.monthsUsed = 13;           failsOn(x, AssetField::MonthsUsed, K::UsedOverLife);
    x = a; x.salvage = 20000; x.accumulated = 100001; failsOn(x, AssetField::Accumulated, K::AccumOverDepreciable);

    CHECK(HasDepreciation(db.code, "E001"));
    CHECK_FALSE(HasDepreciation(db.code, "E005"));
}

TEST_CASE("Deleting a type: refused while an asset uses it (D9)", "[assets]")
{
    FixtureDb db;
    std::string usedBy;
    CHECK(DeleteAssetType(db.code, "EQ", &usedBy) == DeleteTypeResult::InUse);
    CHECK(usedBy == "E001");
    CHECK(DeleteAssetType(db.code, "XX ", &usedBy) == DeleteTypeResult::InUse);
    CHECK(usedBy == "E005");
    CHECK(DeleteAssetType(db.code, "ZZ") == DeleteTypeResult::NotFound);

    AssetType fu{"FU", "Furniture", {"151000", "", ""}, {"761000", "", ""}, {"161000", "", ""}};
    SaveAssetType(db.code, fu);
    CHECK(DeleteAssetType(db.code, "FU") == DeleteTypeResult::Deleted);
    CHECK_FALSE(FindAssetType(db.code, "FU"));
    CHECK(LoadAssetTypes(db.code).size() == 2);
    CHECK(FindAssetType(db.code, "EQ")->accum.account == "161000");   // the others are untouched
}

TEST_CASE("A company without the tables gets them; a blank ACCUM_DEP is filled", "[assets]")
{
    EmptyDb db;
    CHECK(EnsureAssetTables(db.code) == 0);
    CHECK(LoadAssets(db.code).empty());
    CHECK(LoadAssetTypes(db.code).empty());
    CHECK(EnsureAssetTables(db.code) == 0);   // a second time: nothing to do

    // A migrated asset as the original left it: months used, no ACCUM_DEP.
    {
        Data4 fa(&db.code, "gl_fa");
        Field4 facode(fa, "facode"), cost(fa, "cost"), salvage(fa, "salvage"), currDep(fa, "curr_dep"),
            lifetime(fa, "lifetime"), lifeleft(fa, "lifeleft");
        for (const auto& [c, used] : {std::pair{"F001", 8}, std::pair{"F002", 60}}) {
            fa.appendStart();
            fa.blank();
            facode.assign(c);
            cost.assign("100000.00");
            salvage.assign("100.00");
            currDep.assign("2081.25");
            lifetime.assignLong(48);
            lifeleft.assignLong(used);
            fa.append();
        }
        fa.close();
    }
    REQUIRE(LoadAssets(db.code).size() == 2);
    CHECK(EnsureAssetTables(db.code) == 2);
    REQUIRE(FindAsset(db.code, "F001"));
    REQUIRE(FindAsset(db.code, "F002"));
    CHECK(FindAsset(db.code, "F001")->accumulated == 208125 * 8);   // 16,650.00
    CHECK(FindAsset(db.code, "F002")->accumulated == 9990000);             // capped at cost - salvage
    CHECK(EnsureAssetTables(db.code) == 0);
}

// ---- the run (spec #4 4.2, 4.3; plan S4-M3) -----------------------------------

namespace {

struct LineRow {
    int journal;
    std::string account, dept, reference, period, date;
    double debit, credit;
};

std::vector<LineRow> voucherLines(Code4& code, long batchno)
{
    std::vector<LineRow> out;
    cb4sqlite::Query q(code, "SELECT journal, trim(account), trim(coalesce(dept,'')), trim(reference), trim(period), "
                             "date, debit, credit FROM gl_btchd WHERE _deleted = 0 AND batchno = ?1 ORDER BY journal");
    q.bindInt(1, batchno);
    while (q.next())
        out.push_back({static_cast<int>(q.integer(0)), q.text(1), q.text(2), q.text(3), q.text(4), q.text(5),
                       q.number(6), q.number(7)});
    return out;
}

// Sets gl_pds's end dates a year on, as Company Profile > Periods does for a new year.
void nextYearPeriods(Code4& code)
{
    Data4 pds(&code, "gl_pds");
    Field4 periode(pds, "periode");
    for (pds.top(); !pds.eof(); pds.skip()) {
        std::string d = TrimText(periode.str());
        d = std::to_string(std::stoi(d.substr(0, 4)) + 1) + d.substr(4);
        periode.assign(d.c_str());
        pds.flush();
    }
    pds.close();
}

} // namespace

TEST_CASE("PlanRun: the assets due in period 7, and those skipped", "[assets][run]")
{
    FixtureDb db;
    const RunPlan plan = PlanRun(db.code, 7);
    CHECK(plan.periodEnd == "20261031");
    CHECK(plan.yearEnd == "20270331");
    CHECK_FALSE(plan.alreadyRun);
    REQUIRE(plan.due.size() == 2);
    CHECK(plan.due[0].asset.code == "E001");
    CHECK(plan.due[0].amount == 50000);
    CHECK(plan.due[1].asset.code == "E002");
    CHECK(plan.due[1].amount == 10000);
    CHECK(plan.total == 60000);
    REQUIRE(plan.skipped.size() == 2);
    CHECK(plan.skipped[0].asset.code == "E004");
    CHECK(plan.skipped[0].reason == SkipReason::Disposed);
    CHECK(plan.skipped[1].asset.code == "E005");
    CHECK(plan.skipped[1].reason == SkipReason::BadAccount);   // XX: 999999 isn't in the chart

    // A type that doesn't exist (string 8120).
    {
        Asset orphan = *FindAsset(db.code, "E005");
        orphan.code = "E008";
        orphan.type = "ZZ";
        SaveAsset(db.code, orphan);
        const RunPlan p = PlanRun(db.code, 7);
        CHECK(p.skipped.back().asset.code == "E008");
        CHECK(p.skipped.back().reason == SkipReason::NoType);
    }

    // Period 6 has the fixture's run, booked without a voucher (D5).
    const RunPlan six = PlanRun(db.code, 6);
    CHECK(six.alreadyRun);
    CHECK(six.existingVoucher == 0);

    // An asset bought after the period's end isn't due yet (D8).
    Asset later = *FindAsset(db.code, "E001");
    later.code = "E009";
    later.datePurchased = "20261115";
    later.accumulated = 0;
    later.monthsUsed = 0;
    SaveAsset(db.code, later);
    const RunPlan withLater = PlanRun(db.code, 7);
    CHECK(withLater.due.size() == 2);
    CHECK(withLater.skipped.back().asset.code == "E009");
    CHECK(withLater.skipped.back().reason == SkipReason::NotYetBought);
}

TEST_CASE("RunDepreciation: one voucher, two lines per asset, the assets moved", "[assets][run]")
{
    FixtureDb db;
    const RunResult r = RunDepreciation(db.code, 7, 0);
    REQUIRE(r.status == RunStatus::Created);
    CHECK(r.batchno == 104);   // NextVoucherNumber: after 100-103
    CHECK(r.assets == 2);
    CHECK(r.total == 60000);

    VoucherHeaders h(db.code);
    REQUIRE(SeekVoucher(h, r.batchno));
    const VoucherHeader header = ReadHeader(h);
    h.close();
    CHECK(header.mode == "1");
    CHECK(header.description == "Monthly Depreciation Voucher");
    CHECK(header.date == "20261031");   // D10
    CHECK(header.period == 7);
    CHECK(header.debit == 600.0);
    CHECK(header.credit == 600.0);
    CHECK(header.lines == 4);

    const auto lines = voucherLines(db.code, r.batchno);
    REQUIRE(lines.size() == 4);
    CHECK(lines[0].journal == 1);
    CHECK(lines[0].account == "761000");
    CHECK(lines[0].debit == 500.0);
    CHECK(lines[0].reference == "E001");
    CHECK(lines[0].date == "20261031");
    CHECK(lines[0].period == "7");
    CHECK(lines[1].account == "161000");
    CHECK(lines[1].credit == 500.0);
    CHECK(lines[2].reference == "E002");
    CHECK(lines[2].debit == 100.0);
    CHECK(lines[3].journal == 4);
    CHECK(lines[3].credit == 100.0);

    const Asset e001 = *FindAsset(db.code, "E001"), e002 = *FindAsset(db.code, "E002");
    CHECK(e001.accumulated == 700000);
    CHECK(e001.monthsUsed == 14);
    CHECK(e002.accumulated == 20000);
    CHECK(e002.monthsUsed == 2);
    CHECK(FindAsset(db.code, "E005")->accumulated == 0);
    const auto history = LoadDepreciation(db.code, "E001");
    REQUIRE(history.size() == 2);
    CHECK(history[1].period == 7);
    CHECK(history[1].batchno == 104);
    CHECK(history[1].amount == 50000);

    // D5: the period can't be run twice.
    const RunResult again = RunDepreciation(db.code, 7, 0);
    CHECK(again.status == RunStatus::AlreadyRun);
    CHECK(again.batchno == 104);
    CHECK(FindAsset(db.code, "E001")->accumulated == 700000);
}

TEST_CASE("Cancelling the run's voucher undoes it; an ordinary voucher's doesn't touch the assets", "[assets][run]")
{
    FixtureDb db;
    REQUIRE(CancelVoucher(db.code, 101) == CancelResult::Cancelled);   // an ordinary voucher
    CHECK(FindAsset(db.code, "E001")->accumulated == 650000);
    CHECK(LoadDepreciation(db.code, "E001").size() == 1);

    const RunResult r = RunDepreciation(db.code, 7, 0);
    REQUIRE(r.status == RunStatus::Created);
    REQUIRE(CancelVoucher(db.code, r.batchno) == CancelResult::Cancelled);
    const Asset e001 = *FindAsset(db.code, "E001"), e002 = *FindAsset(db.code, "E002");
    CHECK(e001.accumulated == 650000);
    CHECK(e001.monthsUsed == 13);
    CHECK(e002.accumulated == 10000);
    CHECK(e002.monthsUsed == 1);
    CHECK(LoadDepreciation(db.code, "E001").size() == 1);   // period 6 only
    CHECK(voucherLines(db.code, r.batchno).empty());

    // The period can be run again.
    const RunResult again = RunDepreciation(db.code, 7, 0);
    CHECK(again.status == RunStatus::Created);
    CHECK(FindAsset(db.code, "E001")->accumulated == 700000);
}

TEST_CASE("Posting the run's voucher: the ledger matches the register", "[assets][run]")
{
    FixtureDb db;
    const RunResult r = RunDepreciation(db.code, 7, 0);
    REQUIRE(r.status == RunStatus::Created);
    REQUIRE(PostVoucher(db.code, r.batchno).posted == std::vector<long>{r.batchno});
    CHECK(balance(db.code, "161000") == -7200.0);
    CHECK(balance(db.code, "761000") == 1200.0);
    std::int64_t accum = 0;
    for (const Asset& a : LoadAssets(db.code))
        if (a.type == "EQ" && !a.disposed()) accum += a.accumulated;
    CHECK(accum == 720000);
    // A posted voucher can't be cancelled, so the run stands.
    CHECK(CancelVoucher(db.code, r.batchno) == CancelResult::Posted);
    CHECK(FindAsset(db.code, "E001")->accumulated == 700000);
}

TEST_CASE("No period end, nothing due, and a new fiscal year's declining amount", "[assets][run]")
{
    FixtureDb db;
    CHECK(RunDepreciation(db.code, 13, 0).status == RunStatus::NoPeriodEnd);

    const RunResult r = RunDepreciation(db.code, 7, 0);
    REQUIRE(r.status == RunStatus::Created);
    REQUIRE(PostVoucher(db.code, r.batchno).posted == std::vector<long>{r.batchno});
    REQUIRE(YearEnd(db.code).closed);
    nextYearPeriods(db.code);   // the new year's dates, as Company Profile > Periods sets them
    CHECK(FiscalYearEnd(db.code) == "20280331");

    // E002: (5,000.00 - 200.00 depreciated last year) x 24 % / 12 = 96.00.
    const RunPlan plan = PlanRun(db.code, 1);
    CHECK_FALSE(plan.alreadyRun);
    const auto e002 =
        std::find_if(plan.due.begin(), plan.due.end(), [](const RunItem& i) { return i.asset.code == "E002"; });
    REQUIRE(e002 != plan.due.end());
    CHECK(e002->amount == 9600);
    CHECK(e002->monthly == 9600);

    // With every asset disposed, nothing is due and no voucher is written.
    for (Asset a : LoadAssets(db.code)) {
        a.mode = "D";
        SaveAsset(db.code, a);
    }
    const RunResult none = RunDepreciation(db.code, 1, 0);
    CHECK(none.status == RunStatus::NothingDue);
    CHECK(none.batchno == 0);
}

// ---- Disposal (spec #4 6.3, D7; plan S4-M4) ------------------------------------

TEST_CASE("Disposal: E002 after the period-7 run, sold at a loss", "[assets][disposal]")
{
    FixtureDb db;
    REQUIRE(RunDepreciation(db.code, 7, 0).status == RunStatus::Created);
    const Asset e002 = *FindAsset(db.code, "E002");
    CHECK(BookValue(e002) == 480000);
    CHECK(GainOnSale(e002, 400000) == -80000);   // a loss of 800.00
    CHECK(GainOnSale(e002, 500000) == 20000);    // or a gain of 200.00

    CHECK(DisposeAsset(db.code, "E002", "20261031") == DisposeResult::Disposed);
    const Asset after = *FindAsset(db.code, "E002");
    CHECK(after.disposed());
    CHECK(after.dateSold == "20261031");
    CHECK(after.accumulated == 20000);   // the rest of the record is kept
    CHECK(after.cost == 500000);
    CHECK(DisposeAsset(db.code, "E002", "20261031") == DisposeResult::AlreadyDisposed);
    CHECK(DisposeAsset(db.code, "E999", "20261031") == DisposeResult::NotFound);

    // The next period's run skips it.
    const RunPlan eight = PlanRun(db.code, 8);
    REQUIRE(eight.due.size() == 1);
    CHECK(eight.due[0].asset.code == "E001");
    CHECK(std::any_of(eight.skipped.begin(), eight.skipped.end(), [](const RunSkip& s) {
        return s.asset.code == "E002" && s.reason == SkipReason::Disposed;
    }));
}

TEST_CASE("Disposal's Delete: only an asset the run hasn't depreciated", "[assets][disposal]")
{
    FixtureDb db;
    CHECK(DeleteAsset(db.code, "E001") == DeleteAssetResult::HasHistory);
    CHECK(DeleteAsset(db.code, "E999") == DeleteAssetResult::NotFound);
    CHECK(DeleteAsset(db.code, "E005") == DeleteAssetResult::Deleted);   // never depreciated
    CHECK_FALSE(FindAsset(db.code, "E005"));
    CHECK(LoadAssets(db.code).size() == 3);
    CHECK(FindAsset(db.code, "E001")->accumulated == 650000);   // the others are untouched
}
