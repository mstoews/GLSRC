//
// Layer-1 tests: cb4sqlite::Query, the read-only report query (spec #3 7).
//

#include "TestDb.h"

using namespace test;
using Catch::Matchers::ContainsSubstring;

namespace {

struct Books {
    TempDb db;
    Code4  code;
    Books()
    {
        db.open(code);
        createAccounts(code);
        Data4 d(code, "accts");
        addSampleAccounts(d);
    }
};

} // namespace

TEST_CASE("Query reads rows in SQL order, with trimmed text and numbers", "[query]")
{
    Books b;
    cb4sqlite::Query q(b.code, "SELECT account, dept, descript, curr_bal FROM accts "
                               "WHERE _deleted = 0 ORDER BY account, dept");
    REQUIRE(q.columns() == 4);
    std::vector<std::string> keys;
    double total = 0;
    while (q.next()) {
        keys.push_back(q.text(0) + "/" + q.text(1));
        total += q.number(3);
    }
    CHECK(keys == std::vector<std::string>{"100/01", "100/02", "200/01", "300/01", "400/01"});
    CHECK(total == Catch::Approx(-252.25));
}

TEST_CASE("Query binds text, integers, doubles and NULL", "[query]")
{
    Books b;
    cb4sqlite::Query q(b.code, "SELECT count(*) FROM accts WHERE dept = ?1 AND curr_bal > ?2");
    q.bindText(1, "01 ").bindDouble(2, 0.0);
    REQUIRE(q.next());
    CHECK(q.integer(0) == 2);   // 100/01 and 400/01

    q.reset();
    q.bindText(1, "02 ").bindInt(2, 100);
    REQUIRE(q.next());
    CHECK(q.integer(0) == 0);

    cb4sqlite::Query n(b.code, "SELECT ?1 IS NULL");
    n.bindNull(1);
    REQUIRE(n.next());
    CHECK(n.integer(0) == 1);
}

TEST_CASE("Query reads NULL and blank numbers as zero", "[query]")
{
    Books b;
    cb4sqlite::Query q(b.code, "SELECT NULL, '   ', ' 12.50 ', 7, 'abc   '");
    REQUIRE(q.next());
    CHECK(q.isNull(0));
    CHECK_FALSE(q.isNull(1));
    CHECK(q.number(0) == 0.0);
    CHECK(q.number(1) == 0.0);
    CHECK(q.integer(1) == 0);
    CHECK(q.number(2) == Catch::Approx(12.5));
    CHECK(q.integer(3) == 7);
    CHECK(q.number(3) == 7.0);
    CHECK(q.text(0).empty());
    CHECK(q.text(4) == "abc");
    CHECK_FALSE(q.next());
}

TEST_CASE("Query leaves deleted rows to the WHERE clause", "[query]")
{
    Books b;
    {
        Data4 d(b.code, "accts");
        REQUIRE(d.SetIndex("accounts") == r4success);
        REQUIRE(d.seek("200    01") == r4success);
        d.deleteRec();
    }
    cb4sqlite::Query all(b.code, "SELECT count(*) FROM accts");
    REQUIRE(all.next());
    CHECK(all.integer(0) == 5);
    cb4sqlite::Query live(b.code, "SELECT count(*) FROM accts WHERE _deleted = 0");
    REQUIRE(live.next());
    CHECK(live.integer(0) == 4);
}

TEST_CASE("Query refuses writing statements and reports bad SQL with its text", "[query]")
{
    Books b;
    CHECK_THROWS_WITH(cb4sqlite::Query(b.code, "DELETE FROM accts"), ContainsSubstring("read-only") &&
                                                                       ContainsSubstring("DELETE FROM accts"));
    CHECK_THROWS_WITH(cb4sqlite::Query(b.code, "UPDATE accts SET curr_bal = 0"), ContainsSubstring("read-only"));
    CHECK_THROWS_WITH(cb4sqlite::Query(b.code, "SELECT nosuch FROM accts"),
                      ContainsSubstring("nosuch") && ContainsSubstring("SELECT nosuch FROM accts"));
    CHECK(raw(b.code, "SELECT count(*) FROM accts") == "5");
}

TEST_CASE("Query checks for a current row and the column number", "[query]")
{
    Books b;
    cb4sqlite::Query q(b.code, "SELECT account FROM accts");
    CHECK_THROWS_WITH(q.text(0), ContainsSubstring("no current row"));
    REQUIRE(q.next());
    CHECK_THROWS_WITH(q.text(1), ContainsSubstring("out of range"));

    Code4 closed;
    CHECK_THROWS_WITH(cb4sqlite::Query(closed, "SELECT 1"), ContainsSubstring("no database is open"));
}
