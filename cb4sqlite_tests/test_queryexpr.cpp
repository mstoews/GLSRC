//
// Layer-1 tests: the querySet expression subset (spec #2 5.1).
//
// Rows (TestDb.h addSampleAccounts), in "accounts" tag order:
//   100/01 Cash  100/02 Cash branch  200/01 Payables (deleted)  300/01 Revenue  400/01 Expenses
// ACCOUNT is C(7) and DEPT C(3), stored padded: "100    ", "01 ".
//

#include "TestDb.h"

using namespace test;
using Catch::Matchers::ContainsSubstring;
using V = std::vector<std::string>;

namespace {

struct Rows {
    TempDb db;
    Code4  code;
    Data4* d = nullptr;
    Rows()
    {
        db.open(code);
        createAccounts(code);
        d = new Data4(code, "accts");
        addSampleAccounts(*d);
        REQUIRE(d->SetIndex("accounts") == r4success);
        REQUIRE(d->seek("200") == r4success);
        d->deleteRec();
    }
    ~Rows() { delete d; }

    V forward(const char* query)
    {
        Relate4set q(d);
        REQUIRE(q.querySet(query) == r4success);
        V keys;
        for (int rc = q.top(); rc != r4eof; rc = q.skip(1L))
            keys.push_back(key(*d));
        return keys;
    }
    V backward(const char* query)
    {
        Relate4set q(d);
        REQUIRE(q.querySet(query) == r4success);
        V keys;
        for (int rc = q.bottom(); rc == r4success; rc = q.skip(-1L))
            keys.push_back(key(*d));
        return keys;
    }
};

} // namespace

TEST_CASE("= is a left match on the padded value (dBase SET EXACT OFF)", "[query]")
{
    Rows fx;
    CHECK(fx.forward("ACCOUNT = '1'") == V{"100/01", "100/02"});
    CHECK(fx.forward("ACCOUNT = '100'") == V{"100/01", "100/02"});
    CHECK(fx.forward("ACCOUNT = '100    '") == V{"100/01", "100/02"});    // trailing blanks are part of the value
    CHECK(fx.forward("ACCOUNT = '100   X'").empty());
    CHECK(fx.forward("ACCOUNT = '100     X'").empty());                   // longer than the field: never equal
    CHECK(fx.forward("ACCOUNT = '10 '").empty());                         // "100" doesn't start with "10 "
    CHECK(fx.forward("ACCOUNT = ''") == V{"100/01", "100/02", "200/01", "300/01", "400/01"});
    CHECK(fx.forward("DEPT = '01 '") == V{"100/01", "200/01", "300/01", "400/01"});
}

TEST_CASE("<>, # and != are the negation of =", "[query]")
{
    Rows fx;
    const V notOne{"200/01", "300/01", "400/01"};
    CHECK(fx.forward("ACCOUNT <> '1'") == notOne);
    CHECK(fx.forward("ACCOUNT # '1'") == notOne);
    CHECK(fx.forward("ACCOUNT != '1'") == notOne);
    CHECK(fx.forward("ACCOUNT <> ' '") == V{"100/01", "100/02", "200/01", "300/01", "400/01"});   // gl_acctl.cpp
    CHECK(fx.forward("ACCOUNT <> ''").empty());
}

TEST_CASE("Keywords, field names and quotes", "[query]")
{
    Rows fx;
    CHECK(fx.forward("account = \"3\"") == V{"300/01"});
    CHECK(fx.forward("  Account='3'  ") == V{"300/01"});
    CHECK(fx.forward("DESCRIPT = 'Cash'") == V{"100/01", "100/02"});
    CHECK(fx.forward("DESCRIPT = 'cash'").empty());                       // values compare case-sensitively
    CHECK(fx.forward("DESCRIPT = \"Cash' OR '1'='1\"").empty());          // a value is bound, never SQL
}

TEST_CASE(".AND. and DELETED() combine; the parent's tag order is kept", "[query]")
{
    Rows fx;
    CHECK(fx.forward("DEPT = '01' .AND. .NOT. DELETED()") == V{"100/01", "300/01", "400/01"});
    CHECK(fx.forward("dept = '01' .and. account <> '4' .and. !deleted()") == V{"100/01", "300/01"});
    CHECK(fx.backward("DEPT = '01' .AND. .NOT. DELETED()") == V{"400/01", "300/01", "100/01"});
    REQUIRE(fx.d->SetIndex("descrip") == r4success);
    CHECK(fx.forward("DEPT = '01'") == V{"100/01", "400/01", "200/01", "300/01"});
    CHECK(fx.backward("DEPT = '01'") == V{"300/01", "200/01", "400/01", "100/01"});
}

TEST_CASE("The gl_trans.cpp inquiry shape", "[query]")
{
    Rows fx;
    // gl_trans.cpp:193: "ACCOUNT = '%s' .AND. DEPT = '%s' .AND. MODE = '0'", here without MODE.
    CHECK(fx.forward("ACCOUNT = '100' .AND. DEPT = '02'") == V{"100/02"});
    CHECK(fx.forward("ACCOUNT = '100' .AND. DEPT = ''") == V{"100/01", "100/02"});
}

TEST_CASE("A NULL character value reads as blanks", "[query]")
{
    Rows fx;
    REQUIRE(raw(fx.code, "UPDATE accts SET dept = NULL WHERE account LIKE '400%'") == "<no row>");
    CHECK(fx.forward("DEPT = ' '") == V{"400/"});
    CHECK(fx.forward("DEPT <> ' '") == V{"100/01", "100/02", "200/01", "300/01"});
}

TEST_CASE("Expressions outside the grammar are rejected", "[query]")
{
    Rows fx;
    Relate4set q(fx.d);
    for (const char* bad : {"ACCOUNT == '1'", "ACCOUNT > '1'", "ACCOUNT = '1' .OR. DEPT = '01'",
                            ".NOT. ACCOUNT = '1'", "UPPER(ACCOUNT) = 'X'", "ACCOUNT = 1", "ACCOUNT = '1",
                            "ACCOUNT '1'", "= '1'", "ACCOUNT = '1' DEPT = '01'", "DELETED()", "ACCOUNT = [1]",
                            "ACCOUNT = '1' .AND."})
        CHECK_THROWS_WITH(q.querySet(bad), ContainsSubstring("not implemented"));
}

TEST_CASE("Unknown and non-character fields are errors", "[query]")
{
    Rows fx;
    Relate4set q(fx.d);
    CHECK_THROWS_WITH(q.querySet("NOSUCH = '1'"), ContainsSubstring("has no field NOSUCH"));
    CHECK_THROWS_WITH(q.querySet("CURR_BAL = '1'"), ContainsSubstring("not a character field"));
    CHECK_THROWS_WITH(q.querySet("OPENED = '2026'"), ContainsSubstring("not a character field"));
    // A failed querySet leaves the previous filter in place.
    REQUIRE(q.querySet("ACCOUNT = '3'") == r4success);
    CHECK_THROWS(q.querySet("NOSUCH = '1'"));
    REQUIRE(q.top() == r4success);
    CHECK(key(*fx.d) == "300/01");
    CHECK(q.skip(1) == r4eof);
}
