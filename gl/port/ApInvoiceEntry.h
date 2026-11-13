//
// ApInvoiceEntry.h - port of TAPInvoiceEntry (gl/src/ap_btche.cpp,
// gl/header/ap_btche.h) on dialog 1042: Transactions > Purchase Invoice
// Vouchers (ID_APBATCH, 4044; also the toolbar's AP voucher button). Spec #5
// design 4.1, 4.2, D4-D6, D13; plan S5-M2.
//
// A purchase invoice voucher: the header (vendor, invoice number, dates,
// period, the AP and VAT accounts, the VAT and W/H rates, company or
// individual), then its expense lines, each an amount typed on an account.
// Every Add or Delete recomputes the voucher's VAT, W/H and AP lines (Ap.h
// SaveExpenseLine), so it always balances. The Distribution List beside it
// shows all the lines; double-click an expense line to change it. Modal, with
// four windows of its own: the line list, and the account, vendor and voucher
// lists (shown on request).
//
#pragma once

#include <Windows.h>

INT_PTR RunApInvoiceEntry(HWND frame, bool closeAtOnce = false);
