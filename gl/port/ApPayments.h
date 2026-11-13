//
// ApPayments.h - the AP payment screens (spec #5 6; plans S5-M5, S5-M6):
//
//   RunApCreateCheques  TAPCreatePayment and TAPHoldForce (gl/src/ap_payrq.cpp)
//                       on dialogs 1045 and 1066: Receipts/Payments > Create
//                       Cheques (ID_GLSLIP, 1057)
//   RunApConfirmCheques  Receipts/Payments > Confirm Printed Cheques (8074,
//                       replacing Set Cheque Numbers, TPrintCheques; dialog
//                       IDD_AP_CONFIRM): confirm a range printed, renumber a
//                       spoiled cheque (S5-M6, D9)
//   RunApBankReconciliation  TAPPayment / TAPSetChequeNumber on 1046 and
//                       1051: Receipts/Payments > Bank Reconciliation (6085):
//                       clear a printed cheque, or void one (S5-M6)
//   RunApPurge          TAPPurgeBatch on 1048: Maintenance > Purge Paid
//                       Payables (1066): the cleared items paid before a date
//                       (S5-M6, D8)
//
// Create Cheques lists the invoices due by a date; double-click one to hold
// it, force it (pay it even if not due) or open it again. Create pays the
// open invoices due and the forced ones: one cheque per vendor, numbered
// from the starting number, drawn on the bank chosen, with the payment's GL
// entry (Dr AP, Cr bank) in the payment date's period. The record operations
// are in Ap.h.
//
#pragma once

#include <Windows.h>

INT_PTR RunApCreateCheques(HWND frame, bool closeAtOnce = false);
INT_PTR RunApConfirmCheques(HWND frame, bool closeAtOnce = false);
INT_PTR RunApBankReconciliation(HWND frame, bool closeAtOnce = false);
INT_PTR RunApPurge(HWND frame, bool closeAtOnce = false);
