//
// Login.h - port of gl/src/gl_acces.cpp (plan M5 task 4).
//
//   Login               TGLAccess, dialog 1158: user name + password against
//                       access.psw; loads the user's menu-access string.
//   UserAdministration  TGLAdministration, dialog 1159 (Maintenance > User
//                       Access): browse/add/modify/delete access.psw users.
//   ApplyMenuAccess     TManilaGL::WMInitMenuPopup: grays the frame-menu
//                       items the access string turns off.
//   LoginRequired       GL.INI [ACCESS] PSW=T (TGlWindow::SetupWindow).
//
// TAccessMenus (dialog 1160, the menu-access editor behind 1159's Menu
// button) needs the tbSpread grid, which Spec #1 doesn't have; the button
// says so (see UserAdministration::Menus).
//
#pragma once

#include <Windows.h>

// True if GL.INI [ACCESS] PSW is "T": the frame then asks for a login at
// startup. Otherwise there is no login and szUser is "NOLOGIN".
bool LoginRequired();

// Runs the login dialog over `frame`. IDOK: szUser, szAccessLevel and szMsg
// are set and bMenuSetup is FALSE. IDCANCEL: the user gave up; the original
// then closed the program (TGLAccess::Cancel sent CM_EXIT), which the
// caller does.
INT_PTR RunLogin(HWND frame);

// Maintenance > User Access.
void RunUserAdministration(HWND frame);

// Call on WM_INITMENUPOPUP. Once after each login (bMenuSetup FALSE), walks
// accessm in "menu" tag order alongside szMsg: '1' grays that row's menu
// command on `menu`, anything else enables it.
void ApplyMenuAccess(HMENU menu);
