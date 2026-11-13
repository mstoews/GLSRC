//
// Ar.h - the Accounts Receivable tables, records and sales invoice arithmetic
// (spec #6 2.1, 4.1, 5.3, D3, D4, D6, D8; plan S6-M0). Portable: no Win32, so
// gl_tests checks it.
//
//   ar_cust    customers (dialog 1052), in ap_vend's layout plus ship-to
//   ar_btch    sales invoice voucher headers (dialog 1053), and ar_btchd
//              their lines, with each product line's quantity and price
//   ar_dist    the open-item ledger: one row per posted invoice (PAYMENTCTL)
//   ar_jsum    AR's journal, staged for the General Ledger (ap_jsum's layout)
//   ar_linex   the port's own: each line's role (sales, VAT, AR) and a sales
//              line's amount as typed (spec #6 4.1, as Spec #5's ap_linex)
//   ar_rcpt    the port's own: one row per receipt (spec #6 5.3; ar_dist
//              holds only the latest receipt's fields)
//   ar_xfer    the port's own: which GL voucher each transferred ar_jsum row
//              went into (spec #6 4.4, as ap_xfer)
//
// The fields are read by name from the deployed layouts (checked against
// dbf/AR_*.DBF and .MDX, 2026-11-13), not from include/db_ar*.h, which don't
// match them (spec #6 2.0). Amounts are in cents (Spec #2 D12).
//
// Shared with AP (plan, "Shared with AP"): the account type ApAccount, the
// journal row JournalRow (ar_jsum has ap_jsum's layout), AddDays,
// DaysBetween, OpenPeriodOf and AccountName, all from Ap.h.
//
// Two naming traps of the deployed tables, kept because the tables are real:
//   - AR_CUST.ACCOUNT / DEPART is the customer's AR control account, and
//     AR_ACCT / AR_DEPT its sales account (AP_VEND's ACCOUNT is the AP account).
//   - AR_DIST.APACCOUNT / APDEPART holds the invoice's AR account, and
//     ACCOUNT / DEPT the bank of the latest receipt (blank until one).
//
#pragma once

#include "Ap.h"
#include "cb4sqlite.h"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

// ---- modes (D3, D8) ----------------------------------------------------------

// ar_btch MODE (D3). The original wrote "0" for both posted and cancelled;
// no deployed voucher exists (spec #6 2.7), so nothing is migrated.
namespace ar_mode {
inline constexpr char kOpen[] = "1";
inline constexpr char kPosted[] = "0";
inline constexpr char kTransferred[] = "2";
inline constexpr char kCancelled[] = "X";
} // namespace ar_mode

// ar_dist PAYMENTCTL (D8): O owed in full -> R partly received -> C settled
// (received in full, or with a discount). The original's P and D were marks
// waiting for Create; they are no longer states.
namespace ar_receipt {
inline constexpr char kOpen[] = "O";
inline constexpr char kPartial[] = "R";
inline constexpr char kSettled[] = "C";
} // namespace ar_receipt

// ar_jsum TYPE: an invoice's rows, and a receipt's (spec #6 5.2). Both
// carry the invoice voucher's number in BATCHNO; a receipt's rows carry its
// RCPTNO in CHEQUE, so reversing it (D10) finds exactly its own rows.
namespace ar_journal {
inline constexpr char kInvoice[] = "AR";
inline constexpr char kReceipt[] = "RC";
} // namespace ar_journal

// ---- tables -------------------------------------------------------------------

// Creates ar_cust, ar_btch, ar_btchd, ar_dist and ar_jsum (the deployed
// layouts and tags) when the company lacks them, and the port's ar_linex,
// ar_rcpt and ar_xfer when missing. AR_REQST, AR_CTRL, AR_SETUP, AR_SYS and
// AR_TAG are left alone (spec #6 1.2). Run by every AR screen, the transfer
// and the reports.
void EnsureArTables(cb4sqlite::Code4& code);

// ---- accounts (GL_SETAT) -----------------------------------------------------------

struct ArAccounts {
    ApAccount ar;            // AR: the default AR control account
    ApAccount sales;         // SALE: the default sales account
    ApAccount vatPayable;    // VATP: output VAT, the customers' default
    std::vector<ApBank> banks;   // TYPE "CA", in SHORT order
};

// The named accounts; any GL_SETAT row missing leaves its account empty.
ArAccounts LoadArAccounts(cb4sqlite::Code4& code);

// ---- customers ---------------------------------------------------------------------

struct Customer {
    std::string number;        // VENDORNO
    std::string shortName;     // VDSHORT
    std::string name;          // VDNAME
    std::string address1, address2, address3, postal, phone, fax;
    std::string shipAddress1, shipAddress2, shipAddress3, shipPostal, shipPhone, shipFax;   // S_*
    std::string contact;       // CONTACT
    std::string vin;           // VIN
    ApAccount ar;              // ACCOUNT / DEPART: the AR control account
    ApAccount sales;           // AR_ACCT / AR_DEPT: the sales account
    ApAccount vat;             // VAT_ACCT / VAT_DEPT: output VAT
    int terms = 0;             // TERMS, days
    int vatRate = 0;           // VAT, whole %, 0 none
};

std::vector<Customer> LoadCustomers(cb4sqlite::Code4& code);   // in number order, deleted rows skipped
std::optional<Customer> FindCustomer(cb4sqlite::Code4& code, const std::string& number);

// Adds the customer, or replaces the one with its number. True if it was
// added. TERMS_DAYS, TERMS_PERC, TERMS_TXT and CREDIT are never used (spec #6
// 2.1, Q7); a new customer gets them blank, and a replaced one keeps them.
bool SaveCustomer(cb4sqlite::Code4& code, const Customer& c);

// What Customers' Add refuses (plan S6-M1), in the order the screen checks it.
// `field` names the input to put the focus on: the number and names are on
// the main screen (1052), the rest on its Options (1063).
enum class CustomerField { None, Number, ShortName, Name, VatRate, ArAccount, SalesAccount, VatAccount };
struct CustomerProblem {
    CustomerField field = CustomerField::None;
    enum class Kind {
        Required,           // string 8101, naming the field (TARVendorAdd::Ok's szEnterError)
        BadRate,            // a VAT rate outside 0-99
        BadAccount,         // the account and dept aren't in the chart (TARVendorOptions::Ok)
        VatAccountNeeded,   // a VAT rate with no VAT account: invoices would book VAT nowhere
    } kind = Kind::Required;
};
std::optional<CustomerProblem> CheckCustomer(cb4sqlite::Code4& code, const Customer& c);

// TARVendorAdd::Delete, refusing a customer still in use (D12): any sales
// voucher (cancelled ones too: they're the record), open item or receipt.
// `usedBy` describes the first found ("sales voucher 1", "receipt 2").
// Otherwise the record is deleted and the table packed, as the original.
enum class DeleteCustomerResult { Deleted, NotFound, InUse };
DeleteCustomerResult DeleteCustomer(cb4sqlite::Code4& code, const std::string& number, std::string* usedBy = nullptr);

// ---- vouchers ---------------------------------------------------------------------

struct ArVoucher {
    long number = 0;           // BATCHNO
    std::string mode;          // ar_mode
    std::string customer;      // VENDORNO (AR_BTCH has no name field: the name is the customer's)
    std::string invoiceNo;     // INVOICENO, 8 characters
    std::string orderNo;       // ORDERNO
    std::string orderDate;     // ORDERDATE
    std::string vin;           // VIN
    std::string units;         // UNITS
    std::string reference;     // REFERENCE
    std::string descript;      // DESCRIPT, 10 characters
    std::string date;          // DATE, yyyymmdd
    std::string invoiceDate;   // INVDATE
    std::string dueDate;       // DUEDATE
    std::string memo;          // MEMOFIELD
    int period = 0;
    int terms = 0;
    int vatRate = 0;           // VATPERCENT
    ApAccount account;         // ACCOUNT / DEPART: the AR control account
    std::int64_t total = 0;    // PAYABLE: the invoice total, VAT included
    std::int64_t vatAmount = 0;   // VATAMT
    int lines = 0;             // ENTRYNO

    bool cancelled() const { return mode == ar_mode::kCancelled; }
};

enum class ArLineRole { Sales, Vat, Receivable };   // Receivable: the AR (debit) line

struct ArLine {
    int line = 0;              // ENTRYNO
    ArLineRole role = ArLineRole::Sales;   // ar_linex ROLE
    std::string descript, medrep, drcode, product;
    ApAccount account;
    std::int64_t qty = 0;      // QTY in hundredths (N14.2): 3 units are 300
    std::int64_t price = 0;    // PRICE, cents; 0: the amount was typed
    std::int64_t debit = 0, credit = 0;
    std::int64_t gross = 0;    // sales lines: the amount, VAT included (ar_linex GROSS); 0 for the others
};

std::vector<ArVoucher> LoadArVouchers(cb4sqlite::Code4& code);   // in number order, deleted rows skipped
std::optional<ArVoucher> FindArVoucher(cb4sqlite::Code4& code, long number);

// The voucher's lines in line order, with their roles and gross amounts from
// ar_linex. AR never ran (spec #6 2.7), so every voucher on file was written
// by the port; a line without an ar_linex row is read by its side (a debit
// is the AR line, a credit a sales line whose gross is its credit).
std::vector<ArLine> LoadArLines(cb4sqlite::Code4& code, long number);

// The number a new voucher gets: the highest + 1, or 1.
long NextArVoucherNumber(cb4sqlite::Code4& code);

// Writes the header (a new voucher when `v.number` is 0 or doesn't exist:
// numbered NextArVoucherNumber) and replaces all its lines with `lines`,
// renumbered 1..n, with their ar_linex rows. ENTRYNO becomes the line count.
// One transaction. Returns the voucher's number.
long SaveArVoucher(cb4sqlite::Code4& code, const ArVoucher& v, const std::vector<ArLine>& lines);

// D6: another voucher, not cancelled, posted or not, with the same customer
// and invoice number (trimmed, case as typed); `except` is the voucher being
// edited (0: none). Returns its number, or 0.
long DuplicateArInvoice(cb4sqlite::Code4& code, const std::string& customer, const std::string& invoiceNo,
                        long except = 0);

// ---- the arithmetic (spec #6 4.1, 5.2, D4) ----------------------------------------------

// A product line's amount: quantity (hundredths) x price (cents), rounded to
// the cent; or `typed` when there is no price (price 0).
std::int64_t LineAmount(std::int64_t qty, std::int64_t price, std::int64_t typed);

struct SaleSplit {
    std::int64_t sales = 0, vat = 0;
};

// An amount with VAT included, the rate in whole percent (0: none):
// VAT = round(a x v / (100 + v)), sales = a - VAT, rounding half away from
// zero. The same split gives a receipt discount's sales and VAT parts
// (spec #6 5.2), so a discount of the whole invoice reverses it exactly.
SaleSplit SplitSale(std::int64_t amount, int vatRate);

struct SaleAccounts {
    ApAccount vat;          // the VAT line's account (the customer's VAT_ACCT)
    ApAccount ar;           // the AR line's account (the voucher's ACCOUNT)
    std::string vatText, arText;   // the computed lines' descriptions (the accounts' names)
};

struct ComputedSale {
    std::vector<ArLine> lines;   // the sales lines (credit = their sales), then VAT and AR
    std::int64_t total = 0;      // the sum of the product lines' amounts: the AR line
    std::int64_t vat = 0;        // the sum of the lines' VAT
    std::int64_t debits = 0, credits = 0;   // the lines' totals: always equal
};

// D4: the voucher's lines from its product lines (`gross` set, or `qty` and
// `price`: LineAmount; role, debit and credit are ignored). Each line is split
// by SplitSale and credited to its own account; then one VAT line (Cr, the
// sum of the lines' VAT) and one AR line (Dr, the total), each left out when
// zero, with the descriptions `accounts` gives.
ComputedSale ComputeSale(const std::vector<ArLine>& productLines, int vatRate, const SaleAccounts& accounts);

// ---- invoice entry (spec #6 4.1, 4.2, D4, D5; plan S6-M2) ---------------------------------

// A product line as the entry screen gives it: a quantity and price, or (price
// 0) the amount typed, VAT included.
struct SaleEntry {
    int line = 0;              // the product line it replaces (1-based among the product lines); 0 adds one
    std::string descript, medrep, drcode, product;
    ApAccount account;         // the sales account
    std::int64_t qty = 0;      // hundredths
    std::int64_t price = 0;    // cents; 0: `gross` was typed
    std::int64_t gross = 0;    // the amount typed (used only when price is 0)
};

// Adds or replaces a product line of voucher `header.number` (a new voucher
// when 0 or not on file, in mode open), then recomputes the VAT and AR lines
// from all its product lines with the header's VAT rate (D4): the VAT line
// on `vat`, the AR line on `header.account`. The header is saved with the
// totals (PAYABLE, VATAMT). A line with no text gets its account's name, as
// do the computed lines. Returns the voucher's number, or 0 if the voucher
// on file isn't open (posted, transferred or cancelled).
long SaveSaleLine(cb4sqlite::Code4& code, ArVoucher header, const SaleEntry& entry, const ApAccount& vat);

// Removes product line `line` (1-based among the product lines) and
// recomputes, as SaveSaleLine, with the voucher's own rate and AR account.
// False if the voucher isn't open or has no such line.
bool DeleteSaleLine(cb4sqlite::Code4& code, long number, int line, const ApAccount& vat);

// D5: "No" to keeping a voucher made in this session removes that voucher
// (header, lines, ar_linex rows) and nothing else. Only an open voucher.
bool DeleteArVoucher(cb4sqlite::Code4& code, long number);

// TARMemo::Ok: the voucher's memo (MEMOFIELD) replaced by `memo`. False if
// there is no such voucher.
bool SaveArMemo(cb4sqlite::Code4& code, long number, const std::string& memo);

// ---- Post, Cancel, Recover (spec #6 4.3, D3, D7; plan S6-M3) -----------------------------

// TARPostBatch::Ok / HandleListBox, in one transaction: every open voucher,
// or only `number`. For each that balances to the cent, has lines, and whose
// accounts are in the chart: an ar_dist row (PAYMENTCTL "O", AMOUNT =
// REMAINDER = the total, its VAT, AR account, invoice and due dates), its
// ar_jsum rows (TYPE "AR", waiting "1", in the voucher's period, dated its
// invoice date), then MODE "0". A voucher that isn't open is neither posted
// nor refused. The refusals are AP's (Ap.h ApPostRefusal).
ApPostResult PostArVouchers(cb4sqlite::Code4& code, long number = 0);

// TARCancelBatch::HandleListBox: an open voucher becomes "X" (D3), its lines
// kept for the record (the original zeroed them and wrote "0", as posted).
ApCancelResult CancelArVoucher(cb4sqlite::Code4& code, long number);

enum class ArRecoverResult {
    Recovered,
    NotPosted,     // not on file, or not "0"
    Transferred,   // its journal is in a GL voucher: cancel that first
    Received,      // a receipt has been entered against it
};

// D7, in one transaction: reopens posted voucher `number` whose journal is
// still waiting and which has no receipt: its ar_dist row and its "AR"
// ar_jsum rows go, and MODE becomes "1".
ArRecoverResult RecoverArVoucher(cb4sqlite::Code4& code, long number);

// ---- the transfer to the General Ledger (spec #6 4.4, D2; plan S6-M3) ---------------------

// Maintenance > Update AR and AP, the AR half (TGLAPUpdate::ARUpdate), in one
// transaction: as Ap.h TransferAp, on ar_jsum (invoices' and receipts' rows),
// described "Accounts Receivable GL Update", linked in ar_xfer. Then each
// posted voucher whose "AR" rows are all transferred gets "2"; cancelled ones
// keep "X" (2.6 #13).
ApTransferResult TransferAr(cb4sqlite::Code4& code, long iniBatchno);

// Spec #2's CancelVoucher calls this beside UndoApTransfer: the rows
// transferred into GL voucher `glVoucher` wait again, and their vouchers go
// back from "2" to "0". Returns the rows put back.
int UndoArTransfer(cb4sqlite::Code4& code, long glVoucher);

// ---- the ledger rows (read-only until their milestones) ------------------------------------

struct ArOpenItem {   // ar_dist
    long batchno = 0;
    std::string customer, invoiceNo, reference;
    std::string paymentCtl;    // ar_receipt
    std::string date, dueDate, datePaid;
    int period = 0;
    ApAccount ar;              // APACCOUNT / APDEPART: the invoice's AR account
    ApAccount bank;            // ACCOUNT / DEPT: the latest receipt's bank (blank until one)
    std::int64_t amount = 0;   // AMOUNT: the invoice total
    std::int64_t vat = 0;      // VATAMT
    std::int64_t received = 0; // AMOUNTPAID: all receipts so far
    std::int64_t remainder = 0;   // REMAINDER: what's owed
};

struct Receipt {   // ar_rcpt
    long number = 0;           // RCPTNO, numbered across all invoices
    long batchno = 0;          // the invoice voucher
    std::string customer, invoiceNo;
    std::string date;          // when the money came in
    int period = 0;            // the date's period
    ApAccount bank;            // BANKACCT / BANKDEPT
    std::int64_t amount = 0;   // received
    std::int64_t discount = 0; // written off as an early-payment discount (0: none)
    std::int64_t discountVat = 0;   // the discount's VAT part
    std::string reference;     // the customer's cheque number or a note
};

std::vector<ArOpenItem> LoadArOpenItems(cb4sqlite::Code4& code);              // in voucher order
std::vector<Receipt> LoadReceipts(cb4sqlite::Code4& code, long batchno = 0);   // in receipt order; 0: all
std::vector<JournalRow> LoadArJournal(cb4sqlite::Code4& code);                // in table order
