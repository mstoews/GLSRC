//
// Data4.cpp - cursor over one table with CodeBase record-buffer semantics.
//
// Navigation is keyset-based: each move runs one query of the form
//
//   SELECT _rowid, _deleted, <fields...>, (<key>) FROM t
//    WHERE ((<key>), _rowid) > (?cur_key, ?cur_rowid) [AND filter]
//    ORDER BY (<key>), _rowid LIMIT 1 OFFSET n-1
//
// where <key> is the selected tag's key_expr, or _rowid in record order. The
// current record's key value is kept (key_) so the next move continues from
// it. Every query is backed by the (key_expr, _rowid) index made by createTag.
//

#include "Data4.h"
#include "Internal.h"

#include <cstdlib>
#include <cstring>

namespace cb4sqlite {

using detail::Stmt;

namespace {

std::string andWhere(std::string_view a, const std::string& filter)
{
    std::string w;
    if (!a.empty()) w += "(" + std::string(a) + ")";
    if (!filter.empty()) w += (w.empty() ? "" : " AND ") + std::string("(") + filter + ")";
    return w.empty() ? std::string() : " WHERE " + w;
}

} // namespace

// --- lifetime ------------------------------------------------------------------

Data4::Data4(Code4* code, const char* tableName)
    : code_(code)
{
    info_.name = normaliseTableName(tableName);
    if (!code_ || !code_->isOpen() || !loadTableInfo(code_->handle(), info_.name, info_)) {
        if (code_) code_->errorCode = e4open;
        code_ = nullptr;
        return;
    }
    valid_ = true;
    code_->attach(this);
    buf_.resize(info_.fields.size());
    setEof();          // opened but not positioned: like eof until the first move
}

Data4::~Data4()
{
    try {
        close();
    } catch (...) {
        // A destructor can't report a failed final write; close() already
        // detached us, so nothing dangles.
    }
    if (key_) sqlite3_value_free(key_);
}

int Data4::close()
{
    if (!valid_)
        return r4success;
    try {
        writeBuffer();
    } catch (...) {
        detachFromCode();
        throw;
    }
    detachFromCode();
    return r4success;
}

void Data4::detachFromCode() noexcept
{
    if (code_) code_->detach(this);
    code_  = nullptr;
    valid_ = false;
    dirty_ = appending_ = false;
    rowid_ = 0;
    eof_ = bof_ = true;
    if (key_) { sqlite3_value_free(key_); key_ = nullptr; }
    // Keep buf_ sized, so a Field4 read after close() returns blanks, not UB.
    for (size_t i = 0; i < buf_.size(); ++i)
        buf_[i] = detail::blankImage(info_.fields[i]);
}

bool Data4::requireValid() const noexcept
{
    return valid_ && code_ && code_->handle();
}

// --- helpers ------------------------------------------------------------------

std::string Data4::keyExpr(int tag) const
{
    if (tag < 0) return "_rowid";
    return "(" + info_.tags[static_cast<size_t>(tag)].keyExpr + ")";
}

std::string Data4::selectList(int tag) const
{
    std::string sql = "SELECT _rowid, _deleted";
    for (const FieldDef& f : info_.fields)
        sql += ", " + quoteIdent(f.name);
    return sql + ", " + keyExpr(tag) + " FROM " + quoteIdent(info_.name);
}

void Data4::loadRow(sqlite3_stmt* s)
{
    rowid_   = sqlite3_column_int64(s, 0);
    deleted_ = sqlite3_column_int(s, 1) != 0;
    for (size_t i = 0; i < info_.fields.size(); ++i)
        buf_[i] = detail::imageFromColumn(info_.fields[i], s, static_cast<int>(i) + 2);
    if (key_) sqlite3_value_free(key_);
    key_ = sqlite3_value_dup(sqlite3_column_value(s, static_cast<int>(info_.fields.size()) + 2));
    eof_ = bof_ = false;
    dirty_ = false;
}

void Data4::setEof()
{
    rowid_ = 0;
    deleted_ = false;
    if (key_) { sqlite3_value_free(key_); key_ = nullptr; }
    for (size_t i = 0; i < info_.fields.size(); ++i)
        buf_[i] = detail::blankImage(info_.fields[i]);
    eof_ = true;
    dirty_ = false;
}

void Data4::refreshKey()
{
    if (key_) { sqlite3_value_free(key_); key_ = nullptr; }
    if (rowid_ == 0) return;
    Stmt q(code_->handle(), "SELECT " + keyExpr(tag_) + " FROM " + quoteIdent(info_.name) +
                            " WHERE _rowid = ?1", "Data4::refreshKey");
    q.bindInt(1, rowid_);
    if (q.step("Data4::refreshKey"))
        key_ = sqlite3_value_dup(sqlite3_column_value(q.get(), 0));
}

void Data4::writeBuffer()
{
    // Moving (or closing) between appendStart() and append() abandons the
    // pending new record, as in CodeBase. Its buffered values belong to the
    // new record, so they must not be written over the current one.
    if (appending_) {
        appending_ = false;
        dirty_ = false;
        return;
    }
    if (!dirty_ || rowid_ == 0 || !requireValid()) {
        dirty_ = false;
        return;
    }
    static const char* M = "Data4::flush";
    std::string sql = "UPDATE " + quoteIdent(info_.name) + " SET ";
    for (size_t i = 0; i < info_.fields.size(); ++i)
        sql += (i ? ", " : "") + quoteIdent(info_.fields[i].name) + " = ?" + std::to_string(i + 1);
    sql += " WHERE _rowid = ?" + std::to_string(info_.fields.size() + 1);
    Stmt u(code_->handle(), sql, M);
    for (size_t i = 0; i < info_.fields.size(); ++i)
        detail::bindImage(u, static_cast<int>(i) + 1, info_.fields[i], buf_[i]);
    u.bindInt(static_cast<int>(info_.fields.size()) + 1, rowid_);
    u.step(M);
    dirty_ = false;
    refreshKey();      // the key fields may have changed
}

void Data4::setValue(int field, std::string v)
{
    buf_[static_cast<size_t>(field)] = std::move(v);
    dirty_ = true;
}

void Data4::trimValue(int field)
{
    std::string& s = buf_[static_cast<size_t>(field)];
    s.resize(detail::trimRight(s).size());
}

// --- navigation ---------------------------------------------------------------

int Data4::first(bool forward, const std::string& filter)
{
    if (!requireValid()) return e4data;
    writeBuffer();
    const char* dir = forward ? " ASC" : " DESC";
    Stmt q(code_->handle(), selectList(tag_) + andWhere({}, filter) + " ORDER BY " + keyExpr(tag_) +
                            dir + ", _rowid" + dir + " LIMIT 1", "Data4::top/bottom");
    if (!q.step("Data4::top/bottom")) {
        setEof();
        bof_ = true;
        return r4eof;
    }
    loadRow(q.get());
    return r4success;
}

int Data4::step(long n, const std::string& filter)
{
    if (!requireValid()) return e4data;
    writeBuffer();
    if (n == 0) {
        if (rowid_ == 0) return eof_ ? r4eof : r4success;
        return go(static_cast<long>(rowid_));
    }
    if (rowid_ == 0 || !key_) {
        // At eof (or never positioned): moving forward stays at eof; moving
        // back starts from the last record, as CodeBase does.
        if (n > 0) { setEof(); return r4eof; }
        const int rc = first(false, filter);
        if (rc != r4success || n == -1) return rc;
        return step(n + 1, filter);
    }
    const bool fwd = n > 0;
    const long count = fwd ? n : -n;
    const char* dir = fwd ? " ASC" : " DESC";
    Stmt q(code_->handle(),
           selectList(tag_) +
           andWhere("(" + keyExpr(tag_) + ", _rowid) " + (fwd ? ">" : "<") + " (?1, ?2)", filter) +
           " ORDER BY " + keyExpr(tag_) + dir + ", _rowid" + dir +
           " LIMIT 1 OFFSET " + std::to_string(count - 1),
           "Data4::skip");
    q.bindValue(1, key_);
    q.bindInt(2, rowid_);
    if (q.step("Data4::skip")) {
        loadRow(q.get());
        return r4success;
    }
    if (fwd) {
        setEof();
        return r4eof;
    }
    // Skipped back past the first record: CodeBase leaves you on the first
    // record with bof() set.
    const int rc = first(true, filter);
    bof_ = true;
    return rc == r4success ? r4bof : rc;
}

int Data4::top()            { return first(true, {}); }
int Data4::bottom()         { return first(false, {}); }
int Data4::skip(long n)     { return step(n, {}); }

int Data4::go(long recNo)
{
    if (!requireValid()) return e4data;
    writeBuffer();
    Stmt q(code_->handle(), selectList(tag_) + " WHERE _rowid = ?1", "Data4::go");
    q.bindInt(1, recNo);
    if (!q.step("Data4::go")) {
        setEof();
        return r4entry;
    }
    loadRow(q.get());
    return r4success;
}

int Data4::seek(const char* key)
{
    static const char* M = "Data4::seek";
    if (!requireValid()) return e4data;
    // CodeBase seeks on the selected tag, or the default (first) tag when
    // record order is selected.
    const int tag = tag_ >= 0 ? tag_ : (info_.tags.empty() ? -1 : 0);
    if (tag < 0)
        throw Error(M, "table '" + info_.name + "' has no tags to seek on");
    writeBuffer();
    const std::string k(key ? key : "");
    Stmt q(code_->handle(), selectList(tag) + " WHERE " + keyExpr(tag) + " >= ?1 ORDER BY " +
                            keyExpr(tag) + ", _rowid LIMIT 1", M);
    q.bindText(1, k);
    if (!q.step(M)) {
        setEof();
        return r4eof;
    }
    // Partial-key match: success when the found key starts with the search key.
    const int keyCol = static_cast<int>(info_.fields.size()) + 2;
    const auto* found = reinterpret_cast<const char*>(sqlite3_column_text(q.get(), keyCol));
    const int   flen  = sqlite3_column_bytes(q.get(), keyCol);
    const bool  match = found && static_cast<size_t>(flen) >= k.size() &&
                        std::memcmp(found, k.data(), k.size()) == 0;
    loadRow(q.get());
    if (tag != tag_) refreshKey();   // key_ must follow the *selected* order
    return match ? r4success : r4after;
}

int Data4::seek(double key)
{
    static const char* M = "Data4::seek(double)";
    if (!requireValid()) return e4data;
    const int tag = tag_ >= 0 ? tag_ : (info_.tags.empty() ? -1 : 0);
    if (tag < 0)
        throw Error(M, "table '" + info_.name + "' has no tags to seek on");
    writeBuffer();
    Stmt q(code_->handle(), selectList(tag) + " WHERE " + keyExpr(tag) + " >= ?1 ORDER BY " +
                            keyExpr(tag) + ", _rowid LIMIT 1", M);
    q.bindDouble(1, key);
    if (!q.step(M)) {
        setEof();
        return r4eof;
    }
    const bool match = sqlite3_column_double(q.get(), static_cast<int>(info_.fields.size()) + 2) == key;
    loadRow(q.get());
    if (tag != tag_) refreshKey();
    return match ? r4success : r4after;
}

long Data4::recNo()
{
    if (!requireValid()) return 0;
    if (rowid_ == 0) return recCount() + 1;
    return static_cast<long>(rowid_);
}

long Data4::recCount()
{
    if (!requireValid()) return 0;
    Stmt q(code_->handle(), "SELECT count(*) FROM " + quoteIdent(info_.name), "Data4::recCount");
    q.step("Data4::recCount");
    return static_cast<long>(sqlite3_column_int64(q.get(), 0));
}

// --- tags ---------------------------------------------------------------------

int Data4::SetIndex(const char* tagName)
{
    if (!requireValid()) return e4data;
    if (!tagName || !*tagName) {
        select();
        return r4success;
    }
    writeBuffer();
    int found = -1;
    for (size_t i = 0; i < info_.tags.size(); ++i)
        if (detail::iequals(info_.tags[i].name, tagName))
            found = static_cast<int>(i);
    tag_ = found;
    refreshKey();
    return found >= 0 ? r4success : e4tagName;
}

void Data4::select() noexcept
{
    tag_ = -1;
    try {
        if (requireValid()) { writeBuffer(); refreshKey(); }
    } catch (...) {
        // select() is void and noexcept in CodeBase; a failed write surfaces
        // on the next flush/navigation instead.
    }
}

// --- writing --------------------------------------------------------------------

int Data4::appendStart(int)
{
    if (!requireValid()) return e4data;
    writeBuffer();
    appending_ = true;
    dirty_ = false;        // buffer keeps its contents (CodeBase); blank() clears
    return r4success;
}

int Data4::append()
{
    static const char* M = "Data4::append";
    if (!requireValid()) return e4data;
    if (!appending_)
        throw Error(M, "append() without appendStart() on " + info_.name);
    std::string sql = "INSERT INTO " + quoteIdent(info_.name) + " (_deleted";
    std::string vals = "0";
    for (size_t i = 0; i < info_.fields.size(); ++i) {
        sql  += ", " + quoteIdent(info_.fields[i].name);
        vals += ", ?" + std::to_string(i + 1);
    }
    Stmt ins(code_->handle(), sql + ") VALUES (" + vals + ")", M);
    for (size_t i = 0; i < info_.fields.size(); ++i)
        detail::bindImage(ins, static_cast<int>(i) + 1, info_.fields[i], buf_[i]);
    ins.step(M);
    appending_ = false;
    dirty_ = false;
    return go(static_cast<long>(sqlite3_last_insert_rowid(code_->handle())));
}

int Data4::appendBlank()
{
    const int rc = appendStart();
    if (rc != r4success) return rc;
    blank();
    return append();
}

void Data4::blank()
{
    for (size_t i = 0; i < buf_.size(); ++i)
        buf_[i] = detail::blankImage(info_.fields[i]);
    dirty_ = true;
}

int Data4::flush()
{
    if (!requireValid()) return e4data;
    if (appending_) return r4success;   // nothing written until append()
    writeBuffer();
    return r4success;
}

void Data4::markDeleted(bool del)
{
    if (!requireValid() || rowid_ == 0) return;
    writeBuffer();
    Stmt u(code_->handle(), "UPDATE " + quoteIdent(info_.name) + " SET _deleted = ?1 WHERE _rowid = ?2",
           "Data4::deleteRec/recall");
    u.bindInt(1, del ? 1 : 0);
    u.bindInt(2, rowid_);
    u.step("Data4::deleteRec/recall");
    deleted_ = del;
}

void Data4::deleteRec() { markDeleted(true); }
void Data4::recall()    { markDeleted(false); }

int Data4::pack()
{
    if (!requireValid()) return e4data;
    writeBuffer();
    detail::exec(code_->handle(), "DELETE FROM " + quoteIdent(info_.name) + " WHERE _deleted <> 0",
                 "Data4::pack");
    return top() == r4success || eof_ ? r4success : e4data;
}

int Data4::zap(long firstRec, long lastRec)
{
    if (!requireValid()) return e4data;
    if (firstRec != 1 || lastRec != 2000000000L)
        notImplemented("Data4::zap", "record ranges");
    appending_ = false;
    dirty_ = false;
    detail::exec(code_->handle(), "DELETE FROM " + quoteIdent(info_.name), "Data4::zap");
    setEof();
    bof_ = true;
    return r4success;
}

int Data4::fieldNumber(const char* name) const noexcept
{
    if (!name) return 0;
    for (size_t i = 0; i < info_.fields.size(); ++i)
        if (detail::iequals(info_.fields[i].name, name))
            return static_cast<int>(i) + 1;
    return 0;
}

} // namespace cb4sqlite
