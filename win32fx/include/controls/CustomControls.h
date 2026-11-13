//
// CustomControls.h - the Toolbox controls the legacy dialogs use (spec 3.6).
//
// RegisterCustomControls() registers window classes under the original
// names, so the dialog templates in gl.rc create them unchanged:
//
//   tbPic      edit with a picture mask    (PicEdit.cpp)
//   tbDate     date edit                   (DateEdit.cpp)
//   tbFloat    decimal edit                (FloatEdit.cpp)
//   tbInteger  integer edit                (IntEdit.cpp)
//   tbSuperBtn push button with a picture  (SuperBtn.cpp)
//   tbSpread   grid                        (Grid.cpp; fx::Grid in Grid.h)
//   tbTime     a tbPic, only so templates load (PicEdit.cpp): the one
//              in the GL (dialog 4004's 1035) is never set and is hidden
//
// The edits superclass the system EDIT control and the button superclasses
// BUTTON: the dialog code talks to them with the usual WM_SETTEXT /
// WM_GETTEXT / EN_CHANGE / BN_CLICKED. A template's text is the control's
// mask ("XXXXXXX", "(999) 999 - 9999", "9999999999999.99"), not its initial
// text, as with Toolbox; the Toolbox style bits are translated to EDIT/BUTTON
// styles (several share values with unrelated EDIT styles: FS_SEPARATOR is
// ES_NUMBER's bit). See win32fx/README.md for what each control does.
//
// The Toolbox calls and messages the legacy code sends are below, with their
// original names.
//
#pragma once

#include <Windows.h>

#include <string>

namespace win32fx {

// Registers all seven classes (CS_GLOBALCLASS) for `inst`; calls
// InitCommonControlsEx first. False if any registration failed. Safe to call
// more than once.
bool RegisterCustomControls(HINSTANCE inst);

// The date in a tbDate control as YYYYMMDD (cb4sqlite's date format), or ""
// if it is blank or not a valid date. Two-digit years: 00-49 are 20xx, 50-99 19xx.
std::wstring DateGetYmd(HWND ctl);

// The modern icon that replaces bitmap `resource` (UI refresh spec 4.2, D2):
// icon "P<id>" for a numeric resource, "P<name>" for a named one, loaded at
// `sizePx` with LoadIconWithScaleDown. Null if `inst` has no such icon. The
// caller destroys it (DestroyIcon). tbSuperBtn's SBM_SETPICT uses it.
HICON LoadModernIcon(HINSTANCE inst, LPCWSTR resource, int sizePx);

} // namespace win32fx

// ---- Toolbox compatibility (include/toolbox.h names and values) -----------

// Picture edit messages (tbPic, and tbDate's mask).
#ifndef EPM_GETMASK
#define EPM_GETMASK     (WM_USER + 38)   // lParam: wchar_t* buffer, wParam: its size
#define EPM_SETMASK     (WM_USER + 39)   // lParam: const wchar_t* mask
#define EPM_SETCALLBACK (WM_USER + 40)   // lParam: PICCALLBACK, null to remove
#endif

// Called for each character typed into a tbPic: return the character to
// insert (possibly changed), or 0 to reject it.
typedef WORD(CALLBACK* PICCALLBACK)(HWND ctl, short id, short pos, wchar_t ch);

// tbSuperBtn messages. Only SBM_SETPICT does something; the rest are
// accepted and ignored (the look is the themed system button).
#ifndef SBM_SETPICT
#define SBM_SETCOLOR         (WM_USER + 50)
#define SBM_SETPICT          (WM_USER + 51)   // wParam: SUPERBTN_PICT_*, lParam: resource ID / name / handle
#define SBM_SETPICT_DISABLED (WM_USER + 52)
#define SBM_SETPICT_BTNDOWN  (WM_USER + 53)
#define SBM_GETCOLOR         (WM_USER + 54)
#define SBM_SETBUTTONTYPE    (WM_USER + 55)
#define SBM_GETBUTTONSTATE   (WM_USER + 56)
#define SBM_SETBUTTONSTATE   (WM_USER + 57)
#define SBM_SETSHADOWSIZE    (WM_USER + 58)
#define SBM_GETSHADOWSIZE    (WM_USER + 59)
#define SBM_SETSTRETCH       (WM_USER + 60)
#define SBM_GETSTRETCH       (WM_USER + 61)
#define SUPERBTN_PICT_NONE   0
#define SUPERBTN_PICT_ICON   0x0001           // BT_ICON
#define SUPERBTN_PICT_BITMAP 0x0002           // BT_BITMAP
#define SUPERBTN_PICT_HANDLE 0x0004           // BT_HANDLE: lParam is an HBITMAP (not owned)
#endif

// Values of tbFloat / tbInteger controls. Get returns FALSE (and 0) when the
// control is blank or its text isn't a number; thousands separators are ignored.
BOOL FloatGetValue(HWND ctl, double* value);
BOOL FloatSetValue(HWND ctl, double value);
BOOL IntGetValue(HWND ctl, LONG* value);
BOOL IntSetValue(HWND ctl, LONG value);

// tbSpread notifications to the parent (WM_COMMAND code; see Grid.h). The
// values are those of the Toolbox SSM_* messages they replace.
#ifndef GRN_PICK
#define GRN_PICK        0x0501   // SSM_DBLCLK
#define GRN_CELLCHANGED 0x0508   // SSM_DATACHANGE
#define GRN_SELCHANGE   0x050A   // SSM_CLICK
#define GRN_KEYDOWN     0x050B   // SSM_KEYDOWN
#define GRN_RCLICK      0x050D   // SSM_RBUTTON
#endif

#define FloatGetDlgItemValue(h, i, v) FloatGetValue(GetDlgItem((h), (i)), (v))
#define FloatSetDlgItemValue(h, i, v) FloatSetValue(GetDlgItem((h), (i)), (v))
#define IntGetDlgItemValue(h, i, v)   IntGetValue(GetDlgItem((h), (i)), (v))
#define IntSetDlgItemValue(h, i, v)   IntSetValue(GetDlgItem((h), (i)), (v))
