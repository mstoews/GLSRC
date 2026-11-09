//
// Vouchers.cpp - see Vouchers.h.
//
// Translation notes (corrections are Spec #2 design D11):
//   - The header's description went to gl_btch.descript, which the real
//     table doesn't have; it is gl_btch.reference.
//   - AppendNewEntry numbered a new voucher from the last header + 1, which
//     is 1 when gl_btch is empty, while the dialog showed GL.INI [PERIOD]
//     BATCHNO + 1. The port saves the number it shows (NextVoucherNumber).
//   - Its totals started from whichever header and line happened to be
//     current; CurrentBatch then recomputed them. The port computes them
//     from the voucher's lines, to the cent.
//   - Add replaced or appended according to the line record that happened
//     to be current (glbtchd->iJournal()); a line number lower than that one
//     saved nothing. The port replaces the voucher's line with the typed
//     number, or appends.
//   - ReplaceHeader wrote the dialog's mode, so editing an adjustment ("3")
//     in Voucher Entry turned it into a normal voucher ("1"); the port keeps
//     the voucher's mode.
//   - CurrentBatch renumbered the lines but flushed only some of the new
//     numbers; the port writes them all.
//
#include "Vouchers.h"

#include "Accounts.h"   // TrimText
#include "Company.h"    // UpdateUserTime

#include <cmath>
#include <cstdio>
#include <cstdlib>

extern char szUser[11];   // Globals.h, which needs <Windows.h>; this file stays portable

namespace {

long toLong(const Field4& f) { return std::lround(f.num()); }

double cents(double v) { return std::round(v * 100.0) / 100.0; }

// gl_btchd.period: the period number right-aligned in its C(2) field (" 7").
std::string periodText(int period, unsigned width)
{
    std::string s = std::to_string(period);
    if (s.size() < width) s.insert(0, width - s.size(), ' ');
    return s;
}

bool seekLines(VoucherLines& l, long number)
{
    if (!l.isValid()) return false;
    l.SetIndex("batchno");
    return l.seek(static_cast<double>(number)) == r4success && toLong(l.batchno) == number;
}

VoucherLine readLine(const VoucherLines& l)
{
    VoucherLine v;
    v.line = static_cast<int>(toLong(l.journal));
    v.period = std::atoi(TrimText(l.period.str()).c_str());
    v.date = TrimText(l.date.str());
    v.reference = TrimText(l.reference.str());
    v.descript = TrimText(l.descript.str());
    v.cheque = TrimText(l.cheque.str());
    v.account = TrimText(l.account.str());
    v.dept = TrimText(l.dept.str());
    v.medrep = TrimText(l.medrep.str());
    v.drcode = TrimText(l.drcode.str());
    v.prod = TrimText(l.prod.str());
    v.debit = l.debit.num();
    v.credit = l.credit.num();
    return v;
}

// AppendDetail / ReplaceDetail: the line's fields (not batchno, not mode).
void assignLine(VoucherLines& l, const VoucherLine& v)
{
    l.journal.assignLong(v.line);
    l.date.assign(v.date.c_str());
    l.period.assign(periodText(v.period, l.period.len()).c_str());
    l.reference.assign(v.reference.c_str());
    l.descript.assign(v.descript.c_str());
    l.account.assign(v.account.c_str());
    l.dept.assign(v.dept.c_str());
    l.cheque.assign(v.cheque.c_str());
    l.medrep.assign(v.medrep.c_str());
    l.prod.assign(v.prod.c_str());
    l.drcode.assign(v.drcode.c_str());
    l.debit.assignDouble(cents(v.debit));
    l.credit.assignDouble(cents(v.credit));
}

// ReplaceHeader's fields, without the mode or totals.
void assignHeader(VoucherHeaders& h, const VoucherHeader& v)
{
    h.date.assign(v.date.c_str());
    h.period.assignLong(v.period);
    h.reference.assign(v.description.c_str());
    h.cheque.assign(v.cheque.c_str());
}

} // namespace

VoucherHeaders::VoucherHeaders(Code4& c) : Data4(&c, "gl_btch")
{
    if (!isValid()) return;
    SetIndex("batchno");
    batchno.init(*this, "batchno");
    mode.init(*this, "mode");
    reference.init(*this, "reference");
    period.init(*this, "period");
    date.init(*this, "date");
    debit.init(*this, "debit");
    credit.init(*this, "credit");
    entryno.init(*this, "entryno");
    cheque.init(*this, "cheque");
    top();
}

VoucherLines::VoucherLines(Code4& c) : Data4(&c, "gl_btchd")
{
    if (!isValid()) return;
    SetIndex("batchno");
    batchno.init(*this, "batchno");
    mode.init(*this, "mode");
    journal.init(*this, "journal");
    period.init(*this, "period");
    sourcecode.init(*this, "sourcecode");
    date.init(*this, "date");
    reference.init(*this, "reference");
    descript.init(*this, "descript");
    cheque.init(*this, "cheque");
    account.init(*this, "account");
    dept.init(*this, "dept");
    medrep.init(*this, "medrep");
    drcode.init(*this, "drcode");
    prod.init(*this, "prod");
    debit.init(*this, "debit");
    credit.init(*this, "credit");
    top();
}

long NextVoucherNumber(VoucherHeaders& h, long iniBatchno)
{
    if (!h.isValid()) return iniBatchno + 1;
    h.SetIndex("batchno");
    h.bottom();
    return (h.eof() ? iniBatchno : toLong(h.batchno)) + 1;
}

bool SeekVoucher(VoucherHeaders& h, long number)
{
    if (!h.isValid() || number <= 0) return false;
    h.SetIndex("batchno");
    return h.seek(static_cast<double>(number)) == r4success && toLong(h.batchno) == number;
}

VoucherHeader ReadHeader(const VoucherHeaders& h)
{
    VoucherHeader v;
    v.number = toLong(h.batchno);
    v.mode = TrimText(h.mode.str());
    v.description = TrimText(h.reference.str());
    v.date = TrimText(h.date.str());
    v.cheque = TrimText(h.cheque.str());
    v.period = static_cast<int>(toLong(h.period));
    v.debit = h.debit.num();
    v.credit = h.credit.num();
    v.lines = static_cast<int>(toLong(h.entryno));
    return v;
}

bool Balanced(double debit, double credit)
{
    return std::llround(debit * 100.0) == std::llround(credit * 100.0);
}

bool UpdateTotals(VoucherHeaders& h, VoucherLines& l, long number)
{
    if (!SeekVoucher(h, number)) return false;
    long long debit = 0, credit = 0;   // cents
    int count = 0;
    if (seekLines(l, number))
        for (; !l.eof() && toLong(l.batchno) == number; l.skip()) {
            if (l.deleted()) continue;
            debit += std::llround(l.debit.num() * 100.0);
            credit += std::llround(l.credit.num() * 100.0);
            ++count;
        }
    if (!SeekVoucher(h, number)) return false;
    h.debit.assignDouble(static_cast<double>(debit) / 100.0);
    h.credit.assignDouble(static_cast<double>(credit) / 100.0);
    h.entryno.assignLong(count);
    UpdateUserTime(h, szUser);   // gl_btch has no update/user/time fields: a no-op
    h.flush();
    return true;
}

LoadResult LoadVoucher(VoucherHeaders& h, VoucherLines& l, long number, VoucherHeader& header,
                       std::vector<VoucherLine>& lines)
{
    lines.clear();
    if (!SeekVoucher(h, number)) return LoadResult::NotFound;
    header = ReadHeader(h);
    if (header.mode == voucher_mode::kPosted) return LoadResult::Posted;
    if (!seekLines(l, number)) return LoadResult::NoLines;

    // Debit-only, then credit-only, then both (CurrentBatch's three passes).
    // A line with neither went in no pass and isn't shown, as before.
    std::vector<long> order[3];
    for (; !l.eof() && toLong(l.batchno) == number; l.skip()) {
        if (l.deleted()) continue;
        const double d = l.debit.num(), c = l.credit.num();
        if (d > 0 && c == 0) order[0].push_back(l.recNo());
        else if (c > 0 && d == 0) order[1].push_back(l.recNo());
        else if (c > 0 && d > 0) order[2].push_back(l.recNo());
    }
    int n = 0;
    for (const auto& pass : order)
        for (long rec : pass) {
            l.go(rec);
            l.journal.assignLong(++n);
            l.flush();
            lines.push_back(readLine(l));
        }
    UpdateTotals(h, l, number);
    SeekVoucher(h, number);
    header = ReadHeader(h);
    return lines.empty() ? LoadResult::NoLines : LoadResult::Loaded;
}

long SaveLine(VoucherHeaders& h, VoucherLines& l, long number, const VoucherHeader& header,
              const VoucherLine& line, long iniBatchno, SaveKind* kind)
{
    SaveKind how;
    if (!SeekVoucher(h, number)) {
        number = NextVoucherNumber(h, iniBatchno);   // AppendNewEntry, numbered as the dialog showed
        h.appendStart();
        h.batchno.assignLong(number);
        h.mode.assign(header.mode.c_str());
        assignHeader(h, header);
        UpdateUserTime(h, szUser);
        h.append();
        how = SaveKind::NewVoucher;
    } else {
        assignHeader(h, header);   // ReplaceHeader, keeping the voucher's mode
        UpdateUserTime(h, szUser);
        h.flush();
        how = SaveKind::LineAppended;
    }

    bool replaced = false;
    if (how != SaveKind::NewVoucher && seekLines(l, number))
        for (; !l.eof() && toLong(l.batchno) == number; l.skip())
            if (!l.deleted() && toLong(l.journal) == line.line) {
                assignLine(l, line);   // ReplaceDetail
                l.flush();
                replaced = true;
                break;
            }
    if (!replaced) {
        l.appendStart();   // AppendDetail
        l.batchno.assignLong(number);
        assignLine(l, line);
        l.append();
    }
    if (replaced) how = SaveKind::LineReplaced;
    UpdateTotals(h, l, number);
    if (kind) *kind = how;
    return number;
}

bool DeleteLine(VoucherHeaders& h, VoucherLines& l, long number, int line)
{
    bool found = false;
    if (seekLines(l, number))
        for (; !l.eof() && toLong(l.batchno) == number; l.skip())
            if (!l.deleted() && toLong(l.journal) == line) {
                l.deleteRec();
                found = true;
            }
    if (!found) return false;
    l.pack();
    UpdateTotals(h, l, number);
    return true;
}

void DeleteVoucher(VoucherHeaders& h, VoucherLines& l, long number)
{
    if (SeekVoucher(h, number)) {
        h.deleteRec();
        h.pack();
    }
    bool any = false;
    if (seekLines(l, number))
        for (; !l.eof() && toLong(l.batchno) == number; l.skip()) {
            l.deleteRec();
            any = true;
        }
    if (any) l.pack();
}

std::vector<OpenPeriod> OpenPeriods(Code4& code)
{
    std::vector<OpenPeriod> out;
    Data4 pds(&code, "gl_pds");
    if (!pds.isValid()) return out;
    pds.SetIndex("period");
    Field4 period(pds, "period"), desc(pds, "pd_desc"), status(pds, "status");
    // Backwards from the last period, as GetCurrentPeriod (Company.cpp) walks.
    pds.bottom();
    while (!pds.eof() && !pds.bof()) {
        if (TrimText(status.str()) == "0")
            out.push_back({static_cast<int>(std::lround(period.num())), TrimText(desc.str())});
        if (pds.skip(-1) == r4bof) break;
    }
    pds.close();
    return out;
}
