//
// Ar.cpp - see Ar.h.
//
// Translation notes (plan per-port checklist):
//   - SplitSale / ComputeSale replace TARInvoiceEntry::ModifyCurrentSlip's
//     arithmetic (ar_btche.cpp:970-1060; spec #6 2.3): SALES =
//     floor(AR / (1 + v/100) x 1000) / 1000 in doubles, a cumulative VAT line,
//     and an AR line on the sales dept after the first. Here one formula in
//     cents (D4), per product line, and the VAT and AR lines recomputed from
//     all the product lines, so the voucher always balances (2.6 #5).
//   - The original wrote the doctor code to PRODUCT and the product to
//     DRCODE (2.6 #12); SaveArVoucher writes each to its own column (D13).
//   - The tables have no update/user/time fields, so the original's
//     UpdateUserTime calls wrote nothing; they aren't repeated.
//
#include "Ar.h"

#include "Accounts.h"       // TrimText
#include "Assets.h"         // ValidateAccount
#include "MasterTables.h"   // SeekCode, DeleteRecord

#include "report/Format.h"  // rpt::ToCents

#include <cstdio>
#include <cstdlib>
#include <map>

namespace {

bool hasTable(cb4sqlite::Code4& code, const char* name)
{
    cb4sqlite::Query q(code, "SELECT count(*) FROM sqlite_master WHERE type = 'table' AND name = ?1");
    q.bindText(1, name);
    return q.next() && q.integer(0) > 0;
}

// Cents as the text a numeric field takes ("1234.50").
std::string money(std::int64_t cents)
{
    char buf[40];
    const std::int64_t a = cents < 0 ? -cents : cents;
    std::snprintf(buf, sizeof buf, "%s%lld.%02lld", cents < 0 ? "-" : "", static_cast<long long>(a / 100),
                  static_cast<long long>(a % 100));
    return buf;
}

// a / b rounded half away from zero, in integers.
std::int64_t divRound(std::int64_t a, std::int64_t b)
{
    if (b == 0) return 0;
    const bool neg = (a < 0) != (b < 0);
    const std::int64_t ua = a < 0 ? -a : a, ub = b < 0 ? -b : b;
    const std::int64_t q = (ua + ub / 2) / ub;
    return neg ? -q : q;
}

char roleCode(ArLineRole r)
{
    switch (r) {
    case ArLineRole::Vat: return 'V';
    case ArLineRole::Receivable: return 'R';
    default: return 'S';
    }
}

ArLineRole roleOf(const std::string& code)
{
    if (code == "V") return ArLineRole::Vat;
    if (code == "R") return ArLineRole::Receivable;
    return ArLineRole::Sales;
}

constexpr const char* kCustomerColumns =
    "vendorno, vdshort, vdname, address1, address2, address3, postal, phone, fax, s_add1, s_add2, s_add3, "
    "s_postal, s_phone, s_fax, contact, vin, account, depart, ar_acct, ar_dept, vat_acct, vat_dept, terms, vat";

Customer readCustomer(const cb4sqlite::Query& q)
{
    Customer c;
    c.number = q.text(0);
    c.shortName = q.text(1);
    c.name = q.text(2);
    c.address1 = q.text(3);
    c.address2 = q.text(4);
    c.address3 = q.text(5);
    c.postal = q.text(6);
    c.phone = q.text(7);
    c.fax = q.text(8);
    c.shipAddress1 = q.text(9);
    c.shipAddress2 = q.text(10);
    c.shipAddress3 = q.text(11);
    c.shipPostal = q.text(12);
    c.shipPhone = q.text(13);
    c.shipFax = q.text(14);
    c.contact = q.text(15);
    c.vin = q.text(16);
    c.ar = {TrimText(q.text(17)), TrimText(q.text(18))};
    c.sales = {TrimText(q.text(19)), TrimText(q.text(20))};
    c.vat = {TrimText(q.text(21)), TrimText(q.text(22))};
    c.terms = static_cast<int>(q.integer(23));
    c.vatRate = static_cast<int>(q.integer(24));
    return c;
}

constexpr const char* kVoucherColumns =
    "batchno, mode, vendorno, invoiceno, orderno, orderdate, vin, units, reference, descript, date, invdate, "
    "duedate, memofield, period, terms, vatpercent, account, depart, payable, vatamt, entryno";

ArVoucher readVoucher(const cb4sqlite::Query& q)
{
    ArVoucher v;
    v.number = static_cast<long>(q.integer(0));
    v.mode = TrimText(q.text(1));
    v.customer = TrimText(q.text(2));
    v.invoiceNo = TrimText(q.text(3));
    v.orderNo = q.text(4);
    v.orderDate = q.text(5);
    v.vin = q.text(6);
    v.units = q.text(7);
    v.reference = q.text(8);
    v.descript = q.text(9);
    v.date = q.text(10);
    v.invoiceDate = q.text(11);
    v.dueDate = q.text(12);
    v.memo = q.text(13);
    v.period = static_cast<int>(q.integer(14));
    v.terms = static_cast<int>(q.integer(15));
    v.vatRate = static_cast<int>(q.integer(16));
    v.account = {TrimText(q.text(17)), TrimText(q.text(18))};
    v.total = rpt::ToCents(q.number(19));
    v.vatAmount = rpt::ToCents(q.number(20));
    v.lines = static_cast<int>(q.integer(21));
    return v;
}

// Seeks voucher `number` in a table on its batchno tag.
bool seekBatch(Data4& d, const Field4& batchno, long number)
{
    if (!d.isValid() || number <= 0) return false;
    d.SetIndex("batchno");
    return d.seek(static_cast<double>(number)) == r4success && std::atol(batchno.str()) == number;
}

// Deletes every row of voucher `number` from a table with a batchno tag and
// packs it if anything went. No transaction of its own.
void deleteRows(cb4sqlite::Code4& code, const char* table, long number)
{
    Data4 d(&code, table);
    if (!d.isValid()) return;
    Field4 batchno(d, "batchno");
    bool any = false;
    if (seekBatch(d, batchno, number))
        for (; !d.eof() && std::atol(batchno.str()) == number; d.skip())
            if (!d.deleted()) {
                d.deleteRec();
                any = true;
            }
    if (any) d.pack();
    d.close();
}

} // namespace

// ---- tables -----------------------------------------------------------------------

void EnsureArTables(cb4sqlite::Code4& code)
{
    const char* names[] = {"ar_cust", "ar_btch", "ar_btchd", "ar_dist", "ar_jsum", "ar_linex", "ar_rcpt", "ar_xfer"};
    bool have[8] = {};
    bool all = true;
    for (int i = 0; i < 8; ++i) all = (have[i] = hasTable(code, names[i])) && all;
    if (all) return;

    cb4sqlite::Transaction tx(code);
    if (!have[0]) {   // dbf/AR_CUST
        cb4sqlite::createTable(code, "ar_cust",
                               {{"VENDORNO", 'C', 6},  {"VDSHORT", 'C', 12},   {"VDNAME", 'C', 40},
                                {"ADDRESS1", 'C', 30}, {"ADDRESS2", 'C', 30},  {"ADDRESS3", 'C', 30},
                                {"POSTAL", 'C', 8},    {"PHONE", 'C', 16},     {"FAX", 'C', 16},
                                {"S_ADD1", 'C', 40},   {"S_ADD2", 'C', 40},    {"S_ADD3", 'C', 40},
                                {"S_POSTAL", 'C', 8},  {"S_PHONE", 'C', 16},   {"S_FAX", 'C', 16},
                                {"DEPART", 'C', 4},    {"ACCOUNT", 'C', 7},    {"AR_DEPT", 'C', 4},
                                {"AR_ACCT", 'C', 7},   {"VAT_ACCT", 'C', 7},   {"VAT_DEPT", 'C', 4},
                                {"CONTACT", 'C', 12},  {"TERMS", 'N', 3},      {"TERMS_DAYS", 'N', 3},
                                {"TERMS_PERC", 'N', 2}, {"TERMS_TXT", 'C', 20}, {"CREDIT", 'N', 16, 2},
                                {"VAT", 'N', 3},       {"VIN", 'C', 10}});
        cb4sqlite::createTag(code, "ar_cust", "vendorno", "\"VENDORNO\"");
    }
    // The header table and its lines go together: a company has both or neither.
    if (!have[1] && !have[2]) {
        cb4sqlite::createTable(code, "ar_btch",   // dbf/AR_BTCH, with its memo
                               {{"BATCHNO", 'N', 6},    {"MODE", 'C', 1},       {"TYPE", 'C', 2},
                                {"ADJ", 'C', 1},        {"PERIOD", 'N', 2},     {"VENDORNO", 'C', 6},
                                {"INVOICENO", 'C', 8},  {"ORDERNO", 'C', 10},   {"ORDERDATE", 'D', 8},
                                {"VIN", 'C', 10},       {"UNITS", 'C', 7},      {"DUEDATE", 'D', 8},
                                {"INVDATE", 'D', 8},    {"REFERENCE", 'C', 40}, {"DESCRIPT", 'C', 10},
                                {"DATE", 'D', 8},       {"ACCOUNT", 'C', 7},    {"DEPART", 'C', 4},
                                {"PAYABLE", 'N', 16, 2}, {"REBATEAMT", 'N', 16, 2}, {"ENTRYNO", 'N', 3},
                                {"VATPERCENT", 'N', 2}, {"REBATE", 'N', 2},     {"TERMS", 'N', 3},
                                {"VATAMT", 'N', 16, 2}, {"ADJAMT", 'N', 16, 2}, {"MEMOFIELD", 'M', 10}});
        // The tags in dbf/AR_BTCH.MDX's order (the first is the default).
        cb4sqlite::createTag(code, "ar_btch", "account", "\"ACCOUNT\" || \"DEPART\"");
        cb4sqlite::createTag(code, "ar_btch", "invoiceno", "\"VENDORNO\" || \"INVOICENO\"");
        cb4sqlite::createTag(code, "ar_btch", "recno", "_rowid");
        cb4sqlite::createTag(code, "ar_btch", "batchno", "\"BATCHNO\"");
        cb4sqlite::createTable(code, "ar_btchd",   // dbf/AR_BTCHD
                               {{"BATCHNO", 'N', 6},  {"ENTRYNO", 'N', 3},  {"DESCRIPT", 'C', 40},
                                {"MEDREP", 'C', 5},   {"PRODUCT", 'C', 7},  {"DRCODE", 'C', 5},
                                {"QTY", 'N', 14, 2},  {"PRICE", 'N', 14, 2}, {"ACCOUNT", 'C', 7},
                                {"DEPT", 'C', 4},     {"DEBIT", 'N', 14, 2}, {"CREDIT", 'N', 14, 2}});
        cb4sqlite::createTag(code, "ar_btchd", "recno", "_rowid");
        cb4sqlite::createTag(code, "ar_btchd", "batchno", "\"BATCHNO\"");
    }
    if (!have[3]) {   // dbf/AR_DIST
        cb4sqlite::createTable(code, "ar_dist",
                               {{"ACCOUNT", 'C', 7},      {"DEPT", 'C', 4},         {"MODE", 'C', 1},
                                {"APACCOUNT", 'C', 7},    {"APDEPART", 'C', 4},     {"PERIOD", 'N', 2},
                                {"DATE", 'D', 8},         {"DUEDATE", 'D', 8},      {"BATCHNO", 'N', 6},
                                {"INVOICENO", 'C', 8},    {"PAYMENTCTL", 'C', 1},   {"VENDORNO", 'C', 6},
                                {"CHEQUENO", 'C', 10},    {"REFERENCE", 'C', 20},   {"DESCRIPT", 'C', 50},
                                {"AMOUNT", 'N', 16, 2},   {"REBATEAMT", 'N', 16, 2}, {"AMOUNTPAID", 'N', 16, 2},
                                {"PAYMENTREQ", 'N', 10},  {"DATEPAID", 'D', 8},     {"VATAMT", 'N', 16, 2},
                                {"ADJUSTMENT", 'N', 16, 2}, {"REBATE", 'C', 2},     {"REMAINDER", 'N', 16, 2}});
        cb4sqlite::createTag(code, "ar_dist", "paymentreq", "\"PAYMENTREQ\"");
        cb4sqlite::createTag(code, "ar_dist", "vendorno", "\"VENDORNO\"");
        cb4sqlite::createTag(code, "ar_dist", "batchno", "\"BATCHNO\"");
        cb4sqlite::createTag(code, "ar_dist", "invoiceno", "\"VENDORNO\" || \"INVOICENO\"");
        cb4sqlite::createTag(code, "ar_dist", "dept", "\"DEPT\"");
        cb4sqlite::createTag(code, "ar_dist", "account", "\"ACCOUNT\"");
    }
    if (!have[4]) {   // dbf/AR_JSUM
        cb4sqlite::createTable(code, "ar_jsum",
                               {{"ACCOUNT", 'C', 7},   {"DEPT", 'C', 4},      {"PERIOD", 'C', 4},
                                {"MODE", 'C', 1},      {"DATE", 'D', 8},      {"TYPE", 'C', 3},
                                {"BATCHNO", 'N', 6},   {"REFERENCE", 'C', 30}, {"CHEQUE", 'C', 7},
                                {"DESCRIPT", 'C', 50}, {"DEBIT", 'N', 16, 2}, {"CREDIT", 'N', 16, 2}});
        cb4sqlite::createTag(code, "ar_jsum", "ardist", "\"BATCHNO\"",
                             "(substr(\"TYPE\", 1, length('AR ')) = 'AR ')");
        cb4sqlite::createTag(code, "ar_jsum", "apdist", "\"BATCHNO\"",
                             "(substr(\"TYPE\", 1, length('AP ')) = 'AP ')");
        cb4sqlite::createTag(code, "ar_jsum", "descript", "\"DESCRIPT\"");
        cb4sqlite::createTag(code, "ar_jsum", "batchno", "\"BATCHNO\"");
        cb4sqlite::createTag(code, "ar_jsum", "account", "\"ACCOUNT\" || \"DEPT\"");
    }
    if (!have[5]) {   // the port's own (Ar.h)
        cb4sqlite::createTable(code, "ar_linex",
                               {{"BATCHNO", 'N', 6}, {"ENTRYNO", 'N', 3}, {"ROLE", 'C', 1}, {"GROSS", 'N', 16, 2}});
        cb4sqlite::createTag(code, "ar_linex", "batchno", "\"BATCHNO\"");
    }
    if (!have[6]) {   // the port's own (Ar.h)
        cb4sqlite::createTable(code, "ar_rcpt",
                               {{"RCPTNO", 'N', 6},     {"BATCHNO", 'N', 6},     {"VENDORNO", 'C', 6},
                                {"INVOICENO", 'C', 8},  {"DATE", 'D', 8},        {"PERIOD", 'N', 2},
                                {"BANKACCT", 'C', 7},   {"BANKDEPT", 'C', 4},    {"AMOUNT", 'N', 16, 2},
                                {"DISCOUNT", 'N', 16, 2}, {"DISCVAT", 'N', 16, 2}, {"REFERENCE", 'C', 30}});
        cb4sqlite::createTag(code, "ar_rcpt", "batchno", "\"BATCHNO\"");
        cb4sqlite::createTag(code, "ar_rcpt", "rcptno", "\"RCPTNO\"");
    }
    if (!have[7]) {   // the port's own (Ar.h)
        cb4sqlite::createTable(code, "ar_xfer", {{"JSUMROW", 'N', 10}, {"GLBATCH", 'N', 6}, {"PERIOD", 'N', 2}});
        cb4sqlite::createTag(code, "ar_xfer", "glbatch", "\"GLBATCH\"");
    }
    tx.commit();
}

// ---- accounts ------------------------------------------------------------------------

ArAccounts LoadArAccounts(cb4sqlite::Code4& code)
{
    ArAccounts a;
    if (!hasTable(code, "gl_setat")) return a;
    cb4sqlite::Query q(code, "SELECT short, name, account, dept, type FROM gl_setat WHERE _deleted = 0 "
                             "ORDER BY short, _rowid");
    while (q.next()) {
        const std::string shortName = TrimText(q.text(0)), type = TrimText(q.text(4));
        const ApAccount acct{TrimText(q.text(2)), TrimText(q.text(3))};
        if (type == "CA") {
            a.banks.push_back({shortName, q.text(1), acct});
            continue;
        }
        if (type != "GL") continue;
        ApAccount* slot = shortName == "AR"     ? &a.ar
                          : shortName == "SALE" ? &a.sales
                          : shortName == "VATP" ? &a.vatPayable
                                                : nullptr;
        if (slot && slot->empty()) *slot = acct;
    }
    return a;
}

// ---- customers ---------------------------------------------------------------------------

std::vector<Customer> LoadCustomers(cb4sqlite::Code4& code)
{
    std::vector<Customer> out;
    cb4sqlite::Query q(code, std::string("SELECT ") + kCustomerColumns +
                                 " FROM ar_cust WHERE _deleted = 0 ORDER BY vendorno, _rowid");
    while (q.next()) out.push_back(readCustomer(q));
    return out;
}

std::optional<Customer> FindCustomer(cb4sqlite::Code4& code, const std::string& number)
{
    cb4sqlite::Query q(code, std::string("SELECT ") + kCustomerColumns +
                                 " FROM ar_cust WHERE _deleted = 0 AND trim(vendorno) = ?1 ORDER BY _rowid LIMIT 1");
    q.bindText(1, TrimText(number));
    if (!q.next()) return std::nullopt;
    return readCustomer(q);
}

bool SaveCustomer(cb4sqlite::Code4& code, const Customer& c)
{
    cb4sqlite::Transaction tx(code);
    Data4 d(&code, "ar_cust");
    d.SetIndex("vendorno");
    Field4 vendorno(d, "vendorno");
    const bool added = !SeekCode(d, vendorno, c.number);
    if (added) {
        d.appendStart();   // then blank(): blank() first would flush blanks over the current record
        d.blank();
    }
    const std::pair<const char*, const std::string*> texts[] = {
        {"vendorno", &c.number},      {"vdshort", &c.shortName},     {"vdname", &c.name},
        {"address1", &c.address1},    {"address2", &c.address2},     {"address3", &c.address3},
        {"postal", &c.postal},        {"phone", &c.phone},           {"fax", &c.fax},
        {"s_add1", &c.shipAddress1},  {"s_add2", &c.shipAddress2},   {"s_add3", &c.shipAddress3},
        {"s_postal", &c.shipPostal},  {"s_phone", &c.shipPhone},     {"s_fax", &c.shipFax},
        {"contact", &c.contact},      {"vin", &c.vin},               {"account", &c.ar.account},
        {"depart", &c.ar.dept},       {"ar_acct", &c.sales.account}, {"ar_dept", &c.sales.dept},
        {"vat_acct", &c.vat.account}, {"vat_dept", &c.vat.dept}};
    for (const auto& [name, value] : texts) Field4(d, name).assign(value->c_str());
    Field4(d, "terms").assignLong(c.terms);
    Field4(d, "vat").assignLong(c.vatRate);
    if (added) d.append(); else d.flush();
    d.close();
    tx.commit();
    return added;
}

std::optional<CustomerProblem> CheckCustomer(cb4sqlite::Code4& code, const Customer& c)
{
    using K = CustomerProblem::Kind;
    if (TrimText(c.number).empty()) return CustomerProblem{CustomerField::Number, K::Required};
    if (TrimText(c.shortName).empty()) return CustomerProblem{CustomerField::ShortName, K::Required};
    if (TrimText(c.name).empty()) return CustomerProblem{CustomerField::Name, K::Required};
    if (c.vatRate < 0 || c.vatRate > 99) return CustomerProblem{CustomerField::VatRate, K::BadRate};
    if (!ValidateAccount(code, c.ar.account, c.ar.dept)) return CustomerProblem{CustomerField::ArAccount, K::BadAccount};
    if (!ValidateAccount(code, c.sales.account, c.sales.dept))
        return CustomerProblem{CustomerField::SalesAccount, K::BadAccount};
    if (c.vat.empty()) {
        if (c.vatRate > 0) return CustomerProblem{CustomerField::VatAccount, K::VatAccountNeeded};
    } else if (!ValidateAccount(code, c.vat.account, c.vat.dept)) {
        return CustomerProblem{CustomerField::VatAccount, K::BadAccount};
    }
    return std::nullopt;
}

DeleteCustomerResult DeleteCustomer(cb4sqlite::Code4& code, const std::string& number, std::string* usedBy)
{
    const std::string want = TrimText(number);
    const std::pair<const char*, const char*> uses[] = {
        {"SELECT batchno FROM ar_btch WHERE _deleted = 0 AND trim(vendorno) = ?1 ORDER BY batchno LIMIT 1",
         "sales voucher "},
        {"SELECT batchno FROM ar_dist WHERE _deleted = 0 AND trim(vendorno) = ?1 ORDER BY batchno LIMIT 1",
         "the open item of voucher "},
        {"SELECT rcptno FROM ar_rcpt WHERE _deleted = 0 AND trim(vendorno) = ?1 ORDER BY rcptno LIMIT 1",
         "receipt "},
    };
    for (const auto& [sql, what] : uses) {
        cb4sqlite::Query q(code, sql);
        q.bindText(1, want);
        if (q.next()) {
            if (usedBy) *usedBy = what + std::to_string(q.integer(0));
            return DeleteCustomerResult::InUse;
        }
    }
    cb4sqlite::Transaction tx(code);
    Data4 d(&code, "ar_cust");
    d.SetIndex("vendorno");
    Field4 vendorno(d, "vendorno");
    const bool deleted = DeleteRecord(d, vendorno, want);
    d.close();
    tx.commit();
    return deleted ? DeleteCustomerResult::Deleted : DeleteCustomerResult::NotFound;
}

// ---- vouchers ---------------------------------------------------------------------------

std::vector<ArVoucher> LoadArVouchers(cb4sqlite::Code4& code)
{
    std::vector<ArVoucher> out;
    cb4sqlite::Query q(code, std::string("SELECT ") + kVoucherColumns +
                                 " FROM ar_btch WHERE _deleted = 0 ORDER BY batchno, _rowid");
    while (q.next()) out.push_back(readVoucher(q));
    return out;
}

std::optional<ArVoucher> FindArVoucher(cb4sqlite::Code4& code, long number)
{
    cb4sqlite::Query q(code, std::string("SELECT ") + kVoucherColumns +
                                 " FROM ar_btch WHERE _deleted = 0 AND batchno = ?1 ORDER BY _rowid LIMIT 1");
    q.bindInt(1, number);
    if (!q.next()) return std::nullopt;
    return readVoucher(q);
}

std::vector<ArLine> LoadArLines(cb4sqlite::Code4& code, long number)
{
    std::vector<ArLine> out;
    {
        cb4sqlite::Query q(code, "SELECT entryno, descript, medrep, drcode, product, account, dept, qty, price, "
                                 "debit, credit FROM ar_btchd WHERE _deleted = 0 AND batchno = ?1 "
                                 "ORDER BY entryno, _rowid");
        q.bindInt(1, number);
        while (q.next()) {
            ArLine l;
            l.line = static_cast<int>(q.integer(0));
            l.descript = q.text(1);
            l.medrep = TrimText(q.text(2));
            l.drcode = TrimText(q.text(3));
            l.product = TrimText(q.text(4));
            l.account = {TrimText(q.text(5)), TrimText(q.text(6))};
            l.qty = rpt::ToCents(q.number(7));
            l.price = rpt::ToCents(q.number(8));
            l.debit = rpt::ToCents(q.number(9));
            l.credit = rpt::ToCents(q.number(10));
            // Without an ar_linex row (Ar.h): read by its side.
            l.role = l.debit != 0 ? ArLineRole::Receivable : ArLineRole::Sales;
            l.gross = l.role == ArLineRole::Sales ? l.credit : 0;
            out.push_back(l);
        }
    }
    if (!hasTable(code, "ar_linex")) return out;
    std::map<int, std::pair<ArLineRole, std::int64_t>> extra;
    cb4sqlite::Query q(code, "SELECT entryno, role, gross FROM ar_linex WHERE _deleted = 0 AND batchno = ?1");
    q.bindInt(1, number);
    while (q.next()) extra[static_cast<int>(q.integer(0))] = {roleOf(TrimText(q.text(1))), rpt::ToCents(q.number(2))};
    for (ArLine& l : out)
        if (auto it = extra.find(l.line); it != extra.end()) {
            l.role = it->second.first;
            l.gross = l.role == ArLineRole::Sales ? it->second.second : 0;
        }
    return out;
}

long NextArVoucherNumber(cb4sqlite::Code4& code)
{
    cb4sqlite::Query q(code, "SELECT coalesce(max(batchno), 0) FROM ar_btch WHERE _deleted = 0");
    return q.next() ? static_cast<long>(q.integer(0)) + 1 : 1;
}

long SaveArVoucher(cb4sqlite::Code4& code, const ArVoucher& v, const std::vector<ArLine>& lines)
{
    cb4sqlite::Transaction tx(code);
    long number = v.number;
    {
        Data4 d(&code, "ar_btch");
        Field4 batchno(d, "batchno");
        const bool exists = seekBatch(d, batchno, number);
        if (!exists) {
            number = NextArVoucherNumber(code);
            d.appendStart();   // then blank(): blank() first would flush blanks over the current record
            d.blank();
            batchno.assignLong(number);
        }
        const std::pair<const char*, std::string> texts[] = {
            {"mode", v.mode},           {"vendorno", TrimText(v.customer)}, {"invoiceno", TrimText(v.invoiceNo)},
            {"orderno", v.orderNo},     {"orderdate", v.orderDate},       {"vin", v.vin},
            {"units", v.units},         {"reference", v.reference},       {"descript", v.descript},
            {"date", v.date},           {"invdate", v.invoiceDate},       {"duedate", v.dueDate},
            {"account", v.account.account}, {"depart", v.account.dept},   {"memofield", v.memo}};
        for (const auto& [name, value] : texts) Field4(d, name).assign(value.c_str());
        Field4(d, "period").assignLong(v.period);
        Field4(d, "terms").assignLong(v.terms);
        Field4(d, "vatpercent").assignLong(v.vatRate);
        Field4(d, "payable").assign(money(v.total).c_str());
        Field4(d, "vatamt").assign(money(v.vatAmount).c_str());
        Field4(d, "entryno").assignLong(static_cast<long>(lines.size()));
        if (exists) d.flush(); else d.append();
        d.close();
    }

    deleteRows(code, "ar_btchd", number);
    deleteRows(code, "ar_linex", number);
    {
        Data4 d(&code, "ar_btchd");
        Field4 batchno(d, "batchno"), entryno(d, "entryno"), descript(d, "descript"), medrep(d, "medrep"),
            drcode(d, "drcode"), product(d, "product"), qty(d, "qty"), price(d, "price"), account(d, "account"),
            dept(d, "dept"), debit(d, "debit"), credit(d, "credit");
        Data4 x(&code, "ar_linex");
        Field4 xbatch(x, "batchno"), xentry(x, "entryno"), xrole(x, "role"), xgross(x, "gross");
        int n = 0;
        for (const ArLine& l : lines) {
            ++n;
            d.appendStart();
            d.blank();
            batchno.assignLong(number);
            entryno.assignLong(n);
            descript.assign(l.descript.c_str());
            medrep.assign(l.medrep.c_str());
            drcode.assign(l.drcode.c_str());
            product.assign(l.product.c_str());
            qty.assign(money(l.qty).c_str());
            price.assign(money(l.price).c_str());
            account.assign(l.account.account.c_str());
            dept.assign(l.account.dept.c_str());
            debit.assign(money(l.debit).c_str());
            credit.assign(money(l.credit).c_str());
            d.append();

            x.appendStart();
            x.blank();
            xbatch.assignLong(number);
            xentry.assignLong(n);
            const char role[2] = {roleCode(l.role), 0};
            xrole.assign(role);
            xgross.assign(money(l.role == ArLineRole::Sales ? l.gross : 0).c_str());
            x.append();
        }
        d.close();
        x.close();
    }
    tx.commit();
    return number;
}

long DuplicateArInvoice(cb4sqlite::Code4& code, const std::string& customer, const std::string& invoiceNo,
                        long except)
{
    const std::string cust = TrimText(customer), inv = TrimText(invoiceNo);
    if (cust.empty() || inv.empty()) return 0;
    cb4sqlite::Query q(code, "SELECT batchno FROM ar_btch WHERE _deleted = 0 AND trim(vendorno) = ?1 "
                             "AND trim(invoiceno) = ?2 AND trim(mode) <> 'X' AND batchno <> ?3 "
                             "ORDER BY batchno, _rowid LIMIT 1");
    q.bindText(1, cust);
    q.bindText(2, inv);
    q.bindInt(3, except);
    return q.next() ? static_cast<long>(q.integer(0)) : 0;
}

// ---- the arithmetic ----------------------------------------------------------------------

std::int64_t LineAmount(std::int64_t qty, std::int64_t price, std::int64_t typed)
{
    return price == 0 ? typed : divRound(qty * price, 100);
}

SaleSplit SplitSale(std::int64_t amount, int vatRate)
{
    SaleSplit s;
    if (vatRate > 0) s.vat = divRound(amount * vatRate, 100 + vatRate);
    s.sales = amount - s.vat;
    return s;
}

ComputedSale ComputeSale(const std::vector<ArLine>& productLines, int vatRate, const SaleAccounts& accounts)
{
    ComputedSale c;
    for (ArLine l : productLines) {
        if (l.price != 0) l.gross = LineAmount(l.qty, l.price, l.gross);
        const SaleSplit s = SplitSale(l.gross, vatRate);
        l.role = ArLineRole::Sales;
        l.debit = 0;
        l.credit = s.sales;
        c.total += l.gross;
        c.vat += s.vat;
        c.lines.push_back(l);
    }
    if (c.vat != 0) {
        ArLine v;
        v.role = ArLineRole::Vat;
        v.descript = accounts.vatText;
        v.account = accounts.vat;
        v.credit = c.vat;
        c.lines.push_back(v);
    }
    if (c.total != 0) {
        ArLine a;
        a.role = ArLineRole::Receivable;
        a.descript = accounts.arText;
        a.account = accounts.ar;
        a.debit = c.total;
        c.lines.push_back(a);
    }
    for (ArLine& l : c.lines) {
        c.debits += l.debit;
        c.credits += l.credit;
    }
    int n = 0;
    for (ArLine& l : c.lines) l.line = ++n;
    return c;
}

// ---- the ledger rows ---------------------------------------------------------------------

std::vector<ArOpenItem> LoadArOpenItems(cb4sqlite::Code4& code)
{
    std::vector<ArOpenItem> out;
    cb4sqlite::Query q(code, "SELECT batchno, vendorno, invoiceno, reference, paymentctl, date, duedate, datepaid, "
                             "period, apaccount, apdepart, account, dept, amount, vatamt, amountpaid, remainder "
                             "FROM ar_dist WHERE _deleted = 0 ORDER BY batchno, _rowid");
    while (q.next()) {
        ArOpenItem i;
        i.batchno = static_cast<long>(q.integer(0));
        i.customer = TrimText(q.text(1));
        i.invoiceNo = TrimText(q.text(2));
        i.reference = q.text(3);
        i.paymentCtl = TrimText(q.text(4));
        i.date = q.text(5);
        i.dueDate = q.text(6);
        i.datePaid = q.text(7);
        i.period = static_cast<int>(q.integer(8));
        i.ar = {TrimText(q.text(9)), TrimText(q.text(10))};
        i.bank = {TrimText(q.text(11)), TrimText(q.text(12))};
        i.amount = rpt::ToCents(q.number(13));
        i.vat = rpt::ToCents(q.number(14));
        i.received = rpt::ToCents(q.number(15));
        i.remainder = rpt::ToCents(q.number(16));
        out.push_back(i);
    }
    return out;
}

std::vector<Receipt> LoadReceipts(cb4sqlite::Code4& code, long batchno)
{
    std::vector<Receipt> out;
    cb4sqlite::Query q(code, "SELECT rcptno, batchno, vendorno, invoiceno, date, period, bankacct, bankdept, "
                             "amount, discount, discvat, reference FROM ar_rcpt WHERE _deleted = 0 "
                             "AND (?1 = 0 OR batchno = ?1) ORDER BY rcptno, _rowid");
    q.bindInt(1, batchno);
    while (q.next()) {
        Receipt r;
        r.number = static_cast<long>(q.integer(0));
        r.batchno = static_cast<long>(q.integer(1));
        r.customer = TrimText(q.text(2));
        r.invoiceNo = TrimText(q.text(3));
        r.date = q.text(4);
        r.period = static_cast<int>(q.integer(5));
        r.bank = {TrimText(q.text(6)), TrimText(q.text(7))};
        r.amount = rpt::ToCents(q.number(8));
        r.discount = rpt::ToCents(q.number(9));
        r.discountVat = rpt::ToCents(q.number(10));
        r.reference = q.text(11);
        out.push_back(r);
    }
    return out;
}

std::vector<JournalRow> LoadArJournal(cb4sqlite::Code4& code)
{
    std::vector<JournalRow> out;
    cb4sqlite::Query q(code, "SELECT account, dept, trim(period), mode, date, type, batchno, reference, cheque, "
                             "descript, debit, credit FROM ar_jsum WHERE _deleted = 0 ORDER BY _rowid");
    while (q.next()) {
        JournalRow r;
        r.account = {TrimText(q.text(0)), TrimText(q.text(1))};
        r.period = static_cast<int>(q.integer(2));
        r.mode = TrimText(q.text(3));
        r.date = q.text(4);
        r.type = TrimText(q.text(5));
        r.batchno = static_cast<long>(q.integer(6));
        r.reference = q.text(7);
        r.cheque = TrimText(q.text(8));
        r.descript = q.text(9);
        r.debit = rpt::ToCents(q.number(10));
        r.credit = rpt::ToCents(q.number(11));
        out.push_back(r);
    }
    return out;
}
