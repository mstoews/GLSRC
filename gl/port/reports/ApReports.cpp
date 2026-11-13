//
// ApReports.cpp - see ApReports.h.
//
// Translation notes (plan per-port checklist; decisions are spec #5 §8):
//   - 4091 printed every AP_REQST row (which Set Cheque Numbers rebuilt from
//     the "P" items) with Crystal's amount in words; the port prints the
//     cheques made and not yet confirmed printed, so a run prints only new
//     cheques, and Confirm Printed (8074) then marks them.
//
#include "ApReports.h"

#include "../Accounts.h"   // TrimText
#include "../Ap.h"

#include "report/Format.h"

#include <string>
#include <utility>
#include <vector>

namespace {

std::string join(std::initializer_list<std::string> parts, const char* sep)
{
    std::string out;
    for (const std::string& p : parts) {
        const std::string t = TrimText(p);
        if (t.empty()) continue;
        if (!out.empty()) out += sep;
        out += t;
    }
    return out;
}

std::string bankName(const ApAccounts& setat, const ApAccount& bank)
{
    for (const ApBank& b : setat.banks)
        if (b.account == bank) return TrimText(b.name) + " (" + bank.account + (bank.dept.empty() ? "" : "-" + bank.dept) + ")";
    return bank.account + (bank.dept.empty() ? "" : "-" + bank.dept);
}

} // namespace

rpt::Report ApCheques(cb4sqlite::Code4& code, const ReportContext& ctx)
{
    (void)ctx;
    rpt::Report r;
    r.title = "Cheques";
    r.footerNote = "ap_cheq.rpt";
    r.slipMode = true;
    r.columns = {
        {"Voucher", 0.8, rpt::CellType::Integer},
        {"Invoice", 1.2},
        {"Inv. Date", 1.1, rpt::CellType::Date},
        {"Reference", 3.0},
        {"Amount", 1.3, rpt::CellType::Amount},
        {"W/H", 1.1, rpt::CellType::Amount},
        {"Paid", 1.3, rpt::CellType::Amount},
    };
    const ApAccounts setat = LoadApAccounts(code);
    const std::vector<ChequeToPrint> cheques = LoadChequesIn(code, ap_payment::kPaid);
    for (const ChequeToPrint& c : cheques) {   // each slip starts a page (slipMode)
        const Vendor& v = c.payee;
        const std::string payee = TrimText(v.name).empty() ? v.number : TrimText(v.name);
        r.rows.push_back(rpt::Row::Slip({{"Cheque No.", std::to_string(c.cheque.number)},
                                         {"Date", rpt::FormatDate(c.cheque.date)},
                                         {"Pay to the order of", payee},
                                         {"Address", join({v.address1, v.address2, v.address3, v.postal}, ", ")},
                                         {"Amount", rpt::FormatAmount(c.cheque.amount)},
                                         {"Drawn on", bankName(setat, c.cheque.bank)}}));
        r.rows.push_back(rpt::Row::Note("The sum of " + AmountInWords(c.cheque.amount)));   // the full width
        std::int64_t amount = 0, wh = 0, paid = 0;
        for (const ChequeLine& l : c.lines) {
            r.rows.push_back(rpt::Row::Detail({rpt::Cell::Integer(l.batchno), rpt::Cell::Text(l.invoiceNo),
                                               l.invoiceDate.empty() ? rpt::Cell::Empty() : rpt::Cell::Date(l.invoiceDate),
                                               rpt::Cell::Text(TrimText(l.reference)), rpt::Cell::Amount(l.amount),
                                               rpt::Cell::Amount(l.wh), rpt::Cell::Amount(l.paid)}));
            amount += l.amount;
            wh += l.wh;
            paid += l.paid;
        }
        r.rows.push_back(rpt::Row::Total({{}, {}, {}, rpt::Cell::Text("Cheque total"), rpt::Cell::Amount(amount),
                                          rpt::Cell::Amount(wh), rpt::Cell::Amount(paid)},
                                         true));
    }
    if (cheques.empty()) r.rows.push_back(rpt::Row::Note("No cheques are waiting to be printed."));
    return r;
}
