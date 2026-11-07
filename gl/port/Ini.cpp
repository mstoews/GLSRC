//
// Ini.cpp - see Ini.h.
//
#include "Ini.h"

#include "win32fx.h"

#include <vector>

namespace {

std::wstring exeDirectory()
{
    std::vector<wchar_t> buf(MAX_PATH);
    for (;;) {
        const DWORD n = GetModuleFileNameW(nullptr, buf.data(), static_cast<DWORD>(buf.size()));
        if (n == 0) return L".";
        if (n < buf.size()) break;
        buf.resize(buf.size() * 2);
    }
    std::wstring path(buf.data());
    const size_t slash = path.find_last_of(L"\\/");
    return slash == std::wstring::npos ? L"." : path.substr(0, slash);
}

std::wstring fullPath(const std::wstring& path)
{
    const DWORD n = GetFullPathNameW(path.c_str(), 0, nullptr, nullptr);
    if (n == 0) return path;
    std::wstring out(n, L'\0');
    const DWORD m = GetFullPathNameW(path.c_str(), n, out.data(), nullptr);
    out.resize(m);
    return out;
}

bool isRelative(const std::wstring& p)
{
    if (p.size() >= 2 && (p[0] == L'\\' || p[0] == L'/')) return false;   // \\server, \root
    if (p.size() >= 2 && p[1] == L':') return false;                       // C:...
    return true;
}

// Spec risk 10: SQLite over SMB is not safe without the locking a later spec adds.
bool isNetworkPath(const std::wstring& full)
{
    auto starts = [&](const wchar_t* prefix) {
        const int n = static_cast<int>(wcslen(prefix));
        return static_cast<int>(full.size()) >= n && CompareStringOrdinal(full.c_str(), n, prefix, n, TRUE) == CSTR_EQUAL;
    };
    if (starts(L"\\\\?\\UNC\\")) return true;
    if (starts(L"\\\\?\\") || starts(L"\\\\.\\")) return false;   // a local device path
    if (starts(L"\\\\") || starts(L"//")) return true;              // \\server\share
    if (full.size() >= 3 && full[1] == L':') {
        const wchar_t root[4] = {full[0], L':', L'\\', 0};
        return GetDriveTypeW(root) == DRIVE_REMOTE;                   // a mapped network drive
    }
    return false;
}

} // namespace

std::wstring IniPath(const wchar_t* file)
{
    return exeDirectory() + L"\\" + file;
}

std::wstring GetIniString(const wchar_t* file, const wchar_t* section, const wchar_t* key, const wchar_t* def)
{
    const std::wstring path = IniPath(file);
    std::vector<wchar_t> buf(256);
    for (;;) {
        const DWORD n = GetPrivateProfileStringW(section, key, def, buf.data(), static_cast<DWORD>(buf.size()),
                                                 path.c_str());
        if (n + 1 < buf.size()) return std::wstring(buf.data(), n);
        buf.resize(buf.size() * 2);
    }
}

bool WriteIniString(const wchar_t* file, const wchar_t* section, const wchar_t* key, const wchar_t* value)
{
    return WritePrivateProfileStringW(section, key, value, IniPath(file).c_str()) != FALSE;
}

CompanyDbLocation FindCompanyDb()
{
    CompanyDbLocation r;
    const std::wstring dir = fx::Trim(GetIniString(L"GLDR.INI", L"GL_DIR", L"DIR"));
    if (dir.empty()) {
        r.error = L"Can't find ini file.\n\n" + IniPath(L"GLDR.INI") +
                  L"\nneeds [GL_DIR] DIR=<company database>.";   // TManilaGL::TManilaGL
        return r;
    }
    r.path = fullPath(isRelative(dir) ? exeDirectory() + L"\\" + dir : dir);
    if (isNetworkPath(r.path)) {
        r.error = L"Spec #1: company DB must be on local disk.\n\n" + r.path;
        return r;
    }
    const DWORD attr = GetFileAttributesW(r.path.c_str());
    if (attr == INVALID_FILE_ATTRIBUTES) {
        r.error = L"The company database does not exist:\n" + r.path +
                  L"\n\nCreate it from the .dbf directory with dbf2sqlite (gl\\README.md).";
    } else if (attr & FILE_ATTRIBUTE_DIRECTORY) {
        r.error = L"GLDR.INI [GL_DIR] DIR names a directory:\n" + r.path +
                  L"\n\nThe port reads one database file per company. Migrate the directory with "
                  L"dbf2sqlite and set DIR to the .db file (gl\\README.md).";
    }
    return r;
}
