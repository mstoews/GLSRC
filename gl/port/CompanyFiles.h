//
// CompanyFiles.h - the file side of Backup and Restore (spec #2 D5, plan
// S2-M6), without the dialogs (Backup.cpp has those), so gl_tests can check
// the swap and its rollback.
//
// The original ran external DOS programs, savegl.pif and restore.pif
// (gl/src/gl.cpp). The company is one SQLite file now:
//
//   Backup   Code4::backupTo, to a file named BackupFileName by default.
//   Restore  RestoreDatabase: check the chosen file, close the company,
//            keep the current file as <name>.bak, copy the chosen file over
//            it and reopen. If anything fails after the close, the .bak is
//            put back and the company reopened as it was.
//
#pragma once

#include "cb4sqlite.h"

#include <filesystem>
#include <string>

// "<stem>-<yyyymmdd>.db" for the company file `database`
// ("C:\GL\manila.db", 2026-11-09 -> "manila-20261109.db").
std::filesystem::path BackupFileName(const std::filesystem::path& database, int year, int month, int day);

// Code4::open on a path, as UTF-16 on Windows and UTF-8 elsewhere.
int OpenCompany(Code4& code, const std::filesystem::path& database);

// Is `file` a company database: an undamaged SQLite file with glaccts and
// _cb_fields (Code4::checkDatabase), and not `current` itself? False, with
// the reason in `why`, otherwise. The file isn't changed.
bool CheckRestoreFile(const std::filesystem::path& current, const std::filesystem::path& file, std::string* why);

struct RestoreResult {
    bool restored = false;
    bool reopened = false;   // `code` is open, on the restored or the original file
    std::string error;       // why it wasn't restored
};

// Restores `file` over the company database `current`, which `code` has
// open. `code` is closed first, so every table open on it is closed. On
// success `current` holds a copy of `file`, the old one is
// `current`.bak (replacing an older .bak), and `code` is open on it.
RestoreResult RestoreDatabase(Code4& code, const std::filesystem::path& current, const std::filesystem::path& file);

// "<current>.bak".
std::filesystem::path BackupOfCurrent(const std::filesystem::path& current);
