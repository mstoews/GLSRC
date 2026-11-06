//
// IntEdit.cpp - tbInteger: a whole number.
//
// The template text is the mask: "9999" allows four digits (no 9s: ten). A
// leading '-' is allowed. IS_SPIN / IS_SPINWRAP are not implemented (no
// up-down arrows). IntGetValue / IntSetValue are the Toolbox calls.
//

#include "ControlsInternal.h"

#include <climits>
#include <cmath>
#include <string>

namespace win32fx::detail {

namespace {

void configure(EditState& s)
{
    s.number = rules::NumberFormatFromMask(s.mask, {10, 0, false});
    s.number.decimals = 0;
    s.number.separators = false;
}

bool conform(const EditState& s, const std::wstring& text, size_t caret, std::wstring& out, size_t& caretOut)
{
    if (!rules::AcceptNumber(s.number, text)) return false;
    out = text;
    caretOut = caret;
    return true;
}

bool format(const EditState& s, const std::wstring& in, std::wstring& out)
{
    if (in.find_first_not_of(L' ') == std::wstring::npos) { out.clear(); return true; }
    double v = 0;
    if (!rules::ParseNumber(in, v)) return false;
    out = rules::FormatNumber(v, s.number);
    return true;
}

const EditBehavior kBehavior{configure, conform, format, format, true, false};

LRESULT CALLBACK IntEditProc(HWND h, UINT m, WPARAM w, LPARAM l) { return EditProc(kBehavior, h, m, w, l); }

} // namespace

bool RegisterIntEdit(HINSTANCE inst) { return RegisterEditClass(inst, L"tbInteger", IntEditProc); }

} // namespace win32fx::detail

BOOL IntGetValue(HWND ctl, LONG* value)
{
    double v = 0;
    const bool ok = ctl && win32fx::rules::ParseNumber(win32fx::detail::WindowText(ctl), v) &&
                    v == std::floor(v) && v >= LONG_MIN && v <= LONG_MAX;
    if (value) *value = ok ? static_cast<LONG>(v) : 0;
    return ok ? TRUE : FALSE;
}

BOOL IntSetValue(HWND ctl, LONG value)
{
    return SetWindowTextW(ctl, std::to_wstring(value).c_str());
}
