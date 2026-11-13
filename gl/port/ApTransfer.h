//
// ApTransfer.h - port of TGLAPUpdate (gl/src/gl_apupd.cpp) on dialog 1030:
// Maintenance > Update AR and AP (1028). Spec #5 design 4.4, D2, D10; plan
// S5-M3.
//
// The AP half: AP's waiting journal becomes one open GL voucher per period
// (Ap.h TransferAp), to post with Transactions > Post Voucher. The AR half
// comes with Spec #6; until then its box is disabled.
//
#pragma once

#include <Windows.h>

INT_PTR RunApTransfer(HWND frame, bool closeAtOnce = false);
