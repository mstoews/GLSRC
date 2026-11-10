//
// gl_tests: the voucher record operations (gl/port/Vouchers.cpp, plan S2-M3)
// on the dev fixture's vouchers:
//   100  posted ("0"), 250.00 / 250.00
//   101  open ("1"), 1150.00 / 1150.00: a debit to 111101, credits to
//        410001 depts 10 (600.00) and 20 (550.00)
//   102  open, out of balance: 100.00 / 90.00
//   103  an open adjustment ("3"), 75.00 / 75.00
// and periods 1-6 closed, 7-12 open (current: "Oct 26").
//

#include "GlTestDb.h"

#include "Vouchers.h"

#include <string>
#include <vector>

using namespace gltest;

namespace {

std::vector<std::string> accounts(const std::vector<VoucherLine>& lines)
{
    std::vector<std::string> out;
    for (const VoucherLine& l : lines) out.push_back(l.account + (l.dept.empty() ? "" : "/" + l.dept));
    return out;
}

VoucherLine line(int number, const std::string& account, const std::string& dept, double debit, double credit)
{
    VoucherLine v;
    v.line = number;
    v.period = 8;
    v.date = "20261120";
    v.reference = "INV-77";
    v.descript = "Test line";
    v.account = account;
    v.dept = dept;
    v.debit = debit;
    v.credit = credit;
    return v;
}

VoucherHeader header(const char* mode = voucher_mode::kOpen)
{
    VoucherHeader h;
    h.mode = mode;
    h.description = "November accrual";
    h.date = "20261120";
    h.period = 8;
    h.cheque = "000777";
    return h;
}

} // namespace

TEST_CASE("NextVoucherNumber: the last voucher + 1, or GL.INI BATCHNO + 1", "[vouchers]")
{
    FixtureDb db;
    VoucherHeaders h(db.code);
    VoucherLines l(db.code);
    REQUIRE(h.isValid());
    CHECK(NextVoucherNumber(h, 99) == 104);
    for (long n : {100L, 101L, 102L, 103L}) DeleteVoucher(h, l, n);
    CHECK(h.recCount() == 0);
    CHECK(NextVoucherNumber(h, 99) == 100);   // the original saved 1 here (D11)
}

TEST_CASE("LoadVoucher: debit lines first, renumbered, totals rewritten", "[vouchers]")
{
    FixtureDb db;
    VoucherHeaders h(db.code);
    VoucherLines l(db.code);
    VoucherHeader hdr;
    std::vector<VoucherLine> lines;

    REQUIRE(LoadVoucher(h, l, 101, hdr, lines) == LoadResult::Loaded);
    CHECK(hdr.number == 101);
    CHECK(hdr.mode == "1");
    CHECK(hdr.description == "October sales");
    CHECK(hdr.period == 7);
    CHECK(hdr.date == "20261015");
    CHECK(accounts(lines) == std::vector<std::string>{"111101", "410001/10", "410001/20"});
    CHECK(lines[0].line == 1);
    CHECK(lines[2].line == 3);
    CHECK(lines[1].medrep == "MR001");
    CHECK(lines[1].drcode == "DR001");
    CHECK(lines[1].prod == "P100");
    CHECK(lines[1].period == 7);
    CHECK(hdr.debit == Catch::Approx(1150.0));
    CHECK(hdr.credit == Catch::Approx(1150.0));
    CHECK(hdr.lines == 3);
    CHECK(Balanced(hdr.debit, hdr.credit));

    REQUIRE(LoadVoucher(h, l, 102, hdr, lines) == LoadResult::Loaded);
    CHECK_FALSE(Balanced(hdr.debit, hdr.credit));
    CHECK(hdr.debit == Catch::Approx(100.0));
    CHECK(hdr.credit == Catch::Approx(90.0));

    CHECK(LoadVoucher(h, l, 100, hdr, lines) == LoadResult::Posted);
    CHECK(LoadVoucher(h, l, 999, hdr, lines) == LoadResult::NotFound);
}

TEST_CASE("LoadVoucher writes the new line numbers back", "[vouchers]")
{
    FixtureDb db;
    VoucherHeaders h(db.code);
    VoucherLines l(db.code);
    // A voucher whose credit line was entered first.
    const long n = SaveLine(h, l, 0, header(), line(1, "410001", "10", 0, 40.0), 99);
    SaveLine(h, l, n, header(), line(2, "111101", "", 40.0, 0), 99);
    VoucherHeader hdr;
    std::vector<VoucherLine> lines;
    REQUIRE(LoadVoucher(h, l, n, hdr, lines) == LoadResult::Loaded);
    CHECK(accounts(lines) == std::vector<std::string>{"111101", "410001/10"});
    // Reloading reads the stored numbers: unchanged, debit line still 1.
    VoucherHeaders h2(db.code);
    VoucherLines l2(db.code);
    REQUIRE(LoadVoucher(h2, l2, n, hdr, lines) == LoadResult::Loaded);
    CHECK(lines[0].line == 1);
    CHECK(lines[0].account == "111101");
    CHECK(lines[1].line == 2);
}

TEST_CASE("SaveLine: a new voucher, numbered as shown", "[vouchers]")
{
    FixtureDb db;
    VoucherHeaders h(db.code);
    VoucherLines l(db.code);
    VoucherLine v = line(1, "111101", "", 500.0, 0);
    v.medrep = "MR003";
    v.drcode = "DR002";
    v.prod = "P300";
    SaveKind kind{};
    const long n = SaveLine(h, l, 104, header(), v, 99, &kind);
    CHECK(kind == SaveKind::NewVoucher);
    CHECK(n == 104);
    REQUIRE(SeekVoucher(h, 104));
    const VoucherHeader back = ReadHeader(h);
    CHECK(back.mode == "1");
    CHECK(back.description == "November accrual");
    CHECK(back.period == 8);
    CHECK(back.cheque == "000777");
    CHECK(back.debit == Catch::Approx(500.0));
    CHECK(back.credit == 0.0);
    CHECK(back.lines == 1);

    VoucherHeader hdr;
    std::vector<VoucherLine> lines;
    REQUIRE(LoadVoucher(h, l, 104, hdr, lines) == LoadResult::Loaded);
    REQUIRE(lines.size() == 1);
    CHECK(lines[0].medrep == "MR003");
    CHECK(lines[0].drcode == "DR002");
    CHECK(lines[0].prod == "P300");
    CHECK(lines[0].reference == "INV-77");
    CHECK(lines[0].period == 8);
    l.SetIndex("batchno");
    REQUIRE(l.seek(104.0) == r4success);
    CHECK(std::string(l.period.str()) == " 8");   // right-aligned in C(2), as the real rows
}

TEST_CASE("SaveLine: replace a line by number, or append one", "[vouchers]")
{
    FixtureDb db;
    VoucherHeaders h(db.code);
    VoucherLines l(db.code);
    VoucherHeader hdr;
    std::vector<VoucherLine> lines;
    REQUIRE(LoadVoucher(h, l, 101, hdr, lines) == LoadResult::Loaded);

    VoucherLine changed = lines[1];   // 410001/10, credit 600
    changed.credit = 700.0;
    SaveKind kind{};
    CHECK(SaveLine(h, l, 101, hdr, changed, 99, &kind) == 101);
    CHECK(kind == SaveKind::LineReplaced);
    REQUIRE(LoadVoucher(h, l, 101, hdr, lines) == LoadResult::Loaded);
    CHECK(lines.size() == 3);
    CHECK(hdr.credit == Catch::Approx(1250.0));
    CHECK_FALSE(Balanced(hdr.debit, hdr.credit));

    CHECK(SaveLine(h, l, 101, header(), line(4, "111101", "", 100.0, 0), 99, &kind) == 101);
    CHECK(kind == SaveKind::LineAppended);
    REQUIRE(LoadVoucher(h, l, 101, hdr, lines) == LoadResult::Loaded);
    CHECK(lines.size() == 4);
    CHECK(hdr.lines == 4);
    CHECK(Balanced(hdr.debit, hdr.credit));   // 1250 / 1250
    CHECK(hdr.description == "November accrual");   // the header's fields are replaced too
}

TEST_CASE("SaveLine keeps an adjustment voucher's mode", "[vouchers]")
{
    FixtureDb db;
    VoucherHeaders h(db.code);
    VoucherLines l(db.code);
    SaveLine(h, l, 103, header(voucher_mode::kOpen), line(3, "144910", "", 5.0, 0), 99);
    REQUIRE(SeekVoucher(h, 103));
    CHECK(ReadHeader(h).mode == "3");   // the original's ReplaceHeader wrote "1" (D11)

    const long n = SaveLine(h, l, 0, header(voucher_mode::kAdjustment), line(1, "144910", "", 5.0, 0), 99);
    REQUIRE(SeekVoucher(h, n));
    CHECK(ReadHeader(h).mode == "3");
}

TEST_CASE("DeleteLine and DeleteVoucher", "[vouchers]")
{
    FixtureDb db;
    VoucherHeaders h(db.code);
    VoucherLines l(db.code);
    VoucherHeader hdr;
    std::vector<VoucherLine> lines;

    CHECK_FALSE(DeleteLine(h, l, 102, 9));
    CHECK(DeleteLine(h, l, 102, 2));
    REQUIRE(LoadVoucher(h, l, 102, hdr, lines) == LoadResult::Loaded);
    CHECK(lines.size() == 1);
    CHECK(hdr.debit == Catch::Approx(100.0));
    CHECK(hdr.credit == 0.0);
    CHECK(hdr.lines == 1);

    const long linesBefore = l.recCount();
    DeleteVoucher(h, l, 101);
    CHECK_FALSE(SeekVoucher(h, 101));
    CHECK(l.recCount() == linesBefore - 3);
    CHECK(LoadVoucher(h, l, 101, hdr, lines) == LoadResult::NotFound);
    CHECK(LoadVoucher(h, l, 103, hdr, lines) == LoadResult::Loaded);   // the others are untouched
}

TEST_CASE("OpenPeriods: the open periods, last first", "[vouchers]")
{
    FixtureDb db;
    const std::vector<OpenPeriod> p = OpenPeriods(db.code);
    REQUIRE(p.size() == 6);
    CHECK(p.front().number == 12);
    CHECK(p.back().number == 7);
    CHECK(p.back().description == "Oct 26");
}
