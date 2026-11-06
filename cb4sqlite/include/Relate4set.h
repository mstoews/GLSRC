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
// Supported querySet() expressions in Spec #1 (case and spaces ignored):
//   nullptr / ""                    every record
//   ".NOT. DELETED()" / "!DELETED()" records not marked deleted
// Anything else throws cb4sqlite::Error ("not implemented in spec #1").
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
    std::string filter_;                          // SQL predicate, empty = none
};

} // namespace cb4sqlite

using cb4sqlite::Relate4set;
