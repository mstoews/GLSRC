//
// ApReports.cpp - see ApReports.h.
//
// Translation notes (plan per-port checklist; decisions are spec #5 §8):
//   - D14: one set of builders over Ap.h's records, in cents. The originals
//     printed the company name as a literal ("Takeda (Thailand) Ltd."); the
//     engine prints the company's.
//   - 4091 printed every AP_REQST row (which Set Cheque Numbers rebuilt from
//     the "P" items) with Crystal's amount in words; the port prints the
//     cheques made and not yet confirmed printed, so a run prints only new
//     cheques, and Confirm Printed (8074) then marks them.
//   - 8071 (AP_REB) and 8067 (AP_VATAP) list the tax lines of the AP
//     vouchers themselves, per invoice: once transferred, AP's journal is
//     netted by account in the GL, so the GL can't show them per invoice.
//     8072 (AP_VATGL) reads the GL, on GL_SETAT VATR's account.
//   - Not ported (spec #5 7): 1082 and 1990 (adjustments: there are none) and
//     the unreferenced AP_CHECK, AP_CHEQA, AP_CHEQS, AP_SUMRY, AP_VAT.
//
#include "ApReports.h"

#include "../Accounts.h"   // TrimText
#include "../Ap.h"

#include "report/Format.h"

#include <algorithm>
#include <map>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace {

using rpt::Cell;
using rpt::Row;

rpt::Report frame(std::string title, std::string footer)
{
    rpt::Report r;
    r.title = std::move(title);
    r.footerNote = std::move(footer);
    return r;
}

std::string periodLine(const ReportContext& ctx)
{
    const FiscalPeriod* p = ctx.period(ctx.currentPeriod);
    return "Period " + std::to_string(ctx.currentPeriod) + (p && !p->description.empty() ? " (" + p->description + ")" : "");
}

Cell dateOrEmpty(const std::string& ymd) { return TrimText(ymd).empty() ? Cell::Empty() : Cell::Date(TrimText(ymd)); }
Cell amountOrEmpty(std::int64_t c) { return c == 0 ? Cell::Empty() : Cell::Amount(c); }

std::string acct(const ApAccount& a) { return a.dept.empty() ? a.account : a.account + "-" + a.dept; }

const char* stateName(const std::string& mode)
{
    if (mode == ap_mode::kOpen) return "Open";
    if (mode == ap_mode::kPosted) return "Posted";
    if (mode == ap_mode::kTransferred) return "Transferred";
    if (mode == ap_mode::kCancelled) return "Cancelled";
    return "?";
}

const char* paymentName(const std::string& c)
{
    if (c == ap_payment::kHeld) return "Held";
    if (c == ap_payment::kOpen) return "Open";
    if (c == ap_payment::kForced) return "Forced";
    if (c == ap_payment::kPaid) return "Cheque made";
    if (c == ap_payment::kPrinted) return "Printed";
    if (c == ap_payment::kCleared) return "Cleared";
    return "";
}

bool unpaid(const OpenItem& i)
{
    return i.paymentCtl == ap_payment::kHeld || i.paymentCtl == ap_payment::kOpen ||
           i.paymentCtl == ap_payment::kForced;
}

std::map<std::string, Vendor> vendorsByNumber(cb4sqlite::Code4& code)
{
    std::map<std::string, Vendor> m;
    for (const Vendor& v : LoadVendors(code)) m[v.number] = v;
    return m;
}

std::string vendorLabel(const std::map<std::string, Vendor>& vendors, const std::string& number)
{
    auto it = vendors.find(number);
    return it == vendors.end() ? number : number + "  " + TrimText(it->second.name);
}

// The unpaid items by vendor.
std::map<std::string, std::vector<OpenItem>> unpaidByVendor(cb4sqlite::Code4& code)
{
    std::map<std::string, std::vector<OpenItem>> by;
    for (const OpenItem& i : LoadOpenItems(code))
        if (unpaid(i)) by[i.vendor].push_back(i);
    return by;
}

// The voucher-with-lines layout (1078, 1081, 8077, 8078).
rpt::Report voucherLines(cb4sqlite::Code4& code, ApKind kind, const std::vector<ApVoucher>& vouchers, std::string title,
                         std::string footer, std::vector<std::string> subtitles)
{
    rpt::Report r = frame(std::move(title), std::move(footer));
    r.subtitles = std::move(subtitles);
    r.columns = {{"No.", 0.4, rpt::CellType::Integer}, {"Account", 0.9}, {"Description", 3.0},
                 {"Debit", 1.2, rpt::CellType::Amount}, {"Credit", 1.2, rpt::CellType::Amount}};
    std::int64_t dr = 0, cr = 0;
    for (const ApVoucher& v : vouchers) {
        std::string head = "Voucher " + std::to_string(v.number) + "  " + v.vendor + " " + TrimText(v.vendorName) +
                           "  invoice " + v.invoiceNo + "  " + rpt::FormatDate(v.invoiceDate) + "  period " +
                           std::to_string(v.period) + "  " + stateName(v.mode);
        if (kind == ApKind::Cash && !v.cheque.empty()) head += "  cheque " + v.cheque;
        r.rows.push_back(Row::GroupHeader(head));
        std::int64_t vdr = 0, vcr = 0;
        for (const ApLine& l : LoadApLines(code, kind, v.number)) {
            r.rows.push_back(Row::Detail({Cell::Integer(l.line), Cell::Text(acct(l.account)), Cell::Text(TrimText(l.descript)),
                                          amountOrEmpty(l.debit), amountOrEmpty(l.credit)},
                                         1));
            vdr += l.debit;
            vcr += l.credit;
        }
        r.rows.push_back(Row::GroupFooter({{}, {}, Cell::Text("Voucher total"), Cell::Amount(vdr), Cell::Amount(vcr)}));
        dr += vdr;
        cr += vcr;
    }
    if (vouchers.empty()) r.rows.push_back(Row::Note("No vouchers."));
    else r.rows.push_back(Row::Total({{}, {}, Cell::Text("Total"), Cell::Amount(dr), Cell::Amount(cr)}, true));
    return r;
}

std::vector<ApVoucher> vouchersWhere(cb4sqlite::Code4& code, ApKind kind, bool (*keep)(const ApVoucher&, int), int period)
{
    std::vector<ApVoucher> out;
    for (const ApVoucher& v : LoadApVouchers(code, kind))
        if (keep(v, period)) out.push_back(v);
    return out;
}

// The journal by account: 8068's summary (a line each), 8069's, 8075's and 8076's detail.
rpt::Report journal(cb4sqlite::Code4& code, const std::vector<JournalRow>& rows, std::string title, std::string footer,
                    std::vector<std::string> subtitles, bool detail)
{
    rpt::Report r = frame(std::move(title), std::move(footer));
    r.subtitles = std::move(subtitles);
    if (detail) {
        r.orientation = rpt::Orientation::Landscape;
        r.columns = {{"Date", 1.0, rpt::CellType::Date}, {"Type", 0.5}, {"Voucher", 0.7, rpt::CellType::Integer},
                     {"Cheque", 0.8}, {"Reference", 2.0}, {"Description", 2.4}, {"Debit", 1.2, rpt::CellType::Amount},
                     {"Credit", 1.2, rpt::CellType::Amount}};
    } else {
        r.columns = {{"Account", 1.0}, {"Description", 3.0}, {"Debit", 1.2, rpt::CellType::Amount},
                     {"Credit", 1.2, rpt::CellType::Amount}};
    }
    std::map<std::pair<std::string, std::string>, std::vector<const JournalRow*>> by;   // account order
    for (const JournalRow& j : rows) by[{j.account.account, j.account.dept}].push_back(&j);
    std::int64_t dr = 0, cr = 0;
    for (const auto& [key, list] : by) {
        const ApAccount a{key.first, key.second};
        std::int64_t adr = 0, acr = 0;
        for (const JournalRow* j : list) {
            adr += j->debit;
            acr += j->credit;
        }
        if (detail) {
            r.rows.push_back(Row::GroupHeader(acct(a) + "  " + TrimText(AccountName(code, a))));
            for (const JournalRow* j : list)
                r.rows.push_back(Row::Detail({dateOrEmpty(j->date), Cell::Text(j->type), Cell::Integer(j->batchno),
                                              Cell::Text(j->cheque), Cell::Text(TrimText(j->reference)),
                                              Cell::Text(TrimText(j->descript)), amountOrEmpty(j->debit),
                                              amountOrEmpty(j->credit)},
                                             1));
            r.rows.push_back(Row::GroupFooter({{}, {}, {}, {}, {}, Cell::Text("Account total"), Cell::Amount(adr), Cell::Amount(acr)}));
        } else {
            r.rows.push_back(Row::Detail({Cell::Text(acct(a)), Cell::Text(TrimText(AccountName(code, a))),
                                          amountOrEmpty(adr), amountOrEmpty(acr)}));
        }
        dr += adr;
        cr += acr;
    }
    if (rows.empty())
        r.rows.push_back(Row::Note("No journal rows."));
    else if (detail)
        r.rows.push_back(Row::Total({{}, {}, {}, {}, {}, Cell::Text("Grand Total"), Cell::Amount(dr), Cell::Amount(cr)}, true));
    else
        r.rows.push_back(Row::Total({{}, Cell::Text("Grand Total"), Cell::Amount(dr), Cell::Amount(cr)}, true));
    return r;
}

// The tax lines (role `role`) of the current period's invoice and cash vouchers, posted or later.
struct TaxLine {
    ApVoucher voucher;
    ApLine line;
};

std::vector<TaxLine> taxLines(cb4sqlite::Code4& code, int period, ApLineRole role)
{
    std::vector<TaxLine> out;
    for (const ApKind kind : {ApKind::Invoice, ApKind::Cash})
        for (const ApVoucher& v : LoadApVouchers(code, kind)) {
            if (v.period != period || v.mode == ap_mode::kCancelled || v.mode == ap_mode::kOpen) continue;
            for (const ApLine& l : LoadApLines(code, kind, v.number))
                if (l.role == role && (l.debit != 0 || l.credit != 0)) out.push_back({v, l});
        }
    return out;
}

std::string voucherNo(const ApVoucher& v) { return (v.kind == ApKind::Cash ? "Cash " : "") + std::to_string(v.number); }

// The aged layout (1062, 1064): a row per unpaid invoice, its amount owed in
// the bucket `bucketOf` gives (0-3), by vendor, with totals.
rpt::Report aged(cb4sqlite::Code4& code, std::string title, std::string footer, std::string subtitle,
                 std::vector<std::string> buckets, int (*bucketOf)(const OpenItem&, const std::string&),
                 const std::string& asAt)
{
    rpt::Report r = frame(std::move(title), std::move(footer));
    r.orientation = rpt::Orientation::Landscape;
    r.subtitles = {std::move(subtitle)};
    r.columns = {{"Invoice", 1.0}, {"Due Date", 1.0, rpt::CellType::Date}};
    for (const std::string& b : buckets) r.columns.push_back({b, 1.2, rpt::CellType::Amount});
    r.columns.push_back({"Total", 1.3, rpt::CellType::Amount});
    const auto vendors = vendorsByNumber(code);
    const auto by = unpaidByVendor(code);
    std::int64_t totals[5] = {};
    auto cells = [](const char* label, const std::int64_t (&v)[5], bool blank) {
        std::vector<Cell> c{label ? Cell::Text(label) : Cell::Empty(), Cell::Empty()};
        for (int k = 0; k < 4; ++k) c.push_back(blank ? amountOrEmpty(v[k]) : Cell::Amount(v[k]));
        c.push_back(Cell::Amount(v[4]));
        return c;
    };
    for (const auto& [vendor, items] : by) {
        r.rows.push_back(Row::GroupHeader(vendorLabel(vendors, vendor)));
        std::int64_t sub[5] = {};
        for (const OpenItem& i : items) {
            const std::int64_t owed = AmountOwed(i);
            std::int64_t v[5] = {};
            v[bucketOf(i, asAt)] = owed;
            v[4] = owed;
            std::vector<Cell> row = cells(nullptr, v, true);
            row[0] = Cell::Text(i.invoiceNo);
            row[1] = dateOrEmpty(i.dueDate);
            r.rows.push_back(Row::Detail(std::move(row), 1));
            for (int k = 0; k < 5; ++k) sub[k] += v[k];
        }
        r.rows.push_back(Row::GroupFooter(cells("Vendor total", sub, false)));
        for (int k = 0; k < 5; ++k) totals[k] += sub[k];
    }
    if (by.empty()) r.rows.push_back(Row::Note("Nothing is owed."));
    else r.rows.push_back(Row::Total(cells("Total", totals, false), true));
    return r;
}

} // namespace

std::string ApAsAtDate(const ReportContext& ctx)
{
    const std::string s = TrimText(ctx.printedOn);   // mm/dd/yyyy
    if (s.size() != 10 || s[2] != '/' || s[5] != '/') return {};
    return s.substr(6, 4) + s.substr(0, 2) + s.substr(3, 2);
}

// ---- vouchers ------------------------------------------------------------------------

rpt::Report ApInvoicePrePosting(cb4sqlite::Code4& code, const ReportContext& ctx)
{
    (void)ctx;
    return voucherLines(code, ApKind::Invoice,
                        vouchersWhere(code, ApKind::Invoice, [](const ApVoucher& v, int) { return v.mode == ap_mode::kOpen; }, 0),
                        "Accounts Payable Invoice Pre-Posting Report", "apbtchsm.rpt", {"Open invoice vouchers"});
}

rpt::Report ApCashPrePosting(cb4sqlite::Code4& code, const ReportContext& ctx)
{
    (void)ctx;
    return voucherLines(code, ApKind::Cash,
                        vouchersWhere(code, ApKind::Cash, [](const ApVoucher& v, int) { return v.mode == ap_mode::kOpen; }, 0),
                        "Accounts Payable Cash Voucher Pre-Posting Report", "ap_cashb.rpt", {"Open cash vouchers"});
}

rpt::Report ApInvoiceVoucherDetail(cb4sqlite::Code4& code, const ReportContext& ctx)
{
    return voucherLines(code, ApKind::Invoice,
                        vouchersWhere(code, ApKind::Invoice, [](const ApVoucher& v, int p) { return v.period == p; },
                                      ctx.currentPeriod),
                        "Accounts Payable Voucher Detail", "ap_btchd.rpt", {periodLine(ctx)});
}

rpt::Report ApCashVoucherDetail(cb4sqlite::Code4& code, const ReportContext& ctx)
{
    return voucherLines(code, ApKind::Cash,
                        vouchersWhere(code, ApKind::Cash, [](const ApVoucher& v, int p) { return v.period == p; },
                                      ctx.currentPeriod),
                        "Accounts Payable Cash Voucher Detail", "ap_cashd.rpt", {periodLine(ctx)});
}

rpt::Report ApVoucherStatus(cb4sqlite::Code4& code, const ReportContext& ctx)
{
    rpt::Report r = frame("Accounts Payable Voucher Status", "ap_bstat.rpt");
    r.subtitles = {periodLine(ctx) + "; the total leaves out the cancelled"};
    r.columns = {{"Voucher", 0.7, rpt::CellType::Integer}, {"Date", 1.0, rpt::CellType::Date}, {"Vendor", 2.4},
                 {"Invoice", 1.0}, {"State", 1.1}, {"Amount", 1.3, rpt::CellType::Amount}};
    std::int64_t total = 0;
    int n = 0;
    for (const ApVoucher& v : LoadApVouchers(code, ApKind::Invoice)) {
        if (v.period != ctx.currentPeriod) continue;
        r.rows.push_back(Row::Detail({Cell::Integer(v.number), dateOrEmpty(v.invoiceDate),
                                      Cell::Text(v.vendor + "  " + TrimText(v.vendorName)), Cell::Text(v.invoiceNo),
                                      Cell::Text(stateName(v.mode)), Cell::Amount(v.payable)}));
        if (!v.cancelled()) total += v.payable;
        ++n;
    }
    if (n == 0) r.rows.push_back(Row::Note("No vouchers in this period."));
    else r.rows.push_back(Row::Total({{}, {}, {}, {}, Cell::Text("Total"), Cell::Amount(total)}, true));
    return r;
}

rpt::Report ApPostingJournal(cb4sqlite::Code4& code, const ReportContext& ctx)
{
    (void)ctx;
    rpt::Report r = frame("Accounts Payable Posting Journal", "ap_postj.rpt");
    r.subtitles = {"Posted invoice vouchers"};
    r.columns = {{"Account", 1.0}, {"Description", 3.0}, {"Debit", 1.2, rpt::CellType::Amount},
                 {"Credit", 1.2, rpt::CellType::Amount}};
    int n = 0;
    for (const ApVoucher& v : LoadApVouchers(code, ApKind::Invoice)) {
        if (v.mode != ap_mode::kPosted && v.mode != ap_mode::kTransferred) continue;
        r.rows.push_back(Row::GroupHeader("Voucher " + std::to_string(v.number) + "  " + v.vendor + " " +
                                          TrimText(v.vendorName) + "  invoice " + v.invoiceNo + "  " +
                                          rpt::FormatDate(v.invoiceDate) + "  due " + rpt::FormatDate(v.dueDate) + "  " +
                                          TrimText(v.reference)));
        std::int64_t ap = 0;
        for (const ApLine& l : LoadApLines(code, ApKind::Invoice, v.number)) {
            if (l.role == ApLineRole::Credit) {   // the AP line (the original: ACCOUNT <> "2210000")
                ap += l.credit - l.debit;
                continue;
            }
            r.rows.push_back(Row::Detail({Cell::Text(acct(l.account)), Cell::Text(TrimText(l.descript)),
                                          amountOrEmpty(l.debit), amountOrEmpty(l.credit)},
                                         1));
        }
        r.rows.push_back(Row::GroupFooter({{}, Cell::Text("Charged to Accounts Payable"), {}, Cell::Amount(ap)}));
        ++n;
    }
    if (n == 0) r.rows.push_back(Row::Note("No posted invoice vouchers."));
    return r;
}

// ---- vendors and open items ---------------------------------------------------------------

rpt::Report ApVendorList(cb4sqlite::Code4& code, const ReportContext& ctx)
{
    (void)ctx;
    rpt::Report r = frame("Accounts Payable Vendor List", "ap_vend.rpt");
    r.orientation = rpt::Orientation::Landscape;
    r.columns = {{"Vendor", 0.7}, {"Name", 2.6}, {"Address", 3.0}, {"Phone", 1.3}, {"AP Acct", 0.9},
                 {"VAT Acct", 0.9}, {"Terms", 0.5, rpt::CellType::Integer}, {"VAT %", 0.5, rpt::CellType::Integer},
                 {"W/H %", 0.5, rpt::CellType::Integer}, {"Type", 0.5}};
    const std::vector<Vendor> vendors = LoadVendors(code);
    for (const Vendor& v : vendors) {
        std::string address = TrimText(v.address1);
        for (const std::string* a : {&v.address2, &v.address3, &v.postal})
            if (!TrimText(*a).empty()) address += (address.empty() ? "" : ", ") + TrimText(*a);
        r.rows.push_back(Row::Detail({Cell::Text(v.number), Cell::Text(TrimText(v.name)), Cell::Text(address),
                                      Cell::Text(TrimText(v.phone)), Cell::Text(acct(v.ap)), Cell::Text(acct(v.vat)),
                                      Cell::Integer(v.terms), Cell::Integer(v.vatRate), Cell::Integer(v.whRate),
                                      Cell::Text(v.type == "I" ? "Ind." : v.type == "C" ? "Co." : v.type)}));
    }
    r.rows.push_back(Row::Blank());
    r.rows.push_back(Row::Note("Total number of vendors: " + std::to_string(vendors.size())));
    return r;
}

rpt::Report ApOpenPayables(cb4sqlite::Code4& code, const ReportContext& ctx)
{
    (void)ctx;
    rpt::Report r = frame("Detailed Vendor Open Payables", "ap_detl.rpt");
    r.columns = {{"Voucher", 0.7, rpt::CellType::Integer}, {"Invoice", 1.0}, {"Due Date", 1.0, rpt::CellType::Date},
                 {"Reference", 2.6}, {"State", 0.8}, {"Owed", 1.3, rpt::CellType::Amount}};
    const auto vendors = vendorsByNumber(code);
    const auto by = unpaidByVendor(code);
    std::int64_t total = 0;
    for (const auto& [vendor, items] : by) {
        r.rows.push_back(Row::GroupHeader(vendorLabel(vendors, vendor)));
        std::int64_t sub = 0;
        for (const OpenItem& i : items) {
            const std::int64_t owed = AmountOwed(i);
            r.rows.push_back(Row::Detail({Cell::Integer(i.batchno), Cell::Text(i.invoiceNo), dateOrEmpty(i.dueDate),
                                          Cell::Text(TrimText(i.reference)), Cell::Text(paymentName(i.paymentCtl)),
                                          Cell::Amount(owed)},
                                         1));
            sub += owed;
        }
        r.rows.push_back(Row::GroupFooter({{}, {}, {}, Cell::Text("Vendor total"), {}, Cell::Amount(sub)}));
        total += sub;
    }
    if (by.empty()) r.rows.push_back(Row::Note("Nothing is owed."));
    else r.rows.push_back(Row::Total({{}, {}, {}, Cell::Text("Total owed"), {}, Cell::Amount(total)}, true));
    return r;
}

rpt::Report ApAgedPayables(cb4sqlite::Code4& code, const ReportContext& ctx)
{
    const std::string asAt = ApAsAtDate(ctx);
    return aged(code, "Overdue Payments Aged 30-60-90 Days", "ap_overd.rpt",
                "As at " + rpt::FormatDate(asAt) + ", by days past the due date",
                {"Current", "31-60", "61-90", "Over 90"},
                [](const OpenItem& i, const std::string& at) {
                    const long late = DaysBetween(i.dueDate, at);   // > 0: overdue
                    return late <= 30 ? 0 : late <= 60 ? 1 : late <= 90 ? 2 : 3;
                },
                asAt);
}

rpt::Report ApCashRequirement(cb4sqlite::Code4& code, const ReportContext& ctx)
{
    const std::string asAt = ApAsAtDate(ctx);
    return aged(code, "Detailed Cash Requirement 30 Days", "ap_agedr.rpt",
                "As at " + rpt::FormatDate(asAt) + ", by days to the due date (overdue in the first column)",
                {"Up to 10 Days", "11 to 20 Days", "21 to 30 Days", "Over 30 Days"},
                [](const OpenItem& i, const std::string& at) {
                    const long until = DaysBetween(at, i.dueDate);
                    return until <= 10 ? 0 : until <= 20 ? 1 : until <= 30 ? 2 : 3;   // each day in one bucket
                },
                asAt);
}

rpt::Report ApRegister(cb4sqlite::Code4& code, const ReportContext& ctx)
{
    (void)ctx;
    rpt::Report r = frame("Accounts Payable Register", "ap_bala.rpt");
    r.orientation = rpt::Orientation::Landscape;
    r.columns = {{"Invoice", 1.0}, {"Entered", 1.0, rpt::CellType::Date}, {"Due Date", 1.0, rpt::CellType::Date},
                 {"Amount", 1.2, rpt::CellType::Amount}, {"W/H", 1.0, rpt::CellType::Amount},
                 {"Paid", 1.2, rpt::CellType::Amount}, {"Date Paid", 1.0, rpt::CellType::Date},
                 {"Cheque", 0.8, rpt::CellType::Integer}, {"Balance", 1.2, rpt::CellType::Amount}, {"State", 1.0}};
    const auto vendors = vendorsByNumber(code);
    std::map<std::string, std::vector<OpenItem>> by;
    for (const OpenItem& i : LoadOpenItems(code)) by[i.vendor].push_back(i);
    std::int64_t tot[4] = {};
    for (const auto& [vendor, items] : by) {
        r.rows.push_back(Row::GroupHeader(vendorLabel(vendors, vendor)));
        std::int64_t sub[4] = {};
        for (const OpenItem& i : items) {
            const std::int64_t balance = unpaid(i) ? AmountOwed(i) : 0;
            r.rows.push_back(Row::Detail({Cell::Text(i.invoiceNo), dateOrEmpty(i.date), dateOrEmpty(i.dueDate),
                                          Cell::Amount(i.amount), amountOrEmpty(i.wh), amountOrEmpty(i.paid),
                                          dateOrEmpty(i.datePaid), i.cheque ? Cell::Integer(i.cheque) : Cell::Empty(),
                                          Cell::Amount(balance), Cell::Text(paymentName(i.paymentCtl))},
                                         1));
            sub[0] += i.amount;
            sub[1] += i.wh;
            sub[2] += i.paid;
            sub[3] += balance;
        }
        r.rows.push_back(Row::GroupFooter({Cell::Text("Vendor total"), {}, {}, Cell::Amount(sub[0]), Cell::Amount(sub[1]),
                                           Cell::Amount(sub[2]), {}, {}, Cell::Amount(sub[3]), {}}));
        for (int k = 0; k < 4; ++k) tot[k] += sub[k];
    }
    if (by.empty())
        r.rows.push_back(Row::Note("No open items."));
    else
        r.rows.push_back(Row::Total({Cell::Text("Total"), {}, {}, Cell::Amount(tot[0]), Cell::Amount(tot[1]),
                                     Cell::Amount(tot[2]), {}, {}, Cell::Amount(tot[3]), {}},
                                    true));
    return r;
}

rpt::Report ApPrepayments(cb4sqlite::Code4& code, const ReportContext& ctx)
{
    (void)ctx;
    rpt::Report r = frame("Accounts Payable Prepayment Report", "ap_prepy.rpt");
    r.subtitles = {"Unpaid invoices, with their order numbers"};
    r.columns = {{"Vendor", 1.6}, {"Invoice", 1.0}, {"Order No.", 1.0}, {"Due Date", 1.0, rpt::CellType::Date},
                 {"Reference", 2.2}, {"Owed", 1.3, rpt::CellType::Amount}};
    const auto vendors = vendorsByNumber(code);
    std::int64_t total = 0;
    int n = 0;
    for (const OpenItem& i : LoadOpenItems(code)) {
        if (!unpaid(i)) continue;
        const std::optional<ApVoucher> v = FindApVoucher(code, ApKind::Invoice, i.batchno);
        auto it = vendors.find(i.vendor);
        const std::string name = it == vendors.end()                     ? i.vendor
                                 : TrimText(it->second.shortName).empty() ? TrimText(it->second.name)
                                                                          : TrimText(it->second.shortName);
        r.rows.push_back(Row::Detail({Cell::Text(i.vendor + "  " + name), Cell::Text(i.invoiceNo),
                                      Cell::Text(v ? TrimText(v->orderNo) : std::string()), dateOrEmpty(i.dueDate),
                                      Cell::Text(TrimText(i.reference)), Cell::Amount(AmountOwed(i))}));
        total += AmountOwed(i);
        ++n;
    }
    if (n == 0) r.rows.push_back(Row::Note("Nothing is owed."));
    else r.rows.push_back(Row::Total({{}, {}, {}, {}, Cell::Text("Total"), Cell::Amount(total)}, true));
    return r;
}

// ---- payments ---------------------------------------------------------------------------------

rpt::Report ApPaymentDetail(cb4sqlite::Code4& code, const ReportContext& ctx)
{
    (void)ctx;
    rpt::Report r = frame("Payment Detail", "ap_reqsd.rpt");
    r.orientation = rpt::Orientation::Landscape;
    r.columns = {{"Voucher", 0.7, rpt::CellType::Integer}, {"Invoice", 1.0}, {"Reference", 2.6},
                 {"Amount", 1.2, rpt::CellType::Amount}, {"VAT", 1.0, rpt::CellType::Amount},
                 {"W/H", 1.0, rpt::CellType::Amount}, {"Net Payment", 1.3, rpt::CellType::Amount}};
    const auto vendors = vendorsByNumber(code);
    std::map<long, std::vector<OpenItem>> by;
    for (const OpenItem& i : LoadOpenItems(code))
        if (i.cheque) by[i.cheque].push_back(i);
    std::int64_t total = 0;
    for (const auto& [cheque, items] : by) {
        r.rows.push_back(Row::GroupHeader("Cheque " + std::to_string(cheque) + "  " + vendorLabel(vendors, items.front().vendor) +
                                          "  paid " + rpt::FormatDate(items.front().datePaid) + "  " +
                                          paymentName(items.front().paymentCtl)));
        std::int64_t sub = 0;
        for (const OpenItem& i : items) {
            r.rows.push_back(Row::Detail({Cell::Integer(i.batchno), Cell::Text(i.invoiceNo), Cell::Text(TrimText(i.reference)),
                                          Cell::Amount(i.amount), amountOrEmpty(i.vat), amountOrEmpty(i.wh),
                                          Cell::Amount(i.paid)},
                                         1));
            sub += i.paid;
        }
        r.rows.push_back(Row::GroupFooter({{}, {}, Cell::Text("Cheque total"), {}, {}, {}, Cell::Amount(sub)}));
        total += sub;
    }
    if (by.empty()) r.rows.push_back(Row::Note("No payments."));
    else r.rows.push_back(Row::Total({{}, {}, Cell::Text("Total paid"), {}, {}, {}, Cell::Amount(total)}, true));
    return r;
}

rpt::Report ApCashChequeSummary(cb4sqlite::Code4& code, const ReportContext& ctx)
{
    rpt::Report r = frame("Accounts Payable Cash Cheque Summary", "ap_cheqc.rpt");
    r.subtitles = {periodLine(ctx)};
    r.orientation = rpt::Orientation::Landscape;
    r.columns = {{"Cheque", 0.8}, {"Voucher", 0.7, rpt::CellType::Integer}, {"Vendor", 2.5}, {"Invoice", 1.0},
                 {"Reference", 2.6}, {"Amount less W/H", 1.4, rpt::CellType::Amount}};
    std::int64_t total = 0;
    int n = 0;
    for (const ApVoucher& v : LoadApVouchers(code, ApKind::Cash)) {
        if (v.period != ctx.currentPeriod || v.cheque.empty() || v.cancelled()) continue;
        const std::int64_t amount = v.payable + std::min<std::int64_t>(v.whAmount, 0);
        r.rows.push_back(Row::Detail({Cell::Text(v.cheque), Cell::Integer(v.number),
                                      Cell::Text(v.vendor + "  " + TrimText(v.vendorName)), Cell::Text(v.invoiceNo),
                                      Cell::Text(TrimText(v.reference)), Cell::Amount(amount)}));
        total += amount;
        ++n;
    }
    if (n == 0) r.rows.push_back(Row::Note("No cash vouchers paid by cheque in this period."));
    else r.rows.push_back(Row::Total({{}, {}, {}, {}, Cell::Text("Total"), Cell::Amount(total)}, true));
    return r;
}

// ---- tax ------------------------------------------------------------------------------------

rpt::Report ApVatAp(cb4sqlite::Code4& code, const ReportContext& ctx)
{
    rpt::Report r = frame("Accounts Payable Value Added Tax", "ap_vatap.rpt");
    r.subtitles = {periodLine(ctx) + ", posted vouchers"};
    r.columns = {{"Voucher", 0.9}, {"Date", 1.0, rpt::CellType::Date}, {"Vendor", 2.2}, {"Invoice", 1.0},
                 {"Account", 0.9}, {"Debit", 1.2, rpt::CellType::Amount}, {"Credit", 1.2, rpt::CellType::Amount}};
    std::int64_t dr = 0, cr = 0;
    const std::vector<TaxLine> lines = taxLines(code, ctx.currentPeriod, ApLineRole::Vat);
    for (const TaxLine& t : lines) {
        r.rows.push_back(Row::Detail({Cell::Text(voucherNo(t.voucher)), dateOrEmpty(t.voucher.invoiceDate),
                                      Cell::Text(t.voucher.vendor + "  " + TrimText(t.voucher.vendorName)),
                                      Cell::Text(t.voucher.invoiceNo), Cell::Text(acct(t.line.account)),
                                      amountOrEmpty(t.line.debit), amountOrEmpty(t.line.credit)}));
        dr += t.line.debit;
        cr += t.line.credit;
    }
    if (lines.empty()) r.rows.push_back(Row::Note("No VAT on this period's posted vouchers."));
    else r.rows.push_back(Row::Total({{}, {}, {}, {}, Cell::Text("Total VAT"), Cell::Amount(dr), Cell::Amount(cr)}, true));
    return r;
}

rpt::Report ApVatGl(cb4sqlite::Code4& code, const ReportContext& ctx)
{
    const ApAccounts setat = LoadApAccounts(code);
    rpt::Report r = frame("Summary of General Ledger VAT Entries", "ap_vatgl.rpt");
    r.orientation = rpt::Orientation::Landscape;
    r.subtitles = {periodLine(ctx) + ", account " + acct(setat.vatInput) + " (Company Profile: VATR)"};
    r.columns = {{"Voucher", 0.7, rpt::CellType::Integer}, {"Date", 1.0, rpt::CellType::Date}, {"Reference", 2.0},
                 {"Description", 2.6}, {"Cheque", 0.8}, {"Debit", 1.2, rpt::CellType::Amount},
                 {"Credit", 1.2, rpt::CellType::Amount}};
    std::int64_t dr = 0, cr = 0;
    int n = 0;
    if (!setat.vatInput.empty()) {
        cb4sqlite::Query q(code, "SELECT batchno, date, reference, descript, cheque, debit, credit FROM gl_btchd "
                                 "WHERE _deleted = 0 AND trim(account) = ?1 AND coalesce(trim(dept), '') = ?2 "
                                 "AND CAST(trim(period) AS INTEGER) = ?3 ORDER BY batchno, journal, _rowid");
        q.bindText(1, setat.vatInput.account);
        q.bindText(2, setat.vatInput.dept);
        q.bindInt(3, ctx.currentPeriod);
        while (q.next()) {
            const std::int64_t d = rpt::ToCents(q.number(5)), c = rpt::ToCents(q.number(6));
            r.rows.push_back(Row::Detail({Cell::Integer(q.integer(0)), dateOrEmpty(q.text(1)), Cell::Text(TrimText(q.text(2))),
                                          Cell::Text(TrimText(q.text(3))), Cell::Text(TrimText(q.text(4))),
                                          amountOrEmpty(d), amountOrEmpty(c)}));
            dr += d;
            cr += c;
            ++n;
        }
    }
    if (n == 0) {
        r.rows.push_back(Row::Note("No General Ledger entries on the VAT account in this period."));
    } else {
        r.rows.push_back(Row::Total({{}, {}, {}, Cell::Text("Total"), {}, Cell::Amount(dr), Cell::Amount(cr)}, true));
        r.rows.push_back(Row::Note("Net VAT (debits less credits): " + rpt::FormatAmount(dr - cr)));
    }
    return r;
}

rpt::Report ApWithholding(cb4sqlite::Code4& code, const ReportContext& ctx)
{
    rpt::Report r = frame("Withholding Tax Report (Company and Individual)", "ap_reb.rpt");
    r.subtitles = {periodLine(ctx) + ", posted vouchers"};
    r.columns = {{"Voucher", 0.9}, {"Date", 1.0, rpt::CellType::Date}, {"Vendor", 2.4}, {"Invoice", 1.0},
                 {"Debit", 1.2, rpt::CellType::Amount}, {"Credit", 1.2, rpt::CellType::Amount}};
    const std::vector<TaxLine> lines = taxLines(code, ctx.currentPeriod, ApLineRole::WithHolding);
    std::map<std::pair<std::string, std::string>, std::vector<const TaxLine*>> by;
    for (const TaxLine& t : lines) by[{t.line.account.account, t.line.account.dept}].push_back(&t);
    std::int64_t dr = 0, cr = 0;
    for (const auto& [key, list] : by) {
        const ApAccount a{key.first, key.second};
        r.rows.push_back(Row::GroupHeader(acct(a) + "  " + TrimText(AccountName(code, a))));
        std::int64_t sdr = 0, scr = 0;
        for (const TaxLine* t : list) {
            r.rows.push_back(Row::Detail({Cell::Text(voucherNo(t->voucher)), dateOrEmpty(t->voucher.invoiceDate),
                                          Cell::Text(t->voucher.vendor + "  " + TrimText(t->voucher.vendorName)),
                                          Cell::Text(t->voucher.invoiceNo), amountOrEmpty(t->line.debit),
                                          amountOrEmpty(t->line.credit)},
                                         1));
            sdr += t->line.debit;
            scr += t->line.credit;
        }
        r.rows.push_back(Row::GroupFooter({{}, {}, {}, Cell::Text("Subtotal"), Cell::Amount(sdr), Cell::Amount(scr)}));
        dr += sdr;
        cr += scr;
    }
    if (lines.empty()) r.rows.push_back(Row::Note("No withholding tax on this period's posted vouchers."));
    else r.rows.push_back(Row::Total({{}, {}, {}, Cell::Text("Grand Total"), Cell::Amount(dr), Cell::Amount(cr)}, true));
    return r;
}

// ---- the journal ----------------------------------------------------------------------------

rpt::Report ApJournalSummary(cb4sqlite::Code4& code, const ReportContext& ctx)
{
    (void)ctx;
    std::vector<JournalRow> rows;
    for (const JournalRow& j : LoadJournal(code))
        if (j.mode == "1" || j.mode.empty()) rows.push_back(j);
    return journal(code, rows, "Accounts Payable Summary of General Ledger Journal", "ap_jsum.rpt",
                   {"Waiting for Update AR and AP"}, false);
}

rpt::Report ApJournalDetail(cb4sqlite::Code4& code, const ReportContext& ctx)
{
    std::vector<JournalRow> rows;
    for (const JournalRow& j : LoadJournal(code))
        if (j.period == ctx.currentPeriod) rows.push_back(j);
    return journal(code, rows, "Accounts Payable Detail of General Ledger Journal", "ap_jsumd.rpt", {periodLine(ctx)}, true);
}

rpt::Report ApJournalCash(cb4sqlite::Code4& code, const ReportContext& ctx)
{
    std::vector<JournalRow> rows;
    for (const JournalRow& j : LoadJournal(code))
        if (j.period == ctx.currentPeriod && j.type == "CA") rows.push_back(j);
    return journal(code, rows, "Cash Vouchers: Detail of General Ledger Journal", "ap_jsumc.rpt", {periodLine(ctx)}, true);
}

rpt::Report ApJournalInvoices(cb4sqlite::Code4& code, const ReportContext& ctx)
{
    std::vector<JournalRow> rows;
    for (const JournalRow& j : LoadJournal(code))
        if (j.period == ctx.currentPeriod && j.type == "AP") rows.push_back(j);
    return journal(code, rows, "Invoices and Payments: Detail of General Ledger Journal", "ap_jsuma.rpt",
                   {periodLine(ctx)}, true);
}

// ---- cheques (S5-M6) -----------------------------------------------------------------------

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
        if (b.account == bank) return TrimText(b.name) + " (" + acct(bank) + ")";
    return acct(bank);
}

} // namespace

rpt::Report ApCheques(cb4sqlite::Code4& code, const ReportContext& ctx)
{
    (void)ctx;
    rpt::Report r = frame("Cheques", "ap_cheq.rpt");
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
        r.rows.push_back(Row::Slip({{"Cheque No.", std::to_string(c.cheque.number)},
                                    {"Date", rpt::FormatDate(c.cheque.date)},
                                    {"Pay to the order of", payee},
                                    {"Address", join({v.address1, v.address2, v.address3, v.postal}, ", ")},
                                    {"Amount", rpt::FormatAmount(c.cheque.amount)},
                                    {"Drawn on", bankName(setat, c.cheque.bank)}}));
        r.rows.push_back(Row::Note("The sum of " + AmountInWords(c.cheque.amount)));   // the full width
        std::int64_t amount = 0, wh = 0, paid = 0;
        for (const ChequeLine& l : c.lines) {
            r.rows.push_back(Row::Detail({Cell::Integer(l.batchno), Cell::Text(l.invoiceNo), dateOrEmpty(l.invoiceDate),
                                          Cell::Text(TrimText(l.reference)), Cell::Amount(l.amount), Cell::Amount(l.wh),
                                          Cell::Amount(l.paid)}));
            amount += l.amount;
            wh += l.wh;
            paid += l.paid;
        }
        r.rows.push_back(Row::Total({{}, {}, {}, Cell::Text("Cheque total"), Cell::Amount(amount), Cell::Amount(wh),
                                     Cell::Amount(paid)},
                                    true));
    }
    if (cheques.empty()) r.rows.push_back(Row::Note("No cheques are waiting to be printed."));
    return r;
}
