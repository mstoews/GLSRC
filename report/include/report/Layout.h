#pragma once
//
// Layout.h - pages from a Report (spec #3 4.2).
//
// Everything is in twips (1/1440 inch), measured with a Measure: on Windows
// the printer's DC (report/win32), in tests a fixed-pitch fake. The preview
// and the printer draw the same Pages, so the preview is what prints.
//
// Each page has:
//   - the header: the company (centred, with "Printed on ..." at the left and
//     "Page n of m" at the right), the title, the subtitles, then the column
//     headers with a rule under them (in slip mode the column headers follow
//     each Slip row instead, and repeat at the top of a slip's later pages);
//   - the rows that fit;
//   - the footer note at the bottom left.
//
// Pagination keeps a Heading, GroupHeader or Slip row with the row after it,
// and a GroupFooter, Subtotal or Total with the row before it, so a page
// never ends on a heading or starts with a total. A PageBreak, and every
// Slip row but the first, start a new page; a Blank is dropped at the top of
// a page. Text cells that don't fit their column are cut short; numbers that
// don't fit print "###". Only Note rows wrap.
//
#include "report/Report.h"

#include <string>
#include <string_view>
#include <vector>

namespace rpt {

struct PageSetup {
    int width = 0, height = 0;                      // the paper
    int left = 0, top = 0, right = 0, bottom = 0;   // margins

    int printableWidth() const noexcept { return width - left - right; }
    int printableHeight() const noexcept { return height - top - bottom; }

    // A4 / US Letter with 0.75 inch margins, for when there is no printer.
    static PageSetup A4(Orientation o = Orientation::Portrait);
    static PageSetup Letter(Orientation o = Orientation::Portrait);
};

class Measure {
public:
    virtual ~Measure() = default;
    // Width of UTF-8 text in the role's font, in twips.
    virtual int TextWidth(Role role, std::string_view utf8) const = 0;
    // Line height (ascent + descent + leading) of the role's font, in twips.
    virtual int LineHeight(Role role) const = 0;
};

// Text drawn in the box [x, x + width) from y (the top of the line) down,
// aligned in the box. Already cut to fit.
struct TextOp {
    int         x = 0, y = 0, width = 0;
    Align       align = Align::Left;
    Role        role = Role::Body;
    std::string text;
};

// A horizontal hairline from x1 to x2 at y.
struct RuleOp {
    int x1 = 0, x2 = 0, y = 0;
};

struct Page {
    std::vector<TextOp> texts;
    std::vector<RuleOp> rules;
};

// Spacing, in twips.
inline constexpr int kIndentStep = 240;   // one indent level
inline constexpr int kColumnGap = 144;    // between columns

std::vector<Page> Layout(const Report& report, const PageSetup& setup, const Measure& measure);

// Cuts UTF-8 text, whole code points at a time, until it fits `width`.
std::string FitText(const Measure& measure, Role role, std::string_view utf8, int width);

} // namespace rpt
