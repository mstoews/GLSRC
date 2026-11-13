//
// ArCustomers.h - port of TARVendorAdd and TARVendorOptions (gl/src/ar_vendo.cpp)
// on dialogs 1052 and 1063: Accounts > Sales Customers (ID_AR_CUSTOMER, 4059).
// Spec #6 design 2.2, D12, D15; plan S6-M1.
//
// The customer master: number, names, address and ship-to address, phones
// and faxes on the main screen; the AR, sales and VAT accounts, VAT %, terms,
// contact and VAT number on Options. Enter (or Find) on the number shows the
// customer; List shows the customer list (GL.INI [AR_CUST]). The record
// operations are in Ar.h.
//
#pragma once

#include <Windows.h>

INT_PTR RunArCustomers(HWND frame, bool closeAtOnce = false);

// gl.exe /stress: opens and closes the AR screens `times` times and returns
// the change in GDI + USER objects (-1 if one failed to open). The later AR
// screens join it.
int StressAr(int times);
