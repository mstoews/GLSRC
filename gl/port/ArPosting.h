//
// ArPosting.h - the AR voucher list dialogs (spec #6 4.3; plan S6-M3):
//
//   RunArPost     TARPostBatch (gl/src/ar_post.cpp) on dialog 1061:
//                 Transactions > Post Voucher > Sales Vouchers (4049)
//   RunArCancel   TARCancelBatch (gl/src/ar_cancb.cpp) on dialog 1054, AR's
//                 own (the original opened AP's 1043): Transactions > Cancel
//                 Voucher > Sales Vouchers (4053)
//   RunArRecover  new (D7), on dialog 1015: Transactions > Recover Sales
//                 Voucher (CM_AR_RECOVER)
//
// Each lists the vouchers it can act on; double-click (or Enter) one to act
// on it, after asking. Post's Post button posts them all. The record
// operations are in Ar.h.
//
#pragma once

#include <Windows.h>

INT_PTR RunArPost(HWND frame, bool closeAtOnce = false);
INT_PTR RunArCancel(HWND frame, bool closeAtOnce = false);
INT_PTR RunArRecover(HWND frame, bool closeAtOnce = false);
