//
// ChartOfAccounts.h - port of gl/src/gl_accts.cpp: TGLAcctsAdd and
// TGLAcctsDelete on dialog 1010, "General Ledger Account Maintenance".
//
//   RunChartOfAccounts   Accounts > Chart of Accounts, GLSUB > Add Accounts (ID_ACCTADD)
//   RunDeleteAccounts    GLSUB > Delete Accounts (ID_ACCTDEL): the same dialog,
//                        with "&Delete" on the OK button
//   RunBudgetData        Accounts > Budget Data (ID_ACCTBUDGET, 10074):
//                        TGLBudget, the same screen on dialog 1017 with a
//                        grid of the 12 budget amounts, which OK saves
//                        (Spec #2 S2-M5)
//
// All three are modal (TGlWindow::GLAcctAdd used ExecDialog), with the account
// list (AccountList.h) beside them.
//
#pragma once

#include <Windows.h>

INT_PTR RunChartOfAccounts(HWND frame);
INT_PTR RunDeleteAccounts(HWND frame);
INT_PTR RunBudgetData(HWND frame);

// gl.exe /stress: opens and closes the Chart of Accounts `times` times, as
// Add, Delete and Budget Data in turn (plan
// risk P6, the per-port stress smoke test) and reports the change in the
// process's GDI and USER object counts. Returns the change, 0 if none.
int StressChartOfAccounts(int times);
