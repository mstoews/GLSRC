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
#include "db_dist.h"    // include/db_dist.h    - Tgldist over ar_jsum
#include "db_arbtc.h"   // include/db_arbtc.h   - Tapbtch over ar_btch (Field4memo)
#include "legacy_headers_end.h"

#include "../dbf2sqlite_core/src/Source.h"   // translateKey / translateFilter

// The committed generator output, compiled here: proves generated headers build
// under the same warnings as everything else, and work against migrated data.
#include "../dbf2sqlite_core/test/golden/db_accessm.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
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
    CHECK(tables == std::vector<std::string>{"access.psw", "accessl", "accessm", "ar_btch", "ar_jsum", "gl_prof",
                                             "glaccts", "gldept"});
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
    CHECK(logHas(r, "[skip] ar_jsum"));                          // include/db_dist.h
    CHECK(logHas(r, "[skip] ar_btch"));                          // include/db_arbtc.h
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
    CHECK(logHas(r, "memotab: field NOTES is a memo but MEMOTAB.DBT is missing"));
    CHECK(logHas(r, "desctag: tag NAME (NAME) is descending"));
    CHECK(logHas(r, "filttag: tag ACT (NAME) FOR SOUNDEX(NAME) = 'A000': function SOUNDEX() is not supported"));
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
    CHECK(logHas(r, "[warn] memotab: field NOTES is a memo but MEMOTAB.DBT is missing"));
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

namespace {

std::vector<double> numbers(Data4& d, const char* field)
{
    std::vector<double> out;
    for (int rc = d.top(); rc == r4success; rc = d.skip(1)) out.push_back(Field4(d, field).num());
    return out;
}

} // namespace

TEST_CASE("FOR tags and RECNO() keys from ar_jsum, through db_dist.h", "[dbf2sqlite][legacy][filter]")
{
    // Rows: 1 AR batch 5, 2 AP 3, 3 AR 2, 4 GL 1, 5 AR 7 (deleted), 6 blank type 4.
    Migrated m;
    CHECK(raw(m.code, "SELECT filter FROM _cb_tags WHERE table_name = 'ar_jsum' AND tag_name = 'ardist'") ==
          "(substr(\"TYPE\", 1, length('AR ')) = 'AR ')");
    CHECK(raw(m.code, "SELECT key_expr FROM _cb_tags WHERE table_name = 'ar_jsum' AND tag_name = 'recno'") == "_rowid");
    CHECK_THAT(raw(m.code, "SELECT sql FROM sqlite_master WHERE name = 'ar_jsum__ardist'"), ContainsSubstring("WHERE"));

    Tgldist dist(&m.code);
    REQUIRE(dist.isValid());
    REQUIRE(dist.SetIndex("ardist") == r4success);
    CHECK(numbers(dist, "batchno") == std::vector<double>{2, 5, 7});    // deleted records stay, as in CodeBase
    CHECK(dist.seek(5.0) == r4success);
    CHECK(dist.seek(3.0) == r4after);          // batch 3 is AP: not in ARDIST
    CHECK(dist.batchno->num() == 5.0);
    CHECK(dist.seek(8.0) == r4eof);
    REQUIRE(dist.bottom() == r4success);
    CHECK(dist.batchno->num() == 7.0);

    REQUIRE(dist.SetIndex("apdist") == r4success);
    CHECK(numbers(dist, "batchno") == std::vector<double>{3});

    REQUIRE(dist.SetIndex("recno") == r4success);
    CHECK(numbers(dist, "batchno") == std::vector<double>{5, 3, 2, 1, 7, 4});   // record order
    CHECK(dist.seek(4.0) == r4success);
    CHECK(dist.recNo() == 4);

    REQUIRE(dist.SetIndex("livear") == r4success);     // TYPE='AR' .AND. .NOT. DELETED()
    CHECK(keys(dist, "account", "dept") == std::vector<std::string>{"1000   /01  ", "1000   /02  "});

    REQUIRE(dist.SetIndex("account") == r4success);    // the header's own tag: every record
    CHECK(dist.recCount() == 6);
    CHECK(keys(dist, "account").size() == 6);

    for (Field4* f : {dist.account, dist.dept, dist.mode, dist.type, dist.period, dist.date, dist.batchno,
                      dist.reference, dist.cheque, dist.descript, dist.debit, dist.credit})
        delete f;
}

TEST_CASE("dBase IV memos from ar_btch, through db_arbtc.h's Field4memo", "[dbf2sqlite][legacy][memo]")
{
    Migrated m;
    CHECK(raw(m.code, "SELECT type || len FROM _cb_fields WHERE table_name = 'ar_btch' AND field_name = 'MEMOFIELD'") ==
          "M10");
    std::string longMemo;
    for (int i = 0; i < 20; ++i) {
        char line[80];
        std::snprintf(line, sizeof line, "Line %03d of a memo longer than one 512-byte block.\r\n", i);
        longMemo += line;
    }
    REQUIRE(longMemo.size() > 512);

    Tapbtch* b = new Tapbtch(&m.code);
    REQUIRE(b->isValid());
    REQUIRE(b->seek(1.0) == r4success);
    CHECK(std::string(b->memo->str()) == "Net 30 days.\r\nCaf\xC3\xA9 on the corner");   // cp1252 -> UTF-8
    CHECK(b->memo->len() == std::strlen(b->memo->str()));
    CHECK(b->lBatchno() == 1);
    CHECK(b->dPayable() == Catch::Approx(250.0));
    REQUIRE(b->seek(2.0) == r4success);
    CHECK(std::string(b->memo->str()).empty());                        // blank pointer = no memo
    CHECK(b->memo->len() == 0);
    REQUIRE(b->seek(3.0) == r4success);
    CHECK(std::string(b->memo->str()) == longMemo);                    // spans two blocks

    // Written back like any field (ar_btche.cpp: arbtch->memo->assign(...)).
    REQUIRE(b->seek(2.0) == r4success);
    b->memo->assign("Call before delivery");
    REQUIRE(b->seek(1.0) == r4success);
    REQUIRE(b->seek(2.0) == r4success);
    CHECK(std::string(b->memo->str()) == "Call before delivery");

    for (Field4* f : {b->batchno, b->mode, b->period, b->vendorno, b->invoiceno, b->orderno, b->duedate,
                      b->invdate, b->terms, b->vatpercent, b->date, b->reference, b->account, b->depart,
                      b->descript, b->payable, b->vatamt, b->entryno, b->vin, b->orderdate, b->units})
        delete f;
    delete b->memo;
    delete b;
}

TEST_CASE("FOR conditions translate with dBase semantics", "[dbf2sqlite][keys][filter]")
{
    using dbf2sqlite::detail::KeyField;
    TempDb db;
    Code4 code;
    db.open(code);
    cb4sqlite::createTable(code, "t", {{"TYPE", r4str, 3, 0}, {"AMT", r4num, 8, 2}, {"WHEN", r4date, 8, 0},
                                       {"OK", r4log, 1, 0}});
    raw(code, "INSERT INTO t (_rowid, \"TYPE\", AMT, \"WHEN\", OK) VALUES "
              "(1, 'AR ', 10, '20261106', 1), (2, 'AP ', NULL, NULL, 0), (3, 'A  ', -5, '19950101', NULL)");
    const std::vector<KeyField> fields = {{"TYPE", 'C'}, {"AMT", 'N'}, {"WHEN", 'D'}, {"OK", 'L'}};
    auto rows = [&](const std::string& cond) {
        const auto k = dbf2sqlite::detail::translateFilter(cond, fields);
        INFO(cond << " -> " << k.error);
        REQUIRE_FALSE(k.sql.empty());
        return raw(code, "SELECT ifnull(group_concat(_rowid), '') FROM (SELECT _rowid FROM t WHERE " + k.sql +
                         " ORDER BY _rowid)");
    };
    CHECK(rows("type = 'AR '") == "1");
    CHECK(rows("type = 'AR'") == "1");
    CHECK(rows("TYPE='A'") == "1,2,3");                    // SET EXACT OFF: a prefix matches
    CHECK(rows("type == 'A'") == "3");                     // == ignores trailing blanks only
    CHECK(rows("type <> 'AR '") == "2,3");
    CHECK(rows("type # 'AR'") == "2,3");
    CHECK(rows("type != 'AR'") == "2,3");
    CHECK(rows("type = [AP]") == "2");
    CHECK(rows("type = \"AP\"") == "2");
    CHECK(rows("'R' $ type") == "1");
    CHECK(rows("AMT > 0") == "1");
    CHECK(rows("amt = 0") == "2");                         // blank numbers read as 0
    CHECK(rows("amt + 5 >= 0") == "1,2,3");
    CHECK(rows("DTOS(when) < '2000'") == "2,3");           // blank date = 8 spaces
    CHECK(rows("OK") == "1");
    CHECK(rows(".NOT. OK") == "2,3");
    CHECK(rows("!ok") == "2,3");
    CHECK(rows("ok .and. type = 'AR'") == "1");
    CHECK(rows("type = 'AP' .OR. amt < 0") == "2,3");
    CHECK(rows("(type = 'AR' .OR. type = 'AP') .AND. .NOT. OK") == "2");
    CHECK(rows(".NOT. DELETED()") == "1,2,3");
    CHECK(rows("RECNO() > 1") == "2,3");
    CHECK(rows(".T.") == "1,2,3");

    auto error = [&](const std::string& cond) { return dbf2sqlite::detail::translateFilter(cond, fields).error; };
    CHECK(error("type") == "the FOR condition isn't a logical expression");
    CHECK(error("SOUNDEX(type) = 'A000'") == "function SOUNDEX() is not supported");
    CHECK(error("amt = 'x'") == "can't compare N with C");
    CHECK(error("type = 'AR") == "unterminated text literal");
    CHECK(error("x->type = 'A'") == "alias references (X->...) are not supported");
    CHECK(error("nosuch = 'A'") == "no field named NOSUCH");
    CHECK(error("type = 'A' .AND.") == "expected a field name or function at position 17");

    const auto recno = dbf2sqlite::detail::translateKey("RECNO()", fields);
    CHECK(recno.sql == "_rowid");
    CHECK(recno.type == 'N');
    CHECK(dbf2sqlite::detail::translateKey("'A'+TYPE", fields).error ==
          "text literals aren't supported in a key expression");
    CHECK(dbf2sqlite::detail::translateKey("DELETED()", fields).error == "DELETED() can't be part of a key");
}
