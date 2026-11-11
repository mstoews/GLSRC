//
// FixedAssetDisposal.h - Fixed Assets > Disposal (ID_DISPOSAL 6035, dialog
// 1001; gl/src/gl_fa.cpp TGLFixedDelete; spec #4 6.3, D7; plan S4-M4).
//
// Marks an asset sold or scrapped, with the date, and shows the gain or loss
// on a sale price; the entry itself is made in Voucher Entry (D7). Delete
// removes an asset entered by mistake, if the run has never depreciated it.
// The record operations are in Assets.h.
//
#pragma once

#include <Windows.h>

// `closeAtOnce`: open and close at once, for /stress.
INT_PTR RunAssetDisposal(HWND frame, bool closeAtOnce = false);
