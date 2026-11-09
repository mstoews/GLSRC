//
// gl_tests: the voucher-cycle table headers against the dev fixture, whose
// layouts and tags copy the real company tables (plan S2-M1 task 6; spec #2
// 5.4 records the findings pinned here).
//

#include "GlTestDb.h"

#include "db_gl_dist.h"
#include "db_gl_distb.h"

#include <cctype>
#include <string>
#include <vector>

using namespace gltest;

namespace {

std::vector<std::string> invalidFields(Data4& d, const std::vector<std::string>& names)
{
    std::vector<std::string> bad;
    for (const std::string& n : names)
        if (!Field4(d, n.c_str()).isValid()) bad.push_back(n);
    return bad;
}

std::string trimmed(const Field4& f)
{
    std::string s = f.str();
    s.erase(s.find_last_not_of(' ') + 1);
    s.erase(0, s.find_first_not_of(' '));
    return s;
}

} // namespace

TEST_CASE("Tgldist and Tgldistb open every field, in batchno order", "[tables]")
{
    FixtureDb db;
    {
        Tgldist dist(&db.code);
        REQUIRE(dist.isValid());
        for (Field4* f : {dist.account, dist.dept, dist.medrep, dist.drcode, dist.prod, dist.period, dist.date,
                          dist.type, dist.batchno, dist.reference, dist.cheque, dist.descript, dist.debit,
                          dist.credit})
            CHECK(f->isValid());
        CHECK(dist.numFields() == 14);
        REQUIRE_FALSE(dist.eof());                       // the "batchno" tag shows GL lines (type blank)
        CHECK(trimmed(*dist.batchno) == "100");
        CHECK(trimmed(*dist.period) == "7");
        CHECK(dist.debit->num() + dist.credit->num() == 250.0);
        // The generator's first tag, "ardist", is FOR TYPE = 'AR ': it hides them.
        REQUIRE(dist.SetIndex("ardist") == r4success);
        CHECK(dist.top() == r4eof);
    }
    Tgldistb archive(&db.code);
    REQUIRE(archive.isValid());
    for (Field4* f : {archive.account, archive.dept, archive.period, archive.date, archive.type, archive.batchno,
                      archive.reference, archive.cheque, archive.descript, archive.debit, archive.credit})
        CHECK(f->isValid());
    CHECK(archive.numFields() == 11);
    CHECK(archive.recCount() == 0);
}

TEST_CASE("include/ headers against the real layouts: the known mismatches", "[tables]")
{
    FixtureDb db;
    SECTION("db_btch.h: Tglbtch opens descript, which gl_btch doesn't have (it has reference)")
    {
        Data4 d(db.code, "gl_btch");
        CHECK(invalidFields(d, {"batchno", "mode", "period", "date", "descript", "debit", "credit", "entryno",
                                "cheque"}) == std::vector<std::string>{"descript"});
        CHECK(Field4(d, "reference").isValid());
    }
    SECTION("db_btchd.h: Tglbtchd lacks medrep, drcode and prod; gl_btche.cpp's 'product' is 'prod'")
    {
        Data4 d(db.code, "gl_btchd");
        CHECK(invalidFields(d, {"batchno", "mode", "journal", "period", "sourcecode", "date", "reference",
                                "descript", "cheque", "account", "dept", "debit", "credit"})
                  .empty());
        CHECK(invalidFields(d, {"medrep", "drcode", "prod", "product"}) == std::vector<std::string>{"product"});
    }
    SECTION("db_drcod.h opens fields by number, and from field 9 the numbers are off")
    {
        Data4 d(db.code, "gl_drcod");
        // Tgldrcodes: 9 = "fax", 10 = "contact", ..., 17 = "territory".
        const char* const header[] = {"fax", "contact", "ref", "status", "descript", "terms", "balance",
                                      "creditlmt", "territory"};
        const char* const real[] = {"territory", "fax", "descript", "contact", "type", "ref", "status",
                                    "terms", "balance"};
        for (int i = 0; i < 9; ++i) {
            const Field4 f(d, 9 + i);
            std::string name = f.name();
            for (char& c : name) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
            CHECK(name != header[i]);
            CHECK(name == real[i]);
        }
        CHECK(d.numFields() == 18);   // creditlmt (18) is never opened
    }
    SECTION("db_medrp.h opens by name: all present; territory and type are not opened")
    {
        Data4 d(db.code, "gl_medrp");
        CHECK(invalidFields(d, {"repcode", "rpshort", "rpname", "address1", "address2", "address3", "postal",
                                "phone", "fax", "contact", "ref", "status", "descript", "terms", "balance",
                                "creditlmt"})
                  .empty());
    }
    SECTION("db_itemm.h: all fields present; its Man() tag does not exist")
    {
        Data4 d(db.code, "gl_itemm");
        CHECK(invalidFields(d, {"fpcode", "fpname", "ffulln", "fptype", "fclass", "fskbn", "fshop1", "fplace",
                                "funit", "flotsize", "fonhand", "flmhand", "flmprice", "favprice", "fstprice"})
                  .empty());
        CHECK(d.SetIndex("MAN") == e4tagName);
        CHECK(d.SetIndex("ffulln") == r4success);   // LoadProducts
    }
    SECTION("db_dept.h matches")
    {
        Data4 d(db.code, "gldept");
        CHECK(invalidFields(d, {"depart", "descript"}).empty());
    }
}

TEST_CASE("The fixture's vouchers", "[tables]")
{
    FixtureDb db;
    Data4 btch(db.code, "gl_btch");
    REQUIRE(btch.SetIndex("batchno") == r4success);
    std::vector<std::string> modes;
    for (int rc = btch.top(); rc == r4success; rc = btch.skip())
        modes.push_back(Field4(btch, "batchno").str() + std::string(":") + Field4(btch, "mode").str());
    CHECK(modes == std::vector<std::string>{"   100:0", "   101:1", "   102:1", "   103:3"});

    // Transaction Inquiry's query shape (gl_trans.cpp:193) over the posted lines.
    Data4 lines(db.code, "gl_btchd");
    REQUIRE(lines.SetIndex("acctdept") == r4success);
    Relate4set q(lines);
    REQUIRE(q.querySet("ACCOUNT = '102101' .AND. DEPT = '' .AND. MODE = '0'") == r4success);
    REQUIRE(q.top() == r4success);
    CHECK(Field4(lines, "debit").num() == 250.0);
    CHECK(q.skip() == r4eof);
    REQUIRE(q.querySet("ACCOUNT = '410001' .AND. DEPT = '10' .AND. MODE = '0'") == r4success);
    CHECK(q.top() == r4eof);                            // that line belongs to the open voucher 101
    REQUIRE(q.querySet("ACCOUNT = '410001' .AND. DEPT = '10'") == r4success);
    REQUIRE(q.top() == r4success);
    CHECK(trimmed(Field4(lines, "medrep")) == "MR001");
    CHECK(trimmed(Field4(lines, "prod")) == "P100");
    CHECK(trimmed(Field4(lines, "drcode")) == "DR001");
}
