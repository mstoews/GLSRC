//
// Ap.cpp - see Ap.h.
//
// Translation notes (plan per-port checklist):
//   - SplitAmount / ComputeVoucher replace TAPInvoiceEntry::GetAPExpense and
//     UpdateBatchDetail (ap_btche.cpp:817; spec #5 2.3): one set of formulas
//     in cents (D4). The original's sentinel rate of 1 for "none" (2.6 #5) is
//     0 here, and W/H is on the net in F mode (Q2), as the deployed sample.
//   - The original wrote the doctor code to PRODUCT and the product to
//     DRCODE (2.6 #7); SaveApVoucher writes each to its own column (D13).
//     Deployed rows are read as they are.
//   - The tables have no update/user/time fields, so the original's
//     UpdateUserTime calls wrote nothing; they aren't repeated.
//
#include "Ap.h"

#include "Accounts.h"       // TrimText
#include "MasterTables.h"   // SeekCode

#include "report/Format.h"  // rpt::ToCents

#include <algorithm>
#include <cctype>
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

std::string upper(std::string s)
{
    for (char& c : s) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    return s;
}

const char* headerTable(ApKind k) { return k == ApKind::Cash ? "ap_cash" : "ap_btch"; }
const char* lineTable(ApKind k) { return k == ApKind::Cash ? "ap_cashd" : "ap_btchd"; }
const char* drcodeColumn(ApKind k) { return k == ApKind::Cash ? "drcodes" : "drcode"; }
const char* kindCode(ApKind k) { return k == ApKind::Cash ? "C" : "I"; }

char roleCode(ApLineRole r)
{
    switch (r) {
    case ApLineRole::Vat: return 'V';
    case ApLineRole::WithHolding: return 'W';
    case ApLineRole::Credit: return 'C';
    default: return 'E';
    }
}

ApLineRole roleOf(const std::string& code)
{
    if (code == "V") return ApLineRole::Vat;
    if (code == "W") return ApLineRole::WithHolding;
    if (code == "C") return ApLineRole::Credit;
    return ApLineRole::Expense;
}

// The headers' and lines' tables: dbf/AP_BTCH, AP_CASH, AP_BTCHD, AP_CASHD (.DBF and .MDX).
void createVoucherTables(cb4sqlite::Code4& code, ApKind kind)
{
    const bool cash = kind == ApKind::Cash;
    std::vector<cb4sqlite::FieldDef> h = {
        {"BATCHNO", 'N', 6},   {"MODE", 'C', 1},       {"TYPE", 'C', 2},      {"ADJ", 'C', 1},
        {"PERIOD", 'N', 2},    {"VENDORNO", 'C', 6},   {"VDNAME", 'C', 40},   {"INVOICENO", 'C', cash ? 10 : 8},
    };
    if (cash) h.push_back({"CHEQUENO", 'C', 10});
    for (cb4sqlite::FieldDef f :
         std::vector<cb4sqlite::FieldDef>{{"ORDERNO", 'C', 10}, {"DUEDATE", 'D', 8},      {"INVDATE", 'D', 8},
                                          {"REFERENCE", 'C', 40}, {"DESCRIPT", 'C', 40},  {"DATE", 'D', 8},
                                          {"ACCOUNT", 'C', 7},    {"DEPART", 'C', 4},      {"PAYABLE", 'N', 16, 2},
                                          {"REBATEAMT", 'N', 16, 2}, {"ENTRYNO", 'N', 3},  {"VATPERCENT", 'N', 2},
                                          {"REBATE", 'N', 2},     {"TERMS", 'N', 3},       {"VATAMT", 'N', 16, 2}})
        h.push_back(f);
    if (cash) {
        h.push_back({"VATACCT", 'C', 7});
        h.push_back({"VATDEPT", 'C', 4});
    } else {
        h.push_back({"ADJAMT", 'N', 16, 2});
    }
    const char* ht = headerTable(kind);
    cb4sqlite::createTable(code, ht, h);
    cb4sqlite::createTag(code, ht, "account", "\"ACCOUNT\" || \"DEPART\"");
    cb4sqlite::createTag(code, ht, "batchno", "\"BATCHNO\"");
    cb4sqlite::createTag(code, ht, "invoiceno", "\"VENDORNO\" || \"INVOICENO\"");
    cb4sqlite::createTag(code, ht, "recno", "_rowid");

    const char* lt = lineTable(kind);
    cb4sqlite::createTable(code, lt,
                           {{"BATCHNO", 'N', 6},   {"ENTRYNO", 'N', 3},    {"DESCRIPT", 'C', 40},
                            {"MEDREP", 'C', 5},    {cash ? "DRCODES" : "DRCODE", 'C', 5}, {"PRODUCT", 'C', 7},
                            {"ACCOUNT", 'C', 7},   {"DEPT", 'C', 4},       {"DEBIT", 'N', 14, 2},
                            {"CREDIT", 'N', 14, 2}});
    cb4sqlite::createTag(code, lt, "batchno", "\"BATCHNO\"");
    cb4sqlite::createTag(code, lt, "recno", "_rowid");
}

// D3 on one header table: "2" + "Cancelled" -> "X".
int migrateCancelled(cb4sqlite::Code4& code, const char* table)
{
    int n = 0;
    Data4 d(&code, table);
    if (!d.isValid()) return 0;
    Field4 mode(d, "mode"), reference(d, "reference");
    for (d.top(); !d.eof(); d.skip()) {
        if (d.deleted() || TrimText(mode.str()) != ap_mode::kTransferred) continue;
        if (upper(TrimText(reference.str())) != "CANCELLED") continue;
        mode.assign(ap_mode::kCancelled);
        d.flush();
        ++n;
    }
    d.close();
    return n;
}

constexpr const char* kVendorColumns =
    "vendorno, vdshort, vdname, address1, address2, address3, postal, phone, fax, contact, descript, vin, type, "
    "medrep, status, account, depart, vat_acct, vat_dept, terms, percentage, rebate, dis_days, dis_perct";

Vendor readVendor(const cb4sqlite::Query& q)
{
    Vendor v;
    v.number = q.text(0);
    v.shortName = q.text(1);
    v.name = q.text(2);
    v.address1 = q.text(3);
    v.address2 = q.text(4);
    v.address3 = q.text(5);
    v.postal = q.text(6);
    v.phone = q.text(7);
    v.fax = q.text(8);
    v.contact = q.text(9);
    v.descript = q.text(10);
    v.vin = q.text(11);
    v.type = TrimText(q.text(12));
    v.medrep = q.text(13);
    v.status = q.text(14);
    v.ap = {TrimText(q.text(15)), TrimText(q.text(16))};
    v.vat = {TrimText(q.text(17)), TrimText(q.text(18))};
    v.terms = static_cast<int>(q.integer(19));
    v.vatRate = static_cast<int>(q.integer(20));
    v.whRate = static_cast<int>(q.integer(21));
    v.discountDays = static_cast<int>(q.integer(22));
    v.discountPercent = static_cast<int>(q.integer(23));
    return v;
}

std::string voucherColumns(ApKind k)
{
    std::string c = "batchno, mode, vendorno, vdname, invoiceno, orderno, reference, descript, date, invdate, duedate, "
                    "period, terms, vatpercent, rebate, account, depart, payable, vatamt, rebateamt, entryno";
    c += k == ApKind::Cash ? ", chequeno, vatacct, vatdept" : ", '', '', ''";
    return c;
}

ApVoucher readVoucher(const cb4sqlite::Query& q, ApKind kind)
{
    ApVoucher v;
    v.kind = kind;
    v.number = static_cast<long>(q.integer(0));
    v.mode = TrimText(q.text(1));
    v.vendor = TrimText(q.text(2));
    v.vendorName = q.text(3);
    v.invoiceNo = TrimText(q.text(4));
    v.orderNo = q.text(5);
    v.reference = q.text(6);
    v.descript = q.text(7);
    v.date = q.text(8);
    v.invoiceDate = q.text(9);
    v.dueDate = q.text(10);
    v.period = static_cast<int>(q.integer(11));
    v.terms = static_cast<int>(q.integer(12));
    v.vatRate = static_cast<int>(q.integer(13));
    v.whRate = static_cast<int>(q.integer(14));
    v.account = {TrimText(q.text(15)), TrimText(q.text(16))};
    v.payable = rpt::ToCents(q.number(17));
    v.vatAmount = rpt::ToCents(q.number(18));
    v.whAmount = rpt::ToCents(q.number(19));
    v.lines = static_cast<int>(q.integer(20));
    v.cheque = TrimText(q.text(21));
    v.vat = {TrimText(q.text(22)), TrimText(q.text(23))};
    return v;
}

// Seeks voucher `number` in a header or line table on its batchno tag.
bool seekBatch(Data4& d, const Field4& batchno, long number)
{
    if (!d.isValid() || number <= 0) return false;
    d.SetIndex("batchno");
    return d.seek(static_cast<double>(number)) == r4success && std::atol(batchno.str()) == number;
}

// Deletes every row of voucher `number` from a line table (or ap_linex,
// filtered by kind) and packs it if anything went. No transaction of its own.
void deleteRows(cb4sqlite::Code4& code, const char* table, long number, const char* kind = nullptr)
{
    Data4 d(&code, table);
    if (!d.isValid()) return;
    Field4 batchno(d, "batchno");
    std::optional<Field4> k;
    if (kind) k.emplace(d, "kind");
    bool any = false;
    if (seekBatch(d, batchno, number))
        for (; !d.eof() && std::atol(batchno.str()) == number; d.skip())
            if (!d.deleted() && (!k || TrimText(k->str()) == kind)) {
                d.deleteRec();
                any = true;
            }
    if (any) d.pack();
    d.close();
}

} // namespace

// ---- tables -----------------------------------------------------------------------

int EnsureApTables(cb4sqlite::Code4& code)
{
    const char* names[] = {"ap_vend", "ap_btch", "ap_btchd", "ap_cash", "ap_cashd",
                           "ap_dist", "ap_reqst", "ap_jsum", "ap_linex"};
    bool have[9] = {};
    bool all = true;
    for (int i = 0; i < 9; ++i) all = (have[i] = hasTable(code, names[i])) && all;
    if (!all) {
        cb4sqlite::Transaction tx(code);
        if (!have[0]) {   // dbf/AP_VEND
            cb4sqlite::createTable(code, "ap_vend",
                                   {{"VENDORNO", 'C', 6},  {"VDSHORT", 'C', 12},  {"VDNAME", 'C', 40},
                                    {"ADDRESS1", 'C', 30}, {"ADDRESS2", 'C', 30}, {"ADDRESS3", 'C', 30},
                                    {"POSTAL", 'C', 7},    {"PHONE", 'C', 16},    {"FAX", 'C', 16},
                                    {"DEPART", 'C', 4},    {"ACCOUNT", 'C', 7},   {"VAT_DEPT", 'C', 4},
                                    {"VAT_ACCT", 'C', 7},  {"AP_ACCT", 'C', 7},   {"AP_DEPT", 'C', 4},
                                    {"DESCRIPT", 'C', 40}, {"CONTACT", 'C', 12},  {"TYPE", 'C', 3},
                                    {"MEDREP", 'C', 6},    {"STATUS", 'C', 3},    {"TERMS", 'N', 3},
                                    {"VAT", 'N', 1},       {"FIVE", 'N', 1},      {"THREE", 'N', 1},
                                    {"PERCENTAGE", 'N', 2}, {"REBATE", 'N', 2},   {"VIN", 'C', 10},
                                    {"DIS_DAYS", 'N', 2},  {"DIS_PERCT", 'N', 2}});
            cb4sqlite::createTag(code, "ap_vend", "vendorno", "\"VENDORNO\"");
        }
        // A header table and its lines go together: a company has both or neither.
        if (!have[1] && !have[2]) createVoucherTables(code, ApKind::Invoice);
        if (!have[3] && !have[4]) createVoucherTables(code, ApKind::Cash);
        if (!have[5]) {   // dbf/AP_DIST
            cb4sqlite::createTable(code, "ap_dist",
                                   {{"ACCOUNT", 'C', 7},     {"DEPT", 'C', 4},        {"APACCOUNT", 'C', 7},
                                    {"APDEPART", 'C', 4},    {"PERIOD", 'N', 2},      {"DATE", 'D', 8},
                                    {"DUEDATE", 'D', 8},     {"BATCHNO", 'N', 6},     {"INVOICENO", 'C', 8},
                                    {"PAYMENTCTL", 'C', 1},  {"VENDORNO", 'C', 6},    {"CHEQUENO", 'N', 10},
                                    {"REFERENCE", 'C', 40},  {"DESCRIPT", 'C', 40},   {"AMOUNT", 'N', 16, 2},
                                    {"REBATEAMT", 'N', 16, 2}, {"AMOUNTPAID", 'N', 16, 2}, {"PAYMENTREQ", 'N', 10},
                                    {"DATEPAID", 'D', 8},    {"VATAMT", 'N', 16, 2},  {"ADJUSTMENT", 'N', 16, 2},
                                    {"REBATE", 'C', 2}});
            cb4sqlite::createTag(code, "ap_dist", "account", "\"ACCOUNT\"");
            cb4sqlite::createTag(code, "ap_dist", "batchno", "\"BATCHNO\"");
            cb4sqlite::createTag(code, "ap_dist", "chequeno", "\"CHEQUENO\"");
            cb4sqlite::createTag(code, "ap_dist", "cheques", "\"VENDORNO\"",
                                 "(substr(\"PAYMENTCTL\", 1, length('P')) = 'P')");
            cb4sqlite::createTag(code, "ap_dist", "dept", "\"DEPT\"");
            cb4sqlite::createTag(code, "ap_dist", "invoiceno", "\"VENDORNO\" || \"INVOICENO\"");
            cb4sqlite::createTag(code, "ap_dist", "paymentreq", "\"PAYMENTREQ\"");
            cb4sqlite::createTag(code, "ap_dist", "vendorno", "\"VENDORNO\"");
        }
        if (!have[6]) {   // dbf/AP_REQST
            cb4sqlite::createTable(code, "ap_reqst",
                                   {{"VENDORNO", 'C', 6},  {"PAYMENTREQ", 'N', 10}, {"REFERENCE", 'C', 20},
                                    {"AMOUNTPAID", 'N', 16, 2}, {"DATEPAID", 'D', 8}, {"CHEQUE", 'N', 10},
                                    {"BANKACCT", 'C', 7},  {"BANKDEPT", 'C', 4},    {"CLEARED", 'C', 1}});
            cb4sqlite::createTag(code, "ap_reqst", "paymentreq", "\"PAYMENTREQ\"");
            cb4sqlite::createTag(code, "ap_reqst", "vendorno", "\"VENDORNO\"");
        }
        if (!have[7]) {   // dbf/AP_JSUM
            cb4sqlite::createTable(code, "ap_jsum",
                                   {{"ACCOUNT", 'C', 7},   {"DEPT", 'C', 4},      {"PERIOD", 'C', 4},
                                    {"MODE", 'C', 1},      {"DATE", 'D', 8},      {"TYPE", 'C', 3},
                                    {"BATCHNO", 'N', 6},   {"REFERENCE", 'C', 30}, {"CHEQUE", 'C', 7},
                                    {"DESCRIPT", 'C', 40}, {"DEBIT", 'N', 16, 2}, {"CREDIT", 'N', 16, 2}});
            cb4sqlite::createTag(code, "ap_jsum", "account", "\"ACCOUNT\" || \"DEPT\"");
            cb4sqlite::createTag(code, "ap_jsum", "apdist", "\"BATCHNO\"",
                                 "(substr(\"TYPE\", 1, length('AP ')) = 'AP ')");
            cb4sqlite::createTag(code, "ap_jsum", "ardist", "\"BATCHNO\"",
                                 "(substr(\"TYPE\", 1, length('AR ')) = 'AR ')");
            cb4sqlite::createTag(code, "ap_jsum", "batchno", "\"BATCHNO\"");
            cb4sqlite::createTag(code, "ap_jsum", "descript", "\"DESCRIPT\"");
        }
        if (!have[8]) {   // the port's own (Ap.h)
            cb4sqlite::createTable(code, "ap_linex",
                                   {{"KIND", 'C', 1}, {"BATCHNO", 'N', 6}, {"ENTRYNO", 'N', 3}, {"ROLE", 'C', 1},
                                    {"GROSS", 'N', 16, 2}});
            cb4sqlite::createTag(code, "ap_linex", "batchno", "\"BATCHNO\"");
        }
        tx.commit();
    }

    cb4sqlite::Transaction tx(code);
    const int n = migrateCancelled(code, "ap_btch") + migrateCancelled(code, "ap_cash");
    tx.commit();
    return n;
}

// ---- accounts ------------------------------------------------------------------------

ApAccounts LoadApAccounts(cb4sqlite::Code4& code)
{
    ApAccounts a;
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
        ApAccount* slot = shortName == "AP"     ? &a.ap
                          : shortName == "VATR" ? &a.vatInput
                          : shortName == "VATP" ? &a.vatPayable
                          : shortName == "WTXC" ? &a.whCompany
                          : shortName == "WTXI" ? &a.whIndividual
                                                : nullptr;
        if (slot && slot->empty()) *slot = acct;
    }
    return a;
}

// ---- vendors ---------------------------------------------------------------------------

std::vector<Vendor> LoadVendors(cb4sqlite::Code4& code)
{
    std::vector<Vendor> out;
    cb4sqlite::Query q(code, std::string("SELECT ") + kVendorColumns +
                                 " FROM ap_vend WHERE _deleted = 0 ORDER BY vendorno, _rowid");
    while (q.next()) out.push_back(readVendor(q));
    return out;
}

std::optional<Vendor> FindVendor(cb4sqlite::Code4& code, const std::string& number)
{
    cb4sqlite::Query q(code, std::string("SELECT ") + kVendorColumns +
                                 " FROM ap_vend WHERE _deleted = 0 AND trim(vendorno) = ?1 ORDER BY _rowid LIMIT 1");
    q.bindText(1, TrimText(number));
    if (!q.next()) return std::nullopt;
    return readVendor(q);
}

bool SaveVendor(cb4sqlite::Code4& code, const Vendor& v)
{
    cb4sqlite::Transaction tx(code);
    Data4 d(&code, "ap_vend");
    d.SetIndex("vendorno");
    Field4 vendorno(d, "vendorno");
    const bool added = !SeekCode(d, vendorno, v.number);
    if (added) {
        d.appendStart();   // then blank(): blank() first would flush blanks over the current record
        d.blank();
    }
    const std::pair<const char*, const std::string*> texts[] = {
        {"vendorno", &v.number}, {"vdshort", &v.shortName}, {"vdname", &v.name},     {"address1", &v.address1},
        {"address2", &v.address2}, {"address3", &v.address3}, {"postal", &v.postal}, {"phone", &v.phone},
        {"fax", &v.fax},         {"contact", &v.contact},   {"descript", &v.descript}, {"vin", &v.vin},
        {"type", &v.type},       {"medrep", &v.medrep},     {"status", &v.status},   {"account", &v.ap.account},
        {"depart", &v.ap.dept},  {"vat_acct", &v.vat.account}, {"vat_dept", &v.vat.dept}};
    for (const auto& [name, value] : texts) Field4(d, name).assign(value->c_str());
    const std::pair<const char*, int> numbers[] = {{"terms", v.terms},          {"percentage", v.vatRate},
                                                   {"rebate", v.whRate},        {"dis_days", v.discountDays},
                                                   {"dis_perct", v.discountPercent}};
    for (const auto& [name, value] : numbers) Field4(d, name).assignLong(value);
    if (added) d.append(); else d.flush();
    d.close();
    tx.commit();
    return added;
}

// ---- vouchers ---------------------------------------------------------------------------

std::vector<ApVoucher> LoadApVouchers(cb4sqlite::Code4& code, ApKind kind)
{
    std::vector<ApVoucher> out;
    cb4sqlite::Query q(code, "SELECT " + voucherColumns(kind) + " FROM " + headerTable(kind) +
                                 " WHERE _deleted = 0 ORDER BY batchno, _rowid");
    while (q.next()) out.push_back(readVoucher(q, kind));
    return out;
}

std::optional<ApVoucher> FindApVoucher(cb4sqlite::Code4& code, ApKind kind, long number)
{
    cb4sqlite::Query q(code, "SELECT " + voucherColumns(kind) + " FROM " + headerTable(kind) +
                                 " WHERE _deleted = 0 AND batchno = ?1 ORDER BY _rowid LIMIT 1");
    q.bindInt(1, number);
    if (!q.next()) return std::nullopt;
    return readVoucher(q, kind);
}

std::vector<ApLine> LoadApLines(cb4sqlite::Code4& code, ApKind kind, long number)
{
    std::vector<ApLine> out;
    {
        cb4sqlite::Query q(code, std::string("SELECT entryno, descript, medrep, ") + drcodeColumn(kind) +
                                     ", product, account, dept, debit, credit FROM " + lineTable(kind) +
                                     " WHERE _deleted = 0 AND batchno = ?1 ORDER BY entryno, _rowid");
        q.bindInt(1, number);
        while (q.next()) {
            ApLine l;
            l.line = static_cast<int>(q.integer(0));
            l.descript = q.text(1);
            l.medrep = TrimText(q.text(2));
            l.drcode = TrimText(q.text(3));
            l.product = TrimText(q.text(4));
            l.account = {TrimText(q.text(5)), TrimText(q.text(6))};
            l.debit = rpt::ToCents(q.number(7));
            l.credit = rpt::ToCents(q.number(8));
            out.push_back(l);
        }
    }

    std::map<int, std::pair<ApLineRole, std::int64_t>> extra;
    if (hasTable(code, "ap_linex")) {
        cb4sqlite::Query q(code, "SELECT entryno, role, gross FROM ap_linex WHERE _deleted = 0 AND kind = ?1 "
                                 "AND batchno = ?2");
        q.bindText(1, kindCode(kind));
        q.bindInt(2, number);
        while (q.next())
            extra[static_cast<int>(q.integer(0))] = {roleOf(TrimText(q.text(1))), rpt::ToCents(q.number(2))};
    }
    if (!extra.empty()) {
        for (ApLine& l : out)
            if (auto it = extra.find(l.line); it != extra.end()) {
                l.role = it->second.first;
                l.gross = l.role == ApLineRole::Expense ? it->second.second : 0;
            }
        return out;
    }

    // A deployed voucher (Ap.h).
    const std::optional<ApVoucher> v = FindApVoucher(code, kind, number);
    if (!v) return out;
    const TaxMode mode = v->whAmount > 0 ? TaxMode::T : TaxMode::F;
    bool vatSeen = false, whSeen = false, creditSeen = false;
    for (ApLine& l : out) {
        if (l.credit != 0 && l.debit == 0) {
            if (!creditSeen && l.account == v->account) {
                l.role = ApLineRole::Credit;
                creditSeen = true;
            } else {
                l.role = ApLineRole::WithHolding;
                whSeen = true;
            }
        } else if (!vatSeen && v->vatAmount > 0 && l.debit == v->vatAmount) {
            l.role = ApLineRole::Vat;
            vatSeen = true;
        } else if (!whSeen && v->whAmount > 0 && l.debit == v->whAmount) {
            l.role = ApLineRole::WithHolding;
            whSeen = true;
        } else {
            l.role = ApLineRole::Expense;
            l.gross = GrossFromExpense(l.debit, v->vatRate, v->whRate, mode);
        }
    }
    return out;
}

long NextApVoucherNumber(cb4sqlite::Code4& code, ApKind kind)
{
    cb4sqlite::Query q(code, std::string("SELECT coalesce(max(batchno), 0) FROM ") + headerTable(kind) +
                                 " WHERE _deleted = 0");
    return q.next() ? static_cast<long>(q.integer(0)) + 1 : 1;
}

long SaveApVoucher(cb4sqlite::Code4& code, const ApVoucher& v, const std::vector<ApLine>& lines)
{
    cb4sqlite::Transaction tx(code);
    const bool cash = v.kind == ApKind::Cash;
    long number = v.number;
    {
        Data4 d(&code, headerTable(v.kind));
        Field4 batchno(d, "batchno");
        const bool exists = seekBatch(d, batchno, number);
        if (!exists) {
            number = NextApVoucherNumber(code, v.kind);
            d.appendStart();   // then blank(): blank() first would flush blanks over the current record
            d.blank();
            batchno.assignLong(number);
        }
        const std::pair<const char*, std::string> texts[] = {
            {"mode", v.mode},         {"vendorno", TrimText(v.vendor)}, {"vdname", v.vendorName},
            {"invoiceno", TrimText(v.invoiceNo)}, {"orderno", v.orderNo}, {"reference", v.reference},
            {"descript", v.descript}, {"date", v.date},               {"invdate", v.invoiceDate},
            {"duedate", v.dueDate},   {"account", v.account.account}, {"depart", v.account.dept}};
        for (const auto& [name, value] : texts) Field4(d, name).assign(value.c_str());
        if (cash) {
            Field4(d, "chequeno").assign(TrimText(v.cheque).c_str());
            Field4(d, "vatacct").assign(v.vat.account.c_str());
            Field4(d, "vatdept").assign(v.vat.dept.c_str());
        }
        Field4(d, "period").assignLong(v.period);
        Field4(d, "terms").assignLong(v.terms);
        Field4(d, "vatpercent").assignLong(v.vatRate);
        Field4(d, "rebate").assignLong(v.whRate);
        Field4(d, "payable").assign(money(v.payable).c_str());
        Field4(d, "vatamt").assign(money(v.vatAmount).c_str());
        Field4(d, "rebateamt").assign(money(v.whAmount).c_str());
        Field4(d, "entryno").assignLong(static_cast<long>(lines.size()));
        if (exists) d.flush(); else d.append();
        d.close();
    }

    deleteRows(code, lineTable(v.kind), number);
    deleteRows(code, "ap_linex", number, kindCode(v.kind));
    {
        Data4 d(&code, lineTable(v.kind));
        Field4 batchno(d, "batchno"), entryno(d, "entryno"), descript(d, "descript"), medrep(d, "medrep"),
            drcode(d, drcodeColumn(v.kind)), product(d, "product"), account(d, "account"), dept(d, "dept"),
            debit(d, "debit"), credit(d, "credit");
        Data4 x(&code, "ap_linex");
        Field4 xkind(x, "kind"), xbatch(x, "batchno"), xentry(x, "entryno"), xrole(x, "role"), xgross(x, "gross");
        int n = 0;
        for (const ApLine& l : lines) {
            ++n;
            d.appendStart();
            d.blank();
            batchno.assignLong(number);
            entryno.assignLong(n);
            descript.assign(l.descript.c_str());
            medrep.assign(l.medrep.c_str());
            drcode.assign(l.drcode.c_str());
            product.assign(l.product.c_str());
            account.assign(l.account.account.c_str());
            dept.assign(l.account.dept.c_str());
            debit.assign(money(l.debit).c_str());
            credit.assign(money(l.credit).c_str());
            d.append();

            x.appendStart();
            x.blank();
            xkind.assign(kindCode(v.kind));
            xbatch.assignLong(number);
            xentry.assignLong(n);
            const char role[2] = {roleCode(l.role), 0};
            xrole.assign(role);
            xgross.assign(money(l.role == ApLineRole::Expense ? l.gross : 0).c_str());
            x.append();
        }
        d.close();
        x.close();
    }
    tx.commit();
    return number;
}

std::optional<ApVoucherRef> DuplicateInvoice(cb4sqlite::Code4& code, const std::string& vendor,
                                             const std::string& invoiceNo, std::optional<ApVoucherRef> except)
{
    const std::string vend = TrimText(vendor), inv = TrimText(invoiceNo);
    if (vend.empty() || inv.empty()) return std::nullopt;
    for (const ApKind kind : {ApKind::Invoice, ApKind::Cash}) {
        cb4sqlite::Query q(code, std::string("SELECT batchno FROM ") + headerTable(kind) +
                                     " WHERE _deleted = 0 AND trim(vendorno) = ?1 AND trim(invoiceno) = ?2 "
                                     "AND trim(mode) <> 'X' ORDER BY batchno, _rowid");
        q.bindText(1, vend);
        q.bindText(2, inv);
        while (q.next()) {
            const long n = static_cast<long>(q.integer(0));
            if (except && except->kind == kind && except->number == n) continue;
            return ApVoucherRef{kind, n};
        }
    }
    return std::nullopt;
}

// ---- the arithmetic ----------------------------------------------------------------------

TaxMode ParseTaxMode(const std::string& iniValue)
{
    return upper(TrimText(iniValue)) == "T" ? TaxMode::T : TaxMode::F;
}

AmountSplit SplitAmount(std::int64_t amount, int vatRate, int whRate, TaxMode mode)
{
    const int v = std::max(0, vatRate), w = std::max(0, whRate);
    AmountSplit s;
    if (mode == TaxMode::F) {
        s.vat = divRound(amount * v, 100 + v);
        s.expense = amount - s.vat;
        s.wh = divRound(s.expense * w, 100);
    } else {
        s.expense = divRound(amount * 100, 100 + v + w);
        s.vat = divRound(s.expense * v, 100);
        s.wh = amount - s.expense - s.vat;
    }
    return s;
}

std::int64_t GrossFromExpense(std::int64_t expense, int vatRate, int whRate, TaxMode mode)
{
    const int v = std::max(0, vatRate), w = std::max(0, whRate);
    const std::int64_t gross = expense + divRound(expense * v, 100);
    return mode == TaxMode::F ? gross : gross + divRound(expense * w, 100);
}

ComputedVoucher ComputeVoucher(const std::vector<ApLine>& expenseLines, int vatRate, int whRate, TaxMode mode,
                               const VoucherAccounts& accounts)
{
    ComputedVoucher c;
    for (ApLine l : expenseLines) {
        const AmountSplit s = SplitAmount(l.gross, vatRate, whRate, mode);
        l.role = ApLineRole::Expense;
        l.debit = s.expense;
        l.credit = 0;
        c.lines.push_back(l);
        c.payable += l.gross;
        c.vat += s.vat;
        c.wh += s.wh;
    }
    c.credit = mode == TaxMode::F ? c.payable - c.wh : c.payable;
    auto add = [&](ApLineRole role, const ApAccount& account, const std::string& text, std::int64_t dr, std::int64_t cr) {
        if (dr == 0 && cr == 0) return;
        ApLine l;
        l.role = role;
        l.account = account;
        l.descript = text;
        l.debit = dr;
        l.credit = cr;
        c.lines.push_back(l);
    };
    add(ApLineRole::Vat, accounts.vat, accounts.vatText, c.vat, 0);
    if (mode == TaxMode::F)
        add(ApLineRole::WithHolding, accounts.wh, accounts.whText, 0, c.wh);
    else
        add(ApLineRole::WithHolding, accounts.wh, accounts.whText, c.wh, 0);
    add(ApLineRole::Credit, accounts.credit, accounts.creditText, 0, c.credit);
    int n = 0;
    for (ApLine& l : c.lines) {
        l.line = ++n;
        c.debits += l.debit;
        c.credits += l.credit;
    }
    return c;
}

// ---- the ledger rows ----------------------------------------------------------------------

std::vector<OpenItem> LoadOpenItems(cb4sqlite::Code4& code)
{
    std::vector<OpenItem> out;
    cb4sqlite::Query q(code, "SELECT batchno, vendorno, invoiceno, reference, paymentctl, date, duedate, datepaid, "
                             "period, apaccount, apdepart, account, dept, chequeno, amount, vatamt, rebateamt, "
                             "amountpaid FROM ap_dist WHERE _deleted = 0 ORDER BY batchno, _rowid");
    while (q.next()) {
        OpenItem i;
        i.batchno = static_cast<long>(q.integer(0));
        i.vendor = TrimText(q.text(1));
        i.invoiceNo = TrimText(q.text(2));
        i.reference = q.text(3);
        i.paymentCtl = TrimText(q.text(4));
        i.date = q.text(5);
        i.dueDate = q.text(6);
        i.datePaid = q.text(7);
        i.period = static_cast<int>(q.integer(8));
        i.ap = {TrimText(q.text(9)), TrimText(q.text(10))};
        i.bank = {TrimText(q.text(11)), TrimText(q.text(12))};
        i.cheque = static_cast<long>(q.integer(13));
        i.amount = rpt::ToCents(q.number(14));
        i.vat = rpt::ToCents(q.number(15));
        i.wh = rpt::ToCents(q.number(16));
        i.paid = rpt::ToCents(q.number(17));
        out.push_back(i);
    }
    return out;
}

std::vector<Cheque> LoadCheques(cb4sqlite::Code4& code)
{
    std::vector<Cheque> out;
    cb4sqlite::Query q(code, "SELECT cheque, paymentreq, vendorno, reference, datepaid, bankacct, bankdept, "
                             "amountpaid, cleared FROM ap_reqst WHERE _deleted = 0 ORDER BY cheque, _rowid");
    while (q.next()) {
        Cheque c;
        c.number = static_cast<long>(q.integer(0));
        c.request = static_cast<long>(q.integer(1));
        c.vendor = TrimText(q.text(2));
        c.reference = q.text(3);
        c.date = q.text(4);
        c.bank = {TrimText(q.text(5)), TrimText(q.text(6))};
        c.amount = rpt::ToCents(q.number(7));
        c.cleared = TrimText(q.text(8)) == "1";
        out.push_back(c);
    }
    return out;
}

std::vector<JournalRow> LoadJournal(cb4sqlite::Code4& code)
{
    std::vector<JournalRow> out;
    cb4sqlite::Query q(code, "SELECT account, dept, trim(period), mode, date, type, batchno, reference, cheque, "
                             "descript, debit, credit FROM ap_jsum WHERE _deleted = 0 ORDER BY _rowid");
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
