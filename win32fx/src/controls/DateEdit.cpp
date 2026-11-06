//
// DateEdit.cpp - tbDate: a date typed as digits, separators inserted.
//
// The format comes from the Toolbox style: DS_MMDDYY (and no format bits,
// which every legacy use fills with "MM/DD/YY" text), DS_DDMMYY or DS_YYMMDD,
// with a four-digit year under DS_CENTURY. DS_DDMONYY (unused by the legacy
// resources) is treated as MM/DD/YY. '-' and '.' are taken as '/'.
//
// The control's text is the displayed date, as the original's was: legacy
// code reads it with GetField and parses it with Date4::assign(.., "MM/DD/YY").
// WM_SETTEXT also accepts cb4sqlite's YYYYMMDD and shows it in the control's
// format, and DateGetYmd() reads the date back as YYYYMMDD.
//

#include "ControlsInternal.h"

namespace win32fx::detail {

namespace {

void configure(EditState& s)
{
    s.date.century = (s.tbStyle & tbDS_CENTURY) != 0;
    switch (s.tbStyle & tbDS_FORMAT) {
    case 0x4000: s.date.order = rules::DateOrder::DMY; break;
    case 0x8000: s.date.order = rules::DateOrder::YMD; break;
    default:     s.date.order = rules::DateOrder::MDY; break;
    }
    s.picture = rules::DateMask(s.date);
}

bool conform(const EditState& s, const std::wstring& text, size_t caret, std::wstring& out, size_t& caretOut)
{
    std::wstring t = text;
    for (wchar_t& c : t)
        if (c == L'-' || c == L'.') c = L'/';
    return rules::ConformPicture(s.picture, t, caret, out, caretOut);
}

bool setText(const EditState& s, const std::wstring& in, std::wstring& out)
{
    return rules::YmdToDate(in, s.date, out);
}

const EditBehavior kBehavior{configure, conform, setText, nullptr, false, false};

LRESULT CALLBACK DateEditProc(HWND h, UINT m, WPARAM w, LPARAM l) { return EditProc(kBehavior, h, m, w, l); }

} // namespace

bool RegisterDateEdit(HINSTANCE inst) { return RegisterEditClass(inst, L"tbDate", DateEditProc); }

} // namespace win32fx::detail

std::wstring win32fx::DateGetYmd(HWND ctl)
{
    std::wstring ymd;
    const detail::EditState* st = detail::GetEditState(ctl, detail::DateEditProc);
    if (!st || !rules::DateToYmd(detail::WindowText(ctl), st->date, ymd)) return {};
    return ymd;
}
