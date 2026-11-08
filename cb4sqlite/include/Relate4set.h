#pragma once
//
// Relate4set - filtered iteration over one Data4 (spec 4.2).
//
// The original code uses only the single-table form:
//
//   Relate4set* q = new Relate4set(dBase);
//   q->querySet(".NOT. DELETED()");
//   for (int rc = q->top(); rc != r4eof; rc = q->skip(1L)) { ...dBase fields... }
//   q->unlock(); q->free();
//
// Iteration moves the parent Data4 itself (its Field4s read each match), in
// the parent's selected tag order.
//
// Supported querySet() expressions (spec #2 5.1; keywords and field names
// case-insensitive, spaces ignored):
//   nullptr / ""                       every record
//   .NOT. DELETED()  /  !DELETED()     records not marked deleted
//   FIELD = 'text'                     character field starts with text (dBase SET EXACT OFF)
//   FIELD <> 'text'  (also # and !=)   the negation
//   clause .AND. clause ...
// Literals may use ' or ". Text outside this grammar throws cb4sqlite::Error
// ("not implemented"); an unknown or non-character field throws Error too.
// The values are bound as SQL parameters, never pasted into the SQL.
//

#include "Data4.h"

#include <string>

namespace cb4sqlite {

class Relate4set {
public:
    explicit Relate4set(Data4* parent) noexcept : data_(parent) {}
    explicit Relate4set(Data4& parent) noexcept : data_(&parent) {}

    int  querySet(const char* expression = nullptr);
    int  top();
    int  bottom();
    int  skip(long n = 1);
    bool eof() const noexcept { return data_ == nullptr || data_->eof(); }

    int  unlock() noexcept { return 0; }          // no locking in spec #1
    int  free(int = 0) noexcept { data_ = nullptr; return 0; }

private:
    Data4*      data_ = nullptr;
    detail::QueryFilter filter_;                  // empty sql = every record
};

} // namespace cb4sqlite

using cb4sqlite::Relate4set;
