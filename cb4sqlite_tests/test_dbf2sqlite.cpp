//
// Layer-2 tests: dbf2sqlite_core migration over the committed fixtures,
// read back through the shim and the unchanged hand-written headers
// (plan M2 tasks 5-6). Fixtures: dbf2sqlite_core/test/make_fixtures.py.
//

#include "TestDb.h"
#include "Migrator.h"

#include "legacy_headers_begin.h"
#include "db_accts.h"
#include "db_acces.h"
#include "db_dept.h"
#include "legacy_headers_end.h"

// The committed generator output, compiled here: proves generated headers build
// under the same warnings as everything else, and work against migrated data.
#include "../dbf2sqlite_core/test/golden/db_accessm.h"

#include <algorithm>
#include <cstdlib>
#include <fstream>
#include <sstream>

using namespace test;
using Catch::Matchers::ContainsSubstring;
namespace fs = std::filesystem;

namespace {

// The repo root: found by walking up from the working directory (VS runs tests
// from the project dir; the command line usually from the root or x64\Debug).
fs::path repoRoot()
{
    if (const char* env = std::getenv("GL_REPO_ROOT"))
        return env;
    for (fs::path p = fs::current_path(); !p.empty(); p = p.parent_path()) {
        if (fs::exists(p / "dbf2sqlite_core" / "test" / "fixtures"))
            return p;
        if (p == p.parent_path()) break;
    }
    FAIL("cannot find the repo root (set GL_REPO_ROOT)");
    return {};
}

fs::path fixture(const char* name) { return repoRoot() / "dbf2sqlite_core" / "test" / "fixtures" / name; }

bool logHas(const dbf2sqlite::Result& r, const std::string& text)
{
    return std::any_of(r.log.begin(), r.log.end(), [&](const std::string& l) { return l.find(text) != std::string::npos; });
}

std::string logText(const dbf2sqlite::Result& r)
{
    std::string s;
    for (const std::string& l : r.log) s += l + "\n";
    return s;
}

std::string readFile(const fs::path& p)
{
    std::ifstream in(p, std::ios::binary);
    std::stringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

// A migrated copy of fixtures/company in a temp file.
struct Migrated {
    TempDb db;
    dbf2sqlite::Result result;
    Code4 code;
    Migrated()
    {
        std::filesystem::remove(db.path());
        dbf2sqlite::Options opt;
        opt.srcDir = fixture("company");
        opt.outPath = db.path();
        result = dbf2sqlite::migrate(opt);
        INFO(logText(result));
        REQUIRE(result.code == dbf2sqlite::ExitCode::Ok);
        REQUIRE(code.open(db.path().c_str()) == r4success);
    }
};

std::vector<std::string> keys(Data4& d, const char* f1, const char* f2 = nullptr)
{
    std::vector<std::string> out;
    for (int rc = d.top(); rc == r4success; rc = d.skip(1)) {
        std::string k = Field4(d, f1).str();
        if (f2) k += std::string("/") + Field4(d, f2).str();
        out.push_back(k);
    }
    return out;
}

} // namespace

TEST_CASE("Migrating the sample company: every table, in order, verified", "[dbf2sqlite]")
{
    Migrated m;
    std::vector<std::string> tables;
    for (const auto& t : m.result.tables) tables.push_back(t.table);
    CHECK(tables == std::vector<std::string>{"access.psw", "accessl", "accessm", "gl_prof", "glaccts", "gldept"});
    CHECK(logHas(m.result, "ACCESS.PSW: dBase table without a .dbf extension"));
    CHECK_FALSE(logHas(m.result, "README"));
    CHECK_FALSE(fs::exists(m.db.path() + ".tmp"));

    CHECK(raw(m.code, "SELECT group_concat(tag_name || '=' || key_expr, '; ') FROM "
                      "(SELECT * FROM _cb_tags WHERE table_name = 'glaccts' ORDER BY rowid)") ==
          "account=\"ACCOUNT\" || \"DEPT\"; accounts=\"ACCOUNT\" || \"DEPT\"; dept=\"DEPT\"; "
          "type=\"TYPE\"; descrip=upper(\"DESCRIPT\")");
    CHECK(raw(m.code, "SELECT count(*) FROM sqlite_master WHERE type = 'index' AND name LIKE 'glaccts__%'") == "5");
}

TEST_CASE("Migrated glaccts through the unchanged db_accts.h", "[dbf2sqlite][legacy]")
{
    Migrated m;
    Tglaccts gl(&m.code);
    REQUIRE(gl.isValid());

    CHECK(gl.recCount() == 5);                                   // the deleted record is kept
    REQUIRE(gl.seek("1000   01  ") == r4success);
    CHECK(std::string(gl.descript->str()).rfind("Caisse g\xC3\xA9n\xC3\xA9rale", 0) == 0);   // cp1252 -> UTF-8
    CHECK(gl.Open_bal() == Catch::Approx(1500.75));
    CHECK(gl.dPeriod3() == Catch::Approx(-20.5));
    CHECK(gl.dBudget1() == 0.0);                                 // blank numeric
    CHECK(std::string(gl.budget1->str()) == std::string(14, ' '));
    CHECK(gl.recNo() == 2);                                      // _rowid = dBase record number

    CHECK(keys(gl, "account", "dept") ==
          std::vector<std::string>{"1000   /01  ", "1000   /02  ", "2000   /01  ", "3000   /01  ", "9999   /99  "});

    REQUIRE(gl.seek("9999") == r4success);
    CHECK(gl.deleted());
    Relate4set q(gl);
    REQUIRE(q.querySet(".NOT. DELETED()") == r4success);
    int live = 0;
    for (int rc = q.top(); rc != r4eof; rc = q.skip(1)) ++live;
    CHECK(live == 4);

    REQUIRE(gl.SetIndex("descrip") == r4success);                // UPPER(DESCRIPT)
    REQUIRE(gl.top() == r4success);
    CHECK(std::string(gl.descript->str()).rfind("Accounts payable", 0) == 0);

    Field4 updated(gl, "update");
    CHECK(std::string(updated.str()) == "20261106");

    for (Field4* f : {gl.accounts, gl.dept, gl.descript, gl.fdescript, gl.type, gl.curr_bal, gl.category,
                      gl.openbal, gl.budget1, gl.budget2, gl.budget3, gl.budget4, gl.budget5, gl.budget6,
                      gl.budget7, gl.budget8, gl.budget9, gl.budget10, gl.budget11, gl.budget12, gl.period1,
                      gl.period2, gl.period3, gl.period4, gl.period5, gl.period6, gl.period7, gl.period8,
                      gl.period9, gl.period10, gl.period11, gl.period12, gl.previous1, gl.previous2,
                      gl.previous3, gl.previous4, gl.previous5, gl.previous6, gl.previous7, gl.previous8,
                      gl.previous9, gl.previous10, gl.previous11, gl.previous12}) {
        CHECK(f->isValid());
        delete f;
    }
}

TEST_CASE("Migrated access.psw, accessl and gldept through their headers", "[dbf2sqlite][legacy]")
{
    Migrated m;
    TAccess* access = new TAccess(&m.code);
    REQUIRE(access->isValid());
    REQUIRE(access->seek("MSTOEWS") == r4success);
    access->fPassword->trim();
    CHECK(std::string(access->fPassword->str()) == "secret");
    CHECK(std::string(access->fDate->str()) == "19950301");
    REQUIRE(access->seek("CLERK") == r4success);
    CHECK(std::string(access->fDate->str()) == "        ");       // blank date
    for (Field4* f : {access->fUsername, access->fDescript, access->fDate, access->fPassword, access->fLevel}) delete f;
    delete access;

    TAccessl* accessl = new TAccessl(&m.code);
    CHECK(accessl->seek("9") == r4success);
    CHECK(std::string(accessl->fMenuCode->str()) == "ALL   ");
    delete accessl->fLevel;
    delete accessl->fMenuCode;
    delete accessl;

    Tgldept* dept = new Tgldept(&m.code);
    CHECK(keys(*dept, "depart") == std::vector<std::string>{"01  ", "02  ", "99  "});
    REQUIRE(dept->seek("01") == r4success);
    CHECK(std::string(Field4(*dept, "active").str()) == "T");
    REQUIRE(dept->seek("99") == r4success);
    CHECK(std::string(Field4(*dept, "active").str()) == " ");    // '?' = unknown -> blank
    delete dept->dept;
    delete dept->descript;
    delete dept;
}

TEST_CASE("Generated header: golden output, compiles, and works", "[dbf2sqlite][headers]")
{
    Migrated m;
    TempDb scratch;                                              // only for a unique dir name
    const fs::path out = fs::path(scratch.path()).replace_extension(".hdr");
    dbf2sqlite::Options opt;
    opt.outPath = m.db.path();
    opt.headerOutDir = out;
    const fs::path root = repoRoot();
    opt.headerCheckDirs = {root / "include", root / "gl" / "header", root / "common"};

    dbf2sqlite::Result r = dbf2sqlite::generateHeaders(opt);
    INFO(logText(r));
    REQUIRE(r.code == dbf2sqlite::ExitCode::Ok);
    CHECK(logHas(r, "[skip] glaccts: already opened by"));
    CHECK(logHas(r, "db_accts.h"));
    CHECK(logHas(r, "[skip] access.psw: already opened by"));
    CHECK(logHas(r, "db_acces.h"));
    CHECK(logHas(r, "[skip] gldept"));
    CHECK(logHas(r, "[skip] gl_prof"));                          // common/gl_prof.cpp: Data4::open(cb, "GL_PROF")
    REQUIRE(fs::exists(out / "db_accessm.h"));
    CHECK(fs::directory_iterator(out) != fs::directory_iterator());
    CHECK(std::distance(fs::directory_iterator(out), fs::directory_iterator()) == 1);

    // Byte-for-byte equal to the committed golden file (deterministic output).
    CHECK(readFile(out / "db_accessm.h") == readFile(root / "dbf2sqlite_core" / "test" / "golden" / "db_accessm.h"));

    // Re-running changes nothing: the new header now covers accessm itself.
    dbf2sqlite::Result again = dbf2sqlite::generateHeaders(opt);
    CHECK(again.code == dbf2sqlite::ExitCode::Ok);
    CHECK(logHas(again, "[skip] accessm: already opened by"));
    fs::remove_all(out);

    // The golden header (included at the top of this file) against real data.
    Taccessm menu(&m.code);
    REQUIRE(menu.isValid());
    CHECK(keys(menu, "count") == std::vector<std::string>{"    1", "    2", "   10"});   // STR(COUNT,5) order
    REQUIRE(menu.seek("    2") == r4success);
    CHECK(std::string(menu.text->str()).rfind("&Open company...", 0) == 0);
    CHECK(menu.id->num() == 1001.0);
}

TEST_CASE("Untranslatable tables fail with exit 4 and write nothing", "[dbf2sqlite][errors]")
{
    TempDb db;
    fs::remove(db.path());
    dbf2sqlite::Options opt;
    opt.srcDir = fixture("bad");
    opt.outPath = db.path();
    dbf2sqlite::Result r = dbf2sqlite::migrate(opt);
    INFO(logText(r));
    CHECK(r.code == dbf2sqlite::ExitCode::Translate);
    CHECK(r.exitCode() == 4);
    CHECK(logHas(r, "memotab: field NOTES has type 'M' (memo"));
    CHECK(logHas(r, "desctag: tag NAME (NAME) is descending"));
    CHECK(logHas(r, "filttag: tag ACT (NAME) has a FOR clause (ACTIVE)"));
    CHECK(logHas(r, "dupuniq: tag NAME (NAME) is UNIQUE but 1 key values occur more than once"));
    CHECK(logHas(r, "badexpr: tag PFX (SUBSTR(NAME,1,3)): function SUBSTR() is not supported"));
    CHECK_FALSE(logHas(r, "NOTDBF"));                            // not a dBase file: ignored
    CHECK_FALSE(fs::exists(db.path()));
    CHECK_FALSE(fs::exists(db.path() + ".tmp"));
}

TEST_CASE("--force skips what can't be represented, with warnings", "[dbf2sqlite][errors]")
{
    TempDb db;
    fs::remove(db.path());
    dbf2sqlite::Options opt;
    opt.srcDir = fixture("bad");
    opt.outPath = db.path();
    opt.force = true;
    dbf2sqlite::Result r = dbf2sqlite::migrate(opt);
    INFO(logText(r));
    REQUIRE(r.code == dbf2sqlite::ExitCode::Ok);
    CHECK(logHas(r, "[warn] memotab: field NOTES has type 'M'"));
    CHECK(logHas(r, "field dropped (--force)"));
    CHECK(logHas(r, "[warn] widen.NAME: widened from 4 to 5 bytes"));
    CHECK(logHas(r, "[warn] badvals.AMOUNT record 1: not a number '12a.0' stored as empty"));
    CHECK(logHas(r, "[warn] badvals.WHEN record 1: not a YYYYMMDD date '2026XX01' stored as empty"));

    Code4 code;
    REQUIRE(code.open(db.path().c_str()) == r4success);
    CHECK(raw(code, "SELECT group_concat(field_name) FROM _cb_fields WHERE table_name = 'memotab'") == "ID");
    CHECK(raw(code, "SELECT count(*) FROM _cb_tags WHERE table_name IN ('desctag','filttag','badexpr')") == "0");
    CHECK(raw(code, "SELECT count(*) FROM _cb_tags WHERE table_name = 'dupuniq'") == "1");   // as non-unique
    CHECK(raw(code, "SELECT len FROM _cb_fields WHERE table_name = 'widen'") == "5");
    CHECK(raw(code, "SELECT name FROM widen WHERE _rowid = 1") == "Caf\xC3\xA9");
    CHECK(raw(code, "SELECT '[' || name || ']' FROM widen WHERE _rowid = 2") == "[Tea  ]");
    CHECK(raw(code, "SELECT amount FROM badvals WHERE _rowid = 1") == "<null>");
    CHECK(raw(code, "SELECT amount || ' ' || \"WHEN\" FROM badvals WHERE _rowid = 2") == "1.5 20261106");
}

TEST_CASE("Argument, source and existing-output errors", "[dbf2sqlite][errors]")
{
    TempDb db;
    dbf2sqlite::Options opt;
    CHECK(dbf2sqlite::migrate(opt).code == dbf2sqlite::ExitCode::BadArgs);

    // --out exists: refused without --force, replaced with it.
    { std::ofstream(db.path()) << "old"; }
    opt.srcDir = fixture("company");
    opt.outPath = db.path();
    CHECK(dbf2sqlite::migrate(opt).code == dbf2sqlite::ExitCode::BadArgs);
    CHECK(readFile(db.path()) == "old");
    opt.force = true;
    CHECK(dbf2sqlite::migrate(opt).code == dbf2sqlite::ExitCode::Ok);
    CHECK(readFile(db.path()).rfind("SQLite format 3", 0) == 0);

    opt.srcDir = fixture("no_such_dir");
    CHECK(dbf2sqlite::migrate(opt).code == dbf2sqlite::ExitCode::SourceRead);

    opt.srcDir = fixture("company");
    opt.tables = {"nosuch"};
    CHECK(dbf2sqlite::migrate(opt).code == dbf2sqlite::ExitCode::SourceRead);

    opt.srcDir = fixture("corrupt_dbf");
    opt.tables = {"broken"};
    CHECK(dbf2sqlite::migrate(opt).code == dbf2sqlite::ExitCode::SourceRead);

    opt.srcDir = fixture("corrupt_mdx");
    opt.tables = {};
    dbf2sqlite::Result r = dbf2sqlite::migrate(opt);
    CHECK(r.code == dbf2sqlite::ExitCode::SourceRead);
    CHECK(logHas(r, "not a dBase IV .mdx layout"));
    CHECK(readFile(db.path()).rfind("SQLite format 3", 0) == 0); // earlier good output untouched
}

TEST_CASE("--table restricts the migration", "[dbf2sqlite]")
{
    TempDb db;
    fs::remove(db.path());
    dbf2sqlite::Options opt;
    opt.srcDir = fixture("company");
    opt.outPath = db.path();
    opt.tables = {"GLACCTS", "access.psw"};
    dbf2sqlite::Result r = dbf2sqlite::migrate(opt);
    INFO(logText(r));
    REQUIRE(r.code == dbf2sqlite::ExitCode::Ok);
    REQUIRE(r.tables.size() == 2);
    CHECK(r.tables[0].table == "access.psw");
    CHECK(r.tables[1].table == "glaccts");
}

TEST_CASE("STR() keys sort like dBase character keys", "[dbf2sqlite][keys]")
{
    // STR(COUNT,5) right-justifies, so as text "    1" < "    2" < "   10":
    // numeric order, as in the original index.
    Migrated m;
    CHECK(raw(m.code, "SELECT group_concat(c, ',') FROM (SELECT count AS c FROM accessm ORDER BY "
                      "printf('%5.0f', ifnull(\"COUNT\", 0)))") == "1,2,10");
}
