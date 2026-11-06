//
// KeyExpr.cpp - dBase index expressions -> SQL, for cb4sqlite's _cb_tags.
//
// Two entry points share one parser:
//
//   translateKey     the tag's key expression  -> key_expr
//   translateFilter  the tag's FOR condition   -> filter
//
// cb4sqlite stores character fields right-padded to their width, so the dBase
// concatenation ACCOUNT+DEPT is reproduced byte for byte by "ACCOUNT" || "DEPT".
//
// Grammar (case-insensitive):
//   cond    := and { .OR. and }
//   and     := not { .AND. not }
//   not     := (.NOT. | !) not | compare
//   compare := sum [ (= | == | <> | # | != | < | <= | > | >= | $) sum ]
//   sum     := term { + term }        character + character, or number + number
//   term    := FIELD | 'text' | "text" | [text] | number | .T. | .F.
//            | UPPER(c) | DTOS(d) | STR(n [, len [, dec]]) | RECNO() | DELETED()
//            | ( cond )
// A key expression is a single `sum` without literals; a FOR condition is a
// `cond` and must be logical.
//
// Comparisons follow dBase with SET EXACT OFF (the default the app ran with):
// for text, a = b is true when a starts with b, so `TYPE = 'AR'` matches
// "AR " as well. == compares ignoring trailing blanks. Inside a FOR condition,
// blank numbers read as 0 and blank dates as eight spaces, as dBase saw them.
//

#include "Source.h"

#include <cctype>
#include <optional>

namespace dbf2sqlite::detail {

namespace {

struct Val {
    std::string sql;
    char        type = 'C';   // C, N, D or L
};

struct Translator {
    const std::string& src;
    const std::vector<KeyField>& fields;
    bool filterMode = false;
    size_t pos = 0;
    std::string error;

    // --- lexing -----------------------------------------------------------------
    void skipSpace()
    {
        while (pos < src.size() && std::isspace(static_cast<unsigned char>(src[pos]))) ++pos;
    }
    bool peek(char c)
    {
        skipSpace();
        return pos < src.size() && src[pos] == c;
    }
    bool eat(char c)
    {
        if (!peek(c)) return false;
        ++pos;
        return true;
    }
    bool eatOp(const char* op)
    {
        skipSpace();
        const size_t n = std::char_traits<char>::length(op);
        if (src.compare(pos, n, op) != 0) return false;
        pos += n;
        return true;
    }
    // .AND. .OR. .NOT. .T. .F. (any case)
    bool eatDotWord(const char* word)
    {
        skipSpace();
        const size_t n = std::char_traits<char>::length(word);
        if (pos + n + 2 > src.size() || src[pos] != '.' || src[pos + n + 1] != '.') return false;
        for (size_t i = 0; i < n; ++i)
            if (std::toupper(static_cast<unsigned char>(src[pos + 1 + i])) != word[i]) return false;
        pos += n + 2;
        return true;
    }
    std::string ident()
    {
        skipSpace();
        const size_t start = pos;
        while (pos < src.size() &&
               (std::isalnum(static_cast<unsigned char>(src[pos])) || src[pos] == '_'))
            ++pos;
        return upper(src.substr(start, pos - start));
    }
    std::optional<int> integer()
    {
        skipSpace();
        const size_t start = pos;
        while (pos < src.size() && std::isdigit(static_cast<unsigned char>(src[pos]))) ++pos;
        if (start == pos || pos - start > 6) return std::nullopt;
        return std::stoi(src.substr(start, pos - start));
    }
    bool fail(const std::string& why)
    {
        if (error.empty()) error = why;
        return false;
    }

    static std::string quoteIdent(const std::string& name) { return "\"" + name + "\""; }
    static std::string quoteText(const std::string& text)
    {
        std::string out = "'";
        for (char c : text) {
            if (c == '\'') out += '\'';
            out += c;
        }
        return out + "'";
    }
    static std::string upper(std::string s)
    {
        for (char& c : s) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        return s;
    }

    // --- conditions ---------------------------------------------------------------
    bool cond(Val& v)
    {
        if (!conj(v)) return false;
        while (eatDotWord("OR")) {
            Val r;
            if (!conj(r)) return false;
            if (v.type != 'L' || r.type != 'L') return fail(".OR. needs logical operands");
            v.sql = "(" + v.sql + " OR " + r.sql + ")";
        }
        return true;
    }
    bool conj(Val& v)
    {
        if (!negation(v)) return false;
        while (eatDotWord("AND")) {
            Val r;
            if (!negation(r)) return false;
            if (v.type != 'L' || r.type != 'L') return fail(".AND. needs logical operands");
            v.sql = "(" + v.sql + " AND " + r.sql + ")";
        }
        return true;
    }
    bool negation(Val& v)
    {
        bool negate = eatDotWord("NOT");
        if (!negate && peek('!') && !(pos + 1 < src.size() && src[pos + 1] == '=')) {
            ++pos;
            negate = true;
        }
        if (!negate) return compare(v);
        if (!negation(v)) return false;
        if (v.type != 'L') return fail(".NOT. needs a logical operand");
        v.sql = "(NOT " + v.sql + ")";
        return true;
    }
    bool compare(Val& v)
    {
        if (!sum(v)) return false;
        static const char* const ops[] = {"==", "<>", "!=", "<=", ">=", "=", "#", "<", ">", "$"};
        const char* op = nullptr;
        for (const char* o : ops)
            if (eatOp(o)) { op = o; break; }
        if (!op) return true;
        Val r;
        if (!sum(r)) return false;
        if (v.type != r.type) return fail(std::string("can't compare ") + v.type + " with " + r.type);
        const std::string o(op);
        if (o == "$") {
            if (v.type != 'C') return fail("$ needs character operands");
            v.sql = "(instr(" + r.sql + ", " + v.sql + ") > 0)";
        } else if (v.type == 'C' && (o == "=" || o == "<>" || o == "#" || o == "!=")) {
            // SET EXACT OFF: equal when the left side starts with the right side.
            const std::string eq = "(substr(" + v.sql + ", 1, length(" + r.sql + ")) = " + r.sql + ")";
            v.sql = (o == "=") ? eq : "(NOT " + eq + ")";
        } else if (v.type == 'C' && o == "==") {
            v.sql = "(rtrim(" + v.sql + ") = rtrim(" + r.sql + "))";
        } else {
            const std::string sqlOp = (o == "==") ? "=" : (o == "#" || o == "!=") ? "<>" : o;
            v.sql = "(" + v.sql + " " + sqlOp + " " + r.sql + ")";
        }
        v.type = 'L';
        return true;
    }

    // --- values -------------------------------------------------------------------
    bool sum(Val& v)
    {
        if (!term(v)) return false;
        while (eat('+')) {
            Val r;
            if (!term(r)) return false;
            if (v.type == 'C' && r.type == 'C')
                v.sql += " || " + r.sql;
            else if (v.type == 'N' && r.type == 'N')
                v.sql = "(" + v.sql + " + " + r.sql + ")";
            else
                return fail("'+' is only supported between character values or between numbers");
        }
        return true;
    }

    // Text, number or logical literal. False without consuming anything (and
    // without an error) if there is none here.
    bool literal(Val& v)
    {
        skipSpace();
        if (pos >= src.size()) return false;
        const char c = src[pos];
        if (c == '\'' || c == '"' || c == '[') {
            if (!filterMode) return fail("text literals aren't supported in a key expression");
            const char close = (c == '[') ? ']' : c;
            const size_t end = src.find(close, pos + 1);
            if (end == std::string::npos) return fail("unterminated text literal");
            v = {quoteText(src.substr(pos + 1, end - pos - 1)), 'C'};
            pos = end + 1;
            return true;
        }
        if (eatDotWord("T") || eatDotWord("Y")) { v = {"1", 'L'}; return true; }
        if (eatDotWord("F") || eatDotWord("N")) { v = {"0", 'L'}; return true; }
        size_t p = pos + (c == '-' ? 1 : 0);
        bool digits = false, dot = false;
        while (p < src.size() && (std::isdigit(static_cast<unsigned char>(src[p])) || (src[p] == '.' && !dot))) {
            if (src[p] == '.') dot = true; else digits = true;
            ++p;
        }
        if (!digits) return false;
        if (!filterMode) return fail("numeric literals aren't supported in a key expression");
        v = {src.substr(pos, p - pos), 'N'};
        pos = p;
        return true;
    }

    bool field(const std::string& id, Val& v)
    {
        for (const KeyField& f : fields) {
            if (upper(f.name) != id) continue;
            const char t = (f.type == 'F') ? 'N' : f.type;
            const std::string col = quoteIdent(f.name);
            if (t == 'C')      v = {col, 'C'};
            else if (t == 'N') v = {filterMode ? "ifnull(" + col + ", 0)" : col, 'N'};
            else if (t == 'D') v = {filterMode ? "ifnull(" + col + ", '        ')" : col, 'D'};
            else if (t == 'L' && filterMode) v = {"(ifnull(" + col + ", 0) = 1)", 'L'};
            else return fail("field " + id + " of type " + f.type + " can't be used here");
            return true;
        }
        return fail("no field named " + id);
    }

    bool term(Val& v)
    {
        if (eat('(')) {
            if (!cond(v)) return false;
            if (!eat(')')) return fail("expected ')'");
            return true;
        }
        if (literal(v)) return true;
        if (!error.empty()) return false;

        const std::string id = ident();
        if (id.empty())
            return fail("expected a field name or function at position " + std::to_string(pos + 1));
        skipSpace();
        if (pos + 1 < src.size() && src[pos] == '-' && src[pos + 1] == '>')
            return fail("alias references (" + id + "->...) are not supported");
        if (!eat('('))
            return field(id, v);

        // Functions without arguments.
        if (id == "RECNO" || id == "DELETED") {
            if (!eat(')')) return fail("expected ')' after " + id + "(");
            if (id == "RECNO") { v = {"_rowid", 'N'}; return true; }   // the dBase record number
            if (!filterMode) return fail("DELETED() can't be part of a key");
            v = {"(_deleted <> 0)", 'L'};
            return true;
        }

        Val inner;
        if (!sum(inner)) return false;
        if (id == "UPPER") {
            if (inner.type != 'C') return fail("UPPER() needs a character argument");
            if (!eat(')')) return fail("expected ')' after UPPER(");
            v = {"upper(" + inner.sql + ")", 'C'};
            return true;
        }
        if (id == "DTOS") {
            if (inner.type != 'D') return fail("DTOS() needs a date argument");
            if (!eat(')')) return fail("expected ')' after DTOS(");
            v = {"ifnull(" + inner.sql + ", '        ')", 'C'};   // blank date -> 8 spaces, as dBase
            return true;
        }
        if (id == "STR") {
            if (inner.type != 'N') return fail("STR() needs a numeric argument");
            int len = 10, dec = 0;                     // dBase defaults
            if (eat(',')) {
                const auto l = integer();
                if (!l || *l < 1 || *l > 255) return fail("STR() length must be 1..255");
                len = *l;
                if (eat(',')) {
                    const auto d = integer();
                    if (!d || *d < 0 || *d >= len) return fail("bad STR() decimals");
                    dec = *d;
                }
            }
            if (!eat(')')) return fail("expected ')' after STR(");
            v = {"printf('%" + std::to_string(len) + "." + std::to_string(dec) + "f', ifnull(" + inner.sql + ", 0))", 'C'};
            return true;
        }
        return fail("function " + id + "() is not supported");
    }

    bool atEnd()
    {
        skipSpace();
        if (pos == src.size()) return true;
        return fail("unexpected '" + src.substr(pos) + "'");
    }
};

} // namespace

KeySql translateKey(const std::string& expression, const std::vector<KeyField>& fields)
{
    Translator t{expression, fields, false, 0, {}};
    KeySql out;
    Val v;
    if (!t.sum(v) || !t.atEnd()) {
        out.error = t.error;
        return out;
    }
    if (v.type == 'L') {
        out.error = "a logical value can't be a key";
        return out;
    }
    out.sql = v.sql;
    out.type = v.type;
    return out;
}

KeySql translateFilter(const std::string& condition, const std::vector<KeyField>& fields)
{
    Translator t{condition, fields, true, 0, {}};
    KeySql out;
    Val v;
    if (!t.cond(v) || !t.atEnd()) {
        out.error = t.error;
        return out;
    }
    if (v.type != 'L') {
        out.error = "the FOR condition isn't a logical expression";
        return out;
    }
    out.sql = v.sql;
    out.type = 'L';
    return out;
}

} // namespace dbf2sqlite::detail
