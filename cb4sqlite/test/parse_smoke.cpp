//
// parse_smoke.cpp
//
// M0 exit criterion (plan M0 task 7): this file must compile cleanly, under
// /W4 /WX, against the cb4sqlite stub headers plus the hand-written schema
// headers found on the IncludePath ($(GlRoot)include, then $(GlRoot)gl\header).
// It is built and linked by cb4sqlite_tests; it is not run as a test.
//

#include "cb4sqlite.h"

#include "legacy_headers_begin.h"
#include "db_accts.h"   // include/db_accts.h    - Tglaccts (44 fields)
#include "db_acces.h"   // gl/header/db_acces.h  - TAccess, TAccessl
#include "legacy_headers_end.h"

// External linkage and never called: forces every inline member of the schema
// classes to be parsed and type-checked without triggering C4505 / C4702.
void parse_smoke_touchpoints(Code4& cb_local, Data4& d4, Field4& f4)
{
    Tglaccts accts(&cb_local);
    TAccess  access(&cb_local);
    TAccessl accessl(&cb_local);

    double total = accts.Open_bal() + accts.Curr_bal() + accts.dPeriod12()
                 + accts.dBudget1() + accts.dPrevious12();
    (void)total;
    (void)access.fUsername->str();
    (void)accessl.fMenuCode->str();

    // The CodeBase helpers ported code calls unqualified (spec 4.2).
    (void)Str4ptr("123.45");
    (void)Str4long("42");

    // A few Data4 / Field4 / Date4 signatures that the M5/M6 ports call, so a
    // stub-header change that breaks them fails here first.
    (void)d4.top();
    (void)d4.SetIndex("accounts");
    (void)d4.isValid();
    (void)f4.str();
    (void)f4.assignDouble(0.0);

    Date4 today;
    today.today();               // gl.cpp: `Date4 d; d.today(); fDate.assign(d.str());`
    (void)f4.assign(today.str().c_str());
}

int main()
{
    return 0;
}
