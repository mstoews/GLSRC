//
// Balances.cpp - see Balances.h.
//
#include "Balances.h"

#include "report/Format.h"

#include <algorithm>
#include <string>

namespace {

std::int64_t sumRange(const std::array<std::int64_t, 12>& v, int a, int b)
{
    a = std::max(a, 1);
    b = std::min(b, 12);
    std::int64_t s = 0;
    for (int i = a; i <= b; ++i) s += v[static_cast<size_t>(i - 1)];
    return s;
}

std::string columnList(const char* prefix)
{
    std::string s;
    for (int i = 1; i <= 12; ++i) s += std::string(", ") + prefix + std::to_string(i);
    return s;
}

// gl_dist.PERIOD is text, " 7  " in the real rows.
const char* kDistWhere = "WHERE _deleted = 0 AND CAST(trim(period) AS INTEGER) BETWEEN ?1 AND ?2";

} // namespace

std::int64_t AccountBalances::balanceAt(int p) const { return openbal + sumRange(period, 1, p); }
std::int64_t AccountBalances::movement(int a, int b) const { return sumRange(period, a, b); }
std::int64_t AccountBalances::previousMovement(int a, int b) const { return sumRange(previous, a, b); }
std::int64_t AccountBalances::budgetMovement(int a, int b) const { return sumRange(budget, a, b); }
bool AccountBalances::consistent() const { return currBal == balanceAt(12); }

bool AccountBalances::empty() const
{
    if (openbal != 0 || currBal != 0) return false;
    return std::all_of(period.begin(), period.end(), [](std::int64_t v) { return v == 0; });
}

std::vector<AccountBalances> LoadBalances(cb4sqlite::Code4& code)
{
    const std::string sql = "SELECT account, dept, descript, type, openbal, curr_bal" + columnList("period") +
                            columnList("previous") + columnList("budget") +
                            " FROM glaccts WHERE _deleted = 0 ORDER BY account, dept";
    cb4sqlite::Query q(code, sql);
    std::vector<AccountBalances> out;
    while (q.next()) {
        AccountBalances a;
        a.account = q.text(0);
        a.dept = q.text(1);
        a.descript = q.text(2);
        a.type = q.text(3);
        a.openbal = rpt::ToCents(q.number(4));
        a.currBal = rpt::ToCents(q.number(5));
        for (int i = 0; i < 12; ++i) {
            a.period[static_cast<size_t>(i)] = rpt::ToCents(q.number(6 + i));
            a.previous[static_cast<size_t>(i)] = rpt::ToCents(q.number(18 + i));
            a.budget[static_cast<size_t>(i)] = rpt::ToCents(q.number(30 + i));
        }
        out.push_back(std::move(a));
    }
    return out;
}

std::string ConsistencyNote(const std::vector<AccountBalances>& accounts)
{
    std::string list;
    for (const AccountBalances& a : accounts) {
        if (a.consistent()) continue;
        if (!list.empty()) list += ", ";
        list += a.account + (a.dept.empty() ? std::string() : "-" + a.dept);
    }
    if (list.empty()) return {};
    return "These accounts' balances don't agree with their period amounts (current balance is not the opening "
           "balance plus the periods): " + list + ". Their figures in this report come from the period amounts.";
}

std::map<DistKey, DistTotals> DistTotalsByAccount(cb4sqlite::Code4& code, int from, int to)
{
    cb4sqlite::Query q(code, std::string("SELECT account, dept, sum(debit), sum(credit) FROM gl_dist ") + kDistWhere +
                                 " GROUP BY trim(account), trim(dept)");
    q.bindInt(1, from).bindInt(2, to);
    std::map<DistKey, DistTotals> out;
    while (q.next()) {
        DistTotals& t = out[{q.text(0), q.text(1)}];
        t.debit += rpt::ToCents(q.number(2));
        t.credit += rpt::ToCents(q.number(3));
    }
    return out;
}

std::vector<DistLine> DistLines(cb4sqlite::Code4& code, int from, int to)
{
    cb4sqlite::Query q(code, std::string("SELECT account, dept, CAST(trim(period) AS INTEGER), date, batchno, type, "
                                         "reference, descript, cheque, debit, credit FROM gl_dist ") +
                                 kDistWhere + " ORDER BY account, dept, 3, date, batchno, _rowid");
    q.bindInt(1, from).bindInt(2, to);
    std::vector<DistLine> out;
    while (q.next()) {
        DistLine l;
        l.account = q.text(0);
        l.dept = q.text(1);
        l.period = static_cast<int>(q.integer(2));
        l.date = q.text(3);
        l.batchno = static_cast<long>(q.integer(4));
        l.type = q.text(5);
        if (l.type.empty()) l.type = "GL";
        l.reference = q.text(6);
        l.descript = q.text(7);
        l.cheque = q.text(8);
        l.debit = rpt::ToCents(q.number(9));
        l.credit = rpt::ToCents(q.number(10));
        out.push_back(std::move(l));
    }
    return out;
}
