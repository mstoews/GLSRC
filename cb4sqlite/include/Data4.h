#pragma once
//
// Data4 - a cursor over one table, with CodeBase's record-buffer model
// (spec 4.2, 4.4). See cb4sqlite/README.md for the full semantics.
//
//  * The current record is held in memory as one fixed-width dBase text image
//    per field (what Field4::str() returns). Moving the cursor replaces it.
//  * Field4::assign*() edits that buffer. The change is written to SQLite by
//    flush(), or automatically before any navigation, appendStart() or close()
//    - exactly as CodeBase wrote a changed record when you moved off it.
//  * appendStart() keeps the current buffer contents (CodeBase behaviour; call
//    blank() to clear it); append() inserts the buffer as a new record and
//    positions on it.
//  * Navigation follows the selected tag's key, then _rowid. Without a tag
//    (or after an unknown SetIndex name) it is record order (_rowid).
//  * Deleted records stay visible, as in CodeBase (no SET DELETED). deleteRec()
//    marks, recall() unmarks, pack() removes them. Use a Relate4set with
//    ".NOT. DELETED()" to iterate without them.
//
// Return codes are CodeBase's: r4success, r4after, r4eof, r4bof, r4entry, or a
// negative e4* code. SQLite failures throw cb4sqlite::Error.
//
// A Data4 whose table is missing is !isValid(): navigation returns e4data and
// Field4s on it are invalid. Not copyable: it owns the cursor state.
//

#include "Code4.h"
#include "Schema.h"

#include <string>
#include <vector>

struct sqlite3_value;
struct sqlite3_stmt;

namespace cb4sqlite {

class Field4;
class Relate4set;

class Data4 {
public:
    Data4(Code4* code, const char* tableName);
    Data4(Code4& code, const char* tableName) : Data4(&code, tableName) {}
    virtual ~Data4();
    Data4(const Data4&) = delete;
    Data4& operator=(const Data4&) = delete;

    // --- navigation ---------------------------------------------------------
    int  top();
    int  bottom();
    int  skip(long n = 1);
    int  go(long recNo);                 // r4success, or r4entry if no such record
    int  seek(const char* key);          // partial (prefix) match on the tag key
    int  seek(double key);               // numeric tag keys
    bool eof() const noexcept { return eof_; }
    bool bof() const noexcept { return bof_; }
    long recNo();                        // _rowid; recCount() + 1 at eof
    long recCount();                     // all records, including deleted ones

    // --- tags ----------------------------------------------------------------
    // Selects a tag by name (case-insensitive). The current record is kept.
    // Unknown name: record order is selected and e4tagName returned.
    int  SetIndex(const char* tagName);
    void select() noexcept;              // record order

    // --- writing -------------------------------------------------------------
    int  appendStart(int memo = 0);
    int  append();
    int  appendBlank();
    void blank();
    int  flush();
    void deleteRec();
    void recall();
    bool deleted() const noexcept { return deleted_; }
    int  pack();
    int  zap(long firstRec = 1, long lastRec = 2000000000L);

    // --- misc ----------------------------------------------------------------
    int  numFields() const noexcept { return static_cast<int>(info_.fields.size()); }
    int  fieldNumber(const char* name) const noexcept;   // 1-based, 0 if none
    bool isValid() const noexcept { return valid_; }
    int  close();
    int  unlock() noexcept { return 0; }  // no locking in spec #1
    const std::string& tableName() const noexcept { return info_.name; }

private:
    friend class Field4;
    friend class Relate4set;
    friend class Code4;

    int  first(bool forward, const std::string& filter);
    int  step(long n, const std::string& filter);
    void loadRow(sqlite3_stmt* s);
    void setEof();
    void refreshKey();
    void writeBuffer();
    void markDeleted(bool del);
    bool requireValid() const noexcept;
    void detachFromCode() noexcept;
    std::string keyExpr(int tag) const;
    std::string selectList(int tag) const;

    // Field4 access to the record buffer.
    const std::string& value(int field) const { return buf_[static_cast<size_t>(field)]; }
    void setValue(int field, std::string v);
    void trimValue(int field);

    Code4*       code_  = nullptr;
    TableInfo    info_;
    bool         valid_ = false;
    int          tag_   = -1;          // index into info_.tags, -1 = record order

    std::vector<std::string> buf_;     // record buffer, one dBase image per field
    long long    rowid_    = 0;        // 0 when not positioned on a record
    sqlite3_value* key_    = nullptr;  // current record's tag key (owned)
    bool         eof_      = true;
    bool         bof_      = true;
    bool         deleted_  = false;
    bool         dirty_    = false;
    bool         appending_ = false;
};

} // namespace cb4sqlite

using cb4sqlite::Data4;
