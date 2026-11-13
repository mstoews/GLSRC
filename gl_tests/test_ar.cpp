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

#include <map>
#include <string>
#include <vector>

using namespace gltest;

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
