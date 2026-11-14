# Spec #7 — Implementation Plan: Report Writer

**Spec reference:** `gl/docs/superpowers/specs/2026-11-14-owl-to-win32-sdk-port-spec7-design.md`
**Date:** 2026-11-14
**Plan status:** In progress (S7-M0 to M2 built)

**Paths:** relative to the repository root. The legacy sources are `gl/src/gl_rept.cpp` and `gl/src/ar_rept.cpp` (`TGLCustomReport`, `TGLAddReport`, `TGLModifyReport`, `TGLDeleteReport`), dialog 1022 and `GL_ADDREPORT` in `gl/gl_new.rc`, and `dbf/GL_REPT.DBF`. New code goes in `gl/port/` (the portable `ReportWriter.{h,cpp}`) and `gl/port/reports/` (the designer, the prompt dialog, the report list and the runner). New dialog templates go in `gl/res/port.rc` with their IDs in `gl/port/PortIds.h`, as `IDD_AP_CONFIRM` did.

This plan carries out the Spec #7 design. If a milestone finds that the spec is wrong, **stop, fix the spec, then resume**.

**Read first, as in Specs #2–#6.** Each milestone starts by re-reading what it touches: the original's custom report code and dialogs for M1 and M2, and for each data source (M0, M4) the tables and the port's own loaders (`Accounts.h`, `Vouchers.h`, `Assets.h`, `Ap.h`, `Ar.h`, `MasterTables.h`), whose meanings the sources must agree with. A source never reads a table its owner's loader would read differently.

**Open questions.** Q1–Q7 (spec §13) stay as the spec assumes until answered:
- the first sources are sales lines, GL voucher lines and AR open items (Q1);
- designing is behind a new User Access item (Q2);
- §5.3's calculated columns are enough (Q3);
- CSV export is from any preview (Q4);
- there is no SQL source (Q5);
- definitions move by export and import (Q6);
- all of §12 is built (Q7).

Each milestone notes where an answer would change it. **If Q7 chooses the smaller first step**, M0–M2 stand alone: M3–M5 are then a later spec.

**Where it runs.** As Specs #5 and #6: Windows 11 with Visual Studio 18 (v145), each milestone built, tested and driven in the app as it is written. New portable files join `tools/linux/build_tests.sh`.

**Reused, not copied.** The builder fills an `rpt::Report` (`report/include/report/Report.h`) and runs it through `RunReport` (`gl/port/reports/RunReport.h`), whose options hook carries the prompt dialog (M3). `ReportContext` gives the company, the current period and its dates. The lookups the designer and prompts need (the account list, periods, customers, vendors) are the screens' own (`AccountList`, `OpenPeriods`, `LoadCustomers`, `LoadVendors`).

---

## Milestone summary

| # | Milestone | Size | Depends on | Critical path? |
|---|---|---|---|---|
| 0 | `ReportWriter` core: the model, the text format, the builder, three sources, `GL_RPTDEF`; tests | M | none | yes |
| 1 | Custom Report: user reports listed, run and deleted; the fixture's example definitions | S | M0 | yes |
| 2 | The designer (`IDD_RW_DESIGN`) with Preview; Add, Modify, Copy | M | M1 | yes |
| 3 | Asked when run (`IDD_RW_PROMPT`); calculated columns | M | M2 | yes |
| 4 | The other sources (spec §4) | M | M0 | no |
| 5 | Export and import; the access item (D8); the row limit; CSV export (Q4) | S | M2 | no |
| 6 | Acceptance sheet, walkthrough, sign-off | S | all | yes |

S is less than a day, M is 1–3 days. **Parallel work:** M4 needs only M0, and can run beside M1–M3; M5 needs M2, and can run beside M3.

---

## Milestone 0 — The portable core

> **As built (2026-11-14; built and run on Windows 11, VS 18, v145).**
> - **Read** (task 1): the engine draws what the builder needs (group headers and footers with an indent, page breaks, notes, totals). `gl_btchd` keeps its codes as `PROD`, `MEDREP`, `DRCODE` and its `PERIOD` as text; `gl_btch` modes are 1 open, 0 posted, 3 adjustment, 2 posted adjustment, 9 closed year. The master tables are `gl_itemm` (`FPCODE`, `FFULLN`), `gl_medrp` (`REPCODE`, `RPNAME`) and `gl_drcod` (`DRCODE`, `RPNAME`). A memo is a `'M'` field, as `ar_btch.MEMOFIELD`.
> - **`gl/port/ReportWriter.{h,cpp}`** (task 2), in namespace `rw`:
>   - **The model and format:** `ReportDefinition`, `ParseDefinition` (refusals name their line), `WriteDefinition` (round-trips exactly). The format's grammar is in the header.
>   - **Spec change (§5.4):** one `column` line per column with its options (`total`, `blank`, `label`, `width`), instead of one `columns` line; an asked filter has a `?` per value, then its question. The spec's example now follows it.
>   - **Sources:** `Sales lines` (through `LoadArVouchers` / `LoadArLines`, so each line's net and VAT are the ledger's; cancelled left out by default), `GL voucher lines` (`gl_btchd` with its header's state and the account's name), and `AR open items` (`LoadArOpenItems` with the customer's name). A code field names its name field, which a group header shows beside the code.
>   - **`BuildReport`:** the definition's filters (asked values from `Answers`), then the source's defaults for fields it doesn't filter. Then it sorts (groups first), groups with indented headers and totalled footers (a page break for "page" groups), adds a grand total and counts. A field the source lacks becomes a note (D9). Refusals: an unknown source, no column, an unanswered or unreadable value, and more than 20,000 rows.
>   - **Values:** filter values parse by the field's type: amounts like "2,200.00", dates as yyyymmdd or mm/dd/yyyy (checked: S7-M0's tests caught 31/12/2026 passing as a date), and text without regard to case or surrounding spaces.
>   - **`gl_rptdef`:** `EnsureReportWriterTables`, `LoadDefinitions` (name order), `FindDefinition`, `SaveDefinition` (new as the highest + 1, or replaced in place, keeping who made it), `DeleteDefinition`, `DefinitionNameTaken`.
> - **Tests** (task 3), `gl_tests/test_reportwriter.cpp`, five cases:
>   - the format: a round trip of every setting, defaults, and 16 refusals with their lines;
>   - each source against its module's figures (sales lines' posted net 3,800.00 and VAT 300.00 equal the vouchers' sales and VAT lines; GL lines equal `gl_btchd`; open items equal `LoadArOpenItems`);
>   - the builder: sales by product (headers with the product's name, footers, the grand total 3,800.00 / 300.00); two groups with pages; counts; the cancelled default and asking for it; sort descending; no total;
>   - every filter test, asked values and their refusals, a missing field's notes, the refusals, an empty result;
>   - `gl_rptdef`: save, replace, load, name taken, delete, in an empty company and the fixture.
>
>   `gl_tests`: 168 cases (`[reportwriter]` 5).
> - **Build lists** (task 4): `gl_tests.vcxproj`, `build_tests.sh`; `gl.vcxproj` by `genproj.py` (`gl.exe` links the core; nothing calls it until M1).
> - **Checked:** Debug and Release `/W4 /WX` clean; `gl_tests`, `cb4sqlite_tests` and `report_tests` pass; `/checkres` 0; `/stress` 0 (164 s).

**Goal:** a definition, as text or in memory, turns into an `rpt::Report` from real data, under test.

### Tasks

1. **Read** `Report.h` and `Layout.cpp` (what the engine can show: groups, footers, totals, notes, page breaks), the loaders the three first sources rest on (`LoadArLines` and `ar_linex` with `SplitSale` for sales lines, Spec #2's voucher tables for GL voucher lines, `LoadArOpenItems`), and how `cb4sqlite` keeps a memo field (`ar_btch.MEMOFIELD`).
2. **`gl/port/ReportWriter.{h,cpp}`**, portable (no Win32), over `cb4sqlite` and `report`:
   - **The model** (spec §5.1): `ReportDefinition` with name, description, title, subtitle, orientation; the source and its version; columns (field key or calculated, label, width, blank-when-zero, total); up to three sort keys; up to two groups with "new page"; filters (field, test, value or "ask" with its question); grand total and row count.
   - **The text format** (§5.4): `ParseDefinition(text)` → a definition or the error with its line number; `WriteDefinition(def)` → text. Round-trips exactly. A newer `writer` version is refused, saying so.
   - **Sources** (§4) as a registry: `Sources()` lists them; each has a key, a name, a version, its fields (key, label, type, width), a default filter, and `Rows(code, ctx)` that loads its rows as typed values (text, cents, integers, yyyymmdd dates). M0 gives three, picked by Q1:
     - **Sales lines** (`ar_btchd` with `ar_btch`, `ar_linex`, the customer, and the product, med rep and doctor names), each line's net and VAT share from `SplitSale`;
     - **GL voucher lines** (`gl_btchd` with `gl_btch`, the account's name, the state);
     - **AR open items** (`ar_dist` with the customer).
   - **The builder** (§7): `BuildReport(code, ctx, def, answers)` → `rpt::Report`. It filters (the asked values from `answers`; M3 adds the dialog), sorts, groups (headers with the value and, for an account or customer, its name; footers with the totalled columns; page breaks), totals in cents, and counts. A field the source no longer has makes a note and is left out (D9). Past 20,000 rows it stops with a refusal the caller shows (§7).
   - **`GL_RPTDEF`** (§5.2): `EnsureReportWriterTables`, and `LoadDefinitions`, `FindDefinition`, `SaveDefinition` (new or replace, with who and when) and `DeleteDefinition`. The definition text is a memo.
3. **Tests**, `gl_tests/test_reportwriter.cpp`:
   - the format: round trips; every setting; refusals with their line numbers; a newer version refused;
   - each source's rows on the fixture, against the owning module's own figures: sales lines' net and VAT add to the posted vouchers' sales and VAT lines; GL lines to Spec #2's voucher totals; AR open items to `LoadArOpenItems`;
   - the builder: columns, sort (ascending, descending, ties), one and two groups with their footers, new page per group, grand total, count, each filter test, an unknown field (noted, left out), the row limit;
   - `GL_RPTDEF`: save, replace, load, delete, in an empty company and in the fixture; nothing else changes.
4. **Build lists:** `ReportWriter.cpp` and the test in `gl_tests.vcxproj` and `tools/linux/build_tests.sh`; `ReportWriter.{h,cpp}` in `gl.vcxproj` (`genproj.py`).

### Exit criteria

- `gl_tests` passes in Debug and Release.
- "Sales by product" written as text builds from the fixture with totals that agree with the AR reports.

---

## Milestone 1 — Custom Report: list, run, delete

> **As built (2026-11-14; built and run on Windows 11, VS 18, v145).**
> - **Read** (task 1): `TGLCustomReport` as spec §2.1. The port's dialog 1022 returned a built-in report's command; `ManilaGL::RunReportList` ran it.
> - **`ReportList.{h,cpp}`** (task 2):
>   - The list shows the 55 built-in reports, then the user reports by **name** (not description, which is longer than the list is wide: spec §5.1 and §6.1 now say so), with a "Made by" column.
>   - It returns a `ReportChoice`: a built-in command or a `gl_rptdef` number.
>   - **Delete** shows again, in its own place (Print and Close take Add/Mod's and Modify's, which stay hidden until M2). It asks, then `DeleteDefinition`. On a built-in report it says it can't.
> - **`RunReport.cpp`** (task 3): `RunUserReport` finds the definition, parses it (a definition that can't be read says which line) and builds it. A refusal is a message and opens nothing; until M3, that covers a report that asks a value. Otherwise it goes to the usual preview, its footer the report's name. `ManilaGL` runs a chosen definition through it.
> - **The fixture** (task 4): `report_writer()` writes `GL_RPTDEF` (`.DBF`, `.DBT`, `.MDX`) with three examples, as `WriteDefinition` writes them: *Sales by product*, *GL lines by account* (period 7, landscape), *Open items by customer*.
>   - The fixture's AR sales lines get med rep, product and doctor codes from the master lists (they had none), so Sales by product groups by product.
>   - Regenerating also rewrote `ACCESSL` / `ACCESSM`, which `make_dev_fixture.py` builds from the menu; they lag Spec #6's menu items. That's left for M5, the access item, and those two files are kept as they were.
>   - Every other table is byte-identical.
> - **`/stress`** (task 5) previews and pages each user report that builds; `/checkres` is unchanged.
> - **Tests** (task 6): the examples parse, are canonical, build, and agree with their modules: sales 3,800.00 / VAT 300.00 by product; the period-7 GL lines equal `gl_btchd`'s; 2,800.00 owed by C001 and C002. M0's `gl_rptdef` test now starts from the fixture's three. `gl_tests`: 169 cases (`[reportwriter]` 6), the AR tests unchanged by the new codes.
> - **Checked:** Debug and Release `/W4 /WX` clean; `gl_tests` passes in both; `/checkres` 0; `/stress` 0 (173 s, the three examples among the previews). Driven in the app, on a scratch copy migrated from the new fixture, by messages to its own windows:
>   - Custom Report lists 55 + 4 reports (a fourth, "Asks a period", added by `sqlite3`);
>   - GL lines by account and Sales by product open their previews;
>   - "Asks a period" says "The report asks: Periods. Answer it to run the report." and opens nothing;
>   - Delete on Trial Balance: "built into the program"; on "Asks a period": asks, then it's gone;
>   - Trial Balance still runs from the list (its options dialog).
>
>   Screenshots checked the list and both previews.

**Goal:** user reports show in Custom Report and run like the built-in ones.

### Tasks

1. **Read** `TGLCustomReport` (`ar_rept.cpp`, `gl_rept.cpp`), dialog 1022, the port's `ReportList.cpp` (Spec #3 D9: Add/Mod, Delete, Modify hidden, and the list returns the chosen command) and `ManilaGL::RunReportList`.
2. **`ReportList.cpp`:** the list shows the built-in reports, then the user reports from `GL_RPTDEF`, marked (a second column, "Yours"). The dialog returns either a built-in command, as now, or a definition's number; `ManilaGL` runs a definition through `RunUserReport` (below). **Delete** shows again: it asks, then `DeleteDefinition`; on a built-in report it says it can't. Opening the list closes no table (spec §2.4 #3).
3. **`RunReport.cpp`:** `RunUserReport(frame, number)`: loads the definition, builds it with `BuildReport`, and opens the preview; a refusal (the row limit, a definition that doesn't parse) is a message. Its footer note is the definition's name.
4. **The fixture:** three example definitions in `GL_RPTDEF` (spec §9): sales by product, GL lines by account (a fixed period range until M3), AR open items by customer.
5. **`/stress`:** each example definition previewed and paged, as the built-in reports; `/checkres` unchanged.
6. **Tests:** the list's entries with user reports; deleting one; the examples build.

### Exit criteria

- The three examples run from Custom Report on the fixture; Delete removes one; built-in reports can't be deleted.

---

## Milestone 2 — The designer

> **As built (2026-11-14; built and run on Windows 11, VS 18, v145).**
> - **Read** (task 1): `GL_ADDREPORT` took a description, a name and a path. The port's Statement Layout dialog is the pattern: a `MainDialog`, its working copy in memory, and a preview over the dialog with `rpt::win32::ShowPreviewWindow`.
> - **`IDD_RW_DESIGN`** (task 2, `port.rc`; IDs `IDC_RW_*` 20180–20217 in `PortIds.h`). One dialog, 470 × 316 units:
>   - name, title, description, subtitle, landscape; the source, with its description beside it;
>   - fields and columns, with Add, Remove, Up, Down, and the selected column's label, width, total and blank-when-zero (the last two only for amounts and numbers);
>   - three sort fields with "desc.", two groups with "new page";
>   - the filter list, with a field, a test (is, is not, from ... to, one of, contains, is blank, isn't blank) and one or two values, Add filter and Remove filter;
>   - grand total, row counts, Preview, Save, Close.
> - **`gl/port/reports/ReportDesigner.{h,cpp}`** (task 3), `RunReportDesigner(owner, number, copy)`:
>   - **Editing:** the working copy is a `ReportDefinition`, and each control writes its part as it changes. A new column on an amount starts totalled.
>   - **Changing the source** with columns asks first, and clears what named the old fields.
>   - **Preview** builds the definition as it stands into a preview window over the dialog.
>   - **Save** runs `CheckDefinition`, then `SaveDefinition` in place: Modify changes its row, with no duplicate. Without logins the user is stored blank, not "NOLOGIN".
>   - **Close** asks to save changes.
>   - **Missing fields:** a field the source no longer has shows "(missing)", with a message on opening (D9). Asked filters are kept and shown, and made in M3.
> - **`ReportWriter`** (task 6): `CheckDefinition` (name, 40 at most, not another's; source; a column; each filter's values), `FilterProblem` (a filter's values suit its field's type: the drive caught "Date is one of Posted"), `CopyName` ("Copy of X", "(2)", ...), `MissingFields`, `DescribeFilter`.
> - **`ReportList.cpp`** (task 4):
>   - **The buttons:** Add/Mod is "Add...", Modify is "Modify...", and a Copy button is added in code (`IDC_RW_COPY`, `tbSuperBtn` like the others). The six are stacked in the template's column: Print, Add, Modify, Copy, Delete, Close.
>   - **What they do:** Modify and Copy say so on a built-in report. After the designer closes, the list refills and selects the report saved.
> - **Wire** (task 5): `/checkres` loads `IDD_RW_DESIGN`; `/stress` gains a designer step, `StressReportDesigner`, which opens and closes it on a new and an existing report.
> - **Tests:** two cases (`CheckDefinition`, `CopyName`, `MissingFields`, `DescribeFilter`; `FilterProblem`). `gl_tests`: 171 cases (`[reportwriter]` 8).
> - **Checked:** Debug and Release `/W4 /WX` clean; `gl_tests` passes in both; `/checkres` 0; `/stress` 0 (211 s). Driven in the app, on a scratch copy migrated from the fixture, by messages to its own windows:
>   - **Add:** "Sales by med rep", built from nothing: four columns (sales and VAT totalled), grouped by med rep, filtered to posted and transferred. "Date is one of Posted" was refused first.
>   - **Preview:** MR001 2,800.00 / 200.00, MR002 1,000.00 / 100.00, total 3,800.00 / 300.00.
>   - **Save:** report 4, with its description and a blank user.
>   - **Modify:** a new title, then Close, asked, Yes; the same row changed.
>   - **Copy:** "Copy of Sales by med rep" saved as report 5.
>   - **Built-in reports:** Modify and Copy on Trial Balance say so.
>   - **Refusals:** Save with no name says "Give the report a name."; changing the source with columns asks, and No keeps it.
>   - **Run:** the new report runs from the list ("Sales by medical representative").
>
>   Screenshots checked the designer and the preview.

**Goal:** a user makes, changes and copies a report without code.

### Tasks

1. **Read** the original's `GL_ADDREPORT` and `TGLAddReport` / `TGLModifyReport` (Modify appended a duplicate: spec §2.4 #2), and the port's list-and-detail dialogs for patterns (Accounts > Statement Layout, from Spec #3, is the nearest: a grid, fields for the selected row, Apply).
2. **`IDD_RW_DESIGN`** in `port.rc` (spec §6.2): report (name, description, title, subtitle, orientation), source combo, the source's fields and the report's columns (Add, Remove, Up, Down; label, width, blank-when-zero, total for the selected column), three sort fields, two group fields with "new page", the filter grid (field, test, value; "ask" comes in M3), Preview, Save, Close.
3. **`gl/port/reports/ReportDesigner.{h,cpp}`:** loads and edits a `ReportDefinition`; changing the source on a definition with columns asks; **Preview** builds the definition as it stands into the usual preview, without saving; **Save** checks (a name, unique; a source; a column) and saves; **Close** asks to save changes. A definition opened with a field its source no longer has shows the column marked (D9).
4. **`ReportList.cpp`:** **Add** opens the designer on a new definition, **Modify** on the chosen one (changing that row: no duplicate), **Copy** on a copy named "Copy of …"; Modify and Copy say so on a built-in report.
5. **`/checkres`** loads `IDD_RW_DESIGN`; **`/stress`** opens and closes it.
6. **Tests** (portable): the designer's checks, as functions in `ReportWriter` (a name's uniqueness, a definition's completeness), and copy naming.
7. **Driven in the app:** make "Sales by med rep" from nothing; preview; save; modify; copy; delete.

### Exit criteria

- On the fixture: a report made in the designer previews with the same figures as the same definition written as text in M0's tests.

---

## Milestone 3 — Asked when run; calculated columns

**Goal:** one report serves every period; the few calculations spec §5.3 allows.

### Tasks

1. **The model and format:** a filter's value can be "ask", with its question (`?` in §5.4's format); calculated columns: the sum or difference of two amount fields, a count, a percentage of the group's or the report's total (§5.3; Q3).
2. **`IDD_RW_PROMPT`** in `port.rc` and **`gl/port/reports/ReportPrompt.{h,cpp}`** (spec §6.3): built from the asked filters, one input per question, suiting the field: a period combo (`OpenPeriods`), two date boxes, account and dept with the account list, a customer or vendor combo; text otherwise. It is `RunReport`'s options hook, so Cancel stops the run. Answers are kept until the program exits, per definition.
3. **The designer:** "Ask when run" and the question in the filter grid; calculated columns in the column list.
4. **The builder:** answers applied; calculated columns computed in cents (a percentage to two places, of the group's total in a group, of the report's in the grand total).
5. **`/checkres`** loads `IDD_RW_PROMPT`.
6. **Tests:** each asked kind; two runs with two answers; each calculated column, with groups; a percentage column adds to 100.00 per group (rounding noted when it doesn't, as the engine prints it).

### Exit criteria

- The GL lines example asks for a period range; two runs give the two periods' figures. Spec §10 #2 and #5 pass on the fixture.

---

## Milestone 4 — The other sources

**Goal:** every source of spec §4.

### Tasks

1. **Read** each owner's loader and its tables again: `Accounts.h` (balances, period movements, prior year, budget), Spec #2's `gl_dist`, `Assets.h`, `Ap.h` (vendors, open items, purchase lines with `ap_linex`), `Ar.h` (customers, receipts), `MasterTables.h`.
2. **Sources:** Accounts, Posted ledger lines, Fixed assets, Vendors, Customers, AP open items, AR receipts, Purchase lines, Med reps, Products, Doctors. Each with its default filter (cancelled left out) and field labels that say what a field is (`AR_ACCT` is "Sales account").
3. **Tests:** each source's rows against its owner's own figures on the fixture (the Trial Balance, the fixed-asset listing, the AP and AR reports' totals).

### Exit criteria

- Every source builds on the fixture, and each agrees with the built-in report that covers the same data.

---

## Milestone 5 — Export, import, access, limits, CSV

**Goal:** definitions move between companies; designing is a right; large reports are stopped; rows go to Excel.

### Tasks

1. **Export and import** (spec §6.1, Q6): in Custom Report, Export writes the chosen definition's text to a `.gldef` file (a Save As dialog); Import reads one, refusing an unknown source or a newer format, and renaming on a name clash.
2. **The access item** (D8, Q2): **read** `Login.cpp` (`ApplyMenuAccess`, `accessm`, `accessl`) and User Access (4078) first. Add a "Report Writer" right: a new `accessm` row, added by `EnsureReportWriterTables` when missing, so User Access lists it. Without it, Add, Modify, Copy, Delete and Import are greyed; running is unaffected.
3. **The row limit** (§7): the 20,000-row refusal shown in the app, naming the asked filters to narrow.
4. **CSV export** (Q4): an Export button on the preview window writes the report's detail, group and total rows as CSV (UTF-8 with a BOM, amounts as plain numbers), for any report, built-in or user.
5. **Tests:** export then import round trips; refusals; the name clash; the access row added once; CSV of a grouped report.

### Exit criteria

- A definition exported from one copy of the fixture imports into another and gives the same figures; a user without the right can run but not design; CSV opens in Excel with the preview's figures.

---

## Milestone 6 — Acceptance

1. **`gl/docs/superpowers/specs/2026-11-14-spec7-acceptance.md`**, in the Spec #6 sheet's shape, with spec §10's checks, the fixture's figures and the `sqlite3` queries that produce them; a `gl_tests` case that makes the same reports and checks every figure.
2. Release build check, the walkthrough with a witness, triage, sign-off.

---

## Cross-cutting

- **Per-port checklist** from the Spec #1 plan for every ported file. Each screen's header lists its translation notes, naming the decisions it applies.
- **Per-milestone gates:**
  - `/W4 /WX` clean on Debug and Release x64;
  - the milestone's tests pass;
  - `/checkres` and `/stress` exit 0;
  - a spec drift check;
  - an entry in `gl/docs/superpowers/plans/2026-05-10-progress.md`;
  - the milestone's "As built" note here.
- **What not to do:**
  - Don't write to any table but `GL_RPTDEF` (and, in M5, the one `accessm` row): a report reads and changes nothing.
  - Don't run a query built from user text: sources run their own, read-only.
  - Don't turn built-in reports into definitions (spec §1.2), and don't change them.
  - Don't touch `GL_REPT` (Spec #3 D9).
  - Don't edit `gl/res/gl.rc` by hand: new templates go in `port.rc`, menu changes in `prune_rc.py`.

## Risks specific to the plan

| # | Severity | Risk | Mitigation |
|---|---|---|---|
| P1 | Medium | Q1's answer asks for reports the sources or §5.3 can't make. | M0's three sources are picked by Q1; a report beyond the writer is a built-in report, in a later spec. |
| P2 | Medium | A source disagrees with its module's own figures (a sign, a cancelled row, a VAT share). | Every source is tested against the owning module's figures (M0, M4). |
| P3 | Medium | The designer dialog is crowded on a small screen. | One dialog, as spec §6.2, sized for 100 % at 1366 × 768; tabs only if M2's drive shows it doesn't fit. |
| P4 | Low | The prompt dialog's inputs vary by field and grow. | A fixed set of input kinds (M3), the rest as text. |
| P5 | Low | The `accessm` row upsets User Access or an existing user's rights string (one character per `accessm` row, in its tag order). | M5 reads the access code first, and gives the new row a key that sorts last, so existing rights strings keep their positions; a test checks an existing user's rights before and after. |

## How to use this plan

M0 → M1 → M2 → M3, with M4 beside M1–M3 and M5 after M2, then M6. If the plan and the spec conflict, stop, edit the spec, recheck the plan, then continue.
