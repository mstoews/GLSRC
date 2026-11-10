#pragma once
//
// Gdi.h - measuring and drawing report pages with GDI (spec #3 4.2, 4.5;
// plan S3-M1 task 1).
//
// Pages are in twips (Layout.h). A DC is put in "twips mapping" with
// SetTwipsMapping: MM_ANISOTROPIC, one logical unit per twip, y down, the
// paper's top-left corner at a chosen device point. Fonts are created with
// heights in twips, so the same FontSet serves the printer, the preview at
// any zoom, and the measuring DC.
//
//   rpt::win32::GdiMeasure m(rpt::win32::CreatePrinterIC(o));   // or GdiMeasure() for the screen
//   auto pages = rpt::Layout(report, setup, m);
//   rpt::win32::SetTwipsMapping(dc, dpiX, dpiY, origin);
//   rpt::win32::DrawPage(dc, pages[0], fonts);
//
// The four roles are the house style proposed in spec #3 14 Q2 (settled
// from the S3-M2 sample pages): Arial, a 12 pt bold title, 9 pt body, 9 pt
// bold for headings and bold cells.
//
#include "report/Layout.h"

#include <Windows.h>

#include <string>
#include <string_view>

namespace rpt::win32 {

// UTF-8 -> UTF-16 (invalid sequences become U+FFFD).
std::wstring Widen(std::string_view utf8);

// Puts `dc` in twips mapping. pxPerInchX/Y: device units per inch at the
// wanted zoom (the DC's LOGPIXELSX/Y for 100 %). originPx: where the paper's
// top-left corner falls, in device units.
void SetTwipsMapping(HDC dc, double pxPerInchX, double pxPerInchY, POINT originPx = {0, 0});

// The fonts of the four roles, heights in twips.
class FontSet {
public:
    FontSet();
    ~FontSet();
    FontSet(const FontSet&) = delete;
    FontSet& operator=(const FontSet&) = delete;

    HFONT get(Role role) const noexcept { return fonts_[static_cast<int>(role)]; }

private:
    HFONT fonts_[4] = {};
};

// A Measure over a DC in its own resolution: a printer's information
// context, or (null) a DC compatible with the screen. It owns the DC.
class GdiMeasure : public Measure {
public:
    explicit GdiMeasure(HDC dc = nullptr);
    ~GdiMeasure() override;
    GdiMeasure(const GdiMeasure&) = delete;
    GdiMeasure& operator=(const GdiMeasure&) = delete;

    int TextWidth(Role role, std::string_view utf8) const override;
    int LineHeight(Role role) const override { return lineHeight_[static_cast<int>(role)]; }

    // True when measuring with the screen (no printer).
    bool onScreen() const noexcept { return screen_; }

private:
    HDC dc_ = nullptr;
    bool screen_ = false;
    FontSet fonts_;
    int lineHeight_[4] = {};
};

// Draws one page in black on whatever is under it. `dc` must be in twips
// mapping with the paper's corner at the origin.
void DrawPage(HDC dc, const Page& page, const FontSet& fonts);

} // namespace rpt::win32
