# Spec #7 acceptance sign-off

This sheet records plan Milestone 6 (`gl/docs/superpowers/plans/2026-11-14-owl-to-win32-sdk-port-spec7-plan.md`, "Milestone 6"). It covers the manual checks in spec §10 (`2026-11-14-owl-to-win32-sdk-port-spec7-design.md`).

Fill in every blank (`____`) and set each **Result** to `PASS`, `FAIL` or `PASS with known gap`. A `FAIL` needs a triage line in [Gaps and triage](#gaps-and-triage): either fixed in Spec #7, or filed against a later spec.

**Which copy of `dev.db`:** #1–#6 run in order on one fresh copy (`gl\test\dev-fixture\setup.cmd Release`), each building on the one before. #7 then sets that copy aside as `dev-A.db` and makes a second fresh copy, company B; #8 and #9 run on company B. Keep an untouched copy so the walkthrough can be repeated.

**The figures** below come from `gl_tests` ("The Spec #7 acceptance walkthrough's figures" in `gl_tests/test_reportwriter.cpp`). It makes the same reports from the same definitions the designer saves and checks every figure quoted here. If a figure on screen differs, the test says which one is right.

Run the `sqlite3` queries from `x64\Release`, against `dev.db`, with the program closed or between screens.

## Run details

| Item | Value |
|---|---|
| Commit tested (`git rev-parse --short HEAD`) | ____ |
| Build machine | ____ |
| Test machine (fresh install, not the dev box) | ____ |
| Windows edition and build (`winver`) | ____ (target: Windows 11 23H2 or later, x64) |
| Display scaling | ____ (target: 100% / 96 DPI) |
| Printers installed | ____ (at least *Microsoft Print to PDF*) |
| Excel (or another program that opens CSV) | ____ |
| Tester | ____ |
| Witness | ____ |
| Date | ____ |

## Before the walkthrough

### P1. Release build and hardening

As in the Spec #3 sheet (P1): build `gl.sln` `Release|x64` with `/W4 /WX`, then confirm the hardening options from `common.props`, or check them with `dumpbin /headers x64\Release\gl.exe`.

| Check | Result | Notes |
|---|---|---|
| Release build, no warnings | ____ | |
| `/sdl`, `/guard:cf`; `/DYNAMICBASE`, `/NXCOMPAT`, `/GUARD:CF` | ____ | |

### P2. Automated checks (dev machine)

Run `gl\test\dev-fixture\setup.cmd` for each configuration first. From `cmd`, run each check with `start /wait ... /quiet` where it takes `/quiet`, then `echo %errorlevel%`. Each should print 0.

| Check | Debug | Release | Notes |
|---|---|---|---|
| `x64\<Config>\cb4sqlite_tests.exe` (77 test cases pass) | ____ | ____ | |
| `x64\<Config>\gl_tests.exe` (179 test cases pass; `[reportwriter]` 16 of them) | ____ | ____ | |
| `x64\<Config>\report_tests.exe` (19 test cases pass) | ____ | ____ | |
| `gl.exe /checkres /quiet` (now also `IDD_RW_DESIGN`, `IDD_RW_CALC`, `IDD_RW_PROMPT`) | ____ | ____ | |
| `gl.exe /stress /quiet` (now also the designer, the calculation dialog, the questions and the user reports' previews) | ____ | ____ | |
| `trivial_dialog.exe /selftest /quiet` | ____ | ____ | |
| Debug `gl.exe` under the debugger: open Custom Report, the designer, a preview and the questions, then exit. The Output window shows no "Detected memory leaks!" | ____ | n/a | |

### P3. Fresh-machine install

As in the Spec #3 sheet (P3), with the current `dev.db`. The fixture now includes `GL_RPTDEF` and the "Report Writer" row in `ACCESSM`.

| Check | Result | Notes |
|---|---|---|
| `gl.exe` starts with no Visual C++ runtime installed; login as `ADMIN` / `ADMIN` works | ____ | |
| `sqlite3 dev.db "select number, trim(name) from gl_rptdef where _deleted=0 order by number"` returns `1|Sales by product`, `2|GL lines by account`, `3|Open items by customer` | ____ | |

## The fixture

The Spec #6 company, current period 7 ("Oct 26"). Every voucher and line is in period 7.

- **Sales vouchers** (AR): 1 and 2 open (S-1001 3,000.00 net; S-1002 1,000.00 and 200.00), 3, 4 and 5 posted (S-1003 2,000.00 + VAT 200.00, product P100; S-1004 1,000.00 + 100.00, P200; S-1005 800.00, no VAT, P300), 6 cancelled (S-1006, 500.00 net). Sales accounts 410001-10 and 410001-20.
- **AR open items:** S-1003 2,200.00 owed (C001), S-1004 600.00 owed of 1,100.00 (C002), S-1005 settled.
- **GL vouchers:** 100 posted, 101 (October sales: Cr 410001-10 600.00, Cr 410001-20 550.00), 102 (out of balance: Dr 100.00, Cr 90.00) and 103 open.
- **User reports:** the three examples above. *GL lines by account* asks for the periods and has a debit − credit column.
- **Users:** `ADMIN` / `ADMIN` (level A, nothing greyed); `CLERK` / `clerk` (level B: the Maintenance menu greyed, and no Report Writer right).

Before #1, run `sqlite3 dev.db "select (select count(*) from gl_btchd where _deleted=0), (select count(*) from gl_dist where _deleted=0), (select count(*) from ar_btch where _deleted=0), (select count(*) from ap_btch where _deleted=0), (select count(*) from ar_dist where _deleted=0)"`: it returns `9|27|6|6|3`. #9 runs it again.

## The criteria (spec §10)

Log in as `ADMIN`. **Reporting > Custom Report** lists the built-in reports, then the user reports by name. In the designer, a field is added by picking it in **Fields** and clicking **Add >**; a filter by choosing its field and test, typing its value and clicking **Add filter**.

### 1. Make a report

- [ ] **Custom Report**, **Add...**. The designer opens on "Report Designer - new report", source **Sales lines**.
- [ ] Name "Sales by product (made)". Add Invoice, Customer name, Quantity, Sales (net) and VAT: Quantity, Sales and VAT start totalled ("[total]"). **Group by:** Product. Filter: State, **one of**, `Posted, Transferred`.
- [ ] **Preview:** three groups, "Product: P100  Analgesic tablets 100mg", "P200  Cough syrup 200ml", "P300  Topical ointment 30g". Their footers: quantity 1.00, 4.00, 0.00; sales 2,000.00, 1,000.00, 800.00; VAT 200.00, 100.00, 0.00. The total: 5.00, 3,800.00, 300.00.
- [ ] The sales and VAT add to the posted vouchers' invoices: `sqlite3 dev.db "select sum(amount), sum(vatamt) from ar_dist where _deleted=0"` returns `4100.0|300.0` (3,800.00 + 300.00, and 300.00).
- [ ] **Save**: the caption becomes "Report Designer - Sales by product (made)". **Close**: the list shows it, made by ADMIN. `sqlite3 dev.db "select number, trim(madeby) from gl_rptdef where trim(name)='Sales by product (made)'"` returns `4|ADMIN`.

**Result:** ____ **Notes:** ____

### 2. Ask when run

- [ ] Select "Sales by product (made)", **Modify...**. Filter: Period, **from ... to**, tick **Ask when run** (the value boxes grey), question `Periods`; **Add filter**. The list says "Period: asked when run (Periods)". **Save**, **Close**.
- [ ] `sqlite3 dev.db "select count(*) from gl_rptdef where _deleted=0"` returns `4`: Modify changed the row.
- [ ] **Print**. The questions ask "Periods:", from and to both "7  Oct 26". **OK**: the preview's subtitles are "State is one of Posted, Transferred" and "Period from 7 to 7", and the total is 5.00, 3,800.00, 300.00, as in #1.
- [ ] **Options...** on the preview: the questions again. From and to `8  Nov 26`, **OK**: "Period from 8 to 8", "No rows match the report's filters."
- [ ] Close the preview, **Print** the report again: the questions start at 8 to 8. **Cancel**: nothing runs.

**Result:** ____ **Notes:** ____

### 3. Groups and pages

- [ ] **Add...**, name "Open items, a page each", source **AR open items**. Add Invoice, Due date, Amount, Received, Owed (amounts start totalled). **Group by:** Customer, tick **new page**. Filter: State, **is not**, `Settled`. **Save**, **Close**.
- [ ] **Print**: two pages. Page 1: "Customer: C001  City General Hospital", S-1003, owed 2,200.00. Page 2: "Customer: C002  Reyes, Ana Dra.", S-1004, received 500.00, owed 600.00; the total: received 500.00, owed 2,800.00.
- [ ] **Reporting > Accounts Receivable > Open Receivables Detail**: its total owed is 2,800.00, the same.

**Result:** ____ **Notes:** ____

### 4. Filters

- [ ] **Add...**, source **GL voucher lines**. Add Voucher, Dept, Description, Credit. Filters: Account **is** `410001`; Period **from ... to** `7` and `8`. **Preview:** voucher 101's two lines, depts 10 and 20, credit total 1,150.00.
- [ ] Change the source to **Sales lines** (**Yes** to clear). Add Voucher, State, Sales (net). Filter: Sales account **contains** `410001`. **Preview:** vouchers 1, 2, 2, 3, 4, 5 (open and posted), sales 8,000.00; cancelled voucher 6 is left out.
- [ ] Filter: State **is** `Cancelled`; **Preview:** voucher 6 only, 500.00. **Close**, **No** (not saved).

**Result:** ____ **Notes:** ____

### 5. Calculated columns

- [ ] **Print** *GL lines by account*, periods 7 to 7: the total debit is 1,575.00, credit 1,565.00, and Debit - Credit 10.00 (voucher 102 is out of balance). Each account's footer reads "Total" and the whole account number ("Total 101000").
- [ ] Select "Sales by product (made)", **Modify...**, **Calculation...**: **Percent of the report total**, of **Sales (net)**; **OK**. The column list shows "Sales (net) % of total  [total]". **Preview** (periods 7 to 7): the product footers 52.63, 26.32, 21.05; the total 100.00. **Save**, **Close**.

**Result:** ____ **Notes:** ____

### 6. Manage

- [ ] Select "Sales by product (made)", **Copy...**: the designer opens on "Copy of Sales by product (made)". **Save**, **Close**. `sqlite3 dev.db "select count(*) from gl_rptdef where _deleted=0"` returns `6`.
- [ ] Select the copy, **Delete**: "Delete the report “Copy of Sales by product (made)”? Only the report is deleted: the books aren't touched. It can't be undone." **Yes**: gone; the query returns `5`.
- [ ] Select **Trial Balance**: **Modify...** says "“Trial Balance” is built into the program: it can't be changed. Add makes a report of your own."; **Copy...** "... it can't be copied ..."; **Delete** "... it can't be deleted."

**Result:** ____ **Notes:** ____

### 7. Export and import between two copies of the company

- [ ] Select "Sales by product (made)", **Export...**, save as `sales-made.gldef` on the desktop. Open it in Notepad: it starts `writer 1`, `name "Sales by product (made)"`, `source sales_lines 1`.
- [ ] **Print** it, periods 7 to 7, and on the preview **Export...** as `company-a.csv`.
- [ ] Close the program. Rename `x64\Release\dev.db` to `dev-A.db`, then run `gl\test\dev-fixture\setup.cmd Release` again: `dev.db` is now company B, fresh.
- [ ] Log in as `ADMIN`, **Custom Report**, **Import...** `sales-made.gldef`: "Imported as “Sales by product (made)”." **Import...** it again: "Imported as “Sales by product (made) (2)”. Another report had the file's name, so this one is numbered."
- [ ] **Print** "Sales by product (made)", periods 7 to 7: the same figures as #5. **Export...** as `company-b.csv`: the file is the same as `company-a.csv` (`fc company-a.csv company-b.csv`: "no differences encountered").
- [ ] Open `company-b.csv` in Excel: a header row (Invoice, Customer name, Quantity, Sales (net), VAT, Sales (net) % of total), the group and footer rows, and the total 5.00, 3800.00, 300.00, 100.00; "Reyes, Ana Dra." in one cell.
- [ ] **Import...** a file holding just `hello`: "The report wasn't imported. The file isn't a report the program can read (line 1): The first line must be "writer 1"."

**Result:** ____ **Notes:** ____

### 8. Access

- [ ] Log out (close the program) and log in as `CLERK` / `clerk`. **Custom Report**: **Add...**, **Modify...**, **Copy...**, **Delete** and **Import...** are greyed; **Print** and **Export...** aren't.
- [ ] **Print** "Sales by product (made)": it runs.
- [ ] Log in as `ADMIN` again: all eight buttons are available. **Maintenance > User Access**, the `CLERK` user (level B), **Menu**: the last row is "Report Writer (design reports)", with the minus sign (off). **Cancel**.

**Result:** ____ **Notes:** ____

### 9. Safety and limits

- [ ] Write a file `old.gldef` holding these lines, and **Import...** it:

  ```
  writer 1
  name "Old report"
  source sales_lines 1
  column invoice
  column colour
  ```

  **Modify...** "Old report": "These fields aren't in the source any more; they are marked "(missing)" ..." with `colour`; the column list shows "colour (missing)". **Close**. **Print** it: the invoices only, with a note naming `colour`.
- [ ] The row limit: the fixture is far smaller than 20,000 rows, so it can't be reached here (G3).
- [ ] Nothing but the report writer's own rows changed: `sqlite3 dev.db "select (select count(*) from gl_btchd where _deleted=0), (select count(*) from gl_dist where _deleted=0), (select count(*) from ar_btch where _deleted=0), (select count(*) from ap_btch where _deleted=0), (select count(*) from ar_dist where _deleted=0)"` still returns `9|27|6|6|3`.

**Result:** ____ **Notes:** ____

### 10. Regression and resources

- [ ] The Spec #1–#6 sheets' criteria still pass, on a fresh copy of `dev.db`. (The fixture's `ACCESSM` now follows the menu, with the Report Writer row last; levels A and B keep their menus.)
- [ ] The built-in reports are unchanged: **File > Print** lists the 55 built-in reports first, then the user reports; each built-in report runs from there.
- [ ] `/checkres /quiet` and `/stress /quiet` exit 0 (P2).
- [ ] No leaks in Debug (P2).

**Result:** ____ **Notes:** ____

## Known gaps accepted before the walkthrough

| # | Gap | Why accepted | Follow-up |
|---|---|---|---|
| G1 | Spec §10 #4 asks for GL lines with "cancelled vouchers left out, then included". The General Ledger's Cancel deletes an open voucher (Spec #2, `CancelVoucher`), so GL lines have no cancelled rows; #4 shows it on Sales lines, where cancelled vouchers stay. | Spec #2's behaviour, not the writer's. | None, unless GL cancellation changes. |
| G2 | Every fixture voucher is in period 7, so the second period asked (#2) gives the "No rows" note rather than other figures. | Fixture data. | Run #2 on the deployed company's copy, two real periods. |
| G3 | The 20,000-row stop can't be reached on the fixture. `gl_tests` checks its message with a small limit. | The fixture is small by design. | Try a whole-year GL lines report on the deployed company's copy. |
| G4 | An account or dept question is a text box, not the account list (spec §6.3). | The answer is checked against the field; a list is an addition. | If the finance team wants it. |
| G5 | Accounts give last year and the budget as year totals; Purchase lines have no VAT share column. | See the plan's Milestone 4 note. | If a report needs them. |
| G6 | A long calculated column header is cut at its width; typing a label or a width fixes it. | The designer shows the label it will print. | |
| G7 | The port doesn't read or write Crystal `.RPT` files: user reports are its own definitions (spec §1). | Decided in the spec. | |
| G8 | High DPI and real printers aren't tested; the Linux ASan/UBSan build hasn't been run for Spec #7. | As in the earlier sheets. | |
| G9 | **Nothing has run against the deployed company.** Opening Custom Report or the designer there creates `GL_RPTDEF` and adds the `ACCESSM` row, and every existing level gets the right (its code doesn't reach the new row). | The plan forbids it before go-live is agreed. | A go-live step: back up, open Custom Report, then turn the right off for the levels that shouldn't design. |

## Gaps and triage

Every `FAIL` above gets a line here.

| # | Criterion | What happened | Decision (fix in Spec #7 / file for Spec #N) | Fixed in commit / issue |
|---|---|---|---|---|
| T1 | 5 (found while preparing the sheet) | A group footer's label under a narrow first column was cut: "Total 10100" for account 101000. | Fixed in Spec #7 (S7-M6): a footer's or total's label runs on over the empty columns to its right (`report/src/Layout.cpp`). | S7-M6 |
| | | | | |

## Sign-off

| Criterion | Result |
|---|---|
| 1. Make a report | ____ |
| 2. Ask when run | ____ |
| 3. Groups and pages | ____ |
| 4. Filters | ____ |
| 5. Calculated columns | ____ |
| 6. Manage | ____ |
| 7. Export and import | ____ |
| 8. Access | ____ |
| 9. Safety and limits | ____ |
| 10. Regression and resources | ____ |

All criteria pass, or pass with the known gaps recorded above:

- Tester: ____ Date: ____
- Witness: ____ Date: ____
