//
// Ap.h - the Accounts Payable tables, records and invoice arithmetic (spec #5
// 2.1, 4.1, D3, D4, D6; plan S5-M0). Portable: no Win32, so gl_tests checks it.
//
//   ap_vend    vendors (dialog 1041)
//   ap_btch    invoice voucher headers (dialog 1042), and ap_btchd their lines
//   ap_cash    cash voucher headers (dialog 1044), and ap_cashd their lines
//   ap_dist    the open-item ledger: one row per posted invoice (PAYMENTCTL)
//   ap_reqst   the cheques
//   ap_jsum    AP's journal, staged for the General Ledger
//   ap_linex   the port's own: each line's role (expense, VAT, W/H, credit)
//              and an expense line's amount as typed (spec #5 4.1). The
//              lines store the net expense, and the gross can't be recovered
//              from it: VAT rounding maps two grosses to one net (at 12 %,
//              11.24 and 11.25 both give 10.04). The role can't be
//              told from the account either: in T mode W/H is a debit too.
//
// The fields are read by name from the deployed layouts (spec #5 2.1,
// "Layouts worth knowing"), not from include/db_ap*.h. Amounts are in cents
// (Spec #2 D12).
//
#pragma once

#include "cb4sqlite.h"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

// ---- modes (D3, D8) ----------------------------------------------------------

// ap_btch / ap_cash MODE. The original's "2" meant both transferred and
// cancelled; EnsureApTables moves its cancelled vouchers to "X".
namespace ap_mode {
inline constexpr char kOpen[] = "1";
inline constexpr char kPosted[] = "0";
inline constexpr char kTransferred[] = "2";
inline constexpr char kCancelled[] = "X";
} // namespace ap_mode

// ap_dist PAYMENTCTL (D8): H held -> O open / F forced -> P cheque made ->
// V printed -> C cleared.
namespace ap_payment {
inline constexpr char kHeld[] = "H";
inline constexpr char kOpen[] = "O";
inline constexpr char kForced[] = "F";
inline constexpr char kPaid[] = "P";
inline constexpr char kPrinted[] = "V";
inline constexpr char kCleared[] = "C";
} // namespace ap_payment

// ---- tables -------------------------------------------------------------------

// Creates the AP tables (the deployed layouts and tags) when the company
// lacks them, and ap_linex when missing; then migrates D3: an invoice or cash
// voucher with MODE "2" whose reference is "Cancelled" (string 1042, what the
// original's Cancel wrote) becomes "X". Returns how many vouchers it
// migrated. Run by every AP screen, the transfer and the reports.
int EnsureApTables(cb4sqlite::Code4& code);

// ---- accounts (GL_SETAT) -----------------------------------------------------------

struct ApAccount {
    std::string account, dept;
    bool empty() const { return account.empty(); }
    bool operator==(const ApAccount& o) const { return account == o.account && dept == o.dept; }
};

struct ApBank {
    std::string shortName;   // SHORT
    std::string name;        // NAME
    ApAccount account;
};

struct ApAccounts {
    ApAccount ap;            // AP: the default AP control account
    ApAccount vatInput;      // VATR: prepaid (input) VAT, the vendors' default (D12)
    ApAccount vatPayable;    // VATP
    ApAccount whCompany;     // WTXC: W/H on a company vendor
    ApAccount whIndividual;  // WTXI: W/H on an individual
    std::vector<ApBank> banks;   // TYPE "CA", in SHORT order
};

// The named accounts; any GL_SETAT row missing leaves its account empty.
ApAccounts LoadApAccounts(cb4sqlite::Code4& code);

// ---- vendors ---------------------------------------------------------------------

struct Vendor {
    std::string number;        // VENDORNO
    std::string shortName;     // VDSHORT
    std::string name;          // VDNAME
    std::string address1, address2, address3, postal, phone, fax, contact;
    std::string descript;      // DESCRIPT
    std::string vin;           // VIN
    std::string type;          // TYPE: "C" company, "I" individual
    std::string medrep;        // MEDREP
    std::string status;        // STATUS
    ApAccount ap;              // ACCOUNT / DEPART: the vendor's AP control account
    ApAccount vat;             // VAT_ACCT / VAT_DEPT: its input-VAT account
    int terms = 0;             // TERMS, days
    int vatRate = 0;           // PERCENTAGE, whole %, 0 none
    int whRate = 0;            // REBATE, whole %, 0 none
    int discountDays = 0;      // DIS_DAYS
    int discountPercent = 0;   // DIS_PERCT

    bool individual() const { return type == "I"; }
};

std::vector<Vendor> LoadVendors(cb4sqlite::Code4& code);   // in number order, deleted rows skipped
std::optional<Vendor> FindVendor(cb4sqlite::Code4& code, const std::string& number);

// Adds the vendor, or replaces the one with its number. True if it was added.
// AP_ACCT / AP_DEPT, VAT, FIVE and THREE are never used (spec #5 2.1); a new
// vendor gets them blank, and a replaced one keeps them.
bool SaveVendor(cb4sqlite::Code4& code, const Vendor& v);

// ---- vouchers ---------------------------------------------------------------------

enum class ApKind { Invoice, Cash };   // ap_btch / ap_cash

struct ApVoucher {
    ApKind kind = ApKind::Invoice;
    long number = 0;           // BATCHNO
    std::string mode;          // ap_mode
    std::string vendor;        // VENDORNO
    std::string vendorName;    // VDNAME
    std::string invoiceNo;     // INVOICENO (8 characters on invoices, 10 on cash vouchers)
    std::string orderNo;       // ORDERNO
    std::string cheque;        // CHEQUENO: cash vouchers only
    std::string reference;     // REFERENCE
    std::string descript;      // DESCRIPT
    std::string date;          // DATE, yyyymmdd
    std::string invoiceDate;   // INVDATE
    std::string dueDate;       // DUEDATE
    int period = 0;
    int terms = 0;
    int vatRate = 0;           // VATPERCENT
    int whRate = 0;            // REBATE
    ApAccount account;         // ACCOUNT / DEPART: the AP account (invoices) or the bank (cash)
    ApAccount vat;             // VATACCT / VATDEPT: cash vouchers only (invoices have no such field)
    std::int64_t payable = 0;  // PAYABLE: the gross, the sum of the typed amounts
    std::int64_t vatAmount = 0;   // VATAMT
    std::int64_t whAmount = 0;    // REBATEAMT, signed as stored: F mode negative (a credit), T positive
    int lines = 0;             // ENTRYNO

    bool cancelled() const { return mode == ap_mode::kCancelled; }
};

enum class ApLineRole { Expense, Vat, WithHolding, Credit };   // Credit: the AP or bank line

struct ApLine {
    int line = 0;              // ENTRYNO
    ApLineRole role = ApLineRole::Expense;   // ap_linex ROLE
    std::string descript, medrep, drcode, product;
    ApAccount account;
    std::int64_t debit = 0, credit = 0;
    std::int64_t gross = 0;    // the amount typed: expense lines (ap_linex GROSS); 0 for the others
};

// The vouchers of a kind, in number order, deleted rows skipped.
std::vector<ApVoucher> LoadApVouchers(cb4sqlite::Code4& code, ApKind kind);
std::optional<ApVoucher> FindApVoucher(cb4sqlite::Code4& code, ApKind kind, long number);

// The voucher's lines in line order, with their roles and gross amounts from
// ap_linex. A deployed voucher's lines (written before ap_linex) are
// classified: a credit on the voucher's account is the Credit line, another
// credit the W/H line; a debit equal to VATAMT the VAT line and, when
// REBATEAMT is positive (T mode), one equal to it the W/H line; the other
// debits are expense lines, their gross GrossFromExpense in the mode
// REBATEAMT's sign shows.
std::vector<ApLine> LoadApLines(cb4sqlite::Code4& code, ApKind kind, long number);

// The number a new voucher of that kind gets: the highest + 1, or 1.
long NextApVoucherNumber(cb4sqlite::Code4& code, ApKind kind);

// Writes the header (a new voucher when `v.number` is 0 or doesn't exist:
// numbered NextApVoucherNumber) and replaces all its lines with `lines`,
// renumbered 1..n, with their ap_linex rows. ENTRYNO becomes
// the line count. One transaction. Returns the voucher's number.
long SaveApVoucher(cb4sqlite::Code4& code, const ApVoucher& v, const std::vector<ApLine>& lines);

// D6: another voucher, invoice or cash, not cancelled, with the same vendor
// and invoice number (trimmed, case as typed). `except` is the voucher being
// edited. Returns that voucher, or nullopt.
struct ApVoucherRef {
    ApKind kind = ApKind::Invoice;
    long number = 0;
};
std::optional<ApVoucherRef> DuplicateInvoice(cb4sqlite::Code4& code, const std::string& vendor,
                                             const std::string& invoiceNo, std::optional<ApVoucherRef> except = {});

// ---- the arithmetic (spec #5 4.1, D4) ----------------------------------------------

// GL.INI [WITHHOLDING] TAX. F (the default, D4/Q2): VAT is included in the
// amount and W/H withheld from the payment, on the net. T: the amount is
// grossed up, W/H debited.
enum class TaxMode { F, T };
TaxMode ParseTaxMode(const std::string& iniValue);   // "T" (any case, trimmed) -> T; else F

struct AmountSplit {
    std::int64_t expense = 0, vat = 0, wh = 0;   // all >= 0 for a positive amount
};

// One typed amount, rates in whole percent (0: none):
//   F: VAT = round(a x v / (100 + v)); expense = a - VAT; W/H = round(expense x w / 100)
//   T: expense = round(a x 100 / (100 + v + w)); VAT = round(expense x v / 100); W/H = a - expense - VAT
// Rounding is half away from zero.
AmountSplit SplitAmount(std::int64_t amount, int vatRate, int whRate, TaxMode mode);

// The inverse for a deployed line without ap_linex: an amount whose split
// gives `expense` (F: expense + round(expense x v / 100); T: plus the W/H too).
std::int64_t GrossFromExpense(std::int64_t expense, int vatRate, int whRate, TaxMode mode);

struct VoucherAccounts {
    ApAccount vat;      // the VAT line's account
    ApAccount wh;       // WTXC or WTXI
    ApAccount credit;   // the AP account (invoices) or the bank (cash)
    std::string vatText, whText, creditText;   // the computed lines' descriptions (the accounts' names)
};

struct ComputedVoucher {
    std::vector<ApLine> lines;   // the expense lines (debit = their expense), then VAT, W/H and the credit line
    std::int64_t payable = 0;    // the sum of the typed amounts
    std::int64_t vat = 0, wh = 0;   // totals, >= 0
    std::int64_t credit = 0;        // the credit line: F payable - W/H; T payable
    std::int64_t debits = 0, credits = 0;   // the lines' totals: always equal
};

// D4: the voucher's lines from its expense lines (`gross` set; role, debit
// and credit are ignored). Each line is split by SplitAmount; then one VAT
// line (Dr), one W/H line (F: Cr; T: Dr) and one credit line (Cr), each left
// out when zero, with the descriptions `accounts` gives.
ComputedVoucher ComputeVoucher(const std::vector<ApLine>& expenseLines, int vatRate, int whRate, TaxMode mode,
                               const VoucherAccounts& accounts);

// ---- the ledger rows (read-only until their milestones) ------------------------------------

struct OpenItem {   // ap_dist
    long batchno = 0;
    std::string vendor, invoiceNo, reference;
    std::string paymentCtl;    // ap_payment
    std::string date, dueDate, datePaid;
    int period = 0;
    ApAccount ap;              // APACCOUNT / APDEPART: the voucher's AP account
    ApAccount bank;            // ACCOUNT / DEPT: the bank, set when a cheque is made (blank until then)
    long cheque = 0;           // CHEQUENO
    std::int64_t amount = 0, vat = 0, wh = 0, paid = 0;   // AMOUNT, VATAMT, REBATEAMT (as stored), AMOUNTPAID
};

struct Cheque {   // ap_reqst
    long number = 0;           // CHEQUE
    long request = 0;          // PAYMENTREQ
    std::string vendor, reference, date;
    ApAccount bank;            // BANKACCT / BANKDEPT
    std::int64_t amount = 0;   // AMOUNTPAID
    bool cleared = false;      // CLEARED "1" (the deployed rows)
};

struct JournalRow {   // ap_jsum
    ApAccount account;
    int period = 0;            // PERIOD is text (C4) in the table
    std::string mode;          // "1" waiting, "2" transferred
    std::string date, type;    // TYPE "AP" (invoices, payments) or "CA" (cash vouchers)
    long batchno = 0;
    std::string reference, cheque, descript;
    std::int64_t debit = 0, credit = 0;
};

std::vector<OpenItem> LoadOpenItems(cb4sqlite::Code4& code);   // in voucher order
std::vector<Cheque> LoadCheques(cb4sqlite::Code4& code);       // in cheque order
std::vector<JournalRow> LoadJournal(cb4sqlite::Code4& code);   // in table order
