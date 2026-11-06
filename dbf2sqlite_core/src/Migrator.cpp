//
// Migrator.cpp - dBase company directory -> cb4sqlite database (spec 4.5).
//

#include "Migrator.h"
#include "Source.h"

#include "cb4sqlite.h"
#include "../../cb4sqlite/sqlite3.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <map>
#include <memory>
#include <set>

namespace dbf2sqlite {

using namespace detail;
namespace fs = std::filesystem;

namespace {

// Failures that stop the whole run; the exit code says which side failed.
struct Fatal {
    ExitCode code;
    std::string message;
};

std::string lower(std::string s)
{
    for (char& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return s;
}

std::string_view trimRight(std::string_view s)
{
    while (!s.empty() && (s.back() == ' ' || s.back() == '\0')) s.remove_suffix(1);
    return s;
}

std::string_view trim(std::string_view s)
{
    s = trimRight(s);
    while (!s.empty() && s.front() == ' ') s.remove_prefix(1);
    return s;
}

bool isIndexOrSidecar(const std::string& extLower)
{
    static const std::set<std::string> skip = {".mdx", ".ndx", ".cdx", ".ntx", ".dbt", ".fpt",
                                               ".cpg", ".tmp", ".bak", ".db", ".db-journal"};
    return skip.count(extLower) != 0;
}

struct SourceFile {
    fs::path    path;
    std::string table;
};

// Files in srcDir by lower-case name (dBase names are case-insensitive).
std::map<std::string, fs::path> listDir(const fs::path& dir)
{
    std::error_code ec;
    if (!fs::is_directory(dir, ec))
        throw Fatal{ExitCode::SourceRead, "source directory not found: " + pathUtf8(dir)};
    std::map<std::string, fs::path> files;
    for (const auto& e : fs::directory_iterator(dir, ec))
        if (e.is_regular_file())
            files[lower(pathUtf8(e.path().filename()))] = e.path();
    if (ec)
        throw Fatal{ExitCode::SourceRead, "cannot list " + pathUtf8(dir) + ": " + ec.message()};
    return files;
}

std::vector<SourceFile> findSources(const Options& opt, const std::map<std::string, fs::path>& files,
                                    Result& r)
{
    std::vector<SourceFile> out;
    if (opt.tables.empty()) {
        for (const auto& [name, path] : files) {
            const std::string ext = lower(pathUtf8(path.extension()));
            if (isIndexOrSidecar(ext)) continue;
            if (ext == ".dbf" || looksLikeDbf(path)) {
                if (ext != ".dbf")
                    r.log.push_back("[info] " + pathUtf8(path.filename()) +
                                    ": dBase table without a .dbf extension");
                out.push_back({path, cb4sqlite::normaliseTableName(pathUtf8(path.filename()).c_str())});
            }
        }
    } else {
        for (const std::string& t : opt.tables) {
            const std::string key = lower(t);
            auto it = files.find(key);
            if (it == files.end()) it = files.find(key + ".dbf");
            if (it == files.end())
                throw Fatal{ExitCode::SourceRead, "no dBase file for table '" + t + "' in " + pathUtf8(opt.srcDir)};
            out.push_back({it->second, cb4sqlite::normaliseTableName(pathUtf8(it->first).c_str())});
        }
    }
    std::sort(out.begin(), out.end(), [](const SourceFile& a, const SourceFile& b) { return a.table < b.table; });
    if (out.empty())
        throw Fatal{ExitCode::SourceRead, "no dBase tables found in " + pathUtf8(opt.srcDir)};
    return out;
}

// The production index shares the table's base name: GLACCTS.MDX, ACCESS.MDX.
fs::path findMdx(const fs::path& dbf, const std::map<std::string, fs::path>& files)
{
    const auto it = files.find(lower(pathUtf8(dbf.stem())) + ".mdx");
    return it == files.end() ? fs::path() : it->second;
}

class Sql {
public:
    Sql(sqlite3* db, const std::string& sql) : db_(db)
    {
        if (sqlite3_prepare_v2(db, sql.c_str(), -1, &s_, nullptr) != SQLITE_OK)
            throw Fatal{ExitCode::DestWrite, std::string(sqlite3_errmsg(db)) + " [" + sql + "]"};
    }
    ~Sql() { sqlite3_finalize(s_); }
    sqlite3_stmt* get() const { return s_; }
    bool step()
    {
        const int rc = sqlite3_step(s_);
        if (rc == SQLITE_ROW) return true;
        if (rc == SQLITE_DONE) return false;
        throw Fatal{ExitCode::DestWrite, sqlite3_errmsg(db_)};
    }
private:
    sqlite3* db_;
    sqlite3_stmt* s_ = nullptr;
};

void exec(sqlite3* db, const std::string& sql)
{
    char* err = nullptr;
    if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &err) != SQLITE_OK) {
        const std::string msg = err ? err : "unknown error";
        sqlite3_free(err);
        throw Fatal{ExitCode::DestWrite, msg + " [" + sql + "]"};
    }
}

struct Column {
    DbfField src;
    cb4sqlite::FieldDef def;
    int badValues = 0;
};

// Migrates one table. Returns false if it hit untranslatable schema or index
// definitions (exit 4); fatal problems throw.
bool migrateTable(Code4& code, const SourceFile& sf, const std::map<std::string, fs::path>& files,
                  const Options& opt, Result& r)
{
    const std::string T = sf.table;
    const std::string file = pathUtf8(sf.path.filename());
    TableResult tr;
    tr.table = T;
    tr.sourceFile = file;
    bool ok = true;
    auto problem = [&](const std::string& what, const std::string& skipped) {
        if (opt.force) {
            r.log.push_back("[warn] " + T + ": " + what + " - " + skipped + " (--force)");
        } else {
            r.log.push_back("[error] " + T + ": " + what);
            ok = false;
        }
    };

    std::unique_ptr<DbfFile> dbf;
    try {
        dbf = std::make_unique<DbfFile>(sf.path);
    } catch (const SourceError& e) {
        throw Fatal{ExitCode::SourceRead, e.what()};
    }

    int cp = opt.codePage ? opt.codePage : codePageFromLdid(dbf->languageDriver());
    if (cp == 0) cp = 1252;

    // --- columns -------------------------------------------------------------
    std::vector<Column> cols;
    for (const DbfField& f : dbf->fields()) {
        if (f.type == 'C' || f.type == 'N' || f.type == 'F' || f.type == 'D' || f.type == 'L') {
            cols.push_back({f, {f.name, f.type, f.len, f.dec}, 0});
        } else {
            problem("field " + f.name + " has type '" + std::string(1, f.type) +
                        (f.type == 'M' ? "' (memo, out of scope for spec #1)" : "' (unsupported)"),
                    "field dropped");
        }
    }
    if (cols.empty()) {
        problem("no migratable fields", "table skipped");
        return ok;
    }

    // --- pass 1: widths after conversion to UTF-8 -------------------------------
    try {
        for (int i = 0; i < dbf->recordCount(); ++i) {
            const std::string_view rec = dbf->record(i);
            for (Column& c : cols) {
                if (c.def.type != 'C') continue;
                const std::string u = toUtf8(trimRight(rec.substr(static_cast<size_t>(c.src.offset),
                                                                  static_cast<size_t>(c.src.len))), cp);
                if (static_cast<int>(u.size()) > c.def.len) c.def.len = static_cast<int>(u.size());
            }
        }
    } catch (const SourceError& e) {
        throw Fatal{ExitCode::SourceRead, file + ": " + e.what()};
    }
    for (const Column& c : cols)
        if (c.def.len != c.src.len)
            r.log.push_back("[warn] " + T + "." + c.src.name + ": widened from " + std::to_string(c.src.len) +
                            " to " + std::to_string(c.def.len) + " bytes (text grew when converted from code page " +
                            std::to_string(cp) + " to UTF-8)");

    if (!ok && !opt.force)
        return false;          // don't build half a table

    std::vector<cb4sqlite::FieldDef> defs;
    for (const Column& c : cols) defs.push_back(c.def);
    try {
        cb4sqlite::createTable(code, T.c_str(), defs);
    } catch (const cb4sqlite::Error& e) {
        throw Fatal{ExitCode::DestWrite, e.what()};
    }

    // --- pass 2: rows, in record order, _rowid = dBase record number ------------
    sqlite3* db = code.handle();
    std::string sql = "INSERT INTO " + cb4sqlite::quoteIdent(T) + " (_rowid, _deleted";
    std::string vals = "?1, ?2";
    for (size_t i = 0; i < cols.size(); ++i) {
        sql += ", " + cb4sqlite::quoteIdent(cols[i].def.name);
        vals += ", ?" + std::to_string(i + 3);
    }
    exec(db, "BEGIN");
    try {
        Sql ins(db, sql + ") VALUES (" + vals + ")");
        for (int i = 0; i < dbf->recordCount(); ++i) {
            const std::string_view rec = dbf->record(i);
            sqlite3_stmt* s = ins.get();
            sqlite3_reset(s);
            sqlite3_bind_int64(s, 1, i + 1);
            const bool del = dbf->deleted(i);
            sqlite3_bind_int(s, 2, del ? 1 : 0);
            if (del) ++tr.deletedRows;
            for (size_t k = 0; k < cols.size(); ++k) {
                Column& c = cols[k];
                const int p = static_cast<int>(k) + 3;
                const std::string_view raw = rec.substr(static_cast<size_t>(c.src.offset), static_cast<size_t>(c.src.len));
                const auto bad = [&](const char* kind) {
                    if (++c.badValues <= 5)
                        r.log.push_back("[warn] " + T + "." + c.src.name + " record " + std::to_string(i + 1) +
                                        ": " + kind + " '" + std::string(trim(raw)) + "' stored as empty");
                    sqlite3_bind_null(s, p);
                };
                switch (c.def.type) {
                    case 'C': {
                        std::string u = toUtf8(trimRight(raw), cp);
                        u.resize(static_cast<size_t>(c.def.len), ' ');
                        sqlite3_bind_text(s, p, u.data(), static_cast<int>(u.size()), SQLITE_TRANSIENT);
                        break;
                    }
                    case 'N':
                    case 'F': {
                        const std::string t(trim(raw));
                        if (t.empty()) { sqlite3_bind_null(s, p); break; }
                        char* end = nullptr;
                        const double v = std::strtod(t.c_str(), &end);
                        if (end != t.c_str() + t.size() || !std::isfinite(v)) { bad("not a number"); break; }
                        if (c.def.dec == 0 && std::fabs(v) < 9.0e15 && v == std::floor(v))
                            sqlite3_bind_int64(s, p, static_cast<sqlite3_int64>(v));
                        else
                            sqlite3_bind_double(s, p, v);
                        break;
                    }
                    case 'D': {
                        const std::string_view t = trim(raw);
                        if (t.empty()) { sqlite3_bind_null(s, p); break; }
                        if (t.size() != 8 || !std::all_of(t.begin(), t.end(), [](char ch) { return ch >= '0' && ch <= '9'; })) {
                            bad("not a YYYYMMDD date");
                            break;
                        }
                        sqlite3_bind_text(s, p, t.data(), 8, SQLITE_TRANSIENT);
                        break;
                    }
                    case 'L': {
                        const char ch = raw.empty() ? ' ' : raw[0];
                        if (ch == 'T' || ch == 't' || ch == 'Y' || ch == 'y') sqlite3_bind_int(s, p, 1);
                        else if (ch == 'F' || ch == 'f' || ch == 'N' || ch == 'n') sqlite3_bind_int(s, p, 0);
                        else sqlite3_bind_null(s, p);
                        break;
                    }
                    default:
                        sqlite3_bind_null(s, p);
                }
            }
            ins.step();
        }
        exec(db, "COMMIT");
    } catch (const SourceError& e) {
        sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr);
        throw Fatal{ExitCode::SourceRead, file + ": " + e.what()};
    } catch (...) {
        sqlite3_exec(db, "ROLLBACK", nullptr, nullptr, nullptr);
        throw;
    }
    for (const Column& c : cols)
        if (c.badValues > 5)
            r.log.push_back("[warn] " + T + "." + c.src.name + ": " + std::to_string(c.badValues) +
                            " unreadable values in total, stored as empty");
    tr.rows = dbf->recordCount();

    // --- tags from the production index ------------------------------------------
    const fs::path mdx = findMdx(sf.path, files);
    if (!mdx.empty()) {
        std::vector<MdxTag> tags;
        try {
            tags = readMdxTags(mdx);
        } catch (const SourceError& e) {
            throw Fatal{ExitCode::SourceRead, e.what()};
        }
        std::vector<KeyField> keyFields;
        for (const Column& c : cols) keyFields.push_back({c.def.name, c.def.type});
        for (const MdxTag& tag : tags) {
            const std::string what = "tag " + tag.name + " (" + tag.expression + ")";
            if (!tag.filter.empty()) { problem(what + " has a FOR clause (" + tag.filter + ")", "tag skipped"); continue; }
            if (tag.descending)      { problem(what + " is descending", "tag skipped"); continue; }
            const KeySql k = translateKey(tag.expression, keyFields);
            if (k.sql.empty())       { problem(what + ": " + k.error, "tag skipped"); continue; }
            if (k.type != tag.keyType)
                r.log.push_back("[warn] " + T + ": " + what + " is a '" + std::string(1, tag.keyType) +
                                "' key in the index but translates to type '" + std::string(1, k.type) + "'");
            try {
                cb4sqlite::createTag(code, T.c_str(), tag.name.c_str(), k.sql.c_str());
            } catch (const cb4sqlite::Error& e) {
                problem(what + ": " + e.what(), "tag skipped");
                continue;
            }
            if (tag.unique) {
                Sql q(db, "SELECT count(*) FROM (SELECT 1 FROM " + cb4sqlite::quoteIdent(T) + " GROUP BY " +
                              k.sql + " HAVING count(*) > 1)");
                q.step();
                if (const auto dups = sqlite3_column_int64(q.get(), 0); dups > 0)
                    problem(what + " is UNIQUE but " + std::to_string(dups) +
                                " key values occur more than once (cb4sqlite shows every record)",
                            "migrated as non-unique");
            }
            tr.tags.push_back(lower(tag.name) + " = " + k.sql);
        }
    }

    r.log.push_back("[info] " + T + ": " + std::to_string(tr.rows) + " rows (" + std::to_string(tr.deletedRows) +
                    " deleted), " + std::to_string(tr.tags.size()) + " tags, from " + file +
                    (mdx.empty() ? "" : " + " + pathUtf8(mdx.filename())) + ", code page " + std::to_string(cp));
    r.tables.push_back(std::move(tr));
    return ok;
}

} // namespace

Result migrate(const Options& opt)
{
    Result r;
    if (opt.srcDir.empty() || opt.outPath.empty()) {
        r.code = ExitCode::BadArgs;
        r.log.push_back("[error] --src and --out are required");
        return r;
    }
    const fs::path tmp = fs::path(opt.outPath).concat(".tmp");
    std::error_code ec;
    if (fs::exists(opt.outPath, ec) && !opt.force) {
        r.code = ExitCode::BadArgs;
        r.log.push_back("[error] " + pathUtf8(opt.outPath) + " already exists (use --force to overwrite)");
        return r;
    }

    bool translateOk = true;
    try {
        const auto files = listDir(opt.srcDir);
        const auto sources = findSources(opt, files, r);

        fs::remove(tmp, ec);
        fs::remove(fs::path(tmp).concat("-journal"), ec);
        {
            Code4 code;
            if (code.open(pathUtf8(tmp).c_str(), true) != r4success)
                throw Fatal{ExitCode::DestWrite, "cannot create " + pathUtf8(tmp)};
            for (const SourceFile& sf : sources)
                translateOk = migrateTable(code, sf, files, opt, r) && translateOk;
        }

        if (translateOk) {
            // Verify by reading back through the shim, as the app will.
            Code4 code;
            if (code.open(pathUtf8(tmp).c_str()) != r4success)
                throw Fatal{ExitCode::Verify, "cannot reopen " + pathUtf8(tmp)};
            for (const TableResult& t : r.tables) {
                Data4 d(code, t.table.c_str());
                if (!d.isValid() || d.recCount() != t.rows)
                    throw Fatal{ExitCode::Verify, t.table + ": expected " + std::to_string(t.rows) +
                                                      " rows, read back " + std::to_string(d.isValid() ? d.recCount() : -1)};
            }
        }
    } catch (const Fatal& f) {
        r.code = f.code;
        r.log.push_back("[error] " + f.message);
    } catch (const cb4sqlite::Error& e) {
        r.code = ExitCode::DestWrite;
        r.log.push_back(std::string("[error] ") + e.what());
    } catch (const fs::filesystem_error& e) {
        r.code = ExitCode::DestWrite;
        r.log.push_back(std::string("[error] ") + e.what());
    }
    if (r.code == ExitCode::Ok && !translateOk) {
        r.code = ExitCode::Translate;
        r.log.push_back("[error] some schema or index definitions can't be represented; nothing was written "
                        "(re-run with --force to skip them)");
    }

    if (r.code != ExitCode::Ok) {
        fs::remove(tmp, ec);
        return r;
    }
    fs::remove(opt.outPath, ec);
    fs::rename(tmp, opt.outPath, ec);
    if (ec) {
        r.code = ExitCode::DestWrite;
        r.log.push_back("[error] cannot move " + pathUtf8(tmp) + " to " + pathUtf8(opt.outPath) + ": " + ec.message());
        fs::remove(tmp, ec);
    }
    return r;
}

} // namespace dbf2sqlite
