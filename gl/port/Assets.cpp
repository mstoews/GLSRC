//
// Assets.cpp - see Assets.h.
//
// Translation notes (plan per-port checklist):
//   - The arithmetic replaces TGLFixedAssets::UpdateData's straight-line and
//     "declining" amounts (spec #4 2.2), dAccum and the screen's net book
//     value (2.4 #3), and GL_FA.RPT's formulas (2.4 #4): one set, in cents.
//   - gl_fa and gl_fatyp have no update/user/time fields, so the original's
//     UpdateUserTime calls wrote nothing; they aren't repeated.
//
#include "Assets.h"

#include "Accounts.h"       // TrimText
#include "MasterTables.h"   // SeekCode
#include "Vouchers.h"

#include "report/Format.h"  // rpt::ToCents

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <stdexcept>

namespace {

bool hasTable(cb4sqlite::Code4& code, const char* name)
{
    cb4sqlite::Query q(code, "SELECT count(*) FROM sqlite_master WHERE type = 'table' AND name = ?1");
    q.bindText(1, name);
    return q.next() && q.integer(0) > 0;
}

// Cents as the text a numeric field takes ("1234.50").
std::string money(std::int64_t cents)
{
    char buf[40];
    const std::int64_t a = cents < 0 ? -cents : cents;
    std::snprintf(buf, sizeof buf, "%s%lld.%02lld", cents < 0 ? "-" : "", static_cast<long long>(a / 100),
                  static_cast<long long>(a % 100));
    return buf;
}

// a / b rounded half away from zero, in integers.
std::int64_t divRound(std::int64_t a, std::int64_t b)
{
    if (b == 0) return 0;
    const bool neg = (a < 0) != (b < 0);
    const std::int64_t ua = a < 0 ? -a : a, ub = b < 0 ? -b : b;
    const std::int64_t q = (ua + ub / 2) / ub;
    return neg ? -q : q;
}

DepMethod methodOf(const std::string& text)
{
    return TrimText(text) == "Declining" ? DepMethod::Declining : DepMethod::Straight;
}

const char* methodText(DepMethod m) { return m == DepMethod::Declining ? "Declining" : "Straight"; }

constexpr const char* kAssetColumns =
    "facode, fatype, descript, reference, date_pur, date_sold, dep_meth, cost, salvage, curr_dep, accum_dep, "
    "lifetime, lifeleft, percent, mode";

Asset readAsset(const cb4sqlite::Query& q)
{
    Asset a;
    a.code = q.text(0);
    a.type = q.text(1);
    a.description = q.text(2);
    a.reference = q.text(3);
    a.datePurchased = q.text(4);
    a.dateSold = q.text(5);
    a.method = methodOf(q.text(6));
    a.cost = rpt::ToCents(q.number(7));
    a.salvage = rpt::ToCents(q.number(8));
    a.monthly = rpt::ToCents(q.number(9));
    a.accumulated = rpt::ToCents(q.number(10));
    a.life = static_cast<int>(q.integer(11));
    a.monthsUsed = static_cast<int>(q.integer(12));
    a.percent = static_cast<int>(q.integer(13));
    a.mode = q.text(14);
    return a;
}

constexpr const char* kTypeColumns =
    "fatype, fadesc, asset_acct, asset_dept, asset_desc, expen_acct, expen_dept, expen_desc, "
    "accum_acct, accum_dept, accum_desc";

AssetType readType(const cb4sqlite::Query& q)
{
    AssetType t;
    t.code = q.text(0);
    t.description = q.text(1);
    t.asset = {q.text(2), q.text(3), q.text(4)};
    t.expense = {q.text(5), q.text(6), q.text(7)};
    t.accum = {q.text(8), q.text(9), q.text(10)};
    return t;
}

} // namespace

// ---- tables -------------------------------------------------------------

int EnsureAssetTables(cb4sqlite::Code4& code)
{
    const bool fa = hasTable(code, "gl_fa"), fatyp = hasTable(code, "gl_fatyp"), fadep = hasTable(code, "gl_fadep");
    if (!fa || !fatyp || !fadep) {
        cb4sqlite::Transaction tx(code);
        if (!fa) {   // dbf/GL_FA.DBF and GL_FA.MDX
            cb4sqlite::createTable(code, "gl_fa",
                                   {{"FACODE", 'C', 10},   {"DEP_METH", 'C', 10}, {"REFERENCE", 'C', 40},
                                    {"DESCRIPT", 'C', 40}, {"DATE_SOLD", 'D', 8}, {"DATE_PUR", 'D', 8},
                                    {"LIFELEFT", 'N', 16}, {"LIFETIME", 'N', 16}, {"PERCENT", 'N', 2},
                                    {"COST", 'N', 16, 2},  {"ACCUM_DEP", 'N', 16, 2}, {"CURR_DEP", 'N', 16, 2},
                                    {"SALVAGE", 'N', 16, 2}, {"FATYPE", 'C', 4},    {"MODE", 'C', 1}});
            cb4sqlite::createTag(code, "gl_fa", "facode", "\"FACODE\"");
            cb4sqlite::createTag(code, "gl_fa", "dep_meth", "\"DEP_METH\"");
        }
        if (!fatyp) {   // dbf/GL_FATYP.DBF and GL_FATYP.MDX
            cb4sqlite::createTable(code, "gl_fatyp",
                                   {{"FATYPE", 'C', 4},      {"FADESC", 'C', 30},     {"FAYEARS", 'N', 2},
                                    {"FAPERCENT", 'N', 2},   {"ASSET_ACCT", 'C', 7},  {"ASSET_DEPT", 'C', 4},
                                    {"ASSET_DESC", 'C', 40}, {"EXPEN_ACCT", 'C', 7},  {"EXPEN_DEPT", 'C', 4},
                                    {"EXPEN_DESC", 'C', 40}, {"ACCUM_ACCT", 'C', 7},  {"ACCUM_DEPT", 'C', 4},
                                    {"ACCUM_DESC", 'C', 40}});
            cb4sqlite::createTag(code, "gl_fatyp", "fatype", "\"FATYPE\"");
        }
        if (!fadep) {   // spec #4 5.3
            cb4sqlite::createTable(code, "gl_fadep",
                                   {{"FACODE", 'C', 10}, {"YEAR_END", 'D', 8}, {"PERIOD", 'N', 2},
                                    {"BATCHNO", 'N', 6}, {"AMOUNT", 'N', 16, 2}});
            cb4sqlite::createTag(code, "gl_fadep", "facode", "\"FACODE\"");
            cb4sqlite::createTag(code, "gl_fadep", "batchno", "\"BATCHNO\"");
        }
        tx.commit();
    }

    // The one-time fill (spec #4 5.1): a blank ACCUM_DEP with months used.
    std::vector<std::pair<std::string, std::int64_t>> fills;
    {
        cb4sqlite::Query q(code, "SELECT facode, cost, salvage, curr_dep, lifeleft FROM gl_fa "
                                 "WHERE _deleted = 0 AND accum_dep IS NULL AND lifeleft > 0");
        while (q.next()) {
            const std::int64_t depreciable = std::max<std::int64_t>(0, rpt::ToCents(q.number(1)) - rpt::ToCents(q.number(2)));
            const std::int64_t accum = std::min<std::int64_t>(depreciable, rpt::ToCents(q.number(3)) * q.integer(4));
            fills.emplace_back(q.text(0), accum);
        }
    }
    if (fills.empty()) return 0;
    cb4sqlite::Transaction tx(code);
    Data4 table(&code, "gl_fa");
    table.SetIndex("facode");
    Field4 facode(table, "facode"), accum(table, "accum_dep");
    for (const auto& [c, cents] : fills)
        if (SeekCode(table, facode, c)) {
            accum.assign(money(cents).c_str());
            table.flush();
        }
    table.close();
    tx.commit();
    return static_cast<int>(fills.size());
}

// ---- records ------------------------------------------------------------

std::vector<AssetType> LoadAssetTypes(cb4sqlite::Code4& code)
{
    std::vector<AssetType> out;
    cb4sqlite::Query q(code, std::string("SELECT ") + kTypeColumns + " FROM gl_fatyp WHERE _deleted = 0 ORDER BY fatype, _rowid");
    while (q.next()) out.push_back(readType(q));
    return out;
}

std::optional<AssetType> FindAssetType(cb4sqlite::Code4& code, const std::string& type)
{
    cb4sqlite::Query q(code, std::string("SELECT ") + kTypeColumns +
                                 " FROM gl_fatyp WHERE _deleted = 0 AND trim(fatype) = ?1 ORDER BY _rowid LIMIT 1");
    q.bindText(1, TrimText(type));
    if (!q.next()) return std::nullopt;
    return readType(q);
}

std::vector<Asset> LoadAssets(cb4sqlite::Code4& code)
{
    std::vector<Asset> out;
    cb4sqlite::Query q(code, std::string("SELECT ") + kAssetColumns + " FROM gl_fa WHERE _deleted = 0 ORDER BY facode, _rowid");
    while (q.next()) out.push_back(readAsset(q));
    return out;
}

std::optional<Asset> FindAsset(cb4sqlite::Code4& code, const std::string& facode)
{
    cb4sqlite::Query q(code, std::string("SELECT ") + kAssetColumns +
                                 " FROM gl_fa WHERE _deleted = 0 AND trim(facode) = ?1 ORDER BY _rowid LIMIT 1");
    q.bindText(1, TrimText(facode));
    if (!q.next()) return std::nullopt;
    return readAsset(q);
}

bool SaveAssetType(cb4sqlite::Code4& code, const AssetType& t)
{
    cb4sqlite::Transaction tx(code);
    Data4 d(&code, "gl_fatyp");
    d.SetIndex("fatype");
    Field4 fatype(d, "fatype"), fadesc(d, "fadesc");
    Field4 acct[3][3] = {{{d, "asset_acct"}, {d, "asset_dept"}, {d, "asset_desc"}},
                         {{d, "expen_acct"}, {d, "expen_dept"}, {d, "expen_desc"}},
                         {{d, "accum_acct"}, {d, "accum_dept"}, {d, "accum_desc"}}};
    const bool added = !SeekCode(d, fatype, t.code);
    if (added) {
        d.appendStart();   // then blank(): blank() first would flush blanks over the current record
        d.blank();
    }
    fatype.assign(TrimText(t.code).c_str());
    fadesc.assign(t.description.c_str());
    const AssetAccount* a[3] = {&t.asset, &t.expense, &t.accum};
    for (int i = 0; i < 3; ++i) {
        acct[i][0].assign(TrimText(a[i]->account).c_str());
        acct[i][1].assign(TrimText(a[i]->dept).c_str());
        acct[i][2].assign(a[i]->descript.c_str());
    }
    if (added) d.append(); else d.flush();
    d.close();
    tx.commit();
    return added;
}

namespace {

// SaveAsset without a transaction of its own, for the run's.
bool writeAsset(cb4sqlite::Code4& code, const Asset& a)
{
    Data4 d(&code, "gl_fa");
    d.SetIndex("facode");
    Field4 facode(d, "facode"), fatype(d, "fatype"), descript(d, "descript"), reference(d, "reference"),
        datePur(d, "date_pur"), dateSold(d, "date_sold"), depMeth(d, "dep_meth"), cost(d, "cost"),
        salvage(d, "salvage"), currDep(d, "curr_dep"), accumDep(d, "accum_dep"), lifetime(d, "lifetime"),
        lifeleft(d, "lifeleft"), percent(d, "percent"), mode(d, "mode");
    const bool added = !SeekCode(d, facode, a.code);
    if (added) {
        d.appendStart();   // then blank(): blank() first would flush blanks over the current record
        d.blank();
    }
    facode.assign(TrimText(a.code).c_str());
    fatype.assign(TrimText(a.type).c_str());
    descript.assign(a.description.c_str());
    reference.assign(a.reference.c_str());
    datePur.assign(a.datePurchased.c_str());
    dateSold.assign(a.dateSold.c_str());
    depMeth.assign(methodText(a.method));
    cost.assign(money(a.cost).c_str());
    salvage.assign(money(a.salvage).c_str());
    currDep.assign(money(a.monthly).c_str());
    accumDep.assign(money(a.accumulated).c_str());
    lifetime.assignLong(a.life);
    lifeleft.assignLong(a.monthsUsed);
    percent.assignLong(a.percent);
    mode.assign(a.mode.c_str());
    if (added) d.append(); else d.flush();
    d.close();
    return added;
}

} // namespace

bool SaveAsset(cb4sqlite::Code4& code, const Asset& a)
{
    cb4sqlite::Transaction tx(code);
    const bool added = writeAsset(code, a);
    tx.commit();
    return added;
}

DeleteTypeResult DeleteAssetType(cb4sqlite::Code4& code, const std::string& type, std::string* usedBy)
{
    const std::string want = TrimText(type);
    {
        cb4sqlite::Query q(code, "SELECT facode FROM gl_fa WHERE _deleted = 0 AND trim(fatype) = ?1 "
                                 "ORDER BY facode, _rowid LIMIT 1");
        q.bindText(1, want);
        if (q.next()) {
            if (usedBy) *usedBy = q.text(0);
            return DeleteTypeResult::InUse;
        }
    }
    Data4 d(&code, "gl_fatyp");
    d.SetIndex("fatype");
    Field4 fatype(d, "fatype");
    const bool deleted = DeleteRecord(d, fatype, want);
    d.close();
    return deleted ? DeleteTypeResult::Deleted : DeleteTypeResult::NotFound;
}

bool ValidateAccount(cb4sqlite::Code4& code, const std::string& account, const std::string& dept)
{
    const std::string acct = TrimText(account);
    if (acct.empty()) return false;
    cb4sqlite::Query q(code, "SELECT count(*) FROM glaccts WHERE _deleted = 0 AND trim(account) = ?1 "
                             "AND coalesce(trim(dept), '') = ?2");
    q.bindText(1, acct);
    q.bindText(2, TrimText(dept));
    return q.next() && q.integer(0) > 0;
}

// ---- checking -------------------------------------------------------------

std::optional<AssetProblem> CheckAsset(cb4sqlite::Code4& code, const Asset& a)
{
    using K = AssetProblem::Kind;
    auto problem = [](AssetField f, K k) { return std::optional<AssetProblem>(AssetProblem{f, k}); };
    if (TrimText(a.code).empty()) return problem(AssetField::Code, K::Required);
    if (TrimText(a.type).empty()) return problem(AssetField::Type, K::Required);
    if (!FindAssetType(code, a.type)) return problem(AssetField::Type, K::NoSuchType);
    if (TrimText(a.description).empty()) return problem(AssetField::Description, K::Required);
    if (a.datePurchased.size() != 8) return problem(AssetField::Date, K::Required);
    if (a.cost <= 0) return problem(AssetField::Cost, K::Required);
    if (a.life <= 0) return problem(AssetField::Life, K::Required);
    if (a.method == DepMethod::Declining && a.percent <= 0) return problem(AssetField::Rate, K::Required);
    if (a.salvage > a.cost) return problem(AssetField::Salvage, K::SalvageOverCost);
    if (a.monthsUsed > a.life) return problem(AssetField::MonthsUsed, K::UsedOverLife);
    if (a.accumulated > Depreciable(a)) return problem(AssetField::Accumulated, K::AccumOverDepreciable);
    return std::nullopt;
}

bool HasDepreciation(cb4sqlite::Code4& code, const std::string& facode)
{
    cb4sqlite::Query q(code, "SELECT count(*) FROM gl_fadep WHERE _deleted = 0 AND trim(facode) = ?1");
    q.bindText(1, TrimText(facode));
    return q.next() && q.integer(0) > 0;
}

// ---- the arithmetic -----------------------------------------------------

std::int64_t Depreciable(const Asset& a) { return a.cost - a.salvage; }
std::int64_t BookValue(const Asset& a) { return a.cost - a.accumulated; }
std::int64_t Remaining(const Asset& a) { return std::max<std::int64_t>(0, a.cost - a.salvage - a.accumulated); }

std::int64_t MonthlyAmount(const Asset& a, std::int64_t startOfYearBookValue)
{
    if (a.method == DepMethod::Declining) {
        if (a.percent <= 0) return 0;
        return divRound(startOfYearBookValue * a.percent, 100 * 12);
    }
    if (a.life <= 0) return 0;
    return divRound(Depreciable(a), a.life);
}

std::int64_t DueAmount(const Asset& a, std::int64_t startOfYearBookValue)
{
    const std::int64_t left = Remaining(a);
    if (a.disposed() || left <= 0) return 0;
    if (a.method == DepMethod::Straight && a.life > 0 && a.monthsUsed + 1 >= a.life) return left;   // the last month
    return std::min(MonthlyAmount(a, startOfYearBookValue), left);
}

// ---- history ------------------------------------------------------------

std::string FiscalYearEnd(cb4sqlite::Code4& code)
{
    if (!hasTable(code, "gl_pds")) return {};
    cb4sqlite::Query q(code, "SELECT periode FROM gl_pds WHERE period = 12 ORDER BY _rowid LIMIT 1");
    return q.next() ? q.text(0) : std::string();
}

std::vector<DepreciationRow> LoadDepreciation(cb4sqlite::Code4& code, const std::string& facode)
{
    std::vector<DepreciationRow> out;
    cb4sqlite::Query q(code, "SELECT facode, year_end, period, batchno, amount FROM gl_fadep "
                             "WHERE _deleted = 0 AND trim(facode) = ?1 ORDER BY year_end, period, _rowid");
    q.bindText(1, TrimText(facode));
    while (q.next())
        out.push_back({q.text(0), q.text(1), static_cast<int>(q.integer(2)), static_cast<long>(q.integer(3)),
                       rpt::ToCents(q.number(4))});
    return out;
}

void AppendDepreciation(cb4sqlite::Code4& code, const DepreciationRow& row)
{
    Data4 d(&code, "gl_fadep");
    Field4 facode(d, "facode"), yearEnd(d, "year_end"), period(d, "period"), batchno(d, "batchno"), amount(d, "amount");
    d.appendStart();   // then blank(), as Data4::appendBlank
    d.blank();
    facode.assign(TrimText(row.facode).c_str());
    yearEnd.assign(row.yearEnd.c_str());
    period.assignLong(row.period);
    batchno.assignLong(row.batchno);
    amount.assign(money(row.amount).c_str());
    d.append();
    d.close();
}

std::int64_t StartOfYearBookValue(cb4sqlite::Code4& code, const Asset& a, const std::string& yearEnd)
{
    std::int64_t thisYear = 0;
    for (const DepreciationRow& r : LoadDepreciation(code, a.code))
        if (r.yearEnd == yearEnd) thisYear += r.amount;
    return a.cost - (a.accumulated - thisYear);
}

// ---- Disposal -------------------------------------------------------------

std::int64_t GainOnSale(const Asset& a, std::int64_t salePrice) { return salePrice - BookValue(a); }

DisposeResult DisposeAsset(cb4sqlite::Code4& code, const std::string& facode, const std::string& dateSold)
{
    std::optional<Asset> a = FindAsset(code, facode);
    if (!a) return DisposeResult::NotFound;
    if (a->disposed()) return DisposeResult::AlreadyDisposed;
    a->dateSold = dateSold;
    a->mode = "D";
    SaveAsset(code, *a);
    return DisposeResult::Disposed;
}

DeleteAssetResult DeleteAsset(cb4sqlite::Code4& code, const std::string& facode)
{
    if (!FindAsset(code, facode)) return DeleteAssetResult::NotFound;
    if (HasDepreciation(code, facode)) return DeleteAssetResult::HasHistory;
    Data4 d(&code, "gl_fa");
    d.SetIndex("facode");
    Field4 f(d, "facode");
    const bool deleted = DeleteRecord(d, f, TrimText(facode));
    d.close();
    return deleted ? DeleteAssetResult::Deleted : DeleteAssetResult::NotFound;
}

// ---- the run --------------------------------------------------------------

namespace {

std::string periodEnd(cb4sqlite::Code4& code, int period)
{
    if (!hasTable(code, "gl_pds")) return {};
    cb4sqlite::Query q(code, "SELECT periode FROM gl_pds WHERE period = ?1 ORDER BY _rowid LIMIT 1");
    q.bindInt(1, period);
    return q.next() ? q.text(0) : std::string();
}

bool voucherExists(cb4sqlite::Code4& code, long batchno)
{
    cb4sqlite::Query q(code, "SELECT count(*) FROM gl_btch WHERE _deleted = 0 AND batchno = ?1");
    q.bindInt(1, batchno);
    return q.next() && q.integer(0) > 0;
}

double toDouble(std::int64_t cents) { return static_cast<double>(cents) / 100.0; }

} // namespace

RunPlan PlanRun(cb4sqlite::Code4& code, int period)
{
    RunPlan plan;
    plan.period = period;
    plan.periodEnd = periodEnd(code, period);
    plan.yearEnd = FiscalYearEnd(code);

    // D5: this period of this year already has a run whose voucher still
    // exists, or one booked without a voucher. Rows whose voucher has gone
    // without UndoRun are ignored.
    {
        cb4sqlite::Query q(code, "SELECT DISTINCT batchno FROM gl_fadep WHERE _deleted = 0 AND year_end = ?1 "
                                 "AND period = ?2 ORDER BY batchno");
        q.bindText(1, plan.yearEnd);
        q.bindInt(2, period);
        std::vector<long> runs;
        while (q.next()) runs.push_back(static_cast<long>(q.integer(0)));
        for (long b : runs)
            if (b == 0 || voucherExists(code, b)) {
                plan.alreadyRun = true;
                plan.existingVoucher = b;
                break;
            }
    }

    for (const Asset& a : LoadAssets(code)) {
        if (a.disposed()) {
            plan.skipped.push_back({a, SkipReason::Disposed});
            continue;
        }
        const std::optional<AssetType> type = FindAssetType(code, a.type);
        if (!type) {
            plan.skipped.push_back({a, SkipReason::NoType});
            continue;
        }
        if (!ValidateAccount(code, type->expense.account, type->expense.dept) ||
            !ValidateAccount(code, type->accum.account, type->accum.dept)) {
            plan.skipped.push_back({a, SkipReason::BadAccount});
            continue;
        }
        if (!plan.periodEnd.empty() && !a.datePurchased.empty() && a.datePurchased > plan.periodEnd) {
            plan.skipped.push_back({a, SkipReason::NotYetBought});
            continue;
        }
        const std::int64_t start = StartOfYearBookValue(code, a, plan.yearEnd);
        const std::int64_t due = DueAmount(a, start);
        if (due <= 0) {
            plan.skipped.push_back({a, SkipReason::FullyDepreciated});
            continue;
        }
        plan.due.push_back({a, *type, due, MonthlyAmount(a, start)});
        plan.total += due;
    }
    return plan;
}

RunResult RunDepreciation(cb4sqlite::Code4& code, int period, long iniBatchno)
{
    RunResult result;
    const RunPlan plan = PlanRun(code, period);
    if (plan.alreadyRun) {
        result.status = RunStatus::AlreadyRun;
        result.batchno = plan.existingVoucher;
        return result;
    }
    if (plan.periodEnd.empty()) {
        result.status = RunStatus::NoPeriodEnd;
        return result;
    }
    if (plan.due.empty()) return result;   // NothingDue

    cb4sqlite::Transaction tx(code);
    VoucherHeaders h(code);
    VoucherLines l(code);
    if (!h.isValid() || !l.isValid()) throw std::runtime_error("gl: a voucher table is missing");
    VoucherHeader header;
    header.mode = voucher_mode::kOpen;
    header.description = "Monthly Depreciation Voucher";   // string 8118
    header.date = plan.periodEnd;                           // D10
    header.period = period;
    long number = 0;
    int line = 0;
    for (const RunItem& item : plan.due) {
        VoucherLine v;
        v.period = period;
        v.date = plan.periodEnd;
        v.reference = TrimText(item.asset.code);
        v.line = ++line;   // Dr the expense
        v.account = item.type.expense.account;
        v.dept = item.type.expense.dept;
        v.descript = item.type.expense.descript;
        v.debit = toDouble(item.amount);
        v.credit = 0;
        number = SaveLine(h, l, number, header, v, iniBatchno);
        v.line = ++line;   // Cr the accumulated depreciation
        v.account = item.type.accum.account;
        v.dept = item.type.accum.dept;
        v.descript = item.type.accum.descript;
        v.debit = 0;
        v.credit = toDouble(item.amount);
        number = SaveLine(h, l, number, header, v, iniBatchno);

        Asset a = item.asset;
        a.accumulated += item.amount;
        a.monthsUsed += 1;
        a.monthly = item.monthly;
        writeAsset(code, a);
        AppendDepreciation(code, {a.code, plan.yearEnd, period, number, item.amount});
    }
    h.close();
    l.close();
    tx.commit();
    result.status = RunStatus::Created;
    result.batchno = number;
    result.assets = static_cast<int>(plan.due.size());
    result.total = plan.total;
    return result;
}

int UndoRun(cb4sqlite::Code4& code, long batchno)
{
    if (!hasTable(code, "gl_fadep")) return 0;
    std::vector<DepreciationRow> rows;
    {
        cb4sqlite::Query q(code, "SELECT facode, year_end, period, batchno, amount FROM gl_fadep "
                                 "WHERE _deleted = 0 AND batchno = ?1 ORDER BY _rowid");
        q.bindInt(1, batchno);
        while (q.next())
            rows.push_back({q.text(0), q.text(1), static_cast<int>(q.integer(2)), static_cast<long>(q.integer(3)),
                            rpt::ToCents(q.number(4))});
    }
    if (rows.empty()) return 0;
    for (const DepreciationRow& r : rows) {
        std::optional<Asset> a = FindAsset(code, r.facode);
        if (!a) continue;
        a->accumulated = std::max<std::int64_t>(0, a->accumulated - r.amount);
        a->monthsUsed = std::max(0, a->monthsUsed - 1);
        writeAsset(code, *a);
    }
    Data4 d(&code, "gl_fadep");
    Field4 b(d, "batchno");
    for (d.top(); !d.eof(); d.skip())
        if (!d.deleted() && static_cast<long>(b.num()) == batchno) d.deleteRec();
    d.pack();
    d.close();
    return static_cast<int>(rows.size());
}
