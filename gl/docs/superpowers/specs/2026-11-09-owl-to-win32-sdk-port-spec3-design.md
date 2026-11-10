# Spec #3 — OWL-to-Win32 SDK Port: General Ledger Reports

**Status:** Draft
**Date:** 2026-11-09
**Scope:** Third spec of the migration. It replaces the Crystal Reports print engine with a small native report engine (page model, pagination, on-screen preview, GDI printing). On it, it builds a **finance set** of new reports — Income Statement, Balance Sheet, Cash Flow Statement, a Distribution Trial Balance and a plain Trial Balance — driven by a statement layout the finance team maintains, and then ports the original General Ledger reports.
**Depends on:** Spec #1 (`2026-05-10-owl-to-win32-sdk-port-design.md`) and Spec #2 (`2026-11-08-owl-to-win32-sdk-port-spec2-design.md`). Everything they fixed still holds: the build, the error idioms, the Unicode strategy, the translation pattern and the per-port checklist. Spec #2's voucher modes (`Vouchers.h`) and year end are used and, for year end, changed (D16).

**Numbering.** This is seam 3 of Spec #1 §9 ("Reporting"). Its scope is the **General Ledger**. The AP and AR reports are ported with their modules (Specs #5 and #6) on this spec's engine; the fixed-asset reports go with Fixed Assets (Spec #4).

---

## 1. Goals & Non-Goals

### 1.1 Goals (Spec #3 deliverables)

1. **`report`, a reusable report library** (§4): a portable core (page model, columns, groups, totals, pagination, formatting), built and tested on Linux, and a Win32 part (preview MDI child, printing through the Print dialog, Printer Setup).
2. **The finance set** (§5), each with an options dialog, a preview, and printing:
   - **F1 Trial Balance** (6076): every account's balance at the end of a period, in debit and credit columns.
   - **F2 Distribution Trial Balance** (1075): for a range of periods, each account's opening balance, its debits and credits from posted vouchers, and its closing balance; optionally the voucher lines themselves.
   - **F3 Income Statement** (1015): month and year to date, optionally against the prior year and the budget, consolidated or for one department.
   - **F4 Balance Sheet** (1016): at the end of a period, against the start of the year.
   - **F5 Cash Flow Statement** (1018, renamed): indirect method, month and year to date.
3. **The statement layout** (§6): two new tables that say which account ranges make up each statement line, with headings, subtotals and signs; a default built from the company's account numbering; and a **Statement Layout** screen to maintain it.
4. **The original GL reports** (§3), after the finance set: Trial Balance by Dept. (6080), Chart of Accounts Report (10071), Budget Report (10073), Voucher Slips (6074), Voucher Detail (8082), Voucher Summary (6075), Preposting Distribution List (6078), General Ledger Pre-Posting Report (8136), the Voucher Entry dialog's **Print**, and the Medical Representatives (6039), Product code (6038) and Doctor Codes (6041) listings.
5. **Printer Setup** (2001) and **File > Print / Custom Report** (3068), redefined in D9 and D10.
6. **`cb4sqlite::Query`**, a read-only query helper (§7).
7. **Year end** keeps the books consistent for the statements: it sets the opening balance to the closing balance and archives the year's distribution lines (D16; settles Spec #2 §12 Q6).

### 1.2 Non-Goals

- **AP and AR reports** (Specs #5 and #6) and the **fixed-asset reports** 1019 and 1020 (Spec #4). They keep their stubs.
- **Statement of Retained Earnings and Manufacturing Statement** (the original 1018 and 1025). They never worked (§2.4); 1018's menu item becomes the Cash Flow Statement and 1025 leaves the menu (D13).
- **Running Crystal `.RPT` files**, including custom reports in `gl_rept` (D9).
- **Export** to Excel or CSV. PDF comes from printing to *Microsoft Print to PDF*.
- **A report designer.** Each report is C++; only the statements' line structure is data (§6).
- Multi-year history: the statements cover the current fiscal year and, through the `PREVIOUS<n>` columns, the one before (§2.5).
- Pixel fidelity with the Crystal layouts (D1). Hi-DPI testing of the preview.

---

## 2. Reports and balances in the original

The inventory behind this section (every handler, `.RPT` file, table, link and formula, with file:line citations) was taken in S3 planning. `.RPT` records can be re-read with `tools/rptdump.py`.

### 2.1 The print path

Almost every GL report goes through `PrintReport(name, hwnd)` (`gl/src/ap_repts.cpp:112-174`). No report shows a dialog first:

1. `PEOpenEngine`, `PEOpenPrintJob(name)` by bare file name against the company's `dbf/`. A failure shows Crystal's error text.
2. `PEGetReportTitle` → the preview window's caption.
3. `PESetFormula("Company Name", "'<gl_prof.COMPANY>'")`, from the first `gl_prof` record.
4. `PESetFormula("Current Period", "'<n>'")`, the **lowest-numbered open period** in `gl_pds` (`iGetPeriod`, `:189-206`), as a quoted string.
5. `PEOutputToWindow` (Crystal's preview, from which the user prints), `PEStartPrintJob`, `PEClosePrintJob`.

`PESetFormula`'s result is ignored, so a report without one of those formulas keeps its stored default ("Takeda", "4"…). Nothing sets a sort, group, printer or selection. **No original report takes parameters** — no account range, period, department or voucher number — and none filters by period or voucher mode, whatever its status text says.

Defects the port doesn't inherit: `char szFile[12]` overflows for 12-character names; `new char[4]` overflows for periods 10–12; `GetCompany` and `szGetPeriod` return pointers to locals; `iGetPeriod` returns an uninitialised value when no period is open.

### 2.2 The original reports

Titles, tables, links, formulas and the order of fields come from the `.RPT` records; column positions and fonts are binary and weren't decoded, and sort order is inferred from where the sort fields sit.

| ID | Menu (`gl_new.rc` MANILA) | `.RPT` | Tables | What it lists |
|---|---|---|---|---|
| 6076 | Reporting > General Ledger > Trial Balance | `GLTB.RPT` | `glaccts` | Per account+dept: opening balance, current-period amount (`PERIOD<n>`), current year (Σ`PERIOD1`–`12`), current balance; totals. Debit/credit split formulas exist but aren't on the layout. **Replaced by F1** (D14). |
| 6080 | … > Trial Balance by Dept. | `GLDEPT.RPT` | `glaccts` | Account, dept, description, opening balance, current period, "YTD", current balance. **"YTD" is `CURR_BAL − OPENBAL − PERIOD<n>`.** No department grouping. Totals. |
| 10071 | Accounts > Chart of Accounts Report | `GL_CHART.RPT` | `glaccts` | Account, dept, description, opening balance, "Accumulated This Year" (Σ periods), current balance. Totals. |
| 10073 | Accounts > Budget Report | `glbudgt.rpt` (**missing**; `GL_BUDGT.RPT` intended) | `glaccts` | Account, dept, description, current year, current-period amount, budget (`BUDGET<n>`), difference. Totals. |
| 6074 | … > Voucher Slips | `GLSLIP.RPT` | `gl_btchd`, `gl_btch` | One slip per voucher (every mode): slip number, date, slip description, reference, period; lines (journal no., account, dept, description, cheque, debit, credit); "Total Batch Entry". |
| 8082 | … > Voucher Detail | `GLBTCHD.RPT` | `gl_btchd`, `gl_btch` | Every line grouped by voucher: no., account, dept, description, cheque, date, doctor, med rep, product, debit, credit; group debit and credit totals and **"Difference", printed only when debits exceed credits**. |
| 6075 | … > Voucher Summary | `GL_BTCHS.RPT` | `gl_btchd`, `gl_btch` | One line per voucher: period, number, cheque, date, reference, debit and credit totals. |
| 6078 | … > Preposting Distribution List | `GLPRE.RPT` | `glaccts`, `gl_btchd`, `gl_btch` | By account, then dept: current balance, the voucher lines against it, count, totals, and a projected closing balance `CURR_BAL + Σdebit − Σcredit`. **Includes posted lines**, which `CURR_BAL` already contains. |
| 8136 | Transactions > Voucher Reports > General Ledger Pre-Posting Report | `gl_prept.rpt` (**missing**) | — | Crystal reports the missing file. String 8136: "Print a list of vouchers that have not yet been posted". |
| (1013) | Voucher Entry > **Print** | `GL_SLIPS.RPT` | `gl_btchd`, `gl_btch` | The slip layout selected by `{@Batch No}`. **Broken:** the code sets `Batchno` (`ap_btche.cpp:1082-1126`), so the stored "548" stays. Goes straight to the printer, and only for a saved voucher. |
| 1075 | … > Distribution List | `GL_DIST.RPT` | `glaccts`, `gl_dist` | By account+dept: every `gl_dist` row of any type and period (med rep, product, period, reference, slip no., cheque, debit, credit); group debit and credit sums, an opening balance backed out as `CURR_BAL − Σdebit + Σcredit`, closing `CURR_BAL`. **Replaced by F2** (D14). |
| 6039 | … > Medical Representatives | `GL_MEDRP.RPT` | `gl_medrp` | Code, short name, name, address 1–3, postal, phone, fax, contact, description, type, terms, credit limit. |
| 6038 | … > Product code summary | `GL_PRODL.RPT` | `gl_itemm` | Code, name, description (`FFULLN`), shop, lot size, price (`FAVPRICE`). |
| 6041 | … > Doctor Codes | `GL_DRCDS.RPT` | `gl_drcod` | Code, short name, name, address, postal, phone, fax, territory, contact, description. Titled "Medical Representatives" (copied); table path hard-coded to `c:\gl\dbf\`. |

Page headers carry the company name and title; most add "Printed on", "Current Period" and "Page No.". Most footers print the `.RPT` name.

### 2.3 Unreachable and odd commands

- **6077** (`GLAccounts`, `GLACCTS.RPT`) is bound but on no menu or toolbar. Not ported (D11).
- **1025** (Manufacturing statement) has no handler; `GLManAccounts` is bound to 2021, which no menu uses.
- `gl_distr.cpp`, a Distribution List with an account-range dialog (1025), isn't built.
- Unreferenced GL `.RPT` files: `GL_BARCH`, `GL_BATCH`, `GL_LINEI`, `GL_TEST`, `GL_DRCOD`, `SLIP`, `SLIPS`, `TEST`, `PRT_INIT`, `ONLINE`.

### 2.4 Financial statements, custom reports and printer setup

- **Financial Statements** (1015 income statement, 1016 balance sheet, 1018 retained earnings, 1025 manufacturing) open Visual Tools spreadsheets (`gl_xls.cpp:401-465`) whose cells call `GLDBF.DLL` (`CB(account, dept)` returns `CURR_BAL`; `CBR(from, to)` sums a range but never resets its total). In the deployed data `INCOME.XLS` and `MANUFACT.XLS` are empty, `BALANCE.XLS` and `FIXED.XLS` don't exist, and `FINRE.XLS` is a balance sheet for "Jul 1994". So 1016 and 1018 open an "(untitled)" sheet and 1025 does nothing. The company had no working financial statements.
- **Custom Report** (3068: File > Print, Reporting > Custom Report, GLSUB, the toolbar Print button) opens dialog 1022 over the `gl_rept` table and runs the chosen `.RPT`. Its `SetupWindow` closes every open table; Modify appends a duplicate. The deployed `gl_rept` has six rows, five AP/AR reports and one missing file.
- **Printer setup** (2001) calls the Toolbox DLL's `DlgPrinterSetup`, which changes the Windows default printer.

### 2.5 The chart of accounts and its balances

Checked against the real company's `GLACCTS.DBF` and `GL_DIST.DBF`, migrated:

- **389 accounts**, 6-digit numbers, some split by department (`410001`-10 / -20 …). `TYPE` is `B` (balance sheet, 109), `I` (income statement, 278) or `R` (retained earnings, 2: `340000` and, by mistake, `101000` Petty Cash; Spec #2 D2). **`CATEGORY` is blank on every account** and `FDESCRIPT` is empty, so nothing in the chart groups accounts into statement lines.
- **The numbering is a clean scheme**, which the default layout (§6.3) follows:

  | Range | Contents | Range | Contents |
  |---|---|---|---|
  | 10 | cash and bank | 30 | share capital |
  | 11 | trade receivables, allowance | 34 | retained earnings |
  | 12 | inventories | 35 | dividends |
  | 14 | accruals, advances, prepayments, other receivables | 36 | net income for the year |
  | 15 | property and equipment | 41 / 42 / 44 | gross sales / returns / allowances |
  | 16 | accumulated depreciation | 50 | cost of sales |
  | 18 | deposits, deferred charges, other long-term assets | 70–76 | operating expenses (selling, development, promotion, personnel, travel, admin, other) |
  | 20 | short-term borrowings | 80 / 81 | non-operating income / expenses |
  | 22 / 23 / 26 | payables / accruals / other payables | 82 / 83 | extraordinary income / expenses |
  | 29 | long-term liabilities | 91 | income tax |

- **Balances.** Amounts are signed, debit positive and credit negative; the books balance (`Σ CURR_BAL = 0`). `OPENBAL` is the balance at the start of the fiscal year, `PERIOD1`–`PERIOD12` are the net movements of each fiscal period, and **`CURR_BAL = OPENBAL + Σ PERIOD<n>`** holds for every account (checked: no exceptions). So the balance at the end of any period `p` is `OPENBAL + Σ PERIOD1..p`. Posting keeps this (Spec #2 §2.4); an adjustment changes `OPENBAL` and recomputes `CURR_BAL` the same way.
- **`PREVIOUS1`–`12`** hold the prior year's period movements, written by year end; `BUDGET1`–`12` the budget (Spec #2 Budget Data).
- **`gl_dist`** has one row per posted line (Spec #2 §2.4): account, dept, `PERIOD` (text, e.g. `" 2  "`), date, voucher number, reference, cheque, description, debit, credit, `TYPE` (blank or `GL` for GL vouchers, `AP`, `AR`). In the real data its 13 rows, all period 2, agree exactly with the accounts' `PERIOD2`. Adjustments write no `gl_dist` row.
  - *Rechecked in S3-M2:* the identity holds for all 389 accounts, every period nets to zero, and every `gl_dist` row agrees with its account's period column. `TYPE` is blank (not `GL`) on GL lines, and the voucher number is `BATCHNO`. The real lines are dated 03/02/1995 but filed in period 2 (May 94): a line's date needn't fall in its period, so reports select by `PERIOD`, never by date.
- **Fiscal periods.** `gl_pds` has 12 periods with descriptions ("Apr 94" … "Mar 95") and start and end dates; the real company's year starts in April.
- **Year end** (Spec #2, as ported) moves each period into `PREVIOUS<n>` and zeroes the periods, zeroes the income accounts into the `R` account, and leaves `OPENBAL` alone. After it, a balance-sheet account's `CURR_BAL` no longer equals `OPENBAL + Σ PERIOD`, and `gl_dist` keeps the old year's rows under the same period numbers. D16 fixes both, which the statements need.

---

## 3. Screens and port files

| Command | Original | Port file | Notes |
|---|---|---|---|
| balances and ranges | — | `gl/port/reports/Balances.{h,cpp}` | portable: balance at a period, movements over a range, the `CURR_BAL` check (§5.1) |
| 6076 F1, 1075 F2 | `ap_repts.cpp:59,61` | `gl/port/reports/TrialBalances.{h,cpp}` | portable builders |
| 1015 F3, 1016 F4, 1018 F5 | `gl_xls.cpp:432-465` | `gl/port/reports/Statements.{h,cpp}` | portable: evaluates a layout (§6) |
| statement layout tables, default, checks | — | `gl/port/reports/StatementLayout.{h,cpp}` | portable |
| Statement Layout screen | — | `gl/port/StatementLayoutDialog.{h,cpp}` | new dialog (§6.4) |
| report options dialogs | — | `gl/port/reports/ReportOptions.{h,cpp}` | new dialogs (§5.6) |
| 6080, 10071, 10073 | `ap_repts.cpp:54-63` | `gl/port/reports/AccountReports.{h,cpp}` | portable |
| 6074, 8082, 6075, 6078, 8136, Voucher Entry Print | `ap_repts.cpp:55-65`, `ap_btche.cpp:1082` | `gl/port/reports/VoucherReports.{h,cpp}` | portable |
| 6039, 6038, 6041 | `ap_repts.cpp:64-67` | `gl/port/reports/MasterReports.{h,cpp}` | portable |
| run a report | `PrintReport` | `gl/port/reports/RunReport.{h,cpp}` | Win32: context, options, build, preview. *Corrected in S3-M1:* the context is portable, in `gl/port/reports/ReportContext.{h,cpp}`, so `gl_tests` checks it |
| the engine's test report | — | `gl/port/reports/SampleReport.{h,cpp}` | *Added in S3-M1:* portable; `gl.exe /samplereport`, `/stress`; not on a menu |
| 3068 | `gl_rept.cpp` (1022) | `gl/port/reports/ReportList.{h,cpp}` | D9 |
| 2001 | `gl.cpp:207` | `report/src/win32/PrinterSetup.cpp` | D10 |

**New resources.** The options dialogs and the Statement Layout screen have no original template. They go in a hand-written `gl/res/port.rc` (with IDs in `gl/port/PortIds.h`, from a range no original resource uses), which `prune_rc.py` `#include`s into the generated `gl.rc`. The menu changes (D13, the new Accounts > Statement Layout item) are made by `prune_rc.py`, not by hand. `/checkres` covers every new dialog. New menu items get a menu-access code like their neighbours (Spec #1 user access).

`kReportingCommands` keeps only the commands that stay stubs: the fixed-asset reports and the AP and AR reports.

---

## 4. The `report` library

A new top-level project, `report/`, beside `win32fx/` and `cb4sqlite/`, so the AP and AR ports can use it. Namespace `rpt`.

```
report/
├── include/report/          Report.h  Layout.h  Format.h  TextRender.h    (portable)
├── include/report/win32/    Preview.h  Print.h  PrinterSetup.h            (Windows)
├── src/                     Layout.cpp Format.cpp TextRender.cpp    (Report.h is header-only)
├── src/win32/               GdiMeasure.cpp Preview.cpp Print.cpp PrinterSetup.cpp
└── report.vcxproj           static library
report_tests/                Catch2, portable core; also built on Linux under ASan/UBSan
```

*Corrected in S3-M1:* `include/report/win32/` also has `Gdi.h` (`GdiMeasure`, the fonts, the twips mapping and `DrawPage`, shared by preview and printing). The Windows half uses `win32fx` (the preview is an `MdiChild`), so a program that uses it links `win32fx.lib` too.

### 4.1 The model (`Report.h`)

A report is data, built by a function and then laid out; it knows nothing about the database or GDI.

- `Report`: `title`, `subtitle` lines (company, "For the period ended 31 Oct 26", "Department 10"…), `footerNote` (the report's name), `orientation`, `columns`, and a sequence of `Row`s.
- `Column`: header (one or two lines), relative width, alignment, kind (`Text`, `Amount`, `Integer`, `Date`), `blankZero`.
- `Row` kinds: `Detail`; `Heading` (statement section headings, bold, with an indent); `GroupHeader` (kept with its first detail row); `GroupFooter`, `Subtotal` and `Total` (a single rule above the amount cells, a double rule below a `Total` marked final); `Note` (a full-width line of text, e.g. a warning); `Blank`; `PageBreak`.
- Each row has an `indent` level (statements indent account detail under its line).
- `Slip` mode, for voucher slips: each group is a block with label/value pairs, its own line table and totals, starting a new page.

Totals are computed by the builders, in integer cents (as Spec #2 D12), not by the engine.

### 4.2 Layout and pagination (`Layout.h`)

`Layout(const Report&, const PageSetup&, Measure&) → std::vector<Page>`.

- `PageSetup`: paper size and margins in twips, from the printer, or A4 when there is none.
- `Measure`: text width and line height in twips per font role (`Title`, `Heading`, `Body`, `Bold`). Win32 measures with the printer's DC; tests use a fixed-pitch fake.
- Relative column widths scale to the printable width. Text that doesn't fit is clipped; amounts that don't fit print `###`. Only `Note` rows wrap (at spaces).
- Each page: header (company, title, subtitles, "Printed on <date>", "Page n of m"), the column headers, as many rows as fit, the footer note. Headings and group headers aren't left at the foot of a page; a statement's final total stays with the row before it.
  - *As built (S3-M0):* "Printed on" and "Page n of m" sit at the left and right of the company's line when they fit beside it, and on a line of their own above it when they don't. Pagination works on blocks: a `Heading`, `GroupHeader` or `Slip` row is glued to the row after it, a `GroupFooter`, `Subtotal` or `Total` to the row before it, and a block that fits on a page is never split. A `Blank` at the top of a page is dropped; a `PageBreak` on an empty page does nothing. *Changed in S3-M2:* a date that doesn't fit prints `###` like a number: cut short, "10/15/2026" read as "10/15/20".
- A `Page` is a list of draw operations in twips (`Text`, `Rule`). Preview and print draw the same pages.

### 4.3 Formatting (`Format.h`)

- Amounts: `1,234,567.89`; negatives with a leading minus on lists, in parentheses `(1,234.56)` on the statements (F3–F5), the accounting convention; blank for zero when the column asks.
- Dates as the port's screens show them (rule fixed in S3-M0 from the stored forms of `gl_btch.DATE`, `gl_btchd.DATE` and `gl_dist.DATE`); period-end dates from `gl_pds`.
  - *Settled in S3-M0:* all three are stored as `yyyymmdd` text: `gl_btch` and `gl_btchd` as dBase `D` fields, `gl_dist` in a `C(10)` field, blank-padded (`"19950302  "`). The screens' `tbDate` controls use `DS_MMDDYY` with a two-digit year (style `0x6440` on voucher entry's 3035). Reports print **`MM/DD/YYYY`**: the screens' order, with a four-digit year so a printout is never ambiguous. Statement subtitles use the long form, "October 31, 2026".

### 4.4 Report builders (GL side)

One portable function per report in `gl/port/reports/`, e.g.

```cpp
rpt::Report IncomeStatement(cb4sqlite::Code4& cb, const ReportContext& ctx, const StatementOptions& opt);
```

`ReportContext` carries the company name, the current period (D2) and the fiscal periods. Builders query through `cb4sqlite::Query` (§7), compute in cents, and build on Linux, where `gl_tests` checks them against the fixture.

### 4.5 Preview (`win32/Preview.h`)

An MDI child per report run, titled with the report's title:

- one page at a time, scaled to fit the width, zoom (fit page, fit width, 100 %);
- PgUp/PgDn, Home/End, Ctrl+P, mouse wheel;
- a button strip: **Print…**, **First / Previous / Next / Last**, "3 of 12", **Zoom**, **Options…** (reopens the report's options dialog and rebuilds), **Close**;
- File > Print (3068) and the toolbar Print button print the active preview (D9).

Previews lay out for the current printer setup and re-lay out when it changes.

### 4.6 Printing and Printer Setup

- **Print…**: `PrintDlgW` (printer, page range, copies), re-layout for the chosen printer's DC, `StartDoc` … `EndDoc` as one document named with the title; a "Printing page n of m" dialog with Cancel (`SetAbortProc`); GDI errors shown.
- **Printer Setup** (2001): `PageSetupDlgW` (printer, paper, orientation, margins), kept for the session and used by later previews and prints (D10).
- Default orientation per report: landscape for F2 with voucher lines, F3 with comparisons, 8082; portrait otherwise.

*Corrected in S3-M1:*
- Printer Setup doesn't offer orientation (`PSD_DISABLEORIENTATION`): a preview lays out in its report's orientation, so a setting there would be overridden every time. The Print dialog starts in the report's orientation and can still change it for one print.
- The printer chosen in a Print dialog also becomes the session printer (open previews lay out again for it), as Windows programs usually behave.
- On Windows 11, `PrintDlgW` shows the system print window, which runs in another process (`explorer.exe`). *Microsoft Print to PDF* asks for its file name during `StartDocW`.
- With no printer installed, layouts use A4 measured on the screen, and Printer Setup and Print say there is no printer.

---

## 5. The finance set

### 5.1 Balances (`Balances.h`)

All five reports read balances one way, from `glaccts`:

- **balance at the end of period `p`** = `OPENBAL + Σ PERIOD1..p` (`p = 0`: `OPENBAL`, the start of the year);
- **movement over periods `a..b`** = `Σ PERIODa..b`; prior year: `Σ PREVIOUSa..b`; budget: `Σ BUDGETa..b`;
- in cents, summed per account, per department or across departments.

**Consistency check.** Each report checks `CURR_BAL = OPENBAL + Σ PERIOD1..12` for the accounts it reads. Any account that fails is listed in a `Note` at the end of the report ("These accounts' balances don't agree with their period amounts: …"), so a report is never silently wrong. The check also catches a year end run before D16.

### 5.2 F1 Trial Balance (6076)

- **Options:** period (default the current period); include zero balances (default off).
- **Columns:** account, dept, description, debit, credit. The balance at the end of the period goes in debit if positive, credit if negative.
- **Order:** account, then dept. **Totals:** debit and credit; if they differ, a `Note` with the difference.
- Title "Trial Balance", subtitle "As at the end of <period description>".

### 5.3 F2 Distribution Trial Balance (1075)

- **Options:** from period and to period (default both the current period); show voucher lines (default off); include accounts with no balance and no activity (default off).
- **Columns:** account, dept, description, opening balance, debits, credits, closing balance.
  - opening = balance at the end of `from − 1`;
  - debits and credits = Σ `gl_dist.DEBIT` / `CREDIT` for the account+dept with `PERIOD` in `from..to` (every `TYPE`: GL, AP, AR);
  - closing = opening + movement over `from..to` (the period columns).
- **Reconciliation.** If debits − credits differs from the movement (lines missing from `gl_dist`, or an adjustment), the row is marked `*` and a `Note` explains the mark. In the real data there are none.
- **With voucher lines:** under each account, one indented row per `gl_dist` line — period, date, voucher no., type, reference, description, cheque, debit, credit — the content of the original Distribution List (`GL_DIST.RPT`).
  - *As built (S3-M2):* the line's date goes in the Account column and `P<n>` in Dept; the Description column holds "No. <voucher> <type>  <reference> - <description>  chq <cheque>"; debit and credit in the Debits and Credits columns. F2 turns landscape and widens Description and Account for them. Subtitles: F1 "As at October 31, 2026, the end of period 7 (Oct 26)"; F2 "For periods 1 (Apr 26) to 7 (Oct 26), ending October 31, 2026". The menu items keep their original text ("Trial Balance...", "Distribution List...").
- **Totals:** opening, debits, credits, closing. Opening and closing totals are 0 when the books balance.

### 5.4 F3 Income Statement, F4 Balance Sheet, F5 Cash Flow Statement

Each evaluates its layout (§6) over the accounts and prints one row per layout line (headings, account lines, subtotals), optionally with the accounts of each line listed beneath it.

| | F3 Income Statement | F4 Balance Sheet | F5 Cash Flow Statement |
|---|---|---|---|
| Account line value | movement over the column's periods, × sign | balance at the column's date, × sign | **− change in balance** over the column's periods, × sign |
| Columns | **Month** (period `p`), **Year to date** (`1..p`); optional **Prior year YTD** (`PREVIOUS1..p`) and **Budget YTD** (`BUDGET1..p`) with **Variance** | **End of `p`**, **Start of year** (`OPENBAL`) | **Month**, **Year to date** |
| Department | all (consolidated) or one department: only account+dept rows with that dept | all | all |
| Built-in check | — | Total assets = total liabilities and equity; else a `Note` with the difference | Opening cash + net change = closing cash; else an "Unreconciled difference" row |

Signs: amounts are stored debit-positive. A layout line's **sign** (+1 or −1) turns them into the figure the statement shows, so revenue and liabilities print positive with sign −1 (§6.3). The cash-flow value is `−Δ`: an increase in an asset (a debit) uses cash; an increase in a liability or accumulated depreciation (a credit) provides it, which is how depreciation becomes an add-back without a special rule.

**Net income** (layout kind `N`) is `− Σ movement` of every `TYPE I` account over the column's periods (consolidated, or for the chosen department). F4 shows the year to date in equity, since income accounts are only closed at year end; F5 starts operating activities with it.

Why the indirect cash flow balances: every posting is balanced, so over any periods `Σ Δ(all accounts) = 0`, hence `Δ cash = Net income + Σ (−Δ) over every other balance-sheet account`. If every non-cash `B` and `R` account is on some account line of the layout, the statement reconciles; if one isn't, the layout check (§6.5) names it and the "Unreconciled difference" row shows the amount.

### 5.5 Presentation

- Statement rows: headings in bold, account lines indented under them, subtotals with a single rule, the final total ("Net income", "Total liabilities and equity", "Cash at end of period") with a double rule.
- Zero lines are hidden unless "show zero lines" is on; headings with nothing beneath them are hidden too.
- Subtitles: company; "For the month and year to date ended <period end date>" (F3, F5) or "As at <period end date>" (F4); "Department <code> <description>" when chosen.
- Footer note: the report name and "Layout: default" or "Layout: modified <date>", so a printed statement says which layout made it.

### 5.6 Options dialogs

One small dialog per report family, opened before the preview and from the preview's **Options…**:

- **Statements** (F3–F5): period (combo of the 12 periods by description, default the current period); department (F3: "All departments" or one from `gldept`); compare with prior year, compare with budget (F3); show accounts under each line; show zero lines.
- **Trial balances** (F1, F2): period, or from/to period (F2); show voucher lines (F2); include zero balances.

The chosen options are kept for the session per report.

---

## 6. The statement layout

### 6.1 Tables

Created by the port with `cb4sqlite::createTable` (so `Data4` and `Query` both read them) the first time a statement or the layout screen opens, and filled with the default (§6.3). Backup and Restore carry them with the company database.

**`gl_fsline`** — one row per printed line:

| Field | Type | Meaning |
|---|---|---|
| `STATEMENT` | C(2) | `IS`, `BS` or `CF` |
| `LINENO` | N(5) | order within the statement; also how totals refer to a line |
| `KIND` | C(1) | `H` heading, `A` account line, `T` total, `N` net income, `O` opening cash, `E` closing cash, `S` blank line |
| `CAPTION` | C(60) | the text printed |
| `SIGN` | N(2) | +1 or −1, for `A`, `O`, `E` lines |
| `FORMULA` | C(120) | for `T`: line numbers with `+` and `−`, e.g. `50-60`, `90+100+110`; evaluated over the lines' printed amounts |
| `INDENT` | N(1) | 0–3 |
| `UPDATED` | C(16) | when the line was last changed (for the footer note) |

**`gl_fsrange`** — the accounts of an `A`, `O` or `E` line:

| Field | Type | Meaning |
|---|---|---|
| `STATEMENT`, `LINENO` | C(2), N(5) | the line |
| `ACCT_FROM`, `ACCT_TO` | C(6), C(6) | an inclusive range of account numbers, compared as text of equal length |

A line can have several ranges. Departments aren't part of a range: an account line takes every department of its accounts (F3's department option filters afterwards).

### 6.2 Evaluation

For each column, in `LINENO` order: `A` lines sum their accounts' values (§5.4) × sign; `N` takes net income; `O`/`E` sum their accounts' balances at the start/end of the column's periods × sign; `T` evaluates its formula over lines already evaluated. A formula referring to a missing or later line is a layout error (§6.5). In cents throughout.

### 6.3 The default layout

Built from the numbering scheme of §2.5, which the fixture also follows. Signs: −1 where the normal balance is a credit, so the figure prints positive.

**Income Statement**

| No. | Kind | Caption | Ranges | Sign / formula |
|---|---|---|---|---|
| 10 | H | Revenue | | |
| 20 | A | Gross sales | 410000–419999 | −1 |
| 30 | A | Sales returns | 420000–429999 | −1 |
| 40 | A | Sales allowances | 430000–499999 | −1 |
| 50 | T | Net sales | | 20+30+40 |
| 60 | A | Cost of sales | 500000–599999 | +1 |
| 70 | T | Gross profit | | 50−60 |
| 80 | H | Operating expenses | | |
| 90–150 | A | Variable selling (700000–709999); Product development (710000–719999); Promotion and advertising (720000–729999); Personnel (730000–739999); Travel and meetings (740000–749999); General and administrative (750000–759999); Other operating expenses (600000–699999, 760000–799999) | as listed | +1 |
| 160 | T | Total operating expenses | | 90+…+150 |
| 170 | T | Operating income | | 70−160 |
| 180 | A | Non-operating income | 800000–809999 | −1 |
| 190 | A | Non-operating expenses | 810000–819999 | +1 |
| 200 | A | Extraordinary items, net | 820000–899999 | −1 |
| 210 | T | Income before income tax | | 170+180−190+200 |
| 220 | A | Income tax | 900000–999999 | +1 |
| 230 | T | Net income | | 210−220 |

**Balance Sheet**

| No. | Kind | Caption | Ranges | Sign / formula |
|---|---|---|---|---|
| 10 | H | Assets | | |
| 20–50 | A | Cash and bank (100000–109999); Receivables (110000–119999); Inventories (120000–129999); Other current assets (130000–149999) | | +1 |
| 60 | T | Total current assets | | 20+30+40+50 |
| 70 | A | Property and equipment | 150000–159999 | +1 |
| 80 | A | Accumulated depreciation | 160000–169999 | +1 (prints negative) |
| 90 | A | Other assets | 170000–199999 | +1 |
| 100 | T | Total assets | | 60+70+80+90 |
| 110 | H | Liabilities | | |
| 120–150 | A | Short-term borrowings (200000–219999); Accounts payable (220000–229999); Accrued expenses (230000–259999); Other current liabilities (260000–289999) | | −1 |
| 160 | T | Total current liabilities | | 120+…+150 |
| 170 | A | Long-term liabilities | 290000–299999 | −1 |
| 180 | T | Total liabilities | | 160+170 |
| 190 | H | Equity | | |
| 200–230 | A | Share capital (300000–339999); Retained earnings (340000–349999); Dividends (350000–359999); Other equity (360000–399999) | | −1 |
| 240 | N | Net income for the year to date | | |
| 250 | T | Total equity | | 200+210+220+230+240 |
| 260 | T | Total liabilities and equity | | 180+250 |

**Cash Flow Statement**

| No. | Kind | Caption | Ranges | Sign / formula |
|---|---|---|---|---|
| 10 | H | Operating activities | | |
| 20 | N | Net income | | |
| 30 | A | Depreciation and amortisation | 160000–169999 | +1 |
| 40–90 | A | (Increase) decrease in receivables (110000–119999), inventories (120000–129999), other current assets (130000–149999); increase (decrease) in accounts payable (220000–229999), accrued expenses (230000–259999), other current liabilities (260000–289999) | | +1 |
| 100 | T | Net cash from operating activities | | 20+…+90 |
| 110 | H | Investing activities | | |
| 120, 130 | A | Property and equipment (150000–159999); Other assets (170000–199999) | | +1 |
| 140 | T | Net cash used in investing activities | | 120+130 |
| 150 | H | Financing activities | | |
| 160–190 | A | Borrowings (200000–219999); Long-term liabilities (290000–299999); Share capital (300000–339999); Retained earnings, dividends and other equity (340000–399999) | | +1 |
| 200 | T | Net cash from financing activities | | 160+…+190 |
| 210 | T | Net increase (decrease) in cash | | 100+140+200 |
| 220 | O | Cash at beginning of period | 100000–109999 | +1 |
| 230 | E | Cash at end of period | 100000–109999 | +1 |

Retained-earnings and dividend movements appear under financing; in the normal year they move only at year end, which is outside any in-year column.

### 6.4 The Statement Layout screen

Accounts > **Statement Layout…** (new command), behind the same access check as the Chart of Accounts:

- a statement picker (Income Statement, Balance Sheet, Cash Flow Statement);
- a read-only `fx::Grid` of the statement's lines: no., kind, caption, sign, formula, ranges (`410000-419999; 420000-429999`), with indentation shown;
- below it, the selected line's fields in ordinary controls (kind combo, caption, sign, indent, formula, ranges as text), with **Add**, **Insert before**, **Delete**, **Apply**;
- **Check**: runs §6.5 and lists the problems;
- **Preview**: runs the statement with the edited layout, before saving;
- **Reset to default** (with a confirmation), **Save** (in one transaction; Spec #2 D1), **Close** (asks to save unsaved changes).

Editing happens in the controls beneath the grid, as the Chart of Accounts screen does, so `fx::Grid` needs no text editing.

### 6.5 Layout checks

Run by **Check**, by **Save** (which refuses errors and warns on warnings) and, for warnings, by each statement run (as a `Note`):

- **errors:** a total's formula refers to a missing or later line; a range with `from > to` or not 6 digits; a kind without what it needs (an `A` line with no ranges, a `T` without a formula); duplicate line numbers; `CF` without exactly one `O` and one `E` line, or with different ranges on them;
- **warnings:** for `IS`, an income account (`TYPE I`) on no line, or on two; for `BS`, a `B` or `R` account on no line or on two; for `CF`, a non-cash `B` or `R` account on no account line, or a cash account (on the `O`/`E` ranges) also on an account line. An account on no line is also printed on its statement as an "Unclassified accounts" line just before the final total, so the totals stay complete.

---

## 7. `cb4sqlite` addition: `Query`

A read-only wrapper over `sqlite3_prepare_v2` on `Code4::handle()`:

```cpp
cb4sqlite::Query q(cb, "SELECT ACCOUNT, DEPT, DESCRIPT, CURR_BAL FROM glaccts "
                       "WHERE _deleted = 0 AND TYPE = ?1 ORDER BY ACCOUNT, DEPT");
q.bindText(1, "B");             // also bindInt, bindDouble, bindNull; reset() to run again
while (q.next()) { q.text(0); q.number(3); q.integer(4); q.isNull(2); }
```

*As built (S3-M0):* the bind calls are named by type (`bindText`, `bindInt`, …) because an overloaded `bind(int, …)` is ambiguous for an `int` literal. Columns count from 0 and parameters from 1, as in SQLite.

- Errors throw `cb4sqlite::Error` with the SQL and SQLite's message.
- `text()` is right-trimmed (character columns are space-padded); `number()` is 0.0 for NULL and blank, as `Field4::getDouble` reads migrated numbers.
- Read-only: `sqlite3_stmt_readonly` is checked and a writing statement throws. Writes stay with `Data4` (Spec #2 §5.2).
- Every report query filters `_deleted = 0`.

---

## 8. Deviations from the original (decision record)

| # | Decision | Why |
|---|---|---|
| D1 | **A native engine instead of Crystal.** Ported reports keep the original's title, columns, groups, totals and formulas, in one house style; positions, fonts and boxes aren't reproduced. | The Crystal runtime is 16-bit and its layout format undocumented. |
| D2 | **Company and current period as the original:** the first `gl_prof` record; the lowest-numbered open period in `gl_pds`. It is the default period of every options dialog. If no period is open, the default is period 12, and the original reports that print a current period (6080, 10073) say there is none. | The original passed the same two values, and read an uninitialised integer without an open period. |
| D3 | **Original formulas kept as written**, including 6080's "YTD" (`CURR_BAL − OPENBAL − PERIOD<n>`). | Changing a figure silently is worse than an odd label; the finance set gives the standard figures. |
| D4 | **Preposting Distribution List (6078) lists only unposted lines** (modes `"1"`, `"3"`); its closing balance is `CURR_BAL` plus them. | The original double-counted every posted voucher. |
| D5 | **Pre-Posting Report (8136)** prints the unposted vouchers in the Voucher Detail layout, each with its mode. | Its `.RPT` was never shipped; string 8136 says what it was for. |
| D6 | **Budget Report (10073)** uses the `GL_BUDGT.RPT` content. | The handler named a file that doesn't exist. |
| D7 | **Voucher Entry's Print** previews the voucher on screen (the slip layout); an unsaved voucher gets a message to save it first. | The original selected voucher 548 through a misnamed formula and skipped the preview. |
| D8 | **Voucher Detail's "Difference"** is signed (debits − credits), blank when zero. | The original hid a credit excess. |
| D9 | **Custom reports can't run.** File > Print and the toolbar Print print the active preview; without one, they open dialog 1022 listing the port's reports, whose Print runs the chosen one (Add/Mod, Delete, Modify hidden). Reporting > Custom Report opens the same list. `gl_rept` is untouched. | `gl_rept` holds Crystal files, mostly AP and AR ones. |
| D10 | **Printer Setup** is `PageSetupDlgW`, kept for the session; it doesn't change the Windows default printer. | A machine-wide change from an application is surprising. |
| D11 | **6077 isn't ported** and leaves `kReportingCommands`. | No menu or toolbar sends it. |
| D12 | **Doctor Codes (6041)** is titled "Doctor Codes"; every report reads the open company database, so hard-coded `c:\gl\dbf\` paths don't carry over. | Copy-paste errors in the `.RPT`. |
| D13 | **Financial Statements menu:** Income statement (1015) and Balance sheet (1016) run F3 and F4; 1018 becomes **Cash flow statement** (F5); Manufacturing statement (1025) leaves the menu. | The spreadsheets were missing or empty (§2.4); the finance team asked for these three statements. |
| D14 | **Trial Balance (6076) becomes F1**, a debit/credit trial balance at a period end; **Distribution List (1075) becomes F2**, whose "show voucher lines" option contains the original listing. | The finance team asked for a plain trial balance and an opening-movement-closing trial balance; the original layouts' figures (opening, current period, year, balance) are all in F2 and the Chart of Accounts Report. |
| D15 | **New tables `gl_fsline` and `gl_fsrange`**, created by the port, seeded with a default from the account numbering. | The chart has no categories (§2.5); the finance team maintains the grouping. |
| D16 | **Year end** (Spec #2 S2-M6, `YearEnd`) also, in the same transaction: sets `OPENBAL = CURR_BAL` for every account after the closing entries (so income accounts start at 0, and `CURR_BAL = OPENBAL + Σ PERIOD` holds again), and moves the year's `gl_dist` rows to `gl_distb`. Settles Spec #2 §12 Q6. *Added in S3-M2 (decided 2026-11-11):* the year's posted vouchers get a new mode `"9"` (closed year), so Transaction Inquiry, which adds the posted lines to `OPENBAL`, doesn't count them twice, and Recover can't reverse one into the new year. | Without it, after the first year end every balance-sheet figure computed from periods (§5.1) would be wrong, and F2 would mix two years' lines under the same period numbers. |
| D17 | **The original voucher reports list every voucher regardless of mode or period**, except D4 and D5; each line shows its period. The finance set takes periods (§5.6). | Faithful; period filters belong to the new reports. |

---

## 9. Development fixture

The Spec #2 fixture has balance-sheet accounts and two sales accounts with no income-statement activity. S3-M2 extends `make_dev_fixture.py`:

- **accounts** across the default layout's ranges: a cost-of-sales account, three or four operating-expense accounts (one with departments 10 and 20), a non-operating income account, income tax, property and equipment with its accumulated depreciation, a short-term borrowing, share capital, and an account outside every default range (to exercise "Unclassified");
- **opening balances** that balance, and **period movements** in periods 1–7 that balance per period, including sales, cost of sales, expenses, depreciation, a fixed-asset purchase and a loan drawdown, so each cash-flow section has figures;
- **`gl_dist` rows** matching the posted movements, except one deliberate adjustment-only movement, so F2's `*` mark has a case;
- `PREVIOUS<n>` and `BUDGET<n>` values on the income accounts, for F3's comparisons;
- a deleted voucher line (`_deleted = 1`) to test the filter.

The expected figures of each statement are worked out by hand in the S3-M2/M3 tests and recorded in the acceptance sheet.

*As built (S3-M2):* `make_dev_fixture.py`'s `FINANCE_*` tables add 14 accounts (25 in all): bank 102201, equipment 151000 and its depreciation 161000, loan 201000, share capital 301000, sales 411000, cost of sales 501000, salaries 731000 depts 10 and 20, rent 751000, depreciation 761000, interest 801000, tax 911000, and 999000 Suspense (outside every default range). Twelve balanced journals in periods 1–5 and 7 have `gl_dist` rows (voucher numbers 1–12, with no `gl_btch` header); period 6's depreciation (761000 / 161000, 600) has none, for F2's `*`; one `gl_dist` row is deleted. Income accounts carry `PREVIOUS` and `BUDGET` amounts (the same each month). The Spec #1/#2 accounts are unchanged, except that 101000's and 102101's `CURR_BAL` now include voucher 100 (250.00 / 12,750.00): before, `CURR_BAL = OPENBAL + Σ PERIOD` failed for them. Expected F1/F2 figures are in `gl_tests/test_reports_tb.cpp`.

---

## 10. Acceptance criteria — manual checks

Signed off in `2026-11-09-spec3-acceptance.md`.

1. **Preview and print.** Every report opens its options (where it has them) and a preview with company, title, subtitles, "Printed on" and "Page n of m"; paging, zoom, Options and Close work; two previews can be open. Printing to a printer and to *Microsoft Print to PDF* matches the preview, including a multi-page report and a page range; Cancel stops a print.
2. **Printer Setup** changes paper and orientation for the next preview without changing the Windows default printer.
3. **F1 Trial Balance** at the current period: debit total = credit total = the figures in the sheet; a period earlier shows the earlier balances.
4. **F2 Distribution Trial Balance**: for the current period and for 1..current, each row's opening + debits − credits = closing; the adjustment-only account is marked `*`; with voucher lines, voucher 100's lines appear under 102101 and 101000.
5. **F3 Income Statement**: month and year-to-date net income match the sheet; the prior-year and budget columns and variance; department 10 shows only department 10's rows.
6. **F4 Balance Sheet** balances, with net income for the year to date equal to F3's year to date; the start-of-year column equals the opening balances.
7. **F5 Cash Flow Statement** reconciles (opening cash + net change = closing cash = F4's cash) for the month and the year to date.
8. **Statement Layout**: move an account range from one line to another, Preview, Save; the statement changes. Remove a range: Check warns, and the statement shows "Unclassified accounts" and still balances. A bad formula can't be saved. Reset to default restores the default.
9. **Year end** (on a copy): afterwards opening balances equal the closing balances, income accounts are 0, `gl_distb` holds the year's lines, `gl_dist` is empty, and F1 and F4 at period 12 of the new year match the pre-year-end balance sheet with net income folded into retained earnings.
10. **Original reports**: 6080, 10071, 10073, 6074, 8082, 6075, 6078, 8136, the Voucher Entry Print, 6039, 6038 and 6041 each preview with the fixture's expected rows and totals (sheet), including D4, D5, D7 and D8.
11. **File > Print** prints the active preview; with none, it lists the reports and runs the chosen one.
12. **Regression and resources.** Spec #1 and #2 checks still pass; `/checkres /quiet` exits 0; `/stress /quiet` opens and closes each report's preview and the layout screen; no leaks in Debug.

---

## 11. Tests

| Layer | Required | What |
|---|---|---|
| `report_tests` (new, Catch2) | Yes | formatting (minus and parentheses); column scaling and clipping; pagination (repeated headers, headings and group headers never last on a page, final totals kept with their row, `Page n of m`, slip mode); a golden text rendering (`TextRender`) |
| `cb4sqlite_tests` | Yes | `Query`: binding, NULL and blank numbers, trimmed text, writes throw, bad SQL throws with its text |
| `gl_tests` | Yes | `Balances` (period ends, ranges, the consistency check); F1–F5 against the extended fixture's hand-worked figures, including department, comparisons, the `*` mark, "Unclassified", the balance-sheet and cash-flow checks; layout evaluation and every §6.5 check; default seeding and reset; year end D16 (opening balances, `gl_distb`, and F4 before and after); the original reports' rows and totals, D4, D5, D8, the deleted line |
| `/stress` | Yes | opens and closes every report's preview and the Statement Layout screen |
| Manual | Yes | §10, signed off |

---

## 12. Risks

| # | Risk | Mitigation |
|---|---|---|
| R1 | The default layout doesn't match how the finance team groups the chart (e.g. which 14x accounts are current). | It is data, editable in the screen (§6.4); show the three default statements on the real data to the finance team at the end of S3-M3, before the layout screen is built. |
| R2 | A statement is silently wrong because an account isn't on the layout or a balance is inconsistent. | The checks are printed on the statements themselves (§5.1, §6.5) and the balance sheet and cash flow carry their own reconciliation. |
| R3 | The house style doesn't match the finance team's expectations. | Sample pages of F1 and F3 after S3-M2 (Q2). |
| R4 | Printer DCs differ from the screen. | Lay out in twips with the printer's DC and re-lay out for the printer chosen; check against *Microsoft Print to PDF* and a real printer. |
| R5 | Changing year end (D16) touches Spec #2's tested code. | Its own tests; the change is additive inside the existing transaction; acceptance #9 on a copy. Until the first real year end it changes nothing for the real data (§2.5). |
| R6 | Large tables make F2 with voucher lines slow. | Builders stream from `Query`; time it on the real company's database in S3-M2. |
| R7 | Sort orders of the original reports were inferred from the `.RPT` (§2.2). | They follow the linking indexes; record as assumed in the sheet. |

---

## 13. Milestones (plan: `2026-11-09-owl-to-win32-sdk-port-spec3-plan.md`)

| # | Milestone | Depends on |
|---|---|---|
| S3-M0 | `report` core, `report_tests`, Linux build; `cb4sqlite::Query` | none |
| S3-M1 | Win32: measure, preview, print, Printer Setup (2001); `RunReport`, `ReportContext`; `port.rc` mechanism | M0 |
| S3-M2 | `Balances`; fixture extension; F1 and F2 with their options; year end D16; sample pages | M1 |
| S3-M3 | Layout tables and default; evaluation; F3, F4, F5 with their options; menu (D13); the statements on real data to the finance team | M2 |
| S3-M4 | Statement Layout screen and checks | M3 |
| S3-M5 | Original account reports 6080, 10071, 10073 | M1 |
| S3-M6 | Original voucher reports 6074, 8082, 6075, 6078, 8136, Voucher Entry Print | M1 |
| S3-M7 | Master listings 6039, 6038, 6041; report list (1022) and File > Print; stub cleanup; `/stress`, `/checkres` | M3–M6 |
| S3-M8 | Acceptance sheet, walkthrough and sign-off | all |

Corrections found while porting are recorded under each milestone, as Specs #1 and #2 did.

---

## 14. Still open

1. ~~**Financial statements.**~~ *Settled 2026-11-09:* the finance team wants an Income Statement, Balance Sheet, Cash Flow, a distribution trial balance and a plain trial balance (§5), with a layout kept in a table and maintained in a screen (§6). The original reports follow the finance set.
2. **House style.** Fonts (proposed: Arial 9 pt body, bold headings, 12 pt title), margins, rules. Settled from the S3-M2 sample pages.
3. **The default layout's groupings** (§6.3), in particular the 14x accounts as current assets, 18x as other assets, 20x borrowings as financing, and accumulated depreciation as an add-back. Confirmed or changed by the finance team at the end of S3-M3.
4. **6080 "by Dept."** keeps the original's lack of department subtotals and its "YTD" (D3). Keep, or add department subtotals?
5. **Department statements beyond F3.** F3 can be run for one department; should F1 or F2 take a department too?
6. **Printer setting persistence.** Session only (D10), or saved in `GL.INI`?
7. **Balance sheet against the same period last year.** F4 compares with the start of the year (`OPENBAL`). After a D16 year end, last year's balance at the end of period `p` can be reconstructed for a `B` account as `OPENBAL − Σ PREVIOUS(p+1)..12`; the `R` account also needs last year's closing transfer taken out (`Σ PREVIOUS1..12` of the income accounts), and last year's year-to-date net income is `− Σ PREVIOUS1..p` of the income accounts. It holds only if no adjustment changed `OPENBAL` since the year end. Add it as an optional column, or leave it out? Settle in S3-M3.
