//
// ArReceipts.h - Clear Receipts (spec #6 5, D8-D10; plan S6-M4): port of
// TARCreatePayment and TARClear (gl/src/ar_payrq.cpp) on dialogs 1156 and
// 1155, gl_new.rc's: Transactions > Clear Receipts (ID_ARPAYMENT, 4054).
//
// The list shows the invoices with something owed (View All: the settled
// ones too). Double-click one to record a receipt in 1155: in full, in part,
// or with an early-payment discount, journalled at once (D9). Receipts
// shows the invoice's receipts and reverses the latest (D10). The record
// operations are in Ar.h.
//
#pragma once

#include <Windows.h>

INT_PTR RunArReceipts(HWND frame, bool closeAtOnce = false);
