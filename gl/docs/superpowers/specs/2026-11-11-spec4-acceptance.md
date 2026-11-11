# Spec #4 acceptance sign-off

This sheet records plan Milestone 6 (`gl/docs/superpowers/plans/2026-11-11-owl-to-win32-sdk-port-spec4-plan.md`, "Milestone 6"). It covers the 9 manual checks in spec §10 (`2026-11-11-owl-to-win32-sdk-port-spec4-design.md`).

Fill in every blank (`____`) and set each **Result** to `PASS`, `FAIL` or `PASS with known gap`. A `FAIL` needs a triage line in [Gaps and triage](#gaps-and-triage): either fixed in Spec #4, or filed against a later spec.

**Which copy of `dev.db`:**
- Criteria #1–#7 run in order on one fresh copy: #3 runs period 7, #4 cancels and re-runs it, #5 posts it, and #6 disposes of an asset after that.
- #8 runs on a second fresh copy, because it closes the year.
- Keep an untouched copy so the walkthrough can be repeated.

## Run details

| Item | Value |
|---|---|
| Commit tested (`git rev-parse --short HEAD`) | ____ |
| Build machine | ____ |
| Test machine (fresh install, not the dev box) | ____ |
| Windows edition and build (`winver`) | ____ (target: Windows 11 23H2 or later, x64) |
| Display scaling | ____ (target: 100% / 96 DPI) |
| Printers installed | ____ (at least *Microsoft Print to PDF*) |
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
| `x64\<Config>\gl_tests.exe` (112 test cases pass; `[assets]` 19 of them) | ____ | ____ | |
| `x64\<Config>\report_tests.exe` (17 test cases pass) | ____ | ____ | |
| `gl.exe /checkres /quiet` (now also 4002, 4004, 4003, 1001) | ____ | ____ | |
| `gl.exe /stress /quiet` (now also the four Fixed Assets screens and the two reports' previews) | ____ | ____ | |
| `trivial_dialog.exe /selftest /quiet` | ____ | ____ | |
| Debug `gl.exe` under the debugger: open and close each Fixed Assets screen, then exit. The Output window shows no "Detected memory leaks!" | ____ | n/a | |

### P3. Fresh-machine install

As in the Spec #3 sheet (P3), with the current `dev.db`. The fixture now includes `GL_FA`, `GL_FATYP` and `GL_FADEP`.

| Check | Result | Notes |
|---|---|---|
| `gl.exe` starts with no Visual C++ runtime installed; login as `ADMIN` / `ADMIN` works | ____ | |
| `sqlite3 dev.db "select count(*) from gl_fa where _deleted=0"` returns 4 | ____ | |

## The fixture

The Spec #3 company has the current period 7 ("Oct 26", ending October 31, 2026). Its fixed-asset register (spec §9) agrees with the ledger: equipment 151000 is 35,000.00 Dr and accumulated depreciation 161000 is 6,600.00 Cr at period 7.

- **Types:**
  - **EQ** "Office equipment": 151000 cost, 761000 expense, 161000 accumulated depreciation.
  - **XX** "Bad accounts (test)": every account is 999999, which isn't in the chart.
- **Assets:**

  | Code | Description | Type | Method | Cost | Life | Used | Accumulated | Monthly | Note |
  |---|---|---|---|---|---|---|---|---|---|
  | E001 | Office furniture | EQ | Straight | 30,000.00 | 60 | 13 | 6,500.00 | 500.00 | bought 04/01/2025 |
  | E002 | Laptops | EQ | Declining 24 % | 5,000.00 | 36 | 1 | 100.00 | 100.00 | bought 06/15/2026 |
  | E004 | Old printer | EQ | Straight | 1,000.00 | 36 | 36 | 1,000.00 | 27.78 | disposed 08/15/2026 |
  | E005 | Projector | XX | Straight | 2,000.00 | 24 | 0 | 0.00 | 83.33 | bought 05/01/2026 |

- **History** (`gl_fadep`): E001 500.00 and E002 100.00 in period 6, with no voucher.
- **Vouchers:** 100 is posted; 101–103 are open. The next voucher number is 104.

The figures below come from `gl_tests` (`test_assets.cpp`, `test_reports_assets.cpp`) and were checked against these queries on a fresh `dev.db`.

## The 9 criteria (spec §10)

Log in as `ADMIN`. The screens are on the **Fixed Assets** menu.

### 1. Asset Types

- [ ] **Asset Types:** type `FU`, description "Furniture", asset account 151000, expense 999999, accumulated 161000; **Add**. The message reads "Invalid Expense Account", with the cursor on the expense account.
- [ ] Change the expense account to 761000; **Add**. The fields clear. **Types** shows FU in the list.
- [ ] Type `FU` and press **Enter**: its description and accounts show. Change the description; **Add**, **Yes** to modify.
- [ ] Type `EQ`, **Delete**, **Yes**: "Asset type EQ can't be deleted: asset E001 uses it."
- [ ] Type `FU`, **Delete**, **Yes**: deleted. **Top** shows EQ; **Next**, **Next** stops at XX.

**Result:** ____ **Notes:** ____

### 2. Add/Modify

- [ ] **Add/Modify:** type `E001` and press **Enter**. The screen shows:
  - type EQ with its three accounts, read-only;
  - monthly 500.00, accumulated 6,500.00, net book value 23,500.00, life 5.0 years;
  - accumulated and months used read-only.
- [ ] **Next**: E002, Declining, with the rate field showing 24 and monthly 100.00.
- [ ] **New**: type the following and press **Enter** after the type:
  - code `E006`, date 10/01/26, description "Desk";
  - type `XX`; its accounts (999999) fill in. XX keeps E006 out of #3's run and of #5's check for 151000/161000;
  - cost 1,200.00, life 12.

  Monthly shows 100.00.
- [ ] Salvage 2,000.00, **Add/Mod**: "The salvage value can not be greater than the initial cost of the asset." Set salvage back to 0, then **Add/Mod**: the fields clear.
- [ ] `E006` + **Enter**, life 24, **Add/Mod**, **Yes**: monthly 50.00. Check with `sqlite3 dev.db "select curr_dep, lifetime from gl_fa where trim(facode)='E006'"`, which returns `50.0|24`.
- [ ] `E004` + **Enter**: the caption ends "- disposed 08/15/2026", and **Add/Mod** says it can't be changed.
- [ ] Type a new asset of type `ZZ`: **Add/Mod** says there is no fixed asset type ZZ.

E006 is part of the register until #6 deletes it. Its type's account isn't in the chart, so #3's run skips it.

**Result:** ____ **Notes:** ____

### 3. Create Voucher in period 7

- [ ] **Create Voucher:** the period shows "7 (Oct 26)", and the line under it reads "2 assets due: 600.00. 3 skipped."
- [ ] **OK** asks, listing:
  - E001 500.00 and E002 100.00;
  - skipped: E004 (disposed), and E005 and E006 (their type's account isn't in the chart).

  Answer **Yes**: "Voucher 104 created: 2 assets, 600.00." The dialog closes.
- [ ] `sqlite3 dev.db "select date, period, debit, credit, entryno, trim(mode) from gl_btch where batchno=104"` returns `20261031|7|600.0|600.0|4|1`.
- [ ] `sqlite3 dev.db "select journal, trim(account), debit, credit, trim(reference) from gl_btchd where batchno=104 order by journal"` returns four lines: 761000 500 Dr / 161000 500 Cr (E001), then 761000 100 Dr / 161000 100 Cr (E002).
- [ ] **Add/Modify**, E001: accumulated 7,000.00, used 14. E002: accumulated 200.00, used 2.

**Result:** ____ **Notes:** ____

### 4. Run again, cancel, run again

- [ ] **Create Voucher** again: "Period 7 has been run: voucher 104." **OK** says to cancel that voucher first.
- [ ] **Transactions > Cancel Voucher > General Ledger Voucher**: double-click voucher 104 and confirm. Then E001 is back at 6,500.00 / 13 and E002 at 100.00 / 1, and `sqlite3 dev.db "select count(*) from gl_fadep where batchno=104"` returns 0.
- [ ] **Create Voucher**, OK, Yes: voucher 104 again, 600.00.
- [ ] Cancel an ordinary voucher (103): the assets don't change.

**Result:** ____ **Notes:** ____

### 5. Post, and check against the ledger

- [ ] **Transactions > Post Voucher > General Ledger Voucher**: double-click voucher 104 and confirm. It is posted.
- [ ] **Reporting > General Ledger > Trial Balance**, period 7: 761000 reads 1,200.00 Debit and 161000 7,200.00 Credit.
- [ ] **Fixed Assets > Fixed assets listing:**
  - "151000 OFFICE EQUIPMENT (cost): register 35,000.00, ledger 35,000.00. Agrees.";
  - "161000 ... (accumulated depreciation): register 7,200.00, ledger 7,200.00. Agrees.";
  - 999999 is named as not in the chart.

  Before posting, 161000 would read "Differs by 600.00" (`gl_tests`).

**Result:** ____ **Notes:** ____

### 6. Disposal

- [ ] **Disposal:** `E002` + **Enter**. The line under the box reads "Cost 5,000.00  Accumulated depreciation 200.00  Book value 4,800.00".
- [ ] Sale Price 4,000.00: Gain/Loss shows -800.00.
- [ ] Date Disposed 10/31/26, then **Dispose**, **Yes**. The message lists Dr 161000 200.00, Dr proceeds 4,000.00, Dr loss 800.00 and Cr 151000 5,000.00. The caption ends "- disposed 10/31/2026", and **Dispose** again says it was already disposed of.
- [ ] **Fixed assets listing:** E002 shows "(disposed)" with its date sold. Now 151000 differs by −5,000.00 and 161000 by −200.00, until the entry is made in Voucher Entry.
- [ ] `E001` + **Enter**, **Delete**, **Yes**: refused, "use Dispose". `E006` + **Enter**, **Delete**, **Yes**: deleted.
- [ ] Running Create Voucher for a later period (after closing period 7 in Company Profile > Periods) lists E002 as disposed.

**Result:** ____ **Notes:** ____

### 7. Reports

- [ ] **Fixed assets listing** previews and prints to PDF:
  - landscape, grouped EQ / XX, with each asset's reference under it;
  - type totals and a total of the assets in use;
  - "1 disposed asset is listed but not totalled" (2 after #6);
  - the footer reads "Report File: gl_fa.rpt".
- [ ] **Fixed asset types:** EQ and XX with their three accounts each, then "2 types". The title reads "General Ledger Fixed Asset Types".
- [ ] **File > Print** with no preview open lists both reports (17 and 18), and each runs from there.

**Result:** ____ **Notes:** ____

### 8. Declining balance across a year end (on a second fresh copy of `dev.db`)

- [ ] **Create Voucher** for period 7, OK, Yes (voucher 104). Post it.
- [ ] **Maintenance > Year End Consolidation**.
- [ ] **File > Company Profile > Periods**: set every end date a year later (e.g. 04/30/27 … 03/31/28). Clear period 1's closed box, so period 1 becomes current.
- [ ] **Create Voucher:** period 1, "2 assets due: 596.00. 2 skipped." OK lists E001 500.00 and E002 **96.00**, which is (5,000.00 − 200.00) × 24 % ÷ 12.
- [ ] Without the new dates (skip the Periods step on a third copy), the run would treat period 1 as part of the old year (design §4.2, R5): note what happens. ____

**Result:** ____ **Notes:** ____

### 9. Regression and resources

- [ ] The Spec #1, #2 and #3 sheets' criteria still pass, on a fresh copy of `dev.db`. The report list now holds 18 reports (Spec #3 sheet #11).
- [ ] `/checkres /quiet` and `/stress /quiet` exit 0 (P2).
- [ ] No leaks in Debug (P2).

**Result:** ____ **Notes:** ____

## Known gaps accepted before the walkthrough

| # | Gap | Why accepted | Follow-up |
|---|---|---|---|
| G1 | The finance team hasn't answered Q2–Q6 (spec §14). The build assumes declining balance, no disposal entry, the period-end voucher date, depreciation from the first run after purchase, and that the real `GL_FA` test records are deleted before go-live. | The module was never used in production (spec §2.5), and each answer touches one function (plan P1). | Settle them with the finance team; change `Assets.cpp` if asked. |
| G2 | **Before go-live, delete the real company's five test records** (F001–F003, TEST, TEST1) with Disposal's Delete (Q4). | Otherwise the first run would depreciate F001–F003 by 6,243.75 a month (spec R1). | A step in the go-live checklist. |
| G3 | After year end, the new year's period dates must be set before the first run (R5). | Year end doesn't change `gl_pds`, as in the original; the README says so. | An automatic check, if #8's last step shows it is needed. |
| G4 | High DPI and real printers aren't tested. | As in the Spec #3 sheet (G1, G3). | |
| G5 | The Linux ASan/UBSan build hasn't been run for Spec #4. | No Linux machine this round. | Run `tools/linux/build_tests.sh --run`. |

## Gaps and triage

Add one row for each `FAIL` or unexpected behaviour found during the walkthrough.

| # | Criterion | What happened | Decision (fix in Spec #4 / file for Spec #N) | Fixed in commit / issue |
|---|---|---|---|---|
| | | | | |

## Sign-off

| Criterion | Result |
|---|---|
| 1. Asset Types | ____ |
| 2. Add/Modify | ____ |
| 3. Create Voucher | ____ |
| 4. Run again, cancel, run again | ____ |
| 5. Post, and check against the ledger | ____ |
| 6. Disposal | ____ |
| 7. Reports | ____ |
| 8. Declining balance across a year end | ____ |
| 9. Regression and resources | ____ |

All criteria pass, or pass with the known gaps recorded above:

- Tester: ____ Date: ____
- Witness: ____ Date: ____
