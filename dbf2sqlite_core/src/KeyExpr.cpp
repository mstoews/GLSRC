//
// KeyExpr.cpp - dBase index key expression -> cb4sqlite key_expr (SQL).
//
// cb4sqlite stores character fields right-padded to their width, so the dBase
// concatenation ACCOUNT+DEPT is reproduced byte for byte by "ACCOUNT" || "DEPT".
//
// Grammar (case-insensitive):
//   expr  := term { '+' term }            all terms character-typed if more than one
//   term  := FIELD | func '(' args ')'
//   func  := UPPER(c) | DTOS(d) | STR(n [, len [, dec]])
//

#include "Source.h"

#include <cctype>
#include <optional>

namespace dbf2sqlite::detail {

namespace {

struct Translator {
    const std::string& src;
    const std::vector<KeyField>& fields;
    size_t pos = 0;
    std::string error;

    void skipSpace()
    {
        while (pos < src.size() && std::isspace(static_cast<unsigned char>(src[pos]))) ++pos;
    }
    bool eat(char c)
    {
        skipSpace();
        if (pos < src.size() && src[pos] == c) { ++pos; return true; }
        return false;
    }
    std::string ident()
    {
        skipSpace();
        const size_t start = pos;
        while (pos < src.size() &&
               (std::isalnum(static_cast<unsigned char>(src[pos])) || src[pos] == '_'))
            ++pos;
        std::string s = src.substr(start, pos - start);
        for (char& c : s) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        return s;
    }
    std::optional<int> number()
    {
        skipSpace();
        const size_t start = pos;
        while (pos < src.size() && std::isdigit(static_cast<unsigned char>(src[pos]))) ++pos;
        if (start == pos) return std::nullopt;
        return std::stoi(src.substr(start, pos - start));
    }
    bool fail(const std::string& why)
    {
        if (error.empty()) error = why;
        return false;
    }

    static std::string quote(const std::string& name) { return "\"" + name + "\""; }
    static std::string upper(std::string s)
    {
        for (char& c : s) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        return s;
    }

    bool term(std::string& sql, char& type)
    {
        const std::string id = ident();
        if (id.empty())
            return fail("expected a field name or function at position " + std::to_string(pos + 1));
        skipSpace();
        if (pos < src.size() && src[pos] == '-' && pos + 1 < src.size() && src[pos + 1] == '>')
            return fail("alias references (" + id + "->...) are not supported");
        if (!eat('(')) {
            for (const KeyField& f : fields)
                if (upper(f.name) == id) {
                    sql = quote(f.name);
                    type = (f.type == 'F') ? 'N' : f.type;
                    if (type != 'C' && type != 'N' && type != 'D')
                        return fail("field " + id + " of type " + f.type + " can't be a key");
                    return true;
                }
            return fail("no field named " + id);
        }

        std::string inner;
        char innerType = 'C';
        if (!expr(inner, innerType)) return false;
        if (id == "UPPER") {
            if (innerType != 'C') return fail("UPPER() needs a character argument");
            if (!eat(')')) return fail("expected ')' after UPPER(");
            sql = "upper(" + inner + ")";
            type = 'C';
            return true;
        }
        if (id == "DTOS") {
            if (innerType != 'D') return fail("DTOS() needs a date argument");
            if (!eat(')')) return fail("expected ')' after DTOS(");
            sql = "ifnull(" + inner + ", '        ')";   // blank date -> 8 spaces, as dBase
            type = 'C';
            return true;
        }
        if (id == "STR") {
            if (innerType != 'N') return fail("STR() needs a numeric argument");
            int len = 10, dec = 0;                     // dBase defaults
            if (eat(',')) {
                const auto l = number();
                if (!l || *l < 1 || *l > 255) return fail("STR() length must be 1..255");
                len = *l;
                if (eat(',')) {
                    const auto d = number();
                    if (!d || *d < 0 || *d >= len) return fail("bad STR() decimals");
                    dec = *d;
                }
            }
            if (!eat(')')) return fail("expected ')' after STR(");
            sql = "printf('%" + std::to_string(len) + "." + std::to_string(dec) + "f', ifnull(" + inner + ", 0))";
            type = 'C';
            return true;
        }
        return fail("function " + id + "() is not supported");
    }

    bool expr(std::string& sql, char& type)
    {
        if (!term(sql, type)) return false;
        while (eat('+')) {
            std::string rhs;
            char rtype = 'C';
            if (!term(rhs, rtype)) return false;
            if (type != 'C' || rtype != 'C')
                return fail("'+' is only supported between character values");
            sql += " || " + rhs;
        }
        return true;
    }
};

} // namespace

KeySql translateKey(const std::string& expression, const std::vector<KeyField>& fields)
{
    Translator t{expression, fields, 0, {}};
    KeySql out;
    std::string sql;
    char type = 'C';
    if (!t.expr(sql, type)) {
        out.error = t.error;
        return out;
    }
    t.skipSpace();
    if (t.pos != expression.size()) {
        out.error = "unexpected '" + expression.substr(t.pos) + "'";
        return out;
    }
    out.sql = sql;
    out.type = type;
    return out;
}

} // namespace dbf2sqlite::detail
