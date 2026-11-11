//
// VoucherReports.cpp - see VoucherReports.h.
//
#include "VoucherReports.h"

#include "Balances.h"

#include "report/Format.h"

#include <algorithm>
#include <cstdint>
#include <map>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace {

struct Line {
    int journal = 0, period = 0;
    std::string date, reference, descript, cheque, account, dept, medrep, drcode, prod;
    std::int64_t debit = 0, credit = 0;
};

struct Voucher {
    long number = 0;
    std::string mode, description, date, cheque;
    int period = 0;
    std::vector<Line> lines;
    std::int64_t debit() const
    {
        std::int64_t s = 0;
        for (const Line& l : lines) s += l.debit;
        return s;
    }
    std::int64_t credit() const
    {
        std::int64_t s = 0;
        for (const Line& l : lines) s += l.credit;
        return s;
    }
};

bool unposted(const std::string& mode) { return mode == "1" || mode == "3"; }

// The vouchers (and their lines) not deleted, in number order; `only` < 0: all.
std::vector<Voucher> loadVouchers(cb4sqlite::Code4& code, long only = -1)
{
    std::string sql =
        "SELECT h.batchno, h.mode, h.reference, h.period, h.date, h.cheque, "
        "d.journal, d.period, d.date, d.reference, d.descript, d.cheque, d.account, d.dept, "
        "d.medrep, d.drcode, d.prod, d.debit, d.credit, d._rowid "
        "FROM gl_btch h LEFT JOIN gl_btchd d ON d.batchno = h.batchno AND d._deleted = 0 "
        "WHERE h._deleted = 0";
    if (only >= 0) sql += " AND h.batchno = ?1";
    sql += " ORDER BY h.batchno, h._rowid, d.journal, d._rowid";
    cb4sqlite::Query q(code, sql);
    if (only >= 0) q.bindInt(1, only);
    std::vector<Voucher> out;
    while (q.next()) {
        const long number = static_cast<long>(q.integer(0));
        if (out.empty() || out.back().number != number) {
            Voucher v;
            v.number = number;
            v.mode = q.text(1);
            v.description = q.text(2);
            v.period = static_cast<int>(q.integer(3));
            v.date = q.text(4);
            v.cheque = q.text(5);
            out.push_back(std::move(v));
        }
        if (q.isNull(19)) continue;   // a voucher without lines
        Line l;
        l.journal = static_cast<int>(q.integer(6));
        l.period = static_cast<int>(q.integer(7));
        l.date = q.text(8);
        l.reference = q.text(9);
        l.descript = q.text(10);
        l.cheque = q.text(11);
        l.account = q.text(12);
        l.dept = q.text(13);
        l.medrep = q.text(14);
        l.drcode = q.text(15);
        l.prod = q.text(16);
        l.debit = rpt::ToCents(q.number(17));
        l.credit = rpt::ToCents(q.number(18));
        out.back().lines.push_back(std::move(l));
    }
    return out;
}

rpt::Report frame(const ReportContext& ctx, std::string title, std::string footer)
{
    rpt::Report r;
    r.company = ctx.company;
    r.printedOn = ctx.printedOn;
    r.title = std::move(title);
    r.footerNote = std::move(footer);
    return r;
}

rpt::Cell amountOrEmpty(std::int64_t cents) { return cents ? rpt::Cell::Amount(cents) : rpt::Cell::Empty(); }

// The slip layout (6074, D7).
rpt::Report slips(const ReportContext& ctx, const std::vector<Voucher>& vouchers, std::string footer)
{
    rpt::Report r = frame(ctx, "General Ledger Batch Entry Slip", std::move(footer));
    r.slipMode = true;
    r.columns = {
        {"No.", 0.5, rpt::CellType::Integer},
        {"Pd", 0.4, rpt::CellType::Integer},
        {"Account", 1.0},
        {"Dept", 0.5},
        {"Account Description", 3.0},
        {"Cheque", 1.1},
        {"Debit", 1.3, rpt::CellType::Amount},
        {"Credit", 1.3, rpt::CellType::Amount},
    };
    for (const Voucher& v : vouchers) {
        r.rows.push_back(rpt::Row::Slip({{"Slip Number", std::to_string(v.number)},
                                         {"Date of Entry", rpt::FormatDate(v.date)},
                                         {"Slip Description", v.description},
                                         {"Period", std::to_string(v.period)},
                                         {"Cheque", v.cheque},
                                         {"Status", VoucherStatus(v.mode)}}));
        for (const Line& l : v.lines)
            r.rows.push_back(rpt::Row::Detail({rpt::Cell::Integer(l.journal), rpt::Cell::Integer(l.period),
                                               rpt::Cell::Text(l.account), rpt::Cell::Text(l.dept),
                                               rpt::Cell::Text(l.descript), rpt::Cell::Text(l.cheque),
                                               amountOrEmpty(l.debit), amountOrEmpty(l.credit)}));
        r.rows.push_back(rpt::Row::Total({{}, {}, {}, {}, rpt::Cell::Text("Total Batch Entry"), {},
                                          rpt::Cell::Amount(v.debit()), rpt::Cell::Amount(v.credit())},
                                         true));
    }
    return r;
}

// The detail layout (8082, 8136).
rpt::Report detail(const ReportContext& ctx, const std::vector<Voucher>& vouchers, std::string title,
                   std::string footer)
{
    rpt::Report r = frame(ctx, std::move(title), std::move(footer));
    r.orientation = rpt::Orientation::Landscape;
    r.columns = {
        {"No.", 0.45, rpt::CellType::Integer},
        {"Pd", 0.35, rpt::CellType::Integer},
        {"Account", 0.9},
        {"Dept", 0.45},
        {"Description", 2.6},
        {"Cheque", 0.8},
        {"Date", 1.0, rpt::CellType::Date},
        {"Dr. Code", 0.7},
        {"Med Rep.", 0.7},
        {"Prd.", 0.6},
        {"Debit", 1.2, rpt::CellType::Amount},
        {"Credit", 1.2, rpt::CellType::Amount},
    };
    std::int64_t debit = 0, credit = 0;
    for (const Voucher& v : vouchers) {
        std::string head = "Slip " + std::to_string(v.number);
        if (!v.description.empty()) head += "  " + v.description;
        head += "  (" + VoucherStatus(v.mode) + ", period " + std::to_string(v.period) + ")";
        r.rows.push_back(rpt::Row::GroupHeader(head));
        for (const Line& l : v.lines)
            r.rows.push_back(rpt::Row::Detail(
                {rpt::Cell::Integer(l.journal), rpt::Cell::Integer(l.period), rpt::Cell::Text(l.account),
                 rpt::Cell::Text(l.dept), rpt::Cell::Text(l.descript), rpt::Cell::Text(l.cheque),
                 l.date.empty() ? rpt::Cell::Empty() : rpt::Cell::Date(l.date), rpt::Cell::Text(l.drcode),
                 rpt::Cell::Text(l.medrep), rpt::Cell::Text(l.prod), amountOrEmpty(l.debit), amountOrEmpty(l.credit)},
                1));
        r.rows.push_back(rpt::Row::GroupFooter({{}, {}, {}, {}, rpt::Cell::Text("Slip total"), {}, {}, {}, {}, {},
                                                rpt::Cell::Amount(v.debit()), rpt::Cell::Amount(v.credit())}));
        // D8: signed, and only when the voucher doesn't balance.
        if (const std::int64_t diff = v.debit() - v.credit(); diff != 0) {
            std::vector<rpt::Cell> cells(12);
            cells[4] = rpt::Cell::Text("Difference (debits less credits)");
            cells[10] = rpt::Cell::Amount(diff);
            r.rows.push_back(rpt::Row::Detail(std::move(cells)));
        }
        debit += v.debit();
        credit += v.credit();
    }
    r.rows.push_back(rpt::Row::Total({{}, {}, {}, {}, rpt::Cell::Text("Total"), {}, {}, {}, {}, {},
                                      rpt::Cell::Amount(debit), rpt::Cell::Amount(credit)},
                                     true));
    return r;
}

std::vector<Voucher> unpostedOnly(std::vector<Voucher> vouchers)
{
    vouchers.erase(std::remove_if(vouchers.begin(), vouchers.end(), [](const Voucher& v) { return !unposted(v.mode); }),
                   vouchers.end());
    return vouchers;
}

} // namespace

std::string VoucherStatus(const std::string& mode)
{
    if (mode == "1") return "Open";
    if (mode == "3") return "Adjustment";
    if (mode == "0") return "Posted";
    if (mode == "2") return "Posted adjustment";
    if (mode == "9") return "Closed year";
    return mode.empty() ? "Unknown" : "Mode " + mode;
}

rpt::Report VoucherSlips(cb4sqlite::Code4& code, const ReportContext& ctx)
{
    return slips(ctx, loadVouchers(code), "glslip.rpt");
}

rpt::Report VoucherSlip(cb4sqlite::Code4& code, const ReportContext& ctx, long voucher)
{
    const std::vector<Voucher> v = loadVouchers(code, voucher);
    rpt::Report r = slips(ctx, v, "gl_slips.rpt");
    if (v.empty()) r.rows.push_back(rpt::Row::Note("Voucher " + std::to_string(voucher) + " doesn't exist."));
    return r;
}

rpt::Report VoucherDetail(cb4sqlite::Code4& code, const ReportContext& ctx)
{
    return detail(ctx, loadVouchers(code), "General Ledger - Slip Detail Listing", "glbtchd");
}

rpt::Report PrePostingReport(cb4sqlite::Code4& code, const ReportContext& ctx)
{
    rpt::Report r = detail(ctx, unpostedOnly(loadVouchers(code)), "General Ledger Pre-Posting Report",
                           "Vouchers not yet posted");
    r.subtitles.push_back("Vouchers not yet posted: open and adjustment");
    return r;
}

rpt::Report VoucherSummary(cb4sqlite::Code4& code, const ReportContext& ctx)
{
    rpt::Report r = frame(ctx, "General Ledger - Slip Summary Report", "gl_btchs.rpt");
    r.columns = {
        {"Prd.", 0.4, rpt::CellType::Integer},
        {"Slip", 0.6, rpt::CellType::Integer},
        {"Cheq.", 0.8},
        {"Date", 1.0, rpt::CellType::Date},
        {"Reference", 3.0},
        {"Debit", 1.3, rpt::CellType::Amount},
        {"Credit", 1.3, rpt::CellType::Amount},
    };
    std::int64_t debit = 0, credit = 0;
    for (const Voucher& v : loadVouchers(code)) {
        r.rows.push_back(rpt::Row::Detail({rpt::Cell::Integer(v.period), rpt::Cell::Integer(v.number),
                                           rpt::Cell::Text(v.cheque),
                                           v.date.empty() ? rpt::Cell::Empty() : rpt::Cell::Date(v.date),
                                           rpt::Cell::Text(v.description), rpt::Cell::Amount(v.debit()),
                                           rpt::Cell::Amount(v.credit())}));
        debit += v.debit();
        credit += v.credit();
    }
    r.rows.push_back(rpt::Row::Total({{}, {}, {}, {}, rpt::Cell::Text("Total"), rpt::Cell::Amount(debit),
                                      rpt::Cell::Amount(credit)},
                                     true));
    return r;
}

rpt::Report PrepostingDistribution(cb4sqlite::Code4& code, const ReportContext& ctx)
{
    rpt::Report r = frame(ctx, "General Ledger - Preposting Distribution Listing", "glpre.rpt");
    r.subtitles.push_back("The lines of vouchers not yet posted, and the balances after posting them");
    r.orientation = rpt::Orientation::Landscape;
    r.columns = {
        {"Pd", 0.35, rpt::CellType::Integer},
        {"Date", 1.0, rpt::CellType::Date},
        {"Slip No.", 0.6, rpt::CellType::Integer},
        {"Reference", 1.8},
        {"Description", 2.4},
        {"Cheque", 0.8},
        {"Debit", 1.2, rpt::CellType::Amount},
        {"Credit", 1.2, rpt::CellType::Amount},
        {"Balance", 1.3, rpt::CellType::Amount},
    };

    // The unposted lines, by account + dept (D4: posted lines are already in CURR_BAL).
    struct Item {
        long voucher;
        const Line* line;
    };
    const std::vector<Voucher> vouchers = loadVouchers(code);
    std::map<DistKey, std::vector<Item>> byAccount;
    for (const Voucher& v : vouchers)
        if (unposted(v.mode))
            for (const Line& l : v.lines) byAccount[{l.account, l.dept}].push_back({v.number, &l});

    std::map<DistKey, const AccountBalances*> accounts;
    const std::vector<AccountBalances> balances = LoadBalances(code);
    for (const AccountBalances& a : balances) accounts[{a.account, a.dept}] = &a;

    std::int64_t tDebit = 0, tCredit = 0;
    int tCount = 0;
    for (auto& [key, items] : byAccount) {
        std::sort(items.begin(), items.end(), [](const Item& a, const Item& b) {
            return std::make_tuple(a.line->period, a.line->date, a.voucher, a.line->journal) <
                   std::make_tuple(b.line->period, b.line->date, b.voucher, b.line->journal);
        });
        const auto acc = accounts.find(key);
        const std::int64_t current = acc != accounts.end() ? acc->second->currBal : 0;
        std::string head = key.first + (key.second.empty() ? std::string() : "-" + key.second);
        head += acc != accounts.end() ? "  " + acc->second->descript : "  (not in the chart of accounts)";
        r.rows.push_back(rpt::Row::GroupHeader(head));
        std::vector<rpt::Cell> start(9);
        start[4] = rpt::Cell::Text("Current balance");
        start[8] = rpt::Cell::Amount(current);
        r.rows.push_back(rpt::Row::Detail(std::move(start), 1));
        std::int64_t debit = 0, credit = 0;
        for (const Item& it : items) {
            const Line& l = *it.line;
            r.rows.push_back(rpt::Row::Detail({rpt::Cell::Integer(l.period),
                                               l.date.empty() ? rpt::Cell::Empty() : rpt::Cell::Date(l.date),
                                               rpt::Cell::Integer(it.voucher), rpt::Cell::Text(l.reference),
                                               rpt::Cell::Text(l.descript), rpt::Cell::Text(l.cheque),
                                               amountOrEmpty(l.debit), amountOrEmpty(l.credit), {}},
                                              1));
            debit += l.debit;
            credit += l.credit;
        }
        const int count = static_cast<int>(items.size());
        r.rows.push_back(rpt::Row::GroupFooter({{}, {}, {},
                                                rpt::Cell::Text(std::to_string(count) + (count == 1 ? " line" : " lines")),
                                                rpt::Cell::Text("Balance after posting"), {},
                                                rpt::Cell::Amount(debit), rpt::Cell::Amount(credit),
                                                rpt::Cell::Amount(current + debit - credit)}));
        tDebit += debit;
        tCredit += credit;
        tCount += count;
    }
    r.rows.push_back(rpt::Row::Total({{}, {}, {}, rpt::Cell::Text(std::to_string(tCount) + " lines"),
                                      rpt::Cell::Text("Total"), {},
                                      rpt::Cell::Amount(tDebit), rpt::Cell::Amount(tCredit), {}},
                                     true));
    if (byAccount.empty()) {
        r.rows.push_back(rpt::Row::Blank());
        r.rows.push_back(rpt::Row::Note("No voucher is waiting to be posted."));
    }
    return r;
}
