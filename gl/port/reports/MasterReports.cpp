//
// MasterReports.cpp - see MasterReports.h.
//
#include "MasterReports.h"

#include "report/Format.h"

#include <cstdio>
#include <string>
#include <vector>

namespace {

rpt::Report frame(const ReportContext& ctx, std::string title, std::string footer, bool periodLine)
{
    rpt::Report r;
    r.company = ctx.company;
    r.printedOn = ctx.printedOn;
    r.title = std::move(title);
    if (periodLine) r.subtitles.push_back(CurrentPeriodLine(ctx));
    r.footerNote = std::move(footer);
    return r;
}

// "3 medical representatives", "1 product".
std::string countLine(size_t n, const char* one, const char* many)
{
    return std::to_string(n) + " " + (n == 1 ? one : many);
}

// A quantity with up to three decimals and no trailing zeros: 10, 12.5.
std::string quantity(double v)
{
    char buf[32];
    std::snprintf(buf, sizeof buf, "%.3f", v);
    std::string s = buf;
    while (!s.empty() && s.back() == '0') s.pop_back();
    if (!s.empty() && s.back() == '.') s.pop_back();
    return s == "-0" ? "0" : s;
}

// A party's first row: `cells` with the address column holding address 1;
// then a row each for address 2 and 3, when set, with only that column.
void addParty(rpt::Report& r, std::vector<rpt::Cell> cells, size_t addressCol,
              const std::string& address2, const std::string& address3)
{
    const size_t n = cells.size();
    r.rows.push_back(rpt::Row::Detail(std::move(cells)));
    for (const std::string* a : {&address2, &address3}) {
        if (a->empty()) continue;
        std::vector<rpt::Cell> more(n);
        more[addressCol] = rpt::Cell::Text(*a);
        r.rows.push_back(rpt::Row::Detail(std::move(more)));
    }
}

// The count, on a line of its own under a blank one ("3 products").
void endWithCount(rpt::Report& r, size_t n, const char* one, const char* many)
{
    r.rows.push_back(rpt::Row::Blank());
    r.rows.push_back(rpt::Row::Note(countLine(n, one, many)));
}

rpt::Column text(const char* h, double w, const char* h2 = "") { return {h, w, rpt::CellType::Text, h2}; }

} // namespace

rpt::Report MedicalRepsListing(cb4sqlite::Code4& code, const ReportContext& ctx)
{
    rpt::Report r = frame(ctx, "Medical Representative Summary Report", "Report File: gl_medrp.rpt", true);
    r.orientation = rpt::Orientation::Landscape;
    // Widths fit the fields' lengths in the fixture and the real data: a
    // 5-character code, "(604) 555 - 0199" phone numbers.
    r.columns = {text("Code", 0.75),       text("Short", 1.0, "Name"), text("Name", 1.6),
                 text("Address", 1.6),     text("Postal", 0.85),       text("Phone", 1.7),
                 text("Fax", 1.7),         text("Contact", 1.0),       text("Description", 1.4),
                 text("Type", 0.55),       {"Terms", 0.65, rpt::CellType::Integer},
                 {"Credit", 1.1, rpt::CellType::Amount, "Limit"}};
    cb4sqlite::Query q(code,
                       "SELECT repcode, rpshort, rpname, address1, address2, address3, postal, phone, fax, "
                       "contact, descript, type, terms, creditlmt FROM gl_medrp WHERE _deleted = 0 "
                       "ORDER BY repcode, _rowid");
    size_t n = 0;
    while (q.next()) {
        ++n;
        addParty(r,
                 {rpt::Cell::Text(q.text(0)), rpt::Cell::Text(q.text(1)), rpt::Cell::Text(q.text(2)),
                  rpt::Cell::Text(q.text(3)), rpt::Cell::Text(q.text(6)), rpt::Cell::Text(q.text(7)),
                  rpt::Cell::Text(q.text(8)), rpt::Cell::Text(q.text(9)), rpt::Cell::Text(q.text(10)),
                  rpt::Cell::Text(q.text(11)), rpt::Cell::Integer(q.integer(12)),
                  rpt::Cell::Amount(rpt::ToCents(q.number(13)))},
                 3, q.text(4), q.text(5));
    }
    endWithCount(r, n, "medical representative", "medical representatives");
    return r;
}

rpt::Report ProductCodesListing(cb4sqlite::Code4& code, const ReportContext& ctx)
{
    // GL_PRODL.RPT's header has no current period.
    rpt::Report r = frame(ctx, "General Ledger Product Codes", "Report File : gl_prodl.rpt", false);
    r.columns = {text("Product", 1.0, "Code"),
                 text("Product", 1.3, "Name"),
                 text("Product Description", 3.2),
                 text("Shop", 0.6),
                 rpt::Column("Lot Size", 0.9).aligned(rpt::Align::Right),
                 {"Product", 1.2, rpt::CellType::Amount, "Price"}};
    cb4sqlite::Query q(code,
                       "SELECT fpcode, fpname, ffulln, fshop1, flotsize, favprice FROM gl_itemm "
                       "WHERE _deleted = 0 ORDER BY fpcode, _rowid");
    size_t n = 0;
    while (q.next()) {
        ++n;
        r.rows.push_back(rpt::Row::Detail(
            {rpt::Cell::Text(q.text(0)), rpt::Cell::Text(q.text(1)), rpt::Cell::Text(q.text(2)),
             rpt::Cell::Text(q.text(3)), rpt::Cell::Text(quantity(q.number(4))),
             rpt::Cell::Amount(rpt::ToCents(q.number(5)))}));
    }
    endWithCount(r, n, "product", "products");
    return r;
}

rpt::Report DoctorCodesListing(cb4sqlite::Code4& code, const ReportContext& ctx)
{
    rpt::Report r = frame(ctx, "Doctor Codes Summary Report", "Report File: gl_drcds.rpt", true);
    r.orientation = rpt::Orientation::Landscape;
    r.columns = {text("Doctor", 0.75, "Code"), text("Short", 1.0, "Name"), text("Name", 1.6),
                 text("Address", 1.6),          text("Postal", 0.85),       text("Phone", 1.7),
                 text("Fax", 1.7),              text("Territory", 0.9),     text("Contact", 1.0),
                 text("Description", 1.6)};
    cb4sqlite::Query q(code,
                       "SELECT drcode, rpshort, rpname, address1, address2, address3, postal, phone, fax, "
                       "territory, contact, descript FROM gl_drcod WHERE _deleted = 0 ORDER BY drcode, _rowid");
    size_t n = 0;
    while (q.next()) {
        ++n;
        addParty(r,
                 {rpt::Cell::Text(q.text(0)), rpt::Cell::Text(q.text(1)), rpt::Cell::Text(q.text(2)),
                  rpt::Cell::Text(q.text(3)), rpt::Cell::Text(q.text(6)), rpt::Cell::Text(q.text(7)),
                  rpt::Cell::Text(q.text(8)), rpt::Cell::Text(q.text(9)), rpt::Cell::Text(q.text(10)),
                  rpt::Cell::Text(q.text(11))},
                 3, q.text(4), q.text(5));
    }
    endWithCount(r, n, "doctor", "doctors");
    return r;
}
