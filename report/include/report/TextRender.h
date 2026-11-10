#pragma once
//
// TextRender.h - pages as plain text, for golden tests and debugging
// (spec #3 4, plan S3-M0 task 6).
//
// FixedMeasure is a fixed-pitch Measure: every code point is `charWidth`
// twips wide and every line `lineHeight` twips high, in every role.
// RenderText draws pages laid out with it on a character grid: each distinct
// y of a text becomes a line (with an empty line for each further line
// of vertical gap), a rule becomes a line of '-' (the two lines of
// a double rule print as '='), and each page starts with "--- page n ---".
// Trailing blanks are trimmed. Bold and title roles look the same.
//
#include "report/Layout.h"

#include <string>
#include <string_view>
#include <vector>

namespace rpt {

class FixedMeasure : public Measure {
public:
    FixedMeasure(int charWidth = 100, int lineHeight = 200) : cw_(charWidth), lh_(lineHeight) {}
    int TextWidth(Role, std::string_view utf8) const override;
    int LineHeight(Role) const override { return lh_; }
    int charWidth() const noexcept { return cw_; }

private:
    int cw_, lh_;
};

std::string RenderText(const std::vector<Page>& pages, const PageSetup& setup, const FixedMeasure& measure);

} // namespace rpt
