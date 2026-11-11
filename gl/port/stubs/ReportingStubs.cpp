//
// ReportingStubs.cpp - see Stubs.h.
//
#include "Stubs.h"

#include <cwchar>

void ReportingStubs::Show(HWND owner, const wchar_t* item, const wchar_t* module, int spec)
{
    wchar_t text[512];
    swprintf(text, sizeof text / sizeof text[0],
             L"%ls%lsThis %ls report is not yet ported.\n\nIt arrives with the %ls port (Spec #%d).",
             item && *item ? item : L"", item && *item ? L"\n\n" : L"", module, module, spec);
    MessageBoxW(owner, text, L"TIAS", MB_OK | MB_ICONINFORMATION);
}
