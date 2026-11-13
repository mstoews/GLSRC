//
// gl_tests: the Accounts Payable core (spec #5 2.1, 4.1, D3, D4, D6; plan
// S5-M0) on the dev fixture's AP data (make_dev_fixture.py accounts_payable):
//
//   V001 company,    VAT 12 %, W/H 1 %, terms 30    V002 individual, W/H 2 %    V003 no VAT or W/H
//
//   invoice 1  open        V001 INV-1001  1,120.00 -> 1,000.00 + VAT 120.00; W/H 10.00; AP 1,110.00
//   invoice 2  open        V001 INV-1002  560.00 + 224.00 -> 700.00 + VAT 84.00; W/H 7.00; AP 777.00
//   invoice 3  posted      V002 INV-2001  5,000.00, W/H 100.00, AP 4,900.00 (no ap_linex rows)
//   invoice 4  cancelled   V003 INV-3001  ("X")
//   invoice 5  "2" + "Cancelled": migrated to "X"
//   invoice 6  posted      V003 INV-3002  1,200.00, cheque 5001 printed, not cleared
//   cash 1     open        V001 INV-1003  336.00 -> 300.00 + VAT 36.00; W/H 3.00; bank 102101 333.00
//
#include "GlTestDb.h"

#include "Ap.h"
#include "Vouchers.h"
#include "reports/ApReports.h"

#include <string>
#include <vector>

using namespace gltest;
using Catch::Approx;

namespace {

ApLine expense(const char* account, std::int64_t gross, const char* dept = "")
{
    ApLine l;
    l.account = {account, dept};
    l.gross = gross;
    return l;
}

const VoucherAccounts kAccounts{{"148000", ""}, {"260004", ""}, {"221002", ""}, "PREPAID VAT",
                                "WITHHOLDING TAX PAYABLE", "ACCOUNTS PAYABLE - TRADE"};

} // namespace

TEST_CASE("SplitAmount, F mode: VAT included, W/H on the net", "[ap]")
{
    // Spec #5 10: V001's 1,120.00 at 12 % / 1 %.
    AmountSplit s = SplitAmount(112000, 12, 1, TaxMode::F);
    CHECK(s.expense == 100000);
    CHECK(s.vat == 12000);
    CHECK(s.wh == 1000);
    // The deployed sample (voucher 4): 1,100.00 at 10 % / 1 % booked 1,000.00 / 100.00 / 10.00.
    s = SplitAmount(110000, 10, 1, TaxMode::F);
    CHECK(s.expense == 100000);
    CHECK(s.vat == 10000);
    CHECK(s.wh == 1000);
    // No VAT, no W/H: 0 means none (the original's sentinel rate of 1 was a real 100 % in T mode).
    s = SplitAmount(50000, 0, 0, TaxMode::F);
    CHECK(s.expense == 50000);
    CHECK(s.vat == 0);
    CHECK(s.wh == 0);
    // Rounding: 1,125.00 at 12 % is VAT 112,500 x 12 / 112 = 12,053.57 cents -> 120.54.
    s = SplitAmount(112500, 12, 1, TaxMode::F);
    CHECK(s.vat == 12054);
    CHECK(s.expense == 100446);
    CHECK(s.wh == 1004);   // 1,004.46 x 1 % = 10.0446 -> 10.04
    CHECK(s.expense + s.vat == 112500);
}

TEST_CASE("SplitAmount, T mode: the amount grossed up, W/H debited", "[ap]")
{
    // Spec #5 2.3: 1,120.00 at 12 % / 1 %: 991.15 / 118.94 / 9.91.
    const AmountSplit s = SplitAmount(112000, 12, 1, TaxMode::T);
    CHECK(s.expense == 99115);
    CHECK(s.vat == 11894);
    CHECK(s.wh == 991);
    CHECK(s.expense + s.vat + s.wh == 112000);
    // A rate of 0 is none in T mode too: no VAT booked equal to the expense (spec #5 2.6 #5).
    const AmountSplit none = SplitAmount(50000, 0, 0, TaxMode::T);
    CHECK(none.expense == 50000);
    CHECK(none.vat == 0);
    CHECK(none.wh == 0);
}

TEST_CASE("ParseTaxMode and GrossFromExpense", "[ap]")
{
    CHECK(ParseTaxMode("T") == TaxMode::T);
    CHECK(ParseTaxMode(" t ") == TaxMode::T);
    CHECK(ParseTaxMode("F") == TaxMode::F);
    CHECK(ParseTaxMode("") == TaxMode::F);   // the default (spec #5 4.1)
    // F mode: the inverse gives an amount whose split has that expense. (It
    // can't give back the amount typed: 11.24 and 11.25 both give 10.04
    // at 12 %, which is why ap_linex keeps the amount.)
    for (std::int64_t e : {1LL, 99LL, 100000LL, 100400LL, 100446LL, 123457LL})
        CHECK(SplitAmount(GrossFromExpense(e, 12, 1, TaxMode::F), 12, 1, TaxMode::F).expense == e);
    CHECK(SplitAmount(1124, 12, 1, TaxMode::F).expense == 1004);
    CHECK(SplitAmount(1125, 12, 1, TaxMode::F).expense == 1004);
    CHECK(GrossFromExpense(100000, 12, 1, TaxMode::F) == 112000);
    CHECK(GrossFromExpense(500000, 0, 2, TaxMode::F) == 500000);
}

TEST_CASE("ComputeVoucher: the VAT, W/H and credit lines from the expense lines", "[ap]")
{
    SECTION("F mode, two expense lines (the fixture's invoice 2)")
    {
        const ComputedVoucher c =
            ComputeVoucher({expense("501000", 56000), expense("751000", 22400)}, 12, 1, TaxMode::F, kAccounts);
        REQUIRE(c.lines.size() == 5);
        CHECK(c.lines[0].debit == 50000);
        CHECK(c.lines[0].role == ApLineRole::Expense);
        CHECK(c.lines[0].gross == 56000);
        CHECK(c.lines[1].debit == 20000);
        CHECK(c.lines[2].role == ApLineRole::Vat);
        CHECK(c.lines[2].account.account == "148000");
        CHECK(c.lines[2].debit == 8400);
        CHECK(c.lines[3].role == ApLineRole::WithHolding);
        CHECK(c.lines[3].credit == 700);
        CHECK(c.lines[4].role == ApLineRole::Credit);
        CHECK(c.lines[4].account.account == "221002");
        CHECK(c.lines[4].credit == 77700);
        CHECK(c.lines[4].descript == "ACCOUNTS PAYABLE - TRADE");
        CHECK(c.payable == 78400);
        CHECK(c.vat == 8400);
        CHECK(c.wh == 700);
        CHECK(c.debits == c.credits);
        for (int i = 0; i < 5; ++i) CHECK(c.lines[static_cast<size_t>(i)].line == i + 1);
    }
    SECTION("Deleting a line recomputes")
    {
        const ComputedVoucher c = ComputeVoucher({expense("501000", 56000)}, 12, 1, TaxMode::F, kAccounts);
        CHECK(c.vat == 6000);
        CHECK(c.wh == 500);
        CHECK(c.credit == 55500);
        CHECK(c.debits == c.credits);
    }
    SECTION("No VAT or W/H: only the expense and AP lines")
    {
        const ComputedVoucher c = ComputeVoucher({expense("751000", 150000)}, 0, 0, TaxMode::F, kAccounts);
        REQUIRE(c.lines.size() == 2);
        CHECK(c.lines[1].role == ApLineRole::Credit);
        CHECK(c.lines[1].credit == 150000);
    }
    SECTION("T mode: W/H is a debit and AP the full amount")
    {
        const ComputedVoucher c = ComputeVoucher({expense("501000", 112000)}, 12, 1, TaxMode::T, kAccounts);
        REQUIRE(c.lines.size() == 4);
        CHECK(c.lines[2].role == ApLineRole::WithHolding);
        CHECK(c.lines[2].debit == 991);
        CHECK(c.lines[3].credit == 112000);
        CHECK(c.debits == c.credits);
    }
    SECTION("Many lines still balance to the cent")
    {
        std::vector<ApLine> lines;
        for (int i = 1; i <= 37; ++i) lines.push_back(expense("501000", 1000 + 337 * i));
        for (const TaxMode m : {TaxMode::F, TaxMode::T}) {
            const ComputedVoucher c = ComputeVoucher(lines, 12, 1, m, kAccounts);
            CHECK(c.debits == c.credits);
            CHECK(c.debits == c.payable);   // the expenses, VAT (and T-mode W/H) make up the typed amounts
        }
    }
}

TEST_CASE("The fixture's AP data", "[ap]")
{
    FixtureDb db;
    CHECK(EnsureApTables(db.code) == 1);   // invoice 5, the original's cancel
    CHECK(EnsureApTables(db.code) == 0);   // only once

    const auto vendors = LoadVendors(db.code);
    REQUIRE(vendors.size() == 3);
    CHECK(vendors[0].number == "V001");
    CHECK(vendors[0].name == "Office Supplies Co.");
    CHECK(vendors[0].vatRate == 12);
    CHECK(vendors[0].whRate == 1);
    CHECK(vendors[0].terms == 30);
    CHECK(vendors[0].ap.account == "221002");
    CHECK(vendors[0].vat.account == "148000");
    CHECK_FALSE(vendors[0].individual());
    CHECK(vendors[1].individual());
    CHECK(FindVendor(db.code, " V003 ")->name == "City Landlord Inc.");
    CHECK_FALSE(FindVendor(db.code, "V999"));

    const auto invoices = LoadApVouchers(db.code, ApKind::Invoice);
    REQUIRE(invoices.size() == 6);
    CHECK(invoices[0].mode == ap_mode::kOpen);
    CHECK(invoices[0].payable == 112000);
    CHECK(invoices[0].vatAmount == 12000);
    CHECK(invoices[0].whAmount == -1000);
    CHECK(invoices[0].invoiceNo == "INV-1001");
    CHECK(invoices[0].account.account == "221002");
    CHECK(invoices[0].period == 7);
    CHECK(invoices[0].dueDate == "20261104");
    CHECK(invoices[2].mode == ap_mode::kPosted);
    CHECK(invoices[3].cancelled());
    CHECK(invoices[4].cancelled());   // migrated
    CHECK(invoices[5].mode == ap_mode::kPosted);

    const auto cash = LoadApVouchers(db.code, ApKind::Cash);
    REQUIRE(cash.size() == 1);
    CHECK(cash[0].cheque == "000123");
    CHECK(cash[0].account.account == "102101");
    CHECK(cash[0].vat.account == "148000");
    CHECK(cash[0].payable == 33600);

    // Invoice 2's lines, as the port wrote them (ap_linex), agree with ComputeVoucher.
    const auto lines = LoadApLines(db.code, ApKind::Invoice, 2);
    REQUIRE(lines.size() == 5);
    CHECK(lines[0].gross == 56000);
    CHECK(lines[1].gross == 22400);
    CHECK(lines[1].account.account == "751000");
    CHECK(lines[2].role == ApLineRole::Vat);
    CHECK(lines[3].role == ApLineRole::WithHolding);
    CHECK(lines[4].role == ApLineRole::Credit);
    const ComputedVoucher c = ComputeVoucher({lines[0], lines[1]}, 12, 1, TaxMode::F, kAccounts);
    for (size_t i = 0; i < 5; ++i) {
        CHECK(c.lines[i].debit == lines[i].debit);
        CHECK(c.lines[i].credit == lines[i].credit);
    }

    // Invoice 3 has no ap_linex rows: its lines are classified.
    const auto three = LoadApLines(db.code, ApKind::Invoice, 3);
    REQUIRE(three.size() == 3);
    CHECK(three[0].role == ApLineRole::Expense);
    CHECK(three[0].account.dept == "20");
    CHECK(three[0].gross == 500000);
    CHECK(three[1].role == ApLineRole::WithHolding);
    CHECK(three[2].role == ApLineRole::Credit);

    const auto cashLines = LoadApLines(db.code, ApKind::Cash, 1);
    REQUIRE(cashLines.size() == 4);
    CHECK(cashLines[3].role == ApLineRole::Credit);
    CHECK(cashLines[3].credit == 33300);

    const auto items = LoadOpenItems(db.code);
    REQUIRE(items.size() == 2);
    CHECK(items[0].batchno == 3);
    CHECK(items[0].paymentCtl == ap_payment::kOpen);
    CHECK(items[0].amount == 500000);
    CHECK(items[0].wh == -10000);
    CHECK(items[0].bank.empty());
    CHECK(items[1].paymentCtl == ap_payment::kPrinted);
    CHECK(items[1].cheque == 5001);
    CHECK(items[1].bank.account == "102101");

    const auto cheques = LoadCheques(db.code);
    REQUIRE(cheques.size() == 1);
    CHECK(cheques[0].number == 5001);
    CHECK(cheques[0].amount == 120000);
    CHECK_FALSE(cheques[0].cleared);

    const auto journal = LoadJournal(db.code);
    REQUIRE(journal.size() == 7);
    std::int64_t dr = 0, cr = 0;
    for (const JournalRow& r : journal) {
        CHECK(r.mode == "1");
        CHECK(r.period == 7);
        CHECK(r.type == "AP");
        dr += r.debit;
        cr += r.credit;
    }
    CHECK(dr == cr);
    CHECK(journal[5].cheque == "5001");
}

TEST_CASE("The GL_SETAT accounts and banks", "[ap]")
{
    FixtureDb db;
    const ApAccounts a = LoadApAccounts(db.code);
    CHECK(a.ap.account == "221002");
    CHECK(a.vatInput.account == "148000");
    CHECK(a.vatPayable.account == "260007");
    CHECK(a.whCompany.account == "260004");
    CHECK(a.whIndividual.account == "260004");
    REQUIRE(a.banks.size() == 2);
    CHECK(a.banks[0].shortName == "CA");
    CHECK(a.banks[0].account.account == "101000");
    CHECK(a.banks[1].shortName == "CHCK");
    CHECK(a.banks[1].account.account == "102101");
}

TEST_CASE("DuplicateInvoice (D6)", "[ap]")
{
    FixtureDb db;
    EnsureApTables(db.code);
    const auto dup = DuplicateInvoice(db.code, "V001", "INV-1001");
    REQUIRE(dup);
    CHECK(dup->kind == ApKind::Invoice);
    CHECK(dup->number == 1);
    // Across kinds: the cash voucher's number.
    const auto cash = DuplicateInvoice(db.code, "V001", " INV-1003 ");
    REQUIRE(cash);
    CHECK(cash->kind == ApKind::Cash);
    // Not for the voucher itself, another vendor, or a cancelled voucher (4, and 5 once migrated).
    CHECK_FALSE(DuplicateInvoice(db.code, "V001", "INV-1001", ApVoucherRef{ApKind::Invoice, 1}));
    CHECK_FALSE(DuplicateInvoice(db.code, "V002", "INV-1001"));
    CHECK_FALSE(DuplicateInvoice(db.code, "V003", "INV-3001"));
    CHECK_FALSE(DuplicateInvoice(db.code, "V003", "INV-3000"));
    CHECK_FALSE(DuplicateInvoice(db.code, "V001", ""));
}

TEST_CASE("Saving vendors and vouchers", "[ap]")
{
    FixtureDb db;
    EnsureApTables(db.code);

    Vendor v = *FindVendor(db.code, "V002");
    v.whRate = 5;
    CHECK_FALSE(SaveVendor(db.code, v));   // replaced
    Vendor n;
    n.number = "V004";
    n.name = "New Vendor";
    n.type = "C";
    n.vatRate = 12;
    n.ap = {"221002", ""};
    CHECK(SaveVendor(db.code, n));        // added
    const auto vendors = LoadVendors(db.code);
    REQUIRE(vendors.size() == 4);
    CHECK(vendors[1].whRate == 5);
    CHECK(vendors[1].name == "J. Santos (consultant)");   // the rest kept
    CHECK(vendors[3].number == "V004");
    CHECK(vendors[0].name == "Office Supplies Co.");      // appending left the others intact

    // A new invoice: numbered after the last, its lines and roles written.
    ApVoucher h;
    h.mode = ap_mode::kOpen;
    h.vendor = "V004";
    h.invoiceNo = "N-1";
    h.period = 7;
    h.vatRate = 12;
    h.account = {"221002", ""};
    const ComputedVoucher c = ComputeVoucher({expense("501000", 22400)}, 12, 0, TaxMode::F, kAccounts);
    h.payable = c.payable;
    h.vatAmount = c.vat;
    h.whAmount = -c.wh;
    const long number = SaveApVoucher(db.code, h, c.lines);
    CHECK(number == 7);
    auto saved = FindApVoucher(db.code, ApKind::Invoice, 7);
    REQUIRE(saved);
    CHECK(saved->payable == 22400);
    CHECK(saved->lines == 3);
    auto lines = LoadApLines(db.code, ApKind::Invoice, 7);
    REQUIRE(lines.size() == 3);
    CHECK(lines[0].gross == 22400);
    CHECK(lines[0].debit == 20000);
    CHECK(lines[2].role == ApLineRole::Credit);

    // Replacing its lines: the old ones and their roles go; other vouchers' stay.
    h.number = 7;
    const ComputedVoucher c2 =
        ComputeVoucher({expense("501000", 22400), expense("751000", 11200)}, 12, 0, TaxMode::F, kAccounts);
    h.payable = c2.payable;
    h.vatAmount = c2.vat;
    CHECK(SaveApVoucher(db.code, h, c2.lines) == 7);
    lines = LoadApLines(db.code, ApKind::Invoice, 7);
    REQUIRE(lines.size() == 4);
    CHECK(lines[1].gross == 11200);
    CHECK(lines[2].role == ApLineRole::Vat);
    CHECK(lines[2].debit == 3600);
    CHECK(FindApVoucher(db.code, ApKind::Invoice, 7)->lines == 4);
    CHECK(LoadApLines(db.code, ApKind::Invoice, 2).size() == 5);
    CHECK(LoadApLines(db.code, ApKind::Invoice, 2)[1].gross == 22400);
    CHECK(LoadApLines(db.code, ApKind::Cash, 1)[0].gross == 33600);   // the cash voucher 1's roles untouched

    // A cash voucher: its own number sequence, cheque and VAT account.
    ApVoucher k;
    k.kind = ApKind::Cash;
    k.mode = ap_mode::kOpen;
    k.vendor = "V003";
    k.invoiceNo = "C-77";
    k.cheque = "000124";
    k.account = {"102101", ""};
    k.vat = {"148000", ""};
    k.payable = 5000;
    CHECK(SaveApVoucher(db.code, k, {}) == 2);
    CHECK(FindApVoucher(db.code, ApKind::Cash, 2)->cheque == "000124");
    CHECK(FindApVoucher(db.code, ApKind::Cash, 2)->vat.account == "148000");
}

TEST_CASE("EnsureApTables creates the tables in an empty company", "[ap]")
{
    EmptyDb db;
    CHECK(EnsureApTables(db.code) == 0);
    CHECK(LoadVendors(db.code).empty());
    CHECK(LoadApVouchers(db.code, ApKind::Invoice).empty());
    CHECK(LoadApVouchers(db.code, ApKind::Cash).empty());
    CHECK(LoadOpenItems(db.code).empty());
    CHECK(LoadCheques(db.code).empty());
    CHECK(LoadJournal(db.code).empty());
    CHECK(NextApVoucherNumber(db.code, ApKind::Invoice) == 1);

    Vendor v;
    v.number = "A1";
    v.name = "First";
    CHECK(SaveVendor(db.code, v));
    ApVoucher h;
    h.vendor = "A1";
    h.invoiceNo = "X";
    h.mode = ap_mode::kOpen;
    CHECK(SaveApVoucher(db.code, h, ComputeVoucher({expense("501000", 1000)}, 0, 0, TaxMode::F, kAccounts).lines) == 1);
    CHECK(LoadApLines(db.code, ApKind::Invoice, 1).size() == 2);
    CHECK(EnsureApTables(db.code) == 0);   // the tables exist now
}

TEST_CASE("CheckVendor: what Vendors' Add refuses (S5-M1)", "[ap]")
{
    FixtureDb db;
    EnsureApTables(db.code);
    Vendor v = *FindVendor(db.code, "V001");
    CHECK_FALSE(CheckVendor(db.code, v));

    auto refused = [&](Vendor x) { return CheckVendor(db.code, x); };
    using K = VendorProblem::Kind;
    {
        Vendor x = v;
        x.number = " ";
        CHECK(refused(x)->field == VendorField::Number);
        CHECK(refused(x)->kind == K::Required);
    }
    {
        Vendor x = v;
        x.shortName.clear();
        CHECK(refused(x)->field == VendorField::ShortName);
    }
    {
        Vendor x = v;
        x.name.clear();
        CHECK(refused(x)->field == VendorField::Name);
    }
    {
        Vendor x = v;
        x.type.clear();   // the deployed vendors have types like "R" and "X"
        CHECK(refused(x)->kind == K::NoType);
        x.type = "R";
        CHECK(refused(x)->kind == K::NoType);
    }
    {
        Vendor x = v;
        x.vatRate = 100;
        CHECK(refused(x)->field == VendorField::VatRate);
        x.vatRate = 12;
        x.whRate = -1;
        CHECK(refused(x)->field == VendorField::WhRate);
    }
    {
        Vendor x = v;
        x.ap = {"999999", ""};
        CHECK(refused(x)->field == VendorField::ApAccount);
        CHECK(refused(x)->kind == K::BadAccount);
        x.ap = {"221002", "10"};   // 221002 has no dept 10
        CHECK(refused(x)->field == VendorField::ApAccount);
    }
    {
        Vendor x = v;
        x.vat = {"999999", ""};
        CHECK(refused(x)->field == VendorField::VatAccount);
        x.vat = {};
        CHECK(refused(x)->kind == K::VatAccountNeeded);   // VAT 12 % with nowhere to book it
        x.vatRate = 0;
        CHECK_FALSE(refused(x));                           // no VAT, no VAT account: fine
    }
}

TEST_CASE("DeleteVendor refuses a vendor in use (D12)", "[ap]")
{
    FixtureDb db;
    EnsureApTables(db.code);
    std::string usedBy;
    CHECK(DeleteVendor(db.code, "V001", &usedBy) == DeleteVendorResult::InUse);
    CHECK(usedBy == "invoice voucher 1");
    CHECK(DeleteVendor(db.code, "V003", &usedBy) == DeleteVendorResult::InUse);
    CHECK(usedBy == "invoice voucher 4");   // cancelled vouchers count: they're the record
    CHECK(DeleteVendor(db.code, "V999") == DeleteVendorResult::NotFound);

    Vendor n;
    n.number = "V004";
    n.name = "Unused";
    SaveVendor(db.code, n);
    CHECK(DeleteVendor(db.code, "V004") == DeleteVendorResult::Deleted);
    CHECK_FALSE(FindVendor(db.code, "V004"));
    CHECK(LoadVendors(db.code).size() == 3);   // the others stay

    // A cash voucher alone keeps a vendor too.
    Vendor c;
    c.number = "V005";
    c.name = "Cash only";
    SaveVendor(db.code, c);
    ApVoucher k;
    k.kind = ApKind::Cash;
    k.vendor = "V005";
    k.mode = ap_mode::kOpen;
    SaveApVoucher(db.code, k, {});
    CHECK(DeleteVendor(db.code, "V005", &usedBy) == DeleteVendorResult::InUse);
    CHECK(usedBy == "cash voucher 2");
}

TEST_CASE("AddDays: the due date (S5-M2)", "[ap]")
{
    CHECK(AddDays("20261005", 30) == "20261104");
    CHECK(AddDays("20261012", 30) == "20261111");
    CHECK(AddDays("20270215", 15) == "20270302");   // February, not a leap year
    CHECK(AddDays("20281215", 30) == "20290114");   // over the year end
    CHECK(AddDays("20280228", 1) == "20280229");    // a leap year
    CHECK(AddDays("20261005", 0) == "20261005");
    CHECK(AddDays("", 30).empty());
    CHECK(AddDays("2026105", 30).empty());
}

TEST_CASE("SaveExpenseLine / DeleteExpenseLine recompute the voucher (D4)", "[ap]")
{
    FixtureDb db;
    EnsureApTables(db.code);
    const VoucherAccounts acc{{"148000", ""}, {"260004", ""}, {"221002", ""}, "", "", ""};

    // Spec #5 10: a V001 invoice of 1,120.00 in F mode.
    ApVoucher h;
    h.vendor = "V001";
    h.vendorName = "Office Supplies Co.";
    h.invoiceNo = "INV-1009";
    h.reference = "Toner";
    h.invoiceDate = h.date = "20261020";
    h.dueDate = AddDays(h.invoiceDate, 30);
    h.period = 7;
    h.terms = 30;
    h.vatRate = 12;
    h.whRate = 1;
    h.account = {"221002", ""};
    ExpenseEntry e;
    e.account = {"501000", ""};
    e.gross = 112000;
    const long n = SaveExpenseLine(db.code, h, e, TaxMode::F, acc);
    CHECK(n == 7);
    auto v = FindApVoucher(db.code, ApKind::Invoice, n);
    REQUIRE(v);
    CHECK(v->mode == ap_mode::kOpen);
    CHECK(v->payable == 112000);
    CHECK(v->vatAmount == 12000);
    CHECK(v->whAmount == -1000);
    CHECK(v->dueDate == "20261119");
    auto lines = LoadApLines(db.code, ApKind::Invoice, n);
    REQUIRE(lines.size() == 4);
    CHECK(lines[0].debit == 100000);   // Dr expense 1,000.00
    CHECK(lines[0].descript == "COST OF SALES");   // a blank description gets the account's name
    CHECK(lines[1].debit == 12000);    // Dr VAT 120.00
    CHECK(lines[1].descript == "PREPAID VAT");
    CHECK(lines[2].credit == 1000);    // Cr W/H 10.00
    CHECK(lines[3].credit == 111000);  // Cr AP 1,110.00

    // A second expense line recomputes; the voucher still balances.
    h.number = n;
    ExpenseEntry e2;
    e2.account = {"751000", ""};
    e2.descript = "Shelving";
    e2.gross = 56000;
    e2.medrep = "MR001";
    e2.product = "P100";
    e2.drcode = "DR002";
    CHECK(SaveExpenseLine(db.code, h, e2, TaxMode::F, acc) == n);
    lines = LoadApLines(db.code, ApKind::Invoice, n);
    REQUIRE(lines.size() == 5);
    CHECK(lines[1].descript == "Shelving");
    CHECK(lines[1].product == "P100");    // D13: each code in its own column
    CHECK(lines[1].drcode == "DR002");
    CHECK(lines[1].medrep == "MR001");
    std::int64_t dr = 0, cr = 0;
    for (const ApLine& l : lines) {
        dr += l.debit;
        cr += l.credit;
    }
    CHECK(dr == cr);
    CHECK(lines[4].credit == 168000 - 1500);   // AP = 1,680.00 - W/H (10.00 + 5.00)
    CHECK(FindApVoucher(db.code, ApKind::Invoice, n)->payable == 168000);

    // Changing line 1's amount replaces it, not appends.
    e.line = 1;
    e.gross = 224000;
    CHECK(SaveExpenseLine(db.code, h, e, TaxMode::F, acc) == n);
    lines = LoadApLines(db.code, ApKind::Invoice, n);
    REQUIRE(lines.size() == 5);
    CHECK(lines[0].gross == 224000);
    CHECK(lines[0].debit == 200000);
    CHECK(FindApVoucher(db.code, ApKind::Invoice, n)->vatAmount == 24000 + 6000);

    // Deleting line 1 recomputes from line 2 alone.
    CHECK(DeleteExpenseLine(db.code, ApKind::Invoice, n, 1, TaxMode::F, acc));
    lines = LoadApLines(db.code, ApKind::Invoice, n);
    REQUIRE(lines.size() == 4);
    CHECK(lines[0].descript == "Shelving");
    CHECK(lines[1].debit == 6000);
    v = FindApVoucher(db.code, ApKind::Invoice, n);
    CHECK(v->payable == 56000);
    CHECK(v->whAmount == -500);
    CHECK_FALSE(DeleteExpenseLine(db.code, ApKind::Invoice, n, 2, TaxMode::F, acc));   // the VAT line isn't an expense line

    // Changing the rates on the header recomputes every line with them.
    h.vatRate = 0;
    h.whRate = 0;
    e2.line = 1;
    CHECK(SaveExpenseLine(db.code, h, e2, TaxMode::F, acc) == n);
    lines = LoadApLines(db.code, ApKind::Invoice, n);
    REQUIRE(lines.size() == 2);
    CHECK(lines[1].credit == 56000);

    // Posted, transferred and cancelled vouchers aren't changed.
    ApVoucher posted = *FindApVoucher(db.code, ApKind::Invoice, 3);
    CHECK(SaveExpenseLine(db.code, posted, e, TaxMode::F, acc) == 0);
    CHECK_FALSE(DeleteExpenseLine(db.code, ApKind::Invoice, 3, 1, TaxMode::F, acc));
    CHECK(LoadApLines(db.code, ApKind::Invoice, 3).size() == 3);
}

TEST_CASE("DeleteApVoucher removes only that open voucher (D5)", "[ap]")
{
    FixtureDb db;
    EnsureApTables(db.code);
    CHECK(DeleteApVoucher(db.code, ApKind::Invoice, 2));
    CHECK_FALSE(FindApVoucher(db.code, ApKind::Invoice, 2));
    CHECK(LoadApLines(db.code, ApKind::Invoice, 2).empty());
    CHECK(LoadApLines(db.code, ApKind::Invoice, 1).size() == 4);   // the others stay, with their roles
    CHECK(LoadApLines(db.code, ApKind::Invoice, 1)[0].gross == 112000);
    CHECK(LoadApLines(db.code, ApKind::Cash, 1)[0].gross == 33600);
    CHECK_FALSE(DeleteApVoucher(db.code, ApKind::Invoice, 3));   // posted
    CHECK_FALSE(DeleteApVoucher(db.code, ApKind::Invoice, 4));   // cancelled
    CHECK_FALSE(DeleteApVoucher(db.code, ApKind::Invoice, 99));
    CHECK(LoadApVouchers(db.code, ApKind::Invoice).size() == 5);
}

namespace {

std::vector<JournalRow> journalOf(Code4& code, const char* type, long batchno)
{
    std::vector<JournalRow> out;
    for (const JournalRow& r : LoadJournal(code))
        if (r.type == type && r.batchno == batchno) out.push_back(r);
    return out;
}

double glBalance(Code4& code, const char* account, const char* dept = "")
{
    cb4sqlite::Query q(code, "SELECT curr_bal FROM glaccts WHERE trim(account) = ?1 AND coalesce(trim(dept), '') = ?2");
    q.bindText(1, account);
    q.bindText(2, dept);
    REQUIRE(q.next());
    return q.number(0);
}

} // namespace

TEST_CASE("PostApVouchers: open items and the journal (S5-M3)", "[ap]")
{
    FixtureDb db;
    EnsureApTables(db.code);

    // One voucher: invoice 1 (1,120.00, F mode).
    ApPostResult r = PostApVouchers(db.code, ApKind::Invoice, 1);
    REQUIRE(r.posted.size() == 1);
    CHECK(r.refused.empty());
    CHECK(FindApVoucher(db.code, ApKind::Invoice, 1)->mode == ap_mode::kPosted);
    const auto items = LoadOpenItems(db.code);
    REQUIRE(items.size() == 3);
    const OpenItem& item = items[0];   // in voucher order: 1, 3, 6
    CHECK(item.batchno == 1);
    CHECK(item.paymentCtl == ap_payment::kOpen);   // D11
    CHECK(item.amount == 112000);
    CHECK(item.wh == -1000);
    CHECK(item.vat == 12000);
    CHECK(item.ap.account == "221002");
    CHECK(item.bank.empty());
    CHECK(item.dueDate == "20261104");
    const auto rows = journalOf(db.code, "AP", 1);
    REQUIRE(rows.size() == 4);
    std::int64_t dr = 0, cr = 0;
    for (const JournalRow& j : rows) {
        CHECK(j.mode == "1");
        CHECK(j.period == 7);
        CHECK(j.date == "20261005");
        dr += j.debit;
        cr += j.credit;
    }
    CHECK(dr == 112000);
    CHECK(cr == 112000);

    // The rest: invoice 2 posts; the posted, cancelled and transferred ones are left alone.
    r = PostApVouchers(db.code, ApKind::Invoice);
    CHECK(r.posted == std::vector<long>{2});
    CHECK(r.refused.empty());
    CHECK(LoadOpenItems(db.code).size() == 4);
    CHECK(FindApVoucher(db.code, ApKind::Invoice, 4)->mode == ap_mode::kCancelled);

    // A cash voucher: TYPE "CA" with its cheque, no open item (D10).
    r = PostApVouchers(db.code, ApKind::Cash);
    CHECK(r.posted == std::vector<long>{1});
    CHECK(LoadOpenItems(db.code).size() == 4);
    const auto cash = journalOf(db.code, "CA", 1);
    REQUIRE(cash.size() == 4);
    CHECK(cash[0].cheque == "000123");
    CHECK(FindApVoucher(db.code, ApKind::Cash, 1)->mode == ap_mode::kPosted);
}

TEST_CASE("PostApVouchers refuses what the GL couldn't post", "[ap]")
{
    FixtureDb db;
    EnsureApTables(db.code);
    // Out of balance by a cent.
    ApVoucher v = *FindApVoucher(db.code, ApKind::Invoice, 1);
    auto lines = LoadApLines(db.code, ApKind::Invoice, 1);
    lines[3].credit -= 1;
    SaveApVoucher(db.code, v, lines);
    // An account not in the chart.
    ApVoucher w = *FindApVoucher(db.code, ApKind::Invoice, 2);
    auto wl = LoadApLines(db.code, ApKind::Invoice, 2);
    wl[0].account = {"999999", ""};
    SaveApVoucher(db.code, w, wl);
    const ApPostResult r = PostApVouchers(db.code, ApKind::Invoice);
    CHECK(r.posted.empty());
    REQUIRE(r.refused.size() == 2);
    CHECK(r.refused[0].number == 1);
    CHECK(r.refused[0].reason == ApPostRefusal::OutOfBalance);
    CHECK(r.refused[1].reason == ApPostRefusal::UnknownAccount);
    CHECK(r.refused[1].account.account == "999999");
    CHECK(FindApVoucher(db.code, ApKind::Invoice, 1)->mode == ap_mode::kOpen);
    CHECK(LoadOpenItems(db.code).size() == 2);   // nothing written
    // No lines.
    ApVoucher e;
    e.vendor = "V003";
    e.invoiceNo = "EMPTY";
    e.mode = ap_mode::kOpen;
    const long n = SaveApVoucher(db.code, e, {});
    CHECK(PostApVouchers(db.code, ApKind::Invoice, n).refused[0].reason == ApPostRefusal::NoLines);
}

TEST_CASE("CancelApVoucher and RecoverApVoucher (D3, D7)", "[ap]")
{
    FixtureDb db;
    EnsureApTables(db.code);
    CHECK(CancelApVoucher(db.code, ApKind::Invoice, 2) == ApCancelResult::Cancelled);
    CHECK(FindApVoucher(db.code, ApKind::Invoice, 2)->cancelled());
    CHECK(LoadApLines(db.code, ApKind::Invoice, 2).size() == 5);   // kept for the record
    CHECK(CancelApVoucher(db.code, ApKind::Invoice, 3) == ApCancelResult::NotOpen);
    CHECK(CancelApVoucher(db.code, ApKind::Invoice, 99) == ApCancelResult::NotFound);

    // Invoice 3: posted, its journal waiting, its open item "O": recovered.
    CHECK(RecoverApVoucher(db.code, ApKind::Invoice, 3) == ApRecoverResult::Recovered);
    CHECK(FindApVoucher(db.code, ApKind::Invoice, 3)->mode == ap_mode::kOpen);
    CHECK(journalOf(db.code, "AP", 3).empty());
    const auto items = LoadOpenItems(db.code);
    REQUIRE(items.size() == 1);
    CHECK(items[0].batchno == 6);
    CHECK(journalOf(db.code, "AP", 6).size() == 4);   // invoice 6's rows untouched
    // Posted again, it's as before.
    CHECK(PostApVouchers(db.code, ApKind::Invoice, 3).posted.size() == 1);
    CHECK(journalOf(db.code, "AP", 3).size() == 3);
    CHECK(LoadOpenItems(db.code).size() == 2);

    // Invoice 6 is paid (cheque 5001, printed): refused.
    CHECK(RecoverApVoucher(db.code, ApKind::Invoice, 6) == ApRecoverResult::Paid);
    CHECK(RecoverApVoucher(db.code, ApKind::Invoice, 1) == ApRecoverResult::NotPosted);   // open
}

TEST_CASE("TransferAp: one GL voucher per period, and its undo (D2, D10)", "[ap]")
{
    FixtureDb db;
    EnsureApTables(db.code);
    PostApVouchers(db.code, ApKind::Cash);   // cash voucher 1: 336.00, period 7

    // A row in closed period 5 is left waiting.
    {
        Data4 j(&db.code, "ap_jsum");
        j.appendStart();
        j.blank();
        Field4(j, "account").assign("751000");
        Field4(j, "period").assign(" 5");
        Field4(j, "mode").assign("1");
        Field4(j, "type").assign("AP");
        Field4(j, "batchno").assignLong(77);
        Field4(j, "debit").assign("10.00");
        Field4(j, "credit").assign("0.00");
        j.append();
        j.close();
    }

    const ApTransferResult r = TransferAp(db.code, 99);
    CHECK(r.closedPeriods == std::vector<int>{5});
    REQUIRE(r.vouchers.size() == 1);
    const ApTransferVoucher& t = r.vouchers[0];
    CHECK(t.period == 7);
    CHECK(t.glVoucher == 104);   // after the fixture's 100-103
    CHECK(t.rows == 11);         // invoice 3 (3), invoice 6 and its payment (4), cash 1 (4)
    // Netted: 731000-20 Dr 5,000; 751000 Dr 1,200; 501000 Dr 300; 148000 Dr 36;
    // 260004 Cr 100 + 3; 221002 Cr 4,900 (the payment cancels invoice 6); 102101 Cr 1,200 + 333.
    CHECK(t.debit == 653600);
    CHECK(t.credit == 653600);
    CHECK(t.lines == 7);

    VoucherHeaders h(db.code);
    VoucherLines l(db.code);
    VoucherHeader header;
    std::vector<VoucherLine> lines;
    REQUIRE(LoadVoucher(h, l, 104, header, lines) == LoadResult::Loaded);
    CHECK(header.mode == voucher_mode::kOpen);
    CHECK(header.description == "Accounts Payable GL Update");
    CHECK(header.date == "20261031");
    CHECK(header.period == 7);
    double ap = 0;
    for (const VoucherLine& v : lines)
        if (v.account == "221002") ap = v.credit - v.debit;
    CHECK(ap == Approx(4900.0));

    for (const JournalRow& j : LoadJournal(db.code)) CHECK(j.mode == (j.period == 5 ? "1" : "2"));
    CHECK(FindApVoucher(db.code, ApKind::Invoice, 3)->mode == ap_mode::kTransferred);
    CHECK(FindApVoucher(db.code, ApKind::Invoice, 6)->mode == ap_mode::kTransferred);
    CHECK(FindApVoucher(db.code, ApKind::Cash, 1)->mode == ap_mode::kTransferred);   // D10
    CHECK(RecoverApVoucher(db.code, ApKind::Invoice, 3) == ApRecoverResult::Transferred);

    // Nothing more to transfer in period 7.
    CHECK(TransferAp(db.code, 99).vouchers.empty());

    // Cancelling an ordinary voucher leaves AP alone; cancelling 104 puts the rows back.
    CHECK(CancelVoucher(db.code, 101) == CancelResult::Cancelled);
    CHECK(FindApVoucher(db.code, ApKind::Invoice, 3)->mode == ap_mode::kTransferred);
    CHECK(CancelVoucher(db.code, 104) == CancelResult::Cancelled);
    for (const JournalRow& j : LoadJournal(db.code)) CHECK(j.mode == "1");
    CHECK(FindApVoucher(db.code, ApKind::Invoice, 3)->mode == ap_mode::kPosted);
    CHECK(FindApVoucher(db.code, ApKind::Cash, 1)->mode == ap_mode::kPosted);
    CHECK(UndoApTransfer(db.code, 104) == 0);   // the links went with it
}

TEST_CASE("The transfer's voucher posts with Spec #2", "[ap]")
{
    FixtureDb db;
    EnsureApTables(db.code);
    const double ap0 = glBalance(db.code, "221002"), exp0 = glBalance(db.code, "731000", "20");
    const ApTransferResult r = TransferAp(db.code, 99);
    REQUIRE(r.vouchers.size() == 1);
    const PostResult p = PostVoucher(db.code, r.vouchers[0].glVoucher);
    REQUIRE(p.posted.size() == 1);
    // 221002 carries the unpaid invoice 3 (4,900.00 credit); invoice 6's 1,200.00 came and went.
    CHECK(glBalance(db.code, "221002") == Approx(ap0 - 4900.0));
    CHECK(glBalance(db.code, "731000", "20") == Approx(exp0 + 5000.0));
    CHECK(CancelVoucher(db.code, r.vouchers[0].glVoucher) == CancelResult::Posted);   // posted: Cancel refuses
}

TEST_CASE("Cash vouchers: entry with a bank, post, transfer, cancel (S5-M4, D10)", "[ap]")
{
    FixtureDb db;
    EnsureApTables(db.code);
    const VoucherAccounts acc{{"148000", ""}, {"260004", ""}, {"102101", ""}, "", "", ""};

    // V001 paid 560.00 at the counter by cheque 000124 from the checking account.
    ApVoucher h;
    h.kind = ApKind::Cash;
    h.vendor = "V001";
    h.invoiceNo = "INV-1010-C";   // ten characters: cash vouchers' INVOICENO is C10
    h.reference = "Supplies";
    h.invoiceDate = h.date = h.dueDate = "20261021";
    h.period = 7;
    h.vatRate = 12;
    h.whRate = 1;
    h.account = {"102101", ""};
    h.vat = {"148000", ""};
    h.cheque = "000124";
    ExpenseEntry e;
    e.account = {"501000", ""};
    e.gross = 56000;
    const long n = SaveExpenseLine(db.code, h, e, TaxMode::F, acc);
    CHECK(n == 2);   // after the fixture's cash voucher 1
    const auto v = FindApVoucher(db.code, ApKind::Cash, n);
    REQUIRE(v);
    CHECK(v->invoiceNo == "INV-1010-C");
    CHECK(v->cheque == "000124");
    CHECK(v->vat.account == "148000");
    CHECK(v->account.account == "102101");
    auto lines = LoadApLines(db.code, ApKind::Cash, n);
    REQUIRE(lines.size() == 4);
    CHECK(lines[3].role == ApLineRole::Credit);
    CHECK(lines[3].account.account == "102101");   // the bank is credited, F: 560.00 - W/H 5.00
    CHECK(lines[3].credit == 55500);
    CHECK(lines[3].descript == "CHECKING ACCOUNT");
    // The same invoice number for V001 is refused on an invoice voucher too (D6).
    CHECK(DuplicateInvoice(db.code, "V001", "INV-1010-C")->kind == ApKind::Cash);

    // Post: TYPE "CA" rows with the cheque; no open item.
    const auto openBefore = LoadOpenItems(db.code).size();
    CHECK(PostApVouchers(db.code, ApKind::Cash, n).posted == std::vector<long>{n});
    CHECK(LoadOpenItems(db.code).size() == openBefore);
    const auto rows = journalOf(db.code, "CA", n);
    REQUIRE(rows.size() == 4);
    for (const JournalRow& r : rows) CHECK(r.cheque == "000124");

    // The transfer carries it and marks it "2" (D10).
    TransferAp(db.code, 99);
    CHECK(FindApVoucher(db.code, ApKind::Cash, n)->mode == ap_mode::kTransferred);

    // Cancel: the fixture's open cash voucher 1.
    CHECK(CancelApVoucher(db.code, ApKind::Cash, 1) == ApCancelResult::Cancelled);
    CHECK(FindApVoucher(db.code, ApKind::Cash, 1)->cancelled());
    CHECK(CancelApVoucher(db.code, ApKind::Cash, n) == ApCancelResult::NotOpen);
}

TEST_CASE("Create Cheques: what's due, Hold / Force, one cheque per vendor (S5-M5, D8, D9)", "[ap]")
{
    FixtureDb db;
    EnsureApTables(db.code);
    CHECK(NextChequeNumber(db.code) == 5002);   // after the fixture's 5001
    CHECK(OpenPeriodOf(db.code, "20261020") == 7);
    CHECK(OpenPeriodOf(db.code, "20260915") == 0);   // period 6 is closed

    // Due by 2026-10-31: invoice 3 (V002, 5,000.00 less 100.00 W/H). Invoice 6 is paid.
    auto due = PaymentItems(db.code, "20261031");
    REQUIRE(due.size() == 1);
    CHECK(due[0].item.batchno == 3);
    CHECK(due[0].owed == 490000);
    CHECK(due[0].vendorName == "SANTOS");
    CHECK(PaymentItems(db.code, "20261031", true).size() == 2);   // View All: the paid one too
    CHECK(PaymentItems(db.code, "20261031", true)[1].owed == 0);

    // Post invoices 1 (due 11-04, owes 1,110.00) and 2 (due 11-11, owes 777.00).
    PostApVouchers(db.code, ApKind::Invoice);
    CHECK(PaymentItems(db.code, "20261105").size() == 2);   // 1 and 3
    // Force invoice 2 (not yet due): listed. Hold invoice 3: listed, not paid.
    CHECK(SetPaymentControl(db.code, 2, ap_payment::kForced));
    CHECK(SetPaymentControl(db.code, 3, ap_payment::kHeld));
    CHECK_FALSE(SetPaymentControl(db.code, 6, ap_payment::kOpen));   // in a payment (4010)
    CHECK_FALSE(SetPaymentControl(db.code, 1, "P"));
    due = PaymentItems(db.code, "20261105");
    REQUIRE(due.size() == 3);

    const ApAccount bank{"102101", ""};
    // Refusals first: nothing is written.
    CHECK(CreateCheques(db.code, "20261105", "20261020", {"999999", ""}, 5002).status == CreateChequesStatus::BadBank);
    CHECK(CreateCheques(db.code, "20261105", "20260915", bank, 5002).status == CreateChequesStatus::ClosedPeriod);
    const auto clash = CreateCheques(db.code, "20261105", "20261020", bank, 5001);
    CHECK(clash.status == CreateChequesStatus::BadNumber);
    CHECK(clash.clash == 5001);
    CHECK(CreateCheques(db.code, "20261105", "20261020", bank, 0).status == CreateChequesStatus::BadNumber);
    CHECK(LoadCheques(db.code).size() == 1);

    // Create: V001's invoices 1 (due) and 2 (forced) on one cheque, 5002; invoice 3 is held.
    const auto r = CreateCheques(db.code, "20261105", "20261020", bank, 5002);
    REQUIRE(r.status == CreateChequesStatus::Created);
    CHECK(r.period == 7);
    REQUIRE(r.cheques.size() == 1);
    CHECK(r.cheques[0].number == 5002);
    CHECK(r.cheques[0].vendor == "V001");
    CHECK(r.cheques[0].amount == 111000 + 77700);
    CHECK(r.cheques[0].items == 2);

    const auto cheques = LoadCheques(db.code);
    REQUIRE(cheques.size() == 2);
    CHECK(cheques[1].number == 5002);
    CHECK(cheques[1].amount == 188700);
    CHECK(cheques[1].date == "20261020");
    CHECK(cheques[1].bank.account == "102101");
    CHECK_FALSE(cheques[1].cleared);
    for (const OpenItem& i : LoadOpenItems(db.code)) {
        if (i.batchno == 1 || i.batchno == 2) {
            CHECK(i.paymentCtl == ap_payment::kPaid);
            CHECK(i.cheque == 5002);
            CHECK(i.bank.account == "102101");
            CHECK(i.datePaid == "20261020");
            CHECK(i.paid == (i.batchno == 1 ? 111000 : 77700));
        }
        if (i.batchno == 3) CHECK(i.paymentCtl == ap_payment::kHeld);
    }
    // The payment's journal: Dr AP / Cr the bank per invoice, dated the payment, period 7, with the cheque.
    std::int64_t apDr = 0, bankCr = 0;
    int rows = 0;
    for (const JournalRow& j : LoadJournal(db.code)) {
        if (j.cheque != "5002") continue;
        ++rows;
        CHECK(j.date == "20261020");
        CHECK(j.period == 7);
        CHECK(j.type == "AP");
        CHECK(j.mode == "1");
        if (j.account.account == "221002") apDr += j.debit;
        if (j.account.account == "102101") bankCr += j.credit;
    }
    CHECK(rows == 4);
    CHECK(apDr == 188700);
    CHECK(bankCr == 188700);

    // Paid: Recover refuses it; the next number moves on; nothing more is due.
    CHECK(RecoverApVoucher(db.code, ApKind::Invoice, 1) == ApRecoverResult::Paid);
    CHECK(NextChequeNumber(db.code) == 5003);
    CHECK(CreateCheques(db.code, "20261105", "20261020", bank, 5003).status == CreateChequesStatus::NothingDue);

    // Released, invoice 3 goes on the next run, with its own cheque.
    CHECK(SetPaymentControl(db.code, 3, ap_payment::kOpen));
    const auto r2 = CreateCheques(db.code, "20261105", "20261020", bank, 5003);
    REQUIRE(r2.cheques.size() == 1);
    CHECK(r2.cheques[0].vendor == "V002");
    CHECK(r2.cheques[0].amount == 490000);
}

TEST_CASE("AmountInWords (the cheque)", "[ap]")
{
    CHECK(AmountInWords(188700) == "One thousand eight hundred eighty-seven and 00/100");
    CHECK(AmountInWords(0) == "Zero and 00/100");
    CHECK(AmountInWords(2105) == "Twenty-one and 05/100");
    CHECK(AmountInWords(1234567) == "Twelve thousand three hundred forty-five and 67/100");
    CHECK(AmountInWords(100000000) == "One million and 00/100");
    CHECK(AmountInWords(1100000099) == "Eleven million and 99/100");
    CHECK(AmountInWords(11500) == "One hundred fifteen and 00/100");
}

TEST_CASE("Cheques: confirm, renumber, clear, void, purge (S5-M6, D8, D9)", "[ap]")
{
    FixtureDb db;
    EnsureApTables(db.code);
    // The fixture's cheque 5001 (V003, 1,200.00) is printed, not cleared.
    CHECK(ChequeState(db.code, 5001) == ap_payment::kPrinted);
    CHECK(LoadChequesIn(db.code, ap_payment::kPaid).empty());
    REQUIRE(LoadChequesIn(db.code, "").size() == 1);

    // Make cheque 5002 for V001's invoices 1 and 2.
    PostApVouchers(db.code, ApKind::Invoice);
    SetPaymentControl(db.code, 3, ap_payment::kHeld);
    const ApAccount bank{"102101", ""};
    REQUIRE(CreateCheques(db.code, "20261111", "20261020", bank, 5002).cheques.size() == 1);
    CHECK(ChequeState(db.code, 5002) == ap_payment::kPaid);
    auto toPrint = LoadChequesIn(db.code, ap_payment::kPaid);
    REQUIRE(toPrint.size() == 1);
    CHECK(toPrint[0].payee.name == "Office Supplies Co.");
    CHECK(toPrint[0].lines.size() == 2);
    CHECK_FALSE(LoadCheques(db.code)[1].cleared);

    // The cheque report: one slip, the stub, the total.
    const rpt::Report report = ApCheques(db.code, ReportContext{});
    REQUIRE(!report.rows.empty());
    CHECK(report.rows[0].kind == rpt::RowKind::Slip);
    REQUIRE(report.rows.size() > 1);
    CHECK(report.rows[1].kind == rpt::RowKind::Note);
    CHECK(report.rows[1].text == "The sum of One thousand eight hundred eighty-seven and 00/100");

    // Renumber the spoiled 5002: not to a number in use; then to 5010, everywhere.
    CHECK(RenumberCheque(db.code, 5002, 5001) == RenumberResult::NumberUsed);
    CHECK(RenumberCheque(db.code, 5001, 5020) == RenumberResult::NotMade);   // printed already
    CHECK(RenumberCheque(db.code, 5099, 5020) == RenumberResult::NotFound);
    CHECK(RenumberCheque(db.code, 5002, 5010) == RenumberResult::Renumbered);
    CHECK(ChequeState(db.code, 5010) == ap_payment::kPaid);
    CHECK(ChequeState(db.code, 5002).empty());
    int rows5010 = 0;
    for (const JournalRow& j : LoadJournal(db.code)) {
        CHECK(j.cheque != "5002");
        if (j.cheque == "5010") ++rows5010;
    }
    CHECK(rows5010 == 4);

    // Clear needs it printed; confirm, then clear.
    CHECK(ClearCheque(db.code, 5010, "20261030") == ClearResult::NotPrinted);
    CHECK(ConfirmPrinted(db.code, 5000, 5099) == 1);
    CHECK(ChequeState(db.code, 5010) == ap_payment::kPrinted);
    CHECK(ClearCheque(db.code, 5010, "20261030") == ClearResult::Cleared);
    CHECK(ChequeState(db.code, 5010) == ap_payment::kCleared);
    CHECK(ClearCheque(db.code, 5010, "20261030") == ClearResult::AlreadyCleared);
    for (const Cheque& c : LoadCheques(db.code))
        if (c.number == 5010) {
            CHECK(c.cleared);
            CHECK(c.reference == "Cleared 20261030");
        }
    CHECK(VoidCheque(db.code, 5010, "20261030").result == VoidResult::Cleared);

    // Void 5001 before the transfer: its waiting payment rows go, invoice 6 is open again.
    const VoidOutcome v = VoidCheque(db.code, 5001, "20261030");
    CHECK(v.result == VoidResult::Voided);
    CHECK(v.itemsReopened == 1);
    CHECK(v.rowsRemoved == 2);
    CHECK(v.rowsReversed == 0);
    CHECK(ChequeState(db.code, 5001).empty());
    bool open6 = false;
    for (const PaymentItem& p : PaymentItems(db.code, "20261231"))
        if (p.item.batchno == 6) open6 = p.owed == 120000;
    CHECK(open6);

    // Pay invoice 6 again (5011), transfer, then void: the rows are reversed, dated the void.
    SetPaymentControl(db.code, 3, ap_payment::kHeld);
    REQUIRE(CreateCheques(db.code, "20261231", "20261020", bank, 5011).cheques.size() == 1);
    TransferAp(db.code, 99);
    CHECK(VoidCheque(db.code, 5011, "20260915").result == VoidResult::ClosedPeriod);   // period 6 is closed
    const VoidOutcome v2 = VoidCheque(db.code, 5011, "20261030");
    CHECK(v2.result == VoidResult::Voided);
    CHECK(v2.rowsRemoved == 0);
    CHECK(v2.rowsReversed == 2);
    std::int64_t bankDr = 0, apCr = 0;
    for (const JournalRow& j : LoadJournal(db.code))
        if (j.cheque == "5011" && j.mode == "1") {
            CHECK(j.date == "20261030");
            CHECK(j.period == 7);
            if (j.account.account == "102101") bankDr += j.debit;
            if (j.account.account == "221002") apCr += j.credit;
        }
    CHECK(bankDr == 120000);
    CHECK(apCr == 120000);

    // Purge: only the cleared items paid before the date (5010's two), and their cheque.
    CHECK(PurgeCandidates(db.code, "20261020").empty());   // paid on the 20th, not before
    const auto candidates = PurgeCandidates(db.code, "20261231");
    REQUIRE(candidates.size() == 2);
    const PurgeOutcome o = PurgePaid(db.code, "20261231");
    CHECK(o.items == 2);
    CHECK(o.cheques == 1);
    for (const Cheque& c : LoadCheques(db.code)) CHECK(c.number != 5010);
    for (const OpenItem& i : LoadOpenItems(db.code)) CHECK(i.paymentCtl != ap_payment::kCleared);
    CHECK(LoadOpenItems(db.code).size() == 2);   // invoices 3 (held) and 6 (open again)
}
