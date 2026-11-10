//
// gl_tests: Post, Recover and Cancel (gl/port/Vouchers.cpp, plan S2-M4) on
// the dev fixture's vouchers:
//   100  posted ("0"): 102101 Dr 250 / 101000 Cr 250 in period 7, with its
//        two gl_dist rows
//   101  open, balanced: 111101 Dr 1150 / 410001-10 Cr 600 / 410001-20 Cr 550
//   102  open, out of balance: 148000 Dr 100 / 260007 Cr 90
//   103  an open adjustment ("3"): 144910 Dr 75 / 111101 Cr 75
//

#include "GlTestDb.h"

#include "Accounts.h"
#include "Vouchers.h"
#include "db_gl_dist.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <string>
#include <vector>

using namespace gltest;
using Catch::Approx;

namespace {

// A field's value for comparing: trimmed, with a blank (NULL) number and
// 0.00 the same, as they are to the books.
std::string value(const Field4& f)
{
    const std::string t = TrimText(f.str());
    char* end = nullptr;
    const double v = std::strtod(t.c_str(), &end);
    if (t.empty() || (end && *end == '\0' && v == 0.0)) return "";
    return t;
}

// Every live row of `table` as text, without the update / user / time
// stamps, sorted: equal snapshots mean equal tables.
std::vector<std::string> snapshot(Code4& code, const char* table)
{
    Data4 d(code, table);
    REQUIRE(d.isValid());
    std::vector<Field4> fields;
    for (int i = 1; i <= d.numFields(); ++i) {
        Field4 f(d, i);
        const std::string name = f.name();
        if (name == "update" || name == "user" || name == "time" || name == "UPDATE" || name == "USER" ||
            name == "TIME")
            continue;
        fields.push_back(f);
    }
    std::vector<std::string> rows;
    for (d.top(); !d.eof(); d.skip()) {
        if (d.deleted()) continue;
        std::string row;
        for (const Field4& f : fields) row += value(f) + "|";
        rows.push_back(row);
    }
    std::sort(rows.begin(), rows.end());
    return rows;
}

struct Books {
    std::vector<std::string> accts, dist, btch, btchd;
    bool operator==(const Books& o) const
    {
        return accts == o.accts && dist == o.dist && btch == o.btch && btchd == o.btchd;
    }
};

Books books(Code4& code)
{
    return {snapshot(code, "glaccts"), snapshot(code, "gl_dist"), snapshot(code, "gl_btch"),
            snapshot(code, "gl_btchd")};
}

struct Balances {
    double period7 = 0, curr = 0, open = 0;
};

Balances balances(Code4& code, const char* account, const char* dept = "")
{
    AccountsTable gl(code);
    REQUIRE(SeekAccount(gl, account, dept));
    return {gl.period7->num(), gl.curr_bal->num(), gl.openbal->num()};
}

std::string mode(Code4& code, long number)
{
    VoucherHeaders h(code);
    REQUIRE(SeekVoucher(h, number));
    return TrimText(h.mode.str());
}

std::vector<std::string> lineModes(Code4& code, long number)
{
    VoucherLines l(code);
    std::vector<std::string> out;
    l.SetIndex("batchno");
    if (l.seek(static_cast<double>(number)) == r4success)
        for (; !l.eof() && std::lround(l.batchno.num()) == number; l.skip()) out.push_back(TrimText(l.mode.str()));
    return out;
}

int distRows(Code4& code, long number)
{
    Tgldist d(&code);
    int n = 0;
    for (d.top(); !d.eof(); d.skip())
        if (!d.deleted() && std::lround(d.batchno->num()) == number) ++n;
    return n;
}

} // namespace

TEST_CASE("ListVouchers: the rows of Post, Recover and Cancel", "[posting]")
{
    FixtureDb db;
    VoucherHeaders h(db.code);
    auto numbers = [](const std::vector<VoucherHeader>& v) {
        std::vector<long> out;
        for (const VoucherHeader& x : v) out.push_back(x.number);
        return out;
    };
    CHECK(numbers(ListVouchers(h, {voucher_mode::kOpen, voucher_mode::kAdjustment})) == std::vector<long>{101, 102, 103});
    CHECK(numbers(ListVouchers(h, {voucher_mode::kPosted})) == std::vector<long>{100});
    const auto posted = ListVouchers(h, {voucher_mode::kPosted});
    REQUIRE(posted.size() == 1);
    CHECK(posted[0].description == "Cash to checking");   // gl_btch.reference, not the missing descript
    CHECK(posted[0].debit == 250.0);
}

TEST_CASE("PostAll: balanced vouchers post, the out-of-balance one is refused", "[posting]")
{
    FixtureDb db;
    const Balances ar = balances(db.code, "111101");
    const Balances s10 = balances(db.code, "410001", "10");
    const Balances s20 = balances(db.code, "410001", "20");
    const Balances transit = balances(db.code, "144910");
    const Balances vat = balances(db.code, "148000");
    const int distBefore = static_cast<int>(snapshot(db.code, "gl_dist").size());

    const PostResult r = PostAll(db.code);
    CHECK(r.posted == std::vector<long>{101, 103});
    REQUIRE(r.refused.size() == 1);
    CHECK(r.refused[0].number == 102);
    CHECK(r.refused[0].reason == PostRefusal::OutOfBalance);

    // 101, a normal voucher: period 7 and curr_bal move by debit - credit.
    const Balances ar2 = balances(db.code, "111101");
    CHECK(ar2.period7 == Approx(ar.period7 + 1150.0));
    CHECK(balances(db.code, "410001", "10").period7 == Approx(s10.period7 - 600.0));
    CHECK(balances(db.code, "410001", "10").curr == Approx(s10.curr - 600.0));
    CHECK(balances(db.code, "410001", "20").curr == Approx(s20.curr - 550.0));
    CHECK(mode(db.code, 101) == "0");
    CHECK(lineModes(db.code, 101) == std::vector<std::string>{"0", "0", "0"});
    CHECK(distRows(db.code, 101) == 3);

    // gl_dist copies each line, codes included.
    {
        Tgldist d(&db.code);
        bool found = false;
        for (d.top(); !d.eof(); d.skip())
            if (std::lround(d.batchno->num()) == 101 && TrimText(d.dept->str()) == "10") {
                found = true;
                CHECK(TrimText(d.account->str()) == "410001");
                CHECK(TrimText(d.medrep->str()) == "MR001");
                CHECK(TrimText(d.drcode->str()) == "DR001");
                CHECK(TrimText(d.prod->str()) == "P100");
                CHECK(TrimText(d.period->str()) == "7");
                CHECK(TrimText(d.descript->str()) == "Analgesic sales");
                CHECK(d.credit->num() == 600.0);
                CHECK(TrimText(d.type->str()).empty());   // PostGLSlip never set it
            }
        CHECK(found);
    }
    CHECK(static_cast<int>(snapshot(db.code, "gl_dist").size()) == distBefore + 3);

    // 103, an adjustment: opening balances, curr_bal recomputed, mode "2", no gl_dist.
    const Balances transit2 = balances(db.code, "144910");
    CHECK(transit2.open == Approx(transit.open + 75.0));
    CHECK(transit2.period7 == Approx(transit.period7));
    CHECK(ar2.open == Approx(ar.open - 75.0));
    CHECK(mode(db.code, 103) == "2");
    CHECK(distRows(db.code, 103) == 0);
    CHECK(lineModes(db.code, 103) == std::vector<std::string>{"", ""});
    {
        AccountsTable gl(db.code);
        REQUIRE(SeekAccount(gl, "144910", ""));
        double sum = gl.openbal->num();
        for (Field4* f : {gl.period1, gl.period2, gl.period3, gl.period4, gl.period5, gl.period6, gl.period7,
                          gl.period8, gl.period9, gl.period10, gl.period11, gl.period12})
            sum += f->num();
        CHECK(gl.curr_bal->num() == Approx(sum));
    }

    // 102 stays open, untouched.
    CHECK(mode(db.code, 102) == "1");
    CHECK(balances(db.code, "148000").curr == Approx(vat.curr));

    // Posted vouchers are no longer listed or loadable.
    VoucherHeaders h(db.code);
    VoucherLines l(db.code);
    CHECK(ListVouchers(h, {voucher_mode::kOpen, voucher_mode::kAdjustment}).size() == 1);
    VoucherHeader hdr;
    std::vector<VoucherLine> lines;
    CHECK(LoadVoucher(h, l, 103, hdr, lines) == LoadResult::Posted);   // "2" is posted too

    // A second run posts nothing more.
    const PostResult again = PostAll(db.code);
    CHECK(again.posted.empty());
    CHECK(again.refused.size() == 1);
}

TEST_CASE("PostVoucher: one voucher, the others untouched", "[posting]")
{
    FixtureDb db;
    const PostResult refused = PostVoucher(db.code, 102);
    CHECK(refused.posted.empty());
    REQUIRE(refused.refused.size() == 1);
    CHECK(refused.refused[0].reason == PostRefusal::OutOfBalance);

    const PostResult r = PostVoucher(db.code, 101);
    CHECK(r.posted == std::vector<long>{101});
    CHECK(mode(db.code, 101) == "0");
    CHECK(mode(db.code, 103) == "3");

    CHECK(PostVoucher(db.code, 100).posted.empty());   // already posted: neither posted nor refused
    CHECK(PostVoucher(db.code, 100).refused.empty());
    CHECK(PostVoucher(db.code, 103).posted == std::vector<long>{103});   // an adjustment, picked
}

TEST_CASE("Posting compares the lines to the cent, not the header totals", "[posting]")
{
    FixtureDb db;
    {
        VoucherHeaders h(db.code);
        REQUIRE(SeekVoucher(h, 102));
        h.credit.assignDouble(100.0);   // a stale header that claims to balance
        h.flush();
    }
    const PostResult r = PostVoucher(db.code, 102);
    CHECK(r.posted.empty());
    REQUIRE(r.refused.size() == 1);
    CHECK(r.refused[0].reason == PostRefusal::OutOfBalance);
}

TEST_CASE("A voucher with an unknown account is refused whole", "[posting]")
{
    FixtureDb db;
    VoucherHeaders h(db.code);
    VoucherLines l(db.code);
    VoucherHeader hdr;
    hdr.mode = voucher_mode::kOpen;
    hdr.description = "Bad account";
    hdr.date = "20261020";
    hdr.period = 7;
    VoucherLine a;
    a.line = 1;
    a.period = 7;
    a.date = "20261020";
    a.account = "111101";
    a.debit = 10;
    VoucherLine b = a;
    b.line = 2;
    b.account = "999999";
    b.debit = 0;
    b.credit = 10;
    const long n = SaveLine(h, l, 0, hdr, a, 99);
    SaveLine(h, l, n, hdr, b, 99);
    const Balances ar = balances(db.code, "111101");

    const PostResult r = PostVoucher(db.code, n);
    CHECK(r.posted.empty());
    REQUIRE(r.refused.size() == 1);
    CHECK(r.refused[0].reason == PostRefusal::UnknownAccount);
    CHECK(r.refused[0].account == "999999");
    CHECK(balances(db.code, "111101").curr == Approx(ar.curr));   // the good line wasn't posted either
    CHECK(mode(db.code, n) == "1");
    CHECK(distRows(db.code, n) == 0);
}

TEST_CASE("A failure part-way through posting writes nothing; a retry posts", "[posting]")
{
    FixtureDb db;
    const Books before = books(db.code);
    SetPostFault(2);   // inside voucher 101, after its second line
    CHECK_THROWS_AS(PostAll(db.code), PostFault);
    SetPostFault(0);
    CHECK(books(db.code) == before);

    const PostResult r = PostAll(db.code);
    CHECK(r.posted == std::vector<long>{101, 103});
    CHECK_FALSE(books(db.code) == before);

    // The fault on the last line of the run (101 has 3, 103 has 2).
    FixtureDb db2;
    const Books before2 = books(db2.code);
    SetPostFault(5);
    CHECK_THROWS_AS(PostAll(db2.code), PostFault);
    SetPostFault(0);
    CHECK(books(db2.code) == before2);
}

TEST_CASE("RecoverVoucher returns the books to their values before posting", "[posting]")
{
    FixtureDb db;
    const Books before = books(db.code);
    REQUIRE(PostVoucher(db.code, 101).posted == std::vector<long>{101});
    CHECK(RecoverVoucher(db.code, 101) == RecoverResult::Recovered);
    CHECK(books(db.code) == before);   // glaccts, gl_dist, gl_btch ("1") and gl_btchd (blank modes)
    CHECK(mode(db.code, 101) == "1");

    // It can be posted again.
    CHECK(PostVoucher(db.code, 101).posted == std::vector<long>{101});
}

TEST_CASE("RecoverVoucher on the fixture's posted voucher", "[posting]")
{
    FixtureDb db;
    const Balances checking = balances(db.code, "102101");
    const Balances petty = balances(db.code, "101000");
    // An AR row in gl_dist that happens to share voucher 100's number.
    {
        Tgldist d(&db.code);
        d.appendStart();
        d.account->assign("111101");
        d.type->assign("AR");
        d.batchno->assignLong(100);
        d.debit->assignDouble(5);
        d.append();
    }
    REQUIRE(distRows(db.code, 100) == 3);

    CHECK(RecoverVoucher(db.code, 100) == RecoverResult::Recovered);
    CHECK(balances(db.code, "102101").period7 == Approx(checking.period7 - 250.0));
    CHECK(balances(db.code, "102101").curr == Approx(checking.curr - 250.0));
    CHECK(balances(db.code, "101000").period7 == Approx(petty.period7 + 250.0));
    CHECK(mode(db.code, 100) == "1");
    CHECK(lineModes(db.code, 100) == std::vector<std::string>{"", ""});
    CHECK(distRows(db.code, 100) == 1);   // the AR row stays

    CHECK(RecoverVoucher(db.code, 100) == RecoverResult::NotPosted);
    CHECK(RecoverVoucher(db.code, 102) == RecoverResult::NotPosted);
    CHECK(RecoverVoucher(db.code, 999) == RecoverResult::NotPosted);
}

TEST_CASE("Recover refuses a posted adjustment and an unknown account", "[posting]")
{
    FixtureDb db;
    REQUIRE(PostVoucher(db.code, 103).posted == std::vector<long>{103});
    CHECK(RecoverVoucher(db.code, 103) == RecoverResult::NotPosted);   // "2": the original listed only "0"

    {
        VoucherLines l(db.code);
        l.SetIndex("batchno");
        REQUIRE(l.seek(100.0) == r4success);
        l.account.assign("999999");
        l.flush();
    }
    const Books before = books(db.code);
    std::string account;
    CHECK(RecoverVoucher(db.code, 100, &account) == RecoverResult::UnknownAccount);
    CHECK(account == "999999");
    CHECK(books(db.code) == before);
}

TEST_CASE("CancelVoucher removes an unposted voucher", "[posting]")
{
    FixtureDb db;
    CHECK(CancelVoucher(db.code, 102) == CancelResult::Cancelled);
    VoucherHeaders h(db.code);
    CHECK_FALSE(SeekVoucher(h, 102));
    CHECK(lineModes(db.code, 102).empty());
    CHECK(lineModes(db.code, 101).size() == 3);

    CHECK(CancelVoucher(db.code, 103) == CancelResult::Cancelled);   // an adjustment
    CHECK(CancelVoucher(db.code, 100) == CancelResult::Posted);
    CHECK(CancelVoucher(db.code, 999) == CancelResult::NotFound);
    CHECK(SeekVoucher(h, 100));
}
