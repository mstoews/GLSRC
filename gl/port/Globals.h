//
// Globals.h - the original module-level globals (spec 3.7), defined in
// App.cpp. They stay plain globals; win32fx never touches them.
//
// Text that comes from or goes back to the database stays char/UTF-8 (the
// cb4sqlite boundary, plan per-port checklist 5): szUser, szAccessLevel and
// szMsg. Text that only reaches Win32 is WCHAR (spec 5.1 tier 3).
//
#pragma once

#include <Windows.h>

#include "cb4sqlite.h"   // extern Code4 cb

extern HINSTANCE hInst;
extern HMENU HGLMenu;      // MANILA, the frame menu
extern HMENU HGLExMenu;    // GLE, swapped in for an MDI child that asks for it (ManilaGL.cpp)
extern HMENU HGLSubMenu;   // GLSUB, the right-click popup (the original loaded it per click)
extern WCHAR szApp[20];    // "TIAS": the message-box caption
extern WCHAR CurDir[MAX_PATH];   // the company database, from GLDR.INI (was the .dbf directory)

// gl_acces.cpp
extern char szUser[11];          // the logged-in user, or "NOLOGIN" without [ACCESS] PSW=T
extern char szAccessLevel[11];
extern BOOL bMenuSetup;          // FALSE: the next WM_INITMENUPOPUP applies the menu-access string
extern BOOL bUpdate;             // TAccessMenus changed the string
// szMsg (char[256], declared by gl.h): the user's menu-access string, one '0'/'1' per accessm row.
