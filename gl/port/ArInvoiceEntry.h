//
// ArInvoiceEntry.h - port of TARInvoiceEntry (gl/src/ar_btche.cpp,
// gl/header/ar_btch.h) on dialog 1053: Transactions > Sales Invoice Vouchers
// (5045). Spec #6 design 2.3, 4.1, 4.2, D4-D6, D13, D16; plan S6-M2.
//
// A sales invoice voucher: the header (customer, invoice and order numbers,
// dates, period, the AR, sales and VAT accounts, the VAT rate, terms), then
// its product lines, each a quantity at a price or an amount typed, VAT
// included. Every Add or Delete recomputes the voucher's VAT and AR lines
// (Ar.h SaveSaleLine), so it always balances. The Distribution List beside
// it shows all the lines; double-click a product line to change it. Modal
// (D16), with four windows of its own: the line list, and the account,
// customer and voucher lists (shown on request).
//
#pragma once

#include <Windows.h>

INT_PTR RunArInvoiceEntry(HWND frame, bool closeAtOnce = false);
