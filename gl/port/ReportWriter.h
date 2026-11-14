//
// ReportWriter.h - the report writer's core (spec #7 4, 5, 7, D1-D4, D9; plan
// S7-M0). Portable: no Win32, so gl_tests checks it.
//
// A report definition (ReportDefinition) names a data source (Source) and
// says which of its fields print, sorted, grouped, totalled and filtered.
// BuildReport turns it into an rpt::Report for Spec #3's engine, which lays
// it out, previews and prints it like a built-in report. Definitions are
// kept as text (WriteDefinition / ParseDefinition, spec #7 5.4) in the port
// table gl_rptdef.
//
// The sources are fixed, in code (D3): each reads its tables with its own
// read-only queries, gives its fields readable names and types, and converts
// the legacy encodings (amounts to cents, text periods to numbers, codes
// trimmed). Nothing a user types reaches the database.
//
// The text format, a setting a line ('#' starts a comment; words, or
// "quoted text" with \" and \\ escapes):
//
//   writer 1                                   first line; the format's version
//   name "Sales by product"                    required
//   description "Net sales and VAT by product" the report list's text (default: the name)
//   title "Sales by product"                   default: the name
//   subtitle "Posted vouchers"
//   orientation landscape                      or portrait (the default)
//   source sales_lines 1                       a source's key and the version used
//   column product_name                        in order; options after the field:
//   column sales total blank label "Net" width 1.4   total, blank (zero), label, width
//   column calc diff amount received total     calculated (spec #7 5.3, S7-M3): sum a b,
//   column calc pct sales total                diff a b, count, pct a group|total
//   sort date desc                             up to three; asc (default) or desc
//   group product page                         up to two; page: a new page per group
//   filter state <> Cancelled                  =, <>, between a b, in a b..., contains a,
//   filter period between ? ? "Periods"        blank, notblank; ? asks when run, with
//                                              the question last
//   total yes                                  the grand total (default yes)
//   count no                                   row counts on totals (default no)
//
#pragma once

#include "reports/ReportContext.h"

#include "report/Report.h"

#include "cb4sqlite.h"

#include <cstdint>
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace rw {

// ---- sources (spec #7 4) -----------------------------------------------------------------

enum class FieldType { Text, Amount, Integer, Date };

struct Field {
    std::string key;          // stable: definitions name it
    std::string label;        // the column's default header
    FieldType type = FieldType::Text;
    double width = 1.0;       // the column's default relative width
    std::string nameField;    // a code's name field (a group header shows both); "" none
};

// A row's value of one field: text and dates (yyyymmdd) in `text`, amounts
// (cents) and integers in `number`.
struct Value {
    std::string text;
    std::int64_t number = 0;
};

using SourceRow = std::vector<Value>;   // in the source's field order

enum class Test { Equal, NotEqual, Between, In, Contains, Blank, NotBlank };

struct Filter {
    std::string field;
    Test test = Test::Equal;
    std::vector<std::string> values;   // as typed: text, yyyymmdd, 1234.56, 7
    bool ask = false;                  // the values are asked when the report runs
    std::string question;              // ask: what the prompt says
};

struct Source {
    std::string key;          // "sales_lines"
    std::string name;         // "Sales lines"
    int version = 1;
    std::string description;
    std::vector<Field> fields;
    // Applied unless the definition filters the same field (spec #7 4: the
    // cancelled are left out unless asked for).
    std::vector<Filter> defaults;
    std::function<std::vector<SourceRow>(cb4sqlite::Code4&, const ReportContext&)> rows;

    int fieldIndex(const std::string& key) const;   // -1 if none
};

const std::vector<Source>& Sources();
const Source* FindSource(const std::string& key);

// ---- the definition (spec #7 5) -------------------------------------------------------

// A calculated column (spec #7 5.3, S7-M3).
enum class Calc {
    None,         // the field `field`
    Sum,          // field + field2 (numbers)
    Difference,   // field - field2
    Count,        // 1 a row: totalled, how many rows
    Percent,      // field as a percentage of its group's total (ofGroup) or the report's, to two places
};

struct ColumnDef {
    std::string field;
    std::string label;        // "" the field's (or the calculation's)
    double width = 0;         // 0 the field's
    bool total = false;       // amounts and integers: subtotals and the grand total
    bool blankZero = false;   // print 0 as blank
    Calc calc = Calc::None;
    std::string field2;       // Sum, Difference: the second field
    bool ofGroup = false;     // Percent: of the innermost group's total, not the report's
};


struct SortKey {
    std::string field;
    bool descending = false;
};

struct GroupDef {
    std::string field;
    bool newPage = false;
};

struct ReportDefinition {
    std::string name, description, title, subtitle;
    bool landscape = false;
    std::string source;
    int sourceVersion = 1;
    std::vector<ColumnDef> columns;
    std::vector<SortKey> sort;      // up to 3
    std::vector<GroupDef> groups;   // up to 2
    std::vector<Filter> filters;
    bool grandTotal = true;
    bool count = false;
};

inline constexpr int kFormatVersion = 1;
inline constexpr size_t kMaxSort = 3, kMaxGroups = 2;
inline constexpr size_t kMaxQuestions = 4;   // asked filters a report may have (the prompt's rows)

struct ParseResult {
    std::optional<ReportDefinition> definition;
    int line = 0;              // the line refused (1-based); 0 when parsed
    std::string error;
};

ParseResult ParseDefinition(const std::string& text);
std::string WriteDefinition(const ReportDefinition& def);

// A column's header when it has no label: the field's, or the calculation's
// ("Amount - Received", "Count", "Sales (net) % of total").
std::string ColumnLabel(const Source& source, const ColumnDef& column);

// A calculated column's problem, "" if none: its fields must be numbers.
std::string ColumnProblem(const Source& source, const ColumnDef& column);

// ---- building (spec #7 7) ----------------------------------------------------------------

// The answers to a definition's asked filters, by the filter's index in
// ReportDefinition::filters.
using Answers = std::map<size_t, std::vector<std::string>>;

inline constexpr size_t kRowLimit = 20000;

struct BuildResult {
    std::optional<rpt::Report> report;
    std::string refusal;       // why there is no report (unknown source, too many rows, an unanswered question)
};

// Reads the source's rows, filters (the definition's, its answers, and the
// source's defaults for fields it doesn't filter), sorts (groups first),
// groups, totals in cents and fills the report's columns. A field the source
// doesn't have is left out with a note (D9). More than `rowLimit` rows is a
// refusal: narrow the filters (spec #7 7).
BuildResult BuildReport(cb4sqlite::Code4& code, const ReportContext& ctx, const ReportDefinition& def,
                        const Answers& answers = {}, size_t rowLimit = kRowLimit);

// ---- gl_rptdef (spec #7 5.2) ----------------------------------------------------------

struct StoredDefinition {
    long number = 0;
    std::string name, description, source;
    int sourceVersion = 0;
    std::string text;          // WriteDefinition's
    std::string madeBy, madeOn, changedBy, changedOn;   // user, yyyymmddhhmmss
};

// Creates gl_rptdef when the company lacks it.
void EnsureReportWriterTables(cb4sqlite::Code4& code);

std::vector<StoredDefinition> LoadDefinitions(cb4sqlite::Code4& code);   // in name order
std::optional<StoredDefinition> FindDefinition(cb4sqlite::Code4& code, long number);

// Adds (number 0) or replaces definition `number`; `user` and `stamp`
// (yyyymmddhhmmss) say who and when. Returns its number.
long SaveDefinition(cb4sqlite::Code4& code, long number, const ReportDefinition& def, const std::string& user,
                    const std::string& stamp);

bool DeleteDefinition(cb4sqlite::Code4& code, long number);

// Another definition (not `except`) has this name, ignoring case and spaces at the ends.
bool DefinitionNameTaken(cb4sqlite::Code4& code, const std::string& name, long except = 0);

// ---- the designer's checks (spec #7 6.2; plan S7-M2) -----------------------------------

// Why the designer's Save refuses `def` (definition `except` being changed),
// with the setting to put the focus on; nothing when it can be saved.
enum class DefinitionSetting { Name, Source, Columns, Filters };   // a refused column or question is Columns / Filters
struct DefinitionProblem {
    DefinitionSetting setting = DefinitionSetting::Name;
    std::string text;
};
std::optional<DefinitionProblem> CheckDefinition(cb4sqlite::Code4& code, const ReportDefinition& def, long except = 0);

// A free name for a copy of `name`: "Copy of <name>", then "Copy of <name> (2)", ...
std::string CopyName(cb4sqlite::Code4& code, const std::string& name);

// The fields `def` names (columns, sort, groups, filters) that its source
// doesn't have, in that order, once each (D9: the designer marks them).
std::vector<std::string> MissingFields(const ReportDefinition& def);

// Why a filter's values don't suit its field ("Date: \"x\" isn't a date");
// "" when they do, or when they are asked when run. The designer checks each
// filter as it's added, and CheckDefinition checks them all.
std::string FilterProblem(const Source& source, const Filter& filter);

// A filter in words, as the report's subtitle says it: "Period from 7 to 8";
// an asked one: "Period: asked when run (Periods)". The field's key when the
// source lacks it.
std::string DescribeFilter(const Source& source, const Filter& filter);

} // namespace rw
