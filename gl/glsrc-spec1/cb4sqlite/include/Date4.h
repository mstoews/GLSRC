#pragma once
//
// Date4 — date helper.  M0 stub.  Spec §4.2.
//

#include "cb4sqlite_compat.h"
#include <string>

namespace cb4sqlite {

class Date4 {
public:
    static Date4 today();
    std::string str() const;   // "YYYYMMDD"

private:
    int year_ = 0, month_ = 0, day_ = 0;
};

} // namespace cb4sqlite

using cb4sqlite::Date4;
