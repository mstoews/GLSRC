//
// DepreciationRun.h - Fixed Assets > Create Voucher (ID_FASLIP 6003, dialog
// 4003; gl/src/gl_fa.cpp TGLCreateDeprSlip; spec #4 4.2, 6.4; plan S4-M3).
//
// The month's depreciation as one open voucher, for the current period: the
// dialog shows what the run will do, OK asks, then Assets.h RunDepreciation
// writes it. The voucher is posted with Transactions > Post Voucher;
// cancelling it (Cancel Voucher) undoes the run.
//
#pragma once

#include <Windows.h>

// `closeAtOnce`: open and close at once, for /stress.
INT_PTR RunCreateDepreciationVoucher(HWND frame, bool closeAtOnce = false);
