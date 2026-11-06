#pragma once
//
// dbf2sqlite_core - one-way migration of a company's dBase IV files (.dbf +
// production .mdx) into one cb4sqlite database (spec 4.5), and generation of
// db_<table>.h headers for tables that have no hand-written class.
//
// The CLI (dbf2sqlite/main.cpp) and the Layer-2 tests both drive this API.
//

#include <filesystem>
#include <string>
#include <vector>

namespace dbf2sqlite {

// Process exit codes (spec 4.5).
enum class ExitCode : int {
    Ok          = 0,
    BadArgs     = 1,
    SourceRead  = 2,   // missing / unreadable / malformed .dbf or .mdx
    DestWrite   = 3,   // SQLite or file-system failure on the output side
    Translate   = 4,   // schema or index expression the shim can't represent
    Verify      = 5,   // row counts differ after migration
};

struct Options {
    std::filesystem::path srcDir;          // company directory with .dbf / .mdx files
    std::filesystem::path outPath;         // company .db to create
    std::vector<std::string> tables;       // empty = every dBase table in srcDir
    bool force = false;                    // overwrite outPath; skip (with a warning)
                                           // tags/fields that can't be translated
    int codePage = 0;                      // source code page; 0 = from each file's
                                           // language-driver byte, else 1252

    // Header generation (generateHeaders only).
    std::filesystem::path headerOutDir;               // where new db_<table>.h go
    std::vector<std::filesystem::path> headerCheckDirs; // existing headers/sources to scan
};

struct TableResult {
    std::string table;                      // normalised table name
    std::string sourceFile;                 // file name in srcDir
    long rows = 0;
    long deletedRows = 0;
    std::vector<std::string> tags;          // "name = key_expr" for each migrated tag
    std::string header;                     // generateHeaders: written path or skip reason
};

struct Result {
    ExitCode code = ExitCode::Ok;
    std::vector<TableResult> tables;
    std::vector<std::string> log;           // [info] / [warn] / [error] / [skip] lines
    int exitCode() const noexcept { return static_cast<int>(code); }
};

// Migrates srcDir into outPath. The database is built in "<outPath>.tmp" and
// renamed into place only if everything succeeded, so a failed run never
// leaves a half-written company database behind.
Result migrate(const Options& opt);

// Writes db_<table>.h into headerOutDir for each table in outPath that no
// existing header or source in headerCheckDirs/headerOutDir opens (detected by
// `Data4(cb, "<table>")` / `Data4::open(cb, "<table>")`). Never overwrites a
// file. Output is deterministic: re-running produces no diff.
Result generateHeaders(const Options& opt);

} // namespace dbf2sqlite
