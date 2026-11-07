//
// ResourceCheck.h - gl.exe /checkres: loads every Spec #1 resource from the
// running module (plan M4 exit criteria). Dialogs are created for real, so the
// custom controls must be registered first.
//
#pragma once

#include <Windows.h>

// Returns the number of failures; the report goes to a message box unless quiet.
int RunResourceCheck(HINSTANCE inst, bool quiet);
