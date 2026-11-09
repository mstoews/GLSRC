//
// CompanyFiles.cpp - see CompanyFiles.h.
//
#include "CompanyFiles.h"

#include <cstdio>
#include <exception>
#include <system_error>

namespace fs = std::filesystem;

namespace {

std::string utf8(const fs::path& p)
{
    const auto u8 = p.u8string();
    return std::string(u8.begin(), u8.end());
}

bool sameFile(const fs::path& a, const fs::path& b)
{
    std::error_code ec;
    if (fs::exists(a, ec) && fs::exists(b, ec) && fs::equivalent(a, b, ec)) return true;
    return fs::weakly_canonical(a, ec) == fs::weakly_canonical(b, ec);
}

} // namespace

fs::path BackupFileName(const fs::path& database, int year, int month, int day)
{
    char date[16];
    std::snprintf(date, sizeof date, "%04d%02d%02d", year, month, day);
    fs::path name = database.stem();
    name += "-";
    name += date;
    name += ".db";
    return name;
}

fs::path BackupOfCurrent(const fs::path& current)
{
    fs::path bak = current;
    bak += ".bak";
    return bak;
}

int OpenCompany(Code4& code, const fs::path& database)
{
#ifdef _WIN32
    return code.open(std::wstring_view(database.native()));
#else
    return code.open(database.c_str());
#endif
}

bool CheckRestoreFile(const fs::path& current, const fs::path& file, std::string* why)
{
    if (sameFile(current, file)) {
        if (why) *why = "it is the company database already open";
        return false;
    }
    return Code4::checkDatabase(utf8(file).c_str(), {"_cb_fields", "glaccts"}, why);
}

RestoreResult RestoreDatabase(Code4& code, const fs::path& current, const fs::path& file)
{
    RestoreResult r;
    if (!CheckRestoreFile(current, file, &r.error)) {
        r.reopened = code.isOpen();
        return r;
    }
    const fs::path bak = BackupOfCurrent(current);
    code.close();

    std::error_code ec;
    bool moved = false;
    try {
        fs::rename(current, bak, ec);   // replaces an older .bak
        if (ec) throw std::runtime_error("the current file can't be renamed to " + utf8(bak.filename()) + ": " +
                                         ec.message());
        moved = true;
        fs::copy_file(file, current, fs::copy_options::overwrite_existing, ec);
        if (ec) throw std::runtime_error("the file can't be copied: " + ec.message());
        if (OpenCompany(code, current) != r4success) throw std::runtime_error("the restored file can't be opened");
        r.restored = r.reopened = true;
        return r;
    } catch (const std::exception& e) {
        r.error = e.what();
    }

    // Put the original back and reopen it.
    code.close();
    if (moved) {
        fs::remove(current, ec);
        fs::rename(bak, current, ec);
        if (ec) r.error += "; and the original couldn't be put back from " + utf8(bak.filename());
    }
    try {
        r.reopened = OpenCompany(code, current) == r4success;
    } catch (const std::exception&) {
        r.reopened = false;
    }
    return r;
}
