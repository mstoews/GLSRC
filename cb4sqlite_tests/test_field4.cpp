//
// Layer-1 tests: Field4 accessors and the dBase text images in the record buffer.
//

#include "TestDb.h"

#include <cstring>

using namespace test;

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

} // namespace

TEST_CASE("Field4 metadata and every read accessor", "[field4]")
{
    Fixture fx;
    Data4 d(fx.code, "accts");
    addAccount(d, "100", "01", "Cash", 250.5);

    Field4 account(d, "account"), bal(d, "CURR_BAL"), count(d, 5), opened(d, "opened"), active(d, "active");
    REQUIRE(account.isValid());
    CHECK(std::string(account.str()) == "100    ");          // right-padded to width 7
    CHECK(std::string(account.ptr()) == "100    ");
    CHECK(account.len() == 7);
    CHECK(account.type() == r4str);
    CHECK(std::string(account.name()) == "ACCOUNT");
    CHECK(account.number() == 1);

    CHECK(std::string(bal.str()) == "      250.50");          // right-justified, 2 decimals
    CHECK(bal.num() == Catch::Approx(250.5));
    CHECK(bal.len() == 12);
    CHECK(bal.decimals() == 2);
    CHECK(bal.type() == r4num);

    CHECK(std::string(count.name()) == "COUNT");
    CHECK(std::string(count.str()) == "     ");               // empty numeric: blanks
    CHECK(count.num() == 0.0);
    CHECK(std::string(opened.str()) == "        ");           // empty date
    CHECK(opened.len() == 8);
    CHECK(std::string(active.str()) == " ");                  // empty logical

    Field4 byName(d, "descript"), byNumber(d, 3);
    CHECK(std::strcmp(byName.str(), byNumber.str()) == 0);

    Field4 none(d, "no_such_field");
    CHECK_FALSE(none.isValid());
    CHECK(Field4(d, 0).isValid() == false);
    CHECK(Field4(d, 8).isValid() == false);

    Field4 late;                                              // default + init, as gl_accts.cpp
    CHECK_FALSE(late.isValid());
    CHECK(late.init(d, 4) == 0);
    CHECK(late.type() == r4num);
    CHECK(late.init(d, "nope") == -1);
    CHECK_FALSE(late.isValid());
}

TEST_CASE("Character fields are padded on write and truncated UTF-8 safely", "[field4]")
{
    Fixture fx;
    Data4 d(fx.code, "accts");
    addAccount(d, "100", "01", "Cash", 0);

    // Stored right-padded in SQLite (spec 4.3), so concatenated keys match dBase.
    CHECK(raw(fx.code, "SELECT '[' || account || ']' FROM accts") == "[100    ]");

    Field4 dept(d, "dept");
    dept.assign("ABCDEF");                                    // too long: cut to width 3
    CHECK(std::string(dept.str()) == "ABC");
    dept.assign("\xC3\xA9\xC3\xA9");                          // "éé" = 4 bytes, width 3
    CHECK(std::string(dept.str()) == "\xC3\xA9 ");             // "é" + 1 space: no half character
    dept.assign(nullptr);
    CHECK(std::string(dept.str()) == "   ");
}

TEST_CASE("Numeric, date and logical images", "[field4]")
{
    Fixture fx;
    Data4 d(fx.code, "accts");
    addAccount(d, "100", "01", "Cash", 0);
    Field4 bal(d, "curr_bal"), count(d, "count"), opened(d, "opened"), active(d, "active"), account(d, "account");

    bal.assignDouble(-1234.567);
    CHECK(std::string(bal.str()) == "    -1234.57");
    bal.assignDouble(1.0, -1, 0);                             // explicit decimals
    CHECK(std::string(bal.str()) == "           1");
    bal.assignDouble(1e15);                                   // does not fit in 12
    CHECK(std::string(bal.str()) == "************");
    bal.assign("  42.5 ");
    CHECK(std::string(bal.str()) == "       42.50");
    bal.assign("abc");                                        // not a number: blank
    CHECK(std::string(bal.str()) == "            ");

    count.assignLong(42);
    CHECK(std::string(count.str()) == "   42");
    account.assignLong(7, -1, 1);                             // char field, zeros in front
    CHECK(std::string(account.str()) == "0000007");
    account.assignLong(7);
    CHECK(std::string(account.str()) == "      7");

    opened.assign("20261106");
    active.assign("y");
    CHECK(std::string(active.str()) == "T");
    REQUIRE(d.flush() == r4success);
    CHECK(raw(fx.code, "SELECT opened FROM accts") == "20261106");
    CHECK(raw(fx.code, "SELECT active FROM accts") == "1");
    CHECK(raw(fx.code, "SELECT typeof(count) || ':' || count FROM accts") == "integer:42");
    CHECK(raw(fx.code, "SELECT curr_bal FROM accts") == "<null>");   // blank numeric -> NULL

    active.assign("F");
    opened.assign("");
    REQUIRE(d.flush() == r4success);
    CHECK(raw(fx.code, "SELECT active FROM accts") == "0");
    CHECK(raw(fx.code, "SELECT opened FROM accts") == "<null>");
}

TEST_CASE("trim() shortens the buffered text only", "[field4]")
{
    Fixture fx;
    Data4 d(fx.code, "accts");
    addAccount(d, "100", "01", "Cash", 0);
    addAccount(d, "200", "01", "Payables", 0);
    REQUIRE(d.SetIndex("accounts") == r4success);
    REQUIRE(d.top() == r4success);

    Field4 descript(d, "descript");
    descript.trim();
    CHECK(std::string(descript.str()) == "Cash");             // as gl_acces.cpp compares passwords
    Field4(d, "dept").assign("02");                           // another change: forces a write
    REQUIRE(d.flush() == r4success);
    CHECK(raw(fx.code, "SELECT '[' || descript || ']' FROM accts WHERE account='100    '") ==
          "[Cash                ]");                          // still stored padded
    REQUIRE(d.skip(1) == r4success);
    REQUIRE(d.skip(-1) == r4success);
    CHECK(std::string(descript.str()) == "Cash                ");
}

TEST_CASE("Date4", "[date4]")
{
    Date4 empty;
    CHECK(std::string(empty.str()) == "        ");
    CHECK(empty.year() == 0);

    Date4 d;
    d.today();                                                // gl.cpp: `Date4 d; d.today();`
    REQUIRE(std::strlen(d.str()) == 8);
    CHECK(d.year() >= 2024);
    CHECK(d.month() >= 1);
    CHECK(d.month() <= 12);
    CHECK(d.day() >= 1);
    CHECK(d.day() <= 31);

    Date4 fixed("20261106");
    CHECK(fixed.year() == 2026);
    CHECK(fixed.month() == 11);
    CHECK(fixed.day() == 6);
    CHECK(fixed.len() == 8);

    Fixture fx;
    Data4 t(fx.code, "accts");
    addAccount(t, "100", "01", "Cash", 0);
    Field4(t, "opened").assign(fixed.str());                  // fDate.assign(d.str())
    CHECK(std::string(Field4(t, "opened").str()) == "20261106");
}

TEST_CASE("Memo fields hold the whole text; Field4memo reports its length", "[field4][memo]")
{
    TempDb db;
    Code4 code;
    db.open(code);
    cb4sqlite::createTable(code, "notes", {{"ID", r4str, 4, 0}, {"BODY", r4memo, 10, 0}});
    CHECK(raw(code, "SELECT type || len FROM _cb_fields WHERE field_name = 'BODY'") == "M10");
    Data4 d(&code, "notes");
    REQUIRE(d.appendStart() == r4success);
    d.blank();
    Field4(d, "id").assign("A1");
    Field4memo body(d, "body");
    REQUIRE(body.isValid());
    CHECK(body.type() == r4memo);
    CHECK(std::string(body.str()).empty());
    CHECK(body.len() == 0);

    const std::string text = std::string(2000, 'x') + "\r\nCaf\xC3\xA9";
    REQUIRE(body.assign(text.c_str()) == r4success);
    REQUIRE(d.append() == r4success);
    CHECK(raw(code, "SELECT length(CAST(body AS BLOB)) FROM notes") == "2007");   // never truncated

    REQUIRE(d.top() == r4success);
    CHECK(std::string(body.str()) == text);
    CHECK(body.len() == text.size());
    CHECK(Field4(d, "body").len() == 10u);          // a plain Field4 reports the dBase width
    CHECK(Field4memo(d, "id").len() == 4u);         // on a non-memo field: the field width

    body.assign("");
    d.flush();
    CHECK(raw(code, "SELECT typeof(body) FROM notes") == "null");   // empty memo = no memo
    body.free();
}
