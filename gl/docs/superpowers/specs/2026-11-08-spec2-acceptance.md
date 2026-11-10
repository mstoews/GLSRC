# Spec #2 acceptance sign-off

The record of plan Milestone 7 (`gl/docs/superpowers/plans/2026-11-08-owl-to-win32-sdk-port-spec2-plan.md`, "Milestone 7"). It covers the 14 manual checks in spec §8 (`2026-11-08-owl-to-win32-sdk-port-spec2-design.md`).

Fill in every blank (`____`) and set each **Result** to `PASS`, `FAIL` or `PASS with known gap`. A `FAIL` needs a triage line in [Gaps and triage](#gaps-and-triage): either fixed in Spec #2, or filed against a later spec.

The criteria build on each other: run them in order on one copy of `dev.db`, and keep an untouched copy so the walkthrough can be repeated. Each check says what state it expects.

## Run details

| Item | Value |
|---|---|
| Commit tested (`git rev-parse --short HEAD`) | ____ |
| Build machine | ____ |
| Test machine (fresh install, not the dev box) | ____ |
| Windows edition and build (`winver`) | ____ (target: Windows 11 23H2 x64; Windows 10 22H2 x64 if 11 is unavailable) |
| Display scaling | ____ (target: 100% / 96 DPI) |
| Tester | ____ |
| Witness | ____ |
| Date | ____ |

## Before the walkthrough

### P1. Release build and hardening (plan M7 task 1)

1. Build `gl.sln` `Release|x64` with `/W4 /WX`. It should build with no warnings.
2. Confirm the hardening options from `common.props`, as in the Spec #1 sheet (P1): compiler `/sdl`, `/guard:cf`; linker `/DYNAMICBASE`, `/NXCOMPAT`, `/GUARD:CF`. Or run `dumpbin /headers x64\Release\gl.exe`: "DLL characteristics" lists "Dynamic base", "NX compatible" and "Control Flow Guard".

| Check | Result | Notes |
|---|---|---|
| Release build, no warnings | ____ | |
| `/sdl` and `/guard:cf` (compiler) | ____ | |
| `/DYNAMICBASE`, `/NXCOMPAT`, `/GUARD:CF` (linker) | ____ | |

### P2. Automated checks (dev machine)

Run `gl\test\dev-fixture\setup.cmd` for each configuration first (`Debug`, then `Release`). From `cmd`, run each check with `start /wait ... /quiet` where it takes `/quiet`, then `echo %errorlevel%`. Each one should print 0.

| Check | Debug | Release | Notes |
|---|---|---|---|
| `x64\<Config>\cb4sqlite_tests.exe` (69 test cases pass) | ____ | ____ | |
| `x64\<Config>\gl_tests.exe` (41 test cases pass) | ____ | ____ | |
| `gl.exe /checkres /quiet` (resource failures; now includes 1006, 1007, 1013, 1015, 1017, 1023, 1029, 1040, 1043, 1061) | ____ | ____ | |
| `gl.exe /stress /quiet` (GDI + USER growth; now also the master data, voucher, posting and inquiry screens) | ____ | ____ | |
| `trivial_dialog.exe /selftest /quiet` (failures; covers `fx::Grid` editing) | ____ | ____ | |
| Debug `gl.exe` under the debugger: open and close Voucher Entry and Post, then exit. The Output window shows no "Detected memory leaks!" | ____ | n/a | |

### P3. Fresh-machine install

As in the Spec #1 sheet (P3): a folder with `gl.exe` and `dbf2sqlite.exe` from `x64\Release\`, `GL.INI` from `gl\test\dev-fixture\`, a `GLDR.INI` with `[GL_DIR]` `DIR=dev.db`, and `dev.db` migrated with `dbf2sqlite.exe` or copied from the dev machine. Use a `sqlite3.exe` (the command-line shell from sqlite.org) for the external checks.

The database's columns are named as in the `.dbf` files (`ACCOUNT`, `CURR_BAL`, ...; SQLite ignores their case). Character values are padded with spaces, so the queries use `trim()` or `like`.

| Check | Result | Notes |
|---|---|---|
| `gl.exe` starts with no Visual C++ runtime installed (static CRT) | ____ | |
| `dbf2sqlite.exe` migration exits 0 (if used) | ____ | |
| Login as `ADMIN` / `ADMIN` works | ____ | |
| `sqlite3 dev.db "select sum(curr_bal) from glaccts"` returns 0 (the fixture's books balance) | ____ | |

## The fixture

- **Accounts:** 11, as in the Spec #1 sheet. 340000 is the only type-R account. 410001 has depts 10 and 20, both income (type I).
- **Periods:** 1–6 (Apr 26 – Sep 26) are closed and 7–12 (Oct 26 – Mar 27) are open; the current period is "Oct 26".
- **Vouchers:**
  - 100 is posted: 102101 Dr 250.00 / 101000 Cr 250.00 in period 7, with its two `gl_dist` rows;
  - 101 is open and balanced: 111101 Dr 1,150.00 / 410001-10 Cr 600.00 / 410001-20 Cr 550.00;
  - 102 is open and out of balance: 148000 Dr 100.00 / 260007 Cr 90.00;
  - 103 is an open adjustment (mode 3): 144910 Dr 75.00 / 111101 Cr 75.00.
  - The next new voucher is 104.
- **Master data:** departments 10 and 20; medical reps MR001–MR003; doctors DR001–DR003; products P100, P200, P300.

## The 14 criteria (spec §8)

Run these on the test machine, logged in as `ADMIN`.

### 1. Master data

- [ ] **Accounts > Department Codes** (1023): add dept `30` `TEST DEPT`, change its description to `TEST DEPT EDITED`, then delete it. After each step, `sqlite3 dev.db "select * from gldept where trim(depart) = '30'"` shows the change (no row after the delete).
- [ ] **Medical Reps** (1006): add `MR009` with a name and territory, change its name, then delete it. Check each step in `gl_medrp` (`trim(repcode) = 'MR009'`). The territory is saved (D10).
- [ ] **Doctor Codes** (1040): the same with `DR009`, in `gl_drcod` (`trim(drcode) = 'DR009'`).
- [ ] **Product Codes** (1007): the same with `P900`, in `gl_itemm` (`trim(fpcode) = 'P900'`). Add/Mod with a blank code is refused with 4072. Modifying P100 leaves its `flotsize` (10) unchanged (D10).
- [ ] Each screen's list (Search, or its Med Reps / Doctor / Products button) shows the records, and picking a row shows that record.

**Result:** ____ **Notes:** ____

### 2. Voucher entry

- [ ] **Transactions > General Ledger Voucher** (1013) opens with voucher number 104, the Distribution List below it, and the period combo listing only Oct 26 to Mar 27, with Oct 26 selected.
- [ ] Enter a description `ACCEPTANCE 104`, a date, and three lines (Reference and Description are required on each):
  - 102101, Debit 300.00;
  - 410001 dept 10, Credit 200.00, medical rep MR001, product P100, doctor DR001;
  - 410001 dept 20, Credit 100.00, medical rep MR002, product P200, doctor DR002.

  After each Add/Mod the Distribution List shows the line, then "Total" with 300.00 / 300.00 once balanced.
- [ ] `sqlite3 dev.db "select batchno, mode, reference, debit, credit, entryno from gl_btch where batchno = 104"` returns `104|1|ACCEPTANCE 104|300.0|300.0|3`.
- [ ] `sqlite3 dev.db "select journal, trim(account), trim(dept), trim(medrep), trim(prod), trim(drcode), debit, credit from gl_btchd where batchno = 104 order by journal"` returns the three lines with their codes.
- [ ] An unknown account (`999999`), or a known account with an unknown dept (`410001` dept `99`), is refused: a beep, the account list opens at the nearest account, and nothing is saved.

**Result:** ____ **Notes:** ____

### 3. Voucher edit

- [ ] Close Voucher Entry and open it again. Click **Voucher** to show the voucher list, and pick 104.
- [ ] In the Distribution List, double-click the 410001 dept 10 line: its account, amounts, medical rep, product and doctor fill the fields. Change the credit to 250.00 and click **Add/Mod**. The total row shows "Voucher Out Of Balance" in white on red.
- [ ] Pick the 410001 dept 20 line and click **Delete**, answering **Yes**. Then add a line 410001 dept 20, Credit 50.00. The totals balance again at 300.00.
- [ ] `gl_btchd` for 104 has three lines: 300.00 Dr, 250.00 Cr (MR001 / P100 / DR001) and 50.00 Cr. `gl_btch` shows 300.0 / 300.0.
- [ ] Start a new voucher (**New**), add one line, then click **Close** and answer **No** to "Do you want to save the current transaction?" (1056): voucher 105 is not in `gl_btch`.

**Result:** ____ **Notes:** ____

### 4. Prior Period Entry

- [ ] **Transactions > Prior Period Entry** opens the same dialog, captioned "Prior Period Adjustment Entry".
- [ ] Choose voucher 101 (a normal voucher) from the voucher list or by navigating: "Only prior period adjustment entries may be modified in this program." It isn't shown, and `gl_btchd` for 101 is unchanged.
- [ ] Voucher 103 (an adjustment) opens normally.

**Result:** ____ **Notes:** ____

### 5. Post all

Record first: `sqlite3 dev.db "select trim(account), trim(dept), period7, curr_bal, openbal from glaccts"` and `select count(*) from gl_dist`.

- [ ] **Transactions > Post Voucher > General Ledger Voucher** (1061), captioned "Post General Ledger Voucher", lists 101, 102, 103 and 104.
- [ ] Click **Post**. You get "Voucher number 102 is out of balance and can not be posted." (8116), and 102 stays listed. The others leave the list.
- [ ] `gl_btch`: 101 and 104 have mode `0`, 103 has mode `2`, 102 still `1`. `gl_btchd` lines of 101 and 104 have mode `0`.
- [ ] `gl_dist` gained 6 rows: three for 101 and three for 104, with their medrep / prod / drcode. None for 103.
- [ ] `glaccts`, compared with the record:
  - 410001-10: `period7` and `curr_bal` down 850.00 (600 from 101, 250 from 104);
  - 102101: up 300.00;
  - 144910 (adjustment 103): `openbal` up 75.00, `curr_bal` 75.00, `period7` unchanged;
  - 111101: `openbal` down 75.00.
- [ ] `select sum(curr_bal) from glaccts` is still 0.

**Result:** ____ **Notes:** ____

### 6. Atomic posting

- [ ] Copy `dev.db` aside now (after #5), then in Voucher Entry add a voucher 105 with two balanced lines (for example 148000 Dr 10.00 / 260007 Cr 10.00), and close it, keeping it.
- [ ] Record `select * from glaccts`, `select * from gl_dist`, `select * from gl_btch`, `select * from gl_btchd` into files.
- [ ] Exit, and start `gl.exe /failpost 1`. Post: you get "Posting failed, and nothing was changed." naming the `/failpost` fault.
- [ ] The four tables match the recorded files exactly.
- [ ] Exit, start `gl.exe` normally and Post again: 105 posts (102 is refused again).

**Result:** ____ **Notes:** ____

### 7. Recover

Record `glaccts` and `gl_dist` for 101's accounts (111101, 410001-10, 410001-20) first.

- [ ] **Transactions > Recover General Ledger Voucher** (1015) lists the posted vouchers 100, 101, 104 and 105 (not the posted adjustment 103).
- [ ] Double-click 101 and answer **Yes**. It leaves the list.
- [ ] `glaccts`: 111101's `period7` and `curr_bal` are 1,150.00 lower, and 410001-10 and -20 are 600.00 and 550.00 higher: their values before 101 was posted.
- [ ] `gl_dist` has no rows for 101. `gl_btch` 101 has mode `1`, and its lines have a blank mode.
- [ ] Voucher 101 opens in Voucher Entry again, and **Post** posts it again.

**Result:** ____ **Notes:** ____

### 8. Cancel

- [ ] **Transactions > Cancel Voucher > General Ledger Voucher** (1043), captioned "Cancel General Ledger Voucher", lists the unposted vouchers (102).
- [ ] Double-click 102 and answer **Yes** to "Are you sure you want to delete this Voucher?" (8099).
- [ ] `select count(*) from gl_btch where batchno = 102` and the same on `gl_btchd` both return 0.

**Result:** ____ **Notes:** ____

### 9. Transaction Inquiry

- [ ] **Transactions > Voucher Reports > General Ledger Voucher Inquiry** (1029). Type `102101`, blank dept, click **Set Data**.
- [ ] Row 1 shows "Opening Account Balance" 12,500.00 in Debit. From row 4: voucher 100's 250.00 and voucher 104's 300.00 debits, period 7. Then "Account Balance" 13,050.00.
- [ ] `410001` dept `10`: only that dept's posted lines (101 and 104). `410001` with a blank dept: "Unable to find GL Account" (3081) (D13).
- [ ] **Search** shows and hides the account list; picking an account there lists its lines.

**Result:** ____ **Notes:** ____

### 10. Budget Data

- [ ] **Accounts > Budget Data** (1017), captioned "General Ledger - Budget Update". Pick 410001 dept 10 in the account list: the grid shows periods 1–12, with Current Year period 7 at −850.00.
- [ ] Edit Budget for periods 1, 6 and 12 (Enter, F2 or a double-click; Enter commits). Click **Add**, answer **Yes** to "Update the current budget information?" (8098).
- [ ] Close, reopen, pick 410001 dept 10: the three amounts are there. `select budget1, budget6, budget12 from glaccts where trim(account) = '410001' and trim(dept) = '10'` matches.
- [ ] No other account's budget changed.

**Result:** ____ **Notes:** ____

### 11. Year end

- [ ] Copy `dev.db` aside (year end can't be undone, except by Restore). Record `select sum(curr_bal) from glaccts` (0).
- [ ] In the Chart of Accounts set 101000's type to **Retained earnings**. (The Chart refuses a second R account on Add, but Modify allows it, as in the real data.) **Maintenance > Year End Consolidation**, **Yes** to 8111: it refuses and names 101000 and 340000. Nothing changed.
- [ ] Set 101000 back to **Balance Sheet** and run it again: it reports the year closed and 340000's new balance.
- [ ] `glaccts`: 410001-10 and -20 have `curr_bal` 0; 340000's `curr_bal` is its old balance plus theirs; every account's `period1`–`period12` are 0 and `previous1`–`previous12` hold the old periods; balance-sheet accounts keep their `curr_bal`.
- [ ] `select sum(curr_bal) from glaccts` is still 0.

**Result:** ____ **Notes:** ____

### 12. Backup and Restore

- [ ] **Maintenance > Backup Databases**: the Save As dialog suggests `dev-<today>.db` beside `dev.db`. Save it. A message names the file.
- [ ] Add account `150002` `AFTER BACKUP` in the Chart of Accounts.
- [ ] **Maintenance > Restore Databases**, choose the backup, answer **Yes**. A message says it was restored.
- [ ] 150002 is gone from the Chart of Accounts; `dev.db.bak` exists and has it (`sqlite3 dev.db.bak "select count(*) from glaccts where trim(account) = '150002'"` returns 1); `gl.exe` keeps running, and the module bar shows the company and period.
- [ ] Restore a text file renamed to `.db`, and a copy of `GL.INI`: each is refused with a reason, and the company stays open.
- [ ] **Maintenance > Reindex Databases** shows the "doesn't need reindexing" message (D6).

**Result:** ____ **Notes:** ____

### 13. Grid

- [ ] The account list (Chart of Accounts) behaves as Spec #1 §6.7 #7 describes (the Spec #1 sheet's #7), now as a grid.
- [ ] **Maintenance > User Access**, then the Menu button (1160): the menus show with PLUS / MINUS pictures; toggling one and clicking OK saves it, Close doesn't (D9).
- [ ] **File > Company Profile**, **Accounts** (1033), then **Cash** (1034): the Cash Accounts list (1011) beside it lists the cash accounts (102101 and 101000), and picking one fills the dialog.
- [ ] Right-click the Chart of Accounts' account list, set the query to `ACCOUNT = '4'`, OK: the list shows only the two 410001 rows. Set it back to `.NOT. DELETED()`.

**Result:** ____ **Notes:** ____

### 14. Regression and resources

- [ ] The Spec #1 sheet's criteria #1–#9 still pass (on a fresh copy of `dev.db`).
- [ ] `/checkres /quiet` and `/stress /quiet` exit 0 (P2).
- [ ] No leaks in Debug (P2).

**Result:** ____ **Notes:** ____

## Known gaps accepted before the walkthrough

| # | Gap | Why accepted | Follow-up |
|---|---|---|---|
| G1 | High DPI is not tested. The walkthrough runs at 96 DPI. | As Spec #1 G1. | The AP/AR specs (#5/#6). |
| G2 | Voucher Entry's Print, and every report, show the "not yet ported" message. | Reports are Spec #3. | Spec #3. |
| G3 | Month close (1069) is not ported. Periods are closed in Company Profile > Periods. | D4: the original had retired it. | None. |
| G4 | Maintain External Vouchers stays a stub. | D7. | When its purpose is known. |
| G5 | After a year end, Transaction Inquiry's balance and adjustment posting use `openbal`, which year end leaves alone, as the original did. | Spec §12 Q6; the real data's year end has never run. | Settle before the first real year end. |
| G6 | The `querySet` comparison rules follow `dbf2sqlite`'s translation, not CodeBase 4's documented behaviour. | Spec §12 Q5; no CodeBase source in the tree. | Confirm if the original build or its docs turn up. |

## Gaps and triage

Add one row for each `FAIL` or unexpected behaviour found during the walkthrough.

| # | Criterion | What happened | Decision (fix in Spec #2 / file for Spec #N) | Fixed in commit / issue |
|---|---|---|---|---|
| | | | | |

## Sign-off

| Criterion | Result |
|---|---|
| 1. Master data | ____ |
| 2. Voucher entry | ____ |
| 3. Voucher edit | ____ |
| 4. Prior Period Entry | ____ |
| 5. Post all | ____ |
| 6. Atomic posting | ____ |
| 7. Recover | ____ |
| 8. Cancel | ____ |
| 9. Transaction Inquiry | ____ |
| 10. Budget Data | ____ |
| 11. Year end | ____ |
| 12. Backup and Restore | ____ |
| 13. Grid | ____ |
| 14. Regression and resources | ____ |

All 14 criteria pass, or pass with the known gaps recorded above:

- Tester: ____ Date: ____
- Witness: ____ Date: ____
