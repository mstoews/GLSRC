//
// gl_tests: the master-data record operations (gl/port/MasterTables.cpp,
// plan S2-M2) on the dev fixture: three medical reps (MR001-MR003), three
// doctors (DR001-DR003), three products (P100, P200, P300) and departments
// 10 and 20.
//

#include "GlTestDb.h"

#include "Accounts.h"   // TrimText
#include "MasterTables.h"

#include <string>
#include <vector>

using namespace gltest;

namespace {

template <class Table>
std::vector<std::string> codesInOrder(Table& t, const Field4& code)
{
    std::vector<std::string> out;
    for (t.top(); !t.eof(); t.skip()) {
        std::string s = code.str();
        s.erase(s.find_last_not_of(' ') + 1);
        out.push_back(s);
    }
    return out;
}

RepInput newRep(const std::string& code)
{
    RepInput in;
    in.code = code;
    in.shortName = "NEWREP";
    in.name = "New Representative";
    in.address1 = "2 Harbour St";
    in.address3 = "Vancouver BC";
    in.postal = "V6C2B2";
    in.phone = "(604) 555 - 0100";
    in.fax = "(604) 555 - 0101";
    in.contact = "Front desk";
    in.territory = "WEST";
    in.terms = "45";
    in.creditLimit = "12,500.00";   // as a tbFloat shows it
    return in;
}

} // namespace

TEST_CASE("RepTable opens every field of gl_medrp and gl_drcod, in code order", "[master]")
{
    FixtureDb db;
    for (RepTable::Kind kind : {RepTable::Kind::MedicalRep, RepTable::Kind::Doctor}) {
        RepTable t(db.code, kind);
        REQUIRE(t.isValid());
        for (const Field4* f : {&t.code, &t.rpshort, &t.rpname, &t.address1, &t.address2, &t.address3, &t.postal,
                                &t.phone, &t.territory, &t.fax, &t.descript, &t.contact, &t.type, &t.ref, &t.status,
                                &t.terms, &t.balance, &t.creditlmt})
            CHECK(f->isValid());
        const std::vector<std::string> want = kind == RepTable::Kind::MedicalRep
                                                  ? std::vector<std::string>{"MR001", "MR002", "MR003"}
                                                  : std::vector<std::string>{"DR001", "DR002", "DR003"};
        CHECK(codesInOrder(t, t.code) == want);
    }
}

TEST_CASE("SeekCode matches the whole code, not a prefix", "[master]")
{
    FixtureDb db;
    RepTable t(db.code, RepTable::Kind::MedicalRep);
    CHECK(SeekCode(t, t.code, "MR002"));
    CHECK(ReadRep(t).name == "Jordan Lee");
    CHECK(SeekCode(t, t.code, "  MR003 "));   // as typed, trimmed
    CHECK_FALSE(SeekCode(t, t.code, "MR00"));   // seek() alone would land on MR001
    CHECK_FALSE(SeekCode(t, t.code, "MR004"));
    CHECK_FALSE(SeekCode(t, t.code, ""));
    CHECK_FALSE(SeekCode(t, t.code, "mr001"));  // case-sensitive, as CodeBase
}

TEST_CASE("Medical reps: add, update (confirmed or not), delete", "[master]")
{
    FixtureDb db;
    RepTable t(db.code, RepTable::Kind::MedicalRep);
    const RepInput in = newRep("MR010");
    int asked = 0;

    SECTION("a new code is appended without asking")
    {
        CHECK(SaveRecord(t, t.code, in.code, [&] { AssignRep(t, in); }, [&] { return ++asked, true; }) ==
              SaveResult::Added);
        CHECK(asked == 0);
        CHECK(t.recCount() == 4);
        REQUIRE(SeekCode(t, t.code, "MR010"));
        const RepInput back = ReadRep(t);
        CHECK(back.name == "New Representative");
        CHECK(back.territory == "WEST");     // D10: the original saved territory blank from 1006
        CHECK(back.terms == "45");
        CHECK(t.creditlmt.num() == Catch::Approx(12500.0));
        CHECK(t.balance.num() == 0.0);
        CHECK(codesInOrder(t, t.code) == std::vector<std::string>{"MR001", "MR002", "MR003", "MR010"});
    }
    SECTION("an existing code is updated only when confirmed, and its balance is kept")
    {
        REQUIRE(SeekCode(t, t.code, "MR002"));
        t.balance.assignDouble(350.25);   // as if posted from elsewhere
        t.flush();
        RepInput changed = newRep("MR002");
        CHECK(SaveRecord(t, t.code, changed.code, [&] { AssignRep(t, changed); }, [&] { return ++asked, false; }) ==
              SaveResult::Unchanged);
        REQUIRE(SeekCode(t, t.code, "MR002"));
        CHECK(ReadRep(t).name == "Jordan Lee");
        CHECK(SaveRecord(t, t.code, changed.code, [&] { AssignRep(t, changed); }, [&] { return ++asked, true; }) ==
              SaveResult::Updated);
        CHECK(asked == 2);
        CHECK(t.recCount() == 3);
        REQUIRE(SeekCode(t, t.code, "MR002"));
        CHECK(ReadRep(t).name == "New Representative");
        CHECK(t.balance.num() == Catch::Approx(350.25));   // D10: the original wrote 0
    }
    SECTION("delete removes only the code given")
    {
        CHECK_FALSE(DeleteRecord(t, t.code, "MR009"));
        CHECK(t.recCount() == 3);
        CHECK(DeleteRecord(t, t.code, "MR002"));
        CHECK(codesInOrder(t, t.code) == std::vector<std::string>{"MR001", "MR003"});
    }
}

TEST_CASE("Doctors use the same operations on gl_drcod", "[master]")
{
    FixtureDb db;
    RepTable t(db.code, RepTable::Kind::Doctor);
    const RepInput in = newRep("DR009");
    CHECK(SaveRecord(t, t.code, in.code, [&] { AssignRep(t, in); }, [] { return true; }) == SaveResult::Added);
    RepTable reps(db.code, RepTable::Kind::MedicalRep);
    CHECK(reps.recCount() == 3);   // nothing went to gl_medrp
    CHECK(DeleteRecord(t, t.code, "DR001"));
    CHECK(codesInOrder(t, t.code) == std::vector<std::string>{"DR002", "DR003", "DR009"});
}

TEST_CASE("Products: fpcode order, add, update keeps the lot size", "[master]")
{
    FixtureDb db;
    ProductTable t(db.code);
    REQUIRE(t.isValid());
    CHECK(codesInOrder(t, t.fpcode) == std::vector<std::string>{"P100", "P200", "P300"});

    REQUIRE(SeekCode(t, t.fpcode, "P200"));
    CHECK(t.flotsize.num() == Catch::Approx(24.0));
    ProductInput in = ReadProduct(t);
    CHECK(in.fullName == "Cough syrup 200ml");
    in.fullName = "Cough syrup 250ml";
    in.avPrice = "7.10";
    CHECK(SaveRecord(t, t.fpcode, in.code, [&] { AssignProduct(t, in); }, [] { return true; }) ==
          SaveResult::Updated);
    REQUIRE(SeekCode(t, t.fpcode, "P200"));
    CHECK(TrimText(t.ffulln.str()) == "Cough syrup 250ml");
    CHECK(t.favprice.num() == Catch::Approx(7.10));
    CHECK(t.flotsize.num() == Catch::Approx(24.0));   // D10: the original wrote 0

    ProductInput added;
    added.code = "P050";
    added.name = "GAUZE";
    added.fullName = "Sterile gauze pads";
    added.unit = "PK";
    added.onHand = "1,000.000";
    CHECK(SaveRecord(t, t.fpcode, added.code, [&] { AssignProduct(t, added); }, [] { return true; }) ==
          SaveResult::Added);
    CHECK(codesInOrder(t, t.fpcode) == std::vector<std::string>{"P050", "P100", "P200", "P300"});
    REQUIRE(SeekCode(t, t.fpcode, "P050"));
    CHECK(t.fonhand.num() == Catch::Approx(1000.0));
    CHECK(DeleteRecord(t, t.fpcode, "P050"));
    CHECK(t.recCount() == 3);
}

TEST_CASE("Departments: add, update, delete the code typed", "[master]")
{
    FixtureDb db;
    DepartmentTable t(db.code);
    REQUIRE(t.isValid());
    CHECK(codesInOrder(t, t.depart) == std::vector<std::string>{"10", "20"});

    auto save = [&](const std::string& code, const std::string& descript, bool confirm) {
        return SaveRecord(
            t, t.depart, code,
            [&] {
                t.depart.assign(code.c_str());
                t.descript.assign(descript.c_str());
            },
            [&] { return confirm; });
    };
    CHECK(save("30", "Sales", true) == SaveResult::Added);
    CHECK(save("10", "Research", false) == SaveResult::Unchanged);
    CHECK(save("10", "Research", true) == SaveResult::Updated);
    REQUIRE(SeekCode(t, t.depart, "10"));
    CHECK(TrimText(t.descript.str()) == "Research");

    // D10: the original deleted whatever record was current. Leave "30"
    // current and delete "20": only "20" goes.
    REQUIRE(SeekCode(t, t.depart, "30"));
    CHECK(DeleteRecord(t, t.depart, "20"));
    CHECK(codesInOrder(t, t.depart) == std::vector<std::string>{"10", "30"});
    CHECK_FALSE(DeleteRecord(t, t.depart, "1"));   // a prefix of "10" is not "10"
}
