#pragma once
//
// Relate4set — multi-cursor / query helper.  M0 stub.  Spec §4.2.
// Spec #1 supports literal predicates only (e.g. ".NOT. DELETED()").
//

#include "Data4.h"

namespace cb4sqlite {

class Relate4set {
public:
    explicit Relate4set(Data4* parent);
    ~Relate4set();

    int  querySet(const char* expression);
    int  top();
    int  skip(long n);
    bool eof() const noexcept;

    void unlock();   // no-op in spec #1
    void free();     // no-op in spec #1

private:
    void* impl_ = nullptr;
};

} // namespace cb4sqlite

using cb4sqlite::Relate4set;
