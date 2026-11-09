//
// Accounts.cpp - see Accounts.h.
//
#include "Accounts.h"

#include "Company.h"   // UpdateUserTime

#include <array>
#include <cmath>
#include <cstdlib>

extern char szUser[11];   // Globals.h, which needs <Windows.h>; this file stays portable

AccountsTable::AccountsTable(Code4& code) : Tglaccts(&code)
{
    if (isValid()) return;
    Field4** all[] = {&accounts, &dept,      &descript,  &fdescript,  &type,       &curr_bal,  &category,
                      &budget1,  &budget2,   &budget3,   &budget4,    &budget5,    &budget6,   &budget7,
                      &budget8,  &budget9,   &budget10,  &budget11,   &budget12,   &period1,   &period2,
                      &period3,  &period4,   &period5,   &period6,    &period7,    &period8,   &period9,
                      &period10, &period11,  &period12,  &openbal,    &previous1,  &previous2, &previous3,
                      &previous4, &previous5, &previous6, &previous7, &previous8,  &previous9, &previous10,
                      &previous11, &previous12};
    for (Field4** f : all) *f = nullptr;
}

AccountsTable::~AccountsTable()
{
    for (Field4* f : {accounts,   dept,       descript,   fdescript,  type,      curr_bal,  category,
                      budget1,    budget2,    budget3,    budget4,    budget5,   budget6,   budget7,
                      budget8,    budget9,    budget10,   budget11,   budget12,  period1,   period2,
                      period3,    period4,    period5,    period6,    period7,   period8,   period9,
                      period10,   period11,   period12,   openbal,    previous1, previous2, previous3,
                      previous4,  previous5,  previous6,  previous7,  previous8, previous9, previous10,
                      previous11, previous12})
        delete f;
}

std::string TrimText(const std::string& s)
{
    const auto first = s.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return {};
    const auto last = s.find_last_not_of(" \t\r\n");
    return s.substr(first, last - first + 1);
}

std::string AccountKey(const std::string& account, const std::string& dept, unsigned accountWidth)
{
    std::string key = TrimText(account);
    if (key.size() < accountWidth) key.append(accountWidth - key.size(), ' ');
    return key + TrimText(dept);
}

bool SeekAccount(Tglaccts& gl, const std::string& account, const std::string& dept)
{
    if (!gl.isValid() || TrimText(account).empty()) return false;
    // SetIndex("accounts") - already active per Tglaccts ctor; retained from original for defensive symmetry
    gl.SetIndex("accounts");
    if (gl.seek(AccountKey(account, dept, gl.accounts->len()).c_str()) != r4success) return false;
    return TrimText(gl.accounts->str()) == TrimText(account) && TrimText(gl.dept->str()) == TrimText(dept);
}

bool HasActivity(Tglaccts& gl)
{
    for (Field4* f : {gl.period1, gl.period2, gl.period3, gl.period4, gl.period5, gl.period6, gl.period7,
                      gl.period8, gl.period9, gl.period10, gl.period11, gl.period12, gl.previous12})
        if (f->num() != 0) return true;
    return false;
}

namespace {

// tbFloat text ("1,234.50") -> "1234.50"; blank -> "0".
std::string number(const std::string& text)
{
    std::string digits;
    for (char c : text)
        if (c != ',' && c != ' ') digits += c;
    return digits.empty() ? "0" : digits;
}

} // namespace

void AppendAccount(Tglaccts& gl, const AccountInput& in)
{
    gl.appendStart();
    Field4 field;
    for (int n = 1; n <= gl.numFields(); ++n) {
        field.init(gl, n);
        if (field.type() == r4num || field.type() == r4float) field.assignDouble(0);
    }
    gl.accounts->assign(in.account.c_str());
    gl.dept->assign(in.dept.c_str());
    gl.descript->assign(in.descript.c_str());
    gl.fdescript->assign(in.fdescript.c_str());
    gl.type->assign(in.type.c_str());
    gl.curr_bal->assign(number(in.currBal).c_str());
    gl.previous12->assign(number(in.openBal).c_str());
    gl.openbal->assign(number(in.openBal).c_str());
    UpdateUserTime(gl, szUser);   // glaccts has no update/user/time fields: a no-op, as on the real tables
    gl.append();
}

void ModifyAccount(Tglaccts& gl, const AccountInput& in)
{
    gl.descript->assign(in.descript.c_str());
    gl.fdescript->assign(in.fdescript.c_str());
    gl.previous12->assign(number(in.openBal).c_str());
    gl.curr_bal->assign(number(in.currBal).c_str());
    if (in.type != "N") gl.type->assign(in.type.c_str());
    UpdateUserTime(gl, szUser);
    gl.flush();
    gl.unlock();
}

void DeleteAccount(Tglaccts& gl)
{
    gl.deleteRec();
    gl.pack();
}

double ParseAmount(const std::string& text)
{
    return std::strtod(number(text).c_str(), nullptr);
}

namespace {

enum class Column { Budget, Period, Previous };

std::array<Field4*, 12> monthFields(Tglaccts& gl, Column c)
{
    switch (c) {
    case Column::Budget:
        return {gl.budget1, gl.budget2, gl.budget3, gl.budget4,  gl.budget5,  gl.budget6,
                gl.budget7, gl.budget8, gl.budget9, gl.budget10, gl.budget11, gl.budget12};
    case Column::Period:
        return {gl.period1, gl.period2, gl.period3, gl.period4,  gl.period5,  gl.period6,
                gl.period7, gl.period8, gl.period9, gl.period10, gl.period11, gl.period12};
    default:
        return {gl.previous1, gl.previous2, gl.previous3, gl.previous4,  gl.previous5,  gl.previous6,
                gl.previous7, gl.previous8, gl.previous9, gl.previous10, gl.previous11, gl.previous12};
    }
}

double value(const Field4* f) { return f ? f->num() : 0.0; }

} // namespace

std::vector<BudgetRow> ReadBudget(Tglaccts& gl)
{
    // TGLBudget::Update opened fields 33-44, 8-19 and 21-32 by number; in
    // the real layout those are budget1-12, period1-12 and previous1-12.
    const auto budget = monthFields(gl, Column::Budget), period = monthFields(gl, Column::Period),
               previous = monthFields(gl, Column::Previous);
    std::vector<BudgetRow> rows(12);
    for (size_t i = 0; i < 12; ++i) rows[i] = {value(budget[i]), value(period[i]), value(previous[i])};
    return rows;
}

void SaveBudget(Tglaccts& gl, const std::vector<double>& budget)
{
    const auto f = monthFields(gl, Column::Budget);
    for (size_t i = 0; i < 12 && i < budget.size(); ++i)
        if (f[i]) f[i]->assignDouble(std::round(budget[i] * 100.0) / 100.0);
    UpdateUserTime(gl, szUser);
    gl.flush();
}
