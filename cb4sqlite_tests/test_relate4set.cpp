//
// Layer-1 tests: Relate4set, in the shape gl_acctl.cpp TListWindow::Redraw uses it.
//

#include "TestDb.h"

using namespace test;
using Catch::Matchers::ContainsSubstring;
using V = std::vector<std::string>;

namespace {

struct Loaded {
    TempDb db;
    Code4  code;
    Data4* d = nullptr;
    Loaded()
    {
        db.open(code);
        createAccounts(code);
        d = new Data4(code, "accts");
        addSampleAccounts(*d);
        REQUIRE(d->SetIndex("accounts") == r4success);
        REQUIRE(d->seek("200") == r4success);
        d->deleteRec();                                  // 200/01 is deleted but not packed
    }
    ~Loaded() { delete d; }

    V iterate(const char* query)
    {
        Relate4set* q = new Relate4set(d);               // as the original code does
        REQUIRE(q->querySet(query) == r4success);
        V keys;
        for (int qc = q->top(); qc != r4eof; qc = q->skip(1L))
            keys.push_back(key(*d));                     // Field4s on the parent see each match
        CHECK(q->eof());
        q->unlock();
        q->free();
        delete q;
        return keys;
    }
};

} // namespace

TEST_CASE("querySet(\".NOT. DELETED()\") hides deleted records", "[relate]")
{
    Loaded fx;
    CHECK(fx.iterate(".NOT. DELETED()") == V{"100/01", "100/02", "300/01", "400/01"});
    CHECK(fx.iterate(".not.deleted()") == V{"100/01", "100/02", "300/01", "400/01"});
    CHECK(fx.iterate(" !DELETED( ) ") == V{"100/01", "100/02", "300/01", "400/01"});
}

TEST_CASE("Empty query iterates everything, in the parent's tag order", "[relate]")
{
    Loaded fx;
    CHECK(fx.iterate(nullptr) == V{"100/01", "100/02", "200/01", "300/01", "400/01"});
    REQUIRE(fx.d->SetIndex("descrip") == r4success);
    CHECK(fx.iterate("") == V{"100/01", "100/02", "400/01", "200/01", "300/01"});
}

TEST_CASE("Relate4set bottom, backwards skip and an all-deleted table", "[relate]")
{
    Loaded fx;
    Relate4set q(*fx.d);
    REQUIRE(q.querySet(".NOT. DELETED()") == r4success);
    REQUIRE(q.bottom() == r4success);
    CHECK(key(*fx.d) == "400/01");
    REQUIRE(q.skip(-1) == r4success);
    CHECK(key(*fx.d) == "300/01");
    REQUIRE(q.skip(-1) == r4success);                    // jumps over the deleted 200/01
    CHECK(key(*fx.d) == "100/02");
    CHECK(q.skip(-5) == r4bof);
    CHECK(key(*fx.d) == "100/01");

    for (int rc = fx.d->top(); rc == r4success; rc = fx.d->skip(1))
        fx.d->deleteRec();
    CHECK(q.top() == r4eof);

    q.free();
    CHECK(q.top() == e4info);                            // used after free()
    CHECK(q.eof());
}

TEST_CASE("Unsupported query expressions are reported, not ignored", "[relate]")
{
    Loaded fx;
    Relate4set q(fx.d);
    CHECK_THROWS_WITH(q.querySet("ACCOUNT <> ' '"), ContainsSubstring("not implemented in spec #1"));
}
