//
// MessageLoop.h - the application message loop.
//
// Each message goes, in order, to: the MDI system accelerators (if an MDI
// client is given), the accelerator table (if given), IsDialogMessageW for
// each live modeless Dialog (they register themselves), and finally
// TranslateMessage / DispatchMessageW.
//
#pragma once

#include <Windows.h>

namespace win32fx {

// Runs until WM_QUIT; returns its wParam.
int RunMessageLoop(HWND frame = nullptr, HWND mdiClient = nullptr, HACCEL accel = nullptr);

// One message, already retrieved by GetMessageW / PeekMessageW: true if a
// dialog or accelerator consumed it (don't Translate/Dispatch it).
bool PreTranslate(MSG& msg, HWND frame = nullptr, HWND mdiClient = nullptr, HACCEL accel = nullptr);

// Used by Dialog for modeless dialogs.
void AddModeless(HWND dialog);
void RemoveModeless(HWND dialog);

} // namespace win32fx
