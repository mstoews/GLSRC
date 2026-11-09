//
// Layer-1 tests: Code4 transactions and cb4sqlite::Transaction (spec #2 5.2).
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
    std::string count() { return raw(code, "SELECT count(*) FROM accts"); }
    std::string balance(const char* account)
    {
        return raw(code, std::string("SELECT curr_bal FROM accts WHERE account LIKE '") + account + "%' AND dept = '01 '");
    }
};

// The posting pattern: seek, change a field, move on (the write happens on the move).
void addToBalance(Data4& d, const char* key, double amount)
{
    REQUIRE(d.seek(key) == r4success);
    Field4 bal(d, "curr_bal");
    bal.assignDouble(bal.num() + amount);
}

} // namespace

TEST_CASE("commit keeps every write; rollback undoes appends, updates and deletes", "[txn]")
{
    Books b;
    {
        Data4 d(b.code, "accts");
        REQUIRE(d.SetIndex("accounts") == r4success);
        b.code.begin();
        CHECK(b.code.inTransaction());
        addToBalance(d, "100    01", 10);
        addToBalance(d, "300    01", -10);
        addAccount(d, "500", "01", "New", 1);
        b.code.commit();
        CHECK_FALSE(b.code.inTransaction());
    }
    CHECK(b.balance("100") == "260.5");
    CHECK(b.balance("300") == "-510.0");
    CHECK(b.count() == "6");

    {
        Data4 d(b.code, "accts");
        REQUIRE(d.SetIndex("accounts") == r4success);
        b.code.begin();
        addToBalance(d, "100    01", 1000);
        addAccount(d, "600", "01", "Gone", 1);
        REQUIRE(d.seek("400") == r4success);
        d.deleteRec();
        REQUIRE(d.seek("500") == r4success);
        d.deleteRec();
        REQUIRE(d.pack() == r4success);
        b.code.rollback();
    }
    CHECK(b.balance("100") == "260.5");
    CHECK(b.count() == "6");
    CHECK(raw(b.code, "SELECT count(*) FROM accts WHERE _deleted <> 0") == "0");
}

TEST_CASE("commit writes the changed record still in a buffer", "[txn]")
{
    Books b;
    Data4 d(b.code, "accts");
    REQUIRE(d.SetIndex("accounts") == r4success);
    b.code.begin();
    addToBalance(d, "100    01", 5);   // not moved off: only in the buffer
    b.code.commit();
    CHECK(b.balance("100") == "255.5");
}

TEST_CASE("rollback drops the changed record still in a buffer", "[txn]")
{
    Books b;
    Data4 d(b.code, "accts");
    REQUIRE(d.SetIndex("accounts") == r4success);
    b.code.begin();
    addToBalance(d, "100    01", 5);
    b.code.rollback();
    REQUIRE(d.top() == r4success);     // moving must not write the dropped change
    CHECK(b.balance("100") == "250.5");
}

TEST_CASE("begin writes pending changes first, so rollback doesn't take them", "[txn]")
{
    Books b;
    Data4 d(b.code, "accts");
    REQUIRE(d.SetIndex("accounts") == r4success);
    addToBalance(d, "100    01", 7);   // made before the transaction
    b.code.begin();
    addToBalance(d, "300    01", 7);
    b.code.rollback();
    CHECK(b.balance("100") == "257.5");
    CHECK(b.balance("300") == "-500.0");
}

TEST_CASE("Transaction rolls back unless committed, including on an exception", "[txn]")
{
    Books b;
    {
        Data4 d(b.code, "accts");
        REQUIRE(d.SetIndex("accounts") == r4success);
        cb4sqlite::Transaction t(b.code);
        addToBalance(d, "100    01", 1);
        d.flush();
    }   // no commit
    CHECK_FALSE(b.code.inTransaction());
    CHECK(b.balance("100") == "250.5");

    try {
        Data4 d(b.code, "accts");
        REQUIRE(d.SetIndex("accounts") == r4success);
        cb4sqlite::Transaction t(b.code);
        addAccount(d, "700", "01", "Half posted", 1);
        throw std::runtime_error("posting failed");
    } catch (const std::runtime_error&) {
    }
    CHECK_FALSE(b.code.inTransaction());
    CHECK(b.count() == "5");

    {
        Data4 d(b.code, "accts");
        REQUIRE(d.SetIndex("accounts") == r4success);
        cb4sqlite::Transaction t(b.code);
        addToBalance(d, "100    01", 1);
        t.commit();
    }
    CHECK(b.balance("100") == "251.5");
}

TEST_CASE("Transactions are not nestable, and misuse is reported", "[txn]")
{
    Books b;
    CHECK_THROWS_WITH(b.code.commit(), ContainsSubstring("no transaction is open"));
    CHECK_THROWS_WITH(b.code.rollback(), ContainsSubstring("no transaction is open"));
    b.code.begin();
    CHECK_THROWS_WITH(b.code.begin(), ContainsSubstring("already open"));
    CHECK_THROWS_AS(cb4sqlite::Transaction(b.code), cb4sqlite::Error);
    CHECK(b.code.inTransaction());       // the failed begin left the open one alone
    b.code.commit();

    Code4 closed;
    CHECK_THROWS_WITH(closed.begin(), ContainsSubstring("no database is open"));
}

TEST_CASE("close() with an open transaction rolls it back", "[txn]")
{
    Books b;
    {
        Data4 d(b.code, "accts");
        b.code.begin();
        addAccount(d, "800", "01", "Unsaved", 1);
    }
    b.code.close();
    CHECK_FALSE(b.code.inTransaction());
    b.db.open(b.code);
    CHECK(b.count() == "5");
}

TEST_CASE("The re-read rule: after rollback, reopened tables see the stored state", "[txn]")
{
    Books b;
    {
        Data4 d(b.code, "accts");
        REQUIRE(d.SetIndex("accounts") == r4success);
        b.code.begin();
        addToBalance(d, "100    01", 100);
        d.flush();
        b.code.rollback();
        b.code.closeAll();   // TGLPostBatch's cb.closeAll(); cb.init();
        b.code.init();
        CHECK_FALSE(d.isValid());
    }
    Data4 again(b.code, "accts");
    REQUIRE(again.SetIndex("accounts") == r4success);
    REQUIRE(again.seek("100    01") == r4success);
    CHECK(Field4(again, "curr_bal").num() == 250.5);
    // a retry of the same work then succeeds
    cb4sqlite::Transaction t(b.code);
    addToBalance(again, "100    01", 100);
    t.commit();
    CHECK(b.balance("100") == "350.5");
}
