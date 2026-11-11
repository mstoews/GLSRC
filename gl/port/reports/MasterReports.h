//
// MasterReports.h - the master-data listings (spec #3 2.2, D12; plan S3-M7).
// Portable: gl_tests checks them.
//
// Each lists its table's records (not deleted) in code order, as its Crystal
// report did, with a count at the end; the footer names the .RPT the original
// ran. Like the other original reports they take no options (D17).
//
//   6039  Medical Representative Summary Report (GL_MEDRP.RPT, gl_medrp):
//         code, short name, name, address, postal code, phone, fax, contact,
//         description, type, terms and credit limit. Landscape.
//   6038  General Ledger Product Codes (GL_PRODL.RPT, gl_itemm): code, name,
//         description (FFULLN), shop (FSHOP1), lot size and price (FAVPRICE).
//   6041  Doctor Codes Summary Report (GL_DRCDS.RPT, gl_drcod): code, short
//         name, name, address, postal code, phone, fax, territory, contact and
//         description. Landscape. The .RPT's report title was copied from 6039
//         ("Medical Representatives") and its table path was c:\gl\dbf\; the
//         port titles it after its page heading and reads the open company (D12).
//
// A party's second and third address lines, when set, print on the lines
// under the first.
//
#pragma once

#include "ReportContext.h"

#include "report/Report.h"

rpt::Report MedicalRepsListing(cb4sqlite::Code4& code, const ReportContext& ctx);
rpt::Report ProductCodesListing(cb4sqlite::Code4& code, const ReportContext& ctx);
rpt::Report DoctorCodesListing(cb4sqlite::Code4& code, const ReportContext& ctx);
