#pragma once
//
// QueryExpr.h - Relate4set::querySet expressions -> an SQL predicate with bound
// parameters (spec #2 5.1). Not a public header.
//
//   query  := clause { .AND. clause }
//   clause := .NOT. DELETED() | !DELETED()
//           | FIELD op literal
//   op     := = | <> | # | !=
//   literal:= 'text' | "text"
//
// Keywords and field names are case-insensitive; spaces between tokens are
// ignored. Only character fields can be compared with a text literal, as in
// dBase (anything else is a type mismatch there).
//
// Comparison follows dBase with SET EXACT OFF, the rule dbf2sqlite already
// uses for tags' FOR conditions (dbf2sqlite_core/src/KeyExpr.cpp):
//   FIELD = 'lit'   the stored value, at its full padded width, starts with
//                   lit; so ACCOUNT = '4' matches "410001 ", '' matches every
//                   record, and a literal longer than the field never matches.
//   <> # !=         the negation of =.
// The comparison is case-sensitive (dBase compares bytes).
//

#include "Data4.h"

#include <string_view>

namespace cb4sqlite::detail {

// Throws cb4sqlite::Error: notImplemented for text outside the grammar, a plain
// Error for an unknown field or a non-character field.
QueryFilter translateQuery(const TableInfo& table, std::string_view expression);

} // namespace cb4sqlite::detail
