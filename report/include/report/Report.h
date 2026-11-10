#pragma once
//
// Report.h - the report model (spec #3 4.1).
//
// A report is data: a builder fills one in (computing every total itself, in
// cents), then Layout() turns it into pages and a renderer draws them. The
// model knows nothing about the database or GDI. Text is UTF-8.
//
//   rpt::Report r;
//   r.company = "Takeda";
//   r.title = "Trial Balance";
//   r.columns = {{"Account", 1.0}, {"Debit", 1.2, rpt::CellType::Amount}};
//   r.rows.push_back(rpt::Row::Detail({rpt::Cell::Text("101000"), rpt::Cell::Amount(50000)}));
//
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace rpt {

enum class Align { Left, Center, Right };
enum class Orientation { Portrait, Landscape };

// How a negative amount prints: "-1,234.56" on lists, "(1,234.56)" on the
// financial statements (spec #3 4.3).
enum class NegativeStyle { Minus, Parentheses };

// The font roles a Measure knows (spec #3 4.2).
enum class Role { Title, Heading, Body, Bold };

enum class CellType { Empty, Text, Amount, Integer, Date };

struct Cell {
    CellType     type = CellType::Empty;
    std::string  text;          // Text; Date: the stored yyyymmdd
    std::int64_t value = 0;     // Amount: cents; Integer: the number
    bool         bold = false;

    static Cell Empty() { return {}; }
    static Cell Text(std::string s) { Cell c; c.type = CellType::Text; c.text = std::move(s); return c; }
    static Cell Amount(std::int64_t cents) { Cell c; c.type = CellType::Amount; c.value = cents; return c; }
    static Cell Integer(std::int64_t n) { Cell c; c.type = CellType::Integer; c.value = n; return c; }
    static Cell Date(std::string yyyymmdd) { Cell c; c.type = CellType::Date; c.text = std::move(yyyymmdd); return c; }
};

struct Column {
    std::string header;                 // first header line
    double      width = 1.0;            // relative to the other columns
    CellType    type = CellType::Text;  // decides the default alignment and formatting
    std::string header2;                // optional second header line
    bool        blankZero = false;      // amounts and integers: print 0 as blank
    // Left for text and dates, right for numbers, unless set.
    bool        alignSet = false;
    Align       align = Align::Left;

    Column() = default;
    Column(std::string h, double w, CellType t = CellType::Text, std::string h2 = {}, bool blank = false)
        : header(std::move(h)), width(w), type(t), header2(std::move(h2)), blankZero(blank) {}
    Column& aligned(Align a) { align = a; alignSet = true; return *this; }
    Align alignment() const
    {
        if (alignSet) return align;
        return (type == CellType::Amount || type == CellType::Integer) ? Align::Right : Align::Left;
    }
};

enum class RowKind {
    Detail,       // one cell per column
    Heading,      // bold section heading; `text` spans the row; kept with the next row
    GroupHeader,  // starts a group: `text` spans the row, or `cells`; kept with the next row
    GroupFooter,  // group totals: a rule above each amount cell; kept with the row before
    Subtotal,     // the same look, on statements
    Total,        // a rule above each amount cell; with `final`, a double rule below
    Note,         // full-width text, wrapped at spaces (warnings, explanations)
    Blank,        // an empty line
    PageBreak,    // the next row starts a new page
    Slip,         // slip mode: starts a new page with `fields`, then the column headers
};

struct Row {
    RowKind           kind = RowKind::Detail;
    std::vector<Cell> cells;
    std::string       text;
    int               indent = 0;     // levels, applied to the first column (or the spanning text)
    bool              final = false;  // Total: double rule below
    // Slip: label/value pairs printed two to a line above the slip's lines.
    std::vector<std::pair<std::string, std::string>> fields;

    static Row Detail(std::vector<Cell> c, int indent = 0)
    {
        Row r; r.cells = std::move(c); r.indent = indent; return r;
    }
    static Row Heading(std::string t, int indent = 0)
    {
        Row r; r.kind = RowKind::Heading; r.text = std::move(t); r.indent = indent; return r;
    }
    static Row GroupHeader(std::string t)
    {
        Row r; r.kind = RowKind::GroupHeader; r.text = std::move(t); return r;
    }
    static Row GroupFooter(std::vector<Cell> c)
    {
        Row r; r.kind = RowKind::GroupFooter; r.cells = std::move(c); return r;
    }
    static Row Subtotal(std::vector<Cell> c, int indent = 0)
    {
        Row r; r.kind = RowKind::Subtotal; r.cells = std::move(c); r.indent = indent; return r;
    }
    static Row Total(std::vector<Cell> c, bool isFinal = false)
    {
        Row r; r.kind = RowKind::Total; r.cells = std::move(c); r.final = isFinal; return r;
    }
    static Row Note(std::string t)
    {
        Row r; r.kind = RowKind::Note; r.text = std::move(t); return r;
    }
    static Row Blank() { Row r; r.kind = RowKind::Blank; return r; }
    static Row PageBreak() { Row r; r.kind = RowKind::PageBreak; return r; }
    static Row Slip(std::vector<std::pair<std::string, std::string>> f)
    {
        Row r; r.kind = RowKind::Slip; r.fields = std::move(f); return r;
    }
};

struct Report {
    std::string              company;     // first header line
    std::string              title;       // second header line
    std::vector<std::string> subtitles;   // "As at 10/31/2026", "Department 10 Sales"...
    std::string              printedOn;   // the date only: Layout prints "Printed on <date>"; empty: left out (tests)
    std::string              footerNote;  // bottom left of every page, e.g. the report's name
    Orientation              orientation = Orientation::Portrait;
    NegativeStyle            negatives = NegativeStyle::Minus;
    // Slip mode: the column headers follow each Slip row instead of the page header.
    bool                     slipMode = false;
    std::vector<Column>      columns;
    std::vector<Row>         rows;
};

} // namespace rpt
