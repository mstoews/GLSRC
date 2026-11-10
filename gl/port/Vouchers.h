//
// Vouchers.h - the General Ledger voucher tables and the record operations of
// Voucher Entry (gl/src/gl_btche.cpp, dialog 1013) and Prior Period Entry
// (gl/src/gl_prior.cpp), without the dialog. Spec #2 design 2.3, plan S2-M3.
//
// No Win32 here, so gl_tests checks the operations against the migrated
// fixture. VoucherEntry.cpp reads the controls and calls these.
//
// The table classes open their fields by name from the real layouts, not
// from include/db_btch.h / db_btchd.h: db_btch.h opens a "descript" field
// gl_btch doesn't have (the voucher description is "reference"), and
// db_btchd.h lacks medrep, drcode and prod (spec 5.4).
//
//   gl_btch   one header per voucher: batchno, mode, reference (the
//             voucher's description), period, date, debit and credit
//             totals, entryno (its line count), cheque
//   gl_btchd  its lines: batchno, journal (the line number), period,
//             date, reference (the document), descript, cheque, account,
//             dept, medrep, drcode, prod, debit, credit
//
// gl_btch.mode: "1" open, "0" posted, "3" a prior-period adjustment
// (Tglbtch::Open / Closed / Adjustment), "2" a posted adjustment. Voucher
// Entry writes "1", Prior Period Entry "3"; posting (S2-M4) sets "0" or
// "2", and Recover sets "0" back to "1". Lines get no mode until posted
// ("0"); Recover blanks it again.
//
// Posting, Recover and Cancel (gl_postb.cpp, gl_postr.cpp, gl_btcan.cpp;
// plan S2-M4) are at the end. Each runs in one transaction (spec D1) on
// tables of its own; after a failure nothing is written, and tables the
// caller has open must be re-read.
//
#pragma once

#include "cb4sqlite.h"

#include <stdexcept>
#include <string>
#include <vector>

namespace voucher_mode {
inline constexpr char kOpen[] = "1";
inline constexpr char kPosted[] = "0";
inline constexpr char kAdjustment[] = "3";
inline constexpr char kPostedAdjustment[] = "2";   // PostAdjustment
} // namespace voucher_mode

// "0" or "2": posted, so Voucher Entry shows nothing and Cancel refuses it.
bool IsPostedMode(const std::string& mode);

// gl_btch, on its "batchno" tag.
class VoucherHeaders : public Data4 {
public:
    explicit VoucherHeaders(Code4& code);

    Field4 batchno, mode, reference, period, date, debit, credit, entryno, cheque;
};

// gl_btchd, on its "batchno" tag.
class VoucherLines : public Data4 {
public:
    explicit VoucherLines(Code4& code);

    Field4 batchno, mode, journal, period, sourcecode, date, reference, descript, cheque, account, dept, medrep,
        drcode, prod, debit, credit;
};

// A voucher header as the dialog shows it.
struct VoucherHeader {
    long number = 0;
    std::string mode;          // voucher_mode
    std::string description;   // gl_btch.reference (control 2999)
    std::string date;          // YYYYMMDD
    std::string cheque;
    int period = 0;
    double debit = 0, credit = 0;   // totals of the lines
    int lines = 0;                  // entryno
};

// A voucher line. Codes are as stored (medrep / drcode / prod codes, not names).
struct VoucherLine {
    int line = 0;              // journal
    int period = 0;
    std::string date;          // YYYYMMDD
    std::string reference;     // the document (control 3036)
    std::string descript;      // the account name (control 3037)
    std::string cheque;
    std::string account, dept;
    std::string medrep, drcode, prod;
    double debit = 0, credit = 0;
};

// The number the next new voucher gets: the last voucher's number + 1, or
// `iniBatchno` + 1 (GL.INI [PERIOD] BATCHNO) when gl_btch is empty.
long NextVoucherNumber(VoucherHeaders& h, long iniBatchno);

// Seeks voucher `number`; true if that exact voucher is current.
bool SeekVoucher(VoucherHeaders& h, long number);

// The current header record as a VoucherHeader.
VoucherHeader ReadHeader(const VoucherHeaders& h);

enum class LoadResult {
    Loaded,
    NotFound,   // no such voucher
    Posted,     // mode "0" or "2": the original showed nothing
    NoLines,    // a header without lines (string 6087)
};

// TGLBatchEntry::CurrentBatch: voucher `number`'s header and lines. The lines
// come in the original's order (debit-only lines, then credit-only lines,
// then lines with both) and are renumbered 1..n in that order; the new
// numbers are written back, as are the header's debit / credit totals and
// its line count.
LoadResult LoadVoucher(VoucherHeaders& h, VoucherLines& l, long number, VoucherHeader& header,
                       std::vector<VoucherLine>& lines);

enum class SaveKind { NewVoucher, LineAppended, LineReplaced };

// TGLBatchEntry::Add, after the dialog's checks. If voucher `number` doesn't
// exist, a new voucher is created with NextVoucherNumber (in `header.mode`)
// holding `line`. Otherwise the header's description, date, period and
// cheque are replaced (its mode is kept), and the line whose number is
// `line.line` is replaced, or `line` is appended if there is none. Then the
// totals and line count are recomputed. Returns the voucher's number.
long SaveLine(VoucherHeaders& h, VoucherLines& l, long number, const VoucherHeader& header,
              const VoucherLine& line, long iniBatchno, SaveKind* kind = nullptr);

// TGLBatchEntry::Delete: removes line `line` of voucher `number` (pack) and
// recomputes the totals. False if there is no such line.
bool DeleteLine(VoucherHeaders& h, VoucherLines& l, long number, int line);

// TGLBatchEntry::RollBack: removes voucher `number`'s header and lines.
void DeleteVoucher(VoucherHeaders& h, VoucherLines& l, long number);

// Recomputes voucher `number`'s debit / credit totals and line count from
// its lines and writes them to the header. False if there is no header.
bool UpdateTotals(VoucherHeaders& h, VoucherLines& l, long number);

// True when the totals balance to the cent.
bool Balanced(double debit, double credit);

// An open accounting period (gl_pds.status "0").
struct OpenPeriod {
    int number = 0;
    std::string description;   // pd_desc
};

// ::SetPeriods (gl/src/ap_repts.cpp): the open periods, last first, as the
// original filled the combo box. The original selected the last one added,
// the lowest open period.
std::vector<OpenPeriod> OpenPeriods(Code4& code);

// ---- Post, Recover, Cancel (plan S2-M4) -----------------------------------

// The headers whose mode is one of `modes`, in voucher-number order: the rows
// of Post (open and adjustment vouchers), Recover (posted) and Cancel (open
// and adjustment).
std::vector<VoucherHeader> ListVouchers(VoucherHeaders& h, const std::vector<std::string>& modes);

enum class PostRefusal {
    OutOfBalance,     // string 8116
    UnknownAccount,   // string 3081; the account and dept are in the refusal
};

struct VoucherRefusal {
    long number = 0;
    PostRefusal reason = PostRefusal::OutOfBalance;
    std::string account, dept;   // UnknownAccount: the first line's account that isn't in glaccts
};

struct PostResult {
    std::vector<long> posted;
    std::vector<VoucherRefusal> refused;
};

// TGLPostBatch::Ok: posts every open and adjustment voucher, in one
// transaction. A voucher whose lines don't balance to the cent, or with a
// line whose account and dept aren't in glaccts, is refused and the rest
// still post. A voucher without lines is skipped, as before.
//
//   open ("1")        PostGLSlip: each line's debit - credit is added to
//                     glaccts.period<N> (N the line's period) and curr_bal;
//                     the line's mode becomes "0", and a gl_dist row copies
//                     it; the header's mode becomes "0".
//   adjustment ("3")  PostAdjustment: each line's debit - credit is added to
//                     glaccts.openbal and curr_bal is recomputed as openbal
//                     + period1..period12; the header's mode becomes "2". No
//                     gl_dist rows, and the lines' mode is left alone.
//
// Throws (and writes nothing) if the database fails, or on the /failpost
// fault.
PostResult PostAll(Code4& code);

// TGLPostBatch::HandleListBox: posts voucher `number` alone, the same way.
// A voucher that isn't open or an adjustment is neither posted nor refused.
PostResult PostVoucher(Code4& code, long number);

// gl.exe /failpost <n> (spec 8 #6): PostAll and PostVoucher throw PostFault
// after writing the nth line of the call. 0 (the default) turns it off.
void SetPostFault(int nthLine);

class PostFault : public std::runtime_error {
public:
    explicit PostFault(int line);
};

enum class RecoverResult {
    Recovered,
    NotPosted,        // no such voucher, or its mode isn't "0"
    UnknownAccount,   // a line's account and dept aren't in glaccts (string 3081)
};

// TGLRecoverBatch::HandleDoubleClick, in one transaction: for each line of
// posted voucher `number`, debit - credit is subtracted from
// glaccts.period<N> and curr_bal and the line's mode is blanked; the
// header's mode becomes "1"; the voucher's GL rows in gl_dist are deleted
// (not AR or AP rows that share its number). `account` / `dept` name the
// missing account for UnknownAccount.
RecoverResult RecoverVoucher(Code4& code, long number, std::string* account = nullptr, std::string* dept = nullptr);

enum class CancelResult { Cancelled, NotFound, Posted };

// TGLCancelBatch::SelectSlip, in one transaction: removes an unposted
// voucher's header and lines.
CancelResult CancelVoucher(Code4& code, long number);

// ---- Transaction Inquiry (plan S2-M5) -------------------------------------

// A posted voucher line (gl_btchd, mode "0").
struct PostedLine {
    int period = 0;
    long number = 0;          // batchno
    std::string descript;
    double debit = 0, credit = 0;
};

struct AccountActivity {
    bool found = false;       // the account and dept are in glaccts
    std::string descript;     // its description
    double openBal = 0;       // glaccts.openbal
    std::vector<PostedLine> lines;
    double debit = 0, credit = 0;   // the lines' totals
    double balance = 0;             // openBal + debit - credit
};

// TGLTransInq::SetBatchList: the posted lines of exactly that account and
// dept, in the "account" tag's order, with the opening balance. The
// original's query, ACCOUNT = '<a>' .AND. DEPT = '<d>' .AND. MODE = '0',
// was a dBase left match, so a blank dept listed every dept's lines and a
// short account its neighbours' (spec D13).
AccountActivity AccountTransactions(Code4& code, const std::string& account, const std::string& dept);

// ---- Year End (plan S2-M6) -------------------------------------------------

struct RetainedAccount {
    std::string account, dept;
};

struct YearEndResult {
    bool closed = false;
    // The accounts of type "R" found. Year end runs only with exactly one.
    std::vector<RetainedAccount> retained;
    // The total moved into it, its own balance included: its new balance.
    double transferred = 0;
};

// TGlWindow::GLYearEnd (gl_postb.cpp), corrected (spec D2), in one
// transaction:
//   1. every account whose type isn't "B" adds its curr_bal to a total and
//      is set to 0 (the Retained Earnings account included);
//   2. every account's period1-12 are copied to previous1-12 and zeroed;
//   3. the total is added to the Retained Earnings account (type "R").
// The original's step 3 added it to every account that *isn't* type "R".
// Refuses, changing nothing, unless exactly one account has type "R".
// openbal is left alone, as before. Throws on a database failure, having
// written nothing.
YearEndResult YearEnd(Code4& code);
