//
// Stubs.h - "not yet ported" placeholders for the menu commands outside
// Spec #1 (plan M5 task 6, spec 1.2). Each Show() is one MessageBoxW and
// depends on nothing else: no cb4sqlite, no win32fx. `item` is the menu text
// (or empty), shown so the user knows which command it was.
//
// ManilaGL.cpp decides which stub a command goes to (kApCommands, ...).
//
#pragma once

#include <Windows.h>

namespace ApStubs {
void Show(HWND owner, const wchar_t* item);          // Accounts Payable (Spec #5)
}
namespace ArStubs {
void Show(HWND owner, const wchar_t* item);          // Accounts Receivable (Spec #6)
}
namespace ReportingStubs {
void Show(HWND owner, const wchar_t* item);          // Crystal Reports replacement (Spec #3)
}
