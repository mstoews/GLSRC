#pragma once
//
// cb4sqlite_compat.h
//
// OWL/CodeBase compatibility shims.  These exist solely so legacy schema
// headers (db_accts.h, db_acces.h, ...) parse against the new shim without
// needing to be edited.  No OWL behaviour is reintroduced.
//

// _CLASSDEF(T) is an old OWL macro that typedefed T*/T& aliases.  We don't
// use those aliases in the new code, so make it a no-op to keep the existing
// headers compiling.
#ifndef _CLASSDEF
#define _CLASSDEF(name) /* no-op */
#endif

// CodeBase return code convention: 0 = success, negative = error.
// Match the original constant so ports comparing against it still work.
namespace cb4sqlite { inline constexpr int r4eof = -22; }
using cb4sqlite::r4eof;
