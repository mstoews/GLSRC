//
// StatementLayout.cpp - see StatementLayout.h.
//
#include "StatementLayout.h"

#include <algorithm>
#include <cctype>
#include <map>
#include <set>
#include <stdexcept>
#include <string>

namespace {

// ---- the default (spec 6.3) -------------------------------------------------

LayoutLine H(int no, const char* caption)
{
    LayoutLine l;
    l.lineNo = no;
    l.kind = 'H';
    l.caption = caption;
    return l;
}

LayoutLine A(int no, const char* caption, int sign, std::vector<AccountRange> ranges, int indent = 1)
{
    LayoutLine l;
    l.lineNo = no;
    l.kind = 'A';
    l.caption = caption;
    l.sign = sign;
    l.ranges = std::move(ranges);
    l.indent = indent;
    return l;
}

LayoutLine T(int no, const char* caption, const char* formula)
{
    LayoutLine l;
    l.lineNo = no;
    l.kind = 'T';
    l.caption = caption;
    l.formula = formula;
    return l;
}

LayoutLine K(int no, char kind, const char* caption, std::vector<AccountRange> ranges = {}, int indent = 1)
{
    LayoutLine l;
    l.lineNo = no;
    l.kind = kind;
    l.caption = caption;
    l.ranges = std::move(ranges);
    l.indent = indent;
    return l;
}

AccountRange R(const char* from, const char* to) { return {from, to}; }

std::vector<LayoutLine> defaultIncomeStatement()
{
    return {
        H(10, "Revenue"),
        A(20, "Gross sales", -1, {R("410000", "419999")}),
        A(30, "Sales returns", -1, {R("420000", "429999")}),
        A(40, "Sales allowances", -1, {R("430000", "499999")}),
        T(50, "Net sales", "20+30+40"),
        A(60, "Cost of sales", 1, {R("500000", "599999")}),
        T(70, "Gross profit", "50-60"),
        H(80, "Operating expenses"),
        A(90, "Variable selling", 1, {R("700000", "709999")}),
        A(100, "Product development", 1, {R("710000", "719999")}),
        A(110, "Promotion and advertising", 1, {R("720000", "729999")}),
        A(120, "Personnel", 1, {R("730000", "739999")}),
        A(130, "Travel and meetings", 1, {R("740000", "749999")}),
        A(140, "General and administrative", 1, {R("750000", "759999")}),
        A(150, "Other operating expenses", 1, {R("600000", "699999"), R("760000", "799999")}),
        T(160, "Total operating expenses", "90+100+110+120+130+140+150"),
        T(170, "Operating income", "70-160"),
        A(180, "Non-operating income", -1, {R("800000", "809999")}, 0),
        A(190, "Non-operating expenses", 1, {R("810000", "819999")}, 0),
        A(200, "Extraordinary items, net", -1, {R("820000", "899999")}, 0),
        T(210, "Income before income tax", "170+180-190+200"),
        A(220, "Income tax", 1, {R("900000", "999999")}, 0),
        T(230, "Net income", "210-220"),
    };
}

std::vector<LayoutLine> defaultBalanceSheet()
{
    return {
        H(10, "Assets"),
        A(20, "Cash and bank", 1, {R("100000", "109999")}),
        A(30, "Receivables", 1, {R("110000", "119999")}),
        A(40, "Inventories", 1, {R("120000", "129999")}),
        A(50, "Other current assets", 1, {R("130000", "149999")}),
        T(60, "Total current assets", "20+30+40+50"),
        A(70, "Property and equipment", 1, {R("150000", "159999")}),
        A(80, "Accumulated depreciation", 1, {R("160000", "169999")}),
        A(90, "Other assets", 1, {R("170000", "199999")}),
        T(100, "Total assets", "60+70+80+90"),
        H(110, "Liabilities"),
        A(120, "Short-term borrowings", -1, {R("200000", "219999")}),
        A(130, "Accounts payable", -1, {R("220000", "229999")}),
        A(140, "Accrued expenses", -1, {R("230000", "259999")}),
        A(150, "Other current liabilities", -1, {R("260000", "289999")}),
        T(160, "Total current liabilities", "120+130+140+150"),
        A(170, "Long-term liabilities", -1, {R("290000", "299999")}),
        T(180, "Total liabilities", "160+170"),
        H(190, "Equity"),
        A(200, "Share capital", -1, {R("300000", "339999")}),
        A(210, "Retained earnings", -1, {R("340000", "349999")}),
        A(220, "Dividends", -1, {R("350000", "359999")}),
        A(230, "Other equity", -1, {R("360000", "399999")}),
        K(240, 'N', "Net income for the year to date"),
        T(250, "Total equity", "200+210+220+230+240"),
        T(260, "Total liabilities and equity", "180+250"),
    };
}

std::vector<LayoutLine> defaultCashFlow()
{
    return {
        H(10, "Operating activities"),
        K(20, 'N', "Net income"),
        A(30, "Depreciation and amortisation", 1, {R("160000", "169999")}),
        A(40, "(Increase) decrease in receivables", 1, {R("110000", "119999")}),
        A(50, "(Increase) decrease in inventories", 1, {R("120000", "129999")}),
        A(60, "(Increase) decrease in other current assets", 1, {R("130000", "149999")}),
        A(70, "Increase (decrease) in accounts payable", 1, {R("220000", "229999")}),
        A(80, "Increase (decrease) in accrued expenses", 1, {R("230000", "259999")}),
        A(90, "Increase (decrease) in other current liabilities", 1, {R("260000", "289999")}),
        T(100, "Net cash from operating activities", "20+30+40+50+60+70+80+90"),
        H(110, "Investing activities"),
        A(120, "Property and equipment", 1, {R("150000", "159999")}),
        A(130, "Other assets", 1, {R("170000", "199999")}),
        T(140, "Net cash used in investing activities", "120+130"),
        H(150, "Financing activities"),
        A(160, "Borrowings", 1, {R("200000", "219999")}),
        A(170, "Long-term liabilities", 1, {R("290000", "299999")}),
        A(180, "Share capital", 1, {R("300000", "339999")}),
        A(190, "Retained earnings, dividends and other equity", 1, {R("340000", "399999")}),
        T(200, "Net cash from financing activities", "160+170+180+190"),
        T(210, "Net increase (decrease) in cash", "100+140+200"),
        K(220, 'O', "Cash at beginning of period", {R("100000", "109999")}, 0),
        K(230, 'E', "Cash at end of period", {R("100000", "109999")}, 0),
    };
}

// ---- the tables --------------------------------------------------------------

bool hasTable(cb4sqlite::Code4& code, const char* name)
{
    cb4sqlite::Query q(code, "SELECT count(*) FROM sqlite_master WHERE type = 'table' AND name = ?1");
    q.bindText(1, name);
    return q.next() && q.integer(0) > 0;
}

void must(int rc, const char* what)
{
    if (rc != r4success) throw std::runtime_error(std::string("statement layout: ") + what + " failed");
}

// Appends a statement's lines and ranges (inside the caller's transaction).
void appendLines(cb4sqlite::Code4& code, const std::string& statement, const std::vector<LayoutLine>& lines,
                 const std::string& stamp)
{
    Data4 l(&code, "gl_fsline");
    Data4 r(&code, "gl_fsrange");
    if (!l.isValid() || !r.isValid()) throw std::runtime_error("statement layout: the layout tables are missing");
    Field4 lStmt(l, "statement"), lNo(l, "lineno"), lKind(l, "kind"), lCaption(l, "caption"), lSign(l, "sign"),
        lFormula(l, "formula"), lIndent(l, "indent"), lUpdated(l, "updated");
    Field4 rStmt(r, "statement"), rNo(r, "lineno"), rFrom(r, "acct_from"), rTo(r, "acct_to");
    for (const LayoutLine& line : lines) {
        must(l.appendStart(), "adding to gl_fsline");
        l.blank();
        lStmt.assign(statement.c_str());
        lNo.assignLong(line.lineNo);
        const char kind[2] = {line.kind, '\0'};
        lKind.assign(kind);
        lCaption.assign(line.caption.c_str());
        lSign.assignLong(line.sign);
        lFormula.assign(line.formula.c_str());
        lIndent.assignLong(line.indent);
        lUpdated.assign(stamp.c_str());
        must(l.append(), "adding to gl_fsline");
        for (const AccountRange& range : line.ranges) {
            must(r.appendStart(), "adding to gl_fsrange");
            r.blank();
            rStmt.assign(statement.c_str());
            rNo.assignLong(line.lineNo);
            rFrom.assign(range.from.c_str());
            rTo.assign(range.to.c_str());
            must(r.append(), "adding to gl_fsrange");
        }
    }
    l.close();
    r.close();
}

void deleteStatement(cb4sqlite::Code4& code, const char* table, const std::string& statement)
{
    Data4 d(&code, table);
    if (!d.isValid()) throw std::runtime_error(std::string("statement layout: ") + table + " is missing");
    Field4 stmt(d, "statement");
    bool any = false;
    for (d.top(); !d.eof(); d.skip()) {
        std::string s = stmt.str();
        s.erase(s.find_last_not_of(' ') + 1);
        if (s == statement) {
            d.deleteRec();
            any = true;
        }
    }
    if (any) must(d.pack(), std::string("packing ").append(table).c_str());
    d.close();
}

bool validRangeBound(const std::string& s)
{
    return s.size() == 6 && std::all_of(s.begin(), s.end(), [](char c) { return c >= '0' && c <= '9'; });
}

std::string accountName(const AccountBalances& a)
{
    return a.dept.empty() ? a.account : a.account + "-" + a.dept;
}

std::string joinNames(const std::vector<std::string>& names)
{
    std::string s;
    for (const std::string& n : names) s += (s.empty() ? "" : ", ") + n;
    return s;
}

const char* statementName(const std::string& statement)
{
    if (statement == kIncomeStatement) return "income statement";
    if (statement == kBalanceSheet) return "balance sheet";
    return "cash flow statement";
}

} // namespace

std::vector<LayoutLine> DefaultLayout(const std::string& statement)
{
    if (statement == kIncomeStatement) return defaultIncomeStatement();
    if (statement == kBalanceSheet) return defaultBalanceSheet();
    if (statement == kCashFlow) return defaultCashFlow();
    return {};
}

void EnsureLayoutTables(cb4sqlite::Code4& code)
{
    if (hasTable(code, "gl_fsline") && hasTable(code, "gl_fsrange")) return;
    cb4sqlite::Transaction tx(code);
    if (!hasTable(code, "gl_fsline")) {
        cb4sqlite::createTable(code, "gl_fsline",
                               {{"STATEMENT", 'C', 2}, {"LINENO", 'N', 5}, {"KIND", 'C', 1}, {"CAPTION", 'C', 60},
                                {"SIGN", 'N', 2}, {"FORMULA", 'C', 120}, {"INDENT", 'N', 1}, {"UPDATED", 'C', 16}});
        cb4sqlite::createTag(code, "gl_fsline", "line", "statement || printf('%5d', lineno)");
    }
    if (!hasTable(code, "gl_fsrange")) {
        cb4sqlite::createTable(code, "gl_fsrange",
                               {{"STATEMENT", 'C', 2}, {"LINENO", 'N', 5}, {"ACCT_FROM", 'C', 6}, {"ACCT_TO", 'C', 6}});
        cb4sqlite::createTag(code, "gl_fsrange", "line", "statement || printf('%5d', lineno)");
    }
    for (const char* s : {kIncomeStatement, kBalanceSheet, kCashFlow}) appendLines(code, s, DefaultLayout(s), "");
    tx.commit();
}

std::vector<LayoutLine> LoadLayout(cb4sqlite::Code4& code, const std::string& statement)
{
    std::vector<LayoutLine> lines;
    {
        cb4sqlite::Query q(code, "SELECT lineno, kind, caption, sign, formula, indent, updated FROM gl_fsline "
                                 "WHERE _deleted = 0 AND trim(statement) = ?1 ORDER BY lineno, _rowid");
        q.bindText(1, statement);
        while (q.next()) {
            LayoutLine l;
            l.lineNo = static_cast<int>(q.integer(0));
            const std::string kind = q.text(1);
            l.kind = kind.empty() ? 'S' : static_cast<char>(std::toupper(static_cast<unsigned char>(kind[0])));
            l.caption = q.text(2);
            l.sign = static_cast<int>(q.integer(3));
            l.formula = q.text(4);
            l.indent = static_cast<int>(q.integer(5));
            l.updated = q.text(6);
            lines.push_back(std::move(l));
        }
    }
    cb4sqlite::Query r(code, "SELECT lineno, acct_from, acct_to FROM gl_fsrange "
                             "WHERE _deleted = 0 AND trim(statement) = ?1 ORDER BY lineno, _rowid");
    r.bindText(1, statement);
    while (r.next()) {
        const int no = static_cast<int>(r.integer(0));
        for (LayoutLine& l : lines)
            if (l.lineNo == no) {
                l.ranges.push_back({r.text(1), r.text(2)});
                break;
            }
    }
    return lines;
}

void SaveLayout(cb4sqlite::Code4& code, const std::string& statement, const std::vector<LayoutLine>& lines,
                const std::string& stamp)
{
    EnsureLayoutTables(code);
    cb4sqlite::Transaction tx(code);
    deleteStatement(code, "gl_fsline", statement);
    deleteStatement(code, "gl_fsrange", statement);
    appendLines(code, statement, lines, stamp);
    tx.commit();
}

void ResetLayout(cb4sqlite::Code4& code, const std::string& statement)
{
    SaveLayout(code, statement, DefaultLayout(statement), "");
}

std::string LayoutDescription(const std::vector<LayoutLine>& lines)
{
    std::string latest;
    for (const LayoutLine& l : lines) latest = std::max(latest, l.updated);
    return latest.empty() ? "Layout: default" : "Layout: modified " + latest;
}

std::optional<std::vector<std::pair<int, int>>> ParseFormula(const std::string& formula)
{
    // number (('+' | '-') number)*, with an optional sign before the first.
    std::vector<std::pair<int, int>> terms;
    int sign = 1;
    bool expectNumber = true, signed_ = false;
    size_t i = 0;
    while (i < formula.size()) {
        const char c = formula[i];
        if (c == ' ') {
            ++i;
        } else if (c == '+' || c == '-') {
            if (expectNumber && (signed_ || !terms.empty())) return std::nullopt;   // two signs in a row
            sign = c == '-' ? -1 : 1;
            signed_ = true;
            expectNumber = true;
            ++i;
        } else if (c >= '0' && c <= '9') {
            if (!expectNumber) return std::nullopt;
            int n = 0;
            while (i < formula.size() && formula[i] >= '0' && formula[i] <= '9') {
                n = n * 10 + (formula[i] - '0');
                if (n > 99999) return std::nullopt;
                ++i;
            }
            terms.push_back({sign, n});
            sign = 1;
            signed_ = false;
            expectNumber = false;
        } else {
            return std::nullopt;
        }
    }
    if (terms.empty() || expectNumber) return std::nullopt;
    return terms;
}

bool AccountInRanges(const std::string& account, const std::vector<AccountRange>& ranges)
{
    for (const AccountRange& r : ranges)
        if (account.size() == r.from.size() && account >= r.from && account <= r.to) return true;
    return false;
}

bool StatementCovers(const std::string& statement, const AccountBalances& a, const std::vector<LayoutLine>& lines)
{
    if (statement == kIncomeStatement) return a.type == "I";
    if (a.type != "B" && a.type != "R") return false;
    if (statement == kBalanceSheet) return true;
    for (const LayoutLine& l : lines)   // CF: not a cash account
        if (l.kind == 'O' && AccountInRanges(a.account, l.ranges)) return false;
    return true;
}

std::vector<LayoutProblem> CheckLayout(const std::string& statement, const std::vector<LayoutLine>& lines,
                                       const std::vector<AccountBalances>& accounts)
{
    std::vector<LayoutProblem> out;
    auto error = [&](const std::string& t) { out.push_back({true, t}); };
    auto warning = [&](const std::string& t) { out.push_back({false, t}); };

    std::set<int> seen;
    const LayoutLine* opening = nullptr;
    const LayoutLine* closing = nullptr;
    int openings = 0, closings = 0;
    for (const LayoutLine& l : lines) {
        const std::string at = "Line " + std::to_string(l.lineNo) + " (" + l.caption + "): ";
        if (!seen.insert(l.lineNo).second) error("Line " + std::to_string(l.lineNo) + " appears twice.");
        switch (l.kind) {
        case 'H': case 'N': case 'S':
            break;
        case 'A': case 'O': case 'E':
            if (l.ranges.empty()) error(at + "it has no account ranges.");
            if (l.sign != 1 && l.sign != -1) error(at + "its sign must be +1 or -1.");
            for (const AccountRange& r : l.ranges) {
                if (!validRangeBound(r.from) || !validRangeBound(r.to))
                    error(at + "the range " + r.from + "-" + r.to + " isn't two 6-digit account numbers.");
                else if (r.from > r.to)
                    error(at + "the range " + r.from + "-" + r.to + " runs backwards.");
            }
            if (l.kind == 'O') { opening = &l; ++openings; }
            if (l.kind == 'E') { closing = &l; ++closings; }
            break;
        case 'T': {
            if (l.formula.empty()) {
                error(at + "a total needs a formula.");
                break;
            }
            const auto terms = ParseFormula(l.formula);
            if (!terms) {
                error(at + "the formula \"" + l.formula + "\" isn't line numbers joined by + and -.");
                break;
            }
            for (const auto& term : *terms) {
                const int n = term.second;
                const bool earlier = std::any_of(lines.begin(), lines.end(), [&](const LayoutLine& o) {
                    return o.lineNo == n && &o < &l;
                });
                if (!earlier) error(at + "its formula refers to line " + std::to_string(n) + ", which isn't an earlier line.");
            }
            break;
        }
        default:
            error(at + "its kind \"" + std::string(1, l.kind) + "\" isn't one of H, A, T, N, O, E, S.");
        }
    }
    if (statement == kCashFlow) {
        if (openings != 1 || closings != 1) {
            error("The cash flow statement needs exactly one opening-cash (O) and one closing-cash (E) line.");
        } else {
            auto key = [](const LayoutLine* l) {
                std::vector<std::pair<std::string, std::string>> k;
                for (const AccountRange& r : l->ranges) k.push_back({r.from, r.to});
                std::sort(k.begin(), k.end());
                return k;
            };
            if (key(opening) != key(closing)) error("The opening- and closing-cash lines have different ranges.");
        }
    }

    // Coverage (warnings).
    std::vector<std::string> none, twice, cashOnLine;
    for (const AccountBalances& a : accounts) {
        int on = 0;
        for (const LayoutLine& l : lines)
            if (l.kind == 'A' && AccountInRanges(a.account, l.ranges)) ++on;
        if (StatementCovers(statement, a, lines)) {
            if (on == 0) none.push_back(accountName(a));
            if (on > 1) twice.push_back(accountName(a));
        } else if (statement == kCashFlow && on > 0 && (a.type == "B" || a.type == "R")) {
            cashOnLine.push_back(accountName(a));   // a cash account on an account line
        }
    }
    const std::string name = statementName(statement);
    if (!none.empty())
        warning("These accounts are on no line of the " + name + " (they print as \"Unclassified accounts\"): " +
                joinNames(none) + ".");
    if (!twice.empty())
        warning("These accounts are on more than one line of the " + name + " (they count on each): " +
                joinNames(twice) + ".");
    if (!cashOnLine.empty())
        warning("These cash accounts are also on an account line of the cash flow statement: " + joinNames(cashOnLine) +
                ".");
    return out;
}
