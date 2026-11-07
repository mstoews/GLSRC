//
// LegacyIds.h - the legacy command, control and string IDs, from the same
// headers gl_new.rc included (gl/header/gl.h, gl_manil.h -> ar_id.h), so the
// ported code keeps the original symbolic names (ID_ACCTADD, ID_USER, ...).
//
// INCLUDE THIS LAST. The legacy headers #define short words as object-like
// macros (TOP, NEXT, NEW, OPEN, PRINT, MODIFY, TOOLBAR, ...), which would
// rewrite any later header that uses them as identifiers.
//
// gl.h also #defines DELETE (as ID_DELETE), which <winnt.h> already defines
// as an access right; push/pop keeps the Windows value and avoids C4005.
// gl.h's `extern char szMsg[256];` is defined in App.cpp.
//
#pragma once

#include <Windows.h>

#pragma push_macro("DELETE")
#undef DELETE
#include "gl.h"
#include "gl_manil.h"
#pragma pop_macro("DELETE")

// gl/header/gl_tooli.h, which can't be included with gl.h (it redefines
// ID_GENERAL): the module-bar fields and the toolbar height.
#define TOOLBAR_HEIGHT 30
#define ID_STATUSHELP  3222   // 2057: company name (TGlWindow::GLCreateStatusBar)
#define ID_MENUHELP    3223   // 2057: menu help (TGlWindow::WMMenuSelect); unnamed in the original
#define ID_PERIODFIELD 3224   // 2057: current period; unnamed in the original

// gl/header/gl_tooli.h toolbar buttons that gl.h doesn't define.
#define ID_CUT   3076
#define ID_PASTE 3078
#define ID_FONT  3071
#define ID_BORDER 3072
#define ID_ALIGNMENT 3080
#define ID_GRIDS 3089

// gl/header/gl_main.h: ID_GL is checked on the menu by TManilaGL::SetupWindow.
// gl/header/gl_tool.h:
#define ID_PAYABLES 2210
