#pragma once
//
// Field4 - accessor for one field of a Data4's current record (spec 4.2).
//
// A small copyable handle, as in CodeBase. It reads and writes the Data4's
// record buffer, which holds each field as its fixed-width dBase text image:
//
//   C  text, right-padded with spaces to len()           "100    "
//   N  number right-justified in len() with decimals()   "    12.50"
//   D  "YYYYMMDD", or 8 spaces when empty
//   L  "T" / "F", or " " when empty
//   (empty numeric fields are all spaces; num() reads them as 0)
//
// Lifetime of str()/ptr(): the pointer stays valid until the Data4 moves to
// another record or the field is assigned - the same contract as CodeBase.
// Copy the text if you need it longer.
//
// Text is UTF-8 (the SQLite encoding); widths are in bytes, and truncation
// never splits a UTF-8 sequence.
//
// A Field4 must not outlive its Data4.
//

#include "Data4.h"

#ifdef _WIN32
#include <Windows.h>
#endif

namespace cb4sqlite {

class Field4 {
public:
    Field4() noexcept = default;
    Field4(Data4& d, int fieldNo) noexcept { init(d, fieldNo); }     // 1-based
    Field4(Data4& d, const char* name) noexcept { init(d, name); }

    int init(Data4& d, int fieldNo) noexcept;          // 0, or -1 if no such field
    int init(Data4& d, const char* name) noexcept;     // case-insensitive name
    bool isValid() const noexcept { return data_ != nullptr; }

    // --- reading -------------------------------------------------------------
    const char* str() const noexcept;
    const char* ptr() const noexcept { return str(); }
    double      num() const noexcept { return Str4ptr(str()); }
    unsigned    len() const noexcept;
    int         decimals() const noexcept;
    char        type() const noexcept;        // r4str, r4num, r4float, r4date, r4log
    const char* name() const noexcept;
    int         number() const noexcept { return field_ + 1; }

    // --- writing (edits the record buffer; see Data4) -----------------------
    int  assign(const char* value);
    void assignDouble(double value, int newLen = -1, int nDec = -1);
    void assignLong(long value, int newLen = -1, int zerosInFront = 0);
    void trim();   // drops trailing spaces from the buffered text (not written back)

#ifdef _WIN32
    // From the application's Field class (gl/src/gl_field.cpp):
    // SetField puts str() into a dialog control; AssignField reads the control
    // text back (numeric fields are parsed as a number).
    int SetField(HWND hParent, UINT controlId) const;
    int AssignField(HWND hParent, UINT controlId);
#endif

private:
    const FieldDef& def() const noexcept;

    Data4* data_  = nullptr;
    int    field_ = -1;          // 0-based
};

} // namespace cb4sqlite

using cb4sqlite::Field4;
