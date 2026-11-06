//
// App.cpp - gl.exe entry point (spec 2.4).
//
// M0 placeholder: an empty x64 Unicode Win32 EXE that proves the project links
// against win32fx + cb4sqlite and that its IncludePath resolves the schema
// headers. Plan M5 replaces this with the real WinMain (globals, MDI frame,
// login, company profile). The legacy reference is gl/src/gl.cpp.
//
#include <Windows.h>

#include "cb4sqlite.h"

#include "legacy_headers_begin.h"
#include "db_accts.h"
#include "legacy_headers_end.h"

int WINAPI wWinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPWSTR, _In_ int)
{
    return 0;
}
