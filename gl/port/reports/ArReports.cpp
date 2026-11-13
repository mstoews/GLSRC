//
// ArReports.cpp - see ArReports.h.
//
// Translation notes (plan per-port checklist; decisions are spec #6 §8):
//   - D14: one set of builders over Ar.h's records, in cents, as ApReports.
//     The originals' .RPT files were mostly AP reports renamed (AR_DETL read
//     AP_DIST, AR_JSUM read AP_JSUM, AR_VEND read AP_VEND); two (ar_bstat,
//     ar_batch) weren't in dbf/, and two (ar_recei, ar_cust) existed nowhere.
//     Each report here keeps its original's title and purpose, on AR's tables.
//   - "Owed" is the open item's REMAINDER (what Ar.h RecordReceipt keeps),
//     not AMOUNT less AMOUNTPAID: a discount settles an invoice without
//     being received.
//   - 4071 lists the VAT lines of the AR vouchers themselves, per invoice,
//     and the VAT of each receipt's discount (which reduces output VAT,
//     spec #6 5.2); 4072 reads the GL on GL_SETAT VATP's account.
//   - Not ported (spec #6 7): 8137 (ar_cust.rpt, nowhere) and the
//     unreferenced AP copies.
//
#include "ArReports.h"

#include "../Accounts.h"   // TrimText
#include "../Ar.h"

#include "report/Format.h"

#include <map>
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

// "11/14/2026" (ReportContext::printedOn) as yyyymmdd; "" if it isn't a date.
std::string asAtDate(const ReportContext& ctx)
{
    const std::string s = TrimText(ctx.printedOn);
    if (s.size() != 10 || s[2] != '/' || s[5] != '/') return {};
    return s.substr(6, 4) + s.substr(0, 2) + s.substr(3, 2);
}

const char* stateName(const std::string& mode)
{
    if (mode == ar_mode::kOpen) return "Open";
    if (mode == ar_mode::kPosted) return "Posted";
    if (mode == ar_mode::kTransferred) return "Transferred";
    if (mode == ar_mode::kCancelled) return "Cancelled";
    return "?";
}

const char* receiptName(const std::string& c)
{
    if (c == ar_receipt::kOpen) return "Owed";
    if (c == ar_receipt::kPartial) return "Part received";
    if (c == ar_receipt::kSettled) return "Settled";
    return "";
}

bool owing(const ArOpenItem& i) { return i.paymentCtl != ar_receipt::kSettled && i.remainder != 0; }

std::map<std::string, Customer> customersByNumber(cb4sqlite::Code4& code)
{
    std::map<std::string, Customer> m;
    for (const Customer& c : LoadCustomers(code)) m[TrimText(c.number)] = c;
    return m;
}

std::string customerLabel(const std::map<std::string, Customer>& customers, const std::string& number)
{
    auto it = customers.find(number);
    return it == customers.end() ? number : number + "  " + TrimText(it->second.name);
}

std::string address(const Customer& c)
{
    std::string out;
    for (const std::string* a : {&c.address1, &c.address2, &c.address3, &c.postal})
        if (!TrimText(*a).empty()) out += (out.empty() ? "" : ", ") + TrimText(*a);
    return out;
}

// The open items still owing, by customer.
std::map<std::string, std::vector<ArOpenItem>> owingByCustomer(cb4sqlite::Code4& code)
{
    std::map<std::string, std::vector<ArOpenItem>> by;
    for (const ArOpenItem& i : LoadArOpenItems(code))
        if (owing(i)) by[i.customer].push_back(i);
    return by;
}

// The voucher-with-lines layout (4063, 4073).
rpt::Report voucherLines(cb4sqlite::Code4& code, const std::vector<ArVoucher>& vouchers, std::string title,
                         std::string footer, std::vector<std::string> subtitles)
{
    rpt::Report r = frame(std::move(title), std::move(footer));
    r.subtitles = std::move(subtitles);
    r.columns = {{"No.", 0.4, rpt::CellType::Integer}, {"Account", 0.9}, {"Description", 3.0},
                 {"Debit", 1.2, rpt::CellType::Amount}, {"Credit", 1.2, rpt::CellType::Amount}};
    const auto customers = customersByNumber(code);
    std::int64_t dr = 0, cr = 0;
    for (const ArVoucher& v : vouchers) {
        r.rows.push_back(Row::GroupHeader("Voucher " + std::to_string(v.number) + "  " +
                                          customerLabel(customers, v.customer) + "  invoice " + v.invoiceNo + "  " +
                                          rpt::FormatDate(v.invoiceDate) + "  period " + std::to_string(v.period) +
                                          "  " + stateName(v.mode)));
        std::int64_t vdr = 0, vcr = 0;
        for (const ArLine& l : LoadArLines(code, v.number)) {
            std::string text = TrimText(l.descript);
            if (l.role == ArLineRole::Sales && l.price != 0)
                text += "  (" + rpt::FormatAmount(l.qty) + " x " + rpt::FormatAmount(l.price) + ")";
            r.rows.push_back(Row::Detail({Cell::Integer(l.line), Cell::Text(acct(l.account)), Cell::Text(text),
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

// The journal by account: 4074's summary (a line each), 4075's detail.
rpt::Report journal(cb4sqlite::Code4& code, const std::vector<JournalRow>& rows, std::string title, std::string footer,
                    std::vector<std::string> subtitles, bool detail)
{
    rpt::Report r = frame(std::move(title), std::move(footer));
    r.subtitles = std::move(subtitles);
    if (detail) {
        r.orientation = rpt::Orientation::Landscape;
        r.columns = {{"Date", 1.0, rpt::CellType::Date}, {"Type", 0.5}, {"Voucher", 0.7, rpt::CellType::Integer},
                     {"Receipt", 0.7}, {"Reference", 2.0}, {"Description", 2.4}, {"Debit", 1.2, rpt::CellType::Amount},
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
                r.rows.push_back(Row::Detail({dateOrEmpty(j->date), Cell::Text(TrimText(j->type)), Cell::Integer(j->batchno),
                                              Cell::Text(TrimText(j->cheque)), Cell::Text(TrimText(j->reference)),
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

// The aged layout (4068, 4061): a row per invoice owing, its amount in the
// bucket `bucketOf` gives (0-3), by customer, with totals.
rpt::Report aged(cb4sqlite::Code4& code, std::string title, std::string footer, std::string subtitle,
                 std::vector<std::string> buckets, int (*bucketOf)(const ArOpenItem&, const std::string&),
                 const std::string& asAt)
{
    rpt::Report r = frame(std::move(title), std::move(footer));
    r.orientation = rpt::Orientation::Landscape;
    r.subtitles = {std::move(subtitle)};
    r.columns = {{"Invoice", 1.0}, {"Due Date", 1.0, rpt::CellType::Date}};
    for (const std::string& b : buckets) r.columns.push_back({b, 1.2, rpt::CellType::Amount});
    r.columns.push_back({"Total", 1.3, rpt::CellType::Amount});
    const auto customers = customersByNumber(code);
    const auto by = owingByCustomer(code);
    std::int64_t totals[5] = {};
    auto cells = [](const char* label, const std::int64_t (&v)[5], bool blank) {
        std::vector<Cell> c{label ? Cell::Text(label) : Cell::Empty(), Cell::Empty()};
        for (int k = 0; k < 4; ++k) c.push_back(blank ? amountOrEmpty(v[k]) : Cell::Amount(v[k]));
        c.push_back(Cell::Amount(v[4]));
        return c;
    };
    for (const auto& [customer, items] : by) {
        r.rows.push_back(Row::GroupHeader(customerLabel(customers, customer)));
        std::int64_t sub[5] = {};
        for (const ArOpenItem& i : items) {
            std::int64_t v[5] = {};
            v[bucketOf(i, asAt)] = i.remainder;
            v[4] = i.remainder;
            std::vector<Cell> row = cells(nullptr, v, true);
            row[0] = Cell::Text(i.invoiceNo);
            row[1] = dateOrEmpty(i.dueDate);
            r.rows.push_back(Row::Detail(std::move(row), 1));
            for (int k = 0; k < 5; ++k) sub[k] += v[k];
        }
        r.rows.push_back(Row::GroupFooter(cells("Customer total", sub, false)));
        for (int k = 0; k < 5; ++k) totals[k] += sub[k];
    }
    if (by.empty()) r.rows.push_back(Row::Note("Nothing is owed."));
    else r.rows.push_back(Row::Total(cells("Total", totals, false), true));
    return r;
}

} // namespace

// ---- vouchers ------------------------------------------------------------------------

rpt::Report ArSalesPrePosting(cb4sqlite::Code4& code, const ReportContext& ctx)
{
    (void)ctx;
    std::vector<ArVoucher> open;
    for (const ArVoucher& v : LoadArVouchers(code))
        if (v.mode == ar_mode::kOpen) open.push_back(v);
    return voucherLines(code, open, "Accounts Receivable Sales Pre-Posting Report", "arbtchsm.rpt",
                        {"Open sales invoice vouchers"});
}

rpt::Report ArSalesVoucherDetail(cb4sqlite::Code4& code, const ReportContext& ctx)
{
    std::vector<ArVoucher> in;
    for (const ArVoucher& v : LoadArVouchers(code))
        if (v.period == ctx.currentPeriod) in.push_back(v);
    return voucherLines(code, in, "Accounts Receivable Sales Voucher Detail", "ar_btchd.rpt", {periodLine(ctx)});
}

rpt::Report ArVoucherStatus(cb4sqlite::Code4& code, const ReportContext& ctx)
{
    rpt::Report r = frame("Accounts Receivable Voucher Status", "ar_bstat.rpt");
    r.subtitles = {periodLine(ctx) + "; the total leaves out the cancelled"};
    r.columns = {{"Voucher", 0.7, rpt::CellType::Integer}, {"Date", 1.0, rpt::CellType::Date}, {"Customer", 2.4},
                 {"Invoice", 1.0}, {"State", 1.1}, {"Amount", 1.3, rpt::CellType::Amount}};
    const auto customers = customersByNumber(code);
    std::int64_t total = 0;
    int n = 0;
    for (const ArVoucher& v : LoadArVouchers(code)) {
        if (v.period != ctx.currentPeriod) continue;
        r.rows.push_back(Row::Detail({Cell::Integer(v.number), dateOrEmpty(v.invoiceDate),
                                      Cell::Text(customerLabel(customers, v.customer)), Cell::Text(v.invoiceNo),
                                      Cell::Text(stateName(v.mode)), Cell::Amount(v.total)}));
        if (!v.cancelled()) total += v.total;
        ++n;
    }
    if (n == 0) r.rows.push_back(Row::Note("No vouchers in this period."));
    else r.rows.push_back(Row::Total({{}, {}, {}, {}, Cell::Text("Total"), Cell::Amount(total)}, true));
    return r;
}

// ---- customers and open items -------------------------------------------------------------

rpt::Report ArCustomerList(cb4sqlite::Code4& code, const ReportContext& ctx)
{
    (void)ctx;
    rpt::Report r = frame("Accounts Receivable Customer List", "ar_vend.rpt");
    r.orientation = rpt::Orientation::Landscape;
    r.columns = {{"Customer", 0.7}, {"Name", 2.4}, {"Address", 3.0}, {"Phone", 1.3}, {"AR Acct", 0.9},
                 {"Sales Acct", 1.0}, {"VAT Acct", 0.9}, {"Terms", 0.5, rpt::CellType::Integer},
                 {"VAT %", 0.5, rpt::CellType::Integer}};
    const std::vector<Customer> customers = LoadCustomers(code);
    for (const Customer& c : customers)
        r.rows.push_back(Row::Detail({Cell::Text(c.number), Cell::Text(TrimText(c.name)), Cell::Text(address(c)),
                                      Cell::Text(TrimText(c.phone)), Cell::Text(acct(c.ar)), Cell::Text(acct(c.sales)),
                                      Cell::Text(acct(c.vat)), Cell::Integer(c.terms), Cell::Integer(c.vatRate)}));
    r.rows.push_back(Row::Blank());
    r.rows.push_back(Row::Note("Total number of customers: " + std::to_string(customers.size())));
    return r;
}

rpt::Report ArCustomerSummary(cb4sqlite::Code4& code, const ReportContext& ctx)
{
    (void)ctx;
    rpt::Report r = frame("Accounts Receivable Customer Summary", "ar_cust (new)");
    r.subtitles = {"Posted invoices still on file: invoiced = received + discounts + owed"};
    r.columns = {{"Customer", 2.8}, {"Invoices", 0.7, rpt::CellType::Integer}, {"Invoiced", 1.3, rpt::CellType::Amount},
                 {"Received", 1.3, rpt::CellType::Amount}, {"Discounts", 1.1, rpt::CellType::Amount},
                 {"Owed", 1.3, rpt::CellType::Amount}};
    std::map<long, std::int64_t> discounts;
    for (const Receipt& c : LoadReceipts(code)) discounts[c.batchno] += c.discount;
    struct Sum {
        int n = 0;
        std::int64_t invoiced = 0, received = 0, discount = 0, owed = 0;
    };
    std::map<std::string, Sum> by;
    for (const ArOpenItem& i : LoadArOpenItems(code)) {
        Sum& s = by[i.customer];
        ++s.n;
        s.invoiced += i.amount;
        s.received += i.received;
        s.discount += discounts[i.batchno];
        s.owed += i.remainder;
    }
    const auto customers = customersByNumber(code);
    Sum t;
    for (const Customer& c : LoadCustomers(code)) {
        const Sum s = by.count(TrimText(c.number)) ? by[TrimText(c.number)] : Sum{};
        r.rows.push_back(Row::Detail({Cell::Text(customerLabel(customers, TrimText(c.number))), Cell::Integer(s.n),
                                      Cell::Amount(s.invoiced), Cell::Amount(s.received), amountOrEmpty(s.discount),
                                      Cell::Amount(s.owed)}));
        t.n += s.n;
        t.invoiced += s.invoiced;
        t.received += s.received;
        t.discount += s.discount;
        t.owed += s.owed;
    }
    r.rows.push_back(Row::Total({Cell::Text("Total"), Cell::Integer(t.n), Cell::Amount(t.invoiced), Cell::Amount(t.received),
                                 Cell::Amount(t.discount), Cell::Amount(t.owed)},
                                true));
    return r;
}

rpt::Report ArOpenReceivables(cb4sqlite::Code4& code, const ReportContext& ctx)
{
    (void)ctx;
    rpt::Report r = frame("Detailed Customer Open Receivables", "ar_detl.rpt");
    r.columns = {{"Voucher", 0.7, rpt::CellType::Integer}, {"Invoice", 1.0}, {"Due Date", 1.0, rpt::CellType::Date},
                 {"Reference", 2.2}, {"Amount", 1.2, rpt::CellType::Amount}, {"Owed", 1.3, rpt::CellType::Amount}};
    const auto customers = customersByNumber(code);
    const auto by = owingByCustomer(code);
    std::int64_t total = 0;
    for (const auto& [customer, items] : by) {
        r.rows.push_back(Row::GroupHeader(customerLabel(customers, customer)));
        std::int64_t sub = 0;
        for (const ArOpenItem& i : items) {
            r.rows.push_back(Row::Detail({Cell::Integer(i.batchno), Cell::Text(i.invoiceNo), dateOrEmpty(i.dueDate),
                                          Cell::Text(TrimText(i.reference)), Cell::Amount(i.amount), Cell::Amount(i.remainder)},
                                         1));
            sub += i.remainder;
        }
        r.rows.push_back(Row::GroupFooter({{}, {}, {}, Cell::Text("Customer total"), {}, Cell::Amount(sub)}));
        total += sub;
    }
    if (by.empty()) r.rows.push_back(Row::Note("Nothing is owed."));
    else r.rows.push_back(Row::Total({{}, {}, {}, Cell::Text("Total owed"), {}, Cell::Amount(total)}, true));
    return r;
}

rpt::Report ArRegister(cb4sqlite::Code4& code, const ReportContext& ctx)
{
    (void)ctx;
    rpt::Report r = frame("Accounts Receivable Register", "ar_bala.rpt");
    r.orientation = rpt::Orientation::Landscape;
    r.columns = {{"Invoice", 1.4}, {"Date", 1.0, rpt::CellType::Date}, {"Due Date", 1.0, rpt::CellType::Date},
                 {"Amount", 1.2, rpt::CellType::Amount}, {"Received", 1.2, rpt::CellType::Amount},
                 {"Discount", 1.1, rpt::CellType::Amount}, {"Bank", 0.9}, {"Owed", 1.2, rpt::CellType::Amount},
                 {"State", 1.1}};
    const auto customers = customersByNumber(code);
    std::map<long, std::vector<Receipt>> receipts;
    for (const Receipt& c : LoadReceipts(code)) receipts[c.batchno].push_back(c);
    std::map<std::string, std::vector<ArOpenItem>> by;
    for (const ArOpenItem& i : LoadArOpenItems(code)) by[i.customer].push_back(i);
    std::int64_t tot[4] = {};
    for (const auto& [customer, items] : by) {
        r.rows.push_back(Row::GroupHeader(customerLabel(customers, customer)));
        std::int64_t sub[4] = {};
        for (const ArOpenItem& i : items) {
            std::int64_t disc = 0;
            for (const Receipt& c : receipts[i.batchno]) disc += c.discount;
            r.rows.push_back(Row::Detail({Cell::Text(i.invoiceNo), dateOrEmpty(i.date), dateOrEmpty(i.dueDate),
                                          Cell::Amount(i.amount), amountOrEmpty(i.received), amountOrEmpty(disc), {},
                                          Cell::Amount(i.remainder), Cell::Text(receiptName(i.paymentCtl))},
                                         1));
            for (const Receipt& c : receipts[i.batchno])
                r.rows.push_back(Row::Detail({Cell::Text("  receipt " + std::to_string(c.number)), dateOrEmpty(c.date), {},
                                              {}, Cell::Amount(c.amount), amountOrEmpty(c.discount),
                                              Cell::Text(acct(c.bank)), {}, Cell::Text(TrimText(c.reference))},
                                             2));
            sub[0] += i.amount;
            sub[1] += i.received;
            sub[2] += disc;
            sub[3] += i.remainder;
        }
        r.rows.push_back(Row::GroupFooter({Cell::Text("Customer total"), {}, {}, Cell::Amount(sub[0]), Cell::Amount(sub[1]),
                                           Cell::Amount(sub[2]), {}, Cell::Amount(sub[3]), {}}));
        for (int k = 0; k < 4; ++k) tot[k] += sub[k];
    }
    if (by.empty())
        r.rows.push_back(Row::Note("No open items."));
    else
        r.rows.push_back(Row::Total({Cell::Text("Total"), {}, {}, Cell::Amount(tot[0]), Cell::Amount(tot[1]),
                                     Cell::Amount(tot[2]), {}, Cell::Amount(tot[3]), {}},
                                    true));
    return r;
}

rpt::Report ArAgedReceivables(cb4sqlite::Code4& code, const ReportContext& ctx)
{
    const std::string asAt = asAtDate(ctx);
    return aged(code, "Overdue Receivables Aged 30-60-90 Days", "ar_agedc.rpt",
                "As at " + rpt::FormatDate(asAt) + ", by days past the due date (not yet due in Current)",
                {"Current", "31-60", "61-90", "Over 90"},
                [](const ArOpenItem& i, const std::string& at) {
                    const long late = DaysBetween(i.dueDate, at);   // > 0: overdue
                    return late <= 30 ? 0 : late <= 60 ? 1 : late <= 90 ? 2 : 3;
                },
                asAt);
}

rpt::Report ArCollectionsForecast(cb4sqlite::Code4& code, const ReportContext& ctx)
{
    const std::string asAt = asAtDate(ctx);
    return aged(code, "Collections Forecast 30 Days", "ar_agedr.rpt",
                "As at " + rpt::FormatDate(asAt) + ", by days to the due date (overdue in the first column)",
                {"Up to 10 Days", "11 to 20 Days", "21 to 30 Days", "Over 30 Days"},
                [](const ArOpenItem& i, const std::string& at) {
                    const long until = DaysBetween(at, i.dueDate);
                    return until <= 10 ? 0 : until <= 20 ? 1 : until <= 30 ? 2 : 3;   // each day in one bucket
                },
                asAt);
}

// ---- receipts ------------------------------------------------------------------------------

rpt::Report ArReceiptsRegister(cb4sqlite::Code4& code, const ReportContext& ctx)
{
    rpt::Report r = frame("Accounts Receivable Receipts Register", "ar_recei (new)");
    r.orientation = rpt::Orientation::Landscape;
    r.subtitles = {periodLine(ctx)};
    r.columns = {{"Receipt", 0.7, rpt::CellType::Integer}, {"Date", 1.0, rpt::CellType::Date}, {"Customer", 2.3},
                 {"Invoice", 1.0}, {"Bank", 0.9}, {"Reference", 1.8}, {"Received", 1.2, rpt::CellType::Amount},
                 {"Discount", 1.1, rpt::CellType::Amount}};
    const auto customers = customersByNumber(code);
    std::int64_t amount = 0, discount = 0;
    int n = 0;
    for (const Receipt& c : LoadReceipts(code)) {
        if (c.period != ctx.currentPeriod) continue;
        r.rows.push_back(Row::Detail({Cell::Integer(c.number), dateOrEmpty(c.date),
                                      Cell::Text(customerLabel(customers, TrimText(c.customer))), Cell::Text(c.invoiceNo),
                                      Cell::Text(acct(c.bank)), Cell::Text(TrimText(c.reference)), Cell::Amount(c.amount),
                                      amountOrEmpty(c.discount)}));
        amount += c.amount;
        discount += c.discount;
        ++n;
    }
    if (n == 0) r.rows.push_back(Row::Note("No receipts in this period."));
    else r.rows.push_back(Row::Total({{}, {}, {}, {}, {}, Cell::Text("Total"), Cell::Amount(amount), Cell::Amount(discount)}, true));
    return r;
}

// ---- tax ------------------------------------------------------------------------------------

rpt::Report ArVatSales(cb4sqlite::Code4& code, const ReportContext& ctx)
{
    rpt::Report r = frame("Accounts Receivable Value Added Tax", "ar_vatap.rpt");
    r.subtitles = {periodLine(ctx) + ": posted vouchers' output VAT, less the VAT of receipts' discounts"};
    r.columns = {{"Voucher", 0.9}, {"Date", 1.0, rpt::CellType::Date}, {"Customer", 2.2}, {"Invoice", 1.0},
                 {"Account", 0.9}, {"Debit", 1.2, rpt::CellType::Amount}, {"Credit", 1.2, rpt::CellType::Amount}};
    const auto customers = customersByNumber(code);
    std::int64_t dr = 0, cr = 0;
    int n = 0;
    for (const ArVoucher& v : LoadArVouchers(code)) {
        if (v.period != ctx.currentPeriod || v.mode == ar_mode::kOpen || v.cancelled()) continue;
        for (const ArLine& l : LoadArLines(code, v.number)) {
            if (l.role != ArLineRole::Vat || (l.debit == 0 && l.credit == 0)) continue;
            r.rows.push_back(Row::Detail({Cell::Text(std::to_string(v.number)), dateOrEmpty(v.invoiceDate),
                                          Cell::Text(customerLabel(customers, v.customer)), Cell::Text(v.invoiceNo),
                                          Cell::Text(acct(l.account)), amountOrEmpty(l.debit), amountOrEmpty(l.credit)}));
            dr += l.debit;
            cr += l.credit;
            ++n;
        }
    }
    for (const ReceiptItem& item : ReceiptItems(code, true)) {
        for (const Receipt& c : LoadReceipts(code, item.item.batchno)) {
            if (c.period != ctx.currentPeriod || c.discountVat == 0) continue;
            r.rows.push_back(Row::Detail({Cell::Text("Rcpt " + std::to_string(c.number)), dateOrEmpty(c.date),
                                          Cell::Text(customerLabel(customers, TrimText(c.customer))), Cell::Text(c.invoiceNo),
                                          Cell::Text(acct(item.vat)), Cell::Amount(c.discountVat), Cell::Empty()}));
            dr += c.discountVat;
            ++n;
        }
    }
    if (n == 0) {
        r.rows.push_back(Row::Note("No output VAT in this period."));
    } else {
        r.rows.push_back(Row::Total({{}, {}, {}, {}, Cell::Text("Total VAT"), Cell::Amount(dr), Cell::Amount(cr)}, true));
        r.rows.push_back(Row::Note("Net output VAT (credits less debits): " + rpt::FormatAmount(cr - dr)));
    }
    return r;
}

rpt::Report ArVatGl(cb4sqlite::Code4& code, const ReportContext& ctx)
{
    const ArAccounts setat = LoadArAccounts(code);
    rpt::Report r = frame("Summary of General Ledger Output VAT Entries", "ar_vatgl.rpt");
    r.orientation = rpt::Orientation::Landscape;
    r.subtitles = {periodLine(ctx) + ", account " + acct(setat.vatPayable) + " (Company Profile: VATP)"};
    r.columns = {{"Voucher", 0.7, rpt::CellType::Integer}, {"Date", 1.0, rpt::CellType::Date}, {"Reference", 2.0},
                 {"Description", 2.6}, {"Cheque", 0.8}, {"Debit", 1.2, rpt::CellType::Amount},
                 {"Credit", 1.2, rpt::CellType::Amount}};
    std::int64_t dr = 0, cr = 0;
    int n = 0;
    if (!setat.vatPayable.empty()) {
        cb4sqlite::Query q(code, "SELECT batchno, date, reference, descript, cheque, debit, credit FROM gl_btchd "
                                 "WHERE _deleted = 0 AND trim(account) = ?1 AND coalesce(trim(dept), '') = ?2 "
                                 "AND CAST(trim(period) AS INTEGER) = ?3 ORDER BY batchno, journal, _rowid");
        q.bindText(1, setat.vatPayable.account);
        q.bindText(2, setat.vatPayable.dept);
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
        r.rows.push_back(Row::Note("No General Ledger entries on the output VAT account in this period."));
    } else {
        r.rows.push_back(Row::Total({{}, {}, {}, Cell::Text("Total"), {}, Cell::Amount(dr), Cell::Amount(cr)}, true));
        r.rows.push_back(Row::Note("Net output VAT (credits less debits): " + rpt::FormatAmount(cr - dr)));
    }
    return r;
}

// ---- the journal ----------------------------------------------------------------------------

rpt::Report ArJournalSummary(cb4sqlite::Code4& code, const ReportContext& ctx)
{
    (void)ctx;
    std::vector<JournalRow> rows;
    for (const JournalRow& j : LoadArJournal(code))
        if (j.mode == "1" || j.mode.empty()) rows.push_back(j);
    return journal(code, rows, "Accounts Receivable Summary of General Ledger Journal", "ar_jsum.rpt",
                   {"Waiting for Update AR and AP"}, false);
}

rpt::Report ArJournalDetail(cb4sqlite::Code4& code, const ReportContext& ctx)
{
    std::vector<JournalRow> rows;
    for (const JournalRow& j : LoadArJournal(code))
        if (j.period == ctx.currentPeriod) rows.push_back(j);
    return journal(code, rows, "Accounts Receivable Detail of General Ledger Journal", "ar_jsumd.rpt",
                   {periodLine(ctx) + "; type AR invoices, RC receipts"}, true);
}

// ---- statements ----------------------------------------------------------------------------

rpt::Report ArStatements(cb4sqlite::Code4& code, const ReportContext& ctx)
{
    rpt::Report r = frame("Customer Statement", "ar_stat.rpt");
    r.slipMode = true;
    r.columns = {{"Invoice", 1.1}, {"Date", 1.1, rpt::CellType::Date}, {"Due Date", 1.1, rpt::CellType::Date},
                 {"Reference", 2.6}, {"Amount", 1.3, rpt::CellType::Amount}, {"Received", 1.3, rpt::CellType::Amount},
                 {"Owed", 1.3, rpt::CellType::Amount}};
    const std::string asAt = asAtDate(ctx);
    const auto by = owingByCustomer(code);
    const auto customers = customersByNumber(code);
    for (const auto& [number, items] : by) {   // each statement starts a page (slipMode)
        const auto it = customers.find(number);
        const Customer c = it == customers.end() ? Customer{} : it->second;
        r.rows.push_back(Row::Slip({{"Customer", number + "  " + TrimText(c.name)},
                                    {"Address", address(c)},
                                    {"Statement date", rpt::FormatDate(asAt)},
                                    {"Terms", c.terms ? std::to_string(c.terms) + " days" : std::string("Cash")}}));
        std::int64_t amount = 0, received = 0, owed = 0;
        for (const ArOpenItem& i : items) {
            r.rows.push_back(Row::Detail({Cell::Text(i.invoiceNo), dateOrEmpty(i.date), dateOrEmpty(i.dueDate),
                                          Cell::Text(TrimText(i.reference)), Cell::Amount(i.amount),
                                          amountOrEmpty(i.received), Cell::Amount(i.remainder)}));
            amount += i.amount;
            received += i.received;
            owed += i.remainder;
        }
        r.rows.push_back(Row::Total({{}, {}, {}, Cell::Text("Amount owed"), Cell::Amount(amount), Cell::Amount(received),
                                     Cell::Amount(owed)},
                                    true));
    }
    if (by.empty()) r.rows.push_back(Row::Note("No customer owes anything."));
    return r;
}
