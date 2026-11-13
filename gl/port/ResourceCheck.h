//
// ResourceCheck.h - gl.exe /checkres: loads every Spec #1 resource from the
// running module (plan M4 exit criteria). Dialogs are created for real, so the
// custom controls must be registered first.
//
#pragma once

// It also checks that each dialog's captions fit their controls (UI refresh
// spec 4.6).
//
// gl.exe /screenshots [dir] (UI refresh spec 4.6): SaveScreenshots saves the
// frame and every top-level dialog of the check as <dir>\frame-<dpi>.png and
// <dir>\<id>-<dpi>.png, so sets taken at 100% (96) and 150% (144) can share
// a folder. The dialogs are created as the check creates them, with the
// screens' button pictures, and no data.
//
#include <Windows.h>

#include <string>

// Returns the number of failures; the report goes to a message box unless quiet.
int RunResourceCheck(HINSTANCE inst, bool quiet);

// Returns the number of screenshots that failed; a summary goes to a message
// box unless quiet. `frame` must be shown.
int SaveScreenshots(HINSTANCE inst, HWND frame, const std::wstring& dir, bool quiet);

// `wnd`, as drawn (PrintWindow, PW_RENDERFULLCONTENT), without the invisible
// resize borders, to a PNG file.
bool SaveWindowPng(HWND wnd, const std::wstring& path);
