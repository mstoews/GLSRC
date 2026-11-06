//
// dbf2sqlite - migrate a company's dBase IV files into a cb4sqlite database
// (spec 4.5). A thin wrapper over dbf2sqlite_core: argument parsing, output,
// exit code.
//
//   dbf2sqlite --src <dbf-dir> --out <company.db> [--table <name>]... [--force]
//              [--codepage <n>]
//              [--headers-out <dir> [--headers-check <dir>]...]
//

#include "Migrator.h"

#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#ifdef _WIN32
#include <Windows.h>
#endif

namespace {

void usage()
{
    std::fputs(
        "usage: dbf2sqlite --src <dbf-dir> --out <company.db> [options]\n"
        "\n"
        "  --src <dir>            directory with the company's .dbf and .mdx files\n"
        "  --out <file>           SQLite database to create (must not exist unless --force)\n"
        "  --table <name>         migrate only this table (repeatable); default: every\n"
        "                         dBase table in --src, including ones like ACCESS.PSW\n"
        "  --force                overwrite --out, and skip (with a warning) fields and\n"
        "                         index tags that can't be represented\n"
        "  --codepage <n>         source code page (e.g. 1252, 932); default: from each\n"
        "                         file's language-driver byte, else 1252\n"
        "  --headers-out <dir>    after migrating, write db_<table>.h for tables that no\n"
        "                         existing header opens\n"
        "  --headers-check <dir>  directory of existing headers/sources to scan (repeatable),\n"
        "                         e.g. include, gl/header, common\n"
        "\n"
        "exit codes: 0 ok, 1 bad arguments, 2 source read error, 3 destination write error,\n"
        "            4 schema/index translation error, 5 row-count verification mismatch\n",
        stderr);
}

int run(const std::vector<std::string>& args)
{
    dbf2sqlite::Options opt;
    auto path = [](const std::string& s) { return std::filesystem::path(std::u8string(s.begin(), s.end())); };
    for (size_t i = 0; i < args.size(); ++i) {
        const std::string& a = args[i];
        const bool hasValue = i + 1 < args.size();
        if (a == "--force") opt.force = true;
        else if (a == "--src" && hasValue) opt.srcDir = path(args[++i]);
        else if (a == "--out" && hasValue) opt.outPath = path(args[++i]);
        else if (a == "--table" && hasValue) opt.tables.push_back(args[++i]);
        else if (a == "--codepage" && hasValue) opt.codePage = std::atoi(args[++i].c_str());
        else if (a == "--headers-out" && hasValue) opt.headerOutDir = path(args[++i]);
        else if (a == "--headers-check" && hasValue) opt.headerCheckDirs.push_back(path(args[++i]));
        else if (a == "-h" || a == "--help") { usage(); return 0; }
        else {
            std::fprintf(stderr, "dbf2sqlite: unknown or incomplete argument '%s'\n\n", a.c_str());
            usage();
            return static_cast<int>(dbf2sqlite::ExitCode::BadArgs);
        }
    }
    if (opt.srcDir.empty() || opt.outPath.empty()) {
        usage();
        return static_cast<int>(dbf2sqlite::ExitCode::BadArgs);
    }

    dbf2sqlite::Result r = dbf2sqlite::migrate(opt);
    for (const std::string& line : r.log) std::printf("%s\n", line.c_str());
    if (r.code == dbf2sqlite::ExitCode::Ok && !opt.headerOutDir.empty()) {
        dbf2sqlite::Result h = dbf2sqlite::generateHeaders(opt);
        for (const std::string& line : h.log) std::printf("%s\n", line.c_str());
        r.code = h.code;
    }
    std::printf("%s (exit %d)\n", r.code == dbf2sqlite::ExitCode::Ok ? "done" : "FAILED", r.exitCode());
    return r.exitCode();
}

} // namespace

#ifdef _WIN32
int wmain(int argc, wchar_t** argv)
{
    SetConsoleOutputCP(CP_UTF8);
    std::vector<std::string> args;
    for (int i = 1; i < argc; ++i) {
        const int n = WideCharToMultiByte(CP_UTF8, 0, argv[i], -1, nullptr, 0, nullptr, nullptr);
        std::string s(static_cast<size_t>(n > 0 ? n - 1 : 0), '\0');
        WideCharToMultiByte(CP_UTF8, 0, argv[i], -1, s.data(), n, nullptr, nullptr);
        args.push_back(std::move(s));
    }
    return run(args);
}
#else
int main(int argc, char** argv)
{
    return run(std::vector<std::string>(argv + 1, argv + argc));
}
#endif
