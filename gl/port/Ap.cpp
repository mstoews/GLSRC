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
#include "Assets.h"         // ValidateAccount
#include "MasterTables.h"   // SeekCode, DeleteRecord
#include "Vouchers.h"       // SaveLine (the transfer)

#include "report/Format.h"  // rpt::ToCents

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <stdexcept>

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
                           "ap_dist", "ap_reqst", "ap_jsum", "ap_linex", "ap_xfer"};
    bool have[10] = {};
    bool all = true;
    for (int i = 0; i < 10; ++i) all = (have[i] = hasTable(code, names[i])) && all;
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
        if (!have[9]) {   // the port's own (Ap.h)
            cb4sqlite::createTable(code, "ap_xfer",
                                   {{"JSUMROW", 'N', 10}, {"GLBATCH", 'N', 6}, {"PERIOD", 'N', 2}});
            cb4sqlite::createTag(code, "ap_xfer", "glbatch", "\"GLBATCH\"");
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

std::optional<VendorProblem> CheckVendor(cb4sqlite::Code4& code, const Vendor& v)
{
    using K = VendorProblem::Kind;
    if (TrimText(v.number).empty()) return VendorProblem{VendorField::Number, K::Required};
    if (TrimText(v.shortName).empty()) return VendorProblem{VendorField::ShortName, K::Required};
    if (TrimText(v.name).empty()) return VendorProblem{VendorField::Name, K::Required};
    if (v.type != "C" && v.type != "I") return VendorProblem{VendorField::Type, K::NoType};
    if (v.vatRate < 0 || v.vatRate > 99) return VendorProblem{VendorField::VatRate, K::BadRate};
    if (v.whRate < 0 || v.whRate > 99) return VendorProblem{VendorField::WhRate, K::BadRate};
    if (!ValidateAccount(code, v.ap.account, v.ap.dept)) return VendorProblem{VendorField::ApAccount, K::BadAccount};
    if (v.vat.empty()) {
        if (v.vatRate > 0) return VendorProblem{VendorField::VatAccount, K::VatAccountNeeded};
    } else if (!ValidateAccount(code, v.vat.account, v.vat.dept)) {
        return VendorProblem{VendorField::VatAccount, K::BadAccount};
    }
    return std::nullopt;
}

DeleteVendorResult DeleteVendor(cb4sqlite::Code4& code, const std::string& number, std::string* usedBy)
{
    const std::string want = TrimText(number);
    const std::pair<const char*, const char*> uses[] = {
        {"SELECT batchno FROM ap_btch WHERE _deleted = 0 AND trim(vendorno) = ?1 ORDER BY batchno LIMIT 1",
         "invoice voucher "},
        {"SELECT batchno FROM ap_cash WHERE _deleted = 0 AND trim(vendorno) = ?1 ORDER BY batchno LIMIT 1",
         "cash voucher "},
        {"SELECT batchno FROM ap_dist WHERE _deleted = 0 AND trim(vendorno) = ?1 ORDER BY batchno LIMIT 1",
         "the open item of voucher "},
        {"SELECT cheque FROM ap_reqst WHERE _deleted = 0 AND trim(vendorno) = ?1 ORDER BY cheque LIMIT 1",
         "cheque "},
    };
    for (const auto& [sql, what] : uses) {
        cb4sqlite::Query q(code, sql);
        q.bindText(1, want);
        if (q.next()) {
            if (usedBy) *usedBy = what + std::to_string(q.integer(0));
            return DeleteVendorResult::InUse;
        }
    }
    cb4sqlite::Transaction tx(code);
    Data4 d(&code, "ap_vend");
    d.SetIndex("vendorno");
    Field4 vendorno(d, "vendorno");
    const bool deleted = DeleteRecord(d, vendorno, want);
    d.close();
    tx.commit();
    return deleted ? DeleteVendorResult::Deleted : DeleteVendorResult::NotFound;
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

// ---- invoice entry ------------------------------------------------------------------------

namespace {

// Days since 1970-01-01 for a civil date, and back (H. Hinnant's algorithms).
long long daysFromCivil(int y, unsigned m, unsigned d)
{
    y -= m <= 2;
    const long long era = (y >= 0 ? y : y - 399) / 400;
    const unsigned yoe = static_cast<unsigned>(y - era * 400);
    const unsigned doy = (153 * (m > 2 ? m - 3 : m + 9) + 2) / 5 + d - 1;
    const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097 + static_cast<long long>(doe) - 719468;
}

void civilFromDays(long long z, int& y, unsigned& m, unsigned& d)
{
    z += 719468;
    const long long era = (z >= 0 ? z : z - 146096) / 146097;
    const unsigned doe = static_cast<unsigned>(z - era * 146097);
    const unsigned yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    const unsigned doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    const unsigned mp = (5 * doy + 2) / 153;
    d = doy - (153 * mp + 2) / 5 + 1;
    m = mp < 10 ? mp + 3 : mp - 9;
    y = static_cast<int>(static_cast<long long>(yoe) + era * 400) + (m <= 2);
}

// The voucher's expense lines, as entries.
std::vector<ExpenseEntry> expenseEntries(const std::vector<ApLine>& lines)
{
    std::vector<ExpenseEntry> out;
    for (const ApLine& l : lines)
        if (l.role == ApLineRole::Expense)
            out.push_back({0, l.descript, l.medrep, l.drcode, l.product, l.account, l.gross});
    return out;
}

long saveComputed(cb4sqlite::Code4& code, ApVoucher header, const std::vector<ExpenseEntry>& entries, TaxMode mode,
                  VoucherAccounts accounts)
{
    if (accounts.vatText.empty()) accounts.vatText = AccountName(code, accounts.vat);
    if (accounts.whText.empty()) accounts.whText = AccountName(code, accounts.wh);
    if (accounts.creditText.empty()) accounts.creditText = AccountName(code, accounts.credit);
    std::vector<ApLine> in;
    for (const ExpenseEntry& e : entries) {
        ApLine l;
        l.descript = TrimText(e.descript).empty() ? AccountName(code, e.account) : e.descript;
        l.medrep = e.medrep;
        l.drcode = e.drcode;
        l.product = e.product;
        l.account = e.account;
        l.gross = e.gross;
        in.push_back(l);
    }
    const ComputedVoucher c = ComputeVoucher(in, header.vatRate, header.whRate, mode, accounts);
    header.payable = c.payable;
    header.vatAmount = c.vat;
    header.whAmount = mode == TaxMode::F ? -c.wh : c.wh;
    return SaveApVoucher(code, header, c.lines);
}

} // namespace

std::string AddDays(const std::string& yyyymmdd, int days)
{
    const std::string s = TrimText(yyyymmdd);
    if (s.size() != 8 || s.find_first_not_of("0123456789") != std::string::npos) return {};
    const int y = std::atoi(s.substr(0, 4).c_str());
    const unsigned m = static_cast<unsigned>(std::atoi(s.substr(4, 2).c_str()));
    const unsigned d = static_cast<unsigned>(std::atoi(s.substr(6, 2).c_str()));
    if (m < 1 || m > 12 || d < 1 || d > 31) return {};
    int ry = 0;
    unsigned rm = 0, rd = 0;
    civilFromDays(daysFromCivil(y, m, d) + days, ry, rm, rd);
    char buf[16];
    std::snprintf(buf, sizeof buf, "%04d%02u%02u", ry, rm, rd);
    return buf;
}

long DaysBetween(const std::string& from, const std::string& to)
{
    auto days = [](const std::string& ymd, long long& out) {
        const std::string t = TrimText(ymd);
        if (t.size() != 8 || t.find_first_not_of("0123456789") != std::string::npos) return false;
        out = daysFromCivil(std::atoi(t.substr(0, 4).c_str()), static_cast<unsigned>(std::atoi(t.substr(4, 2).c_str())),
                            static_cast<unsigned>(std::atoi(t.substr(6, 2).c_str())));
        return true;
    };
    long long a = 0, b = 0;
    if (!days(from, a) || !days(to, b)) return 0;
    return static_cast<long>(b - a);
}

std::string AccountName(cb4sqlite::Code4& code, const ApAccount& account)
{
    if (account.empty()) return {};
    cb4sqlite::Query q(code, "SELECT descript FROM glaccts WHERE _deleted = 0 AND trim(account) = ?1 "
                             "AND coalesce(trim(dept), '') = ?2 ORDER BY _rowid LIMIT 1");
    q.bindText(1, TrimText(account.account));
    q.bindText(2, TrimText(account.dept));
    return q.next() ? q.text(0) : std::string();
}

long SaveExpenseLine(cb4sqlite::Code4& code, ApVoucher header, const ExpenseEntry& entry, TaxMode mode,
                     VoucherAccounts accounts)
{
    std::vector<ExpenseEntry> entries;
    const std::optional<ApVoucher> onFile =
        header.number > 0 ? FindApVoucher(code, header.kind, header.number) : std::nullopt;
    if (onFile) {
        if (onFile->mode != ap_mode::kOpen) return 0;
        header.mode = onFile->mode;
        entries = expenseEntries(LoadApLines(code, header.kind, header.number));
    } else {
        header.number = 0;
        header.mode = ap_mode::kOpen;
    }
    if (entry.line >= 1 && entry.line <= static_cast<int>(entries.size()))
        entries[static_cast<size_t>(entry.line - 1)] = entry;
    else
        entries.push_back(entry);
    return saveComputed(code, header, entries, mode, accounts);
}

bool DeleteExpenseLine(cb4sqlite::Code4& code, ApKind kind, long number, int line, TaxMode mode,
                       VoucherAccounts accounts)
{
    const std::optional<ApVoucher> v = FindApVoucher(code, kind, number);
    if (!v || v->mode != ap_mode::kOpen) return false;
    std::vector<ExpenseEntry> entries = expenseEntries(LoadApLines(code, kind, number));
    if (line < 1 || line > static_cast<int>(entries.size())) return false;
    entries.erase(entries.begin() + (line - 1));
    saveComputed(code, *v, entries, mode, accounts);
    return true;
}

bool DeleteApVoucher(cb4sqlite::Code4& code, ApKind kind, long number)
{
    const std::optional<ApVoucher> v = FindApVoucher(code, kind, number);
    if (!v || v->mode != ap_mode::kOpen) return false;
    cb4sqlite::Transaction tx(code);
    {
        Data4 d(&code, headerTable(kind));
        Field4 batchno(d, "batchno");
        if (seekBatch(d, batchno, number)) {
            d.deleteRec();
            d.pack();
        }
        d.close();
    }
    deleteRows(code, lineTable(kind), number);
    deleteRows(code, "ap_linex", number, kindCode(kind));
    tx.commit();
    return true;
}

// ---- Post, Cancel, Recover ----------------------------------------------------------------

namespace {

const char* journalType(ApKind k) { return k == ApKind::Cash ? "CA" : "AP"; }

// The ap_jsum PERIOD text, as the deployed rows: the number right-aligned in two.
std::string periodText(int period)
{
    char buf[8];
    std::snprintf(buf, sizeof buf, "%2d", period);
    return buf;
}

std::string clip(const std::string& s, size_t n) { return s.size() > n ? s.substr(0, n) : s; }

// Why voucher `v` can't be posted, if it can't.
std::optional<ApRefusal> checkPostable(cb4sqlite::Code4& code, const ApVoucher& v, const std::vector<ApLine>& lines)
{
    if (lines.empty()) return ApRefusal{v.number, ApPostRefusal::NoLines, {}};
    std::int64_t dr = 0, cr = 0;
    for (const ApLine& l : lines) {
        dr += l.debit;
        cr += l.credit;
    }
    if (dr != cr) return ApRefusal{v.number, ApPostRefusal::OutOfBalance, {}};
    for (const ApLine& l : lines)
        if ((l.debit != 0 || l.credit != 0) && !ValidateAccount(code, l.account.account, l.account.dept))
            return ApRefusal{v.number, ApPostRefusal::UnknownAccount, l.account};
    return std::nullopt;
}

void setMode(cb4sqlite::Code4& code, ApKind kind, long number, const char* mode)
{
    Data4 d(&code, headerTable(kind));
    Field4 batchno(d, "batchno"), m(d, "mode");
    if (seekBatch(d, batchno, number)) {
        m.assign(mode);
        d.flush();
    }
    d.close();
}

void appendJournal(cb4sqlite::Code4& code, const ApVoucher& v, const std::vector<ApLine>& lines)
{
    Data4 j(&code, "ap_jsum");
    Field4 account(j, "account"), dept(j, "dept"), period(j, "period"), mode(j, "mode"), date(j, "date"),
        type(j, "type"), batchno(j, "batchno"), reference(j, "reference"), cheque(j, "cheque"),
        descript(j, "descript"), debit(j, "debit"), credit(j, "credit");
    for (const ApLine& l : lines) {
        if (l.debit == 0 && l.credit == 0) continue;
        j.appendStart();
        j.blank();
        account.assign(l.account.account.c_str());
        dept.assign(l.account.dept.c_str());
        period.assign(periodText(v.period).c_str());
        mode.assign("1");
        date.assign(v.invoiceDate.empty() ? v.date.c_str() : v.invoiceDate.c_str());
        type.assign(journalType(v.kind));
        batchno.assignLong(v.number);
        reference.assign(clip(TrimText(v.reference), 30).c_str());
        cheque.assign(clip(TrimText(v.cheque), 7).c_str());
        descript.assign(clip(TrimText(l.descript), 40).c_str());
        debit.assign(money(l.debit).c_str());
        credit.assign(money(l.credit).c_str());
        j.append();
    }
    j.close();
}

void appendOpenItem(cb4sqlite::Code4& code, const ApVoucher& v)
{
    Data4 d(&code, "ap_dist");
    d.appendStart();
    d.blank();
    Field4(d, "apaccount").assign(v.account.account.c_str());
    Field4(d, "apdepart").assign(v.account.dept.c_str());
    Field4(d, "period").assignLong(v.period);
    Field4(d, "date").assign(v.invoiceDate.c_str());
    Field4(d, "duedate").assign(v.dueDate.c_str());
    Field4(d, "batchno").assignLong(v.number);
    Field4(d, "invoiceno").assign(clip(TrimText(v.invoiceNo), 8).c_str());
    Field4(d, "paymentctl").assign(ap_payment::kOpen);
    Field4(d, "vendorno").assign(TrimText(v.vendor).c_str());
    Field4(d, "chequeno").assignLong(0);
    Field4(d, "reference").assign(TrimText(v.reference).c_str());
    Field4(d, "descript").assign(TrimText(v.descript).c_str());
    Field4(d, "amount").assign(money(v.payable).c_str());
    Field4(d, "rebateamt").assign(money(v.whAmount).c_str());
    Field4(d, "amountpaid").assign(money(0).c_str());
    Field4(d, "paymentreq").assignLong(0);
    Field4(d, "vatamt").assign(money(v.vatAmount).c_str());
    Field4(d, "adjustment").assign(money(0).c_str());
    d.append();
    d.close();
}

// Deletes the rows of `table` matching `where` (SQL over the table, bound to ?1 and ?2) and packs it.
void deleteWhere(cb4sqlite::Code4& code, const char* table, const std::string& where, const std::string& a, long b)
{
    std::vector<long long> rows;
    {
        cb4sqlite::Query q(code, std::string("SELECT _rowid FROM ") + table + " WHERE _deleted = 0 AND " + where);
        q.bindText(1, a);
        q.bindInt(2, b);
        while (q.next()) rows.push_back(q.integer(0));
    }
    if (rows.empty()) return;
    Data4 d(&code, table);
    for (long long r : rows)
        if (d.go(static_cast<long>(r)) == r4success) d.deleteRec();
    d.pack();
    d.close();
}

} // namespace

ApPostResult PostApVouchers(cb4sqlite::Code4& code, ApKind kind, long number)
{
    ApPostResult result;
    std::vector<ApVoucher> todo;
    for (const ApVoucher& v : LoadApVouchers(code, kind))
        if (v.mode == ap_mode::kOpen && (number == 0 || v.number == number)) todo.push_back(v);
    if (todo.empty()) return result;
    cb4sqlite::Transaction tx(code);
    for (const ApVoucher& v : todo) {
        const std::vector<ApLine> lines = LoadApLines(code, kind, v.number);
        if (const std::optional<ApRefusal> r = checkPostable(code, v, lines)) {
            result.refused.push_back(*r);
            continue;
        }
        if (kind == ApKind::Invoice) appendOpenItem(code, v);   // AppendAPDist
        appendJournal(code, v, lines);                          // AppendAPSummary
        setMode(code, kind, v.number, ap_mode::kPosted);
        result.posted.push_back(v.number);
    }
    tx.commit();
    return result;
}

ApCancelResult CancelApVoucher(cb4sqlite::Code4& code, ApKind kind, long number)
{
    const std::optional<ApVoucher> v = FindApVoucher(code, kind, number);
    if (!v) return ApCancelResult::NotFound;
    if (v->mode != ap_mode::kOpen) return ApCancelResult::NotOpen;
    cb4sqlite::Transaction tx(code);
    setMode(code, kind, number, ap_mode::kCancelled);
    tx.commit();
    return ApCancelResult::Cancelled;
}

ApRecoverResult RecoverApVoucher(cb4sqlite::Code4& code, ApKind kind, long number)
{
    const std::optional<ApVoucher> v = FindApVoucher(code, kind, number);
    if (v && v->mode == ap_mode::kTransferred) return ApRecoverResult::Transferred;
    if (!v || v->mode != ap_mode::kPosted) return ApRecoverResult::NotPosted;
    {
        cb4sqlite::Query q(code, "SELECT count(*) FROM ap_jsum WHERE _deleted = 0 AND trim(type) = ?1 AND batchno = ?2 "
                                 "AND trim(mode) = '2'");
        q.bindText(1, journalType(kind));
        q.bindInt(2, number);
        if (q.next() && q.integer(0) > 0) return ApRecoverResult::Transferred;
    }
    if (kind == ApKind::Invoice) {
        cb4sqlite::Query q(code, "SELECT count(*) FROM ap_dist WHERE _deleted = 0 AND batchno = ?1 AND "
                                 "trim(paymentctl) IN ('P', 'V', 'C')");
        q.bindInt(1, number);
        if (q.next() && q.integer(0) > 0) return ApRecoverResult::Paid;
    }
    cb4sqlite::Transaction tx(code);
    if (kind == ApKind::Invoice) deleteWhere(code, "ap_dist", "?1 = ?1 AND batchno = ?2", "", number);
    deleteWhere(code, "ap_jsum", "trim(type) = ?1 AND batchno = ?2", journalType(kind), number);
    setMode(code, kind, number, ap_mode::kOpen);
    tx.commit();
    return ApRecoverResult::Recovered;
}

// ---- the transfer ---------------------------------------------------------------------------

namespace {

struct PeriodInfo {
    bool found = false, closed = false;
    std::string end;   // yyyymmdd
};

PeriodInfo periodInfo(cb4sqlite::Code4& code, int period)
{
    PeriodInfo p;
    cb4sqlite::Query q(code, "SELECT status, periode FROM gl_pds WHERE _deleted = 0 AND period = ?1 ORDER BY _rowid LIMIT 1");
    q.bindInt(1, period);
    if (!q.next()) return p;
    p.found = true;
    p.closed = TrimText(q.text(0)) == "1";
    p.end = TrimText(q.text(1));
    return p;
}

// After a transfer or its undo: each posted / transferred voucher of a kind
// is "2" when it has rows and all are transferred, "0" otherwise.
void markTransferred(cb4sqlite::Code4& code, ApKind kind)
{
    std::vector<std::pair<long, const char*>> changes;
    for (const ApVoucher& v : LoadApVouchers(code, kind)) {
        if (v.mode != ap_mode::kPosted && v.mode != ap_mode::kTransferred) continue;
        cb4sqlite::Query q(code, "SELECT count(*), sum(trim(mode) = '2') FROM ap_jsum WHERE _deleted = 0 AND "
                                 "trim(type) = ?1 AND batchno = ?2");
        q.bindText(1, journalType(kind));
        q.bindInt(2, v.number);
        if (!q.next() || q.integer(0) == 0) continue;
        const bool all = q.integer(1) == q.integer(0);
        const char* want = all ? ap_mode::kTransferred : ap_mode::kPosted;
        if (v.mode != want) changes.emplace_back(v.number, want);
    }
    for (const auto& [n, m] : changes) setMode(code, kind, n, m);
}

double toDouble(std::int64_t cents) { return static_cast<double>(cents) / 100.0; }

} // namespace

ApTransferResult TransferAp(cb4sqlite::Code4& code, long iniBatchno)
{
    ApTransferResult result;
    struct Row {
        long long rowid;
        ApAccount account;
        std::int64_t debit, credit;
    };
    std::map<int, std::vector<Row>> byPeriod;
    {
        cb4sqlite::Query q(code, "SELECT _rowid, trim(period), account, dept, debit, credit FROM ap_jsum "
                                 "WHERE _deleted = 0 AND coalesce(trim(mode), '') IN ('', '1') ORDER BY _rowid");
        while (q.next())
            byPeriod[static_cast<int>(q.integer(1))].push_back(
                {q.integer(0), {TrimText(q.text(2)), TrimText(q.text(3))}, rpt::ToCents(q.number(4)),
                 rpt::ToCents(q.number(5))});
    }
    if (byPeriod.empty()) return result;

    cb4sqlite::Transaction tx(code);
    VoucherHeaders h(code);
    VoucherLines l(code);
    if (!h.isValid() || !l.isValid()) throw std::runtime_error("gl: a voucher table is missing");
    for (const auto& [period, rows] : byPeriod) {
        const PeriodInfo p = periodInfo(code, period);
        if (!p.found || p.end.empty()) {
            result.undatedPeriods.push_back(period);
            continue;
        }
        if (p.closed) {
            result.closedPeriods.push_back(period);
            continue;
        }
        // Net by account and dept, in the order the accounts first appear.
        std::vector<std::pair<ApAccount, std::int64_t>> nets;
        for (const Row& r : rows) {
            auto it = std::find_if(nets.begin(), nets.end(), [&](const auto& n) { return n.first == r.account; });
            if (it == nets.end()) {
                nets.emplace_back(r.account, 0);
                it = nets.end() - 1;
            }
            it->second += r.debit - r.credit;
        }
        ApTransferVoucher tv;
        tv.period = period;
        tv.rows = static_cast<int>(rows.size());
        VoucherHeader header;
        header.mode = voucher_mode::kOpen;
        header.description = "Accounts Payable GL Update";   // string 1049
        header.date = p.end;
        header.period = period;
        long number = 0;
        for (const auto& [account, net] : nets) {
            if (net == 0) continue;
            VoucherLine v;
            v.line = ++tv.lines;
            v.period = period;
            v.date = p.end;
            v.account = account.account;
            v.dept = account.dept;
            v.descript = clip(AccountName(code, account), 40);
            v.reference = "AP GL Update";
            v.debit = net > 0 ? toDouble(net) : 0;
            v.credit = net < 0 ? toDouble(-net) : 0;
            (net > 0 ? tv.debit : tv.credit) += net > 0 ? net : -net;
            number = SaveLine(h, l, number, header, v, iniBatchno);
        }
        tv.glVoucher = number;
        Data4 j(&code, "ap_jsum");
        Field4 mode(j, "mode");
        Data4 x(&code, "ap_xfer");
        Field4 xrow(x, "jsumrow"), xbatch(x, "glbatch"), xperiod(x, "period");
        for (const Row& r : rows) {
            if (j.go(static_cast<long>(r.rowid)) == r4success) {
                mode.assign(ap_mode::kTransferred);
                j.flush();
            }
            x.appendStart();
            x.blank();
            xrow.assignLong(static_cast<long>(r.rowid));
            xbatch.assignLong(number);
            xperiod.assignLong(period);
            x.append();
        }
        j.close();
        x.close();
        result.vouchers.push_back(tv);
    }
    markTransferred(code, ApKind::Invoice);
    markTransferred(code, ApKind::Cash);
    tx.commit();
    return result;
}

int UndoApTransfer(cb4sqlite::Code4& code, long glVoucher)
{
    if (glVoucher <= 0 || !hasTable(code, "ap_xfer") || !hasTable(code, "ap_jsum")) return 0;
    std::vector<long long> rows, links;
    {
        cb4sqlite::Query q(code, "SELECT _rowid, jsumrow FROM ap_xfer WHERE _deleted = 0 AND glbatch = ?1");
        q.bindInt(1, glVoucher);
        while (q.next()) {
            links.push_back(q.integer(0));
            rows.push_back(q.integer(1));
        }
    }
    if (rows.empty()) return 0;
    {
        Data4 j(&code, "ap_jsum");
        Field4 mode(j, "mode");
        for (long long r : rows)
            if (j.go(static_cast<long>(r)) == r4success) {
                mode.assign("1");
                j.flush();
            }
        j.close();
        Data4 x(&code, "ap_xfer");
        for (long long r : links)
            if (x.go(static_cast<long>(r)) == r4success) x.deleteRec();
        x.pack();
        x.close();
    }
    markTransferred(code, ApKind::Invoice);
    markTransferred(code, ApKind::Cash);
    return static_cast<int>(rows.size());
}

// ---- payments ---------------------------------------------------------------------------------

std::int64_t AmountOwed(const OpenItem& item) { return item.amount + (item.wh < 0 ? item.wh : 0) - item.paid; }

std::vector<PaymentItem> PaymentItems(cb4sqlite::Code4& code, const std::string& dueBy, bool all)
{
    std::vector<PaymentItem> out;
    std::map<std::string, std::string> names;
    for (const Vendor& v : LoadVendors(code)) names[v.number] = TrimText(v.shortName).empty() ? v.name : v.shortName;
    for (const OpenItem& i : LoadOpenItems(code)) {
        const std::string c = i.paymentCtl;
        const bool unpaid = c == ap_payment::kHeld || c == ap_payment::kOpen || c == ap_payment::kForced;
        const bool due = TrimText(i.dueDate) <= TrimText(dueBy) || c == ap_payment::kForced;
        if (!all && !(unpaid && due)) continue;
        PaymentItem p;
        p.item = i;
        p.vendorName = names.count(i.vendor) ? names[i.vendor] : std::string();
        p.owed = unpaid ? AmountOwed(i) : 0;
        out.push_back(p);
    }
    std::stable_sort(out.begin(), out.end(),
                     [](const PaymentItem& a, const PaymentItem& b) { return a.item.vendor < b.item.vendor; });
    return out;
}

bool SetPaymentControl(cb4sqlite::Code4& code, long batchno, const std::string& control)
{
    if (control != ap_payment::kHeld && control != ap_payment::kOpen && control != ap_payment::kForced) return false;
    cb4sqlite::Transaction tx(code);
    Data4 d(&code, "ap_dist");
    Field4 batch(d, "batchno"), ctl(d, "paymentctl");
    bool done = false;
    if (seekBatch(d, batch, batchno))
        for (; !d.eof() && std::atol(batch.str()) == batchno; d.skip()) {
            if (d.deleted()) continue;
            const std::string c = TrimText(ctl.str());
            if (c != ap_payment::kHeld && c != ap_payment::kOpen && c != ap_payment::kForced) break;   // 4010
            ctl.assign(control.c_str());
            d.flush();
            done = true;
            break;
        }
    d.close();
    tx.commit();
    return done;
}

long NextChequeNumber(cb4sqlite::Code4& code)
{
    long last = 0;
    {
        cb4sqlite::Query q(code, "SELECT coalesce(max(chequeno), 0) FROM ap_dist WHERE _deleted = 0");
        if (q.next()) last = static_cast<long>(q.integer(0));
    }
    {
        cb4sqlite::Query q(code, "SELECT coalesce(max(cheque), 0) FROM ap_reqst WHERE _deleted = 0");
        if (q.next()) last = std::max(last, static_cast<long>(q.integer(0)));
    }
    return last + 1;
}

int OpenPeriodOf(cb4sqlite::Code4& code, const std::string& yyyymmdd)
{
    const std::string d = TrimText(yyyymmdd);
    if (d.empty()) return 0;
    cb4sqlite::Query q(code, "SELECT period FROM gl_pds WHERE _deleted = 0 AND trim(status) = '0' AND periods <= ?1 "
                             "AND periode >= ?1 ORDER BY period LIMIT 1");
    q.bindText(1, d);
    return q.next() ? static_cast<int>(q.integer(0)) : 0;
}

CreateChequesResult CreateCheques(cb4sqlite::Code4& code, const std::string& dueBy, const std::string& payDate,
                                  const ApAccount& bank, long firstNumber)
{
    CreateChequesResult r;
    if (!ValidateAccount(code, bank.account, bank.dept)) {
        r.status = CreateChequesStatus::BadBank;
        return r;
    }
    r.period = OpenPeriodOf(code, payDate);
    if (r.period == 0) {
        r.status = CreateChequesStatus::ClosedPeriod;
        return r;
    }
    // The items to pay: O due by the date, F whatever their date (D8; H waits).
    std::vector<PaymentItem> pay;
    for (const PaymentItem& p : PaymentItems(code, dueBy))
        if (p.item.paymentCtl != ap_payment::kHeld && p.owed > 0) pay.push_back(p);
    if (pay.empty()) return r;   // NothingDue
    std::vector<std::string> vendors;
    for (const PaymentItem& p : pay)
        if (vendors.empty() || vendors.back() != p.item.vendor) vendors.push_back(p.item.vendor);
    if (firstNumber <= 0) {
        r.status = CreateChequesStatus::BadNumber;
        return r;
    }
    for (long n = firstNumber; n < firstNumber + static_cast<long>(vendors.size()); ++n) {
        cb4sqlite::Query q(code, "SELECT (SELECT count(*) FROM ap_reqst WHERE _deleted = 0 AND cheque = ?1) + "
                                 "(SELECT count(*) FROM ap_dist WHERE _deleted = 0 AND chequeno = ?1)");
        q.bindInt(1, n);
        if (q.next() && q.integer(0) > 0) {
            r.status = CreateChequesStatus::BadNumber;
            r.clash = n;
            return r;
        }
    }

    cb4sqlite::Transaction tx(code);
    Data4 items(&code, "ap_dist");
    Field4 batch(items, "batchno"), ctl(items, "paymentctl"), cheque(items, "chequeno"), paid(items, "amountpaid"),
        datePaid(items, "datepaid"), account(items, "account"), dept(items, "dept");
    Data4 cheques(&code, "ap_reqst");
    Data4 j(&code, "ap_jsum");
    Field4 jaccount(j, "account"), jdept(j, "dept"), jperiod(j, "period"), jmode(j, "mode"), jdate(j, "date"),
        jtype(j, "type"), jbatch(j, "batchno"), jref(j, "reference"), jcheque(j, "cheque"), jdesc(j, "descript"),
        jdebit(j, "debit"), jcredit(j, "credit");
    auto journal = [&](const ApAccount& a, const PaymentItem& p, long number, std::int64_t dr, std::int64_t cr) {
        j.appendStart();
        j.blank();
        jaccount.assign(a.account.c_str());
        jdept.assign(a.dept.c_str());
        jperiod.assign(periodText(r.period).c_str());
        jmode.assign("1");
        jdate.assign(payDate.c_str());
        jtype.assign("AP");
        jbatch.assignLong(p.item.batchno);
        jref.assign(clip(TrimText(p.item.reference), 30).c_str());
        jcheque.assign(clip(std::to_string(number), 7).c_str());
        jdesc.assign(clip("Cheque " + std::to_string(number) + " " + TrimText(p.vendorName), 40).c_str());
        jdebit.assign(money(dr).c_str());
        jcredit.assign(money(cr).c_str());
        j.append();
    };
    long number = firstNumber;
    for (const std::string& vendor : vendors) {
        ChequeMade made;
        made.number = number;
        made.vendor = vendor;
        for (const PaymentItem& p : pay) {
            if (p.item.vendor != vendor) continue;
            made.vendorName = p.vendorName;
            if (seekBatch(items, batch, p.item.batchno)) {
                ctl.assign(ap_payment::kPaid);
                cheque.assignLong(number);
                paid.assign(money(p.item.paid + p.owed).c_str());
                datePaid.assign(payDate.c_str());
                account.assign(bank.account.c_str());
                dept.assign(bank.dept.c_str());
                items.flush();
            }
            journal(p.item.ap, p, number, p.owed, 0);   // Dr AP
            journal(bank, p, number, 0, p.owed);        // Cr the bank
            made.amount += p.owed;
            ++made.items;
        }
        cheques.appendStart();
        cheques.blank();
        Field4(cheques, "vendorno").assign(vendor.c_str());
        Field4(cheques, "paymentreq").assignLong(0);
        Field4(cheques, "reference").assign(clip(TrimText(made.vendorName), 20).c_str());
        Field4(cheques, "amountpaid").assign(money(made.amount).c_str());
        Field4(cheques, "datepaid").assign(payDate.c_str());
        Field4(cheques, "cheque").assignLong(number);
        Field4(cheques, "bankacct").assign(bank.account.c_str());
        Field4(cheques, "bankdept").assign(bank.dept.c_str());
        Field4(cheques, "cleared").assign("1");   // not cleared (the original's AddCheque)
        cheques.append();
        r.cheques.push_back(made);
        ++number;
    }
    items.close();
    cheques.close();
    j.close();
    tx.commit();
    r.status = CreateChequesStatus::Created;
    return r;
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
        c.cleared = TrimText(q.text(8)) == "0";
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

// ---- cheques: print, confirm, clear, void, purge ----------------------------------------------

namespace {

// The cheque's ap_reqst row, current in `d` (opened on ap_reqst); false if none.
bool seekCheque(Data4& d, long cheque)
{
    Field4 number(d, "cheque");
    for (d.top(); !d.eof(); d.skip())
        if (!d.deleted() && std::atol(number.str()) == cheque) return true;
    return false;
}

const char* kOnes[] = {"",        "one",     "two",       "three",    "four",     "five",    "six",
                       "seven",   "eight",   "nine",      "ten",      "eleven",   "twelve",  "thirteen",
                       "fourteen", "fifteen", "sixteen",  "seventeen", "eighteen", "nineteen"};
const char* kTens[] = {"", "", "twenty", "thirty", "forty", "fifty", "sixty", "seventy", "eighty", "ninety"};

std::string below1000(int n)
{
    std::string out;
    if (n >= 100) {
        out = std::string(kOnes[n / 100]) + " hundred";
        n %= 100;
        if (n) out += " ";
    }
    if (n >= 20) {
        out += kTens[n / 10];
        if (n % 10) out += std::string("-") + kOnes[n % 10];
    } else if (n > 0) {
        out += kOnes[n];
    }
    return out;
}

} // namespace

std::string ChequeState(cb4sqlite::Code4& code, long cheque)
{
    cb4sqlite::Query q(code, "SELECT trim(paymentctl) FROM ap_dist WHERE _deleted = 0 AND chequeno = ?1");
    q.bindInt(1, cheque);
    std::string state;
    while (q.next()) {
        const std::string c = q.text(0);
        // The least advanced item decides: P before V before C.
        if (state.empty() || (c == ap_payment::kPaid) || (c == ap_payment::kPrinted && state == ap_payment::kCleared))
            state = c;
    }
    return state;
}

std::vector<ChequeToPrint> LoadChequesIn(cb4sqlite::Code4& code, const std::string& state)
{
    std::vector<ChequeToPrint> out;
    for (const Cheque& c : LoadCheques(code)) {
        const std::string st = ChequeState(code, c.number);
        if (state.empty() ? (st == ap_payment::kCleared || c.cleared) : st != state) continue;
        ChequeToPrint t;
        t.cheque = c;
        t.state = st;
        if (const std::optional<Vendor> v = FindVendor(code, c.vendor)) t.payee = *v;
        else t.payee.number = c.vendor;
        cb4sqlite::Query q(code, "SELECT batchno, invoiceno, date, reference, amount, rebateamt, amountpaid FROM ap_dist "
                                 "WHERE _deleted = 0 AND chequeno = ?1 ORDER BY batchno, _rowid");
        q.bindInt(1, c.number);
        while (q.next())
            t.lines.push_back({static_cast<long>(q.integer(0)), TrimText(q.text(1)), q.text(2), q.text(3),
                               rpt::ToCents(q.number(4)), rpt::ToCents(q.number(5)), rpt::ToCents(q.number(6))});
        out.push_back(std::move(t));
    }
    return out;
}

std::string AmountInWords(std::int64_t cents)
{
    if (cents < 0) cents = -cents;
    std::int64_t whole = cents / 100;
    const int frac = static_cast<int>(cents % 100);
    std::string words;
    if (whole == 0) {
        words = "zero";
    } else {
        const char* scales[] = {"", " thousand", " million", " billion"};
        std::vector<std::string> parts;
        for (int i = 0; whole > 0 && i < 4; ++i, whole /= 1000) {
            const int chunk = static_cast<int>(whole % 1000);
            if (chunk) parts.insert(parts.begin(), below1000(chunk) + scales[i]);
        }
        for (const std::string& part : parts) words += (words.empty() ? "" : " ") + part;
    }
    words[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(words[0])));
    char tail[16];
    std::snprintf(tail, sizeof tail, " and %02d/100", frac);
    return words + tail;
}

int ConfirmPrinted(cb4sqlite::Code4& code, long from, long to)
{
    std::vector<long> cheques;
    for (const Cheque& c : LoadCheques(code))
        if (c.number >= from && c.number <= to && ChequeState(code, c.number) == ap_payment::kPaid)
            cheques.push_back(c.number);
    if (cheques.empty()) return 0;
    cb4sqlite::Transaction tx(code);
    Data4 d(&code, "ap_dist");
    Field4 cheque(d, "chequeno"), ctl(d, "paymentctl");
    for (d.top(); !d.eof(); d.skip()) {
        if (d.deleted() || TrimText(ctl.str()) != ap_payment::kPaid) continue;
        const long n = std::atol(cheque.str());
        if (std::find(cheques.begin(), cheques.end(), n) == cheques.end()) continue;
        ctl.assign(ap_payment::kPrinted);
        d.flush();
    }
    d.close();
    tx.commit();
    return static_cast<int>(cheques.size());
}

RenumberResult RenumberCheque(cb4sqlite::Code4& code, long from, long to)
{
    const std::string state = ChequeState(code, from);
    {
        Data4 d(&code, "ap_reqst");
        const bool found = seekCheque(d, from);
        d.close();
        if (!found) return RenumberResult::NotFound;
    }
    if (state != ap_payment::kPaid) return RenumberResult::NotMade;
    {
        cb4sqlite::Query q(code, "SELECT (SELECT count(*) FROM ap_reqst WHERE _deleted = 0 AND cheque = ?1) + "
                                 "(SELECT count(*) FROM ap_dist WHERE _deleted = 0 AND chequeno = ?1)");
        q.bindInt(1, to);
        if (to <= 0 || (q.next() && q.integer(0) > 0)) return RenumberResult::NumberUsed;
    }
    cb4sqlite::Transaction tx(code);
    {
        Data4 d(&code, "ap_reqst");
        if (seekCheque(d, from)) {
            Field4(d, "cheque").assignLong(to);
            d.flush();
        }
        d.close();
    }
    {
        Data4 d(&code, "ap_dist");
        Field4 cheque(d, "chequeno");
        for (d.top(); !d.eof(); d.skip())
            if (!d.deleted() && std::atol(cheque.str()) == from) {
                cheque.assignLong(to);
                d.flush();
            }
        d.close();
    }
    {
        Data4 j(&code, "ap_jsum");
        Field4 cheque(j, "cheque"), desc(j, "descript");
        const std::string old = std::to_string(from), now = std::to_string(to);
        for (j.top(); !j.eof(); j.skip())
            if (!j.deleted() && TrimText(cheque.str()) == old) {
                cheque.assign(now.c_str());
                std::string d = TrimText(desc.str());
                if (d.rfind("Cheque " + old, 0) == 0) d = "Cheque " + now + d.substr(7 + old.size());
                desc.assign(clip(d, 40).c_str());
                j.flush();
            }
        j.close();
    }
    tx.commit();
    return RenumberResult::Renumbered;
}

ClearResult ClearCheque(cb4sqlite::Code4& code, long cheque, const std::string& date)
{
    {
        Data4 d(&code, "ap_reqst");
        const bool found = seekCheque(d, cheque);
        d.close();
        if (!found) return ClearResult::NotFound;
    }
    const std::string state = ChequeState(code, cheque);
    if (state == ap_payment::kCleared) return ClearResult::AlreadyCleared;
    if (state != ap_payment::kPrinted) return ClearResult::NotPrinted;
    cb4sqlite::Transaction tx(code);
    {
        Data4 d(&code, "ap_dist");
        Field4 n(d, "chequeno"), ctl(d, "paymentctl");
        for (d.top(); !d.eof(); d.skip())
            if (!d.deleted() && std::atol(n.str()) == cheque) {
                ctl.assign(ap_payment::kCleared);
                d.flush();
            }
        d.close();
    }
    {
        Data4 d(&code, "ap_reqst");
        if (seekCheque(d, cheque)) {
            Field4(d, "cleared").assign("0");
            Field4(d, "reference").assign(clip("Cleared " + TrimText(date), 20).c_str());
            d.flush();
        }
        d.close();
    }
    tx.commit();
    return ClearResult::Cleared;
}

VoidOutcome VoidCheque(cb4sqlite::Code4& code, long cheque, const std::string& voidDate)
{
    VoidOutcome out;
    Cheque found;
    {
        bool have = false;
        for (const Cheque& c : LoadCheques(code))
            if (c.number == cheque) {
                found = c;
                have = true;
            }
        if (!have) return out;
    }
    const std::string state = ChequeState(code, cheque);
    if (state == ap_payment::kCleared || found.cleared) {
        out.result = VoidResult::Cleared;
        return out;
    }
    // The payment's journal rows: waiting ones go; transferred ones are reversed.
    struct JRow {
        long long rowid;
        bool transferred;
        ApAccount account;
        std::int64_t debit, credit;
        long batchno;
        std::string reference;
    };
    std::vector<JRow> rows;
    {
        cb4sqlite::Query q(code, "SELECT _rowid, trim(mode), account, dept, debit, credit, batchno, reference FROM ap_jsum "
                                 "WHERE _deleted = 0 AND trim(type) = 'AP' AND trim(cheque) = ?1");
        q.bindText(1, std::to_string(cheque));
        while (q.next())
            rows.push_back({q.integer(0), q.text(1) == ap_mode::kTransferred, {TrimText(q.text(2)), TrimText(q.text(3))},
                            rpt::ToCents(q.number(4)), rpt::ToCents(q.number(5)), static_cast<long>(q.integer(6)),
                            q.text(7)});
    }
    int period = 0;
    if (std::any_of(rows.begin(), rows.end(), [](const JRow& r) { return r.transferred; })) {
        period = OpenPeriodOf(code, voidDate);
        if (period == 0) {
            out.result = VoidResult::ClosedPeriod;
            return out;
        }
    }
    cb4sqlite::Transaction tx(code);
    {
        Data4 d(&code, "ap_dist");
        Field4 n(d, "chequeno"), ctl(d, "paymentctl"), paid(d, "amountpaid"), datePaid(d, "datepaid"),
            account(d, "account"), dept(d, "dept");
        for (d.top(); !d.eof(); d.skip())
            if (!d.deleted() && std::atol(n.str()) == cheque) {
                ctl.assign(ap_payment::kOpen);
                n.assignLong(0);
                paid.assign(money(0).c_str());
                datePaid.assign("");
                account.assign("");
                dept.assign("");
                d.flush();
                ++out.itemsReopened;
            }
        d.close();
    }
    {
        Data4 j(&code, "ap_jsum");
        bool deleted = false;
        for (const JRow& r : rows)
            if (!r.transferred && j.go(static_cast<long>(r.rowid)) == r4success) {
                j.deleteRec();
                deleted = true;
                ++out.rowsRemoved;
            }
        if (deleted) j.pack();
        Field4 jaccount(j, "account"), jdept(j, "dept"), jperiod(j, "period"), jmode(j, "mode"), jdate(j, "date"),
            jtype(j, "type"), jbatch(j, "batchno"), jref(j, "reference"), jcheque(j, "cheque"), jdesc(j, "descript"),
            jdebit(j, "debit"), jcredit(j, "credit");
        for (const JRow& r : rows) {
            if (!r.transferred) continue;
            j.appendStart();
            j.blank();
            jaccount.assign(r.account.account.c_str());
            jdept.assign(r.account.dept.c_str());
            jperiod.assign(periodText(period).c_str());
            jmode.assign("1");
            jdate.assign(voidDate.c_str());
            jtype.assign("AP");
            jbatch.assignLong(r.batchno);
            jref.assign(clip(TrimText(r.reference), 30).c_str());
            jcheque.assign(clip(std::to_string(cheque), 7).c_str());
            jdesc.assign(clip("Void of cheque " + std::to_string(cheque), 40).c_str());
            jdebit.assign(money(r.credit).c_str());   // the reverse
            jcredit.assign(money(r.debit).c_str());
            j.append();
            ++out.rowsReversed;
        }
        j.close();
    }
    {
        Data4 d(&code, "ap_reqst");
        if (seekCheque(d, cheque)) {
            d.deleteRec();
            d.pack();
        }
        d.close();
    }
    tx.commit();
    out.result = VoidResult::Voided;
    return out;
}

std::vector<OpenItem> PurgeCandidates(cb4sqlite::Code4& code, const std::string& before)
{
    std::vector<OpenItem> out;
    for (const OpenItem& i : LoadOpenItems(code))
        if (i.paymentCtl == ap_payment::kCleared && !TrimText(i.datePaid).empty() &&
            TrimText(i.datePaid) < TrimText(before))
            out.push_back(i);
    return out;
}

PurgeOutcome PurgePaid(cb4sqlite::Code4& code, const std::string& before)
{
    PurgeOutcome out;
    const std::vector<OpenItem> items = PurgeCandidates(code, before);
    if (items.empty()) return out;
    std::vector<long> batches, cheques;
    for (const OpenItem& i : items) {
        batches.push_back(i.batchno);
        if (i.cheque) cheques.push_back(i.cheque);
    }
    cb4sqlite::Transaction tx(code);
    {
        Data4 d(&code, "ap_dist");
        Field4 batch(d, "batchno"), ctl(d, "paymentctl"), paid(d, "datepaid");
        for (d.top(); !d.eof(); d.skip()) {
            if (d.deleted() || TrimText(ctl.str()) != ap_payment::kCleared) continue;
            const std::string dp = TrimText(paid.str());
            if (dp.empty() || dp >= TrimText(before)) continue;
            if (std::find(batches.begin(), batches.end(), std::atol(batch.str())) == batches.end()) continue;
            d.deleteRec();
            ++out.items;
        }
        d.pack();
        d.close();
    }
    {
        Data4 d(&code, "ap_reqst");
        Field4 number(d, "cheque");
        bool any = false;
        for (d.top(); !d.eof(); d.skip()) {
            if (d.deleted()) continue;
            const long n = std::atol(number.str());
            if (std::find(cheques.begin(), cheques.end(), n) == cheques.end()) continue;
            cb4sqlite::Query q(code, "SELECT count(*) FROM ap_dist WHERE _deleted = 0 AND chequeno = ?1");
            q.bindInt(1, n);
            if (q.next() && q.integer(0) == 0) {
                d.deleteRec();
                any = true;
                ++out.cheques;
            }
        }
        if (any) d.pack();
        d.close();
    }
    tx.commit();
    return out;
}
