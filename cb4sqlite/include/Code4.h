#pragma once
//
// Code4 - the database session (spec 4.2).
//
// In CodeBase, Code4 was only an environment: tables were separate .dbf files
// that each Data4 opened by name. Here one Code4 owns the connection to one
// company SQLite file, and Data4 objects open tables inside it.
//
//   open(path)   - new in the shim: opens the company database.
//   close()      - new in the shim: closes every Data4 and the connection.
//   closeAll()   - as CodeBase: closes every open Data4 (they become
//                  !isValid()); the connection stays open so the legacy
//                  pattern `cb.closeAll(); cb.init(); ...reopen tables...` works.
//   init()       - as CodeBase: resets error state; no-op otherwise.
//   initUndo()   - compile-only stub, returns 0 (CodeBase undo log not modelled).
//

#include "cb4sqlite_compat.h"
#include "Errors.h"

#include <cstdlib>
#include <string>
#include <string_view>
#include <vector>

struct sqlite3;

namespace cb4sqlite {

class Data4;

class Code4 {
public:
    Code4() noexcept = default;
    ~Code4();
    Code4(const Code4&) = delete;
    Code4& operator=(const Code4&) = delete;

    // Opens the company database (UTF-8 path). Returns r4success, or e4open if
    // the file doesn't exist (and create == false) or can't be opened. Any
    // previously open database is closed first. Pragmas: foreign_keys=ON,
    // journal_mode=DELETE (a single file, so the legacy file-copy backup stays
    // correct; WAL would leave committed data in a -wal side file).
    int open(const char* utf8Path, bool create = false);
#ifdef _WIN32
    int open(std::wstring_view path, bool create = false);
#endif
    void close() noexcept;
    bool isOpen() const noexcept { return db_ != nullptr; }

    int  closeAll() noexcept;
    int  init() noexcept { errorCode = 0; return 0; }
    int  initUndo() noexcept { return 0; }

    sqlite3* handle() noexcept { return db_; }

    // Last error code, as in CodeBase (0 = none). Set by failed opens.
    int errorCode = 0;

private:
    friend class Data4;
    void attach(Data4* d);
    void detach(Data4* d) noexcept;

    sqlite3*            db_ = nullptr;
    std::vector<Data4*> open_;   // Data4 objects currently attached
};

// CodeBase numeric helpers. In CodeBase Str4ptr was a class with operator
// double(); the original code only ever writes `double(Str4ptr(f->str()))`,
// which these functions satisfy.
inline double Str4ptr(const char* s) noexcept
{
    return (s && *s) ? std::atof(s) : 0.0;
}
inline long Str4long(const char* s) noexcept
{
    return (s && *s) ? std::atol(s) : 0L;
}

} // namespace cb4sqlite

using cb4sqlite::Code4;
using cb4sqlite::Str4ptr;
using cb4sqlite::Str4long;

// The application-wide session, as in the original gl.cpp (`Code4 cb;`).
// Declared here so ported code can keep writing `&cb`; *defined* in
// gl/port/App.cpp (plan M5). Tests never use it: they create a local Code4.
extern cb4sqlite::Code4 cb;
