//
// YearEnd.cpp - see YearEnd.h.
//
// Translation notes: the original said nothing after closing the year, and
// nothing when it couldn't. The port reports the amount moved into Retained
// Earnings, or names the type-R accounts it found and points to the Chart
// of Accounts.
//
#include "YearEnd.h"

#include "Globals.h"
#include "Vouchers.h"

#include "win32fx.h"

#include <exception>
#include <string>

void RunYearEnd(HWND frame)
{
    // "Do you want to close the current year?\nAll current year entires will be cleared!"
    if (MessageBoxW(frame, fx::LoadResString(hInst, 8111).c_str(), szApp, MB_YESNO | MB_ICONQUESTION) != IDYES)
        return;
    YearEndResult r;
    try {
        HCURSOR old = SetCursor(LoadCursorW(nullptr, IDC_WAIT));
        r = YearEnd(cb);
        SetCursor(old);
    } catch (const std::exception& e) {
        MessageBoxW(frame, (L"Year end failed, and nothing was changed.\n\n" + fx::Utf8ToWide(e.what())).c_str(),
                    szApp, MB_ICONSTOP);
        return;
    }
    if (!r.closed) {
        std::wstring found;
        for (const RetainedAccount& a : r.retained) {
            found += L"\n    " + fx::Utf8ToWide(a.account);
            if (!a.dept.empty()) found += L" / " + fx::Utf8ToWide(a.dept);
        }
        const std::wstring text =
            L"Year end needs exactly one Retained Earnings account (type R), and " +
            (r.retained.empty() ? std::wstring(L"there is none.") : L"these accounts have type R:" + found) +
            L"\n\nCorrect the account types in Accounts > Chart of Accounts, then run Year End again. Nothing was "
            L"changed.";
        MessageBoxW(frame, text.c_str(), szApp, MB_ICONEXCLAMATION);
        return;
    }
    const RetainedAccount& re = r.retained.front();
    MessageBoxW(frame,
                fx::Format(L"The year is closed. The income statement balances were moved into Retained Earnings, "
                           L"account %s, whose balance is now %.2f; the period amounts are in the previous-year "
                           L"columns. The new year's opening balances are the closing balances; the year's %d "
                           L"posted lines moved to the archive, and its %d posted vouchers are closed (Recover "
                           L"and Transaction Inquiry show only the new year's).",
                           fx::Utf8ToWide(re.account).c_str(), r.transferred, r.archived, r.closedVouchers)
                    .c_str(),
                szApp, MB_ICONINFORMATION);
}
