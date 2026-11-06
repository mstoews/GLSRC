//
// Layer-1 tests: Data4 navigation, tags, seek, writes, delete/pack/zap.
//

#include "TestDb.h"

using namespace test;
using V = std::vector<std::string>;

namespace {

struct Fixture {
    TempDb db;
    Code4  code;
    Fixture()
    {
        db.open(code);
        createAccounts(code);
    }
};

struct Loaded : Fixture {
    Data4 d{code, "accts"};
    Loaded() { addSampleAccounts(d); }
};

} // namespace

TEST_CASE("Navigation on an empty table", "[data4][nav]")
{
    Fixture fx;
    Data4 d(fx.code, "accts");
    REQUIRE(d.isValid());
    CHECK(d.eof());                       // opened, not positioned
    CHECK(d.top() == r4eof);
    CHECK(d.eof());
    CHECK(d.bof());
    CHECK(d.bottom() == r4eof);
    CHECK(d.skip(1) == r4eof);
    CHECK(d.recCount() == 0);
    CHECK(d.recNo() == 1);                // recCount() + 1 at eof
    CHECK(d.numFields() == 7);
    CHECK(d.fieldNumber("curr_bal") == 4);
    CHECK(d.fieldNumber("nope") == 0);
}

TEST_CASE("Tag order: top, skip, bottom, eof and bof", "[data4][nav]")
{
    Loaded fx;
    Data4& d = fx.d;
    REQUIRE(d.SetIndex("accounts") == r4success);
    CHECK(walk(d) == V{"100/01", "100/02", "200/01", "300/01", "400/01"});
    CHECK(d.eof());
    CHECK(d.recNo() == d.recCount() + 1);
    CHECK(std::string(Field4(d, "account").str()) == "       ");   // blank record at eof

    // Backwards from the bottom.
    REQUIRE(d.bottom() == r4success);
    V back{key(d)};
    while (d.skip(-1) == r4success) back.push_back(key(d));
    CHECK(back == V{"400/01", "300/01", "200/01", "100/02", "100/01"});
    CHECK(d.bof());                       // skip(-1) on the first record...
    CHECK(key(d) == "100/01");            // ...stays on it
    CHECK_FALSE(d.eof());

    // Multi-record skips.
    REQUIRE(d.top() == r4success);
    CHECK(d.skip(3) == r4success);
    CHECK(key(d) == "300/01");
    CHECK(d.skip(-2) == r4success);
    CHECK(key(d) == "100/02");
    CHECK(d.skip(10) == r4eof);
    CHECK(d.skip(-1) == r4success);       // from eof, back to the last record
    CHECK(key(d) == "400/01");
    CHECK(d.skip(-10) == r4bof);
    CHECK(key(d) == "100/01");
    CHECK(d.skip(0) == r4success);        // re-read in place
    CHECK(key(d) == "100/01");
}

TEST_CASE("Record order, other tags, and SetIndex keeping the record", "[data4][tags]")
{
    Loaded fx;
    Data4& d = fx.d;
    CHECK(walk(d) == V{"300/01", "100/02", "200/01", "100/01", "400/01"});   // insertion order

    REQUIRE(d.SetIndex("DESCRIP") == r4success);                              // case-insensitive
    CHECK(walk(d) == V{"100/01", "100/02", "400/01", "200/01", "300/01"});   // Cash, Cash branch, ...

    // Switching tags keeps the current record, then follows the new order.
    REQUIRE(d.SetIndex("accounts") == r4success);
    REQUIRE(d.seek("200") == r4success);
    REQUIRE(d.SetIndex("descrip") == r4success);
    CHECK(key(d) == "200/01");
    REQUIRE(d.skip(1) == r4success);
    CHECK(key(d) == "300/01");            // Payables -> Revenue

    CHECK(d.SetIndex("no_such_tag") == e4tagName);
    CHECK(key(d) == "300/01");
    CHECK(walk(d) == V{"300/01", "100/02", "200/01", "100/01", "400/01"});   // record order
    d.select();
    CHECK(walk(d).size() == 5);
}

TEST_CASE("go() and recNo()", "[data4][nav]")
{
    Loaded fx;
    Data4& d = fx.d;
    REQUIRE(d.SetIndex("accounts") == r4success);
    REQUIRE(d.seek("300") == r4success);
    const long rec = d.recNo();
    REQUIRE(d.top() == r4success);
    CHECK(d.go(rec) == r4success);
    CHECK(key(d) == "300/01");
    CHECK(d.skip(1) == r4success);        // continues in tag order from there
    CHECK(key(d) == "400/01");
    CHECK(d.go(9999) == r4entry);
    CHECK(d.eof());
}

TEST_CASE("seek: exact, partial, after, eof", "[data4][seek]")
{
    Loaded fx;
    Data4& d = fx.d;
    REQUIRE(d.SetIndex("accounts") == r4success);

    CHECK(d.seek("100    02") == r4success);          // full account||dept key
    CHECK(key(d) == "100/02");
    CHECK(d.seek("100") == r4success);                // shorter key: partial match, first hit
    CHECK(key(d) == "100/01");
    CHECK(d.seek("100    ") == r4success);            // the spec 4.3 padding example
    CHECK(key(d) == "100/01");
    CHECK(d.seek("150") == r4after);                  // not found: on the next key
    CHECK(key(d) == "200/01");
    CHECK(d.seek("999") == r4eof);
    CHECK(d.eof());
    CHECK(d.seek("") == r4success);                   // empty key matches the first record
    CHECK(key(d) == "100/01");

    // In record order, seek uses the first (default) tag, then skip follows
    // record order again.
    d.select();
    CHECK(d.seek("200") == r4success);
    REQUIRE(d.skip(1) == r4success);
    CHECK(key(d) == "100/01");                        // next by _rowid, not by key

    // Numeric tag key.
    REQUIRE(d.SetIndex("balance") == r4success);
    CHECK(d.seek(75.25) == r4success);
    CHECK(key(d) == "100/02");
    CHECK(d.seek(0.0) == r4after);
    CHECK(key(d) == "400/01");                        // 42.00 is the next balance
    CHECK(d.seek(1e9) == r4eof);
}

TEST_CASE("seek on a table without tags is an error", "[data4][seek]")
{
    Fixture fx;
    cb4sqlite::createTable(fx.code, "plain", {{"A", r4str, 3, 0}});
    Data4 d(fx.code, "plain");
    CHECK_THROWS_AS(d.seek("x"), cb4sqlite::Error);
}

TEST_CASE("assign + flush, and the automatic write when moving", "[data4][write]")
{
    Loaded fx;
    Data4& d = fx.d;
    REQUIRE(d.SetIndex("accounts") == r4success);

    REQUIRE(d.seek("200") == r4success);
    Field4(d, "descript").assign("Accounts payable");
    CHECK(d.flush() == r4success);
    CHECK(raw(fx.code, "SELECT trim(descript) FROM accts WHERE account='200    '") == "Accounts payable");

    REQUIRE(d.seek("300") == r4success);
    Field4(d, "curr_bal").assignDouble(-750);
    REQUIRE(d.top() == r4success);                    // moving writes the changed record
    CHECK(raw(fx.code, "SELECT curr_bal FROM accts WHERE account='300    '") == "-750.0");

    REQUIRE(d.seek("400") == r4success);
    Field4(d, "dept").assign("09");
    CHECK(d.close() == r4success);                    // so does closing
    CHECK(raw(fx.code, "SELECT dept FROM accts WHERE account='400    '") == "09 ");
}

TEST_CASE("Changing a key field re-sorts the record", "[data4][write]")
{
    Loaded fx;
    Data4& d = fx.d;
    REQUIRE(d.SetIndex("accounts") == r4success);
    REQUIRE(d.top() == r4success);
    Field4(d, "account").assign("999");
    REQUIRE(d.flush() == r4success);
    CHECK(key(d) == "999/01");
    CHECK(d.skip(1) == r4eof);                        // it is now the last key
    CHECK(walk(d) == V{"100/02", "200/01", "300/01", "400/01", "999/01"});
}

TEST_CASE("appendStart keeps the buffer; blank() clears it", "[data4][append]")
{
    Loaded fx;
    Data4& d = fx.d;
    REQUIRE(d.SetIndex("accounts") == r4success);
    REQUIRE(d.seek("200") == r4success);              // "Payables", -120.00

    REQUIRE(d.appendStart() == r4success);
    Field4(d, "account").assign("500");               // only the account changes
    REQUIRE(d.append() == r4success);
    CHECK(key(d) == "500/01");                        // positioned on the new record
    CHECK(std::string(Field4(d, "descript").str()) == "Payables            ");
    CHECK(Field4(d, "curr_bal").num() == Catch::Approx(-120.0));
    CHECK(d.recCount() == 6);

    REQUIRE(d.appendStart() == r4success);
    d.blank();
    Field4(d, "account").assign("600");
    REQUIRE(d.append() == r4success);
    CHECK(std::string(Field4(d, "descript").str()) == "                    ");
    CHECK(raw(fx.code, "SELECT curr_bal FROM accts WHERE account='600    '") == "<null>");

    CHECK(d.appendBlank() == r4success);
    CHECK(d.recCount() == 8);
}

TEST_CASE("Mis-sequenced appends", "[data4][append]")
{
    Loaded fx;
    Data4& d = fx.d;
    CHECK_THROWS_AS(d.append(), cb4sqlite::Error);    // no appendStart()

    REQUIRE(d.appendStart() == r4success);
    Field4(d, "account").assign("700");
    CHECK(d.flush() == r4success);                    // nothing is written before append()
    CHECK(d.recCount() == 5);
    REQUIRE(d.top() == r4success);                    // moving abandons the pending append
    CHECK_THROWS_AS(d.append(), cb4sqlite::Error);
    CHECK(d.recCount() == 5);
    // ...without writing the abandoned values over the record we were on.
    CHECK(raw(fx.code, "SELECT count(*) FROM accts WHERE account = '700    '") == "0");
    CHECK(raw(fx.code, "SELECT count(*) FROM accts WHERE account = '400    '") == "1");
}

TEST_CASE("deleteRec / recall / pack", "[data4][delete]")
{
    Loaded fx;
    Data4& d = fx.d;
    REQUIRE(d.SetIndex("accounts") == r4success);

    REQUIRE(d.seek("200") == r4success);
    d.deleteRec();
    CHECK(d.deleted());
    CHECK(raw(fx.code, "SELECT _deleted FROM accts WHERE account='200    '") == "1");
    CHECK(d.recCount() == 5);                         // still counted, as in CodeBase
    CHECK(walk(d).size() == 5);                       // and still visible to navigation

    REQUIRE(d.seek("300") == r4success);
    d.deleteRec();
    d.recall();
    CHECK_FALSE(d.deleted());

    REQUIRE(d.seek("200") == r4success);
    CHECK(d.deleted());                               // flag is read back with the record
    CHECK(d.pack() == r4success);
    CHECK(d.recCount() == 4);
    CHECK(key(d) == "100/01");                        // pack leaves you on top()
    CHECK(walk(d) == V{"100/01", "100/02", "300/01", "400/01"});
}

TEST_CASE("zap", "[data4][delete]")
{
    Loaded fx;
    Data4& d = fx.d;
    CHECK_THROWS_AS(d.zap(2, 3), cb4sqlite::Error);   // ranges not in spec #1
    CHECK(d.zap() == r4success);
    CHECK(d.recCount() == 0);
    CHECK(d.eof());
    CHECK(d.top() == r4eof);
}

TEST_CASE("Two cursors on one table see each other's writes", "[data4]")
{
    Loaded fx;
    Data4 other(fx.code, "accts");
    REQUIRE(fx.d.SetIndex("accounts") == r4success);
    REQUIRE(other.SetIndex("accounts") == r4success);
    REQUIRE(fx.d.seek("100    02") == r4success);
    Field4(fx.d, "descript").assign("Petty cash");
    REQUIRE(fx.d.flush() == r4success);
    REQUIRE(other.seek("100    02") == r4success);
    CHECK(std::string(Field4(other, "descript").str()) == "Petty cash          ");
}
