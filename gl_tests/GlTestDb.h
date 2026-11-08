#pragma once
//
// GlTestDb.h - the migrated dev fixture for gl_tests (spec #2 9, plan S2-M1).
//
// The first use migrates gl/test/dev-fixture/dbf/ with dbf2sqlite_core into a
// temporary "master" database; each FixtureDb then works on its own copy, so
// tests can change the books freely.
//

#include "catch_amalgamated.hpp"

#include "cb4sqlite.h"
#include "Migrator.h"

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <string>

namespace gltest {

namespace fs = std::filesystem;

inline fs::path tempPath(const char* stem)
{
    static std::atomic<int> counter{0};
    const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
    return fs::temp_directory_path() /
           (std::string(stem) + "_" + std::to_string(stamp) + "_" + std::to_string(counter++) + ".db");
}

// The repository root: GL_REPO_ROOT, or the nearest parent of the working
// directory that holds the dev fixture (VS runs tests from the project
// directory, the command line usually from the root or x64\Debug).
inline fs::path repoRoot()
{
#ifdef _WIN32
    char* env = nullptr;
    size_t len = 0;
    if (_dupenv_s(&env, &len, "GL_REPO_ROOT") == 0 && env) {
        fs::path p = env;
        std::free(env);
        return p;
    }
#else
    if (const char* env = std::getenv("GL_REPO_ROOT")) return env;
#endif
    for (fs::path p = fs::current_path(); !p.empty(); p = p.parent_path()) {
        if (fs::exists(p / "gl" / "test" / "dev-fixture" / "dbf")) return p;
        if (p == p.parent_path()) break;
    }
    FAIL("cannot find the repo root (set GL_REPO_ROOT)");
    return {};
}

// Removes the file (and a leftover journal) when the process ends.
class TempFile {
public:
    explicit TempFile(fs::path p) : path_(std::move(p)) {}
    ~TempFile()
    {
        std::error_code ec;
        fs::remove(path_, ec);
        fs::remove(path_.string() + "-journal", ec);
    }
    TempFile(const TempFile&) = delete;
    TempFile& operator=(const TempFile&) = delete;
    const fs::path& path() const { return path_; }

private:
    fs::path path_;
};

inline const fs::path& masterFixture()
{
    static TempFile master(tempPath("gl_tests_master"));
    static const bool migrated = [] {
        dbf2sqlite::Options opt;
        opt.srcDir = repoRoot() / "gl" / "test" / "dev-fixture" / "dbf";
        opt.outPath = master.path();
        const dbf2sqlite::Result r = dbf2sqlite::migrate(opt);
        if (r.code != dbf2sqlite::ExitCode::Ok) {
            std::string log;
            for (const std::string& l : r.log) log += l + "\n";
            FAIL("dev fixture migration failed (exit " << r.exitCode() << "):\n" << log);
        }
        return true;
    }();
    (void)migrated;
    return master.path();
}

// A private copy of the migrated dev fixture, open in `code`.
class FixtureDb {
public:
    FixtureDb() : file_(tempPath("gl_tests"))
    {
        fs::copy_file(masterFixture(), file_.path(), fs::copy_options::overwrite_existing);
        REQUIRE(code.open(file_.path().string().c_str()) == r4success);
    }
    ~FixtureDb() { code.close(); }
    FixtureDb(const FixtureDb&) = delete;
    FixtureDb& operator=(const FixtureDb&) = delete;

    Code4 code;

private:
    TempFile file_;
};

// An empty company database: the meta tables and nothing else.
class EmptyDb {
public:
    EmptyDb() : file_(tempPath("gl_tests_empty"))
    {
        REQUIRE(code.open(file_.path().string().c_str(), true) == r4success);
    }
    ~EmptyDb() { code.close(); }
    EmptyDb(const EmptyDb&) = delete;
    EmptyDb& operator=(const EmptyDb&) = delete;

    Code4 code;

private:
    TempFile file_;
};

} // namespace gltest
