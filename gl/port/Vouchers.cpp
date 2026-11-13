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
// Post / Recover / Cancel (corrections are spec D12):
//   - PostGLSlip and PostAdjustment compared the header's debit and credit
//     text with ==; the port compares the lines' totals to the cent (spec
//     R3), so a stale header total can't post an unbalanced voucher.
//   - A line whose account wasn't found got message 3081, but its curr_bal
//     change went to whatever glaccts record was current, and the line was
//     still marked posted and copied to gl_dist. PostAdjustment and Recover
//     skipped such lines silently. The port refuses the whole voucher.
//   - PostAdjustment skipped an out-of-balance adjustment without a word;
//     the port refuses it with 8116, as an open voucher.
//   - Recover left the lines' mode "0" on a reopened voucher, and deleted
//     every gl_dist row with the voucher's number, AR and AP rows included;
//     the port blanks the lines' mode and deletes only the GL rows.
//   - Each operation is one transaction (D1); the original wrote row by row.
//
#include "Vouchers.h"

#include "Accounts.h"   // TrimText, AccountsTable, SeekAccount
#include "Ap.h"         // UndoApTransfer: cancelling an AP transfer voucher (spec #5 4.4)
#include "Ar.h"         // UndoArTransfer: cancelling an AR transfer voucher (spec #6 4.4)
#include "Assets.h"     // UndoRun: cancelling a depreciation voucher (spec #4 4.3)
#include "Company.h"    // UpdateUserTime
#include "db_gl_dist.h"
#include "db_gl_distb.h"   // year end's archive (spec #3 D16)

#include <algorithm>
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

bool IsPostedMode(const std::string& mode)
{
    return mode == voucher_mode::kPosted || mode == voucher_mode::kPostedAdjustment ||
           mode == voucher_mode::kClosedYear;
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
    if (IsPostedMode(header.mode)) return LoadResult::Posted;
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

// ---- Post, Recover, Cancel ------------------------------------------------

namespace {

int gPostFault = 0;   // SetPostFault

long long toCents(double v) { return std::llround(v * 100.0); }
double fromCents(long long c) { return static_cast<double>(c) / 100.0; }

void must(int rc, const char* what)
{
    if (rc < 0) throw std::runtime_error(std::string("gl: ") + what + " failed");
}

// The tables one operation works on, opened for it alone.
struct Books {
    explicit Books(Code4& c) : code(c), h(c), l(c), gl(c), dist(&c) {}
    Code4& code;
    VoucherHeaders h;
    VoucherLines l;
    AccountsTable gl;
    Tgldist dist;

    bool valid() const { return h.isValid() && l.isValid() && gl.isValid() && dist.isValid(); }
};

Field4* periodField(Tglaccts& gl, int period)
{
    Field4* const f[] = {gl.period1, gl.period2, gl.period3, gl.period4,  gl.period5,  gl.period6,
                         gl.period7, gl.period8, gl.period9, gl.period10, gl.period11, gl.period12};
    return period >= 1 && period <= 12 ? f[period - 1] : nullptr;
}

// Adds `delta` cents to a numeric field, to the cent.
void addCents(Field4& f, long long delta) { f.assignDouble(fromCents(toCents(f.num()) + delta)); }

// The record numbers of voucher `number`'s lines.
std::vector<long> lineRecords(VoucherLines& l, long number)
{
    std::vector<long> recs;
    if (seekLines(l, number))
        for (; !l.eof() && toLong(l.batchno) == number; l.skip())
            if (!l.deleted()) recs.push_back(l.recNo());
    return recs;
}

// Checks the voucher's lines: balanced to the cent and every account+dept
// in glaccts. Fills `why` and returns false otherwise.
bool checkLines(Books& b, const std::vector<long>& recs, long number, VoucherRefusal& why)
{
    long long debit = 0, credit = 0;
    for (long rec : recs) {
        b.l.go(rec);
        debit += toCents(b.l.debit.num());
        credit += toCents(b.l.credit.num());
    }
    why = {};
    why.number = number;
    if (debit != credit) {
        why.reason = PostRefusal::OutOfBalance;
        return false;
    }
    for (long rec : recs) {
        b.l.go(rec);
        const std::string account = TrimText(b.l.account.str()), dept = TrimText(b.l.dept.str());
        if (!SeekAccount(b.gl, account, dept)) {
            why.reason = PostRefusal::UnknownAccount;
            why.account = account;
            why.dept = dept;
            return false;
        }
    }
    return true;
}

void countLine(int& written)
{
    ++written;
    if (gPostFault > 0 && written == gPostFault) throw PostFault(written);
}

// PostGLSlip / PostAdjustment for one voucher, inside the caller's transaction.
void postOne(Books& b, long number, PostResult& result, int& written)
{
    if (!SeekVoucher(b.h, number)) return;
    const std::string mode = TrimText(b.h.mode.str());
    const bool adjustment = mode == voucher_mode::kAdjustment;
    if (mode != voucher_mode::kOpen && !adjustment) return;
    const std::vector<long> recs = lineRecords(b.l, number);
    if (recs.empty()) return;   // the original's loop never ran: the voucher stayed open
    VoucherRefusal why;
    if (!checkLines(b, recs, number, why)) {
        result.refused.push_back(why);
        return;
    }
    for (long rec : recs) {
        must(b.l.go(rec), "reading a voucher line");
        const long long amount = toCents(b.l.debit.num()) - toCents(b.l.credit.num());
        SeekAccount(b.gl, TrimText(b.l.account.str()), TrimText(b.l.dept.str()));   // checked above
        if (!adjustment) {
            // UpdateGL: period<N> (no column for a period outside 1-12, as before), then curr_bal.
            if (Field4* f = periodField(b.gl, std::atoi(TrimText(b.l.period.str()).c_str()))) addCents(*f, amount);
            addCents(*b.gl.curr_bal, amount);
        } else {
            addCents(*b.gl.openbal, amount);
            long long bal = toCents(b.gl.openbal->num());
            for (int p = 1; p <= 12; ++p) bal += toCents(periodField(b.gl, p)->num());
            b.gl.curr_bal->assignDouble(fromCents(bal));
        }
        UpdateUserTime(b.gl, szUser);
        must(b.gl.flush(), "updating glaccts");
        if (!adjustment) {
            b.l.mode.assign(voucher_mode::kPosted);
            must(b.l.flush(), "updating gl_btchd");
            Tgldist& d = b.dist;
            must(d.appendStart(), "adding to gl_dist");
            d.account->assign(b.l.account.str());
            d.dept->assign(b.l.dept.str());
            d.period->assign(b.l.period.str());
            d.date->assign(b.l.date.str());
            d.batchno->assignLong(number);
            d.reference->assign(b.l.reference.str());
            d.cheque->assign(b.l.cheque.str());
            d.medrep->assign(b.l.medrep.str());
            d.prod->assign(b.l.prod.str());
            d.drcode->assign(b.l.drcode.str());
            d.descript->assign(b.l.descript.str());
            d.debit->assignDouble(fromCents(toCents(b.l.debit.num())));
            d.credit->assignDouble(fromCents(toCents(b.l.credit.num())));
            UpdateUserTime(d, szUser);
            must(d.append(), "adding to gl_dist");
        }
        countLine(written);
    }
    if (!SeekVoucher(b.h, number)) throw std::runtime_error("gl: voucher header lost while posting");
    b.h.mode.assign(adjustment ? voucher_mode::kPostedAdjustment : voucher_mode::kPosted);
    UpdateUserTime(b.h, szUser);
    must(b.h.flush(), "updating gl_btch");
    result.posted.push_back(number);
}

PostResult post(Code4& code, const std::vector<long>* only)
{
    PostResult result;
    Books b(code);
    if (!b.valid()) throw std::runtime_error("gl: a voucher table is missing");
    std::vector<long> numbers;
    if (only) {
        numbers = *only;
    } else {
        for (const VoucherHeader& v : ListVouchers(b.h, {voucher_mode::kOpen, voucher_mode::kAdjustment}))
            numbers.push_back(v.number);
    }
    cb4sqlite::Transaction tx(code);
    int written = 0;
    for (long n : numbers) postOne(b, n, result, written);
    tx.commit();
    return result;
}

bool isGlDistRow(const Tgldist& d)
{
    const std::string type = TrimText(d.type->str());
    return type != "AR" && type != "AP";
}

} // namespace

PostFault::PostFault(int line)
    : std::runtime_error("Posting stopped by /failpost after line " + std::to_string(line) + ".")
{
}

void SetPostFault(int nthLine) { gPostFault = nthLine > 0 ? nthLine : 0; }

std::vector<VoucherHeader> ListVouchers(VoucherHeaders& h, const std::vector<std::string>& modes)
{
    std::vector<VoucherHeader> out;
    if (!h.isValid()) return out;
    h.SetIndex("batchno");
    for (h.top(); !h.eof(); h.skip()) {
        if (h.deleted()) continue;
        const std::string mode = TrimText(h.mode.str());
        if (std::find(modes.begin(), modes.end(), mode) != modes.end()) out.push_back(ReadHeader(h));
    }
    return out;
}

PostResult PostAll(Code4& code) { return post(code, nullptr); }

PostResult PostVoucher(Code4& code, long number)
{
    const std::vector<long> one{number};
    return post(code, &one);
}

RecoverResult RecoverVoucher(Code4& code, long number, std::string* account, std::string* dept)
{
    Books b(code);
    if (!b.valid()) throw std::runtime_error("gl: a voucher table is missing");
    if (!SeekVoucher(b.h, number) || TrimText(b.h.mode.str()) != voucher_mode::kPosted)
        return RecoverResult::NotPosted;
    const std::vector<long> recs = lineRecords(b.l, number);
    for (long rec : recs) {
        b.l.go(rec);
        const std::string a = TrimText(b.l.account.str()), d = TrimText(b.l.dept.str());
        if (!SeekAccount(b.gl, a, d)) {
            if (account) *account = a;
            if (dept) *dept = d;
            return RecoverResult::UnknownAccount;
        }
    }

    cb4sqlite::Transaction tx(code);
    for (long rec : recs) {   // SetPeriodAmount
        must(b.l.go(rec), "reading a voucher line");
        const long long amount = toCents(b.l.debit.num()) - toCents(b.l.credit.num());
        SeekAccount(b.gl, TrimText(b.l.account.str()), TrimText(b.l.dept.str()));
        if (Field4* f = periodField(b.gl, std::atoi(TrimText(b.l.period.str()).c_str()))) addCents(*f, -amount);
        addCents(*b.gl.curr_bal, -amount);
        UpdateUserTime(b.gl, szUser);
        must(b.gl.flush(), "updating glaccts");
        b.l.mode.assign("");
        must(b.l.flush(), "updating gl_btchd");
    }
    if (!SeekVoucher(b.h, number)) throw std::runtime_error("gl: voucher header lost while recovering");
    b.h.mode.assign(voucher_mode::kOpen);
    UpdateUserTime(b.h, szUser);
    must(b.h.flush(), "updating gl_btch");

    bool deleted = false;
    b.dist.SetIndex("batchno");
    if (b.dist.seek(static_cast<double>(number)) == r4success)
        for (; !b.dist.eof() && std::lround(b.dist.batchno->num()) == number; b.dist.skip())
            if (!b.dist.deleted() && isGlDistRow(b.dist)) {
                b.dist.deleteRec();
                deleted = true;
            }
    if (deleted) must(b.dist.pack(), "packing gl_dist");
    tx.commit();
    return RecoverResult::Recovered;
}

CancelResult CancelVoucher(Code4& code, long number)
{
    VoucherHeaders h(code);
    VoucherLines l(code);
    if (!h.isValid() || !l.isValid()) throw std::runtime_error("gl: a voucher table is missing");
    if (!SeekVoucher(h, number)) return CancelResult::NotFound;
    if (IsPostedMode(TrimText(h.mode.str()))) return CancelResult::Posted;
    cb4sqlite::Transaction tx(code);
    DeleteVoucher(h, l, number);
    UndoRun(code, number);   // a depreciation voucher: its assets go back (spec #4 4.3); otherwise nothing
    UndoApTransfer(code, number);   // an AP transfer voucher: its rows wait again (spec #5 4.4); otherwise nothing
    UndoArTransfer(code, number);   // an AR transfer voucher: the same (spec #6 4.4)
    tx.commit();
    return CancelResult::Cancelled;
}

// ---- Transaction Inquiry --------------------------------------------------

AccountActivity AccountTransactions(Code4& code, const std::string& account, const std::string& dept)
{
    AccountActivity out;
    const std::string a = TrimText(account), d = TrimText(dept);
    {
        AccountsTable gl(code);
        if (!gl.isValid() || !SeekAccount(gl, a, d)) return out;
        out.found = true;
        out.descript = TrimText(gl.descript->str());
        out.openBal = gl.openbal->num();
    }
    VoucherLines l(code);
    if (!l.isValid()) return out;
    // glbtchd->SetIndex("account"); without that tag, every line in batchno order.
    const bool tagged = l.SetIndex("account") == r4success;
    if (!tagged) l.SetIndex("batchno");
    long long debit = 0, credit = 0;
    for (tagged ? l.seek(a.c_str()) : l.top(); !l.eof(); l.skip()) {
        const std::string la = TrimText(l.account.str());
        if (tagged && la != a) break;
        if (l.deleted() || la != a || TrimText(l.dept.str()) != d || TrimText(l.mode.str()) != voucher_mode::kPosted)
            continue;
        PostedLine p;
        p.period = std::atoi(TrimText(l.period.str()).c_str());
        p.number = toLong(l.batchno);
        p.descript = TrimText(l.descript.str());
        p.debit = l.debit.num();
        p.credit = l.credit.num();
        debit += toCents(p.debit);
        credit += toCents(p.credit);
        out.lines.push_back(p);
    }
    out.debit = fromCents(debit);
    out.credit = fromCents(credit);
    out.balance = fromCents(toCents(out.openBal) + debit - credit);
    return out;
}

// ---- Year End -------------------------------------------------------------

namespace {

// D16 (spec #3): the year's posted lines move from gl_dist to gl_distb, so
// the new year's gl_dist starts empty and a period number means one year.
// Rows already deleted in gl_dist are dropped. gl_distb has no medrep,
// drcode or prod, and a 40-character description (gl_dist's is 50).
int archiveDistribution(Code4& code)
{
    Tgldist d(&code);
    if (!d.isValid()) return 0;
    Tgldistb b(&code);
    if (!b.isValid()) throw std::runtime_error("gl: the gl_distb table is missing");
    int moved = 0;
    for (d.top(); !d.eof(); d.skip()) {
        if (d.deleted()) continue;
        must(b.appendStart(), "adding to gl_distb");
        b.account->assign(d.account->str());
        b.dept->assign(d.dept->str());
        b.period->assign(d.period->str());
        b.date->assign(d.date->str());
        b.type->assign(d.type->str());
        b.batchno->assignLong(toLong(*d.batchno));
        b.reference->assign(d.reference->str());
        b.cheque->assign(d.cheque->str());
        b.descript->assign(d.descript->str());
        b.debit->assignDouble(fromCents(toCents(d.debit->num())));
        b.credit->assignDouble(fromCents(toCents(d.credit->num())));
        must(b.append(), "adding to gl_distb");
        d.deleteRec();
        ++moved;
    }
    must(d.pack(), "packing gl_dist");
    return moved;
}

// With D16: the year's posted vouchers become kClosedYear, headers ("0" and
// "2") and posted lines ("0"), so Transaction Inquiry (opening balance plus
// the posted lines) doesn't count them twice and Recover can't reverse one
// into the new year. Returns the vouchers closed.
int closeYearVouchers(Code4& code)
{
    int closed = 0;
    VoucherHeaders h(code);
    if (h.isValid()) {
        for (h.top(); !h.eof(); h.skip()) {
            const std::string mode = TrimText(h.mode.str());
            if (h.deleted() || (mode != voucher_mode::kPosted && mode != voucher_mode::kPostedAdjustment)) continue;
            h.mode.assign(voucher_mode::kClosedYear);
            UpdateUserTime(h, szUser);
            must(h.flush(), "updating gl_btch");
            ++closed;
        }
    }
    VoucherLines l(code);
    if (l.isValid()) {
        l.SetIndex("batchno");
        for (l.top(); !l.eof(); l.skip()) {
            if (l.deleted() || TrimText(l.mode.str()) != voucher_mode::kPosted) continue;
            l.mode.assign(voucher_mode::kClosedYear);
            must(l.flush(), "updating gl_btchd");
        }
    }
    return closed;
}

} // namespace

YearEndResult YearEnd(Code4& code)
{
    YearEndResult r;
    AccountsTable gl(code);
    if (!gl.isValid()) throw std::runtime_error("gl: the glaccts table is missing");
    gl.SetIndex("accounts");
    for (gl.top(); !gl.eof(); gl.skip())
        if (!gl.deleted() && TrimText(gl.type->str()) == "R")
            r.retained.push_back({TrimText(gl.accounts->str()), TrimText(gl.dept->str())});
    if (r.retained.size() != 1) return r;

    cb4sqlite::Transaction tx(code);
    long long total = 0;   // cents
    for (gl.top(); !gl.eof(); gl.skip()) {
        if (gl.deleted()) continue;
        if (TrimText(gl.type->str()) != "B") {
            total += toCents(gl.curr_bal->num());
            gl.curr_bal->assignDouble(0);
        }
        for (int p = 1; p <= 12; ++p) {
            Field4* period = periodField(gl, p);
            Field4* const previous[] = {gl.previous1, gl.previous2, gl.previous3, gl.previous4,
                                        gl.previous5, gl.previous6, gl.previous7, gl.previous8,
                                        gl.previous9, gl.previous10, gl.previous11, gl.previous12};
            previous[p - 1]->assignDouble(fromCents(toCents(period->num())));
            period->assignDouble(0);
        }
        UpdateUserTime(gl, szUser);
        must(gl.flush(), "updating glaccts");
    }
    if (!SeekAccount(gl, r.retained[0].account, r.retained[0].dept))
        throw std::runtime_error("gl: the Retained Earnings account disappeared during year end");
    addCents(*gl.curr_bal, total);
    UpdateUserTime(gl, szUser);
    must(gl.flush(), "updating glaccts");

    // D16 (spec #3): the new year opens at the closing balances, so
    // CURR_BAL = OPENBAL + the (now zero) periods holds again.
    for (gl.top(); !gl.eof(); gl.skip()) {
        if (gl.deleted()) continue;
        gl.openbal->assignDouble(fromCents(toCents(gl.curr_bal->num())));
        must(gl.flush(), "updating glaccts");
    }
    r.archived = archiveDistribution(code);
    r.closedVouchers = closeYearVouchers(code);

    tx.commit();
    r.closed = true;
    r.transferred = fromCents(total);
    return r;
}
