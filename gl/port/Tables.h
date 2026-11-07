//
// Tables.h - GL_PROF, GL_PDS and GL_SETAT for the ported code.
//
// The legacy classes (include/gl_prof.hpp, gl_pds.hpp, gl_setat.hpp, from
// CodeBase's 1994 class generator) don't compile as C++: `open( Code4& ) ;`
// has no return type. These keep their names and members, but open the table
// in the constructor (cb4sqlite's Data4 has no default constructor) and bind
// fields by name instead of by number:
//   - gl/src/gl_setat.cpp binds GL_SETAT by number as type, name, account,
//     dept, which doesn't match the real table (SHORT, NAME, ACCOUNT, DEPT,
//     TYPE); gl_profi.cpp uses the fields by their names.
// A table that is missing leaves the object !isValid() and its fields invalid.
//
#pragma once

#include "cb4sqlite.h"

class GL_PROF : public Data4 {
public:
    explicit GL_PROF(Code4& code)
        : Data4(code, "gl_prof"), company(*this, "company"), address1(*this, "address1"),
          address2(*this, "address2"), address3(*this, "address3"), postal(*this, "postal"),
          telephone(*this, "telephone"), fax(*this, "fax"), contact(*this, "contact")
    {
    }
    Field4 company, address1, address2, address3, postal, telephone, fax, contact;
};

class GL_PDS : public Data4 {
public:
    explicit GL_PDS(Code4& code)
        : Data4(code, "gl_pds"), company_nm(*this, "company_nm"), period(*this, "period"),
          periods(*this, "periods"), periode(*this, "periode"), pd_desc(*this, "pd_desc"),
          status(*this, "status")
    {
    }
    Field4 company_nm, period, periods, periode, pd_desc, status;
};

class GL_SETAT : public Data4 {
public:
    explicit GL_SETAT(Code4& code)
        : Data4(code, "gl_setat"), name(*this, "name"), account(*this, "account"), dept(*this, "dept"),
          type(*this, "type")
    {
    }
    Field4 name, account, dept, type;
};
