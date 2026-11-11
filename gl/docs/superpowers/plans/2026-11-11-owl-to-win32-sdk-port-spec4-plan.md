# Spec #4 — Implementation Plan: Fixed Assets

**Spec reference:** `gl/docs/superpowers/specs/2026-11-11-owl-to-win32-sdk-port-spec4-design.md`
**Date:** 2026-11-11
**Plan status:** Draft

**Paths:** relative to the repository root. The legacy source is `gl/src/gl_fa.cpp`, with `gl/header/gl_fa.h`, `gl_asstl.h` and `gl_fatyp.h`; the Crystal files are `dbf/GL_FA.RPT` and `dbf/GL_FATYP.RPT`. New code goes in `gl/port/` (screens, and the portable `Assets.{h,cpp}`) and `gl/port/reports/` (the two reports).

This plan carries out the Spec #4 design. If a milestone finds that the spec is wrong, **stop, fix the spec, then resume**.

**Read first, as in Specs #2 and #3.** Each milestone that ports a screen starts by re-reading its part of `gl_fa.cpp` and its dialog in `gl/gl_new.rc`, and checking the spec's §2 against them. The spec's §2.4 lists what doesn't work. Each fix is a decision (D1–D12), and each screen's translation notes say which ones apply.

**Open questions.** Q1 is settled: the module is ported with the cycle fixed (D1). Q2, Q5 and Q6 change the arithmetic and the run, and Q3 changes Disposal. Until the finance team answers, the spec's assumptions stand. M3 and M4 note where an answer would change them, and each answer touches one function in `Assets.cpp`.

---

## Milestone summary

| # | Milestone | Size | Depends on | Critical path? |
|---|---|---|---|---|
| 0 | `Assets` core: tables, records, the arithmetic; the fixture; tests | M | none | yes |
| 1 | Asset Types (4002) | S | M0 | yes |
| 2 | Fixed Assets Add/Modify (4004) | M | M1 | yes |
| 3 | Create Voucher (4003): the run, the Cancel hook | M | M2 | yes |
| 4 | Disposal (1001) | S | M2 | no |
| 5 | Reports 1019 and 1020; report list; `/stress`, `/checkres`; README | S | M3, M4 | yes |
| 6 | Acceptance sheet, walkthrough, sign-off | S | all | yes |

S is less than a day, M is 1–3 days. **Parallel work:** M4 needs only M2 and can run beside M3.

---

## Milestone 0 — The portable core

> **As built (2026-11-11; built and run on Windows 11, VS 18, v145).**
> - **Read** (task 1): spec §2.1 holds. `LIFELEFT` is months used (`TGLCreateDeprSlip::Ok` adds 1; dialog 4004 labels it "Months Used"). The real tags are `FACODE` and `DEP_METH` on `GL_FA`, and `FATYPE` on `GL_FATYP`.
> - **`gl/port/Assets.{h,cpp}`** (task 2):
>   - `EnsureAssetTables` creates the three tables (with the real tags) and does the one-time `ACCUM_DEP` fill: a NULL `ACCUM_DEP` with months used.
>   - `LoadAssetTypes` / `FindAssetType` / `SaveAssetType` and `LoadAssets` / `FindAsset` / `SaveAsset` work in cents. `Asset::disposed()` is `MODE` "D".
>   - `ValidateAccount` matches the account and the dept, blank for none.
>   - The arithmetic is pure: `Depreciable`, `BookValue`, `Remaining`, `MonthlyAmount` and `DueAmount`.
>   - The history: `FiscalYearEnd` (gl_pds period 12), `LoadDepreciation`, `AppendDepreciation` and `StartOfYearBookValue` (cost less the accumulated depreciation before this year's `gl_fadep` rows).
> - **Found while testing:** calling `Data4::blank()` *before* `appendStart()` flushes blanks over the current record. A failed lookup leaves a record current, so adding a second asset wiped the first. Every append here calls `appendStart()` then `blank()`, as `Data4::appendBlank` does, and a test checks that adding leaves the others intact.
> - **Fixture** (task 3):
>   - `fixed_assets()` writes `GL_FATYP` (EQ, XX), `GL_FA` (E001, E002, E004, E005) and `GL_FADEP`. The history holds period 6's 500.00 and 100.00 with `BATCHNO` 0, so E002's start-of-year value is its cost (spec §9).
>   - **Every existing table is row-for-row identical** except `ACCESSM`'s Custom Report row. That row's ID becomes 20501 (`CM_REPORT_LIST`): S3-M7 changed the menu but didn't regenerate the fixture.
>   - Regenerating at first dropped the row, because `manila_items()` read only numeric IDs. It now resolves `gl/port/PortIds.h` symbols and stops on an unknown one.
>   - The Spec #3 sheet's queries give the same results.
> - **Tests** (task 4): `gl_tests/test_assets.cpp`, 7 cases:
>   - the fixture's register, which agrees with 151000 and 161000;
>   - `ValidateAccount`;
>   - straight line (4,800 ÷ 48; 1,000 ÷ 3 as 333.33 + 333.33 + 333.34; 60 months summing to the cost);
>   - the salvage floor;
>   - declining from `gl_fadep`, including the next year's 98.00;
>   - saving without overwriting;
>   - missing tables created, and the blank fill (16,650.00, and one capped at 99,900.00).
>
>   `gl_tests`: 100 cases.
> - **Checked:** Debug and Release `/W4 /WX` clean; every suite, `/checkres`, `/stress` and the self-test pass.

**Goal:** assets and types can be read, written and depreciated on Linux, under test, on a fixture whose register agrees with its ledger.

### Tasks

1. **Read** `gl_fa.cpp` whole: `UpdateData`, `SetData` and `TGLCreateDeprSlip`. Read the deployed `GL_FA`, `GL_FATYP` and `GL_FA.MDX` / `GL_FATYP.MDX` (tags). Confirm spec §2.1's field meanings, in particular that `LIFELEFT` is months used.
2. **`gl/port/Assets.{h,cpp}`**, portable (no Win32), over `cb4sqlite`:
   - `EnsureAssetTables(code)`: creates `gl_fa` and `gl_fatyp` with the original layouts and tags when missing, and always `gl_fadep` (spec §5.3). It also does the one-time `ACCUM_DEP` fill (spec §5.1) and returns how many assets it filled.
   - `AssetType` and `Asset` records (amounts in cents, as Spec #2 D12) with `Load…` / `Save…` / `Find…` functions, deleted rows skipped. `Asset::disposed()` reads `MODE` "D"; "0" and "" are active.
   - **The arithmetic** (spec §4.1), as pure functions with no table access so the tests can call them directly:
     - `MonthlyAmount(asset, startOfYearBookValue)`;
     - `DueAmount(asset, …)`, with the salvage floor and the last-month remainder;
     - `BookValue(asset)`;
     - `StartOfYearBookValue(code, asset, yearEnd)`, from `gl_fadep`.
   - `ValidateAccount(code, account, dept)`, as `ValidateGL`: the account and dept exist in `glaccts`.
3. **The fixture** (spec §9): `GL_FA`, `GL_FATYP` and an empty `GL_FADEP` in `make_dev_fixture.py` (types EQ and XX; assets E001, E002, E004, E005). It must leave every Spec #1–#3 figure unchanged. Check by running the three sheets' queries (`sheet_queries.py`, Spec #3 S3-M8) before and after.
4. **Tests**, `gl_tests/test_assets.cpp`:
   - **Tables:** missing tables are created; the blank-`ACCUM_DEP` fill.
   - **Straight line:** 4,800 ÷ 48 = 100.00, and a 1,000 ÷ 3 rounding case whose three months sum to 1,000.00.
   - **Declining:** the start-of-year value from `gl_fadep`.
   - **The salvage floor:** an asset one month from the end gets only its remainder.
   - **The fixture's register:** cost 35,000.00 and accumulated 6,600.00 for 151000 / 161000.
   - **Accounts:** `ValidateAccount` on 161000 (yes) and 999999 (no).
5. **Build lists:** add `Assets.cpp` and the test to `gl_tests.vcxproj` and `tools/linux/build_tests.sh`; run `genproj.py` for `gl`.

### Exit criteria

- `gl_tests` passes in Debug and Release (and on Linux, when a machine is available).
- The Spec #3 sheet's figures are unchanged on the new fixture.

---

## Milestone 1 — Asset Types (4002)

> **As built (2026-11-11; built and run on Windows 11, VS 18, v145).**
> - **Read** (task 1): spec §2.2 and §2.4 #9 hold. Two further findings:
>   - `TGLTypes::Ok` checked the accounts, then returned silently on a blank type.
>   - `TAssetTypesList` showed `FAYEARS` under a "Description" header, with the screen's caption (string 6061) as the second header.
> - **`prune_rc.py`** (task 2): 4002 added to `DIALOGS`; `gl.rc` regenerated (the template verbatim).
> - **`gl/port/FixedAssetTypes.{h,cpp}`** (task 3), the Spec #2 master-data pattern:
>   - **Add** asks for the type and description (8101), then checks each account and dept (6056–6058, with the focus on the bad one). It modifies an existing type after asking (8108), then clears.
>   - **Delete** asks (8109), then calls `Assets.h`'s new `DeleteAssetType`. That function refuses a type in use, naming the first asset; otherwise it deletes and packs.
>   - **Enter in the type field** shows the type, or moves on to the description for a new one. Enter elsewhere is Add. (Enter and the Add button both send `IDOK`, so the focus decides.)
>   - **Types** opens the list (`GL.INI [GL_FATYP]`; Code, Description).
>   - **New, Top / Bottom / Next / Previous**: Next and Previous stop at the ends, and continue from a type shown by Enter or the list.
> - **Wire** (task 4): 6036 (`ID_FATYPE`) in `ManilaGL.cpp`. `/checkres` loads 4002. `/stress` has a new "fixed assets" step, `StressFixedAssets`, which the later screens join.
> - **Tests** (task 5): `DeleteAssetType` (in use, naming E001 and E005; not found; deleted, leaving the others). `gl_tests`: 101 cases.
> - **Checked:** Debug and Release clean; every suite, `/checkres`, `/stress` and the self-test pass. Driven in the app on the fixture:
>   - FU with expense account 999999: "Invalid Expense Account";
>   - FU with good accounts: added and cleared;
>   - FU typed and Enter: shown;
>   - Delete EQ: refused, naming E001;
>   - Top: EQ; Next: FU, then XX, stopping at XX;
>   - Delete FU: deleted.

**Goal:** types can be maintained, with their accounts checked.

### Tasks

1. **Read** `TGLTypes` and `TAssetTypesList`, and dialog 4002.
2. **`prune_rc.py`:** add 4002 to `DIALOGS` (4004, 1001 and 4003 come in their milestones); regenerate `gl.rc`.
3. **`gl/port/FixedAssetTypes.{h,cpp}`** on the Spec #2 master-data pattern (`MasterData.cpp`):
   - **Add:** the required fields (8101) and the three accounts (6056–6058). An existing type is modified after asking (8108).
   - **Delete** asks (8109), then refuses a type an asset uses (D9), naming one of them.
   - **New, Top / Bottom / Next / Previous;** Enter on the type shows it.
   - **Types** shows a `TableList` (`GL.INI [GL_FATYP]`, columns Code, Description).

   The translation notes name D9. A delete packs, as the original and Spec #2's `DeleteRecord` do.
4. **Wire** 6036 (Fixed Assets > Asset Types) in `ManilaGL.cpp`. **`/checkres`** loads 4002 and **`/stress`** opens and closes it.
5. **Tests:** saving and deleting types, and refusing to delete a type in use, through `Assets.h`.

### Exit criteria

- On the fixture: add type FU with good accounts; a bad account is refused, naming the field; deleting EQ is refused.
- `/checkres` and `/stress` exit 0.

---

## Milestone 2 — Fixed Assets Add/Modify (4004)

> **As built (2026-11-11; built and run on Windows 11, VS 18, v145).**
> - **Read** (task 1): spec §2.2 and §2.4 hold. `TAssetList` showed `REFERENCE` under "Description"; the port shows `DESCRIPT`. `TableList::Spec` gained `fields` (the default field numbers when `GL.INI` has no `FIELD1..3`) for that.
> - **`prune_rc.py`** (task 2): 4004 added. **The template has the GL's only `tbTime`** (1035, never set), which win32fx didn't register, so the dialog failed to load (`/checkres` 1, `/stress` −1). win32fx now registers `tbTime` as a tbPic; the port hides it.
> - **`gl/port/FixedAssets.{h,cpp}`** (task 3):
>   - **Lookup:** Enter on the code shows the asset (a new code moves on). Enter on the type fills its accounts, as does leaving the field. Assets and Types open their lists, both hidden until asked for.
>   - **Add/Mod** runs `Assets.h`'s new `CheckAsset`: 8101 naming the field, an unknown type, 9097, 9098, and accumulated above cost − salvage. The focus goes to the field. An existing asset is modified after asking (8102), then shown again; a new one is added and the screen cleared.
>   - **The figures** (monthly, net book value, years) are recomputed as cost, salvage, life, months used, rate, accumulated or the method change. The declining start-of-year value comes from `gl_fadep` for a depreciated asset.
>   - **Accumulated and months used** are editable (`EM_SETREADONLY`) until `HasDepreciation`. Add/Mod keeps the stored ones after that.
>   - **A disposed asset** shows "- disposed <date>" in the caption, and Add/Mod refuses to change it.
>   - "Full Year" and the time control are hidden; the rate shows only for Declining.
> - **Wire** (task 4):
>   - 6000 (`ID_FAADD`) in `ManilaGL.cpp`; `/checkres` loads 4004.
>   - `StressFixedAssets` moved to `FixedAssets.cpp` and opens both screens. `RunAssetTypes` and `RunFixedAssets` take `closeAtOnce`.
> - **Tests** (task 5): `CheckAsset` (each refusal, by field) and `HasDepreciation`. `gl_tests`: 102 cases.
> - **Checked:** Debug and Release clean; every suite, `/checkres`, `/stress` and the self-test pass. Driven in the app:
>   - E001: 500.00 a month, 6,500.00, 23,500.00, 5.0 years; accumulated and months used read-only.
>   - Next shows E002: 100.00 a month, the rate (24) shown.
>   - New asset E006 (1,200.00, life 12): 100.00 a month; accumulated editable.
>   - Salvage 2,000.00 is refused (9097). E006 is added; shown again, its life changed to 24 and confirmed: 50.00 a month, saved.
>   - E004: the caption says it was disposed 08/15/2026, and Add/Mod refuses to change it.
>   - Type ZZ is refused.

**Goal:** the register can be maintained, showing the figures the run will use.

### Tasks

1. **Read** `TGLFixedAssets` (`SetupWindow`, `Ok`, `UpdateData`, `SetData`, `DepMethod`, `New`), `TAssetList`, and dialog 4004.
2. **`prune_rc.py`:** add 4004; regenerate.
3. **`gl/port/FixedAssets.{h,cpp}`:**
   - **Lookup:** Enter on the code shows the asset (D9). **Assets** and **Types** show lists (`GL.INI [GL_FA]`, `[GL_FATYP]`); picking a type fills in its accounts, read-only.
   - **Add/Mod:** the checks of spec §6.2 (8101, 9097, 9098, the type exists, a valid date). A new asset is appended; an existing one is modified after asking (8102).
   - **Computed fields:** monthly depreciation, accumulated, net book value and life in years, recomputed as cost, salvage, life, method or rate change.
   - **Accumulated** is typed only for a new asset, and fixed once `gl_fadep` has a row for it (D6).
   - **Hidden:** "Full Year" (D8) and the time control (1035). The declining rate shows only for Declining, as in the original.
   - **A disposed asset** shows its date sold, with its fields read-only.
4. **Wire** 6000. Add 4004 to `/checkres` and `/stress`.
5. **Tests:** saving an asset (the stored cents and `LIFELEFT`), modifying one, and the checks.

### Exit criteria

- On the fixture, E001 shows: monthly 500.00, accumulated 6,500.00, net book value 23,500.00, life 5.0 years.
- E002 shows monthly 100.00. Adding E006 (straight line, 1,200.00, life 12) shows monthly 100.00.

---

## Milestone 3 — Create Voucher (4003) and the Cancel hook

> **As built (2026-11-11; built and run on Windows 11, VS 18, v145).**
> - **Read** (task 1): spec §2.3 holds. Spec #2's `SaveLine` creates a voucher numbered `NextVoucherNumber` when asked for one that doesn't exist, and doesn't open a transaction itself, so the run uses it. **cb4sqlite transactions don't nest**, so `SaveAsset` is now `writeAsset` (no transaction) wrapped in one, and the run calls `writeAsset`.
> - **`Assets.h`** (task 2):
>   - **`PlanRun`** lists the assets due (amount, and the monthly amount for `CURR_DEP`) and those skipped, with the reason: disposed; fully depreciated; type missing; **type's expense or accumulated account not in the chart**; or **bought after the period's end** (D8). The last two are new, and the design is updated.
>   - **`alreadyRun`** is set when the period has a `gl_fadep` row whose voucher exists, or one with `BATCHNO` 0 (the fixture's period 6).
>   - **`RunDepreciation`** works in one transaction: one open voucher ("Monthly Depreciation Voucher", the period's end date, lines Dr expense / Cr accumulated per asset, numbered 1, 2, …, the asset code as the reference), then each asset's `ACCUM_DEP`, `LIFELEFT` and `CURR_DEP`, and a `gl_fadep` row each.
>   - **`UndoRun`** puts the assets back and deletes and packs the rows. It returns 0 without `gl_fadep`.
> - **The Cancel hook** (task 3): `CancelVoucher` calls `UndoRun(code, number)` inside its transaction. One line in `Vouchers.cpp`; `Vouchers.h`'s comment says so.
> - **`gl/port/DepreciationRun.{h,cpp}`** on 4003 (task 4):
>   - **Fields:** the current period (the lowest open one), read-only, and in 4095 what OK will do ("2 assets due: 600.00. 2 skipped.", or "Period 7 has been run: voucher 104.").
>   - **OK** asks (8110), listing the assets and the skips, then runs and reports the voucher. After a refusal, it names the voucher to cancel. The caption is spelled right.
> - **Wire** (task 5): 6003 (`ID_FASLIP`); `/checkres` loads 4003; `/stress` opens it too.
> - **Tests** (task 6), 5 cases:
>   - the plan for period 7 (E001 500.00, E002 100.00, E004 disposed, E005 bad account; an E008 of a missing type; an E009 bought after the period);
>   - the run (voucher 104, dated 2026-10-31, 4 lines, 600.00 each way; E001 7,000.00 and 14 months; E002 200.00 and 2; the history row), and a second run refused;
>   - cancelling an ordinary voucher (101: the assets untouched), then the run's voucher (undone), then running again;
>   - posting (161000 7,200.00 Cr, 761000 1,200.00 Dr, matching the register; a posted voucher can't be cancelled);
>   - period 13 without an end date; a year end plus the new year's dates, then E002's first run at 96.00; nothing due.
>
>   `gl_tests`: 107 cases.
> - **Checked:** Debug and Release clean; every suite, `/checkres`, `/stress` and the self-test pass. Driven in the app:
>   - Create Voucher shows "2 assets due: 600.00. 2 skipped.", asks with the list, creates voucher 104 and closes.
>   - Opened again: "Period 7 has been run: voucher 104.", and OK says to cancel it first.
>   - Cancel Voucher lists 4 vouchers; End, Enter, Yes cancels 104.
>   - Create Voucher again: due again, created as 104.
>   - The database then holds E001 7,000.00 / 14, E002 200.00 / 2, one history row per asset per period, and voucher 104 open, 600.00 each way.
> - **Q2, Q5, Q6** are still the spec's assumptions: declining balance, the period-end date, depreciation from the first run after purchase. **Found:** the run depends on the new year's period dates being set after year end (design §4.2, R5).

**Goal:** a month's depreciation becomes one open voucher, which can be posted or undone.

### Tasks

1. **Read** `TGLCreateDeprSlip` and dialog 4003, and Spec #2's `NextVoucherNumber`, `SaveLine`, `UpdateTotals`, `OpenPeriods` and `CancelVoucher` (`Vouchers.cpp`).
2. **`Assets.h`:**
   - `PlanRun(code, period)`: the assets due, their amounts, and the ones skipped and why: disposed, fully depreciated, or type missing (8120). It changes nothing; the dialog shows it before OK.
   - `RunDepreciation(code, period)`: spec §4.2 in one `cb4sqlite::Transaction`.
     - It refuses with the voucher number when the period has a run whose voucher exists (D5).
     - It writes the voucher and lines through Spec #2's functions (D10), then `ACCUM_DEP`, `LIFELEFT` and the `gl_fadep` rows.
   - `UndoRun(code, batchno)`: spec §4.3.
3. **The Cancel hook:** Spec #2's `CancelVoucher` calls `UndoRun` inside its transaction when `gl_fadep` has rows for the voucher. Ordinary vouchers are unaffected, and that has a test. `Vouchers.cpp` gains only that call; the logic stays in `Assets.cpp` (spec R3).
4. **`gl/port/DepreciationRun.{h,cpp}`**, on dialog 4003 (added to `prune_rc.py`):
   - it shows the current period and `PlanRun`'s count, total and skipped assets;
   - OK asks (8110) and runs;
   - the message names the voucher, or the reason nothing was created;
   - the caption is "Create Current Month Depreciation Voucher".
5. **Wire** 6003; `/checkres`.
6. **Tests:**
   - the fixture's period-7 run: one voucher dated 2026-10-31, four lines, 600.00 each way, totals set, `NextVoucherNumber`'s number;
   - E001 at 7,000.00 and 14 months; E002 at 200.00 and 2; E004 and E005 skipped (E005 for its type);
   - a second run refused; Cancel, then a run again;
   - cancelling an ordinary voucher (101) leaves the assets alone;
   - posting the run's voucher, then the ledger matching the register (161000 7,200.00);
   - a year end after the period-7 run, then E002's first run of the new year: (5,000.00 − 200.00) × 24 % ÷ 12 = 96.00.

### Exit criteria

- The fixture's run, post and cancel driven in the app.
- Q2, Q5 and Q6: either answered, or the spec's assumption recorded in the As-built note as still assumed.

---

## Milestone 4 — Disposal (1001)

> **As built (2026-11-11; built and run on Windows 11, VS 18, v145).**
> - **Read** (task 1): spec §2.4 #5 holds. The template's 6031 ("Sale Price") is Add/Modify's cost and 6037 ("Gain/Loss on Sale") its monthly amount. The code (6019) and 6031 are read-only (`ES_STATIC`), and there are no fields for cost, accumulated depreciation or book value.
> - **`prune_rc.py`** (task 2): 1001 added.
> - **`gl/port/FixedAssetDisposal.{h,cpp}`** (task 3):
>   - **Fields:** Enter on the code shows the asset (and "There is no fixed asset X" otherwise). The fields show life, months used (the "Rmd" label becomes "Used", widened) and salvage. Sale Price is typed (made editable); Gain/Loss is computed as you type. Date Disposed defaults to today.
>   - **The dialog grows 22 units in code**: a summary line ("Cost … Accumulated depreciation … Book value …") and a Delete button, IDs `IDC_FA_SUMMARY` and `IDC_FA_DELETE` in `PortIds.h`.
>   - **Dispose** (ID 1, relabelled) asks (8107), disposes, and lists the Voucher Entry lines to make.
>   - **Delete** removes only a never-depreciated asset.
>   - A disposed asset shows "- disposed <date>" in the caption and can't be disposed of again.
>   - Enter in Sale Price or Date Disposed moves on; it doesn't dispose.
> - **`Assets.h`** (task 4): `GainOnSale`, `DisposeAsset` (`NotFound` / `AlreadyDisposed`), `DeleteAsset` (`HasHistory`, else delete and pack).
> - **Wire** (task 5): 6035 (`ID_DISPOSAL`); `/checkres` loads 1001; `/stress` opens it.
> - **Tests** (task 6), 2 cases:
>   - E002 after the period-7 run: book value 4,800.00, a loss of 800.00 at 4,000.00 and a gain of 200.00 at 5,000.00. Disposed with its date, the rest of the record kept; refused a second time. Period 8's plan skips it.
>   - Deleting E001 is refused (history); deleting E005 works and leaves the others.
>
>   `gl_tests`: 109 cases.
> - **Checked:** Debug and Release clean; every suite, `/checkres`, `/stress` and the self-test pass. Driven in the app, after the period-7 run:
>   - E002: cost 5,000.00, accumulated 200.00, book value 4,800.00. Sale 4,000.00: −800.00.
>   - Dispose: the entry (Dr 161000 200.00, Dr proceeds 4,000.00, Dr loss 800.00, Cr 151000 5,000.00); the caption shows 10/31/2026; disposing again is refused.
>   - Delete E001: refused, "use Dispose". Delete E005: deleted.
>   - A screenshot checked the added line and button.
> - **Q3** is still the spec's assumption: nothing is posted.

**Goal:** an asset can be marked disposed, with its gain or loss shown; an asset entered by mistake can be deleted.

### Tasks

1. **Read** `TGLFixedDelete` and dialog 1001. Note which of the original's field IDs show what (spec §2.4 #5).
2. **`prune_rc.py`:** add 1001; regenerate.
3. **`gl/port/FixedAssetDisposal.{h,cpp}`:**
   - **Enter** on the code shows the asset.
   - **The dialog's fields**, relabelled in `OnInitDialog`:
     - cost, accumulated and net book value shown;
     - Sale Price typed;
     - Gain/Loss computed as sale price − net book value;
     - Date Disposed defaults to today.
   - **Dispose** (the original's Delete button, ID 1) asks (8107), then calls `DisposeAsset`. The message gives the amounts for the entry to make in Voucher Entry (D7).
   - **Delete** (a new button, added in code) works only for an asset with no `gl_fadep` rows.
4. **`Assets.h`:** `DisposeAsset(code, facode, date)` and `DeleteAsset(code, facode)` (refused with history).
5. **Wire** 6035; `/checkres`, `/stress`.
6. **Tests:**
   - dispose E002 on 2026-10-31 after the period-7 run: net book value 4,800.00; at a sale price of 4,000.00, a loss of 800.00;
   - the next period's `PlanRun` skips E002;
   - deleting E001 is refused; deleting a new asset works.

### Exit criteria

- Driven in the app on the fixture.
- Q3: if the finance team asks for a disposal entry, stop and change the spec first (it needs accounts no type has).

---

## Milestone 5 — Reports, report list, clean-up

> **As built (2026-11-11; built and run on Windows 11, VS 18, v145).**
> - **Read** (task 1): spec §7 holds.
>   - **`GL_FA.RPT`**, "General Ledger Fixed Asset Summary": Fixed Asset Code, Date Purchased, Date Sold, Reference and Description, Pd. Used, Asset Life, Historical Cost, Salvage Value, and its own formulas for the monthly and accumulated amounts. Current period; "Report File:" footer.
>   - **`GL_FATYP.RPT`**, "General Leger Fixed Asset Types": the type, its description, and each account with dept and description. "Report Name:" footer.
>   - `gl_xls.cpp` ran them by name. `ID_FINFA` / `ID_FINDEP` have no value in the headers, so the port names 1019 and 1020 in `ManilaGL.cpp`.
> - **`gl/port/reports/AssetReports.{h,cpp}`** (task 2):
>   - **1019** (landscape):
>     - grouped by type; each asset with its reference under it;
>     - disposed assets marked "(disposed)" with their date sold, listed but not totalled (a note says so);
>     - net book value added; totals per type and overall.
>   - **1019's ledger check (D11)** is per account: the register's cost and accumulated depreciation of the assets in use, against each account's balance at the current period (accumulated is a credit). Each account gets a line: agrees, differs by …, or not in the chart (naming the types). Then a hint on what a difference means. Found while testing: an account used for both cost and accumulated depreciation (type XX) named its type twice; fixed.
>   - **1020:** each type's three accounts; a count.
>   - **Column widths** were set from PDFs of the fixture: the first print clipped "Fixed Asset", "Months", "(Months)", "Depreciation", and a 1020 description.
> - **Wire** (task 3): 1019 and 1020 run the reports; `kFixedAssetReports` is gone. Both join the report list (now 18 reports; the Spec #3 sheet notes it) and `/stress`'s builders. The report run functions create the asset tables if missing.
> - **Tests** (task 4): `gl_tests/test_reports_assets.cpp`, 3 cases:
>   - 1019's groups, rows, totals (37,000.00 / 683.33 / 6,600.00 / 30,400.00 for the 3 assets in use) and the disposed note;
>   - the check: agrees on the fixture; 161000 differs by 600.00 after an unposted run; 999999 not in the chart;
>   - 1020's rows.
>
>   `gl_tests`: 112 cases.
> - **README** (task 5): the Fixed Assets bullets (from M1–M4), the monthly cycle, and both reports in the Reports table.
> - **Checked** (task 6): Debug and Release clean; every suite, `/checkres`, `/stress` and the self-test pass. Both reports printed to PDF from the app and checked.
> - **Not done:** spec R5's automatic check, refusing a run whose period ends before the last run's (the new year's dates not set). It stays a documented step (README, design §4.2); file it if the walkthrough shows it's needed.

**Goal:** 1019 and 1020 print; the module is complete in the menus, the checks and the README.

### Tasks

1. **Read** `GL_FA.RPT` and `GL_FATYP.RPT` (titles, columns, grouping, footers) and `gl_xls.cpp:467-482`.
2. **`gl/port/reports/AssetReports.{h,cpp}`**, portable:
   - 1019 as spec §7: landscape, grouped by type, with totals and the ledger check per account (D11);
   - 1020;
   - the footers name the `.RPT`; the title spelling is fixed (D12).
3. **Wire:**
   - 1019 and 1020 leave `kFixedAssetReports` (Spec #3 S3-M7), which then goes;
   - both join `ReportList`'s entries (dialog 1022);
   - `/stress` opens both previews.
4. **Tests:**
   - 1019's rows, type totals and check on the fixture (agrees);
   - after an unposted run (differs by 600.00 on 161000);
   - with E005's 999999 named;
   - 1020's rows.
5. **README:** a Fixed Assets section (the four screens, the monthly cycle: run, post, cancel to undo; disposal; the two reports). Fixed Assets also joins the Reports table.
6. **Print** both to PDF from the fixture and check them, as in Spec #3 (column widths).

### Exit criteria

- Debug and Release `/W4 /WX` clean.
- Every suite, `/checkres`, `/stress` and the self-test pass.
- The two PDFs checked.

---

## Milestone 6 — Acceptance

1. **`gl/docs/superpowers/specs/2026-11-11-spec4-acceptance.md`**, in the Spec #3 sheet's shape, with spec §10's checks, the fixture's expected figures and the `sqlite3` queries that produce them.
2. Release build check, the walkthrough with a witness, triage, sign-off: on Windows.

---

## Cross-cutting

- **Per-port checklist** from the Spec #1 plan for every ported file. Each screen's header lists its translation notes, naming the decisions it applies.
- **Per-milestone gates:**
  - `/W4 /WX` clean on Debug and Release x64;
  - the milestone's tests pass;
  - a spec drift check;
  - an entry in `gl/docs/superpowers/plans/2026-05-10-progress.md`;
  - the milestone's "As built" note here.
- **What not to do:**
  - Don't post to the ledger directly: the run writes an open voucher, and Spec #2's posting posts it.
  - Don't change the original tables' layouts: the port's history goes in `gl_fadep`.
  - Don't edit `gl/res/gl.rc` by hand: change `prune_rc.py` or `port.rc`.
  - Don't run anything against the real company's `GL_FA` until Q4 is answered (spec R1).

## Risks specific to the plan

| # | Severity | Risk | Mitigation |
|---|---|---|---|
| P1 | Medium | The finance team's answers to Q2, Q3, Q5 or Q6 arrive after M3 or M4. | Each answer is one function in `Assets.cpp`, with its test; M3 and M4's exit criteria record what is still assumed. |
| P2 | Medium | The Cancel hook changes Spec #2 behaviour. | One call, in Cancel's transaction; a test cancels an ordinary voucher; the Spec #2 sheet's Cancel check is re-run in M6. |
| P3 | Low | The fixture's new tables shift a Spec #1–#3 figure. | M0 task 3 compares the three sheets' queries before and after. |

## How to use this plan

M0 → M1 → M2, then M3 with M4 alongside, then M5 and M6. If the plan and the spec conflict, stop, edit the spec, recheck the plan, then continue.
