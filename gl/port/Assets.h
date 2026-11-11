//
// Assets.h - the fixed-asset tables and depreciation arithmetic (spec #4 4.1,
// 5; plan S4-M0). Portable: no Win32, so gl_tests checks it.
//
//   gl_fa      the asset register (gl/src/gl_fa.cpp, dialog 4004)
//   gl_fatyp   asset types and their GL accounts (dialog 4002)
//   gl_fadep   the port's depreciation history (spec #4 5.3), one row per
//              asset per run
//
// The table classes' header (db_fa.h) isn't in the repository; the fields are
// read by name from the real layouts (dbf/GL_FA.DBF, GL_FATYP.DBF). LIFELEFT
// holds the months *used* (spec #4 2.1, 5.2).
//
// Amounts are in cents, as Spec #2 D12.
//
#pragma once

#include "cb4sqlite.h"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

// ---- tables -------------------------------------------------------------

// Creates gl_fa and gl_fatyp (the original layouts and tags) when the company
// lacks them, and gl_fadep when missing; then fills a blank ACCUM_DEP on any
// asset with months used, as monthly x months used capped at cost - salvage
// (spec #4 5.1: what the original's screen showed). Returns how many assets
// it filled. Run by every Fixed Assets screen, the run and the reports.
int EnsureAssetTables(cb4sqlite::Code4& code);

// ---- records ------------------------------------------------------------

struct AssetAccount {
    std::string account, dept, descript;
};

struct AssetType {
    std::string code;          // FATYPE
    std::string description;   // FADESC
    AssetAccount asset, expense, accum;
};

enum class DepMethod { Straight, Declining };

struct Asset {
    std::string code;          // FACODE
    std::string type;          // FATYPE
    std::string description, reference;
    std::string datePurchased; // yyyymmdd, "" if blank
    std::string dateSold;      // yyyymmdd, "" while in use
    DepMethod method = DepMethod::Straight;
    std::int64_t cost = 0, salvage = 0;
    std::int64_t monthly = 0;       // CURR_DEP
    std::int64_t accumulated = 0;   // ACCUM_DEP
    int life = 0;              // LIFETIME, months
    int monthsUsed = 0;        // LIFELEFT (spec #4 5.2)
    int percent = 0;           // PERCENT, the declining rate a year
    std::string mode;          // MODE: "D" disposed; "" or the original's "0" active (spec #4 5.1)

    bool disposed() const { return mode == "D"; }
};

std::vector<AssetType> LoadAssetTypes(cb4sqlite::Code4& code);   // in type order, deleted rows skipped
std::optional<AssetType> FindAssetType(cb4sqlite::Code4& code, const std::string& type);
std::vector<Asset> LoadAssets(cb4sqlite::Code4& code);           // in code order, deleted rows skipped
std::optional<Asset> FindAsset(cb4sqlite::Code4& code, const std::string& facode);

// Adds the record, or replaces the one with its code. True if it was added.
bool SaveAssetType(cb4sqlite::Code4& code, const AssetType& t);
bool SaveAsset(cb4sqlite::Code4& code, const Asset& a);

// TGLTypes::Ok / TGLFixedAssets::Ok's ValidateGL: the account, with that dept
// (blank for none), is in glaccts and not deleted.
bool ValidateAccount(cb4sqlite::Code4& code, const std::string& account, const std::string& dept);

// ---- the arithmetic (spec #4 4.1) ----------------------------------------

std::int64_t Depreciable(const Asset& a);   // cost - salvage
std::int64_t BookValue(const Asset& a);     // cost - accumulated
std::int64_t Remaining(const Asset& a);     // what is left to depreciate: cost - salvage - accumulated, not below 0

// The month's amount before the salvage floor. Straight line: (cost -
// salvage) / life. Declining balance (D3): the start-of-year book value x
// rate / 12. Rounded to the cent; 0 without a life or a rate.
std::int64_t MonthlyAmount(const Asset& a, std::int64_t startOfYearBookValue);

// What the next run depreciates (D4): nothing for a disposed or fully
// depreciated asset; otherwise the monthly amount, but never more than is
// left, and a straight-line asset's last month (months used + 1 >= life)
// takes all that is left, so the rounding comes out exactly.
std::int64_t DueAmount(const Asset& a, std::int64_t startOfYearBookValue);

// ---- history (gl_fadep) ---------------------------------------------------

struct DepreciationRow {
    std::string facode;
    std::string yearEnd;   // yyyymmdd: the fiscal year's period-12 end date
    int period = 0;
    long batchno = 0;      // the voucher; 0 for depreciation booked without one (the fixture's period 6)
    std::int64_t amount = 0;
};

// The fiscal year's last end date: gl_pds period 12's PERIODE. "" if unset.
std::string FiscalYearEnd(cb4sqlite::Code4& code);

std::vector<DepreciationRow> LoadDepreciation(cb4sqlite::Code4& code, const std::string& facode);
void AppendDepreciation(cb4sqlite::Code4& code, const DepreciationRow& row);

// The book value at the start of the fiscal year ending `yearEnd`: cost less
// the accumulated depreciation before this year's runs. For the declining
// method (D3). An asset bought this year (nothing accumulated before) gives
// its cost.
std::int64_t StartOfYearBookValue(cb4sqlite::Code4& code, const Asset& a, const std::string& yearEnd);
