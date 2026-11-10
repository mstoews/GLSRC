//
// TransactionInquiry.h - port of TGLTransInq (gl/src/gl_trans.cpp) on
// dialog 1029: Accounts > General Ledger Voucher Inquiry (10072). Spec #2
// design 2.6, plan S2-M5.
//
// Type an account and dept, or pick one in the account list (Search shows
// and hides it), and Set Data lists the account's posted voucher lines
// between its opening balance and its balance. The lookup is
// AccountTransactions (Vouchers.h).
//
#pragma once

#include <Windows.h>

INT_PTR RunTransactionInquiry(HWND frame);

// gl.exe /stress: opens and closes it `times` times and returns the change
// in GDI + USER objects (-1 if it failed to open).
int StressTransactionInquiry(int times);
