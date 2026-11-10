//
// GdiMeasure.cpp - see report/win32/Gdi.h.
//
#include "report/win32/Gdi.h"

#include <cmath>

namespace rpt::win32 {

namespace {

constexpr int kTwipsPerInch = 1440;
constexpr int kExtScale = 100;   // SetTwipsMapping keeps two decimals of a fractional zoom

struct FontSpec {
    int heightTwips;   // character height (points * 20)
    int weight;
};

// Indexed by Role: Title, Heading, Body, Bold.
constexpr FontSpec kFonts[4] = {
    {12 * 20, FW_BOLD},
    {9 * 20, FW_BOLD},
    {9 * 20, FW_NORMAL},
    {9 * 20, FW_BOLD},
};

class SelectGuard {
public:
    SelectGuard(HDC dc, HGDIOBJ obj) : dc_(dc), old_(SelectObject(dc, obj)) {}
    ~SelectGuard() { SelectObject(dc_, old_); }
    SelectGuard(const SelectGuard&) = delete;
    SelectGuard& operator=(const SelectGuard&) = delete;

private:
    HDC dc_;
    HGDIOBJ old_;
};

} // namespace

std::wstring Widen(std::string_view utf8)
{
    if (utf8.empty()) return {};
    const int n = MultiByteToWideChar(CP_UTF8, 0, utf8.data(), static_cast<int>(utf8.size()), nullptr, 0);
    std::wstring out(static_cast<size_t>(n), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, utf8.data(), static_cast<int>(utf8.size()), out.data(), n);
    return out;
}

void SetTwipsMapping(HDC dc, double pxPerInchX, double pxPerInchY, POINT originPx)
{
    SetMapMode(dc, MM_ANISOTROPIC);
    SetWindowExtEx(dc, kTwipsPerInch * kExtScale, kTwipsPerInch * kExtScale, nullptr);
    SetViewportExtEx(dc, static_cast<int>(std::lround(pxPerInchX * kExtScale)),
                     static_cast<int>(std::lround(pxPerInchY * kExtScale)), nullptr);
    SetWindowOrgEx(dc, 0, 0, nullptr);
    SetViewportOrgEx(dc, originPx.x, originPx.y, nullptr);
}

FontSet::FontSet()
{
    for (int i = 0; i < 4; ++i) {
        fonts_[i] = CreateFontW(-kFonts[i].heightTwips, 0, 0, 0, kFonts[i].weight, FALSE, FALSE, FALSE,
                                DEFAULT_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                                VARIABLE_PITCH | FF_SWISS, L"Arial");
    }
}

FontSet::~FontSet()
{
    for (HFONT f : fonts_)
        if (f) DeleteObject(f);
}

GdiMeasure::GdiMeasure(HDC dc) : dc_(dc)
{
    if (!dc_) {
        dc_ = CreateCompatibleDC(nullptr);
        screen_ = true;
    }
    SetTwipsMapping(dc_, GetDeviceCaps(dc_, LOGPIXELSX), GetDeviceCaps(dc_, LOGPIXELSY));
    for (int i = 0; i < 4; ++i) {
        SelectGuard font(dc_, fonts_.get(static_cast<Role>(i)));
        TEXTMETRICW tm{};
        GetTextMetricsW(dc_, &tm);
        lineHeight_[i] = tm.tmHeight + tm.tmExternalLeading;
    }
}

GdiMeasure::~GdiMeasure()
{
    if (dc_) DeleteDC(dc_);
}

int GdiMeasure::TextWidth(Role role, std::string_view utf8) const
{
    if (utf8.empty()) return 0;
    const std::wstring text = Widen(utf8);
    SelectGuard font(dc_, fonts_.get(role));
    SIZE size{};
    GetTextExtentPoint32W(dc_, text.c_str(), static_cast<int>(text.size()), &size);
    return size.cx;
}

void DrawPage(HDC dc, const Page& page, const FontSet& fonts)
{
    const int oldBk = SetBkMode(dc, TRANSPARENT);
    const COLORREF oldColor = SetTextColor(dc, RGB(0, 0, 0));
    const UINT oldAlign = GetTextAlign(dc);
    {
        SelectGuard font(dc, fonts.get(Role::Body));
        for (const TextOp& t : page.texts) {
            if (t.text.empty()) continue;
            SelectObject(dc, fonts.get(t.role));
            int x = t.x;
            UINT align = TA_TOP | TA_NOUPDATECP;
            switch (t.align) {
            case Align::Left:   align |= TA_LEFT; break;
            case Align::Right:  align |= TA_RIGHT;  x = t.x + t.width; break;
            case Align::Center: align |= TA_CENTER; x = t.x + t.width / 2; break;
            }
            SetTextAlign(dc, align);
            const std::wstring text = Widen(t.text);
            ExtTextOutW(dc, x, t.y, 0, nullptr, text.c_str(), static_cast<UINT>(text.size()), nullptr);
        }
    }
    if (!page.rules.empty()) {
        // Half a point wide; GDI draws at least one device pixel.
        HPEN pen = CreatePen(PS_SOLID, 10, RGB(0, 0, 0));
        {
            SelectGuard sel(dc, pen);
            for (const RuleOp& r : page.rules) {
                MoveToEx(dc, r.x1, r.y, nullptr);
                LineTo(dc, r.x2, r.y);
            }
        }
        DeleteObject(pen);
    }
    SetTextAlign(dc, oldAlign);
    SetTextColor(dc, oldColor);
    SetBkMode(dc, oldBk);
}

} // namespace rpt::win32
