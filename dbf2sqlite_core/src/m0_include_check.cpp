//
// m0_include_check.cpp - M0 placeholder (plan M0 exit criteria).
//
// Proves this project's IncludePath resolves the cb4sqlite shim and the legacy
// schema headers. Replaced by real sources in plan milestone M2.
//
#include "cb4sqlite.h"

#include "legacy_headers_begin.h"
#include "db_accts.h"
#include "legacy_headers_end.h"

// Gives the library one public symbol (avoids LNK4221 "no public symbols").
int dbf2sqlite_core_m0_anchor() { return static_cast<int>(sizeof(Tglaccts) != 0); }
