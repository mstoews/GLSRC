//
// SampleReport.cpp - see SampleReport.h.
//
#include "SampleReport.h"

#include "report/Format.h"

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>

namespace {

struct Account {
    std::string account, dept, descript, type;
    std::int64_t balance = 0, opening = 0;   // cents
};

std::vector<Account> loadAccounts(cb4sqlite::Code4& code, bool includeZeros)
{
    std::vector<Account> out;
    cb4sqlite::Query q(code, "SELECT account, dept, descript, type, curr_bal, openbal FROM glaccts "
                             "WHERE _deleted = 0 ORDER BY type, account, dept");
    while (q.next()) {
        Account a{q.text(0), q.text(1), q.text(2), q.text(3), rpt::ToCents(q.number(4)), rpt::ToCents(q.number(5))};
        if (!includeZeros && a.balance == 0) continue;
        out.push_back(std::move(a));
    }
    return out;
}

} // namespace

rpt::Report BuildSampleReport(cb4sqlite::Code4& code, const ReportContext& ctx, const SampleOptions& opt)
{
    rpt::Report r;
    r.company = ctx.company;
    r.title = "Sample Report - Chart of Accounts";
    if (const FiscalPeriod* p = ctx.periodOpen ? ctx.period(ctx.currentPeriod) : nullptr)
        r.subtitles.push_back("Current period " + std::to_string(p->number) + ", " + p->description +
                              (p->endDate.empty() ? std::string() : ", ending " + rpt::FormatDateLong(p->endDate)));
    else
        r.subtitles.push_back("No period is open");
    r.printedOn = ctx.printedOn;
    r.footerNote = "Sample report (report engine test)";
    r.orientation = opt.landscape ? rpt::Orientation::Landscape : rpt::Orientation::Portrait;
    r.columns = {
        {"Account", 1.0},
        {"Dept", 0.6},
        {"Description", 3.2},
        {"Type", 0.5},
        {"Opening", 1.4, rpt::CellType::Amount, "balance", true},
        {"Current", 1.4, rpt::CellType::Amount, "balance"},
    };

    const std::vector<Account> accounts = loadAccounts(code, opt.includeZeros);
    const int repeat = std::clamp(opt.repeat, 1, 50);
    std::int64_t grandOpening = 0, grandBalance = 0;
    for (int copy = 1; copy <= repeat; ++copy) {
        if (repeat > 1) r.rows.push_back(rpt::Row::Heading("Copy " + std::to_string(copy) + " of " + std::to_string(repeat)));
        size_t i = 0;
        while (i < accounts.size()) {
            const std::string type = accounts[i].type;
            r.rows.push_back(rpt::Row::GroupHeader("Type " + (type.empty() ? std::string("(blank)") : type)));
            std::int64_t opening = 0, balance = 0;
            for (; i < accounts.size() && accounts[i].type == type; ++i) {
                const Account& a = accounts[i];
                r.rows.push_back(rpt::Row::Detail({rpt::Cell::Text(a.account), rpt::Cell::Text(a.dept),
                                                   rpt::Cell::Text(a.descript), rpt::Cell::Text(a.type),
                                                   rpt::Cell::Amount(a.opening), rpt::Cell::Amount(a.balance)},
                                                  1));
                opening += a.opening;
                balance += a.balance;
            }
            r.rows.push_back(rpt::Row::GroupFooter({rpt::Cell::Text("Total type " + type), {}, {}, {},
                                                    rpt::Cell::Amount(opening), rpt::Cell::Amount(balance)}));
            grandOpening += opening;
            grandBalance += balance;
        }
        r.rows.push_back(rpt::Row::Blank());
    }
    r.rows.push_back(rpt::Row::Total({rpt::Cell::Text("Total"), {}, {}, {}, rpt::Cell::Amount(grandOpening),
                                      rpt::Cell::Amount(grandBalance)},
                                     true));
    r.rows.push_back(rpt::Row::Blank());
    r.rows.push_back(rpt::Row::Note(
        "This is the report engine's test report. It lists " + std::to_string(accounts.size()) + " account" +
        (accounts.size() == 1 ? "" : "s") + (repeat > 1 ? " " + std::to_string(repeat) + " times" : std::string()) +
        ". In a balanced ledger the current balances add up to zero, so the Current total reads 0.00."));
    return r;
}
