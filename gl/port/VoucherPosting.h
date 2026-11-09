//
// VoucherPosting.h - the three screens that finish the voucher cycle
// (Spec #2 design 2.4, 2.5; plan S2-M4). Each lists vouchers in its
// tbSpread grid; the record operations are in Vouchers.h.
//
//   RunPostVouchers    TGLPostBatch (gl/src/gl_postb.cpp), dialog 1061:
//                      Post General Ledger Voucher (10121, the toolbar's
//                      post button, GLSUB). Post posts every voucher; a
//                      double-clicked row posts that one.
//   RunRecoverVoucher  TGLRecoverBatch (gl/src/gl_postr.cpp), dialog 1015:
//                      Recover General Ledger Voucher (10122). A
//                      double-clicked posted voucher is reversed and reopened.
//   RunCancelVoucher   TGLCancelBatch (gl/src/gl_btcan.cpp), dialog 1043:
//                      Cancel Voucher > General Ledger (3054). A
//                      double-clicked voucher is deleted.
//
#pragma once

#include <Windows.h>

INT_PTR RunPostVouchers(HWND frame);
INT_PTR RunRecoverVoucher(HWND frame);
INT_PTR RunCancelVoucher(HWND frame);

// gl.exe /stress: opens and closes each of the three `times` times and
// returns the change in GDI + USER objects (-1 if a dialog failed to open).
int StressVoucherPosting(int times);
