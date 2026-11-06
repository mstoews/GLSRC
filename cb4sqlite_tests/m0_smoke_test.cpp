//
// m0_smoke_test.cpp - proves the Catch2 harness builds and runs (plan M0).
//
// Only exercises code that is real in M0: the header-only CodeBase helpers and
// the SQLite amalgamation linked into cb4sqlite.lib. Plan M1 adds the Layer-1
// shim tests next to this file.
//
#include "catch_amalgamated.hpp"

#include "cb4sqlite.h"
#include "sqlite3.h"

#include <cstring>

TEST_CASE("CodeBase numeric helpers parse like Str4ptr / Str4long", "[m0]")
{
    CHECK(Str4ptr("123.45") == Catch::Approx(123.45));
    CHECK(Str4ptr("") == 0.0);
    CHECK(Str4ptr(nullptr) == 0.0);
    CHECK(Str4long("42") == 42L);
}

TEST_CASE("Vendored SQLite links and opens an in-memory database", "[m0]")
{
    CHECK(std::strcmp(sqlite3_libversion(), SQLITE_VERSION) == 0);

    sqlite3* db = nullptr;
    REQUIRE(sqlite3_open(":memory:", &db) == SQLITE_OK);
    CHECK(sqlite3_exec(db, "CREATE TABLE t(x); INSERT INTO t VALUES (1);",
                       nullptr, nullptr, nullptr) == SQLITE_OK);
    sqlite3_close(db);
}
