# Spec #7 — OWL-to-Win32 SDK Port: Report Writer

**Status:** Draft for review (2026-11-14). Q1–Q7 open (§13). The plan: `plans/2026-11-14-owl-to-win32-sdk-port-spec7-plan.md`.
**Date:** 2026-11-14
**Scope:** Seventh spec of the migration: a report writer, so users can make new reports without code:
- report definitions kept in the company database;
- a designer that builds them from fixed data sources: columns, sort, groups and totals, filters, values asked when the report runs;
- Reporting > Custom Report brought back to life: run, add, change, copy and delete user reports;
- the result previews and prints on Spec #3's engine, like every built-in report.

The original's "report writer" was Crystal Reports' own designer: users drew `.RPT` files in it, and Custom Report (`GL_REPT`) registered them by file name (§2). The port dropped the Crystal runtime in Spec #3, so that path is gone. This spec puts a report writer back, on the port's engine, **without the `.RPT` format** (D1).

**Depends on:** Specs #1–#6. Spec #3's `report` library lays out and prints the reports, and its preview shows them; Specs #2, #4, #5 and #6 own the tables the data sources read. Everything they fixed still holds: the build, the error idioms, the Unicode strategy, the translation pattern and the per-port checklist.

**Sources.** `gl/src/gl_rept.cpp` and `gl/src/ar_rept.cpp` (`TGLCustomReport`, `TGLAddReport`, `TGLModifyReport`, `TGLDeleteReport`), dialog 1022 and the `GL_ADDREPORT` template in `gl/gl_new.rc`, the deployed `dbf/GL_REPT.DBF`, the 91 `.RPT` files in `dbf/`, Spec #3's design (D9) and `report/include/report/Report.h`.

---

## 1. Goals & Non-Goals

### 1.1 Goals (Spec #7 deliverables)

1. **Report definitions as data** (D2): a definition names a data source and says which of its fields print, in what order, sorted and grouped how, totalled, and filtered. Definitions are kept in a port table in the company database, so Backup and Restore carry them.
2. **Data sources** (D3): a fixed set, defined in code, over the books' tables (§4): ledger accounts, GL voucher lines, posted ledger lines, fixed assets, AP and AR records, sales lines with their med rep, product and doctor, and the master lists. Each source gives its fields friendly names and types and does the joins, so a user never sees `AR_ACCT` (which is the sales account) or an amount in a text column.
3. **The designer** (D5): one dialog to make or change a definition, with a **Preview** button that runs it as it stands.
4. **Custom Report** (3068, dialog 1022): the user reports beside the built-in ones; **Add**, **Modify**, **Copy** and **Delete** come back (Spec #3 D9 hid them); a report runs as the built-in ones do.
5. **Asked when run** (D6): a filter can ask for its value each time (a period, a date range, an account range, a customer).
6. **Export and import** of a definition to a text file, to move it between companies (Q6).
7. **A portable core**, `gl/port/ReportWriter.{h,cpp}`, under `gl_tests`: the model, the text format, the sources, and the builder that turns a definition into an `rpt::Report`.

### 1.2 Non-Goals

- **Reading or writing `.RPT` files** (D1).
- **A free-form page designer** (Crystal's banded canvas: fields dragged anywhere, lines, boxes, pictures). Reports are tabular, in the engine's layout. A page designer could follow if the tabular reports turn out not to be enough (R1).
- **Formulas in a language of their own.** Calculated columns are limited to the few kinds in §5.3 (Q3).
- **SQL typed by users** (Q5): sources are fixed in code.
- **Charts, cross-tabs and pivots; scheduling, e-mailing; changing data.** A report reads and changes nothing.
- **Turning the built-in reports into definitions.** They stay in C++: they have arithmetic of their own (aging, statements, the financial statements' layouts) that a definition can't express. Custom Report lists both.
- **`GL_REPT`**, the original's table: left untouched, as Spec #3 D9 left it.

---

## 2. Custom reports in the original

### 2.1 What it did

- **Custom Report** (3068: File > Print, Reporting > Custom Report, GLSUB, the toolbar's Print) opened dialog 1022 over `GL_REPT` (`DESCRIPT` C40, `NAME` C20, `FNAME` C128): a list of descriptions.
- **Print** ran the chosen row's `.RPT` file through the Crystal runtime (`PEOpenPrintJob`, `PEOutputToWindow`).
- **Add / Modify / Delete** (`GL_ADDREPORT`) only registered a file: a description, a name and a path. The report itself was designed outside the program, in the Crystal Reports designer, against the `.DBF` files.

So the original never had a report writer of its own: it had a register of Crystal files, and Crystal was the writer.

### 2.2 The deployed data

`GL_REPT` has six rows: the AP batch summary (`C:\GL\DBF\AP_BTCH.RPT`), the AP register, "Online Rrport" (`ONLINE.RPT`, which exists nowhere), AR balance, the AR aged report and Print Cheques. Every one of them is now a built-in report (Specs #5 and #6), or doesn't exist. There are no user-made reports to bring across.

### 2.3 The `.RPT` files

The 91 files in `dbf/` start `00 10 3B 00 03 00 00 00`: the 16-bit Crystal Reports format of the original's time. It isn't the OLE compound-document format later Crystal versions use, and it is undocumented.

### 2.4 What doesn't work, or is missing

1. **Nothing runs a `.RPT` any more.** The port has no Crystal runtime (Spec #3 D9), and the 16-bit runtime doesn't run on 64-bit Windows.
2. **Modify appended a duplicate row** instead of changing one (noted in Spec #3's design).
3. **`SetupWindow` closed every open table.**
4. **Nothing reports the analysis codes.** Voucher and sales lines carry a med rep, a product and a doctor (Specs #2, #5, #6), and no report, original or ported, groups or totals by them. Sales by product, by med rep or by doctor is the most likely first use of a report writer here (Q1).

---

## 3. Screens and port files

| Original | ID | Port file | Notes |
|---|---|---|---|
| `TGLCustomReport` | 3068 (1022) | `gl/port/reports/ReportList.{h,cpp}`, extended | Add/Mod, Modify, Delete shown again; Copy added; user reports listed (D7) |
| `TGLAddReport` / `TGLModifyReport` | `GL_ADDREPORT` | `gl/port/reports/ReportDesigner.{h,cpp}` | a new template in `port.rc` (`IDD_RW_DESIGN`); the original's only took a description and a file (D5) |
| — | — | `gl/port/reports/ReportPrompt.{h,cpp}` | the "asked when run" dialog (D6), `port.rc` |
| — | — | `gl/port/ReportWriter.{h,cpp}` | portable: the model, the text format, the sources, the builder; `gl_tests` checks it |
| — | — | `gl/port/reports/RunReport.cpp` | runs a user report through the same preview as the built-ins |

---

## 4. Data sources (D3)

A **source** is a named, versioned list of fields over one or more tables, with the join and the conversions done in code: amounts in cents, dates as dates, periods as numbers, codes trimmed, an account's name looked up. Each field has a key (stable, used in definitions), a label, a type (text, amount, integer, date) and a default width.

| Source | Rows | Fields (main ones) |
|---|---|---|
| **Accounts** | `glaccts` | account, dept, description, type, opening balance, current balance, each period's movement, prior year, budget |
| **GL voucher lines** | `gl_btchd` with `gl_btch` | voucher, state (open, posted, cancelled), date, period, reference, cheque, account, dept, account name, description, debit, credit, net, med rep, product, doctor |
| **Posted ledger lines** | `gl_dist` | as above, for the posted lines only |
| **Fixed assets** | `gl_fa`, `gl_fatyp` | asset, description, type, acquired, cost, salvage, monthly and accumulated depreciation, book value, disposed |
| **Vendors** / **Customers** | `ap_vend` / `ar_cust` | number, names, address, phone, accounts (named for what they are), terms, rates |
| **AP open items** / **AR open items** | `ap_dist` / `ar_dist` with the vendor or customer | voucher, invoice, dates, amount, VAT, W/H (AP), paid / received, owed, state |
| **AR receipts** | `ar_rcpt` | receipt, invoice, customer, date, period, bank, amount, discount |
| **Sales lines** | `ar_btchd` with `ar_btch`, `ar_linex`, the customer and the master lists | voucher, state, date, period, customer, invoice, product (code and name), med rep, doctor, quantity, unit price, amount (VAT included), sales (net), VAT share |
| **Purchase lines** | `ap_btchd` / `ap_cashd` with their headers and `ap_linex` | the same, for AP's expense lines |
| **Med reps**, **Products**, **Doctors** | the master tables | their fields |

- **Sales lines** give each product line its net and VAT share by `SplitSale` (Spec #6 §4.1), so a sales-by-product report totals what the ledger has.
- **Cancelled vouchers** are rows with state "cancelled". A source's default filter leaves them out; a definition can ask for them.
- **A source's version** goes up when a field is renamed or removed. A definition records the version it was made with, and a definition naming a field that no longer exists opens with that column marked, not refused (D9).
- **No SQL from users.** The sources run their own queries, read-only (`cb4sqlite::Query` refuses anything else).

---

## 5. The report definition

### 5.1 What it holds (D2)

- **Name and description** (the description is what Custom Report lists); a title and an optional subtitle; portrait or landscape.
- **The source** and its version.
- **Columns:** fields in order, each with a label (default: the field's), a width, and for amounts and integers "blank when zero" and "total".
- **Sort:** up to three fields, each ascending or descending.
- **Groups:** up to two fields. Each group prints a header (the value, and for an account or a customer its name), a footer with the totalled columns, and optionally starts a new page (a statement-like report: one customer a page).
- **Filters:** conditions on fields, all of which must hold: equals, not equal, in a range, one of a list, contains (text), blank / not blank. A condition's value is fixed, or asked when the report runs (D6).
- **Totals:** a grand total row for the totalled columns, and a row count, each optional.

### 5.2 Kept in the company database

A port table, **`GL_RPTDEF`**: number, name, description, source, source version, the definition text (a memo, §5.4), who made it and when, who changed it last and when. A row per report. Backup and Restore Databases carry it with the books.

### 5.3 Calculated columns (Q3)

A column can be one of these instead of a field:
- **the sum or difference of two amount fields** (debit − credit, amount − received);
- **a count** (on group and total rows: how many rows);
- **a percentage of the group's or the report's total** for an amount column.

That covers the reports Q1's examples need; anything more is a built-in report (§1.2).

### 5.4 The text format

The definition is kept as plain UTF-8 text, a line per setting, so it diffs, exports (§1.1 #6) and can be read by a person:

```
writer 1
source sales_lines 1
title Sales by product
columns product_name, med_rep, qty:total, sales:total, vat:total
sort product_name
group product_name
filter period between ? "Periods"
filter state <> cancelled
```

The parser refuses what it doesn't know with the line number. A newer program reads an older definition; an older program refuses a newer one, saying so.

---

## 6. The screens

### 6.1 Custom Report (3068, dialog 1022) (D7)

- **The list** shows the built-in reports, as now, then the user reports, marked as such, by description.
- **Print** (and double-click) runs the chosen one; a user report with asked values asks first (D6).
- **Add** opens the designer on a new definition; **Modify** on the chosen user report; **Copy** on a copy of it; **Delete** asks, then removes it. Modify, Copy and Delete do nothing on a built-in report, and say so.
- **Export** writes the chosen definition to a `.gldef` text file; **Import** reads one, refusing an unknown source or a newer format, and renaming on a clash.

### 6.2 The designer (`IDD_RW_DESIGN`) (D5)

One dialog, top to bottom:
- **Report:** name, description, title, subtitle, orientation.
- **Source:** a combo of §4's sources. Changing it on a definition with columns asks first.
- **Fields and columns:** the source's fields on the left; the report's columns on the right, with Add, Remove, Up, Down, and for the selected column its label, width, blank-when-zero and total.
- **Sort and group:** three sort fields, two group fields, "new page per group".
- **Filters:** a grid of conditions (field, test, value or "ask", the question to ask).
- **Preview** runs the definition as it stands, asking its questions, in the usual preview window, without saving. **Save** checks it (a name, a source, at least one column) and saves; **Close** asks to save changes.

### 6.3 Asked when run (`IDD_RW_PROMPT`) (D6)

A dialog built from the definition's asked conditions: a label per question, and an input that suits the field: a period combo for a period, two date boxes for a date range, account and dept boxes with the account list for an account, a combo of customers or vendors for those. The answers are kept until the program exits, as the built-in reports' options are.

---

## 7. Running a report

`ReportWriter`'s builder turns a definition, the `ReportContext` and the answers into an `rpt::Report`: it reads the source's rows, filters, sorts, groups, totals in cents, and fills the columns. The preview, page layout, printing and fonts are Spec #3's, unchanged, so a user report prints and pages like a built-in one, and its footer note is the definition's name.

- **Large reports.** A source can return a great many rows (every GL line of every period). Past 20,000 rows the builder stops and asks to narrow the filters, rather than lay out thousands of pages (R3).
- **Totals are the builder's**, in cents, not the engine's: the model has no formulas (`Report.h`, Spec #3 §4.1).

---

## 8. Deviations from the original (decision record)

| # | Decision | Why |
|---|---|---|
| D1 | **No `.RPT` files**, read or written. | The format is undocumented 16-bit Crystal (§2.3); nothing in or around the port runs it (§2.4 #1); every report the deployed `GL_REPT` names is now built in (§2.2). Writing them would mean reverse-engineering a format for files no one could open, beside an engine the port already has. |
| D2 | **Definitions are data** in the company database (`GL_RPTDEF`), as versioned plain text. | They travel with the books (backup, restore), can be exported, and can be read and checked. |
| D3 | **Fixed data sources, defined in code**, not user SQL. | The legacy tables' names and meanings mislead (`AR_ACCT` is the sales account, `PERIOD` is text in some tables, amounts need converting); the sources get that right once, and nothing user-typed reaches the database. |
| D4 | **Tabular reports in the engine's layout**, not a page designer. | The engine already does headers, groups, totals, pages and printing; every built-in report fits it. |
| D5 | **One designer dialog with a live Preview**, replacing `GL_ADDREPORT`. | The original's dialog only registered a file. |
| D6 | **Filters can ask when the report runs.** | One "Sales by product" serves every period. |
| D7 | **Custom Report lists built-in and user reports together**; Add, Modify, Delete come back, with Copy, Export and Import. Modify changes the row (the original appended a duplicate, §2.4 #2), and opening the list closes nothing (§2.4 #3). | Spec #3 D9 hid them because there was nothing to add. |
| D8 | **Who may design** (Q2): a new access item, "Report Writer", in User Access; without it, Custom Report runs reports but Add, Modify, Copy, Delete and Import are greyed. | Running a report changes nothing; a badly made one can mislead whoever runs it. |
| D9 | **A definition that no longer fits its source** (a field renamed in a later version) opens in the designer with the column marked, and runs without it, saying so. | A program update shouldn't lose a user's work. |

---

## 9. Development fixture

No new data: the sources read the fixture the earlier specs built (the GL, fixed assets, AP and AR data). The fixture gains three example definitions in `GL_RPTDEF` (sales by product, GL lines by account for a period range, AR open items by customer), which the tests and the acceptance sheet use.

---

## 10. Acceptance criteria (outline)

1. **Make a report:** "Sales by product" from Sales lines, grouped by product, with quantity, sales and VAT totalled; preview it: the product totals add to the posted sales vouchers' sales, and the VAT to their VAT lines.
2. **Ask when run:** the same report asking for a period range; two runs, two periods, the right figures each time.
3. **Groups and pages:** AR open items grouped by customer, a new page each: it matches the Open Receivables Detail's figures.
4. **Filters:** GL voucher lines on 410001 for periods 7–8, cancelled vouchers left out, then included.
5. **Calculated columns:** debit − credit; a percentage of the total.
6. **Manage:** Modify changes the report (no duplicate), Copy makes a second one, Delete removes it; built-in reports can't be modified or deleted.
7. **Export and import** between two copies of the company.
8. **Access:** a user without "Report Writer" runs reports but can't design them.
9. **Safety and limits:** a report over 20,000 rows asks to narrow; a definition with an unknown field opens with it marked; nothing the writer does changes a table but `GL_RPTDEF`.
10. **Regression:** the Spec #1–#6 sheets pass; `/checkres` and `/stress` exit 0; the built-in reports are unchanged.

---

## 11. Risks

| # | Severity | Risk | Mitigation |
|---|---|---|---|
| R1 | Medium | Users want Crystal's freedom (any layout, formulas), and the writer grows toward it. | §1.2 draws the line; Q1 sets the first reports; anything beyond is a built-in report or a later spec. |
| R2 | Medium | A source's fields are misread (a balance summed across account types, a cancelled voucher counted). | Field labels say what they are; default filters leave out the cancelled; the acceptance checks reconcile user reports with built-in ones. |
| R3 | Low | Very large reports are slow or use much memory. | The 20,000-row stop (§7); filters asked when run. |
| R4 | Low | Definitions break across program versions. | Versioned sources and format (§4, §5.4, D9), with tests. |
| R5 | Low | The analysis codes (med rep, product, doctor) are sparsely or inconsistently filled in the deployed data, so reports by them disappoint. | Check the deployed data before Q1 settles the first reports. |

---

## 12. Milestones (the plan comes after review)

| Milestone | Content |
|---|---|
| S7-M0 | `ReportWriter.{h,cpp}`: the model, the text format, the builder; three sources (Sales lines, GL voucher lines, AR open items); `GL_RPTDEF`; tests |
| S7-M1 | Custom Report: user reports listed and run; Delete; the fixture's example definitions |
| S7-M2 | The designer (`IDD_RW_DESIGN`) with Preview; Add, Modify, Copy |
| S7-M3 | Asked when run (`IDD_RW_PROMPT`); calculated columns |
| S7-M4 | The other sources (§4) |
| S7-M5 | Export and import; the access item (D8); the 20,000-row stop |
| S7-M6 | Acceptance sheet, walkthrough, sign-off |

---

## 13. Still open

Questions for the user and the finance team, with the spec's assumption until they answer.

- **Q1 The first reports:** which reports do people want to make? *Assumed: sales by product, by med rep and by doctor; GL lines by account for a period range; open items by customer or vendor. They pick the M0 sources.*
- **Q2 Who may design reports?** *Assumed: users given the new "Report Writer" item in User Access (D8); everyone who can open Custom Report can run them.*
- **Q3 Calculated columns:** are §5.3's enough? *Assumed: yes.*
- **Q4 Export of a report's rows** (to CSV, for Excel), from the preview? *Assumed: yes, from any preview, built-in reports too, as a small addition in S7-M5.*
- **Q5 SQL for an administrator** as an extra source? *Assumed: no (D3).*
- **Q6 Moving definitions between companies:** needed? *Assumed: yes, by export and import of a text file.*
- **Q7 Scope:** the milestones above (about the size of Spec #6), or a smaller first step (M0–M2 only: three sources, no prompts or calculated columns)? *Assumed: all of §12.*
