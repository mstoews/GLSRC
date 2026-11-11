//
// AssetReports.h - the fixed-asset reports (spec #4 7, D11, D12; plan S4-M5).
// Portable: gl_tests checks them.
//
//   1019  General Ledger Fixed Asset Summary (GL_FA.RPT): the register at the
//         current period, landscape, grouped by type. Per asset: code, date
//         purchased, date sold, description (and reference under it), months
//         used, life, cost, salvage, monthly depreciation, accumulated
//         depreciation and net book value (added). Totals per type and
//         overall, of the assets in use; disposed assets are listed with
//         their date sold but not totalled. Then the check against the ledger
//         (D11): for each cost and accumulated-depreciation account the types
//         of the assets in use name, the register's total beside the
//         account's balance at the current period, with what differs.
//   1020  General Ledger Fixed Asset Types (GL_FATYP.RPT): each type with its
//         asset, expense and accumulated-depreciation accounts.
//
// The original GL_FA.RPT computed the monthly amount as cost / life
// (ignoring salvage) and the accumulated depreciation as that x months used
// (spec #4 2.4 #4); the port prints the register's stored figures (D6).
// Both reports read the open company (D12; GL_FA.RPT's table path was
// c:\gl\dbf\), and 1020's title is spelled "Ledger".
//
#pragma once

#include "ReportContext.h"

#include "report/Report.h"

rpt::Report FixedAssetListing(cb4sqlite::Code4& code, const ReportContext& ctx);
rpt::Report FixedAssetTypesListing(cb4sqlite::Code4& code, const ReportContext& ctx);
