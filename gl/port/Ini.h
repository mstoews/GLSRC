//
// Ini.h - GL.INI and GLDR.INI (spec 5.4, plan M5 task 7).
//
// The original passed bare file names ("GL.INI", "GLDR.INI") to
// GetPrivateProfileString, which reads them from the Windows directory. The
// port reads them from the directory of gl.exe instead: one checkout or
// install folder holds the program and its settings, and nothing is written
// under C:\Windows. The file format and keys are unchanged.
//
#pragma once

#include <Windows.h>

#include <string>

// <directory of gl.exe>\<file>
std::wstring IniPath(const wchar_t* file);

std::wstring GetIniString(const wchar_t* file, const wchar_t* section, const wchar_t* key,
                          const wchar_t* def = L"");
bool WriteIniString(const wchar_t* file, const wchar_t* section, const wchar_t* key, const wchar_t* value);

// GLDR.INI [GL_DIR] DIR, which named the .dbf directory, now names the
// company database file (spec 4.3). A relative path is relative to the
// directory of gl.exe. DRIVE is no longer needed (DIR is a full path) and is
// ignored. On failure `error` says why, in the words the user sees:
//   - no DIR: the original's "Can't find ini file.";
//   - DIR is a directory or doesn't exist (the database hasn't been migrated);
//   - a UNC path or network drive (spec risk 10): "Spec #1: company DB must be on local disk."
struct CompanyDbLocation {
    std::wstring path;    // full path of the .db file
    std::wstring error;   // empty on success
};
CompanyDbLocation FindCompanyDb();
