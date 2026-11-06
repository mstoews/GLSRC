//
// FloatEdit.cpp - tbFloat: a decimal number.
//
// The template text is the mask: "9999999999999.99" allows 13 digits before
// the point and 2 after (no 9s: 13 and 2). FS_SEPARATOR shows thousands
// separators. Text set with WM_SETTEXT (a padded cb4sqlite value such as
// "   1234.5") and text left by the user are shown with the mask's decimals;
// text that isn't a number is shown as given. ES_STATIC makes it read-only.
// FloatGetValue / FloatSetValue are the Toolbox calls dGetField uses.
//

#include "ControlsInternal.h"

namespace win32fx::detail {

namespace {

void configure(EditState& s)
{
    s.number = rules::NumberFormatFromMask(s.mask, {13, 2, false});
    s.number.separators = (s.tbStyle & tbFS_SEPARATOR) != 0;
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

LRESULT CALLBACK FloatEditProc(HWND h, UINT m, WPARAM w, LPARAM l) { return EditProc(kBehavior, h, m, w, l); }

} // namespace

bool RegisterFloatEdit(HINSTANCE inst) { return RegisterEditClass(inst, L"tbFloat", FloatEditProc); }

} // namespace win32fx::detail

BOOL FloatGetValue(HWND ctl, double* value)
{
    double v = 0;
    const bool ok = ctl && win32fx::rules::ParseNumber(win32fx::detail::WindowText(ctl), v);
    if (value) *value = ok ? v : 0;
    return ok ? TRUE : FALSE;
}

BOOL FloatSetValue(HWND ctl, double value)
{
    using namespace win32fx;
    const detail::EditState* st = detail::GetEditState(ctl, detail::FloatEditProc);
    const rules::NumberFormat fmt = st ? st->number : rules::NumberFormat{13, 2, false};
    return SetWindowTextW(ctl, rules::FormatNumber(value, fmt).c_str());
}
