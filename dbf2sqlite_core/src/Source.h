#pragma once
//
// Source.h - readers for the dBase IV input files. Internal to dbf2sqlite_core.
//

#include <filesystem>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace dbf2sqlite::detail {

// Thrown for unreadable or malformed input (exit code 2).
struct SourceError : std::runtime_error {
    using std::runtime_error::runtime_error;
};

struct DbfField {
    std::string name;      // as stored, upper-case
    char        type = 'C';
    int         len  = 0;
    int         dec  = 0;
    int         offset = 0;   // byte offset in the raw record (after the deletion flag)
};

// A .dbf (any extension: ACCESS.PSW is a dBase file too), read via shapelib.
class DbfFile {
public:
    explicit DbfFile(const std::filesystem::path& path);
    ~DbfFile();
    DbfFile(const DbfFile&) = delete;
    DbfFile& operator=(const DbfFile&) = delete;

    const std::vector<DbfField>& fields() const noexcept { return fields_; }
    int  recordCount() const noexcept { return records_; }
    int  languageDriver() const noexcept { return ldid_; }   // header byte 29

    // Raw bytes of record i (0-based), without the deletion flag. Valid until
    // the next call.
    std::string_view record(int i);
    bool deleted(int i);

private:
    void* h_   = nullptr;     // DBFHandle
    void* ctx_ = nullptr;     // shapelib open-hook context (see DbfFile.cpp)
    std::vector<DbfField> fields_;
    int records_ = 0;
    int ldid_ = 0;
    int recLen_ = 0;
};

// True if the file looks like a dBase table: known version byte and header /
// record sizes consistent with the file length.
bool looksLikeDbf(const std::filesystem::path& path);

struct MdxTag {
    std::string name;          // tag name as stored (upper-case)
    std::string expression;    // dBase key expression, e.g. "ACCOUNT+DEPT"
    std::string filter;        // FOR clause, empty if none
    char        keyType = 'C'; // C, N or D
    bool        unique = false;
    bool        descending = false;
};

// Reads the tag definitions of a dBase IV production index. Layout from the
// CodeBase S4MDX structures in include/d4data.h (I4HEADER, T4DESC, T4HEADER):
//   file header : slot size at 26 (short, 32), num_tags at 28 (short)
//   tag slots   : 32 bytes each from offset 544 (512 + 32): +0 header block
//                 (512-byte units, long), +4 tag name [10], +20 key type
//   tag header  : +8 type_code (0x08 descending, 0x40 unique), +9 key type,
//                 +22 unique (0x4000), +24 key expression [220],
//                 +0x2FA FOR expression [220]
// Every slot is validated; anything inconsistent raises SourceError.
std::vector<MdxTag> readMdxTags(const std::filesystem::path& path);

// --- code pages -----------------------------------------------------------
int  codePageFromLdid(int ldid) noexcept;          // 0 if unknown / not set
// Converts bytes in `codePage` to UTF-8. ASCII passes through everywhere;
// 1252, 28591 and 437 are built in; any other code page needs Windows.
// Throws SourceError if the conversion isn't available.
std::string toUtf8(std::string_view bytes, int codePage);

// --- key expressions --------------------------------------------------------
struct KeyField { std::string name; char type; };
struct KeySql {
    std::string sql;           // SQL key expression, empty on failure
    char        type = 'C';    // result type: C, N or D
    std::string error;         // why translation failed
};
// dBase index expression -> cb4sqlite key_expr. Supported: field names,
// `+` between character values, UPPER(c), DTOS(d), STR(n[,len[,dec]]).
KeySql translateKey(const std::string& expr, const std::vector<KeyField>& fields);

std::string pathUtf8(const std::filesystem::path& p);

} // namespace dbf2sqlite::detail
