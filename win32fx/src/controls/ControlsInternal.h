//
// ControlsInternal.h - shared machinery of the tb* edit superclasses.
//
// Each edit control is the system EDIT class with a window procedure in
// front of it (EditProc) and a per-window EditState in the window's extra
// bytes, after EDIT's own. What differs per class is an EditBehavior.
//
#pragma once

#include "controls/CustomControls.h"
#include "EditRules.h"

#include <string>

namespace win32fx::detail {

// Toolbox style bits (include/toolbox.h); several collide with EDIT styles.
inline constexpr DWORD tbES_UPPERCASE = 0x0008;
inline constexpr DWORD tbES_LOWERCASE = 0x0010;
inline constexpr DWORD tbES_STATIC    = 0x0080;   // read-only
inline constexpr DWORD tbES_LEFTALIGN = 0x0200;   // numbers left-aligned
inline constexpr DWORD tbES_3D        = 0x0400;   // sunken border
inline constexpr DWORD tbDS_CENTURY   = 0x1000;
inline constexpr DWORD tbDS_FORMAT    = 0xE000;   // DS_DDMONYY 0x2000, DS_DDMMYY 0x4000, DS_MMDDYY 0x6000, DS_YYMMDD 0x8000
inline constexpr DWORD tbFS_SEPARATOR = 0x2000;

struct EditState {
    DWORD tbStyle = 0;             // low word of the template style
    std::wstring mask;             // template text / EPM_SETMASK
    std::wstring picture;          // the mask ConformPicture uses (tbPic: mask; tbDate: "99/99/99")
    rules::NumberFormat number;
    rules::DateFormat date;
    PICCALLBACK callback = nullptr;
};

struct EditBehavior {
    // Derives picture/number/date from tbStyle and mask (also after EPM_SETMASK).
    void (*configure)(EditState&);
    // The text after a keystroke or paste, fitted; false rejects it.
    bool (*conform)(const EditState&, const std::wstring& text, size_t caret, std::wstring& out, size_t& caretOut);
    // WM_SETTEXT text to show instead; false shows it as given. May be null.
    bool (*setText)(const EditState&, const std::wstring& in, std::wstring& out);
    // Text to show when focus leaves; false leaves it. May be null.
    bool (*reformat)(const EditState&, const std::wstring& in, std::wstring& out);
    bool alignRight;               // numbers: right-aligned unless ES_LEFTALIGN
    bool caseStyles;               // ES_UPPERCASE / ES_LOWERCASE apply
};

// The window procedure body shared by tbPic, tbDate, tbFloat and tbInteger.
LRESULT EditProc(const EditBehavior& b, HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

// Registers `name` as an EDIT superclass with window procedure `proc`
// (nothing to do if it is already registered). False on failure.
bool RegisterEditClass(HINSTANCE inst, const wchar_t* name, WNDPROC proc);

// The state of `hwnd` if its class's window procedure is `proc` (one of
// ours), else null: other windows' extra bytes are never read.
EditState* GetEditState(HWND hwnd, WNDPROC proc) noexcept;

bool RegisterPicEdit(HINSTANCE inst);
bool RegisterDateEdit(HINSTANCE inst);
bool RegisterFloatEdit(HINSTANCE inst);
bool RegisterIntEdit(HINSTANCE inst);
bool RegisterSuperBtn(HINSTANCE inst);
bool RegisterGrid(HINSTANCE inst);

std::wstring WindowText(HWND hwnd);

} // namespace win32fx::detail
