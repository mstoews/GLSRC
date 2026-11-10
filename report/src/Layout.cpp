//
// Layout.cpp - see Layout.h.
//
#include "report/Layout.h"

#include "report/Format.h"

#include <algorithm>
#include <cstddef>
#include <string>

namespace rpt {

namespace {

// Paper sizes in twips.
constexpr int kA4W = 11906, kA4H = 16838, kLetterW = 12240, kLetterH = 15840, kMargin = 1080;

PageSetup paper(int w, int h, Orientation o)
{
    PageSetup s;
    s.width = o == Orientation::Portrait ? w : h;
    s.height = o == Orientation::Portrait ? h : w;
    s.left = s.top = s.right = s.bottom = kMargin;
    return s;
}

bool gluedToNext(RowKind k) noexcept
{
    return k == RowKind::Heading || k == RowKind::GroupHeader || k == RowKind::Slip;
}

bool gluedToPrevious(RowKind k) noexcept
{
    return k == RowKind::GroupFooter || k == RowKind::Subtotal || k == RowKind::Total;
}

class Engine {
public:
    Engine(const Report& r, const PageSetup& s, const Measure& m) : r_(r), s_(s), m_(m)
    {
        body_ = m_.LineHeight(Role::Body);
        bold_ = std::max(body_, m_.LineHeight(Role::Bold));
        rulePad_ = body_ / 4;
        columns();
    }

    std::vector<Page> run();

private:
    // --- geometry ---
    void columns();
    int  headerHeight() const;
    bool sideLine() const;
    int  columnHeaderHeight() const;
    int  rowHeight(const Row& row) const;
    std::vector<std::string> wrap(std::string_view text, int width) const;

    // --- drawing into the current page ---
    // Draw into *cur_: the page being filled, or the page whose header is drawn.
    void text(int x, int y, int w, Align a, Role role, std::string_view s);
    void rule(int x1, int x2, int y) { cur_->rules.push_back({x1, x2, y}); }
    void drawColumnHeaders(int& y);
    void drawCells(const Row& row, int y, Role role);
    void drawRow(const Row& row, int& y);
    void drawPageHeaders();

    // --- pagination ---
    void newPage();
    int  bodyTop() const { return s_.top + headerHeight(); }
    int  bodyLimit() const;

    const Report&    r_;
    const PageSetup& s_;
    const Measure&   m_;
    int body_ = 0, bold_ = 0, rulePad_ = 0;
    std::vector<int> colX_, colW_;
    std::vector<Page> pages_;
    Page* cur_ = nullptr;
    int  y_ = 0;
    bool inSlip_ = false;
};

void Engine::columns()
{
    const size_t n = r_.columns.size();
    if (n == 0) return;
    double total = 0;
    for (const Column& c : r_.columns) total += c.width > 0 ? c.width : 0;
    if (total <= 0) total = 1;
    const int gaps = kColumnGap * static_cast<int>(n - 1);
    const int avail = std::max(0, s_.printableWidth() - gaps);
    int x = s_.left;
    for (size_t i = 0; i < n; ++i) {
        const double w = r_.columns[i].width > 0 ? r_.columns[i].width : 0;
        int width = static_cast<int>(avail * w / total);
        if (i + 1 == n) width = s_.left + s_.printableWidth() - x;   // absorb rounding
        colX_.push_back(x);
        colW_.push_back(width);
        x += width + kColumnGap;
    }
}

int Engine::columnHeaderHeight() const
{
    bool any = false, second = false;
    for (const Column& c : r_.columns) {
        any = any || !c.header.empty() || !c.header2.empty();
        second = second || !c.header2.empty();
    }
    if (!any) return 0;
    return bold_ * (second ? 2 : 1) + rulePad_;
}

// "Printed on ..." and "Page n of m" share the company's line when they fit
// beside it, and get a line of their own above it when they don't.
bool Engine::sideLine() const
{
    const int full = s_.printableWidth();
    const int company = m_.TextWidth(Role::Title, r_.company);
    const int need = std::max(m_.TextWidth(Role::Body, "Printed on " + r_.printedOn),
                              m_.TextWidth(Role::Body, "Page 999 of 999"));
    return (full - company) / 2 - kColumnGap < need;
}

int Engine::headerHeight() const
{
    int h = 2 * m_.LineHeight(Role::Title) + (sideLine() ? body_ : 0);
    h += body_ * static_cast<int>(r_.subtitles.size());
    h += body_ / 2;
    if (!r_.slipMode) h += columnHeaderHeight() + rulePad_;
    return h;
}

int Engine::bodyLimit() const
{
    int limit = s_.height - s_.bottom;
    if (!r_.footerNote.empty()) limit -= body_ + body_ / 2;
    return limit;
}

std::vector<std::string> Engine::wrap(std::string_view t, int width) const
{
    std::vector<std::string> lines;
    std::string line;
    size_t i = 0;
    while (i < t.size()) {
        while (i < t.size() && t[i] == ' ') ++i;
        size_t j = i;
        while (j < t.size() && t[j] != ' ') ++j;
        if (j == i) break;
        const std::string word(t.substr(i, j - i));
        const std::string candidate = line.empty() ? word : line + ' ' + word;
        if (line.empty() || m_.TextWidth(Role::Body, candidate) <= width) {
            line = candidate;
        } else {
            lines.push_back(line);
            line = word;
        }
        i = j;
    }
    if (!line.empty() || lines.empty()) lines.push_back(line);
    return lines;
}

int Engine::rowHeight(const Row& row) const
{
    switch (row.kind) {
    case RowKind::Detail:      return body_;
    case RowKind::Heading:     return bold_;
    case RowKind::GroupHeader: return bold_;
    case RowKind::GroupFooter:
    case RowKind::Subtotal:    return rulePad_ + body_;
    case RowKind::Total:       return rulePad_ + bold_ + (row.final ? rulePad_ : 0);
    case RowKind::Note:
        return body_ * static_cast<int>(wrap(row.text, s_.printableWidth() - row.indent * kIndentStep).size());
    case RowKind::Blank:       return body_;
    case RowKind::PageBreak:   return 0;
    case RowKind::Slip: {
        const int lines = static_cast<int>((row.fields.size() + 1) / 2);
        return body_ * lines + body_ / 2 + columnHeaderHeight();
    }
    }
    return body_;
}

void Engine::text(int x, int y, int w, Align a, Role role, std::string_view s)
{
    if (s.empty() || w <= 0) return;
    cur_->texts.push_back({x, y, w, a, role, FitText(m_, role, s, w)});
}

void Engine::drawColumnHeaders(int& y)
{
    bool any = false, second = false;
    for (const Column& c : r_.columns) {
        any = any || !c.header.empty() || !c.header2.empty();
        second = second || !c.header2.empty();
    }
    if (!any) return;
    for (size_t i = 0; i < r_.columns.size(); ++i) {
        const Column& c = r_.columns[i];
        text(colX_[i], y, colW_[i], c.alignment(), Role::Bold, c.header);
        if (second) text(colX_[i], y + bold_, colW_[i], c.alignment(), Role::Bold, c.header2);
    }
    y += bold_ * (second ? 2 : 1);
    rule(s_.left, s_.left + s_.printableWidth(), y + rulePad_ / 2);
    y += rulePad_;
}

void Engine::drawCells(const Row& row, int y, Role role)
{
    for (size_t i = 0; i < r_.columns.size() && i < row.cells.size(); ++i) {
        const Cell& cell = row.cells[i];
        const Column& col = r_.columns[i];
        const std::string s = FormatCell(cell, col, r_.negatives);
        if (s.empty()) continue;
        int x = colX_[i], w = colW_[i];
        if (i == 0 && row.indent > 0) {
            const int in = std::min(w, row.indent * kIndentStep);
            x += in;
            w -= in;
        }
        const Role cellRole = cell.bold ? Role::Bold : role;
        // Numbers and dates that don't fit print "###": cut short, they would
        // read as another value ("10/15/20" for 10/15/2026).
        const bool whole = cell.type == CellType::Amount || cell.type == CellType::Integer || cell.type == CellType::Date;
        if (whole && m_.TextWidth(cellRole, s) > w)
            text(x, y, w, col.alignment(), cellRole, "###");
        else
            text(x, y, w, col.alignment(), cellRole, s);
    }
}

void Engine::drawRow(const Row& row, int& y)
{
    const int full = s_.printableWidth();
    const int in = std::min(full, row.indent * kIndentStep);
    switch (row.kind) {
    case RowKind::Detail:
        drawCells(row, y, Role::Body);
        break;
    case RowKind::Heading:
        text(s_.left + in, y, full - in, Align::Left, Role::Bold, row.text);
        break;
    case RowKind::GroupHeader:
        if (row.cells.empty())
            text(s_.left + in, y, full - in, Align::Left, Role::Bold, row.text);
        else
            drawCells(row, y, Role::Bold);
        break;
    case RowKind::GroupFooter:
    case RowKind::Subtotal:
    case RowKind::Total: {
        for (size_t i = 0; i < r_.columns.size() && i < row.cells.size(); ++i)
            if (row.cells[i].type == CellType::Amount)
                rule(colX_[i], colX_[i] + colW_[i], y + rulePad_ / 2);
        const int ty = y + rulePad_;
        const Role role = row.kind == RowKind::Total ? Role::Bold : Role::Body;
        drawCells(row, ty, role);
        if (row.kind == RowKind::Total && row.final) {
            const int lh = role == Role::Bold ? bold_ : body_;
            for (size_t i = 0; i < r_.columns.size() && i < row.cells.size(); ++i)
                if (row.cells[i].type == CellType::Amount) {
                    rule(colX_[i], colX_[i] + colW_[i], ty + lh + rulePad_ / 3);
                    rule(colX_[i], colX_[i] + colW_[i], ty + lh + (2 * rulePad_) / 3);
                }
        }
        break;
    }
    case RowKind::Note: {
        int ly = y;
        for (const std::string& line : wrap(row.text, full - in)) {
            text(s_.left + in, ly, full - in, Align::Left, Role::Body, line);
            ly += body_;
        }
        break;
    }
    case RowKind::Blank:
    case RowKind::PageBreak:
        break;
    case RowKind::Slip: {
        int ly = y;
        const int half = full / 2;
        for (size_t i = 0; i < row.fields.size(); ++i) {
            const auto& [label, value] = row.fields[i];
            const int x = s_.left + (i % 2 == 0 ? 0 : half);
            const std::string s = label.empty() ? value : label + ": " + value;
            text(x, ly, half - kColumnGap, Align::Left, Role::Body, s);
            if (i % 2 == 1 || i + 1 == row.fields.size()) ly += body_;
        }
        ly += body_ / 2;
        drawColumnHeaders(ly);
        break;
    }
    }
    y += rowHeight(row);
}

void Engine::newPage()
{
    pages_.emplace_back();
    cur_ = &pages_.back();
    y_ = bodyTop();
    // A slip continued from the previous page repeats its column headers.
    if (r_.slipMode && inSlip_) drawColumnHeaders(y_);
}

void Engine::drawPageHeaders()
{
    const int n = static_cast<int>(pages_.size());
    const int full = s_.printableWidth();
    const int title = m_.LineHeight(Role::Title);
    for (int p = 0; p < n; ++p) {
        cur_ = &pages_[static_cast<size_t>(p)];
        int y = s_.top;
        const bool own = sideLine();
        const int side = own ? full / 2 : (full - m_.TextWidth(Role::Title, r_.company)) / 2 - kColumnGap;
        if (!r_.printedOn.empty()) text(s_.left, y, side, Align::Left, Role::Body, "Printed on " + r_.printedOn);
        text(s_.left + full - side, y, side, Align::Right, Role::Body,
             "Page " + std::to_string(p + 1) + " of " + std::to_string(n));
        if (own) y += body_;
        text(s_.left, y, full, Align::Center, Role::Title, r_.company);
        y += title;
        text(s_.left, y, full, Align::Center, Role::Title, r_.title);
        y += title;
        for (const std::string& sub : r_.subtitles) {
            text(s_.left, y, full, Align::Center, Role::Body, sub);
            y += body_;
        }
        y += body_ / 2;
        if (!r_.slipMode) drawColumnHeaders(y);   // just above the body
        if (!r_.footerNote.empty())
            text(s_.left, s_.height - s_.bottom - body_, full, Align::Left, Role::Body, r_.footerNote);
    }
}

std::vector<Page> Engine::run()
{
    // Blocks of rows that must share a page.
    struct Block { size_t first, last; int height; };
    std::vector<Block> blocks;
    for (size_t i = 0; i < r_.rows.size(); ++i) {
        const Row& row = r_.rows[i];
        const int h = rowHeight(row);
        const bool join = !blocks.empty() && row.kind != RowKind::PageBreak &&
                          r_.rows[blocks.back().last].kind != RowKind::PageBreak &&
                          (gluedToNext(r_.rows[blocks.back().last].kind) || gluedToPrevious(row.kind));
        if (join) {
            blocks.back().last = i;
            blocks.back().height += h;
        } else {
            blocks.push_back({i, i, h});
        }
    }

    newPage();
    const int top = bodyTop();
    const int room = bodyLimit() - top;
    for (const Block& b : blocks) {
        const Row& first = r_.rows[b.first];
        if (first.kind == RowKind::PageBreak) {
            if (y_ > top) newPage();
            continue;
        }
        // A block that fits on a page isn't split; a taller one is, row by row.
        if (y_ + b.height > bodyLimit() && y_ > top && b.height <= room) newPage();
        for (size_t i = b.first; i <= b.last; ++i) {
            const Row& row = r_.rows[i];
            if (row.kind == RowKind::Slip) {
                inSlip_ = false;   // a new slip doesn't repeat the previous one's headers
                if (y_ > top) newPage();
            }
            if (y_ + rowHeight(row) > bodyLimit() && y_ > top) newPage();
            if (row.kind == RowKind::Blank && y_ == top) continue;
            drawRow(row, y_);
            if (row.kind == RowKind::Slip) inSlip_ = true;
        }
    }
    drawPageHeaders();
    return std::move(pages_);
}

} // namespace

PageSetup PageSetup::A4(Orientation o) { return paper(kA4W, kA4H, o); }
PageSetup PageSetup::Letter(Orientation o) { return paper(kLetterW, kLetterH, o); }

std::string FitText(const Measure& measure, Role role, std::string_view utf8, int width)
{
    std::string s(utf8);
    while (!s.empty() && measure.TextWidth(role, s) > width) {
        // Drop the last code point: continuation bytes, then its lead byte.
        while (!s.empty() && (static_cast<unsigned char>(s.back()) & 0xC0) == 0x80) s.pop_back();
        if (!s.empty()) s.pop_back();
    }
    while (!s.empty() && s.back() == ' ') s.pop_back();
    return s;
}

std::vector<Page> Layout(const Report& report, const PageSetup& setup, const Measure& measure)
{
    return Engine(report, setup, measure).run();
}

} // namespace rpt
