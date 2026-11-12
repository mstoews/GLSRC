//
// VoucherEntry.h - port of TGLBatchEntry (gl/src/gl_btche.cpp,
// gl/header/gl_btche.h) and TGLPrior (gl/src/gl_prior.cpp) on dialog 1013:
//
//   RunVoucherEntry      Transactions > General Ledger Voucher, GLSUB, the
//                        toolbar's GL batch button (ID_MODBATCH), ID_BATCHENT
//   RunPriorPeriodEntry  Transactions > Prior Period Entry (CM_PRIOR): the
//                        same dialog writing adjustment vouchers (mode "3"),
//                        and showing only those
//
// Both are modal (ExecDialog), with four windows of their own: the line list
// (shown), the account list and voucher list (shown on request) and the
// tool box (the Show check box). The record operations are in Vouchers.h.
//
#pragma once

#include <Windows.h>

// `voucher` > 0 opens the dialog on that voucher, as if picked in its voucher
// list (the voucher browser, VoucherBrowser.h); 0 starts on a new voucher.
INT_PTR RunVoucherEntry(HWND frame, long voucher = 0);
INT_PTR RunPriorPeriodEntry(HWND frame, long voucher = 0);

// gl.exe /stress: opens and closes Voucher Entry `times` times (alternating
// with Prior Period Entry) and returns the change in GDI + USER objects.
int StressVoucherEntry(int times);
