//
// parse_smoke.cpp
//
// M0 exit criterion: this file must compile against the cb4sqlite stub
// headers (M0) plus the existing hand-written schema headers in
// ../../include/.  No link, no run — pure parse check.
//
// If this file fails to compile, M0 is not done.
//

#include "../include/cb4sqlite.h"

// Schema headers from GLSRC/include/.  Pulled in via IncludePath = ..\..\include
// when this project is opened in the gl.sln solution.
#include "db_accts.h"   // Tglaccts (44 fields)
#include "db_acces.h"   // TAccess, TAccessl

namespace {

// Touch every type referenced from the schema headers.  Forces full parse.
void parse_smoke_touchpoints() {
    cb4sqlite::Code4 cb_local;
    Tglaccts*        gl  = nullptr;   (void)gl;
    TAccess*         ac  = nullptr;   (void)ac;
    TAccessl*        acl = nullptr;   (void)acl;

    // Spec §4.2 helpers in scope:
    double d = Str4ptr("123.45");      (void)d;
    long   n = Str4long("42");         (void)n;

    // Compile-only references to a few Field4 / Data4 methods so an accidental
    // signature change in the stub headers breaks the smoke at the same time
    // as the schema headers.
    Data4* d4 = nullptr;
    if (d4) {
        (void)d4->top();
        (void)d4->SetIndex("accounts");
        (void)d4->isValid();
    }
    Field4* f4 = nullptr;
    if (f4) {
        (void)f4->str();
        (void)f4->assignDouble(0.0);
    }
}

} // namespace

int main() {
    // Never executed in CI — this file is compile-only.  The presence of
    // a main() lets a misconfigured project still link an EXE for sanity.
    return 0;
}
