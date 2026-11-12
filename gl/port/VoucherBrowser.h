//
// VoucherBrowser.h - Transactions > Voucher List (CM_VOUCHER_LIST): an MDI
// window, like a report preview, with every General Ledger voucher in a grid.
// The port's own screen; the original had no such list.
//
// One row per gl_btch header, in voucher-number order: number, status
// (VoucherStatus), date, period, description, line count, debit and credit
// totals. Double-click or Enter on a row opens that voucher's editor:
//
//   Open ("1")          Voucher Entry on that voucher (RunVoucherEntry)
//   Adjustment ("3")    Prior Period Entry on it (RunPriorPeriodEntry)
//   posted ("0"/"2"/"9") its voucher slip in a preview: a posted voucher
//                       can't be edited (Recover it first)
//
// The list is read again when an editor closes and on F5. No table stays open
// between reads: the editors close every table when they close (MainDialog).
// A second CM_VOUCHER_LIST activates the window already open.
//
#pragma once

#include "win32fx.h"

// Opens the voucher list in `frame`, or activates the one already open.
void OpenVoucherBrowser(fx::MdiFrame& frame);
