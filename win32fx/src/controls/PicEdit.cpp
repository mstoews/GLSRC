//
// PicEdit.cpp - tbPic: an edit that accepts what its picture mask allows.
//
// The mask is the template text ("XXXXXXX", "NNNN", "(999) 999 - 9999") or
// EPM_SETMASK; EditRules.h lists the mask characters. Literals are inserted
// as the user types; the text is at most as long as the mask. EPM_SETCALLBACK
// installs a per-character callback, as gl_accts.cpp does. WM_SETTEXT is
// shown as given (database values are never refused).
//

#include "ControlsInternal.h"

namespace win32fx::detail {

namespace {

void configure(EditState& s) { s.picture = s.mask; }

bool conform(const EditState& s, const std::wstring& text, size_t caret, std::wstring& out, size_t& caretOut)
{
    return rules::ConformPicture(s.picture, text, caret, out, caretOut);
}

const EditBehavior kBehavior{configure, conform, nullptr, nullptr, false, true};

LRESULT CALLBACK PicEditProc(HWND h, UINT m, WPARAM w, LPARAM l) { return EditProc(kBehavior, h, m, w, l); }

} // namespace

bool RegisterPicEdit(HINSTANCE inst)
{
    // tbTime: the GL's only one (dialog 4004's 1035) is never set and the port
    // hides it; registered as a tbPic so the template creates (spec #4 S4-M2).
    return RegisterEditClass(inst, L"tbPic", PicEditProc) && RegisterEditClass(inst, L"tbTime", PicEditProc);
}

} // namespace win32fx::detail
