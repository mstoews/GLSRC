//
// TextRender.cpp - see TextRender.h.
//
#include "report/TextRender.h"

#include <algorithm>
#include <cstdlib>
#include <map>
#include <string>

namespace rpt {

namespace {

size_t codePoints(std::string_view s) noexcept
{
    size_t n = 0;
    for (char c : s)
        if ((static_cast<unsigned char>(c) & 0xC0) != 0x80) ++n;
    return n;
}

// A line of the grid, as code points (so UTF-8 text keeps its columns).
using Line = std::vector<std::string>;

void put(Line& line, int col, std::string_view text)
{
    if (col < 0) col = 0;
    size_t at = static_cast<size_t>(col);
    size_t i = 0;
    while (i < text.size()) {
        size_t len = 1;
        while (i + len < text.size() && (static_cast<unsigned char>(text[i + len]) & 0xC0) == 0x80) ++len;
        if (line.size() <= at) line.resize(at + 1, " ");
        line[at++] = std::string(text.substr(i, len));
        i += len;
    }
}

std::string join(const Line& line)
{
    std::string s;
    for (const std::string& c : line) s += c;
    while (!s.empty() && s.back() == ' ') s.pop_back();
    return s;
}

} // namespace

int FixedMeasure::TextWidth(Role, std::string_view utf8) const
{
    return static_cast<int>(codePoints(utf8)) * cw_;
}

std::string RenderText(const std::vector<Page>& pages, const PageSetup& setup, const FixedMeasure& measure)
{
    const int cw = measure.charWidth();
    std::string out;
    for (size_t p = 0; p < pages.size(); ++p) {
        out += "--- page " + std::to_string(p + 1) + " ---\n";
        // Lines keyed by y; a rule sorts after the text that starts at the same y.
        std::map<std::pair<int, int>, Line> lines;
        for (const TextOp& t : pages[p].texts) {
            const int len = static_cast<int>(codePoints(t.text));
            const int boxStart = (t.x - setup.left) / cw;
            const int boxChars = t.width / cw;
            int col = boxStart;
            if (t.align == Align::Right) col = boxStart + boxChars - len;
            if (t.align == Align::Center) col = boxStart + (boxChars - len) / 2;
            put(lines[{t.y, 0}], col, t.text);
        }
        // Rules: a double rule is two rules a few twips apart; merge those into '='.
        std::vector<RuleOp> rules = pages[p].rules;
        std::sort(rules.begin(), rules.end(), [](const RuleOp& a, const RuleOp& b) {
            return a.y != b.y ? a.y < b.y : a.x1 < b.x1;
        });
        const int close = measure.LineHeight(Role::Body) / 4;
        for (size_t i = 0; i < rules.size(); ++i) {
            const RuleOp& r = rules[i];
            bool dbl = false;
            for (size_t j = 0; j < rules.size(); ++j)
                if (j != i && rules[j].x1 == r.x1 && rules[j].x2 == r.x2 && rules[j].y != r.y &&
                    std::abs(rules[j].y - r.y) < close)
                    dbl = true;
            // Both halves of a double rule land on the first one's line.
            int y = r.y;
            for (const RuleOp& o : rules)
                if (o.x1 == r.x1 && o.x2 == r.x2 && o.y < y && y - o.y < close) y = o.y;
            const int from = (r.x1 - setup.left) / cw, to = (r.x2 - setup.left) / cw;
            put(lines[{y, 1}], from, std::string(static_cast<size_t>(std::max(0, to - from)), dbl ? '=' : '-'));
        }
        // Vertical gaps of a line or more (blank rows) print as empty lines.
        const int lh = measure.LineHeight(Role::Body);
        int lastText = -1;
        for (const auto& [key, line] : lines) {
            if (key.second == 0) {
                if (lastText >= 0)
                    for (int gap = key.first - lastText; gap >= 2 * lh; gap -= lh) out += '\n';
                lastText = key.first;
            }
            out += join(line) + '\n';
        }
    }
    return out;
}

} // namespace rpt
