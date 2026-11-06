//
// legacy_headers_begin.h / legacy_headers_end.h
//
// Bracket every #include of a hand-written 1994 schema header (db_accts.h,
// db_acces.h, ...) so its warnings don't fail the /W4 /WX build (spec 2.1).
// We silence the specific warnings instead of editing the legacy headers:
//
//   #include "legacy_headers_begin.h"
//   #include "db_accts.h"
//   #include "legacy_headers_end.h"
//
// Add a code here only when a legacy header demonstrably needs it, with a note
// saying which header and why.
//
// No #pragma once: the pair must work every time it is included.
//
#if defined(_MSC_VER)
#  pragma warning(push)
#  pragma warning(disable: 4100)   // unreferenced formal parameter: `LPSTR name=0` ctor args
#elif defined(__GNUC__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
