# Spec #7 — Implementation Plan: Report Writer

**Spec reference:** `gl/docs/superpowers/specs/2026-11-14-owl-to-win32-sdk-port-spec7-design.md`
**Date:** 2026-11-14
**Plan status:** Draft (no milestone built)

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
