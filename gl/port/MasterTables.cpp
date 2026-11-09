//
// MasterTables.cpp - see MasterTables.h.
//
#include "MasterTables.h"

#include "Accounts.h"   // TrimText
#include "Company.h"    // UpdateUserTime

extern char szUser[11];   // Globals.h, which needs <Windows.h>; this file stays portable

namespace {

// tbFloat text ("1,234.50") -> "1234.50"; blank -> "0".
std::string number(const std::string& text)
{
    std::string digits;
    for (char c : text)
        if (c != ',' && c != ' ') digits += c;
    return digits.empty() ? "0" : digits;
}

} // namespace

std::string PadKey(const std::string& text, unsigned width)
{
    std::string key = text;
    if (key.size() < width) key.append(width - key.size(), ' ');
    return key;
}

bool SeekCode(Data4& table, const Field4& field, const std::string& code)
{
    const std::string want = TrimText(code);
    if (!table.isValid() || !field.isValid() || want.empty()) return false;
    if (table.seek(PadKey(want, field.len()).c_str()) != r4success) return false;
    return TrimText(field.str()) == want;
}

bool DeleteRecord(Data4& t, const Field4& codeField, const std::string& code)
{
    if (!SeekCode(t, codeField, code)) return false;
    t.deleteRec();
    t.pack();
    return true;
}

// ---- gl_medrp / gl_drcod ------------------------------------------------

RepTable::RepTable(Code4& c, Kind kind)
    : Data4(&c, kind == Kind::MedicalRep ? "gl_medrp" : "gl_drcod")
{
    if (!isValid()) return;
    SetIndex(kind == Kind::MedicalRep ? "repcode" : "drcode");
    code.init(*this, kind == Kind::MedicalRep ? "repcode" : "drcode");
    rpshort.init(*this, "rpshort");
    rpname.init(*this, "rpname");
    address1.init(*this, "address1");
    address2.init(*this, "address2");
    address3.init(*this, "address3");
    postal.init(*this, "postal");
    phone.init(*this, "phone");
    territory.init(*this, "territory");
    fax.init(*this, "fax");
    descript.init(*this, "descript");
    contact.init(*this, "contact");
    type.init(*this, "type");
    ref.init(*this, "ref");
    status.init(*this, "status");
    terms.init(*this, "terms");
    balance.init(*this, "balance");
    creditlmt.init(*this, "creditlmt");
    top();
}

void AssignRep(RepTable& t, const RepInput& in)
{
    t.code.assign(in.code.c_str());
    t.rpshort.assign(in.shortName.c_str());
    t.rpname.assign(in.name.c_str());
    t.address1.assign(in.address1.c_str());
    t.address2.assign(in.address2.c_str());
    t.address3.assign(in.address3.c_str());
    t.postal.assign(in.postal.c_str());
    t.phone.assign(in.phone.c_str());
    t.fax.assign(in.fax.c_str());
    t.contact.assign(in.contact.c_str());
    t.terms.assign(number(in.terms).c_str());
    t.creditlmt.assign(number(in.creditLimit).c_str());
    t.territory.assign(in.territory.c_str());
    UpdateUserTime(t, szUser);   // gl_medrp / gl_drcod have no update/user/time fields: a no-op
}

RepInput ReadRep(const RepTable& t)
{
    RepInput r;
    r.code = TrimText(t.code.str());
    r.shortName = TrimText(t.rpshort.str());
    r.name = TrimText(t.rpname.str());
    r.address1 = TrimText(t.address1.str());
    r.address2 = TrimText(t.address2.str());
    r.address3 = TrimText(t.address3.str());
    r.postal = TrimText(t.postal.str());
    r.phone = TrimText(t.phone.str());
    r.fax = TrimText(t.fax.str());
    r.contact = TrimText(t.contact.str());
    r.territory = TrimText(t.territory.str());
    r.terms = TrimText(t.terms.str());
    r.creditLimit = TrimText(t.creditlmt.str());
    return r;
}

// ---- gl_itemm -----------------------------------------------------------

ProductTable::ProductTable(Code4& c) : Data4(&c, "gl_itemm")
{
    if (!isValid()) return;
    SetIndex("fpcode");
    fpcode.init(*this, "fpcode");
    fpname.init(*this, "fpname");
    ffulln.init(*this, "ffulln");
    fptype.init(*this, "fptype");
    felement.init(*this, "felement");
    fclass.init(*this, "fclass");
    fskbn.init(*this, "fskbn");
    fshop1.init(*this, "fshop1");
    fshop2.init(*this, "fshop2");
    fplace.init(*this, "fplace");
    funit.init(*this, "funit");
    flotsize.init(*this, "flotsize");
    fonhand.init(*this, "fonhand");
    flmhand.init(*this, "flmhand");
    flmprice.init(*this, "flmprice");
    favprice.init(*this, "favprice");
    fstprice.init(*this, "fstprice");
    top();
}

void AssignProduct(ProductTable& t, const ProductInput& in)
{
    t.fpcode.assign(in.code.c_str());
    t.fpname.assign(in.name.c_str());
    t.ffulln.assign(in.fullName.c_str());
    t.fptype.assign(in.type.c_str());
    t.funit.assign(in.unit.c_str());
    t.fonhand.assign(number(in.onHand).c_str());
    t.favprice.assign(number(in.avPrice).c_str());
    UpdateUserTime(t, szUser);
}

ProductInput ReadProduct(const ProductTable& t)
{
    ProductInput p;
    p.code = TrimText(t.fpcode.str());
    p.name = TrimText(t.fpname.str());
    p.fullName = TrimText(t.ffulln.str());
    p.type = TrimText(t.fptype.str());
    p.unit = TrimText(t.funit.str());
    p.onHand = TrimText(t.fonhand.str());
    p.avPrice = TrimText(t.favprice.str());
    return p;
}

// ---- gldept --------------------------------------------------------------

DepartmentTable::DepartmentTable(Code4& c) : Data4(&c, "gldept")
{
    if (!isValid()) return;
    SetIndex("depart");
    depart.init(*this, "depart");
    descript.init(*this, "descript");
    top();
}
