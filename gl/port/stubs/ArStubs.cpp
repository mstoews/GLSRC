//
// ArStubs.cpp - see Stubs.h.
//
#include "Stubs.h"

#include <cwchar>

void ArStubs::Show(HWND owner, const wchar_t* item)
{
    wchar_t text[512];
    swprintf(text, sizeof text / sizeof text[0],
             L"%ls%lsAccounts Receivable is not yet ported in Spec #1.\n\nIt arrives with the Accounts Receivable port (Spec #6).",
             item && *item ? item : L"", item && *item ? L"\n\n" : L"");
    MessageBoxW(owner, text, L"TIAS", MB_OK | MB_ICONINFORMATION);
}
