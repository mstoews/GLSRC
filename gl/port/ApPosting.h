//
// ApPosting.h - the AP voucher list dialogs (spec #5 4.3, 5; plan S5-M3):
//
//   RunApPost     TAPPostBatch (gl/src/ap_post.cpp) on dialog 1047:
//                 Transactions > Post Voucher > Purchase Vouchers (1048); the
//                 cash vouchers' (1088) in S5-M4
//   RunApCancel   TAPCancelBatch (gl/src/ap_cancb.cpp) on dialog 1043:
//                 Transactions > Cancel Voucher > Purchase Vouchers (3064);
//                 the cash vouchers' (1085) in S5-M4
//   RunApRecover  new (D7), on dialog 1015: Transactions > Recover Purchase
//                 Voucher (CM_AP_RECOVER)
//
// Each lists the vouchers it can act on; double-click (or Enter) one to act
// on it, after asking. Post's OK posts them all. The record operations are
// in Ap.h.
//
#pragma once

#include "Ap.h"

#include <Windows.h>

INT_PTR RunApPost(HWND frame, ApKind kind, bool closeAtOnce = false);
INT_PTR RunApCancel(HWND frame, ApKind kind, bool closeAtOnce = false);
INT_PTR RunApRecover(HWND frame, ApKind kind, bool closeAtOnce = false);
