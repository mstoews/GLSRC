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
//   ap_xfer    the port's own: which GL voucher each transferred ap_jsum row
//              went into (spec #5 4.4; ap_jsum has no field for it), so
//              cancelling that voucher puts its rows back (plan S5-M3)
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
// lacks them, and ap_linex and ap_xfer when missing; then migrates D3: an invoice or cash
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

// What Vendors' Add refuses (plan S5-M1), in the order the screen checks it.
// `field` names the input to put the focus on.
enum class VendorField { None, Number, ShortName, Name, Type, VatRate, WhRate, ApAccount, VatAccount };
struct VendorProblem {
    VendorField field = VendorField::None;
    enum class Kind {
        Required,       // string 8101, naming the field (TAPVendorAdd::Ok's szEnterError)
        NoType,         // neither Individual nor Company: the W/H account depends on it
        BadRate,        // a rate outside 0-99 (the fields are N2)
        BadAccount,     // the account and dept aren't in the chart (the original never checked)
        VatAccountNeeded,   // a VAT rate with no VAT account: invoices would book VAT nowhere
    } kind = Kind::Required;
};
std::optional<VendorProblem> CheckVendor(cb4sqlite::Code4& code, const Vendor& v);

// TAPVendorAdd::Delete, refusing a vendor still in use (D12): any invoice or
// cash voucher (cancelled ones too: they're the record), open item or cheque.
// `usedBy` describes the first found ("invoice voucher 1", "cheque 5001").
// Otherwise the record is deleted and the table packed, as the original.
enum class DeleteVendorResult { Deleted, NotFound, InUse };
DeleteVendorResult DeleteVendor(cb4sqlite::Code4& code, const std::string& number, std::string* usedBy = nullptr);

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

// ---- invoice entry (spec #5 4.2, D4, D5; plan S5-M2) ------------------------------

// yyyymmdd plus `days` (the due date: invoice date + terms); "" for an invalid date.
std::string AddDays(const std::string& yyyymmdd, int days);

// The account's description in glaccts ("" if it isn't there).
std::string AccountName(cb4sqlite::Code4& code, const ApAccount& account);

// An expense line as the entry screen gives it: the amount typed.
struct ExpenseEntry {
    int line = 0;              // the expense line it replaces (1-based among the expense lines); 0 adds one
    std::string descript, medrep, drcode, product;
    ApAccount account;
    std::int64_t gross = 0;
};

// Adds or replaces an expense line of voucher `header.number` (a new voucher
// when 0 or not on file, in mode open), then recomputes the VAT, W/H and
// credit lines from all its expense lines with the header's rates (D4) and
// saves the header with the totals: PAYABLE the typed amounts, VATAMT, and
// REBATEAMT negative in F mode, positive in T. An expense line or computed
// line with no text gets its account's name. Returns the voucher's number,
// or 0 if the voucher on file isn't open (posted, transferred or cancelled).
long SaveExpenseLine(cb4sqlite::Code4& code, ApVoucher header, const ExpenseEntry& entry, TaxMode mode,
                     VoucherAccounts accounts);

// Removes expense line `line` (1-based among the expense lines) and
// recomputes, as SaveExpenseLine, with the voucher's own rates. False if the
// voucher isn't open or has no such line.
bool DeleteExpenseLine(cb4sqlite::Code4& code, ApKind kind, long number, int line, TaxMode mode,
                       VoucherAccounts accounts);

// D5: "No" to keeping a voucher made in this session removes that voucher
// (header, lines, ap_linex rows) and nothing else. Only an open voucher.
bool DeleteApVoucher(cb4sqlite::Code4& code, ApKind kind, long number);

// ---- Post, Cancel, Recover (spec #5 4.3, 5, D3, D7, D10, D11; plan S5-M3) ---------------

enum class ApPostRefusal {
    OutOfBalance,     // string 1007 (the lines' debits and credits differ by a cent or more)
    NoLines,
    UnknownAccount,   // a line's account and dept aren't in glaccts: the GL could never post it
};

struct ApRefusal {
    long number = 0;
    ApPostRefusal reason = ApPostRefusal::OutOfBalance;
    ApAccount account;   // UnknownAccount
};

struct ApPostResult {
    std::vector<long> posted;
    std::vector<ApRefusal> refused;
};

// TAPPostBatch::Ok / HandleListBox (and the cash dialog's), in one
// transaction: every open voucher of the kind, or only `number`. For each
// that balances to the cent, has lines, and whose accounts are in the chart:
//   invoice  an ap_dist row (PAYMENTCTL "O", D11: the original's "H" needed a
//            manual release) and ap_jsum rows, TYPE "AP";
//   cash     ap_jsum rows, TYPE "CA", with the cheque number; no ap_dist row:
//            nothing is owed (D10);
// the rows waiting ("1"), in the voucher's period, dated its invoice date;
// then MODE "0". A voucher that isn't open is neither posted nor refused.
ApPostResult PostApVouchers(cb4sqlite::Code4& code, ApKind kind, long number = 0);

enum class ApCancelResult { Cancelled, NotFound, NotOpen };

// TAPCancelBatch::HandleListBox: an open voucher becomes "X" (D3). Its lines
// are kept for the record (the original zeroed them and wrote "Cancelled"
// over the reference and descriptions).
ApCancelResult CancelApVoucher(cb4sqlite::Code4& code, ApKind kind, long number);

enum class ApRecoverResult {
    Recovered,
    NotPosted,     // not on file, or not "0"
    Transferred,   // its journal is in a GL voucher: cancel that first
    Paid,          // its invoice is in a payment (P, V or C)
};

// D7, in one transaction: reopens posted voucher `number` whose journal is
// still waiting and whose invoice isn't paid. It removes exactly its own
// ap_dist row and ap_jsum rows (the original removed AR's) and sets MODE "1".
ApRecoverResult RecoverApVoucher(cb4sqlite::Code4& code, ApKind kind, long number);

// ---- the transfer to the General Ledger (spec #5 4.4, D2, D10; plan S5-M3) ----------------

struct ApTransferVoucher {
    int period = 0;
    long glVoucher = 0;     // 0: the period's rows netted to nothing; marked transferred, no voucher
    int lines = 0;
    std::int64_t debit = 0, credit = 0;
    int rows = 0;           // ap_jsum rows it carries
};

struct ApTransferResult {
    std::vector<ApTransferVoucher> vouchers;   // in period order
    std::vector<int> closedPeriods;            // left waiting: their periods are closed (gl_pds STATUS "1")
    std::vector<int> undatedPeriods;           // left waiting: no such period, or no end date
};

// Maintenance > Update AR and AP, the AP half (TGLAPUpdate::APUpdate), in one
// transaction: the waiting ap_jsum rows ("1", or blank as the original read
// them) become one open GL voucher per period (D2; the original made one, in
// the GL's current period), its lines netted by account and dept, dated the
// period's end, described "Accounts Payable GL Update" (string 1049),
// numbered as Spec #2's SaveLine numbers a new voucher (`iniBatchno`: GL.INI
// [PERIOD] BATCHNO). The rows get "2" and an ap_xfer row each; then every
// posted invoice and cash voucher whose rows are all transferred gets "2"
// (D10: the original never marked cash vouchers). The GL vouchers post with
// Spec #2's Post Voucher.
ApTransferResult TransferAp(cb4sqlite::Code4& code, long iniBatchno);

// Spec #2's CancelVoucher calls this inside its transaction (spec #5 4.4, as
// Spec #4's UndoRun): the rows transferred into GL voucher `glVoucher` wait
// again ("1"), and their vouchers go back from "2" to "0". Returns the rows
// put back; 0 for an ordinary voucher or a company without AP.
int UndoApTransfer(cb4sqlite::Code4& code, long glVoucher);

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

// ---- payments: Create Cheques (spec #5 6.1, D8, D9; plan S5-M5) ---------------------------

// What a posted invoice still owes: its amount less the W/H withheld (F mode:
// REBATEAMT is negative). In T mode the W/H was debited, so the whole amount.
std::int64_t AmountOwed(const OpenItem& item);

struct PaymentItem {
    OpenItem item;
    std::string vendorName;   // VDSHORT, or VDNAME when blank
    std::int64_t owed = 0;    // AmountOwed
};

// Create Cheques' list (TAPCreatePayment::DistList): the open items held,
// open or forced (H, O, F) due by `dueBy` (yyyymmdd), and the forced ones
// whatever their due date; with `all`, every item, paid ones included (View
// All). In vendor order, then voucher order.
std::vector<PaymentItem> PaymentItems(cb4sqlite::Code4& code, const std::string& dueBy, bool all = false);

// TAPHoldForce::Ok: sets the open item of invoice voucher `batchno` to H, O
// or F. Refused (false) for an item already in a payment (P, V, C: string
// 4010) or not on file.
bool SetPaymentControl(cb4sqlite::Code4& code, long batchno, const std::string& control);

// The next cheque number: the highest on a cheque or an open item, + 1.
long NextChequeNumber(cb4sqlite::Code4& code);

// The open accounting period whose dates hold `yyyymmdd` (gl_pds STATUS
// "0"), or 0 if none (closed, or no such dates).
int OpenPeriodOf(cb4sqlite::Code4& code, const std::string& yyyymmdd);

struct ChequeMade {
    long number = 0;
    std::string vendor, vendorName;
    std::int64_t amount = 0;
    int items = 0;
};

enum class CreateChequesStatus {
    Created,
    NothingDue,
    BadBank,           // the bank isn't in the chart (string 4007)
    BadNumber,         // the first number is 0, or a number it would use is already a cheque's
    ClosedPeriod,      // the payment date isn't in an open period
};

struct CreateChequesResult {
    CreateChequesStatus status = CreateChequesStatus::NothingDue;
    std::vector<ChequeMade> cheques;
    int period = 0;
    long clash = 0;    // BadNumber: the number already used
};

// TAPCreatePayment::Ok and CreateGLEntry, corrected (D8, D9), in one
// transaction: the open items due by `dueBy` (O) and the forced ones (F),
// grouped by vendor, one cheque each, numbered from `firstNumber` in vendor
// order, all drawn on `bank`. Each cheque goes in ap_reqst (not cleared);
// each item gets the cheque, the amount owed as paid, the payment date, the
// bank, and "P"; and its journal gets Dr its AP account / Cr the bank for
// the amount owed, TYPE "AP", the invoice voucher's number, dated the
// payment date, in that date's period, with the cheque number. (The original
// dated these rows by the invoice and numbered the cheques only later, in Set
// Cheque Numbers.)
CreateChequesResult CreateCheques(cb4sqlite::Code4& code, const std::string& dueBy, const std::string& payDate,
                                  const ApAccount& bank, long firstNumber);
