//
// MasterTables.h - the master-data tables of Spec #2 S2-M2 and their record
// operations, without the dialogs (MasterData.cpp):
//
//   gldept     Department Codes  (gl/src/gl_deptc.cpp, dialog 1023)
//   gl_medrp   Medical Reps      (gl/src/gl_medrp.cpp, dialog 1006)
//   gl_drcod   Doctor Codes      (gl/src/gl_drcod.cpp, dialog 1040)
//   gl_itemm   Product Codes     (gl/src/gl_prods.cpp, dialog 1007)
//
// No Win32 here, so gl_tests checks the operations against the migrated
// fixture.
//
// The table classes open their fields by name from the real layouts
// (dbf/GL_MEDRP.DBF etc.), not from the recovered headers: db_medrp.h lacks
// territory and type, db_drcod.h opens fields by number and is wrong from
// field 9 on, and db_itemm.h selects a tag the real index lacks (Spec #2
// design 5.4). gl_medrp and gl_drcod have the same 18-field layout, so one
// class serves both.
//
#pragma once

#include "cb4sqlite.h"

#include <string>

// "AB" -> "AB   " for a 5-wide field; longer text is left as it is.
std::string PadKey(const std::string& text, unsigned width);

// Seeks `code` (trimmed, padded to the field's width) on the active tag and
// checks the found record's `field` is exactly that code; seek() alone is a
// prefix match. True if found; the record is then current.
bool SeekCode(Data4& table, const Field4& field, const std::string& code);

// ---- gl_medrp / gl_drcod ------------------------------------------------

// A medical rep (gl_medrp, tag "repcode") or a doctor (gl_drcod, tag "drcode").
class RepTable : public Data4 {
public:
    enum class Kind { MedicalRep, Doctor };
    RepTable(Code4& code, Kind kind);

    Field4 code;   // repcode / drcode
    Field4 rpshort, rpname, address1, address2, address3, postal, phone, territory, fax, descript,
        contact, type, ref, status, terms, balance, creditlmt;
};

// The fields the dialogs edit, as UTF-8 control text. Numbers may carry the
// thousands separators a tbFloat shows.
struct RepInput {
    std::string code, shortName, name, address1, address2, address3, postal, phone, fax, contact, territory;
    std::string terms, creditLimit;
};

// TGLMedReps::Update / TGLDrCodes::Update on the current record (or the one
// appendStart() began). The balance is not written: neither dialog has a
// control for it, and the original wrote 0 from the missing control on
// every save.
void AssignRep(RepTable& t, const RepInput& in);

// The current record as RepInput (TGLMedReps::SetData).
RepInput ReadRep(const RepTable& t);

// ---- gl_itemm -----------------------------------------------------------

// A product (gl_itemm, tag "fpcode").
class ProductTable : public Data4 {
public:
    explicit ProductTable(Code4& code);

    Field4 fpcode, fpname, ffulln, fptype, felement, fclass, fskbn, fshop1, fshop2, fplace, funit, flotsize,
        fonhand, flmhand, flmprice, favprice, fstprice;
};

struct ProductInput {
    std::string code, name, fullName, type, unit;
    std::string onHand;    // control 6006 ("Cost" on the template), as the original stored it
    std::string avPrice;   // control 6012 ("Unit Price")
};

// TGLProd::UpdateFields. The lot size is not written: dialog 1007 has no
// control for it (6007), and the original wrote 0 on every save.
void AssignProduct(ProductTable& t, const ProductInput& in);
ProductInput ReadProduct(const ProductTable& t);

// ---- gldept --------------------------------------------------------------

// A department (gldept, tag "depart").
class DepartmentTable : public Data4 {
public:
    explicit DepartmentTable(Code4& code);

    Field4 depart, descript;
};

// ---- shared --------------------------------------------------------------

enum class SaveResult { Added, Updated, Unchanged };

// "Add/Mod" on a code table: appends the record if `code` isn't there,
// otherwise updates it when `confirmUpdate` returns true. `assign` writes the
// input fields into the current (or new) record.
template <class Table, class Assign, class Confirm>
SaveResult SaveRecord(Table& t, const Field4& codeField, const std::string& code, Assign assign,
                      Confirm confirmUpdate)
{
    if (!SeekCode(t, codeField, code)) {
        t.appendStart();
        assign();
        t.append();
        return SaveResult::Added;
    }
    if (!confirmUpdate()) return SaveResult::Unchanged;
    assign();
    t.flush();
    return SaveResult::Updated;
}

// Delete on a code table: seeks `code` and removes that record
// (deleteRec() + pack()). False if it isn't there.
bool DeleteRecord(Data4& t, const Field4& codeField, const std::string& code);
