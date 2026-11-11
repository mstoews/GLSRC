# Spec #3 acceptance sign-off

The record of plan Milestone 8 (`gl/docs/superpowers/plans/2026-11-09-owl-to-win32-sdk-port-spec3-plan.md`, "Milestone 8"). It covers the 12 manual checks in spec §10 (`2026-11-09-owl-to-win32-sdk-port-spec3-design.md`).

Fill in every blank (`____`) and set each **Result** to `PASS`, `FAIL`, `PASS with known gap` or `BLOCKED`. A `FAIL` needs a triage line in [Gaps and triage](#gaps-and-triage): either fixed in Spec #3, or filed against a later spec.

**Not built yet:** #8 needs the Statement Layout screen (S3-M4, waiting for the finance team's review). Mark it `BLOCKED` until that lands, then walk it through. Everything else can be signed off now.

The reports only read the books, except the year end in #9 and the layout edits in #8. Run #1–#7 and #10–#12 on one copy of `dev.db`, run #8 and #9 each on a fresh copy, and keep an untouched copy so the walkthrough can be repeated.

## Run details

| Item | Value |
|---|---|
| Commit tested (`git rev-parse --short HEAD`) | ____ |
| Build machine | ____ |
| Test machine (fresh install, not the dev box) | ____ |
| Windows edition and build (`winver`) | ____ (target: Windows 11 23H2 or later, x64) |
| Display scaling | ____ (target: 100% / 96 DPI) |
| Printers installed | ____ (at least *Microsoft Print to PDF*; a real printer if available) |
| Tester | ____ |
| Witness | ____ |
| Date | ____ |

## Before the walkthrough

### P1. Release build and hardening (plan M8 task 2)

1. Build `gl.sln` `Release|x64` with `/W4 /WX`. It should build with no warnings.
2. Confirm the hardening options from `common.props`, as in the Spec #1 and #2 sheets: compiler `/sdl`, `/guard:cf`; linker `/DYNAMICBASE`, `/NXCOMPAT`, `/GUARD:CF`. Alternatively, run `dumpbin /headers x64\Release\gl.exe`: its "DLL characteristics" list "Dynamic base", "NX compatible" and "Control Flow Guard".

| Check | Result | Notes |
|---|---|---|
| Release build, no warnings | ____ | |
| `/sdl` and `/guard:cf` (compiler) | ____ | |
| `/DYNAMICBASE`, `/NXCOMPAT`, `/GUARD:CF` (linker) | ____ | |

### P2. Automated checks (dev machine)

Run `gl\test\dev-fixture\setup.cmd` for each configuration first (`Debug`, then `Release`). From `cmd`, run each check with `start /wait ... /quiet` where it takes `/quiet`, then `echo %errorlevel%`. Each should print 0.

| Check | Debug | Release | Notes |
|---|---|---|---|
| `x64\<Config>\cb4sqlite_tests.exe` (77 test cases pass) | ____ | ____ | |
| `x64\<Config>\gl_tests.exe` (93 test cases pass) | ____ | ____ | |
| `x64\<Config>\report_tests.exe` (17 test cases pass) | ____ | ____ | |
| `gl.exe /checkres /quiet` (now also the port's dialogs in `port.rc`, 20000–20002, the report list 1022, and Custom Report's own command) | ____ | ____ | |
| `gl.exe /stress /quiet` (now also every report's preview, paged at each zoom) | ____ | ____ | |
| `trivial_dialog.exe /selftest /quiet` (covers GDI measuring, drawing a page, and a preview) | ____ | ____ | |
| Debug `gl.exe` under the debugger: open and close three reports' previews, then exit. The Output window shows no "Detected memory leaks!" | ____ | n/a | |

### P3. Fresh-machine install

As in the Spec #2 sheet (P3): a folder with `gl.exe` and `dbf2sqlite.exe` from `x64\Release\`, `GL.INI` from `gl\test\dev-fixture\`, a `GLDR.INI` with `[GL_DIR]` `DIR=dev.db`, and `dev.db` migrated with `dbf2sqlite.exe` or copied from the dev machine. Use a `sqlite3.exe` (the command-line shell from sqlite.org) for the external checks. Character values are padded with spaces, so the queries use `trim()` or `like`.

| Check | Result | Notes |
|---|---|---|
| `gl.exe` starts with no Visual C++ runtime installed (static CRT) | ____ | |
| Login as `ADMIN` / `ADMIN` works | ____ | |
| `sqlite3 dev.db "select round(sum(curr_bal),2) from glaccts"` returns 0.0 (the books balance) | ____ | |

## The fixture

Its company is "Dev Fixture Company Ltd." The fiscal year runs April 2026 to March 2027. Periods 1–6 are closed; the current period is 7, "Oct 26", ending October 31, 2026.

- **Accounts:** 25. Besides the Spec #1/#2 accounts, S3-M2 added:
  - a bank account, 102201;
  - equipment 151000 and its accumulated depreciation 161000;
  - a loan, 201000, and share capital, 301000;
  - sales 411000, cost of sales 501000, salaries 731000 (depts 10 and 20), rent 751000, depreciation 761000, interest 801000 and tax 911000;
  - 999000 Suspense, which no default statement line covers.

  The make-up is in `make_dev_fixture.py`'s `FINANCE_*` tables.
- **Activity:** twelve posted journals in periods 1–5 and 7, with `gl_dist` lines (voucher numbers 1–12), plus voucher 100 (posted, period 7). Period 6 has a 600.00 depreciation movement (761000 / 161000) with **no** `gl_dist` lines, for F2's `*` mark. One `gl_dist` line is deleted.
- **Vouchers in `gl_btch`:** 100 is posted; 101 is open (111101 Dr 1,150 / 410001-10 Cr 600 / -20 Cr 550); 102 is open and out of balance (148000 Dr 100 / 260007 Cr 90); 103 is an open adjustment (144910 Dr 75 / 111101 Cr 75).
- **Prior-year and budget amounts** on the income accounts are the same each month. For example, sales have 9,000 in the prior year and a budget of 10,000.

Every figure below comes from `gl_tests` (`test_reports_tb.cpp`, `test_statements.cpp`, `test_reports_accounts.cpp`, `test_reports_vouchers.cpp`) and was checked against these queries on a fresh `dev.db`.

## The 12 criteria (spec §10)

Run these on the test machine, logged in as `ADMIN`. Every report opens with its options dialog (if it has one), then a preview.

### 1. Preview and print

- [ ] Open **Reporting > General Ledger > Trial Balance** (OK in its options). The preview shows "Dev Fixture Company Ltd.", "Trial Balance", "As at October 31, 2026, the end of period 7 (Oct 26)", "Printed on" with today's date, and "Page 1 of 1".
- [ ] Open **Reporting > General Ledger > Voucher Slips**: "1 of 4". **>**, **>|**, **<**, **|<** and PgDn, PgUp, Home, End move between the slips, and the arrows grey out at the ends. **Zoom** cycles Fit width → Fit page → 100 %, and the mouse wheel scrolls (or turns pages when the page fits).
- [ ] Two previews are open at once (Window menu lists both), and **Close** closes one.
- [ ] **Options...** on the Trial Balance reopens its dialog. Choose period 6: the preview now reads "the end of period 6 (Sep 26)".
- [ ] **Print...** (and Ctrl+P) on the Voucher Slips preview: the Windows print window opens on the session printer. Choose *Microsoft Print to PDF*, pages 2–3, and save. The PDF has 2 pages, slips 101 and 102, matching the preview.
- [ ] Print all four slips to PDF: 4 pages. Print again and press **Cancel** in "Printing page n of m" (with a real printer, or a long report): printing stops and nothing more is printed.
- [ ] If a real printer is installed: print the Trial Balance to it. The page matches the preview (see G3).

**Result:** ____ **Notes:** ____

### 2. Printer Setup

- [ ] **File > Printer Setup**: the Page Setup dialog shows the printer, paper and margins, and has no orientation choice (spec §4.6). Note the Windows default printer (Settings > Printers & scanners).
- [ ] Choose a different paper (e.g. Letter instead of A4) and 1-inch margins, OK. Open a new Trial Balance preview: the page shape and margins change.
- [ ] Open **Distribution List** with **Show voucher lines** ticked: it is landscape, whatever Printer Setup says.
- [ ] The Windows default printer hasn't changed (D10). Restart `gl.exe`: the paper and margins are back to their defaults (A4 / 0.75 inch, or the printer's default paper).

**Result:** ____ **Notes:** ____

### 3. F1 Trial Balance

- [ ] **Trial Balance**, period 7 (the default): 22 accounts. Debit and credit totals are both **96,160.00**. 102201 shows 24,950.00 in Debit, 201000 shows 8,000.00 in Credit, and 144910 (zero) isn't listed.
- [ ] External check: `sqlite3 dev.db "select round(sum(b),2) from (select openbal+coalesce(period1,0)+coalesce(period2,0)+coalesce(period3,0)+coalesce(period4,0)+coalesce(period5,0)+coalesce(period6,0)+coalesce(period7,0) b from glaccts) where b>0"` returns 96160.0.
- [ ] Period 6: 21 accounts, totals **90,160.00**; 102101 shows 12,500.00, before voucher 100.
- [ ] Period 7 with **Include accounts with a zero balance**: 25 accounts, 144910 with both columns blank.

**Result:** ____ **Notes:** ____

### 4. F2 Distribution Trial Balance

- [ ] **Distribution List**, from and to period 7: debits and credits are both **11,550.00**, and the opening and closing totals are 0.00. Each row reads opening + debits − credits = closing (e.g. 102101: 12,500.00 + 250.00 − 0.00 = 12,750.00). Nothing is marked.
- [ ] Periods 1 to 7: debits and credits are both **43,400.00**. **161000** and **761000** are marked `*`, and the note says 2 accounts are marked. 102201 reads 20,000.00 / 24,050.00 / 19,100.00 / 24,950.00. 751000's debits are 1,200.00: the deleted line isn't counted.
- [ ] External check: `sqlite3 dev.db "select round(sum(debit),2), round(sum(credit),2) from gl_dist where _deleted=0 and cast(trim(period) as integer) between 1 and 7"` returns `43400.0|43400.0`.
- [ ] Periods 1 to 7 with **Show voucher lines**: landscape. Under 101000 and 102101 are voucher 100's lines: "10/15/2026", "P7", "No. 100 GL  Cash to checking - ...  chq 000123". Dates print in full (e.g. 10/15/2026, never 10/15/20).

**Result:** ____ **Notes:** ____

### 5. F3 Income Statement

- [ ] **Reporting > Financial Statements > Income statement**, period 7, all departments. The figures are Month / Year to date:
  - gross sales 6,000.00 / 16,000.00;
  - cost of sales 2,400.00 / 6,400.00;
  - gross profit 3,600.00 / 9,600.00;
  - total operating expenses 2,000.00 / 7,300.00;
  - operating income 1,600.00 / 2,300.00;
  - income tax 900.00 / 900.00;
  - **net income 700.00 / 1,450.00**, double-ruled.

  Negatives print in parentheses. Lines with nothing ("Sales returns") aren't shown. The footer reads "Income Statement - Layout: default".
- [ ] With **Compare with the prior year** and **Compare with the budget**: landscape, three more columns. Net income is **(1,120.00)** for the prior year to date, **(350.00)** for the budget to date, and the variance is **1,800.00**.
- [ ] Department **10 Research & Development**: a second subtitle "Department 10 Research & Development". Personnel is 2,000.00 / 4,000.00, net income **(2,000.00) / (4,000.00)**, and there's no sales line.
- [ ] External check: `sqlite3 dev.db "select round(-sum(coalesce(period7,0)),2) from glaccts where type='I'"` returns 700.0.

**Result:** ____ **Notes:** ____

### 6. F4 Balance Sheet

- [ ] **Balance sheet**, period 7. The columns are End of Oct 26 / Start of year:
  - cash and bank 37,950.00 / 33,000.00;
  - total current assets 46,460.00 / 41,510.00;
  - accumulated depreciation (6,600.00) / (6,000.00);
  - **total assets 74,860.00 / 65,510.00**;
  - total liabilities 12,670.00 / 4,670.00;
  - net income for the year to date 1,450.00 / 0.00, the same as F3's year to date;
  - total equity 62,290.00 / 60,840.00;
  - "Unclassified accounts" (100.00) / 0.00;
  - **total liabilities and equity 74,860.00 / 65,510.00**.
- [ ] The start-of-year column equals the opening balances: `sqlite3 dev.db "select round(sum(openbal),2) from glaccts where account like '10%'"` returns 33000.0 (cash). A note names 999000 as on no line, and there's no "doesn't balance" note.

**Result:** ____ **Notes:** ____

### 7. F5 Cash Flow Statement

- [ ] **Cash flow statement**, period 7. The columns are Month / Year to date:
  - net income 700.00 / 1,450.00;
  - depreciation and amortisation 0.00 / 600.00;
  - net cash from operating activities 700.00 / 2,050.00;
  - property and equipment 0.00 / (5,000.00);
  - borrowings 0.00 / 8,000.00;
  - "Unclassified accounts" 0.00 / (100.00);
  - net increase in cash 700.00 / 4,950.00;
  - cash at beginning of period 37,250.00 / 33,000.00;
  - **cash at end of period 37,950.00 / 37,950.00**, F4's cash and bank.
- [ ] There is no "Unreconciled difference" row. Period 1: cash at the beginning is 33,000.00, and the investing heading isn't shown.

**Result:** ____ **Notes:** ____

### 8. Statement Layout — **BLOCKED until S3-M4**

- [ ] **Accounts > Statement Layout...**: move an account range from one line to another, Preview, Save; the statement changes.
- [ ] Remove a range: Check warns, and the statement shows "Unclassified accounts" and still balances.
- [ ] A formula referring to a later or missing line can't be saved.
- [ ] Reset to default restores the default; the footer reads "Layout: default" again.

(The layout's evaluation, checks, save and reset are already covered by `gl_tests`, `test_statements.cpp`.)

**Result:** ____ **Notes:** ____

### 9. Year end (on a fresh copy of `dev.db`)

- [ ] Before: note F1 at period 12 (debits = credits = **96,160.00**) and F4 at period 12 (total assets **74,860.00**).
- [ ] **Maintenance > Year End Consolidation**: the message reports Retained Earnings 340000's new balance (−18,290.00), **27** posted lines archived and **1** voucher closed.
- [ ] `sqlite3 dev.db "select count(*) from gl_dist"` returns 0, and `select count(*) from gl_distb` returns 27. `select trim(mode) from gl_btch where batchno=100` returns 9. In `glaccts`, every income account has `curr_bal` 0, and every account's `openbal` equals its `curr_bal`.
- [ ] F1 at period 12: 14 accounts, debits = credits = **81,560.00**; 340000 is 18,290.00 in Credit; no income accounts.
- [ ] F4 at period 12: total assets **74,860.00** = total liabilities and equity. Retained earnings is 18,290.00 and net income for the year to date is 0.00, so the year's 1,450.00 is folded into retained earnings.
- [ ] **Transactions > Voucher Reports > General Ledger Voucher Inquiry** for 102101: opening 12,750.00, no lines, balance 12,750.00 (voucher 100 isn't counted twice).

**Result:** ____ **Notes:** ____

### 10. Original reports

- [ ] **Reporting > General Ledger > Trial Balance by Dept.**:
  - 25 accounts, landscape, "Current period 7 (Oct 26)";
  - 102201 reads 20,000.00 / 700.00 / **4,250.00** ("YTD" as the original: current less opening less the period, D3) / 24,950.00;
  - every total is 0.00;
  - the footer reads "Report Name : gldept".
- [ ] **Accounts > Chart of Accounts Report** (also the toolbar's inquiry button): 102201's "Accumulated This Year" is 4,950.00, and 161000's is (−)600.00.
- [ ] **Accounts > Budget Report**: 411000 reads −16,000.00 / −6,000.00 / −10,000.00 / **4,000.00**, and the "Chart of Accounts Total" budget is 50.00.
- [ ] **Voucher Slips**: 4 slips, 100–103, each with its status.
- [ ] **Voucher Detail**: 4 groups; only 102 shows "Difference (debits less credits)" **10.00**; totals 1,575.00 / 1,565.00. 101's lines show DR001 / MR001 / P100 and DR002 / MR002 / P200.
- [ ] **Voucher Summary**: 4 lines; totals 1,575.00 / 1,565.00.
- [ ] **Preposting Distribution List**: 6 accounts (no 101000 or 102101: voucher 100 is posted, D4). 111101's balance after posting is **9,275.00**, 148000's 410.00 and 260007's −540.00. The total row reads "7 lines".
- [ ] **Transactions > Voucher Reports > General Ledger Pre-Posting Report**: vouchers 101, 102 and 103 only, each with "Open" or "Adjustment" (D5).
- [ ] **Voucher Entry's Print**:
  - on the new voucher 104: "Voucher 104 isn't saved yet...";
  - after **Top** (voucher 101): a "Preview" window shows its slip over Voucher Entry, which can't be used until the preview closes (D7).
- [ ] **Medical Representatives**: landscape, "Medical Representative Summary Report", MR001–MR003 in code order, each with "1 Clinic Road" and "Vancouver BC" under it, terms 30; "3 medical representatives"; the footer reads "Report File: gl_medrp.rpt".
- [ ] **Product code summary**: P100, P200, P300 with lot sizes 10, 24, 12 and prices 4.25, 6.50, 3.10 (`FAVPRICE`); "3 products".
- [ ] **Doctor Codes**: titled "Doctor Codes Summary Report" (D12), DR001–DR003 with territories NORTH, SOUTH, EAST; "3 doctors".

**Result:** ____ **Notes:** ____

### 11. File > Print

- [ ] With a preview active, **File > Print** and the toolbar's Print button print that preview (D9).
- [ ] Close every preview. **File > Print** opens "Print Custom Report" (dialog 1022) listing 16 reports, with only Print and Close. Choose **14 Medical Representatives** and press Print: its preview opens. Double-click **1 Trial Balance**: its options dialog, then its preview.
- [ ] With a preview open, **Reporting > Custom Report** (and right-click on the background > Custom Report) still opens the list. Close runs nothing.
- [ ] The list doesn't use the original's `gl_rept` table, which the fixture doesn't have. On a migrated real company, `sqlite3 <company>.db "select count(*) from gl_rept"` returns the same before and after using the list.
- [ ] **Fixed Assets > Fixed assets listing**: the message says it arrives with the Fixed Assets port (Spec #4).

**Result:** ____ **Notes:** ____

### 12. Regression and resources

- [ ] The Spec #1 and Spec #2 sheets' criteria still pass, on a fresh copy of `dev.db`. Their notes say where the S3-M2 fixture changed a figure.
- [ ] `/checkres /quiet` and `/stress /quiet` exit 0 (P2).
- [ ] No leaks in Debug (P2).

**Result:** ____ **Notes:** ____

## Known gaps accepted before the walkthrough

| # | Gap | Why accepted | Follow-up |
|---|---|---|---|
| G1 | High DPI is not tested. The walkthrough runs at 96 DPI. | As Spec #1 G1. | The AP/AR specs (#5/#6). |
| G2 | The default statement groupings and the prior-year balance-sheet column aren't confirmed by the finance team. | Spec §14 Q3 and Q7. The statements were sent for review in S3-M3. | Settle before S3-M4; change the default if asked. |
| G3 | Only *Microsoft Print to PDF* was used in development. | No real printer on the build machine. | Check #1's real-printer step on the test machine. |
| G4 | On Windows 11 the Print dialog is the system print window, run by `explorer.exe`. *Microsoft Print to PDF* asks for the file name in its own Save dialog. | Windows behaviour; `PrintDlgW` is used as the spec says. | None. |
| G5 | The Linux ASan/UBSan build of the Spec #3 sources hasn't been run since S3-M1. | No Linux machine in this round. | Run `tools/linux/build_tests.sh --run`. |
| G6 | #8 is blocked. | S3-M4 waits for the finance review. | Walk it through when S3-M4 lands. |

## Gaps and triage

Add one row for each `FAIL` or unexpected behaviour found during the walkthrough.

| # | Criterion | What happened | Decision (fix in Spec #3 / file for Spec #N) | Fixed in commit / issue |
|---|---|---|---|---|
| | | | | |

## Sign-off

| Criterion | Result |
|---|---|
| 1. Preview and print | ____ |
| 2. Printer Setup | ____ |
| 3. F1 Trial Balance | ____ |
| 4. F2 Distribution Trial Balance | ____ |
| 5. F3 Income Statement | ____ |
| 6. F4 Balance Sheet | ____ |
| 7. F5 Cash Flow Statement | ____ |
| 8. Statement Layout | ____ (BLOCKED until S3-M4) |
| 9. Year end | ____ |
| 10. Original reports | ____ |
| 11. File > Print | ____ |
| 12. Regression and resources | ____ |

All criteria pass, or pass with the known gaps recorded above. #8 is signed off separately once S3-M4 lands:

- Tester: ____ Date: ____
- Witness: ____ Date: ____
