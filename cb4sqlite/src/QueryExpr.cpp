//
// QueryExpr.cpp - see QueryExpr.h.
//

#include "QueryExpr.h"
#include "Internal.h"

#include <cctype>

namespace cb4sqlite::detail {

namespace {

class Parser {
public:
    Parser(const TableInfo& t, std::string_view src) : table_(t), src_(src) {}

    QueryFilter run()
    {
        skipSpace();
        if (pos_ == src_.size()) return {};          // empty: every record
        std::string sql = clause();
        while (eatWord(".AND.")) sql += " AND " + clause();
        skipSpace();
        if (pos_ != src_.size()) unsupported();
        out_.sql = std::move(sql);
        return std::move(out_);
    }

private:
    [[noreturn]] void unsupported() const
    {
        notImplemented("Relate4set::querySet", "expression \"" + std::string(src_) + "\"");
    }

    void skipSpace()
    {
        while (pos_ < src_.size() && std::isspace(static_cast<unsigned char>(src_[pos_]))) ++pos_;
    }

    // Case-insensitive match of a fixed token.
    bool eatWord(std::string_view w)
    {
        skipSpace();
        if (src_.size() - pos_ < w.size() || !iequals(src_.substr(pos_, w.size()), w)) return false;
        pos_ += w.size();
        return true;
    }

    bool eatDeletedCall()
    {
        const size_t save = pos_;
        if (eatWord("DELETED") && eatWord("(") && eatWord(")")) return true;
        pos_ = save;
        return false;
    }

    std::string clause()
    {
        if (eatWord(".NOT.") || eatWord("!")) {
            if (!eatDeletedCall()) unsupported();
            return "(_deleted = 0)";
        }
        const FieldDef& f = field();
        bool negate;
        if (eatWord("<>") || eatWord("!=") || eatWord("#")) negate = true;
        else if (eatWord("=")) negate = false;
        else unsupported();
        if (f.type != r4str)
            throw Error("Relate4set::querySet", "field " + f.name + " is not a character field: \"" +
                                                    std::string(src_) + "\"");
        const std::string p = ":q" + std::to_string(out_.params.size() + 1);
        out_.params.push_back({p, literal()});
        // NULL reads as blanks, as dBase saw an empty field.
        const std::string col = "ifnull(" + quoteIdent(f.name) + ", '" + std::string(static_cast<size_t>(f.len), ' ') + "')";
        const std::string eq = "(substr(" + col + ", 1, length(" + p + ")) = " + p + ")";
        return negate ? "(NOT " + eq + ")" : eq;
    }

    const FieldDef& field()
    {
        skipSpace();
        const size_t start = pos_;
        while (pos_ < src_.size() &&
               (std::isalnum(static_cast<unsigned char>(src_[pos_])) || src_[pos_] == '_'))
            ++pos_;
        if (pos_ == start) unsupported();
        const std::string_view name = src_.substr(start, pos_ - start);
        skipSpace();
        if (pos_ < src_.size() && src_[pos_] == '(') unsupported();   // a function call
        for (const FieldDef& f : table_.fields)
            if (iequals(f.name, name)) return f;
        throw Error("Relate4set::querySet",
                    "table " + table_.name + " has no field " + std::string(name) + ": \"" + std::string(src_) + "\"");
    }

    std::string literal()
    {
        skipSpace();
        if (pos_ == src_.size() || (src_[pos_] != '\'' && src_[pos_] != '"')) unsupported();
        const char q = src_[pos_++];
        const size_t end = src_.find(q, pos_);
        if (end == std::string_view::npos) unsupported();
        std::string s(src_.substr(pos_, end - pos_));
        pos_ = end + 1;
        return s;
    }

    const TableInfo& table_;
    std::string_view src_;
    size_t pos_ = 0;
    QueryFilter out_;
};

} // namespace

QueryFilter translateQuery(const TableInfo& table, std::string_view expression)
{
    return Parser(table, expression).run();
}

} // namespace cb4sqlite::detail
