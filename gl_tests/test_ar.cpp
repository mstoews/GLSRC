//
// gl_tests: the Accounts Receivable core (spec #6 2.1, 4.1, 5.3, D3, D4, D6,
// D8, D12; plans S6-M0 and S6-M1) on the dev fixture's AR data (make_dev_fixture.py
// accounts_receivable):
//
//   C001 hospital, VAT 10 %, terms 30, sales 410001-10    C002 doctor, VAT 10 %, sales 410001-20
//   C003 pharmacy, no VAT, terms 0                        all: AR 111101, VAT 260007
//
//   voucher 1  open       C001 S-1001  3 x 1,100.00 -> 3,000.00 + VAT 300.00; AR 3,300.00
//   voucher 2  open       C002 S-1002  2 x 550.00 and 220.00 typed -> 1,200.00 + VAT 120.00; AR 1,320.00
//   voucher 3  posted     C001 S-1003  2,200.00, owed in full (O)
//   voucher 4  posted     C002 S-1004  1,100.00, receipt 1 500.00 (R, 600.00 owed)
//   voucher 5  posted     C003 S-1005  800.00, no VAT, receipt 2 800.00 (C)
//   voucher 6  cancelled  C001 S-1006  550.00 (X)
//
#include "GlTestDb.h"

#include "Accounts.h"   // TrimText
#include "Ar.h"
#include "Vouchers.h"   // the transfer's GL vouchers

#include <map>
#include <string>
#include <vector>

using namespace gltest;
using Catch::Approx;

namespace {

ArLine product(const char* account, const char* dept, std::int64_t qty, std::int64_t price, std::int64_t typed = 0)
{
    ArLine l;
    l.account = {account, dept};
    l.qty = qty;
    l.price = price;
    l.gross = typed;
    return l;
}

const SaleAccounts kAccounts{{"260007", ""}, {"111101", ""}, "VAT PAYABLE", "ACCOUNTS RECEIVABLE - TRADE"};

std::int64_t count(cb4sqlite::Code4& code, const char* sql)
{
    cb4sqlite::Query q(code, sql);
    return q.next() ? q.integer(0) : -1;
}

} // namespace

TEST_CASE("LineAmount and SplitSale: VAT included in the price", "[ar]")
{
    CHECK(LineAmount(300, 110000, 0) == 330000);   // 3 x 1,100.00
    CHECK(LineAmount(150, 333, 0) == 500);         // 1.5 x 3.33 = 4.995 -> 5.00
    CHECK(LineAmount(0, 0, 22000) == 22000);       // no price: the amount typed
    CHECK(LineAmount(250, 0, 12345) == 12345);     // a quantity with no price: still the amount typed

    // Spec #6 10: 3,300.00 at 10 % is 3,000.00 + 300.00.
    SaleSplit s = SplitSale(330000, 10);
    CHECK(s.sales == 300000);
    CHECK(s.vat == 30000);
    s = SplitSale(80000, 0);   // no VAT
    CHECK(s.sales == 80000);
    CHECK(s.vat == 0);
    s = SplitSale(100, 10);    // 1.00: 0.0909 VAT -> 0.09
    CHECK(s.vat == 9);
    CHECK(s.sales == 91);
    s = SplitSale(14, 12);     // 0.14 x 12 / 112 = 0.015 exactly: half away from zero
    CHECK(s.vat == 2);
    CHECK(s.sales == 12);
    s = SplitSale(-330000, 10);   // a negative amount splits the same way
    CHECK(s.vat == -30000);
    CHECK(s.sales == -300000);
    // The sales and VAT always add back to the amount.
    for (std::int64_t a = 1; a < 5000; a += 7)
        for (int v : {0, 1, 5, 10, 12, 20}) {
            const SaleSplit x = SplitSale(a, v);
            REQUIRE(x.sales + x.vat == a);
        }
}

TEST_CASE("ComputeSale: the VAT and AR lines from the product lines (D4)", "[ar]")
{
    // Voucher 2: 2 x 550.00 and 220.00 typed, both on 410001-20.
    ComputedSale c = ComputeSale({product("410001", "20", 200, 55000), product("410001", "20", 0, 0, 22000)}, 10,
                                 kAccounts);
    REQUIRE(c.lines.size() == 4);
    CHECK(c.lines[0].role == ArLineRole::Sales);
    CHECK(c.lines[0].credit == 100000);
    CHECK(c.lines[0].gross == 110000);
    CHECK(c.lines[0].qty == 200);
    CHECK(c.lines[0].price == 55000);
    CHECK(c.lines[1].credit == 20000);
    CHECK(c.lines[1].gross == 22000);
    CHECK(c.lines[2].role == ArLineRole::Vat);
    CHECK(c.lines[2].account.account == "260007");
    CHECK(c.lines[2].credit == 12000);
    CHECK(c.lines[2].descript == "VAT PAYABLE");
    CHECK(c.lines[3].role == ArLineRole::Receivable);
    CHECK(c.lines[3].account.account == "111101");
    CHECK(c.lines[3].debit == 132000);
    CHECK(c.lines[3].line == 4);
    CHECK(c.total == 132000);
    CHECK(c.vat == 12000);
    CHECK(c.debits == c.credits);

    // Deleting a product line recomputes from what's left.
    c = ComputeSale({product("410001", "20", 0, 0, 22000)}, 10, kAccounts);
    REQUIRE(c.lines.size() == 3);
    CHECK(c.lines[1].credit == 2000);
    CHECK(c.lines[2].debit == 22000);

    // Each product line keeps its own sales account.
    c = ComputeSale({product("410001", "10", 100, 110000), product("410001", "20", 100, 55000)}, 10, kAccounts);
    CHECK(c.lines[0].account == ApAccount{"410001", "10"});
    CHECK(c.lines[1].account == ApAccount{"410001", "20"});
    CHECK(c.lines[2].credit == 15000);

    // No VAT: no VAT line. Nothing at all: no lines.
    c = ComputeSale({product("410001", "10", 0, 0, 80000)}, 0, kAccounts);
    REQUIRE(c.lines.size() == 2);
    CHECK(c.lines[1].role == ArLineRole::Receivable);
    CHECK(c.lines[1].debit == 80000);
    CHECK(ComputeSale({}, 10, kAccounts).lines.empty());

    // 37 awkward lines balance to the cent; the VAT line is the lines' VAT.
    std::vector<ArLine> many;
    std::int64_t lineVat = 0;
    for (int i = 1; i <= 37; ++i) {
        many.push_back(product("410001", "10", 100 + i * 37, 1999 + i * 113));
        lineVat += SplitSale(LineAmount(100 + i * 37, 1999 + i * 113, 0), 12).vat;
    }
    c = ComputeSale(many, 12, kAccounts);
    CHECK(c.debits == c.credits);
    CHECK(c.vat == lineVat);
    CHECK(c.lines.size() == 39);
}

TEST_CASE("EnsureArTables creates the tables in an empty company", "[ar]")
{
    EmptyDb db;
    EnsureArTables(db.code);
    CHECK(LoadCustomers(db.code).empty());
    CHECK(LoadArVouchers(db.code).empty());
    CHECK(LoadArOpenItems(db.code).empty());
    CHECK(LoadReceipts(db.code).empty());
    CHECK(LoadArJournal(db.code).empty());
    CHECK(NextArVoucherNumber(db.code) == 1);
    CHECK(LoadArAccounts(db.code).ar.empty());   // no gl_setat either

    Customer c;
    c.number = "A1";
    c.name = "First";
    CHECK(SaveCustomer(db.code, c));
    ArVoucher v;
    v.customer = "A1";
    v.invoiceNo = "X";
    v.mode = ar_mode::kOpen;
    v.memo = "A memo longer than the field's ten characters";
    CHECK(SaveArVoucher(db.code, v, ComputeSale({product("410001", "10", 0, 0, 1100)}, 10, kAccounts).lines) == 1);
    CHECK(LoadArLines(db.code, 1).size() == 3);
    CHECK(FindArVoucher(db.code, 1)->memo == v.memo);
    EnsureArTables(db.code);   // the tables exist now: nothing changes
    CHECK(LoadCustomers(db.code).size() == 1);
    CHECK(LoadArLines(db.code, 1).size() == 3);
}

TEST_CASE("The fixture's customers and accounts", "[ar]")
{
    FixtureDb db;
    EnsureArTables(db.code);

    const ArAccounts a = LoadArAccounts(db.code);
    CHECK(a.ar == ApAccount{"111101", ""});
    CHECK(a.sales == ApAccount{"410001", ""});   // the chart has 410001 only with depts 10 and 20 (plan M1)
    CHECK(a.vatPayable == ApAccount{"260007", ""});
    REQUIRE(a.banks.size() == 2);
    CHECK(a.banks[0].account.account == "101000");   // CA, then CHCK
    CHECK(a.banks[1].account.account == "102101");

    const std::vector<Customer> cs = LoadCustomers(db.code);
    REQUIRE(cs.size() == 3);
    CHECK(cs[0].number == "C001");
    CHECK(cs[1].number == "C002");
    CHECK(cs[2].number == "C003");
    const Customer& c1 = cs[0];
    CHECK(c1.name == "City General Hospital");
    CHECK(c1.ar == ApAccount{"111101", ""});        // ACCOUNT: the AR control
    CHECK(c1.sales == ApAccount{"410001", "10"});   // AR_ACCT / AR_DEPT: sales
    CHECK(c1.vat == ApAccount{"260007", ""});
    CHECK(c1.terms == 30);
    CHECK(c1.vatRate == 10);
    CHECK(c1.shipAddress1 == "Receiving, Pharmacy Wing");
    CHECK(c1.vin == "200-123-45");
    CHECK(FindCustomer(db.code, "C002")->sales == ApAccount{"410001", "20"});
    const Customer c3 = *FindCustomer(db.code, " C003 ");
    CHECK(c3.vatRate == 0);
    CHECK(c3.vat.empty());
    CHECK(c3.terms == 0);
    CHECK_FALSE(FindCustomer(db.code, "C999"));
}

TEST_CASE("The fixture's vouchers, ledger and journal agree", "[ar]")
{
    FixtureDb db;
    EnsureArTables(db.code);

    const std::vector<ArVoucher> vs = LoadArVouchers(db.code);
    REQUIRE(vs.size() == 6);
    const char* modes[] = {"1", "1", "0", "0", "0", "X"};
    const std::int64_t totals[] = {330000, 132000, 220000, 110000, 80000, 55000};
    const std::int64_t vats[] = {30000, 12000, 20000, 10000, 0, 5000};
    for (int i = 0; i < 6; ++i) {
        CHECK(vs[i].number == i + 1);
        CHECK(vs[i].mode == modes[i]);
        CHECK(vs[i].total == totals[i]);
        CHECK(vs[i].vatAmount == vats[i]);
        CHECK(vs[i].period == 7);
        CHECK(vs[i].account == ApAccount{"111101", ""});
    }
    CHECK(vs[0].invoiceNo == "S-1001");
    CHECK(vs[0].memo == "Deliver to the pharmacy wing.");
    CHECK(vs[0].orderNo == "PO-5521");
    CHECK(vs[0].dueDate == AddDays(vs[0].invoiceDate, 30));
    CHECK(vs[4].dueDate == vs[4].invoiceDate);   // C003: terms 0
    CHECK(vs[5].cancelled());
    CHECK(NextArVoucherNumber(db.code) == 7);

    // Every voucher's lines are what ComputeSale makes of its product lines.
    for (const ArVoucher& v : vs) {
        const std::vector<ArLine> lines = LoadArLines(db.code, v.number);
        REQUIRE(static_cast<int>(lines.size()) == v.lines);
        std::vector<ArLine> products;
        for (const ArLine& l : lines)
            if (l.role == ArLineRole::Sales) products.push_back(l);
        const ComputedSale c = ComputeSale(products, v.vatRate, kAccounts);
        INFO("voucher " << v.number);
        REQUIRE(c.lines.size() == lines.size());
        for (size_t i = 0; i < lines.size(); ++i) {
            CHECK(lines[i].role == c.lines[i].role);
            CHECK(lines[i].account == c.lines[i].account);
            CHECK(lines[i].debit == c.lines[i].debit);
            CHECK(lines[i].credit == c.lines[i].credit);
        }
        CHECK(c.total == v.total);
        CHECK(c.vat == v.vatAmount);
    }
    const std::vector<ArLine> one = LoadArLines(db.code, 1);
    CHECK(one[0].qty == 300);
    CHECK(one[0].price == 110000);
    CHECK(one[0].gross == 330000);
    CHECK(one[0].descript == "SALES REVENUE - R&D");
    const std::vector<ArLine> two = LoadArLines(db.code, 2);
    CHECK(two[1].price == 0);
    CHECK(two[1].gross == 22000);   // typed

    // The open items: one per posted voucher; owed = amount less receipts.
    const std::vector<ArOpenItem> items = LoadArOpenItems(db.code);
    REQUIRE(items.size() == 3);
    CHECK(items[0].batchno == 3);
    CHECK(items[0].paymentCtl == ar_receipt::kOpen);
    CHECK(items[0].remainder == 220000);
    CHECK(items[0].bank.empty());
    CHECK(items[1].batchno == 4);
    CHECK(items[1].paymentCtl == ar_receipt::kPartial);
    CHECK(items[1].received == 50000);
    CHECK(items[1].remainder == 60000);
    CHECK(items[1].datePaid == "20261020");
    CHECK(items[1].bank == ApAccount{"102101", ""});
    CHECK(items[2].paymentCtl == ar_receipt::kSettled);
    CHECK(items[2].remainder == 0);
    for (const ArOpenItem& i : items) {
        const ArVoucher v = *FindArVoucher(db.code, i.batchno);
        CHECK(v.mode == ar_mode::kPosted);
        CHECK(i.amount == v.total);
        CHECK(i.vat == v.vatAmount);
        CHECK(i.ar == v.account);
        CHECK(i.customer == v.customer);
        CHECK(i.invoiceNo == v.invoiceNo);
        std::int64_t received = 0;
        for (const Receipt& r : LoadReceipts(db.code, i.batchno)) received += r.amount + r.discount;
        CHECK(i.received == received);
        CHECK(i.remainder == i.amount - received);
    }

    const std::vector<Receipt> rs = LoadReceipts(db.code);
    REQUIRE(rs.size() == 2);
    CHECK(rs[0].number == 1);
    CHECK(rs[0].batchno == 4);
    CHECK(rs[0].amount == 50000);
    CHECK(rs[0].date == "20261020");
    CHECK(rs[0].reference == "Cheque 88231");
    CHECK(rs[1].number == 2);
    CHECK(rs[1].customer == "C003");
    CHECK(LoadReceipts(db.code, 3).empty());

    // The journal: all waiting, balanced; each invoice's AR debit is its total,
    // each receipt's rows credit AR its amount and carry its number.
    const std::vector<JournalRow> js = LoadArJournal(db.code);
    REQUIRE(js.size() == 12);
    std::int64_t dr = 0, cr = 0;
    std::map<long, std::int64_t> arDebit;
    std::map<std::string, std::int64_t> receiptCredit;
    for (const JournalRow& j : js) {
        CHECK(j.mode == "1");
        CHECK(j.period == 7);
        dr += j.debit;
        cr += j.credit;
        if (j.account.account != "111101") continue;
        if (j.type == ar_journal::kInvoice) arDebit[j.batchno] += j.debit;
        else if (j.type == ar_journal::kReceipt) receiptCredit[j.cheque] += j.credit;
    }
    CHECK(dr == cr);
    CHECK(arDebit == std::map<long, std::int64_t>{{3, 220000}, {4, 110000}, {5, 80000}});
    CHECK(receiptCredit == std::map<std::string, std::int64_t>{{"1", 50000}, {"2", 80000}});
}

TEST_CASE("DuplicateArInvoice: any voucher not cancelled (D6)", "[ar]")
{
    FixtureDb db;
    EnsureArTables(db.code);
    CHECK(DuplicateArInvoice(db.code, "C001", "S-1003") == 3);     // posted
    CHECK(DuplicateArInvoice(db.code, " C001", "S-1001 ") == 1);   // open, trimmed
    CHECK(DuplicateArInvoice(db.code, "C001", "S-1006") == 0);     // cancelled
    CHECK(DuplicateArInvoice(db.code, "C002", "S-1003") == 0);     // another customer
    CHECK(DuplicateArInvoice(db.code, "C001", "S-1003", 3) == 0);  // the voucher itself
    CHECK(DuplicateArInvoice(db.code, "C001", "s-1003") == 0);     // case as typed
    CHECK(DuplicateArInvoice(db.code, "C001", "") == 0);
}

TEST_CASE("SaveCustomer and SaveArVoucher leave the others alone", "[ar]")
{
    FixtureDb db;
    EnsureArTables(db.code);

    // Replace C002, keeping the fields the port doesn't use.
    {
        Data4 d(&db.code, "ar_cust");
        Field4 no(d, "vendorno"), credit(d, "credit"), text(d, "terms_txt");
        for (d.top(); !d.eof(); d.skip())
            if (std::string(no.str()).rfind("C002", 0) == 0) {
                credit.assign("5000.00");
                text.assign("Net 30");
                d.flush();
            }
        d.close();
    }
    Customer c = *FindCustomer(db.code, "C002");
    c.terms = 45;
    c.contact = "Clinic";
    CHECK_FALSE(SaveCustomer(db.code, c));   // replaced
    CHECK(FindCustomer(db.code, "C002")->terms == 45);
    CHECK(FindCustomer(db.code, "C001")->terms == 30);
    CHECK(count(db.code, "SELECT count(*) FROM ar_cust WHERE credit = 5000 AND trim(terms_txt) = 'Net 30'") == 1);

    Customer n;
    n.number = "C004";
    n.shortName = "EASTCLINIC";
    n.name = "East Clinic";
    n.ar = {"111101", ""};
    n.sales = {"410001", "20"};
    n.vat = {"260007", ""};
    n.vatRate = 10;
    n.terms = 15;
    CHECK(SaveCustomer(db.code, n));   // added
    CHECK(LoadCustomers(db.code).size() == 4);
    CHECK(FindCustomer(db.code, "C004")->sales == ApAccount{"410001", "20"});

    // A new voucher gets the next number, with its quantities, prices, roles and memo.
    ArVoucher v;
    v.mode = ar_mode::kOpen;
    v.customer = "C004";
    v.invoiceNo = "S-1007";
    v.invoiceDate = v.date = "20261025";
    v.dueDate = AddDays(v.invoiceDate, 15);
    v.period = 7;
    v.vatRate = 10;
    v.account = {"111101", ""};
    v.memo = "First order";
    ComputedSale s = ComputeSale({product("410001", "20", 150, 4400)}, 10, kAccounts);   // 1.5 x 44.00 = 66.00
    v.total = s.total;
    v.vatAmount = s.vat;
    const std::int64_t linesBefore = count(db.code, "SELECT count(*) FROM ar_btchd WHERE _deleted = 0");
    REQUIRE(SaveArVoucher(db.code, v, s.lines) == 7);
    std::vector<ArLine> l7 = LoadArLines(db.code, 7);
    REQUIRE(l7.size() == 3);
    CHECK(l7[0].qty == 150);
    CHECK(l7[0].price == 4400);
    CHECK(l7[0].gross == 6600);
    CHECK(l7[0].credit == 6000);
    CHECK(l7[1].role == ArLineRole::Vat);
    CHECK(l7[1].credit == 600);
    CHECK(l7[2].role == ArLineRole::Receivable);
    CHECK(l7[2].debit == 6600);
    CHECK(FindArVoucher(db.code, 7)->memo == "First order");
    CHECK(FindArVoucher(db.code, 7)->total == 6600);
    CHECK(count(db.code, "SELECT count(*) FROM ar_btchd WHERE _deleted = 0") == linesBefore + 3);

    // Replacing voucher 2's lines (one product line left) touches only voucher 2.
    ArVoucher v2 = *FindArVoucher(db.code, 2);
    s = ComputeSale({product("410001", "20", 0, 0, 22000)}, 10, kAccounts);
    v2.total = s.total;
    v2.vatAmount = s.vat;
    REQUIRE(SaveArVoucher(db.code, v2, s.lines) == 2);
    CHECK(LoadArLines(db.code, 2).size() == 3);
    CHECK(FindArVoucher(db.code, 2)->lines == 3);
    CHECK(FindArVoucher(db.code, 2)->total == 22000);
    CHECK(LoadArLines(db.code, 1).size() == 3);
    CHECK(LoadArLines(db.code, 7).size() == 3);
    CHECK(count(db.code, "SELECT count(*) FROM ar_linex WHERE _deleted = 0 AND batchno = 2") == 3);
    CHECK(count(db.code, "SELECT count(*) FROM ar_linex WHERE _deleted = 0") == 18 - 4 + 3 + 3);
}

TEST_CASE("CheckCustomer: what Customers' Add refuses (S6-M1)", "[ar]")
{
    FixtureDb db;
    EnsureArTables(db.code);
    const Customer c = *FindCustomer(db.code, "C001");
    CHECK_FALSE(CheckCustomer(db.code, c));
    CHECK_FALSE(CheckCustomer(db.code, *FindCustomer(db.code, "C003")));   // no VAT, no VAT account

    auto refused = [&](const Customer& x) { return CheckCustomer(db.code, x); };
    using K = CustomerProblem::Kind;
    {
        Customer x = c;
        x.number = " ";
        CHECK(refused(x)->field == CustomerField::Number);
        CHECK(refused(x)->kind == K::Required);
        x = c;
        x.shortName.clear();
        CHECK(refused(x)->field == CustomerField::ShortName);
        x = c;
        x.name = "  ";
        CHECK(refused(x)->field == CustomerField::Name);
    }
    {
        Customer x = c;
        x.vatRate = 100;
        CHECK(refused(x)->field == CustomerField::VatRate);
        CHECK(refused(x)->kind == K::BadRate);
        x.vatRate = -1;
        CHECK(refused(x)->kind == K::BadRate);
    }
    {
        Customer x = c;
        x.ar = {};
        CHECK(refused(x)->field == CustomerField::ArAccount);
        CHECK(refused(x)->kind == K::BadAccount);
        x.ar = {"111199", ""};   // the deployed customers' AR: not in the fixture's chart
        CHECK(refused(x)->field == CustomerField::ArAccount);
    }
    {
        Customer x = c;
        x.sales = {"410001", ""};   // GL_SETAT SALE: the chart has 410001 only with depts 10 and 20
        CHECK(refused(x)->field == CustomerField::SalesAccount);
        CHECK(refused(x)->kind == K::BadAccount);
        x.sales = {"410001", "20"};
        CHECK_FALSE(refused(x));
    }
    {
        Customer x = c;
        x.vat = {"999999", ""};
        CHECK(refused(x)->field == CustomerField::VatAccount);
        CHECK(refused(x)->kind == K::BadAccount);
        x.vat = {};
        CHECK(refused(x)->kind == K::VatAccountNeeded);   // VAT 10 % with nowhere to book it
        x.vatRate = 0;
        CHECK_FALSE(refused(x));
    }
    // The defaults a new customer gets (D12) pass once the sales dept is given.
    const ArAccounts a = LoadArAccounts(db.code);
    Customer n;
    n.number = "C004";
    n.shortName = "NEW";
    n.name = "New Customer";
    n.ar = a.ar;
    n.sales = a.sales;
    n.vat = a.vatPayable;
    n.vatRate = 10;
    CHECK(refused(n)->field == CustomerField::SalesAccount);
    n.sales.dept = "10";
    CHECK_FALSE(refused(n));
}

TEST_CASE("DeleteCustomer refuses a customer in use (D12)", "[ar]")
{
    FixtureDb db;
    EnsureArTables(db.code);
    std::string usedBy;
    CHECK(DeleteCustomer(db.code, "C001", &usedBy) == DeleteCustomerResult::InUse);
    CHECK(usedBy == "sales voucher 1");
    CHECK(DeleteCustomer(db.code, "C999") == DeleteCustomerResult::NotFound);

    // C003's only voucher is 5: still refused by its open item and receipt
    // once the voucher is gone, and by the receipt alone after that.
    auto run = [&](const char* table) {
        Data4 d(&db.code, table);
        Field4 no(d, "vendorno");
        for (d.top(); !d.eof(); d.skip())
            if (TrimText(no.str()) == "C003") d.deleteRec();
        d.pack();
        d.close();
    };
    run("ar_btch");
    CHECK(DeleteCustomer(db.code, "C003", &usedBy) == DeleteCustomerResult::InUse);
    CHECK(usedBy == "the open item of voucher 5");
    run("ar_dist");
    CHECK(DeleteCustomer(db.code, "C003", &usedBy) == DeleteCustomerResult::InUse);
    CHECK(usedBy == "receipt 2");
    run("ar_rcpt");
    CHECK(DeleteCustomer(db.code, " C003 ") == DeleteCustomerResult::Deleted);
    CHECK_FALSE(FindCustomer(db.code, "C003"));
    CHECK(LoadCustomers(db.code).size() == 2);

    // A customer with nothing at all goes at once; the others stay.
    Customer n;
    n.number = "C004";
    n.name = "Short-lived";
    REQUIRE(SaveCustomer(db.code, n));
    CHECK(DeleteCustomer(db.code, "C004") == DeleteCustomerResult::Deleted);
    CHECK(LoadCustomers(db.code).size() == 2);
    CHECK(FindCustomer(db.code, "C001"));
}

TEST_CASE("SaveSaleLine and DeleteSaleLine recompute the voucher (S6-M2, D4)", "[ar]")
{
    FixtureDb db;
    EnsureArTables(db.code);
    const ApAccount vat{"260007", ""};

    ArVoucher h;
    h.customer = "C001";
    h.invoiceNo = "S-2001";
    h.date = h.invoiceDate = "20261105";
    h.dueDate = "20261205";
    h.period = 11;
    h.terms = 30;
    h.vatRate = 10;
    h.account = {"111101", ""};

    // Spec #6 10: 3 x 1,100.00 at 10 %.
    SaleEntry e;
    e.account = {"410001", "10"};
    e.qty = 300;
    e.price = 110000;
    const long n = SaveSaleLine(db.code, h, e, vat);
    CHECK(n == 7);
    auto lines = LoadArLines(db.code, n);
    REQUIRE(lines.size() == 3);
    CHECK(lines[0].role == ArLineRole::Sales);
    CHECK(lines[0].credit == 300000);
    CHECK(lines[0].qty == 300);
    CHECK(lines[0].price == 110000);
    CHECK_FALSE(TrimText(lines[0].descript).empty());   // the account's name
    CHECK(lines[1].role == ArLineRole::Vat);
    CHECK(lines[1].credit == 30000);
    CHECK(lines[1].account == vat);
    CHECK(lines[2].role == ArLineRole::Receivable);
    CHECK(lines[2].debit == 330000);
    CHECK(TrimText(lines[2].account.account) == "111101");
    auto v = FindArVoucher(db.code, n);
    REQUIRE(v);
    CHECK(v->mode == ar_mode::kOpen);
    CHECK(v->total == 330000);
    CHECK(v->vatAmount == 30000);
    CHECK(v->lines == 3);

    // A second line, typed: 220.00 on 410001-20, described.
    h.number = n;
    SaleEntry t;
    t.account = {"410001", "20"};
    t.gross = 22000;
    t.descript = "Samples";
    t.product = "P1";
    t.drcode = "D1";
    t.medrep = "R1";
    CHECK(SaveSaleLine(db.code, h, t, vat) == n);
    lines = LoadArLines(db.code, n);
    REQUIRE(lines.size() == 4);
    CHECK(lines[1].credit == 20000);
    CHECK(lines[1].gross == 22000);
    CHECK(TrimText(lines[1].descript) == "Samples");
    CHECK(TrimText(lines[1].product) == "P1");
    CHECK(TrimText(lines[1].drcode) == "D1");
    CHECK(TrimText(lines[1].medrep) == "R1");
    CHECK(lines[2].credit == 32000);   // 300.00 + 20.00
    CHECK(lines[3].debit == 352000);
    v = FindArVoucher(db.code, n);
    CHECK(v->total == 352000);
    CHECK(v->vatAmount == 32000);

    // Replace line 1: 2 x 1,100.00.
    e.line = 1;
    e.qty = 200;
    CHECK(SaveSaleLine(db.code, h, e, vat) == n);
    lines = LoadArLines(db.code, n);
    REQUIRE(lines.size() == 4);
    CHECK(lines[0].credit == 200000);
    CHECK(lines[2].credit == 22000);
    CHECK(lines[3].debit == 242000);

    // Delete line 1: the typed line is left, and still balances.
    CHECK_FALSE(DeleteSaleLine(db.code, n, 3, vat));   // only two product lines
    CHECK(DeleteSaleLine(db.code, n, 1, vat));
    lines = LoadArLines(db.code, n);
    REQUIRE(lines.size() == 3);
    CHECK(TrimText(lines[0].descript) == "Samples");
    CHECK(lines[1].credit == 2000);
    CHECK(lines[2].debit == 22000);
    std::int64_t dr = 0, cr = 0;
    for (const ArLine& l : lines) {
        dr += l.debit;
        cr += l.credit;
    }
    CHECK(dr == cr);
    CHECK(FindArVoucher(db.code, n)->total == 22000);

    // The last line deleted: an empty voucher, still there.
    CHECK(DeleteSaleLine(db.code, n, 1, vat));
    CHECK(LoadArLines(db.code, n).empty());
    CHECK(FindArVoucher(db.code, n)->total == 0);
}

TEST_CASE("Sale lines are refused on a voucher that isn't open; DeleteArVoucher (D5)", "[ar]")
{
    FixtureDb db;
    EnsureArTables(db.code);
    const ApAccount vat{"260007", ""};
    SaleEntry e;
    e.account = {"410001", "10"};
    e.gross = 11000;
    for (long posted : {3L, 6L}) {   // posted, cancelled
        ArVoucher h = *FindArVoucher(db.code, posted);
        const auto before = LoadArLines(db.code, posted).size();
        CHECK(SaveSaleLine(db.code, h, e, vat) == 0);
        CHECK_FALSE(DeleteSaleLine(db.code, posted, 1, vat));
        CHECK_FALSE(DeleteArVoucher(db.code, posted));
        CHECK(LoadArLines(db.code, posted).size() == before);
        CHECK(FindArVoucher(db.code, posted)->mode == h.mode);
    }

    // Voucher 2 (open) goes, header, lines and ar_linex rows; voucher 1 stays.
    const auto one = LoadArLines(db.code, 1).size();
    const std::int64_t linex = count(db.code, "SELECT count(*) FROM ar_linex WHERE batchno <> 2");
    CHECK(DeleteArVoucher(db.code, 2));
    CHECK_FALSE(FindArVoucher(db.code, 2));
    CHECK(LoadArLines(db.code, 2).empty());
    CHECK(count(db.code, "SELECT count(*) FROM ar_linex WHERE batchno = 2") == 0);
    CHECK(count(db.code, "SELECT count(*) FROM ar_linex") == linex);
    CHECK(LoadArLines(db.code, 1).size() == one);
    CHECK(LoadArVouchers(db.code).size() == 5);
    CHECK_FALSE(DeleteArVoucher(db.code, 2));   // gone already

    // The memo (TARMemo): replaced whole, the voucher's other fields kept.
    CHECK(SaveArMemo(db.code, 1, "Deliver to ward 3.\r\nCall first."));
    const auto memo = FindArVoucher(db.code, 1);
    CHECK(memo->memo == "Deliver to ward 3.\r\nCall first.");
    CHECK(memo->total == 330000);
    CHECK(LoadArLines(db.code, 1).size() == one);
    CHECK_FALSE(SaveArMemo(db.code, 2, "x"));
}

namespace {

double arGlBalance(cb4sqlite::Code4& code, const char* account, const char* dept = "")
{
    cb4sqlite::Query q(code, "SELECT curr_bal FROM glaccts WHERE trim(account) = ?1 AND coalesce(trim(dept), '') = ?2");
    q.bindText(1, account);
    q.bindText(2, dept);
    REQUIRE(q.next());
    return q.number(0);
}

std::vector<JournalRow> arRows(cb4sqlite::Code4& code, long batchno, const char* type)
{
    std::vector<JournalRow> out;
    for (const JournalRow& j : LoadArJournal(code))
        if (j.batchno == batchno && TrimText(j.type) == type) out.push_back(j);
    return out;
}

} // namespace

TEST_CASE("PostArVouchers: the open item and the journal (S6-M3)", "[ar]")
{
    FixtureDb db;
    EnsureArTables(db.code);
    const auto items0 = LoadArOpenItems(db.code).size();

    // Voucher 1 alone: C001, 3,300.00 with 300.00 VAT.
    ApPostResult r = PostArVouchers(db.code, 1);
    CHECK(r.posted == std::vector<long>{1});
    CHECK(r.refused.empty());
    const ArVoucher v1 = *FindArVoucher(db.code, 1);
    CHECK(v1.mode == ar_mode::kPosted);
    std::optional<ArOpenItem> item;
    for (const ArOpenItem& i : LoadArOpenItems(db.code))
        if (i.batchno == 1) item = i;
    REQUIRE(item);
    CHECK(item->paymentCtl == ar_receipt::kOpen);
    CHECK(item->customer == "C001");
    CHECK(item->invoiceNo == "S-1001");
    CHECK(item->amount == 330000);
    CHECK(item->remainder == 330000);
    CHECK(item->received == 0);
    CHECK(item->vat == 30000);
    CHECK(item->ar.account == "111101");
    CHECK(item->bank.empty());
    CHECK(item->period == v1.period);
    CHECK(TrimText(item->dueDate) == TrimText(v1.dueDate));
    const auto rows = arRows(db.code, 1, "AR");
    REQUIRE(rows.size() == 3);
    std::int64_t dr = 0, cr = 0;
    for (const JournalRow& j : rows) {
        CHECK(j.mode == "1");
        CHECK(j.period == v1.period);
        CHECK(TrimText(j.date) == TrimText(v1.invoiceDate));
        dr += j.debit;
        cr += j.credit;
    }
    CHECK(dr == 330000);
    CHECK(cr == 330000);
    CHECK(PostArVouchers(db.code, 1).posted.empty());   // posted already: neither posted nor refused

    // Refusals: no lines, out of balance, an account not in the chart.
    ArVoucher h;
    h.mode = ar_mode::kOpen;
    h.customer = "C003";
    h.invoiceNo = "S-3001";
    h.date = h.invoiceDate = h.dueDate = "20261020";
    h.period = 7;
    h.account = {"111101", ""};
    const long empty = SaveArVoucher(db.code, h, {});
    ArLine s;
    s.role = ArLineRole::Sales;
    s.account = {"410001", "10"};
    s.credit = 10000;
    s.gross = 10000;
    ArLine a;
    a.role = ArLineRole::Receivable;
    a.account = {"111101", ""};
    a.debit = 9999;
    h.invoiceNo = "S-3002";
    const long unbalanced = SaveArVoucher(db.code, h, {s, a});
    a.debit = 10000;
    s.account = {"999999", ""};
    h.invoiceNo = "S-3003";
    const long unknown = SaveArVoucher(db.code, h, {s, a});

    r = PostArVouchers(db.code);   // all: 2 posts, the three are refused
    CHECK(r.posted == std::vector<long>{2});
    REQUIRE(r.refused.size() == 3);
    CHECK(r.refused[0].number == empty);
    CHECK(r.refused[0].reason == ApPostRefusal::NoLines);
    CHECK(r.refused[1].number == unbalanced);
    CHECK(r.refused[1].reason == ApPostRefusal::OutOfBalance);
    CHECK(r.refused[2].number == unknown);
    CHECK(r.refused[2].reason == ApPostRefusal::UnknownAccount);
    CHECK(r.refused[2].account.account == "999999");
    for (long n : {empty, unbalanced, unknown}) CHECK(FindArVoucher(db.code, n)->mode == ar_mode::kOpen);
    CHECK(LoadArOpenItems(db.code).size() == items0 + 2);
    CHECK(FindArVoucher(db.code, 6)->mode == ar_mode::kCancelled);   // cancelled: untouched
}

TEST_CASE("CancelArVoucher and RecoverArVoucher (D3, D7)", "[ar]")
{
    FixtureDb db;
    EnsureArTables(db.code);

    // Cancel: "X", lines kept; only an open voucher.
    const auto lines = LoadArLines(db.code, 2);
    CHECK(CancelArVoucher(db.code, 2) == ApCancelResult::Cancelled);
    CHECK(FindArVoucher(db.code, 2)->mode == ar_mode::kCancelled);
    CHECK(FindArVoucher(db.code, 2)->total == 132000);
    CHECK(LoadArLines(db.code, 2).size() == lines.size());
    CHECK(CancelArVoucher(db.code, 2) == ApCancelResult::NotOpen);
    CHECK(CancelArVoucher(db.code, 3) == ApCancelResult::NotOpen);
    CHECK(CancelArVoucher(db.code, 99) == ApCancelResult::NotFound);
    CHECK(PostArVouchers(db.code, 2).posted.empty());   // cancelled: never posted

    // Recover: 3 (owed in full) reopens; 4 and 5 have receipts; 1, 2 and 6 aren't posted.
    const auto jsum0 = LoadArJournal(db.code).size();
    CHECK(RecoverArVoucher(db.code, 4) == ArRecoverResult::Received);
    CHECK(RecoverArVoucher(db.code, 5) == ArRecoverResult::Received);
    for (long n : {1L, 2L, 6L, 99L}) CHECK(RecoverArVoucher(db.code, n) == ArRecoverResult::NotPosted);
    CHECK(RecoverArVoucher(db.code, 3) == ArRecoverResult::Recovered);
    CHECK(FindArVoucher(db.code, 3)->mode == ar_mode::kOpen);
    CHECK(arRows(db.code, 3, "AR").empty());
    CHECK(LoadArJournal(db.code).size() == jsum0 - 3);   // only 3's three rows
    for (const ArOpenItem& i : LoadArOpenItems(db.code)) CHECK(i.batchno != 3);
    CHECK(LoadArOpenItems(db.code).size() == 2);         // 4 and 5 stay
    CHECK(arRows(db.code, 4, "RC").size() == 2);         // receipts' rows untouched

    // Posting it again gives it back its open item and rows.
    CHECK(PostArVouchers(db.code, 3).posted == std::vector<long>{3});
    CHECK(arRows(db.code, 3, "AR").size() == 3);
    CHECK(LoadArOpenItems(db.code).size() == 3);
}

TEST_CASE("TransferAr: one GL voucher per period, and its undo (D2)", "[ar]")
{
    FixtureDb db;
    EnsureArTables(db.code);
    EnsureApTables(db.code);

    const ApTransferResult r = TransferAr(db.code, 99);
    CHECK(r.closedPeriods.empty());
    CHECK(r.undatedPeriods.empty());
    REQUIRE(r.vouchers.size() == 1);
    const ApTransferVoucher& t = r.vouchers[0];
    CHECK(t.period == 7);
    CHECK(t.glVoucher == 104);   // after the fixture's 100-103
    CHECK(t.rows == 12);         // vouchers 3, 4, 5 (8) and the two receipts (4)
    // Netted: 410001-10 Cr 2,800; 260007 Cr 300; 410001-20 Cr 1,000;
    // 111101 Dr 4,100 - 1,300 = 2,800; 102101 Dr 1,300.
    CHECK(t.lines == 5);
    CHECK(t.debit == 410000);
    CHECK(t.credit == 410000);

    VoucherHeaders h(db.code);
    VoucherLines l(db.code);
    VoucherHeader header;
    std::vector<VoucherLine> lines;
    REQUIRE(LoadVoucher(h, l, 104, header, lines) == LoadResult::Loaded);
    CHECK(header.mode == voucher_mode::kOpen);
    CHECK(header.description == "Accounts Receivable GL Update");
    CHECK(header.date == "20261031");
    CHECK(header.period == 7);
    std::map<std::string, double> net;
    for (const VoucherLine& v : lines) net[v.account + "-" + v.dept] += v.debit - v.credit;
    CHECK(net["111101-"] == Approx(2800.0));
    CHECK(net["102101-"] == Approx(1300.0));
    CHECK(net["410001-10"] == Approx(-2800.0));
    CHECK(net["410001-20"] == Approx(-1000.0));
    CHECK(net["260007-"] == Approx(-300.0));

    for (const JournalRow& j : LoadArJournal(db.code)) CHECK(j.mode == "2");
    for (long n : {3L, 4L, 5L}) CHECK(FindArVoucher(db.code, n)->mode == ar_mode::kTransferred);
    for (long n : {1L, 2L}) CHECK(FindArVoucher(db.code, n)->mode == ar_mode::kOpen);
    CHECK(FindArVoucher(db.code, 6)->mode == ar_mode::kCancelled);   // 2.6 #13: not "transferred"
    CHECK(RecoverArVoucher(db.code, 3) == ArRecoverResult::Transferred);
    for (const JournalRow& j : LoadJournal(db.code)) CHECK(j.mode != "2");   // AP's rows aren't AR's
    CHECK(UndoApTransfer(db.code, 104) == 0);

    // Nothing more; posting voucher 1 then gives a second voucher of its own.
    CHECK(TransferAr(db.code, 99).vouchers.empty());
    REQUIRE(PostArVouchers(db.code, 1).posted.size() == 1);
    const ApTransferResult r2 = TransferAr(db.code, 99);
    REQUIRE(r2.vouchers.size() == 1);
    CHECK(r2.vouchers[0].glVoucher == 105);
    CHECK(r2.vouchers[0].rows == 3);
    CHECK(FindArVoucher(db.code, 1)->mode == ar_mode::kTransferred);

    // Cancelling 104 puts its rows back; 105's stay transferred.
    CHECK(CancelVoucher(db.code, 104) == CancelResult::Cancelled);
    for (const JournalRow& j : LoadArJournal(db.code)) CHECK(j.mode == (j.batchno == 1 ? "2" : "1"));
    for (long n : {3L, 4L, 5L}) CHECK(FindArVoucher(db.code, n)->mode == ar_mode::kPosted);
    CHECK(FindArVoucher(db.code, 1)->mode == ar_mode::kTransferred);
    CHECK(FindArVoucher(db.code, 6)->mode == ar_mode::kCancelled);
    CHECK(UndoArTransfer(db.code, 104) == 0);   // the links went with it
    CHECK(RecoverArVoucher(db.code, 3) == ArRecoverResult::Recovered);

    // AP's transfer still runs beside it, leaving AR's rows alone.
    const ApTransferResult ap = TransferAp(db.code, 99);
    CHECK_FALSE(ap.vouchers.empty());
    for (const JournalRow& j : LoadArJournal(db.code)) CHECK(j.mode == (j.batchno == 1 ? "2" : "1"));
}

TEST_CASE("The AR transfer's voucher posts with Spec #2", "[ar]")
{
    FixtureDb db;
    EnsureArTables(db.code);
    const double ar0 = arGlBalance(db.code, "111101"), bank0 = arGlBalance(db.code, "102101"),
                 sales0 = arGlBalance(db.code, "410001", "10");
    const ApTransferResult r = TransferAr(db.code, 99);
    REQUIRE(r.vouchers.size() == 1);
    const PostResult p = PostVoucher(db.code, r.vouchers[0].glVoucher);
    REQUIRE(p.posted.size() == 1);
    // 111101 carries what's owed: 2,200.00 (voucher 3) + 600.00 (voucher 4's remainder).
    CHECK(arGlBalance(db.code, "111101") == Approx(ar0 + 2800.0));
    CHECK(arGlBalance(db.code, "102101") == Approx(bank0 + 1300.0));
    CHECK(arGlBalance(db.code, "410001", "10") == Approx(sales0 - 2800.0));
    std::int64_t owed = 0;
    for (const ArOpenItem& i : LoadArOpenItems(db.code)) owed += i.remainder;
    CHECK(owed == 280000);
}

namespace {

struct RcRow {
    std::string account, dept, mode, date, cheque;
    int period = 0;
    std::int64_t debit = 0, credit = 0;
};

std::vector<RcRow> receiptRows(cb4sqlite::Code4& code, long batchno, long receipt)
{
    std::vector<RcRow> out;
    for (const JournalRow& j : LoadArJournal(code))
        if (j.batchno == batchno && TrimText(j.type) == "RC" && TrimText(j.cheque) == std::to_string(receipt))
            out.push_back({j.account.account, j.account.dept, j.mode, TrimText(j.date), TrimText(j.cheque), j.period,
                           j.debit, j.credit});
    return out;
}

ArOpenItem openItem(cb4sqlite::Code4& code, long batchno)
{
    for (const ArOpenItem& i : LoadArOpenItems(code))
        if (i.batchno == batchno) return i;
    FAIL("no open item for voucher " << batchno);
    return {};
}

const ApAccount kBank{"102101", ""};

} // namespace

TEST_CASE("ReceiptItems: what's owed, and all (S6-M4)", "[ar]")
{
    FixtureDb db;
    EnsureArTables(db.code);
    auto items = ReceiptItems(db.code);
    REQUIRE(items.size() == 2);   // 3 (O) and 4 (R); 5 is settled
    CHECK(items[0].item.batchno == 3);
    CHECK(items[0].customerName == "CITYHOSP");
    CHECK(items[0].vatRate == 10);
    CHECK(items[0].sales == ApAccount{"410001", "10"});
    CHECK(items[0].vat == ApAccount{"260007", ""});
    CHECK(items[1].item.batchno == 4);
    CHECK(items[1].sales == ApAccount{"410001", "20"});
    CHECK(items[1].item.remainder == 60000);
    items = ReceiptItems(db.code, true);
    REQUIRE(items.size() == 3);
    CHECK(items[2].item.batchno == 5);
    CHECK(NextReceiptNumber(db.code) == 3);
    CHECK_FALSE(FindReceiptItem(db.code, 1));   // not posted: no open item
}

TEST_CASE("RecordReceipt: in full, in part, with a discount (D8, D9)", "[ar]")
{
    FixtureDb db;
    EnsureArTables(db.code);

    // Refusals.
    CHECK(RecordReceipt(db.code, 99, "20261025", kBank, 100, false).result == ReceiptResult::NotFound);
    CHECK(RecordReceipt(db.code, 5, "20261025", kBank, 100, false).result == ReceiptResult::Settled);
    CHECK(RecordReceipt(db.code, 3, "20261025", kBank, 0, false).result == ReceiptResult::Zero);
    CHECK(RecordReceipt(db.code, 3, "20261025", kBank, 220001, false).result == ReceiptResult::TooMuch);
    CHECK(RecordReceipt(db.code, 3, "20261025", kBank, 220000, true).result == ReceiptResult::NoDiscount);
    CHECK(RecordReceipt(db.code, 3, "20300101", kBank, 100, false).result == ReceiptResult::ClosedPeriod);
    CHECK(RecordReceipt(db.code, 3, "20261025", {"999999", ""}, 100, false).result == ReceiptResult::UnknownBank);
    CHECK(LoadReceipts(db.code).size() == 2);   // nothing written

    // In full: voucher 3, 2,200.00.
    ReceiptOutcome o = RecordReceipt(db.code, 3, "20261025", kBank, 220000, false, "Cheque 1001");
    CHECK(o.result == ReceiptResult::Recorded);
    CHECK(o.number == 3);
    CHECK(o.settled);
    CHECK(o.discount == 0);
    ArOpenItem i = openItem(db.code, 3);
    CHECK(i.paymentCtl == ar_receipt::kSettled);
    CHECK(i.received == 220000);
    CHECK(i.remainder == 0);
    CHECK(TrimText(i.datePaid) == "20261025");
    CHECK(i.bank == kBank);
    auto rows = receiptRows(db.code, 3, 3);
    REQUIRE(rows.size() == 2);
    CHECK(rows[0].account == "102101");
    CHECK(rows[0].debit == 220000);
    CHECK(rows[1].account == "111101");
    CHECK(rows[1].credit == 220000);
    for (const RcRow& r : rows) {
        CHECK(r.mode == "1");
        CHECK(r.period == 7);
        CHECK(r.date == "20261025");
    }
    CHECK(RecoverArVoucher(db.code, 3) == ArRecoverResult::Received);

    // Two partials on voucher 4 (600.00 owed after the fixture's 500.00).
    o = RecordReceipt(db.code, 4, "20261026", kBank, 20000, false);
    CHECK(o.number == 4);
    CHECK_FALSE(o.settled);
    i = openItem(db.code, 4);
    CHECK(i.paymentCtl == ar_receipt::kPartial);
    CHECK(i.received == 70000);
    CHECK(i.remainder == 40000);
    CHECK(RecordReceipt(db.code, 4, "20261027", kBank, 10000, false).number == 5);
    i = openItem(db.code, 4);
    CHECK(i.received == 80000);
    CHECK(i.remainder == 30000);
    CHECK(TrimText(i.datePaid) == "20261027");

    // Then 190.00 with a discount: 110.00 written off, 10.00 of it VAT (10 %).
    o = RecordReceipt(db.code, 4, "20261028", kBank, 19000, true);
    CHECK(o.number == 6);
    CHECK(o.settled);
    CHECK(o.discount == 11000);
    i = openItem(db.code, 4);
    CHECK(i.paymentCtl == ar_receipt::kSettled);
    CHECK(i.received == 99000);
    CHECK(i.remainder == 0);
    rows = receiptRows(db.code, 4, 6);
    REQUIRE(rows.size() == 4);
    CHECK(rows[0].account == "102101");
    CHECK(rows[0].debit == 19000);
    CHECK(rows[1].account == "410001");
    CHECK(rows[1].dept == "20");
    CHECK(rows[1].debit == 10000);
    CHECK(rows[2].account == "260007");
    CHECK(rows[2].debit == 1000);
    CHECK(rows[3].account == "111101");
    CHECK(rows[3].credit == 30000);   // the whole remainder
    const auto receipts = LoadReceipts(db.code, 4);
    REQUIRE(receipts.size() == 4);
    CHECK(receipts.back().discount == 11000);
    CHECK(receipts.back().discountVat == 1000);
    CHECK(ReceiptItems(db.code).empty());   // everything settled
}

TEST_CASE("ReverseReceipt: the latest, waiting or transferred (D10)", "[ar]")
{
    FixtureDb db;
    EnsureArTables(db.code);
    CHECK(ReverseReceipt(db.code, 3, "20261028") == ReverseResult::NoReceipt);

    // Waiting: voucher 4's two partials, latest first, then the fixture's receipt 1.
    REQUIRE(RecordReceipt(db.code, 4, "20261026", kBank, 20000, false).number == 3);
    REQUIRE(RecordReceipt(db.code, 4, "20261027", kBank, 40000, false).number == 4);
    CHECK(openItem(db.code, 4).paymentCtl == ar_receipt::kSettled);
    const auto jsum = LoadArJournal(db.code).size();
    CHECK(ReverseReceipt(db.code, 4, "20261028") == ReverseResult::Reversed);
    CHECK(receiptRows(db.code, 4, 4).empty());
    CHECK(LoadArJournal(db.code).size() == jsum - 2);
    ArOpenItem i = openItem(db.code, 4);
    CHECK(i.paymentCtl == ar_receipt::kPartial);
    CHECK(i.received == 70000);
    CHECK(i.remainder == 40000);
    CHECK(TrimText(i.datePaid) == "20261026");
    CHECK(ReverseReceipt(db.code, 4, "20261028") == ReverseResult::Reversed);
    CHECK(ReverseReceipt(db.code, 4, "20261028") == ReverseResult::Reversed);   // the fixture's receipt 1
    i = openItem(db.code, 4);
    CHECK(i.paymentCtl == ar_receipt::kOpen);
    CHECK(i.received == 0);
    CHECK(i.remainder == 110000);
    CHECK(TrimText(i.datePaid).empty());
    CHECK(i.bank.empty());
    CHECK(LoadReceipts(db.code, 4).empty());
    CHECK(ReverseReceipt(db.code, 4, "20261028") == ReverseResult::NoReceipt);
    CHECK(RecoverArVoucher(db.code, 4) == ArRecoverResult::Recovered);   // no receipt left

    // Transferred: voucher 3 in full, then the transfer; the reversal is new rows.
    const ReceiptOutcome o = RecordReceipt(db.code, 3, "20261025", kBank, 220000, false);
    REQUIRE(o.result == ReceiptResult::Recorded);
    REQUIRE(TransferAr(db.code, 99).vouchers.size() == 1);
    CHECK(ReverseReceipt(db.code, 3, "20300101") == ReverseResult::ClosedPeriod);   // no open period then
    CHECK(ReverseReceipt(db.code, 3, "20261029") == ReverseResult::Reversed);
    const auto rows = receiptRows(db.code, 3, o.number);
    REQUIRE(rows.size() == 4);
    CHECK(rows[0].mode == "2");
    CHECK(rows[1].mode == "2");
    CHECK(rows[2].mode == "1");
    CHECK(rows[2].date == "20261029");
    CHECK(rows[2].account == "102101");
    CHECK(rows[2].credit == 220000);   // the bank's debit reversed
    CHECK(rows[3].account == "111101");
    CHECK(rows[3].debit == 220000);
    i = openItem(db.code, 3);
    CHECK(i.paymentCtl == ar_receipt::kOpen);
    CHECK(i.remainder == 220000);
    CHECK(NextReceiptNumber(db.code) == o.number + 1);   // a transferred receipt's number isn't reused
}

TEST_CASE("PurgeReceived: settled items received before a date, with their receipts (S6-M5, D11)", "[ar]")
{
    FixtureDb db;
    EnsureArTables(db.code);
    // Voucher 5 is settled, last received 15 Oct; 4 is partly received; 3 is owed in full.
    CHECK(ArPurgeCandidates(db.code, "20261015").empty());   // on the date: not before it
    auto c = ArPurgeCandidates(db.code, "20261016");
    REQUIRE(c.size() == 1);
    CHECK(c[0].batchno == 5);
    CHECK(ArPurgeCandidates(db.code, "").empty());

    // Settle 3 on 25 Oct; a partial on 4 (still R).
    REQUIRE(RecordReceipt(db.code, 3, "20261025", kBank, 220000, false).settled);
    REQUIRE(RecordReceipt(db.code, 4, "20261025", kBank, 10000, false).result == ReceiptResult::Recorded);
    c = ArPurgeCandidates(db.code, "20261101");
    REQUIRE(c.size() == 2);
    CHECK(c[0].batchno == 3);
    CHECK(c[1].batchno == 5);

    const auto jsum = LoadArJournal(db.code).size();
    const auto vouchers = LoadArVouchers(db.code).size();
    CHECK(PurgeReceived(db.code, "", 0).items == 0);

    // Double-click one: only 3, with its receipt.
    ArPurgeOutcome o = PurgeReceived(db.code, "20261101", 3);
    CHECK(o.items == 1);
    CHECK(o.receipts == 1);
    CHECK(LoadReceipts(db.code, 3).empty());
    CHECK(ArPurgeCandidates(db.code, "20261101").size() == 1);
    CHECK(PurgeReceived(db.code, "20261101", 4).items == 0);   // not settled: never

    // Then all: 5 and its receipt 2.
    o = PurgeReceived(db.code, "20261101");
    CHECK(o.items == 1);
    CHECK(o.receipts == 1);
    const auto items = LoadArOpenItems(db.code);
    REQUIRE(items.size() == 1);
    CHECK(items[0].batchno == 4);
    CHECK(items[0].paymentCtl == ar_receipt::kPartial);
    CHECK(LoadReceipts(db.code, 4).size() == 2);
    CHECK(LoadReceipts(db.code).size() == 2);
    // The vouchers and the journal are the record: kept.
    CHECK(LoadArJournal(db.code).size() == jsum);
    CHECK(LoadArVouchers(db.code).size() == vouchers);
    CHECK(FindArVoucher(db.code, 3)->mode == ar_mode::kPosted);
    CHECK(ArPurgeCandidates(db.code, "20261101").empty());
}
