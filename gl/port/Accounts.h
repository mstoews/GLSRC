//
// Accounts.h - the glaccts / gldept tables and the record operations of the
// Chart of Accounts (gl/src/gl_accts.cpp), without the dialog.
//
// No Win32 here, so the operations can be checked off Windows against a
// migrated database. ChartOfAccounts.cpp reads the controls into an
// AccountInput and calls these.
//
#pragma once

#include "cb4sqlite.h"

#include <string>
#include <vector>

#include "legacy_headers_begin.h"
#include "db_accts.h"
#include "db_dept.h"
#include "legacy_headers_end.h"

// Tglaccts (include/db_accts.h, unchanged) `new`s its 44 Field4s and never
// frees them; this deletes them. If the table is missing the legacy
// constructor leaves the pointers unset, so they are nulled first.
class AccountsTable : public Tglaccts {
public:
    explicit AccountsTable(Code4& code);
    ~AccountsTable() override;
    AccountsTable(const AccountsTable&) = delete;
    AccountsTable& operator=(const AccountsTable&) = delete;
};

// Tgldept (include/db_dept.h, unchanged), likewise.
class DeptTable : public Tgldept {
public:
    explicit DeptTable(Code4& code) : Tgldept(&code) {}
    ~DeptTable() override
    {
        delete dept;
        delete descript;
    }
    DeptTable(const DeptTable&) = delete;
    DeptTable& operator=(const DeptTable&) = delete;
};

// The "accounts" tag key (account+dept) for an account and dept as typed:
// the account padded to its field width, then the dept.
std::string AccountKey(const std::string& account, const std::string& dept, unsigned accountWidth);

// Seeks account+dept on the "accounts" tag and checks the found record is
// that account and dept (seek() alone is a prefix match). True if found;
// the record is then current.
bool SeekAccount(Tglaccts& gl, const std::string& account, const std::string& dept);

// The current record has balances in a period or the previous year: the
// original refused to delete it (string 1027).
bool HasActivity(Tglaccts& gl);

// The fields the dialog edits, as UTF-8 control text. Numbers may carry the
// thousands separators a tbFloat shows.
struct AccountInput {
    std::string account, dept, descript, fdescript;
    std::string type;          // "B", "I", "R", or "N" when no type is chosen
    std::string currBal, openBal;
};

// TGLAcctsAdd::Ok's append: every numeric field zeroed, then the input.
// openBal goes into both previous12 and openbal, as the original did.
void AppendAccount(Tglaccts& gl, const AccountInput& in);

// TGLAcctsAdd::Modify on the current record: descriptions, previous12
// (from openBal) and curr_bal, and the type when one is chosen.
void ModifyAccount(Tglaccts& gl, const AccountInput& in);

// TGLAcctsAdd::Delete on the current record: deleteRec() then pack(), so
// the row is removed from the table.
void DeleteAccount(Tglaccts& gl);

// "  abc  " -> "abc".
std::string TrimText(const std::string& s);

// tbFloat or grid text ("1,234.50", "-12.00", "") -> its value; blank is 0.
double ParseAmount(const std::string& text);

// One period's row of Budget Data (TGLBudget::Update, dialog 1017).
struct BudgetRow {
    double budget = 0;     // budget<N>
    double current = 0;    // period<N>
    double previous = 0;   // previous<N>
};

// The current record's 12 rows, period 1 first.
std::vector<BudgetRow> ReadBudget(Tglaccts& gl);

// TGLBudget::Ok on the current record: budget1..budget12 from `budget`
// (12 values), then the record is written.
void SaveBudget(Tglaccts& gl, const std::vector<double>& budget);
