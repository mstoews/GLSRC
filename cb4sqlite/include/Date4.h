#pragma once
//
// Date4 - dBase date value "YYYYMMDD" (spec 4.2). Mirrors the parts of the
// CodeBase class the original code uses:
//
//   Date4 d; d.today(); fDate.assign(d.str());        // gl.cpp UpdateUserTime
//

#include "cb4sqlite_compat.h"

namespace cb4sqlite {

class Date4 {
public:
    Date4() noexcept;                       // empty date: 8 spaces
    explicit Date4(const char* yyyymmdd) noexcept { assign(yyyymmdd); }

    void today() noexcept;                  // sets *this to the local date
    void assign(const char* yyyymmdd) noexcept;

    const char* str() const noexcept { return dt_; }
    const char* ptr() const noexcept { return dt_; }
    unsigned    len() const noexcept { return 8; }

    int year()  const noexcept;             // 0 for an empty date
    int month() const noexcept;
    int day()   const noexcept;

private:
    char dt_[9];
};

} // namespace cb4sqlite

using cb4sqlite::Date4;
