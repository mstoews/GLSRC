#pragma once
//
// Field4 — column accessor.  M0 stub: declarations only.
// Spec §4.2.  Real implementation in M1.
//

#include "Data4.h"

#include <Windows.h>   // HWND, UINT

namespace cb4sqlite {

class Field4 {
public:
    Field4(Data4& d, int index);              // 1-based, matches CodeBase
    Field4(Data4& d, const char* name);
    ~Field4();

    // Returns pointer into row buffer; lifetime = until next nav on the parent Data4.
    const char* str()  const;
    double      num()  const;
    long        len()  const noexcept;
    const char* name() const noexcept;

    int  assign(const char* value);
    int  assign(double value);
    int  assignDouble(double value);   // matches gl_budgt.cpp call sites

    int  SetField(HWND hParent, UINT controlId);
    int  AssignField(HWND hParent, UINT controlId);

    bool isValid() const noexcept;

private:
    void* impl_ = nullptr;
};

} // namespace cb4sqlite

using cb4sqlite::Field4;
