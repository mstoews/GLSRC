//
// TrialBalances.cpp - see TrialBalances.h.
//
#include "TrialBalances.h"

#include "Balances.h"

#include "report/Format.h"

#include <algorithm>
#include <cstdint>
#include <string>

namespace {

int clampPeriod(int p) { return std::clamp(p, 1, 12); }

// "7 (Oct 26)"; "7" without a description.
std::string periodName(const ReportContext& ctx, int p)
{
    std::string s = std::to_string(p);
    const FiscalPeriod* fp = ctx.period(p);
    if (fp && !fp->description.empty()) s += " (" + fp->description + ")";
    return s;
}

// "October 31, 2026"; "" without an end date.
std::string periodEnd(const ReportContext& ctx, int p)
{
    const FiscalPeriod* fp = ctx.period(p);
    return fp && !fp->endDate.empty() ? rpt::FormatDateLong(fp->endDate) : std::string();
}

// F1: "As at October 31, 2026, the end of period 7 (Oct 26)".
std::string asAt(const ReportContext& ctx, int p)
{
    const std::string end = periodEnd(ctx, p);
    return (end.empty() ? "As at" : "As at " + end + ",") + " the end of period " + periodName(ctx, p);
}

// F2: "For period 7 (Oct 26), ending October 31, 2026";
// "For periods 1 (Apr 26) to 7 (Oct 26), ending October 31, 2026".
std::string forPeriods(const ReportContext& ctx, int from, int to)
{
    std::string s = from == to ? "For period " + periodName(ctx, to)
                               : "For periods " + periodName(ctx, from) + " to " + periodName(ctx, to);
    const std::string end = periodEnd(ctx, to);
    if (!end.empty()) s += ", ending " + end;
    return s;
}

void addConsistencyNote(rpt::Report& r, const std::vector<AccountBalances>& accounts)
{
    const std::string note = ConsistencyNote(accounts);
    if (note.empty()) return;
    r.rows.push_back(rpt::Row::Blank());
    r.rows.push_back(rpt::Row::Note(note));
}

} // namespace

rpt::Report TrialBalance(cb4sqlite::Code4& code, const ReportContext& ctx, const TrialBalanceOptions& opt)
{
    const int p = clampPeriod(opt.period > 0 ? opt.period : ctx.currentPeriod);
    rpt::Report r;
    r.company = ctx.company;
    r.printedOn = ctx.printedOn;
    r.title = "Trial Balance";
    r.subtitles.push_back(asAt(ctx, p));
    r.footerNote = "GL Trial Balance";
    r.columns = {
        {"Account", 1.0},
        {"Dept", 0.5},
        {"Description", 3.6},
        {"Debit", 1.4, rpt::CellType::Amount, {}, true},
        {"Credit", 1.4, rpt::CellType::Amount, {}, true},
    };

    const std::vector<AccountBalances> accounts = LoadBalances(code);
    std::int64_t debit = 0, credit = 0;
    for (const AccountBalances& a : accounts) {
        const std::int64_t bal = a.balanceAt(p);
        if (bal == 0 && !opt.includeZeros) continue;
        r.rows.push_back(rpt::Row::Detail({rpt::Cell::Text(a.account), rpt::Cell::Text(a.dept),
                                           rpt::Cell::Text(a.descript),
                                           bal > 0 ? rpt::Cell::Amount(bal) : rpt::Cell::Empty(),
                                           bal < 0 ? rpt::Cell::Amount(-bal) : rpt::Cell::Empty()}));
        if (bal > 0) debit += bal;
        else credit -= bal;
    }
    r.rows.push_back(rpt::Row::Total({rpt::Cell::Text("Total"), {}, {}, rpt::Cell::Amount(debit),
                                      rpt::Cell::Amount(credit)},
                                     true));
    if (debit != credit) {
        r.rows.push_back(rpt::Row::Blank());
        r.rows.push_back(rpt::Row::Note("The debit and credit totals differ by " +
                                        rpt::FormatAmount(debit > credit ? debit - credit : credit - debit) +
                                        ": the ledger doesn't balance."));
    }
    addConsistencyNote(r, accounts);
    return r;
}

rpt::Report DistributionTrialBalance(cb4sqlite::Code4& code, const ReportContext& ctx,
                                     const DistTrialBalanceOptions& opt)
{
    int from = clampPeriod(opt.from > 0 ? opt.from : ctx.currentPeriod);
    int to = clampPeriod(opt.to > 0 ? opt.to : ctx.currentPeriod);
    if (from > to) std::swap(from, to);

    rpt::Report r;
    r.company = ctx.company;
    r.printedOn = ctx.printedOn;
    r.title = "Distribution Trial Balance";
    r.subtitles.push_back(forPeriods(ctx, from, to));
    r.footerNote = "GL Distribution Trial Balance";
    r.orientation = opt.voucherLines ? rpt::Orientation::Landscape : rpt::Orientation::Portrait;
    // Landscape (voucher lines) gives the description the extra width: the
    // lines' text (voucher, type, reference, description, cheque) is long.
    // With voucher lines the Account column also holds the lines' dates,
    // indented one level: it needs room for "10/15/2026".
    r.columns = {
        {"Account", opt.voucherLines ? 1.45 : 1.0},
        {"Dept", 0.5},
        {"Description", opt.voucherLines ? 5.2 : 3.6},
        {"Opening", 1.25, rpt::CellType::Amount, "balance"},
        {"Debits", 1.25, rpt::CellType::Amount},
        {"Credits", 1.25, rpt::CellType::Amount},
        {"Closing", 1.25, rpt::CellType::Amount, "balance"},
        {"", 0.2},   // "*"
    };

    const std::vector<AccountBalances> accounts = LoadBalances(code);
    const std::map<DistKey, DistTotals> dist = DistTotalsByAccount(code, from, to);
    std::vector<DistLine> lines;
    if (opt.voucherLines) lines = DistLines(code, from, to);
    size_t nextLine = 0;

    std::int64_t tOpen = 0, tDebit = 0, tCredit = 0, tClose = 0;
    int marked = 0;
    for (const AccountBalances& a : accounts) {
        const std::int64_t opening = a.balanceAt(from - 1);
        const std::int64_t moved = a.movement(from, to);
        const std::int64_t closing = opening + moved;
        const auto it = dist.find({a.account, a.dept});
        const DistTotals t = it != dist.end() ? it->second : DistTotals{};
        const bool active = opening != 0 || closing != 0 || moved != 0 || t.debit != 0 || t.credit != 0;
        // Skip this account's voucher lines too, if the row isn't shown.
        auto ownLines = [&](bool show) {
            while (nextLine < lines.size() &&
                   std::make_pair(lines[nextLine].account, lines[nextLine].dept) < std::make_pair(a.account, a.dept))
                ++nextLine;
            while (nextLine < lines.size() && lines[nextLine].account == a.account && lines[nextLine].dept == a.dept) {
                const DistLine& l = lines[nextLine++];
                if (!show) continue;
                std::string text = "No. " + std::to_string(l.batchno) + " " + l.type;
                if (!l.reference.empty()) text += "  " + l.reference;
                if (!l.descript.empty() && l.descript != l.reference) text += " - " + l.descript;
                if (!l.cheque.empty()) text += "  chq " + l.cheque;
                r.rows.push_back(rpt::Row::Detail({rpt::Cell::Date(l.date), rpt::Cell::Text("P" + std::to_string(l.period)),
                                                   rpt::Cell::Text(text), {},
                                                   l.debit ? rpt::Cell::Amount(l.debit) : rpt::Cell::Empty(),
                                                   l.credit ? rpt::Cell::Amount(l.credit) : rpt::Cell::Empty(), {}, {}},
                                                  1));
            }
        };
        if (!active && !opt.includeInactive) {
            ownLines(false);
            continue;
        }
        const bool mismatch = t.debit - t.credit != moved;
        if (mismatch) ++marked;
        r.rows.push_back(rpt::Row::Detail({rpt::Cell::Text(a.account), rpt::Cell::Text(a.dept),
                                           rpt::Cell::Text(a.descript), rpt::Cell::Amount(opening),
                                           rpt::Cell::Amount(t.debit), rpt::Cell::Amount(t.credit),
                                           rpt::Cell::Amount(closing),
                                           mismatch ? rpt::Cell::Text("*") : rpt::Cell::Empty()}));
        ownLines(true);
        tOpen += opening;
        tDebit += t.debit;
        tCredit += t.credit;
        tClose += closing;
    }
    r.rows.push_back(rpt::Row::Total({rpt::Cell::Text("Total"), {}, {}, rpt::Cell::Amount(tOpen),
                                      rpt::Cell::Amount(tDebit), rpt::Cell::Amount(tCredit),
                                      rpt::Cell::Amount(tClose), {}},
                                     true));
    if (marked > 0) {
        r.rows.push_back(rpt::Row::Blank());
        r.rows.push_back(rpt::Row::Note(
            "* The debits less credits of the voucher lines don't equal the account's movement over these periods "
            "(the closing less the opening balance): some of the movement has no voucher lines, or lines are "
            "missing. " +
            std::to_string(marked) + (marked == 1 ? " account is" : " accounts are") + " marked."));
    }
    addConsistencyNote(r, accounts);
    return r;
}
