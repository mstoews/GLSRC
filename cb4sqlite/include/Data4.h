#pragma once
//
// Data4 — cursor over one table.  M0 stub: declarations only.
// Spec §4.2.  Real implementation in M1.
//

#include "Code4.h"

#include <Windows.h>   // LPSTR

namespace cb4sqlite {

class Data4 {
public:
    Data4(Code4* env, const char* tableName);
    Data4(Code4* env, LPSTR tableName)
        : Data4(env, static_cast<const char*>(tableName)) {}
    ~Data4();

    int  top();
    int  bottom();
    int  skip(long n);

    // ANSI string seek; key is right-padded with spaces to active tag's
    // leading column width before compare (per spec §4.3 padding rule).
    int  seek(const char* key);
    int  seek(long recordNumber);

    int  append();
    int  flush();   // see spec §4.6 — no-op in spec #1

    long recCount() noexcept;
    long recNo()    noexcept;
    bool eof()      noexcept;
    int  numFields() const noexcept;

    int  SetIndex(const char* tagName);
    int  go(long recordNumber);
    bool isValid() const noexcept;
    void close();

protected:
    // Hidden state — populated in M1.  Stub to keep the header self-contained.
    void* impl_ = nullptr;
};

} // namespace cb4sqlite

using cb4sqlite::Data4;
