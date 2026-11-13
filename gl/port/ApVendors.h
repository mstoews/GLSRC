//
// ApVendors.h - port of TAPVendorAdd (gl/src/ap_vendo.cpp, gl/header/ap_vendo.h)
// on dialog 1041: Accounts > Purchase Vendors (ID_ADDVENDOR, 4045). Spec #5
// design 2.2, D12; plan S5-M1.
//
// The vendor master: number, names, address, phone and fax, contact, VAT
// number, VAT % and W/H %, company or individual, terms, discount, AP account
// and VAT account, medical rep. Enter (or Set Data) on the number shows the
// vendor; Search shows the vendor list (GL.INI [AP_VEND]). The record
// operations are in Ap.h.
//
#pragma once

#include <Windows.h>

INT_PTR RunApVendors(HWND frame, bool closeAtOnce = false);

// gl.exe /stress: opens and closes the AP screens `times` times and returns
// the change in GDI + USER objects (-1 if one failed to open). The later AP
// screens join it.
int StressAp(int times);
