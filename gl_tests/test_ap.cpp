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

#include <string>
#include <vector>

using namespace gltest;

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
