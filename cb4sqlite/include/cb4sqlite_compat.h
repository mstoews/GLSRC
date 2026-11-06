#pragma once
//
// cb4sqlite_compat.h
//
// CodeBase 4 constants and macros that ported code and the legacy schema
// headers (db_*.h) use unchanged. Values are copied from the original CodeBase
// headers in include/ (d4defs.h, e4expr.h), so code that compares against
// them behaves exactly as before.
//

// _CLASSDEF(T) was an OWL macro that typedef'd T* / T& aliases. The new code
// doesn't use those aliases; keep the legacy headers compiling.
#ifndef _CLASSDEF
#define _CLASSDEF(name) /* no-op */
#endif

// The legacy schema headers use LPSTR and BOOL. On Windows they come from <Windows.h>
// (pulled in by Field4.h); these typedefs keep them compiling in the portable test
// build of cb4sqlite_tests on other platforms.
#ifndef _WIN32
typedef char* LPSTR;
typedef int   BOOL;    // include/db_arbtc.h declares BOOL members
#endif

// Navigation / seek return codes (d4defs.h).
inline constexpr int r4success = 0;
inline constexpr int r4found   = 1;
inline constexpr int r4after   = 2;
inline constexpr int r4eof     = 3;
inline constexpr int r4bof     = 4;
inline constexpr int r4entry   = 5;   // no record (go) / no index entry

// Error codes (d4defs.h). Negative, as in CodeBase.
inline constexpr int e4open      = -60;
inline constexpr int e4seek      = -100;
inline constexpr int e4write     = -120;
inline constexpr int e4data      = -200;
inline constexpr int e4fieldName = -210;
inline constexpr int e4index     = -310;
inline constexpr int e4tagName   = -330;
inline constexpr int e4info      = -910;
inline constexpr int e4parm      = -930;

// Field types as returned by Field4::type() (e4expr.h).
inline constexpr char r4str   = 'C';
inline constexpr char r4num   = 'N';
inline constexpr char r4float = 'F';
inline constexpr char r4date  = 'D';
inline constexpr char r4log   = 'L';
inline constexpr char r4memo  = 'M';
