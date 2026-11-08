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
// (Tglbtch::Open / Closed / Adjustment). Voucher Entry writes "1", Prior
// Period Entry "3"; posting (S2-M4) sets "0". Lines get no mode until posted.
//
#pragma once

#include "cb4sqlite.h"

#include <string>
#include <vector>

namespace voucher_mode {
inline constexpr char kOpen[] = "1";
inline constexpr char kPosted[] = "0";
inline constexpr char kAdjustment[] = "3";
} // namespace voucher_mode

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
    Posted,     // mode "0": the original showed nothing
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
