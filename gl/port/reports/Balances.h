//
// Balances.h - balances and movements from glaccts, and the posted lines in
// gl_dist (spec #3 5.1, 5.3). Portable: gl_tests checks it on the fixture.
//
// Amounts are signed, debit positive, in cents. For every account
//
//     CURR_BAL = OPENBAL + PERIOD1 + ... + PERIOD12
//
// (spec #3 2.5): OPENBAL is the balance at the start of the fiscal year and
// PERIODn the net movement of fiscal period n. So the balance at the end of
// period p is OPENBAL + PERIOD1..p. Reports check the identity for the
// accounts they read and list the ones that fail (ConsistencyNote), so a
// report is never silently wrong.
//
#pragma once

#include "cb4sqlite.h"

#include <array>
#include <cstdint>
#include <map>
#include <string>
#include <utility>
#include <vector>

struct AccountBalances {
    std::string account, dept, descript, type;   // trimmed
    std::int64_t openbal = 0, currBal = 0;
    std::array<std::int64_t, 12> period{}, previous{}, budget{};

    // The balance at the end of period p (0: the start of the year, OPENBAL).
    std::int64_t balanceAt(int p) const;
    // PERIODa..b, PREVIOUSa..b, BUDGETa..b (1-12; an empty range is 0).
    std::int64_t movement(int a, int b) const;
    std::int64_t previousMovement(int a, int b) const;
    std::int64_t budgetMovement(int a, int b) const;
    // CURR_BAL = OPENBAL + the periods.
    bool consistent() const;
    // True when there is nothing to show: no balance at all, no period movement.
    bool empty() const;
};

// The accounts not deleted, in account + dept order.
std::vector<AccountBalances> LoadBalances(cb4sqlite::Code4& code);

// "These accounts' balances don't agree with their period amounts: 101000,
// 410001-10." for the accounts that fail the identity; "" when all agree.
std::string ConsistencyNote(const std::vector<AccountBalances>& accounts);

// ---- gl_dist ---------------------------------------------------------------

using DistKey = std::pair<std::string, std::string>;   // account, dept (trimmed)

struct DistTotals {
    std::int64_t debit = 0, credit = 0;
};

// Debits and credits per account + dept of the gl_dist rows (not deleted)
// with PERIOD in from..to, of every TYPE.
std::map<DistKey, DistTotals> DistTotalsByAccount(cb4sqlite::Code4& code, int from, int to);

struct DistLine {
    std::string account, dept;
    int period = 0;
    std::string date;        // yyyymmdd
    long batchno = 0;        // the voucher
    std::string type;        // "GL" for a blank TYPE, else AP, AR...
    std::string reference, descript, cheque;
    std::int64_t debit = 0, credit = 0;
};

// The gl_dist rows (not deleted) with PERIOD in from..to, in account, dept,
// period, date, voucher order.
std::vector<DistLine> DistLines(cb4sqlite::Code4& code, int from, int to);
