#pragma once
//
// TestDb.h - fixtures shared by the cb4sqlite Layer-1 tests.
//

#include "catch_amalgamated.hpp"

#include "cb4sqlite.h"
#include "sqlite3.h"

#include <atomic>
#include <chrono>
#include <filesystem>
#include <string>
#include <vector>

namespace test {

// A company database file in the temp directory, deleted afterwards.
class TempDb {
public:
    TempDb()
    {
        static std::atomic<int> counter{0};
        const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
        path_ = std::filesystem::temp_directory_path() /
                ("cb4sqlite_test_" + std::to_string(stamp) + "_" + std::to_string(counter++) + ".db");
    }
    ~TempDb()
    {
        std::error_code ec;
        std::filesystem::remove(path_, ec);
        std::filesystem::remove(path_.string() + "-journal", ec);
    }
    TempDb(const TempDb&) = delete;
    TempDb& operator=(const TempDb&) = delete;

    std::string path() const { return path_.string(); }

    // Opens (creating if needed) into `code`.
    void open(Code4& code) const { REQUIRE(code.open(path().c_str(), true) == r4success); }

private:
    std::filesystem::path path_;
};

// Test chart of accounts: tag "accounts" = account||dept, "descrip" = descript,
// "balance" = curr_bal (numeric key).
inline void createAccounts(Code4& code)
{
    cb4sqlite::createTable(code, "ACCTS.DBF", {
        {"ACCOUNT",  r4str,  7, 0},
        {"DEPT",     r4str,  3, 0},
        {"DESCRIPT", r4str, 20, 0},
        {"CURR_BAL", r4num, 12, 2},
        {"COUNT",    r4num,  5, 0},
        {"OPENED",   r4date, 8, 0},
        {"ACTIVE",   r4log,  1, 0},
    });
    cb4sqlite::createTag(code, "accts", "accounts", "account || dept");
    cb4sqlite::createTag(code, "accts", "descrip", "descript");
    cb4sqlite::createTag(code, "accts", "balance", "curr_bal");
}

inline void addAccount(Data4& d, const char* account, const char* dept, const char* descript, double bal)
{
    REQUIRE(d.appendStart() == r4success);
    d.blank();
    Field4(d, "account").assign(account);
    Field4(d, "dept").assign(dept);
    Field4(d, "descript").assign(descript);
    Field4(d, "curr_bal").assignDouble(bal);
    REQUIRE(d.append() == r4success);
}

// Rows added in deliberately unsorted order.
inline void addSampleAccounts(Data4& d)
{
    addAccount(d, "300", "01", "Revenue",      -500.00);
    addAccount(d, "100", "02", "Cash branch",    75.25);
    addAccount(d, "200", "01", "Payables",     -120.00);
    addAccount(d, "100", "01", "Cash",          250.50);
    addAccount(d, "400", "01", "Expenses",       42.00);
}

// Current record's account+dept, trimmed per field: "100/01".
inline std::string key(Data4& d)
{
    std::string a = Field4(d, "account").str();
    std::string b = Field4(d, "dept").str();
    a.erase(a.find_last_not_of(' ') + 1);
    b.erase(b.find_last_not_of(' ') + 1);
    return a + "/" + b;
}

// Walks forward from top() and returns every key.
inline std::vector<std::string> walk(Data4& d)
{
    std::vector<std::string> keys;
    for (int rc = d.top(); rc == r4success; rc = d.skip(1))
        keys.push_back(key(d));
    return keys;
}

// First column of the first row of an SQL query, as text ("<null>" if NULL).
inline std::string raw(Code4& code, const std::string& sql)
{
    sqlite3_stmt* s = nullptr;
    REQUIRE(sqlite3_prepare_v2(code.handle(), sql.c_str(), -1, &s, nullptr) == SQLITE_OK);
    std::string out = "<no row>";
    if (sqlite3_step(s) == SQLITE_ROW)
        out = sqlite3_column_type(s, 0) == SQLITE_NULL
                  ? "<null>"
                  : reinterpret_cast<const char*>(sqlite3_column_text(s, 0));
    sqlite3_finalize(s);
    return out;
}

} // namespace test
