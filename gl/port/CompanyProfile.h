//
// CompanyProfile.h - port of gl/src/gl_profi.cpp (plan M5 task 5).
//
//   CompanyProfile   TGLProfile, dialog 1021 (File > Company Profile): the
//                    gl_prof record, the fiscal-year dates from gl_pds, the
//                    data location and GL.INI [WITHHOLDING] TAX.
//   SetPeriodDates   TGLSetPeriodDates, dialog 1028 (Profile > Periods).
//   SetAccounts      TGLSetAccounts, dialog 1033 (Profile > Accounts): the
//                    eight GL control accounts in gl_setat (TYPE="GL").
//   CashAccounts     TAPCashAccts, dialog 1034 (Accounts > Cash): the bank
//                    accounts in gl_setat (TYPE="CA"), with TAPCashAcctsList
//                    (dialog 1011, an fx::Grid) beside it. Double-click or
//                    Enter on a row shows that account in 1034;
//                    double-clicking 1034 itself steps to the next one.
//
// Not ported:
//   - TGLProfile::GetCalc. A right-click on Company Profile asked "Not
//     fooling around here..." and, on Yes, zapped every table listed in the
//     `clear` table and zeroed every balance in glaccts. It is a destructive
//     tool outside Spec #1 and has no menu item; it is left out on purpose.
//
#pragma once

#include <Windows.h>

// Returns the dialog result: IDOK after Save, IDCANCEL after Close.
INT_PTR RunCompanyProfile(HWND frame);
