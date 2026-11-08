//
// Code4.cpp - database session.
//

#include "Code4.h"
#include "Data4.h"
#include "Internal.h"

#include <algorithm>
#include <filesystem>
#include <system_error>

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
    inTransaction_ = false;   // sqlite3_close rolls an open transaction back
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

// --- transactions (spec #2 5.2) -------------------------------------------------

void Code4::begin()
{
    static const char* M = "Code4::begin";
    if (!db_) throw Error(M, "no database is open");
    if (inTransaction_ || !sqlite3_get_autocommit(db_)) throw Error(M, "a transaction is already open");
    // Pending record-buffer writes belong before the transaction, not in it.
    for (Data4* d : std::vector<Data4*>(open_)) d->flush();
    detail::exec(db_, "BEGIN IMMEDIATE", M);
    inTransaction_ = true;
}

void Code4::commit()
{
    static const char* M = "Code4::commit";
    if (!db_ || !inTransaction_) throw Error(M, "no transaction is open");
    // Write the last changed records, as moving off them would have.
    for (Data4* d : std::vector<Data4*>(open_)) d->flush();
    try {
        detail::exec(db_, "COMMIT", M);
    } catch (...) {
        inTransaction_ = !sqlite3_get_autocommit(db_);
        throw;
    }
    inTransaction_ = false;
}

void Code4::rollback()
{
    static const char* M = "Code4::rollback";
    if (!db_ || !inTransaction_) throw Error(M, "no transaction is open");
    inTransaction_ = false;
    // Drop unwritten buffer changes: they belong to the rolled-back work.
    for (Data4* d : open_) d->dirty_ = d->appending_ = false;
    if (!sqlite3_get_autocommit(db_))   // SQLite may already have rolled back after an error
        detail::exec(db_, "ROLLBACK", M);
}

// --- backup (spec #2 5.3) --------------------------------------------------------

void Code4::backupTo(const char* utf8Path)
{
    static const char* M = "Code4::backupTo";
    if (!db_) throw Error(M, "no database is open");
    if (inTransaction_) throw Error(M, "a transaction is open");
    if (!utf8Path || !*utf8Path) throw Error(M, "no file name");
    for (Data4* d : std::vector<Data4*>(open_)) d->flush();

    namespace fs = std::filesystem;
    const auto u8 = [](const std::string& s) { return fs::path(std::u8string(s.begin(), s.end())); };
    const std::string part = std::string(utf8Path) + ".part";
    std::error_code ec;
    fs::remove(u8(part), ec);
    sqlite3* dst = nullptr;
    if (sqlite3_open_v2(part.c_str(), &dst, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, nullptr) != SQLITE_OK) {
        const std::string msg = dst ? sqlite3_errmsg(dst) : "out of memory";
        sqlite3_close(dst);
        throw Error(M, msg + " [" + part + "]");
    }
    sqlite3_backup* b = sqlite3_backup_init(dst, "main", db_, "main");
    int rc = b ? sqlite3_backup_step(b, -1) : sqlite3_errcode(dst);
    if (b) {
        const int frc = sqlite3_backup_finish(b);
        if (rc == SQLITE_DONE) rc = frc;
    }
    std::string msg = (rc == SQLITE_OK || rc == SQLITE_DONE) ? std::string() : sqlite3_errmsg(dst);
    if (sqlite3_close(dst) != SQLITE_OK && msg.empty()) msg = "close failed";
    if (msg.empty()) {
        fs::rename(u8(part), u8(utf8Path), ec);   // replaces an existing file (MoveFileExW on Windows)
        if (ec) msg = "cannot move the copy into place: " + ec.message();
    }
    if (!msg.empty()) {
        fs::remove(u8(part), ec);
        throw Error(M, msg + " [" + utf8Path + "]");
    }
}

#ifdef _WIN32
void Code4::backupTo(std::wstring_view path)
{
    const int wlen = static_cast<int>(path.size());
    const int n = WideCharToMultiByte(CP_UTF8, 0, path.data(), wlen, nullptr, 0, nullptr, nullptr);
    std::string utf8(static_cast<size_t>(n), '\0');
    WideCharToMultiByte(CP_UTF8, 0, path.data(), wlen, utf8.data(), n, nullptr, nullptr);
    backupTo(utf8.c_str());
}
#endif

void Code4::attach(Data4* d)
{
    open_.push_back(d);
}

void Code4::detach(Data4* d) noexcept
{
    open_.erase(std::remove(open_.begin(), open_.end(), d), open_.end());
}

} // namespace cb4sqlite
