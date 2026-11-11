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

// TGLTypes::Delete, refusing a type an asset uses (spec #4 D9; the original
// deleted it and left its assets pointing at nothing). The record is deleted
// and the table packed, as the original and Spec #2's DeleteRecord do.
// `usedBy` gets the first asset (in code order) that uses the type.
enum class DeleteTypeResult { Deleted, NotFound, InUse };
DeleteTypeResult DeleteAssetType(cb4sqlite::Code4& code, const std::string& type, std::string* usedBy = nullptr);

// TGLTypes::Ok / TGLFixedAssets::Ok's ValidateGL: the account, with that dept
// (blank for none), is in glaccts and not deleted.
bool ValidateAccount(cb4sqlite::Code4& code, const std::string& account, const std::string& dept);

// ---- checking an asset before it is saved (spec #4 6.2) ----------------------

// What Add/Mod refuses, in the order the screen checks it. `field` names the
// input to put the focus on.
enum class AssetField { None, Code, Type, Description, Date, Cost, Salvage, Life, MonthsUsed, Rate, Accumulated };
struct AssetProblem {
    AssetField field = AssetField::None;
    enum class Kind {
        Required,        // string 8101, naming the field
        NoSuchType,      // the type isn't in gl_fatyp
        SalvageOverCost, // string 9097
        UsedOverLife,    // string 9098
        AccumOverDepreciable,
    } kind = Kind::Required;
};

// TGLFixedAssets::Ok's checks, and those CheckInput meant to make (it was
// never called): code, type (which must exist), description, a valid date
// purchased, cost > 0, life > 0, a rate for the declining method; salvage <=
// cost, months used <= life, accumulated <= cost - salvage.
std::optional<AssetProblem> CheckAsset(cb4sqlite::Code4& code, const Asset& a);

// True if the run has depreciated the asset (it has gl_fadep rows): its
// accumulated depreciation is then the history's, not typed (D6).
bool HasDepreciation(cb4sqlite::Code4& code, const std::string& facode);

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

// ---- Disposal (spec #4 6.3, D7; plan S4-M4) -----------------------------------

// The sale price less the book value: a gain if positive, a loss if negative.
std::int64_t GainOnSale(const Asset& a, std::int64_t salePrice);

enum class DisposeResult { Disposed, NotFound, AlreadyDisposed };

// Disposal's Dispose (the original TGLFixedDelete::Ok deleted the record):
// sets DATE_SOLD to `dateSold` (yyyymmdd) and MODE to "D". Nothing is
// posted (D7): the screen gives the entry to make in Voucher Entry.
DisposeResult DisposeAsset(cb4sqlite::Code4& code, const std::string& facode, const std::string& dateSold);

enum class DeleteAssetResult { Deleted, NotFound, HasHistory };

// Disposal's Delete, for an asset entered by mistake: deletes and packs it,
// unless the run has depreciated it (it has gl_fadep rows).
DeleteAssetResult DeleteAsset(cb4sqlite::Code4& code, const std::string& facode);

// ---- the run: Create Voucher (spec #4 4.2, 4.3; plan S4-M3) ------------------

struct RunItem {
    Asset asset;
    AssetType type;
    std::int64_t amount = 0;    // this month's depreciation
    std::int64_t monthly = 0;   // the monthly amount, written to CURR_DEP
};

enum class SkipReason {
    Disposed,
    FullyDepreciated,   // nothing left above the salvage value
    NoType,             // the type isn't in gl_fatyp (string 8120)
    BadAccount,         // the type's expense or accumulated-depreciation account isn't in the chart:
                        // posting would refuse the whole voucher
    NotYetBought,       // purchased after the period's end (D8)
};

struct RunSkip {
    Asset asset;
    SkipReason reason = SkipReason::Disposed;
};

// What a run for `period` would do, changing nothing: the dialog shows it
// before OK. `alreadyRun` is set when the period of this fiscal year already
// has a run (D5): a gl_fadep row whose voucher still exists, or one booked
// without a voucher (batchno 0); `existingVoucher` is then its voucher.
struct RunPlan {
    int period = 0;
    std::string periodEnd;   // yyyymmdd: gl_pds.periode, the voucher's date (D10); "" if unset
    std::string yearEnd;     // FiscalYearEnd
    std::vector<RunItem> due;
    std::vector<RunSkip> skipped;
    std::int64_t total = 0;
    bool alreadyRun = false;
    long existingVoucher = 0;
};
RunPlan PlanRun(cb4sqlite::Code4& code, int period);

enum class RunStatus {
    Created,      // the voucher was written
    NothingDue,   // no asset is due: no voucher
    AlreadyRun,   // D5: the period has a run (RunResult::batchno is its voucher)
    NoPeriodEnd,  // the period has no end date in gl_pds
};
struct RunResult {
    RunStatus status = RunStatus::NothingDue;
    long batchno = 0;
    int assets = 0;
    std::int64_t total = 0;
};

// TGLCreateDeprSlip::Ok, fixed (spec #4 4.2), in one transaction: one open
// voucher (mode 1) numbered NextVoucherNumber(iniBatchno), "Monthly
// Depreciation Voucher", dated the period's end, with two lines per asset
// due (Dr the type's expense account, Cr its accumulated-depreciation
// account, the asset code as the reference), numbered 1, 2, 3, ...; then
// each asset's ACCUM_DEP, months used and CURR_DEP, and a gl_fadep row.
RunResult RunDepreciation(cb4sqlite::Code4& code, int period, long iniBatchno);

// Undoes a run (spec #4 4.3): for each gl_fadep row of voucher `batchno`, the
// asset's accumulated depreciation goes down by the amount and its months
// used by 1, and the row is deleted. No transaction of its own: Cancel
// Voucher calls it inside its own. Returns the number of assets restored (0
// for an ordinary voucher, or a company without gl_fadep).
int UndoRun(cb4sqlite::Code4& code, long batchno);
