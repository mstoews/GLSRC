//
// Code4.cpp - database session.
//

#include "Code4.h"
#include "Data4.h"
#include "Internal.h"

#include <algorithm>

#ifdef _WIN32
#include <Windows.h>
#endif

namespace cb4sqlite {

Code4::~Code4()
{
    close();
}

int Code4::open(const char* utf8Path, bool create)
{
    close();
    errorCode = 0;
    const int flags = SQLITE_OPEN_READWRITE | (create ? SQLITE_OPEN_CREATE : 0);
    sqlite3* db = nullptr;
    if (!utf8Path || sqlite3_open_v2(utf8Path, &db, flags, nullptr) != SQLITE_OK) {
        sqlite3_close(db);   // a handle is returned even on failure
        errorCode = e4open;
        return e4open;
    }
    db_ = db;
    try {
        detail::exec(db_, "PRAGMA foreign_keys = ON; PRAGMA journal_mode = DELETE;", "Code4::open");
        ensureMetaTables(*this);
    } catch (...) {
        close();
        throw;
    }
    return r4success;
}

#ifdef _WIN32
int Code4::open(std::wstring_view path, bool create)
{
    const int wlen = static_cast<int>(path.size());
    const int n = WideCharToMultiByte(CP_UTF8, 0, path.data(), wlen, nullptr, 0, nullptr, nullptr);
    std::string utf8(static_cast<size_t>(n), '\0');
    WideCharToMultiByte(CP_UTF8, 0, path.data(), wlen, utf8.data(), n, nullptr, nullptr);
    return open(utf8.c_str(), create);
}
#endif

void Code4::close() noexcept
{
    closeAll();
    if (db_) {
        sqlite3_close(db_);   // statements are per-call RAII, so none are left open
        db_ = nullptr;
    }
}

int Code4::closeAll() noexcept
{
    // Data4::close() detaches itself, so iterate over a copy.
    const std::vector<Data4*> snapshot = open_;
    for (Data4* d : snapshot) {
        try {
            d->close();
        } catch (...) {
            d->detachFromCode();   // pending write failed; drop it rather than leak the cursor
        }
    }
    open_.clear();
    return r4success;
}

void Code4::attach(Data4* d)
{
    open_.push_back(d);
}

void Code4::detach(Data4* d) noexcept
{
    open_.erase(std::remove(open_.begin(), open_.end(), d), open_.end());
}

} // namespace cb4sqlite
