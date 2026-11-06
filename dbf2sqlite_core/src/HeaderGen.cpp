//
// HeaderGen.cpp - db_<table>.h for tables without a hand-written class (spec 4.2).
//
// A table counts as covered when any header or source in the check
// directories opens it the way the legacy code does:
//     Data4(cb, "glaccts")            include/db_accts.h
//     Data4 (cb,"access.psw")         gl/header/db_acces.h
//     Data4::open( cb, "GLACCTS" )    common/gl_acct.cpp
// File names can't be used for this: glaccts lives in db_accts.h and
// access.psw in db_acces.h (DOS 8.3 names).
//

#include "Migrator.h"
#include "Source.h"

#include "cb4sqlite.h"
#include "../../cb4sqlite/sqlite3.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <map>
#include <set>
#include <sstream>

namespace dbf2sqlite {

using namespace detail;
namespace fs = std::filesystem;

namespace {

std::string lower(std::string s)
{
    for (char& c : s) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return s;
}

std::string upper(std::string s)
{
    for (char& c : s) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    return s;
}

// Table names opened in `text` by  Data4 <ws> [:: <ws> open <ws>] ( <ws> ident <ws> , <ws> "name"
std::vector<std::string> openedTables(const std::string& text)
{
    std::vector<std::string> out;
    const std::string low = lower(text);
    auto ws = [&](size_t p) { while (p < low.size() && std::isspace(static_cast<unsigned char>(low[p]))) ++p; return p; };
    for (size_t at = low.find("data4"); at != std::string::npos; at = low.find("data4", at + 5)) {
        size_t p = ws(at + 5);
        if (low.compare(p, 2, "::") == 0) {
            p = ws(p + 2);
            if (low.compare(p, 4, "open") != 0) continue;
            p = ws(p + 4);
        }
        if (p >= low.size() || low[p] != '(') continue;
        p = ws(p + 1);
        const size_t id = p;
        while (p < low.size() && (std::isalnum(static_cast<unsigned char>(low[p])) || low[p] == '_')) ++p;
        if (p == id) continue;
        p = ws(p);
        if (p >= low.size() || low[p] != ',') continue;
        p = ws(p + 1);
        if (p >= low.size() || low[p] != '"') continue;
        const size_t close = low.find('"', p + 1);
        if (close == std::string::npos) continue;
        out.push_back(text.substr(p + 1, close - p - 1));
    }
    return out;
}

// table -> file that opens it
std::map<std::string, std::string> scanCovered(const std::vector<fs::path>& dirs)
{
    std::map<std::string, std::string> covered;
    for (const fs::path& dir : dirs) {
        std::error_code ec;
        if (!fs::is_directory(dir, ec)) continue;
        std::vector<fs::path> files;
        for (const auto& e : fs::directory_iterator(dir, ec))
            if (e.is_regular_file()) {
                const std::string ext = lower(pathUtf8(e.path().extension()));
                if (ext == ".h" || ext == ".hpp" || ext == ".cpp") files.push_back(e.path());
            }
        std::sort(files.begin(), files.end());
        for (const fs::path& f : files) {
            std::ifstream in(f, std::ios::binary);
            std::stringstream ss;
            ss << in.rdbuf();
            for (const std::string& name : openedTables(ss.str()))
                covered.emplace(cb4sqlite::normaliseTableName(name.c_str()), pathUtf8(f));  // first wins
        }
    }
    return covered;
}

std::string identifier(const std::string& s)
{
    std::string out;
    for (char c : lower(s))
        out += (std::isalnum(static_cast<unsigned char>(c)) ? c : '_');
    if (out.empty() || std::isdigit(static_cast<unsigned char>(out[0]))) out = "f" + out;
    return out;
}

// Member names that would hide a Data4 member or aren't valid C++ names.
const std::set<std::string>& reserved()
{
    static const std::set<std::string> r = {
        // Data4 / Code4 members a generated class calls or inherits
        "top", "bottom", "skip", "go", "seek", "eof", "bof", "recno", "reccount", "setindex", "select",
        "appendstart", "append", "appendblank", "blank", "flush", "deleterec", "recall", "deleted", "pack",
        "zap", "numfields", "fieldnumber", "isvalid", "close", "unlock", "tablename", "cb", "code",
        // C++ keywords plausible as dBase field names
        "and", "auto", "bool", "break", "case", "catch", "char", "class", "const", "continue", "default",
        "delete", "do", "double", "else", "enum", "explicit", "export", "extern", "false", "float", "for",
        "friend", "goto", "if", "inline", "int", "long", "namespace", "new", "not", "operator", "or",
        "private", "protected", "public", "register", "return", "short", "signed", "sizeof", "static",
        "struct", "switch", "template", "this", "throw", "true", "try", "typedef", "typename", "union",
        "unsigned", "using", "virtual", "void", "volatile", "while", "xor",
    };
    return r;
}

std::string renderHeader(const cb4sqlite::TableInfo& info)
{
    const std::string id = identifier(info.name);
    const std::string cls = "T" + id;
    std::vector<std::string> members;
    std::set<std::string> used;
    for (const cb4sqlite::FieldDef& f : info.fields) {
        std::string m = identifier(f.name);
        if (reserved().count(m)) m = "f" + m;
        const std::string base = m;
        for (int n = 2; used.count(m); ++n) m = base + "_" + std::to_string(n);
        used.insert(m);
        members.push_back(m);
    }

    std::ostringstream h;
    h << "// db_" << id << ".h - generated by dbf2sqlite from table \"" << info.name << "\".\n"
      << "// Hand edits are welcome: dbf2sqlite never overwrites an existing header.\n"
      << "#ifndef DB_" << upper(id) << "_H\n"
      << "#define DB_" << upper(id) << "_H\n\n"
      << "class " << cls << " : public Data4\n{\npublic:\n";
    for (size_t i = 0; i < members.size(); ++i) {
        const cb4sqlite::FieldDef& f = info.fields[i];
        h << "    " << (f.type == 'M' ? "Field4memo*" : "Field4*") << " " << members[i] << " = nullptr;   // "
          << f.name << " " << f.type << "(" << f.len;
        if (f.dec) h << "," << f.dec;
        h << ")\n";
    }
    h << "\n    " << cls << "(Code4* code, LPSTR = 0) : Data4(code, \"" << info.name << "\")\n    {\n"
      << "        if (!isValid())\n            return;\n";
    if (!info.tags.empty())
        h << "        SetIndex(\"" << info.tags.front().name << "\");\n";
    h << "        top();\n";
    for (size_t i = 0; i < members.size(); ++i)
        h << "        " << members[i] << " = new " << (info.fields[i].type == 'M' ? "Field4memo" : "Field4")
          << "(*this, \"" << info.fields[i].name << "\");\n";
    h << "    }\n\n    ~" << cls << "()\n    {\n";
    for (const std::string& m : members)
        h << "        delete " << m << ";\n";
    h << "    }\n};\n\n#endif\n";
    return h.str();
}

} // namespace

Result generateHeaders(const Options& opt)
{
    Result r;
    if (opt.outPath.empty() || opt.headerOutDir.empty()) {
        r.code = ExitCode::BadArgs;
        r.log.push_back("[error] header generation needs --out (the migrated database) and --headers-out");
        return r;
    }
    Code4 code;
    if (code.open(pathUtf8(opt.outPath).c_str()) != r4success) {
        r.code = ExitCode::SourceRead;
        r.log.push_back("[error] cannot open " + pathUtf8(opt.outPath));
        return r;
    }

    std::vector<std::string> tables;
    {
        sqlite3_stmt* s = nullptr;
        sqlite3_prepare_v2(code.handle(), "SELECT DISTINCT table_name FROM _cb_fields ORDER BY table_name", -1, &s, nullptr);
        while (s && sqlite3_step(s) == SQLITE_ROW)
            tables.emplace_back(reinterpret_cast<const char*>(sqlite3_column_text(s, 0)));
        sqlite3_finalize(s);
    }
    if (!opt.tables.empty()) {
        std::set<std::string> want;
        for (const std::string& t : opt.tables) want.insert(cb4sqlite::normaliseTableName(t.c_str()));
        std::erase_if(tables, [&](const std::string& t) { return !want.count(t); });
    }

    std::vector<fs::path> dirs = opt.headerCheckDirs;
    dirs.push_back(opt.headerOutDir);
    const auto covered = scanCovered(dirs);

    std::error_code ec;
    fs::create_directories(opt.headerOutDir, ec);
    for (const std::string& t : tables) {
        TableResult tr;
        tr.table = t;
        if (const auto it = covered.find(t); it != covered.end()) {
            tr.header = "skip: opened by " + it->second;
            r.log.push_back("[skip] " + t + ": already opened by " + it->second + " (hand-written)");
            r.tables.push_back(std::move(tr));
            continue;
        }
        cb4sqlite::TableInfo info;
        if (!cb4sqlite::loadTableInfo(code.handle(), t, info)) {
            r.code = ExitCode::SourceRead;
            r.log.push_back("[error] " + t + ": no field metadata");
            continue;
        }
        const fs::path out = opt.headerOutDir / ("db_" + identifier(t) + ".h");
        if (fs::exists(out, ec)) {
            tr.header = "skip: " + pathUtf8(out) + " exists";
            r.log.push_back("[skip] " + t + ": " + pathUtf8(out) + " exists but doesn't open this table; not overwritten");
            r.tables.push_back(std::move(tr));
            continue;
        }
        std::ofstream f(out, std::ios::binary);
        f << renderHeader(info);
        if (!f) {
            r.code = ExitCode::DestWrite;
            r.log.push_back("[error] cannot write " + pathUtf8(out));
            continue;
        }
        tr.header = pathUtf8(out);
        r.log.push_back("[info] " + t + ": wrote " + pathUtf8(out));
        r.tables.push_back(std::move(tr));
    }
    return r;
}

} // namespace dbf2sqlite
