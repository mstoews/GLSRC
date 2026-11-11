//
// Statements.cpp - see Statements.h.
//
#include "Statements.h"

#include "Balances.h"
#include "StatementLayout.h"

#include "report/Format.h"

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

namespace {

enum class Measure { Movement, Previous, Budget, Balance, CashFlow, Variance };

struct Col {
    std::string header, header2;
    Measure measure = Measure::Movement;
    int from = 1, to = 1;              // periods; Balance: at the end of `to`
    int minuend = -1, subtrahend = -1;   // Variance: column minuend less column subtrahend
};

using Values = std::vector<std::int64_t>;

std::int64_t accountValue(const AccountBalances& a, const Col& c)
{
    switch (c.measure) {
    case Measure::Movement: return a.movement(c.from, c.to);
    case Measure::Previous: return a.previousMovement(c.from, c.to);
    case Measure::Budget:   return a.budgetMovement(c.from, c.to);
    case Measure::Balance:  return a.balanceAt(c.to);
    case Measure::CashFlow: return -a.movement(c.from, c.to);
    default:                return 0;
    }
}

// Minus the income accounts' movement over the column's periods.
std::int64_t netIncome(const std::vector<const AccountBalances*>& accounts, const Col& c)
{
    std::int64_t sum = 0;
    for (const AccountBalances* a : accounts) {
        if (a->type != "I") continue;
        switch (c.measure) {
        case Measure::Movement: sum += a->movement(c.from, c.to); break;
        case Measure::Previous: sum += a->previousMovement(c.from, c.to); break;
        case Measure::Budget:   sum += a->budgetMovement(c.from, c.to); break;
        case Measure::Balance:  sum += a->movement(1, c.to); break;   // year to date; 0 at the start
        case Measure::CashFlow: sum += a->movement(c.from, c.to); break;
        default: break;
        }
    }
    return -sum;
}

void fillVariance(Values& v, const std::vector<Col>& cols)
{
    for (size_t c = 0; c < cols.size(); ++c)
        if (cols[c].measure == Measure::Variance)
            v[c] = v[static_cast<size_t>(cols[c].minuend)] - v[static_cast<size_t>(cols[c].subtrahend)];
}

bool anyNonZero(const Values& v)
{
    return std::any_of(v.begin(), v.end(), [](std::int64_t x) { return x != 0; });
}

std::string accountLabel(const AccountBalances& a)
{
    return (a.dept.empty() ? a.account : a.account + "-" + a.dept) + "  " + a.descript;
}

struct Statement {
    std::string code;     // IS / BS / CF
    std::string title;
    int uSign = -1;       // the sign "Unclassified accounts" prints with
};

struct Evaluated {
    std::vector<Values> lines;   // per layout line
    Values unclassified;
    std::vector<const AccountBalances*> unclassifiedAccounts;
    int lastTotal = -1;          // index of the last T line
    std::vector<std::string> formulaErrors;
};

Evaluated evaluate(const Statement& st, const std::vector<LayoutLine>& layout,
                   const std::vector<const AccountBalances*>& accounts, const std::vector<Col>& cols)
{
    const size_t n = cols.size();
    Evaluated e;
    e.lines.assign(layout.size(), Values(n, 0));
    e.unclassified.assign(n, 0);
    for (size_t i = 0; i < layout.size(); ++i)
        if (layout[i].kind == 'T') e.lastTotal = static_cast<int>(i);

    // Covered accounts on no account line.
    for (const AccountBalances* a : accounts) {
        if (!StatementCovers(st.code, *a, layout)) continue;
        const bool onLine = std::any_of(layout.begin(), layout.end(), [&](const LayoutLine& l) {
            return l.kind == 'A' && AccountInRanges(a->account, l.ranges);
        });
        if (onLine) continue;
        e.unclassifiedAccounts.push_back(a);
        for (size_t c = 0; c < n; ++c) e.unclassified[c] += accountValue(*a, cols[c]) * st.uSign;
    }
    fillVariance(e.unclassified, cols);

    for (size_t i = 0; i < layout.size(); ++i) {
        const LayoutLine& l = layout[i];
        Values& v = e.lines[i];
        switch (l.kind) {
        case 'A':
            for (const AccountBalances* a : accounts)
                if (StatementCovers(st.code, *a, layout) && AccountInRanges(a->account, l.ranges))
                    for (size_t c = 0; c < n; ++c) v[c] += accountValue(*a, cols[c]) * l.sign;
            break;
        case 'N':
            for (size_t c = 0; c < n; ++c) v[c] = netIncome(accounts, cols[c]);
            break;
        case 'O':
        case 'E':
            for (const AccountBalances* a : accounts)
                if ((a->type == "B" || a->type == "R") && AccountInRanges(a->account, l.ranges))
                    for (size_t c = 0; c < n; ++c)
                        v[c] += (l.kind == 'O' ? a->balanceAt(cols[c].from - 1) : a->balanceAt(cols[c].to)) * l.sign;
            break;
        case 'T': {
            const auto terms = ParseFormula(l.formula);
            if (!terms) {
                e.formulaErrors.push_back("line " + std::to_string(l.lineNo));
                break;
            }
            for (const auto& term : *terms) {
                bool found = false;
                for (size_t j = 0; j < i; ++j)
                    if (layout[j].lineNo == term.second) {
                        for (size_t c = 0; c < n; ++c) v[c] += term.first * e.lines[j][c];
                        found = true;
                        break;
                    }
                if (!found) e.formulaErrors.push_back("line " + std::to_string(l.lineNo));
            }
            if (static_cast<int>(i) == e.lastTotal)
                for (size_t c = 0; c < n; ++c) v[c] += e.unclassified[c];
            break;
        }
        default:
            break;
        }
        fillVariance(v, cols);
    }
    return e;
}

std::vector<rpt::Cell> amountCells(const std::string& caption, const Values& v)
{
    std::vector<rpt::Cell> cells{rpt::Cell::Text(caption)};
    for (std::int64_t x : v) cells.push_back(rpt::Cell::Amount(x));
    return cells;
}

std::string deptDescription(cb4sqlite::Code4& code, const std::string& dept)
{
    cb4sqlite::Query has(code, "SELECT count(*) FROM sqlite_master WHERE type = 'table' AND name = 'gldept'");
    if (!has.next() || has.integer(0) == 0) return {};
    cb4sqlite::Query q(code, "SELECT descript FROM gldept WHERE _deleted = 0 AND trim(depart) = ?1");
    q.bindText(1, dept);
    return q.next() ? q.text(0) : std::string();
}

std::string periodEnd(const ReportContext& ctx, int p)
{
    const FiscalPeriod* fp = ctx.period(p);
    if (fp && !fp->endDate.empty()) return rpt::FormatDateLong(fp->endDate);
    std::string s = "period " + std::to_string(p);
    if (fp && !fp->description.empty()) s += " (" + fp->description + ")";
    return s;
}

std::string periodLabel(const ReportContext& ctx, int p)
{
    const FiscalPeriod* fp = ctx.period(p);
    return fp && !fp->description.empty() ? fp->description : "period " + std::to_string(p);
}

// The statement as report rows, with its checks and notes.
rpt::Report build(cb4sqlite::Code4& code, const ReportContext& ctx, const StatementOptions& opt, const Statement& st,
                  const std::vector<Col>& cols, std::vector<std::string> subtitles, rpt::Orientation orientation)
{
    EnsureLayoutTables(code);
    const std::vector<LayoutLine> layout = opt.layout ? *opt.layout : LoadLayout(code, st.code);
    const std::vector<AccountBalances> all = LoadBalances(code);
    std::vector<const AccountBalances*> accounts;
    for (const AccountBalances& a : all)
        if (opt.dept.empty() || a.dept == opt.dept) accounts.push_back(&a);

    const Evaluated e = evaluate(st, layout, accounts, cols);
    const size_t n = cols.size();

    rpt::Report r;
    r.company = ctx.company;
    r.printedOn = ctx.printedOn;
    r.title = st.title;
    r.subtitles = std::move(subtitles);
    r.footerNote = st.title + " - " + (opt.layout ? std::string("Layout: not saved (preview)") : LayoutDescription(layout));
    r.orientation = orientation;
    r.negatives = rpt::NegativeStyle::Parentheses;
    r.columns.push_back({"", n > 3 ? 3.4 : 4.0});
    for (const Col& c : cols) r.columns.push_back({c.header, 1.3, rpt::CellType::Amount, c.header2});

    // Which lines show: account and net-income lines that aren't zero (or
    // all, with the option); totals, blanks and the opening / closing cash
    // always (F5 reconciles them); a heading when an account or net-income
    // line under it shows.
    std::vector<bool> shown(layout.size(), false);
    for (size_t i = 0; i < layout.size(); ++i) {
        const char k = layout[i].kind;
        if (k == 'T' || k == 'S' || k == 'O' || k == 'E') shown[i] = true;
        else if (k == 'A' || k == 'N') shown[i] = opt.showZeroLines || anyNonZero(e.lines[i]);
    }
    for (size_t i = 0; i < layout.size(); ++i) {
        if (layout[i].kind != 'H') continue;
        for (size_t j = i + 1; j < layout.size() && layout[j].kind != 'H'; ++j)
            if (shown[j] && (layout[j].kind == 'A' || layout[j].kind == 'N')) shown[i] = true;
    }
    int finalLine = -1;
    for (size_t i = 0; i < layout.size(); ++i)
        if (layout[i].kind == 'T' || layout[i].kind == 'E') finalLine = static_cast<int>(i);

    // F5: opening cash + net change = closing cash, per column.
    Values unreconciled(n, 0);
    if (st.code == kCashFlow && e.lastTotal >= 0) {
        const auto o = std::find_if(layout.begin(), layout.end(), [](const LayoutLine& l) { return l.kind == 'O'; });
        const auto c = std::find_if(layout.begin(), layout.end(), [](const LayoutLine& l) { return l.kind == 'E'; });
        if (o != layout.end() && c != layout.end()) {
            const Values& open = e.lines[static_cast<size_t>(o - layout.begin())];
            const Values& close = e.lines[static_cast<size_t>(c - layout.begin())];
            const Values& change = e.lines[static_cast<size_t>(e.lastTotal)];
            for (size_t k = 0; k < n; ++k) unreconciled[k] = close[k] - open[k] - change[k];
        }
    }
    const bool showUnclassified = !e.unclassifiedAccounts.empty() && (opt.showZeroLines || anyNonZero(e.unclassified));

    for (size_t i = 0; i < layout.size(); ++i) {
        const LayoutLine& l = layout[i];
        if (static_cast<int>(i) == e.lastTotal && showUnclassified) {
            r.rows.push_back(rpt::Row::Detail(amountCells("Unclassified accounts", e.unclassified), 1));
            if (opt.showAccounts)
                for (const AccountBalances* a : e.unclassifiedAccounts) {
                    Values v(n, 0);
                    for (size_t c = 0; c < n; ++c) v[c] = accountValue(*a, cols[c]) * st.uSign;
                    fillVariance(v, cols);
                    if (opt.showZeroLines || anyNonZero(v)) r.rows.push_back(rpt::Row::Detail(amountCells(accountLabel(*a), v), 2));
                }
        }
        if (!shown[i]) continue;
        switch (l.kind) {
        case 'H':
            if (!r.rows.empty()) r.rows.push_back(rpt::Row::Blank());   // space between sections
            r.rows.push_back(rpt::Row::Heading(l.caption, l.indent));
            break;
        case 'S':
            r.rows.push_back(rpt::Row::Blank());
            break;
        case 'T':
            if (static_cast<int>(i) == finalLine)
                r.rows.push_back(rpt::Row::Total(amountCells(l.caption, e.lines[i]), true));
            else
                r.rows.push_back(rpt::Row::Subtotal(amountCells(l.caption, e.lines[i]), l.indent));
            if (static_cast<int>(i) == e.lastTotal && anyNonZero(unreconciled))
                r.rows.push_back(rpt::Row::Detail(amountCells("Unreconciled difference", unreconciled), l.indent));
            break;
        case 'E':
            if (static_cast<int>(i) == finalLine) {
                r.rows.push_back(rpt::Row::Total(amountCells(l.caption, e.lines[i]), true));
                break;
            }
            [[fallthrough]];
        default:   // A, N, O
            r.rows.push_back(rpt::Row::Detail(amountCells(l.caption, e.lines[i]), l.indent));
            if (l.kind == 'A' && opt.showAccounts)
                for (const AccountBalances* a : accounts) {
                    if (!StatementCovers(st.code, *a, layout) || !AccountInRanges(a->account, l.ranges)) continue;
                    Values v(n, 0);
                    for (size_t c = 0; c < n; ++c) v[c] = accountValue(*a, cols[c]) * l.sign;
                    fillVariance(v, cols);
                    if (opt.showZeroLines || anyNonZero(v))
                        r.rows.push_back(rpt::Row::Detail(amountCells(accountLabel(*a), v), l.indent + 1));
                }
            break;
        }
    }

    // Notes: the balance sheet's own check, layout problems, consistency.
    std::vector<std::string> notes;
    if (st.code == kBalanceSheet) {
        for (size_t c = 0; c < n; ++c) {
            std::int64_t diff = 0;   // assets less liabilities and equity
            for (const AccountBalances* a : accounts) {
                if (a->type == "I") diff += a->movement(1, cols[c].to);
                else diff += a->balanceAt(cols[c].to);
            }
            if (diff != 0)
                notes.push_back("The balance sheet doesn't balance (" + cols[c].header + " " + cols[c].header2 +
                                "): the ledger is out of balance by " + rpt::FormatAmount(diff) +
                                " (debits less credits), so total assets and total liabilities and equity differ.");
        }
    }
    if (!e.formulaErrors.empty()) {
        std::string list;
        for (const std::string& s : e.formulaErrors) list += (list.empty() ? "" : ", ") + s;
        notes.push_back("Layout error: a total's formula refers to a line that isn't an earlier line (" + list +
                        "); those terms count as zero.");
    }
    for (const LayoutProblem& p : CheckLayout(st.code, layout, all))
        if (!p.error || e.formulaErrors.empty()) notes.push_back((p.error ? "Layout error: " : "") + p.text);
    if (const std::string c = ConsistencyNote(all); !c.empty()) notes.push_back(c);
    if (!notes.empty()) r.rows.push_back(rpt::Row::Blank());
    for (const std::string& note : notes) r.rows.push_back(rpt::Row::Note(note));
    return r;
}

int periodOf(const ReportContext& ctx, const StatementOptions& opt)
{
    return std::clamp(opt.period > 0 ? opt.period : ctx.currentPeriod, 1, 12);
}

} // namespace

rpt::Report IncomeStatement(cb4sqlite::Code4& code, const ReportContext& ctx, const StatementOptions& opt)
{
    const int p = periodOf(ctx, opt);
    std::vector<Col> cols = {
        {"Month", periodLabel(ctx, p), Measure::Movement, p, p},
        {"Year", "to date", Measure::Movement, 1, p},
    };
    if (opt.comparePrior) cols.push_back({"Prior year", "to date", Measure::Previous, 1, p});
    if (opt.compareBudget) {
        cols.push_back({"Budget", "to date", Measure::Budget, 1, p});
        Col v{"Variance", "to budget", Measure::Variance};
        v.minuend = 1;
        v.subtrahend = static_cast<int>(cols.size()) - 1;
        cols.push_back(v);
    }
    std::vector<std::string> subtitles{"For the month and year to date ended " + periodEnd(ctx, p)};
    if (!opt.dept.empty()) {
        const std::string d = deptDescription(code, opt.dept);
        subtitles.push_back("Department " + opt.dept + (d.empty() ? std::string() : " " + d));
    }
    const bool wide = opt.comparePrior || opt.compareBudget;
    return build(code, ctx, opt, {kIncomeStatement, "Income Statement", -1}, cols, subtitles,
                 wide ? rpt::Orientation::Landscape : rpt::Orientation::Portrait);
}

rpt::Report BalanceSheet(cb4sqlite::Code4& code, const ReportContext& ctx, const StatementOptions& opt)
{
    const int p = periodOf(ctx, opt);
    StatementOptions all = opt;
    all.dept.clear();   // a balance sheet is for the whole company
    const std::vector<Col> cols = {
        {"End of", periodLabel(ctx, p), Measure::Balance, 1, p},
        {"Start of", "year", Measure::Balance, 1, 0},
    };
    return build(code, ctx, all, {kBalanceSheet, "Balance Sheet", -1}, cols, {"As at " + periodEnd(ctx, p)},
                 rpt::Orientation::Portrait);
}

rpt::Report CashFlowStatement(cb4sqlite::Code4& code, const ReportContext& ctx, const StatementOptions& opt)
{
    const int p = periodOf(ctx, opt);
    StatementOptions all = opt;
    all.dept.clear();
    const std::vector<Col> cols = {
        {"Month", periodLabel(ctx, p), Measure::CashFlow, p, p},
        {"Year", "to date", Measure::CashFlow, 1, p},
    };
    return build(code, ctx, all, {kCashFlow, "Cash Flow Statement", 1}, cols,
                 {"For the month and year to date ended " + periodEnd(ctx, p)}, rpt::Orientation::Portrait);
}
