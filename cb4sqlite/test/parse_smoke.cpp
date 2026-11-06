//
// parse_smoke.cpp - compile-only check (plan M0 task 7, M1 exit criterion 4).
//
// Must compile cleanly under /W4 /WX. It:
//  * includes the hand-written schema headers unchanged, so every inline
//    member of Tglaccts / TAccess / TAccessl / Tgldept is type-checked;
//  * calls every shim method the Spec #1 screens use (gl.cpp, gl_acces.cpp,
//    gl_profi.cpp, gl_accts.cpp, gl_acctl.cpp), in the shapes they use them.
// Built and linked by cb4sqlite_tests (Catch2 supplies main()); nothing here runs.
//

#include "cb4sqlite.h"

#include "legacy_headers_begin.h"
#include "db_accts.h"   // include/db_accts.h    - Tglaccts (44 fields)
#include "db_acces.h"   // gl/header/db_acces.h  - TAccess, TAccessl
#include "db_dept.h"    // include/db_dept.h     - Tgldept
#include "legacy_headers_end.h"

// External linkage and never called: avoids MSVC C4505 / C4702.
void parse_smoke_touchpoints(Code4& code, char* szSeek)
{
    // Code4
    (void)code.open("company.db");
    (void)code.closeAll();
    (void)code.init();
    (void)code.initUndo();

    // Schema classes, constructed as the screens do.
    Tglaccts* glaccts = new Tglaccts(&code);
    TAccess   access(&code);
    TAccessl  accessl(&code);
    Tgldept*  gldept = new Tgldept(&code);
    double total = glaccts->Open_bal() + glaccts->Curr_bal() + glaccts->dPeriod12()
                 + glaccts->dBudget1() + glaccts->dPrevious12();
    (void)total;

    // Data4 navigation / seek / write, as called in the Spec #1 sources.
    Data4& d = *glaccts;
    (void)d.SetIndex("accounts");
    (void)d.top();
    (void)d.bottom();
    (void)d.skip(1L);
    (void)d.go(d.recNo());
    (void)d.recCount();
    (void)d.numFields();
    if (d.seek(szSeek) != 0 && !d.bof() && !d.eof()) {
        (void)d.appendStart();
        Field4 field;
        for (int fieldNum = 1; fieldNum <= d.numFields(); fieldNum++) {
            field.init(*(Data4*)glaccts, fieldNum);
            if (field.type() == r4num) field.assignDouble(0);
        }
        (void)d.append();
    }
    (void)d.flush();
    (void)d.unlock();
    d.deleteRec();
    (void)d.pack();
    (void)d.zap();
    (void)d.isValid();

    // Field4, as the screens use it.
    (void)glaccts->accounts->assign("100");
    (void)glaccts->accounts->str();
    (void)glaccts->accounts->ptr();
    (void)glaccts->accounts->len();
    (void)glaccts->accounts->name();
    glaccts->curr_bal->assignDouble(1.0);
    glaccts->curr_bal->assignLong(1L, 4);
    access.fPassword->trim();
#ifdef _WIN32
    (void)glaccts->accounts->SetField(nullptr, 0);
    (void)glaccts->accounts->AssignField(nullptr, 0);
#endif

    // Date4, as gl.cpp UpdateUserTime.
    Date4 today;
    today.today();
    (void)glaccts->accounts->assign(today.str());

    // Relate4set, as gl_acctl.cpp TListWindow::Redraw.
    Relate4set* rQuery = new Relate4set(glaccts);
    (void)rQuery->querySet(".NOT. DELETED()");
    for (int qc = rQuery->top(); qc != r4eof; qc = rQuery->skip(1L)) {
        Field4 fField1(*(Data4*)glaccts, 1);
        (void)fField1.str();
    }
    (void)rQuery->unlock();
    (void)rQuery->free();
    delete rQuery;

    (void)Str4ptr("123.45");
    (void)Str4long("42");

    (void)accessl.seek("9");
    delete gldept;
    delete glaccts;
}
