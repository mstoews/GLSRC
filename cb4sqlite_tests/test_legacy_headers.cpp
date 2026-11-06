//
// Layer-1 tests: the hand-written 1994 schema headers, unchanged, against the
// shim (plan risk P1: every Field4 those headers open by name or number must
// exist on a real table with that layout).
//
// Field widths are representative; the real ones come from the .dbf files in
// M2. Column names are exactly what the headers ask for.
//

#include "TestDb.h"

#include "legacy_headers_begin.h"
#include "db_accts.h"   // include/db_accts.h    - Tglaccts
#include "db_acces.h"   // gl/header/db_acces.h  - TAccess, TAccessl
#include "db_dept.h"    // include/db_dept.h     - Tgldept
#include "legacy_headers_end.h"

#include <cstring>
#include <string>
#include <vector>

using namespace test;

namespace {

void createGlaccts(Code4& code)
{
    std::vector<cb4sqlite::FieldDef> f = {
        {"ACCOUNT", r4str, 7, 0}, {"DEPT", r4str, 4, 0}, {"DESCRIPT", r4str, 30, 0},
        {"FDESCRIPT", r4str, 30, 0}, {"TYPE", r4str, 1, 0}, {"CURR_BAL", r4num, 14, 2},
        {"CATEGORY", r4str, 4, 0},
    };
    for (int i = 1; i <= 12; ++i) f.push_back({"BUDGET" + std::to_string(i), r4num, 14, 2});
    for (int i = 1; i <= 12; ++i) f.push_back({"PERIOD" + std::to_string(i), r4num, 14, 2});
    f.push_back({"OPENBAL", r4num, 14, 2});
    for (int i = 1; i <= 12; ++i) f.push_back({"PREVIOUS" + std::to_string(i), r4num, 14, 2});
    cb4sqlite::createTable(code, "GLACCTS", f);
    cb4sqlite::createTag(code, "glaccts", "accounts", "account || dept");
}

// The 44 Field4* members of Tglaccts, in declaration order.
std::vector<Field4*> fieldsOf(Tglaccts& g)
{
    return {
        g.accounts, g.dept, g.descript, g.fdescript, g.type, g.curr_bal, g.category,
        g.budget1, g.budget2, g.budget3, g.budget4, g.budget5, g.budget6,
        g.budget7, g.budget8, g.budget9, g.budget10, g.budget11, g.budget12,
        g.period1, g.period2, g.period3, g.period4, g.period5, g.period6,
        g.period7, g.period8, g.period9, g.period10, g.period11, g.period12, g.openbal,
        g.previous1, g.previous2, g.previous3, g.previous4, g.previous5, g.previous6,
        g.previous7, g.previous8, g.previous9, g.previous10, g.previous11, g.previous12,
    };
}

// The legacy headers `new` their Field4s and never delete them.
void deleteFields(const std::vector<Field4*>& fs)
{
    for (Field4* f : fs) delete f;
}

} // namespace

TEST_CASE("Tglaccts (db_accts.h) works unchanged on the shim", "[legacy]")
{
    TempDb db;
    Code4 code;
    db.open(code);
    createGlaccts(code);

    // Empty table: the header's ctor still selects the tag and tops.
    Tglaccts* gl = new Tglaccts(&code);
    REQUIRE(gl->isValid());
    const std::vector<Field4*> fields = fieldsOf(*gl);
    REQUIRE(fields.size() == 44);
    for (size_t i = 0; i < fields.size(); ++i) {
        INFO("Tglaccts field #" << i + 1);
        CHECK(fields[i]->isValid());
    }

    // Add an account the way gl_accts.cpp TGLAcctsAdd does.
    REQUIRE(gl->appendStart() == r4success);
    Field4 field;
    for (int fieldNum = 1; fieldNum <= gl->numFields(); fieldNum++) {
        field.init(*(Data4*)gl, fieldNum);
        if (field.type() == r4num) field.assignDouble(0);
    }
    gl->accounts->assign("1000");
    gl->dept->assign("01");
    gl->descript->assign("Cash on hand");
    gl->type->assign("A");
    gl->openbal->assignDouble(1500.75);
    gl->period3->assignDouble(-20.5);
    gl->budget12->assignDouble(99);
    REQUIRE(gl->append() == r4success);

    CHECK(gl->Open_bal() == Catch::Approx(1500.75));
    CHECK(gl->Curr_bal() == 0.0);
    CHECK(gl->dPeriod3() == Catch::Approx(-20.5));
    CHECK(gl->dBudget12() == Catch::Approx(99.0));
    CHECK(gl->dPrevious12() == 0.0);

    deleteFields(fields);
    delete gl;

    // A second instance reads it back by seek on the "accounts" tag.
    Tglaccts again(&code);
    CHECK(again.seek("1000   01") == r4success);
    CHECK(std::string(again.descript->str()).rfind("Cash on hand", 0) == 0);
    deleteFields(fieldsOf(again));
}

TEST_CASE("Tglaccts on a missing table is invalid and opens no fields", "[legacy]")
{
    TempDb db;
    Code4 code;
    db.open(code);
    Tglaccts gl(&code);              // the header guards with isValid()
    CHECK_FALSE(gl.isValid());
}

TEST_CASE("TAccess / TAccessl (db_acces.h): 1-based fields, login lookup", "[legacy]")
{
    TempDb db;
    Code4 code;
    db.open(code);
    cb4sqlite::createTable(code, "ACCESS.PSW", {
        {"USERNAME", r4str, 10, 0}, {"DESCRIPT", r4str, 30, 0}, {"DATE", r4date, 8, 0},
        {"PASSWORD", r4str, 10, 0}, {"LEVEL", r4str, 2, 0},
    });
    cb4sqlite::createTag(code, "access.psw", "u", "username");
    cb4sqlite::createTable(code, "ACCESSL", {{"LEVEL", r4str, 2, 0}, {"MENUCODE", r4str, 6, 0}});
    cb4sqlite::createTag(code, "accessl", "l", "level");

    TAccess* access = new TAccess(&code);
    REQUIRE(access->isValid());
    REQUIRE(access->appendStart() == r4success);
    access->blank();
    access->fUsername->assign("MSTOEWS");
    access->fPassword->assign("secret");
    access->fLevel->assign("9");
    REQUIRE(access->append() == r4success);

    // gl_acces.cpp: seek the user, trim the password, compare.
    CHECK(access->seek("MSTOEWS") == r4success);
    access->fPassword->trim();
    CHECK(std::strcmp(access->fPassword->str(), "secret") == 0);
    CHECK(access->seek("NOBODY") != r4success);

    TAccessl* accessl = new TAccessl(&code);
    REQUIRE(accessl->isValid());
    CHECK(accessl->fMenuCode->isValid());
    CHECK(accessl->seek("9") == r4eof);

    deleteFields({access->fUsername, access->fDescript, access->fDate, access->fPassword, access->fLevel});
    deleteFields({accessl->fLevel, accessl->fMenuCode});
    delete access;
    delete accessl;
}

TEST_CASE("Tgldept (db_dept.h) used by the Chart of Accounts", "[legacy]")
{
    TempDb db;
    Code4 code;
    db.open(code);
    cb4sqlite::createTable(code, "GLDEPT", {{"DEPART", r4str, 4, 0}, {"DESCRIPT", r4str, 30, 0}});
    cb4sqlite::createTag(code, "gldept", "depart", "depart");
    {
        Data4 raw4(code, "gldept");
        REQUIRE(raw4.appendStart() == r4success);
        raw4.blank();
        Field4(raw4, "depart").assign("01");
        Field4(raw4, "descript").assign("Head office");
        REQUIRE(raw4.append() == r4success);
    }
    Tgldept* gldept = new Tgldept(&code);   // as gl_accts.cpp:233
    REQUIRE(gldept->isValid());
    CHECK(gldept->seek("01") == r4success);
    CHECK(std::string(gldept->descript->str()).rfind("Head office", 0) == 0);
    deleteFields({gldept->dept, gldept->descript});
    delete gldept;                          // its dtor calls close()
}
