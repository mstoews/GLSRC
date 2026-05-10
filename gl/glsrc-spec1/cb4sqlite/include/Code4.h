#pragma once
//
// Code4 — database session.  M0 stub: declarations only, no behaviour.
// Spec §4.2.  Real implementation lands in M1.
//

#include "cb4sqlite_compat.h"
#include "Errors.h"

#include <Windows.h>   // LPSTR, LPCSTR
#include <cstdlib>     // std::atof, std::atol

// Forward-declare the SQLite handle type so M0 doesn't require sqlite3.h.
struct sqlite3;

namespace cb4sqlite {

class Code4 {
public:
    Code4()  noexcept = default;
    ~Code4() noexcept = default;

    // 0 on success, negative on error.  M1 implements via sqlite3_open_v2.
    int      open(const char* dbPath);
    int      open(LPSTR dbPath) { return open(static_cast<const char*>(dbPath)); }
    void     closeAll();
    void     initUndo();
    sqlite3* handle() noexcept;
};

// Global Code4 instance, defined in App.cpp (M5).
// Tests in M1 must not rely on this — they instantiate locally.
extern Code4 cb;

// CodeBase utility helpers used throughout original source.
inline double Str4ptr(const char* s) noexcept {
    return (s && *s) ? std::atof(s) : 0.0;
}
inline long Str4long(const char* s) noexcept {
    return (s && *s) ? std::atol(s) : 0L;
}

} // namespace cb4sqlite

// Original code calls Str4ptr/Str4long unqualified.  Keep ports' diff small
// by exposing them in the global namespace too.
using cb4sqlite::Code4;
using cb4sqlite::Str4ptr;
using cb4sqlite::Str4long;
