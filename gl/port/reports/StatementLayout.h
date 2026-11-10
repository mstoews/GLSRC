//
// StatementLayout.h - the financial statements' layout (spec #3 6, D15):
// which accounts make up which printed line. Portable: gl_tests checks it.
//
// Two tables the port creates in the company database the first time they
// are needed, and fills with the default of spec 6.3:
//
//   gl_fsline   STATEMENT C(2) "IS"/"BS"/"CF", LINENO N(5), KIND C(1),
//               CAPTION C(60), SIGN N(2), FORMULA C(120), INDENT N(1),
//               UPDATED C(16)
//   gl_fsrange  STATEMENT C(2), LINENO N(5), ACCT_FROM C(6), ACCT_TO C(6)
//
// Line kinds: H heading, A account line (its ranges' accounts), T total (a
// formula over earlier lines, "50-60", "90+100+110"), N net income, O opening
// cash, E closing cash, S blank line.
//
#pragma once

#include "Balances.h"

#include "cb4sqlite.h"

#include <optional>
#include <string>
#include <utility>
#include <vector>

inline constexpr char kIncomeStatement[] = "IS";
inline constexpr char kBalanceSheet[] = "BS";
inline constexpr char kCashFlow[] = "CF";

struct AccountRange {
    std::string from, to;   // 6-digit account numbers, inclusive
};

struct LayoutLine {
    int lineNo = 0;
    char kind = 'S';           // H A T N O E S
    std::string caption;
    int sign = 1;              // A, O, E: +1 or -1
    std::string formula;       // T
    int indent = 0;            // 0-3
    std::vector<AccountRange> ranges;   // A, O, E
    std::string updated;       // "" for the default; else when it was last saved
};

// The default layout of a statement (spec 6.3), in line order.
std::vector<LayoutLine> DefaultLayout(const std::string& statement);

// Creates gl_fsline / gl_fsrange with the default layout if they don't exist
// yet (one transaction). Does nothing when they do.
void EnsureLayoutTables(cb4sqlite::Code4& code);

// A statement's lines, in line order (after EnsureLayoutTables).
std::vector<LayoutLine> LoadLayout(cb4sqlite::Code4& code, const std::string& statement);

// Replaces a statement's lines (one transaction; Spec #2 D1). `stamp`, e.g.
// "2026-11-11 14:05", goes in UPDATED on every line saved.
void SaveLayout(cb4sqlite::Code4& code, const std::string& statement, const std::vector<LayoutLine>& lines,
                const std::string& stamp);

// SaveLayout of the default, with UPDATED blank.
void ResetLayout(cb4sqlite::Code4& code, const std::string& statement);

// "Layout: default", or "Layout: modified <latest UPDATED>".
std::string LayoutDescription(const std::vector<LayoutLine>& lines);

// A formula's terms: (+1 or -1, line number). nullopt if it doesn't parse.
std::optional<std::vector<std::pair<int, int>>> ParseFormula(const std::string& formula);

// True if the account (trimmed) lies in one of the ranges.
bool AccountInRanges(const std::string& account, const std::vector<AccountRange>& ranges);

// The accounts a statement covers: IS the income accounts (TYPE I); BS the
// balance-sheet accounts (B and R); CF the B and R accounts not in the cash
// ranges (the O / E lines).
bool StatementCovers(const std::string& statement, const AccountBalances& a, const std::vector<LayoutLine>& lines);

struct LayoutProblem {
    bool error = false;   // false: a warning
    std::string text;
};

// Spec 6.5. Errors: bad formulas and ranges, kinds without what they need,
// duplicate line numbers, CF's O / E lines. Warnings: covered accounts on no
// account line or on two, and (CF) a cash account on an account line.
std::vector<LayoutProblem> CheckLayout(const std::string& statement, const std::vector<LayoutLine>& lines,
                                       const std::vector<AccountBalances>& accounts);
