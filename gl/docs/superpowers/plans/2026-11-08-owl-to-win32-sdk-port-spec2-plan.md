# Spec #2 — Implementation Plan: the General Ledger Voucher Cycle

**Spec reference:** `gl/docs/superpowers/specs/2026-11-08-owl-to-win32-sdk-port-spec2-design.md`
**Date:** 2026-11-08
**Plan status:** Draft

**Paths:** relative to the repository root. The legacy sources are in `gl/src/` and `gl/header/`, shared headers in `include/`, and new GL code in `gl/port/` (Spec #1 §2.1).

This plan carries out the Spec #2 design. If a milestone finds that the spec is wrong, **stop, fix the spec, then resume**. Don't silently diverge.

**The lesson from Spec #1.** The M6 tasks were drafted before the original code was read, and much of them turned out wrong: the dialog, the buttons, the delete semantics, the menu merge. So every port milestone here starts with a **read task**. Read the original handler and every function it calls, write down what it actually does in the milestone's "As built" note, and fix the spec where it differs. The screen tasks below are only as precise as the spec's §2. Where the original disagrees, the original wins (except for the recorded deviations, spec §6).

---

## Milestone summary

| # | Milestone | Size | Depends on | Critical path? |
|---|---|---|---|---|
| 0 | `fx::Grid` (`tbSpread`); `AccountList`, User Access menus (1160) and the Cash list (1011) on it; Linux tools into the repo | M | none | yes |
| 1 | `cb4sqlite`: `querySet` subset, transactions, `backupTo`; correct `gl_dist` header; `gl_tests` project; fixture | M | none | yes, in parallel with M0 |
| 2 | Master data: Department Codes, Medical Reps, Product Codes, Doctor Codes | S | M0 (resources only) | no |
| 3 | `Vouchers.cpp` core and tests; Voucher Entry (1013) and Prior Period Entry | L | M0, M1, M2 | yes |
| 4 | Post (1061), Recover (1015), Cancel (1043) | M | M3 | yes |
| 5 | Transaction Inquiry (1029), Budget Data (1017) | M | M0, M1 | no; in parallel with M3/M4 |
| 6 | Year End, Backup and Restore, Reindex message | S | M1 | no |
| 7 | Acceptance walkthrough and sign-off | S | all | yes |

S is less than a day, M is 1–3 days, L is 3–5 days, for one engineer.

**Parallel work:** M0 and M1 share nothing. M5 and M6 need only M0 and M1, so they can run alongside M3 and M4.

---

## Milestone 0 — `fx::Grid` and the Spec #1 grid gaps

**Goal:** a `tbSpread` window class in `win32fx` that the original dialog templates load unchanged. `AccountList` and the two grid-dependent Spec #1 screens run on it.

> **As built (2026-11-09; checked on Linux only, Windows not yet built or run).**
> - **Tools** (task 1), in `tools/linux/` with a README:
>   - `wincheck.sh` and `winhdr/`; `wincheck_all.sh`, which checks every gl.exe, win32fx and trivial_dialog source;
>   - `genproj.py`, which now takes project names (`genproj.py . gl win32fx`). Without names it would also rewrite `cb4sqlite.vcxproj`, which was retargeted to PlatformToolset v145 by hand while every other project is v143. gl's file list is now every `.h` / `.cpp` in `gl/port/` and `gl/port/stubs/`, so a new port file needs only a rerun. It reproduces the committed `gl.vcxproj` byte for byte, and `win32fx.vcxproj` with `Grid.h` / `Grid.cpp` added;
>   
>   Not brought in: the Spec #1 sanitizer build script, which no longer builds the current sources (they call `sscanf_s` and `_dupenv_s`), and the fake `<Windows.h>` and M6 harness. S2-M1's `gl_tests` replaces the harness and brings the Linux test build.
> - **Read findings** (task 2): spec §4.1 and §12 Q4. `SSM_DBLCLK` is the only grid notification the GL code handles. Colours follow one convention (green on blue marks the picked row). 1160's on/off cells are two-state picture buttons (`PLUS` / `MINUS`).
> - **`fx::Grid`** (tasks 3–4): `win32fx/include/controls/Grid.h`, `win32fx/src/controls/Grid.cpp`, registered by `RegisterCustomControls`; `GRN_*` codes in `CustomControls.h`. The API differs from the draft in spec §4.2 as recorded there: `SetColor(col, row)` instead of `SetRowColor`, and `SetStretchColumn`, `SetImageList` / `SetCellImage`, `ActiveRow` / `ActiveCol` / `SetActiveCell`, `SetRedraw`, `Grid::Create`, `GRN_KEYDOWN` and `GRN_RCLICK` added.
> - **Self-test** (task 5): `trivial_dialog` has a grid; `/selftest` adds 24 grid checks.
> - **`AccountList`** (task 6) is on `fx::Grid`; its ListView code is gone.
> - **1160** (task 7): `AccessMenus` in `gl/port/Login.cpp`. Two defects of the original are corrected (spec D9):
>   - it starts from the menu code of the level shown in 1159, resolved as login does (`LevelMenuCode`), not from the global `szMsg`, the **logged-in** user's string;
>   - only OK saves. The original saved after Close too: it added a level `accessl` didn't have yet, or asked with string 6096 to change an existing one if any row was toggled.
>
>   Editing no longer touches the session's `szMsg`, so Reset (string 8152) now applies the selected level's code to the session. The original's Reset wrote `szMsg` into the level, which with the fix would have overwritten it with the logged-in user's menus.
> - **1011** (task 8): `CashAccountsList` in `gl/port/CompanyProfile.cpp`, a modeless dialog owned by 1034 and placed beside it. Its caption is string 6027, "Account Payable Bank Accounts", as in the original, although the list is GL cash accounts: a candidate for D3. A double-click or Enter on a row shows that account in 1034 (CM_SETDATA). The list is redrawn after Add/Mod and Delete, as the original did; the Spec #1 port had dropped those calls along with the list.
> - **`/checkres`** (task 9) creates 1011 and 1160 and checks that each holds a grid.

### Tasks

1. **Bring the Linux checking tools into the repository** at `tools/linux/`, with a README. They are currently in a session scratch directory and would be lost with it:
   - `wincheck.sh`, the syntax and type check against the MinGW headers, plus its `winhdr/` overrides;
   - the fake `<Windows.h>` used by the portable harness builds;
   - the `.vcxproj` generator (`genproj.py`).

   This is development tooling only; nothing in `gl.sln` uses it.
2. **Read the grid use.** Every `SS*` call in `gl_acctl.cpp`, `gl_btche.cpp` (the voucher and line lists), `gl_postb.cpp`, `gl_postr.cpp`, `gl_btcan.cpp`, `gl_delrq.cpp`, `gl_trans.cpp`, `gl_budgt.cpp`, `gl_acces.cpp` (TAccessMenus, 1160) and the 1011 Cash list. Record in spec §4:
   - what each `SSSetBool` option does, and its ListView equivalent or "ignored" (spec §12 Q4);
   - which notifications each screen handles (`SSM_DBLCLK`, `SSM_CLICK`, …);
   - what `SSSetColor` colours.
3. **`win32fx/include/controls/Grid.h`, `win32fx/src/controls/Grid.cpp`** (spec §4.2):
   - Register `tbSpread` in `RegisterCustomControls`. It creates a child `WC_LISTVIEW` (`LVS_REPORT | LVS_SHOWSELALWAYS | LVS_SINGLESEL`, `LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES`) that fills the control.
   - API, 1-based like `SS*`: `SetColumns`, `SetMaxRows`, `Clear`, `SetData`, `GetData`, `SetRowColor` (`NM_CUSTOMDRAW`), `SelectedRow`, `SelectRow`.
   - Notifications to the parent: `WM_COMMAND` with `GRN_PICK` (double-click or Enter) and `GRN_SELCHANGE`, defined in `CustomControls.h`.
   - The class works both from a dialog template and from `CreateWindowExW`, for the windows the original made in code.
4. **Editable float column.**
   - `SetColumnEditable(col, EditKind::Float)`. Enter, F2 or a double-click on the cell places a `tbFloat` edit over it.
   - Enter, Tab or losing focus commits the value and sends `GRN_CELLCHANGED`. Esc cancels.
   - Up and Down commit, then move to the next row and keep editing, as a spreadsheet would.
5. **`trivial_dialog /selftest`:** add a grid to the example dialog. Test columns, data round-trip, pick notification, row colour, and an edited float cell committed by Enter and discarded by Esc.
6. **Move `AccountList` onto `fx::Grid`.** Its behaviour is unchanged (Spec #1 §6.7 #7), and the ListView code leaves `AccountList.cpp`.
7. **User Access Menu button (1160, `TAccessMenus`).** Read `gl_acces.cpp`, port the dialog onto the grid, and replace the message in `Login.cpp` `UserAdministration::Menus`.
8. **The Cash Accounts list (1011, `TAPCashAcctsList`)** beside Company Profile > Accounts > Cash (1034). Port it, and wire up `Redraw` / `Search` in `CompanyProfile.cpp` (currently `ID_SEARCHCASH` is a no-op).
9. **`ResourceCheck.cpp`:** set `needsSpread` to false for 1011 and 1160, and have `/checkres` create them.

### Exit criteria

- `trivial_dialog /selftest /quiet` exits 0, including the new grid checks.
- The account list passes Spec #1 §6.7 #7 again.
- The Menu button edits and saves a user's menu access to `accessl`.
- The Cash list shows and picks cash accounts.
- `/checkres /quiet` exits 0 with 1011 and 1160 loaded.
- `/stress /quiet` exits 0.

---

## Milestone 1 — Data layer, tests and fixture

**Goal:** the shim supports everything the cycle needs, a test project exists for the portable GL code, and the fixture has vouchers.

> **As built (2026-11-09).** Where the tasks below differ, this note wins.
> - **Comparison rules (task 1).** The tree has no CodeBase source or documentation, only operator codes in `include/e4expr.h`. The rules are the ones `dbf2sqlite_core/src/KeyExpr.cpp` already uses for the real tags' FOR conditions: dBase `SET EXACT OFF`, a left match on the padded value, case-sensitive. They are recorded in spec §5.1, and confirming them against real CodeBase is spec §12 Q5.
> - **Shim:**
>   - `cb4sqlite/src/QueryExpr.cpp`: the grammar, with literals as named parameters `:qN`.
>   - `Data4`'s internal filter is now `detail::QueryFilter {sql, params}`.
>   - `Code4::begin/commit/rollback/inTransaction`, `cb4sqlite::Transaction` and `Code4::backupTo` (spec §5.2–5.3 "As built").
>   - `notImplemented` no longer says "in spec #1".
> - **Tests:** `cb4sqlite_tests` gained `test_queryexpr.cpp`, `test_transaction.cpp` and `test_backup.cpp`: 69 test cases and 1,374 assertions in all, up from 50 and 981.
> - **Headers (task 6).** `gl/port/db_gl_dist.h` and `db_gl_distb.h` have default tag `batchno`, not the generator's `ardist`. The checks of the other headers found real mismatches in `db_btch.h`, `db_btchd.h`, `db_drcod.h` and `db_itemm.h` (spec §5.4 table). **S2-M3 needs correct GL headers for `gl_btch` and `gl_btchd`, and S2-M2 one for `gl_drcod`.** The name clash in task 6 doesn't arise: `gl_distl.h` only forward-declares `Tglbtchd`, and `gl_clsm.h` is legacy-only.
> - **`gl_tests`** (task 7):
>   - `gl_tests/` holds `GlTestDb.h`, which migrates the dev fixture once per run with `dbf2sqlite_core` and gives each test case its own copy.
>   - `Stubs.cpp` supplies `szUser` and an `UpdateUserTime` that refuses tables with update/user/time fields.
>   - `test_accounts.cpp` has the 48 M6 harness checks; `test_tables.cpp` checks the headers and the fixture's vouchers. That is 8 test cases and 139 assertions.
>   - It is `{A0C1C001-0007-…}` in `gl.sln`.
> - **Linux build:** `tools/linux/build_tests.sh [--run] [out]` builds `cb4sqlite_tests`, `gl_tests` and `dbf2sqlite` with ASan/UBSan. The MSVC-only `sscanf_s` / `_dupenv_s` come from `tools/linux/msvc_compat.h` via `-include`. It replaces both the Spec #1 sanitizer script and the planned `build_gl_tests.sh`.
> - **Fixture (task 8):** spec §7 "As built". 101000 now has −250 of period-7 activity, the other side of posted voucher 100.

### Tasks

1. **Read CodeBase's comparison rules** in `include/e4expr.h`, `include/d4data.hpp` and any CodeBase 4 documentation in the tree. Record in spec §5.1:
   - how string `=` / `<>` compare (left match? trailing blanks?);
   - whether field names are case-insensitive;
   - which quote characters are allowed.
2. **`querySet` subset** (spec §5.1), in `cb4sqlite/src/Relate4set.cpp` with a small recursive-descent parser in `cb4sqlite/src/QueryExpr.cpp`:
   - The query becomes `WHERE` SQL with bound parameters. An unknown field throws `Error` naming it.
   - Anything outside the grammar still throws `notImplemented`.
3. **Transactions** (spec §5.2):
   - `Code4::begin()` / `commit()` / `rollback()`, `Code4::inTransaction()`, and `cb4sqlite::Transaction`, which rolls back in its destructor unless committed. Nested `begin()` throws.
   - Check that a `Data4` record-buffer write inside a transaction goes through the same connection and isn't committed on its own.
4. **`Code4::backupTo(const wchar_t* path)`** using `sqlite3_backup_init` / `_step(-1)` / `_finish`. It throws on failure and leaves no partial file behind.
5. **`cb4sqlite_tests`:** new test files `test_queryexpr.cpp`, `test_transaction.cpp` and `test_backup.cpp`, covering every rule recorded in task 1, rollback after appends, updates and deletes, and the RAII guard.
6. **Table headers** (spec §5.4):
   - Generate `db_gl_dist.h` and `db_gl_distb.h` from the real `dbf/GL_DIST.DBF` and `GL_DISTB.DBF` with `dbf2sqlite --headers-out`. Hand-check them, name the classes `Tgldist` / `Tgldistb`, and put them in `gl/port/`.
   - Leave `include/db_dist.h` alone.
   - For every header in spec §2.1 (`db_btch.h`, `db_btchd.h`, `db_dept.h`, `db_medrp.h`, `db_itemm.h`, `db_drcod.h`), compare the `Field4` names with the real `.dbf` field lists, and record any difference in spec §5.4.
   - **Name clash:** `gl/header/gl_clsm.h` and `gl/header/gl_distl.h` also declare `Tglbtch` / `Tglbtchd`. Use only the `include/` versions, and check that no ported file includes both.
7. **`gl_tests` project** (new, in `gl.sln`): Catch2, as `cb4sqlite_tests` uses. It links `cb4sqlite.lib` and the portable `gl/port` sources: `Accounts.cpp` now, `Vouchers.cpp` from M3.
   - Port the M6 harness's 48 `Accounts.cpp` checks into it as its first test cases.
   - It must also build on Linux, as the M6 harness did, so it can run under ASan/UBSan. `tools/linux/` gets the build script.
8. **Fixture** (spec §7): extend `make_dev_fixture.py` with `gl_btch`, `gl_btchd`, `gl_dist`, `gl_distb`, `gl_medrp`, `gl_itemm` and `gl_drcod`, using the real tables' layouts and tags (take both from `dbf/*.DBF` / `*.MDX`, not from the headers). Add departments 10 and 20 if missing, and `GL.INI [PERIOD] BATCHNO`. Then:
   - check that `setup.cmd` migrates the fixture with exit 0;
   - update `gl/README.md`'s fixture notes.

### Exit criteria

- `cb4sqlite_tests` and `gl_tests` pass on Debug and Release x64.
- `gl_tests` passes on Linux under ASan/UBSan.
- The fixture migrates with exit 0.
- A `GL.INI [GLACCTS] QUERY` of `ACCOUNT = '4'` lists only the 4xxxxx accounts in the account list. That closes Spec #1 gap G2.

---

## Milestone 2 — Master data

**Goal:** Department Codes, Medical Reps, Product Codes and Doctor Codes add, modify and delete records. Spec §8 #1.

### Tasks

1. **Read** `gl_deptc.cpp`, `gl_medrp.cpp`, `gl_prods.cpp` and `gl_drcod.cpp` in full. Record each screen's buttons, its navigation (these dialogs have top / bottom / next / previous buttons; 1010 didn't) and its delete rule. Note what `gl_prods.cpp` opens, since it names no table class.
2. **Resources:** add 1006, 1007, 1023 and 1040 to `prune_rc.py` `DIALOGS`, regenerate `gl/res/gl.rc`, add them to `/checkres`, and pull in any bitmaps they use.
3. **`gl/port/MasterData.h/.cpp`:** one `fx::Dialog` per screen, following the per-port checklist (Spec #1 plan, "Cross-cutting checklists"). Record operations that don't need Win32 go in a portable `MasterTables.cpp` with `gl_tests` cases.
4. **Wire the commands** in `ManilaGL.cpp`: 10061, 6042, 6005, 6040. Remove them from the "not part of Spec #1" path.

### Exit criteria

- Spec §8 #1 passes on Debug x64.
- `/checkres /quiet` exits 0.
- `/stress` also opens each dialog, and exits 0.

---

## Milestone 3 — Voucher core and Voucher Entry

**Goal:** vouchers can be created, edited and deleted. Spec §8 #2–#4.

### Tasks

1. **Read** all of `gl_btche.cpp`, `gl_btche.h`, `gl_prior.cpp` and `ap_repts.cpp` `SetPeriods`. Settle spec §12 Q2 (adjustments vs. prior-period entries) and Q3 (date vs. period checks). Record what each tool-box button does, and how the four windows are created, positioned, shown and destroyed.
2. **`gl/port/Vouchers.h/.cpp`**, portable, no Win32:
   - `struct VoucherHeader`, `struct VoucherLine`;
   - `NextVoucherNumber`, `AppendVoucher`, `ReplaceVoucher`, `DeleteVoucherLine`, `DeleteVoucher`;
   - `CheckAccount`, `CheckDept`;
   - `OpenPeriods` (from `gl_pds`).

   Each has `gl_tests` cases against a copy of the fixture database.
3. **Resources:** add 1013 and the tool-box bitmaps to `prune_rc.py`.
4. **`gl/port/VoucherLists.h/.cpp`:** the voucher list (`TGLViewBatchList`) and line list (`TGLDistList`) windows. Both are owned, captioned windows holding an `fx::Grid`, positioned the way `AccountList` is.
5. **`gl/port/VoucherEntry.h/.cpp`:**
   - the dialog, plus the floating tool box (`TGLToolBox`) as an owned tool window of `tbSuperBtn`s;
   - the period combo, showing open periods only;
   - the medical rep, product and doctor combos, each with a blank entry;
   - picking a line in the line list loads it, and picking a voucher in the voucher list loads it;
   - Add, New, Delete, Cancel, and Top / Bottom / Next / Previous.
6. **Prior Period Entry:** the same dialog with a mode flag, carrying the original's restriction message.
7. **Wire the commands:** 10076 and `ID_BATCHENT` (1007); the toolbar's `ID_GLBATCH` (already maps to `ID_MODBATCH`); GLSUB's General Ledger Voucher; 8081.
8. **`/stress`:** add voucher entry, opening and closing it with all four windows.

### Exit criteria

- Spec §8 #2–#4 pass on Debug x64.
- `gl_tests` passes on Windows and on Linux under ASan/UBSan.
- `/stress /quiet` exits 0, with no Debug leaks.

---

## Milestone 4 — Post, Recover, Cancel

**Goal:** the cycle completes. Spec §8 #5–#8.

### Tasks

1. **Read** all of `gl_postb.cpp` (`Ok`, `PostGLSlip`, `PostAdjustment`, `UpdateGL`, the single-voucher pick), `gl_postr.cpp`, `gl_btcan.cpp` and `gl_delrq.cpp`. Settle spec §12 Q1 (Recover and `curr_bal`; what remains in `gl_btch`) and spec risk R3 (the debit/credit comparison).
2. **`Vouchers.cpp`:**
   - `PostAll`, `PostVoucher`, `RecoverVoucher`, `CancelVoucher`. Each runs inside one `cb4sqlite::Transaction` (D1) and returns a result listing the vouchers posted and the vouchers refused, with the reason.
   - After a rollback the caller reopens the tables (spec §5.2).
   - `gl_tests` covers: balanced, out of balance, adjustment, single pick, recover returning the books to their values before posting, cancel, and a forced failure that leaves every table unchanged and then a retry that succeeds.
3. **Fault injection for spec §8 #6:** `gl.exe /failpost <n>` in Debug and Release. It makes the nth line write throw inside `PostAll`. It is documented in `gl/README.md` under Checks.
4. **Resources:** 1061, 1015 and 1043, with the corrected captions (D3) set in `OnInitDialog`, so `gl.rc` stays generated.
5. **`gl/port/PostVouchers.cpp`, `RecoverVoucher.cpp`, `CancelVoucher.cpp`:** each dialog shows its vouchers in an `fx::Grid`. OK acts on all of them, or a picked row acts on one, as the original does. Messages use strings 8115 and 8116.
6. **Wire the commands:** 10121 (also the toolbar's `ID_GLPOST` and GLSUB), 10122, 3054.

### Exit criteria

- Spec §8 #5–#8 pass on Debug x64.
- `gl_tests` passes, including the rollback tests.
- `/stress /quiet` exits 0.

---

## Milestone 5 — Transaction Inquiry and Budget Data

**Goal:** spec §8 #9–#10.

### Tasks

1. **Read** `gl_trans.cpp` and `gl_budgt.cpp`, plus `gl_budgi.cpp` to rule it in or out: it may be the budget inquiry report, 10073, which belongs to Spec #3.
2. **Resources:** 1029 and 1017, with 1017's caption corrected (D3).
3. **`gl/port/TransactionInquiry.cpp`:** the account and dept fields, then the posted lines from the query in spec §2.6, shown in the grid.
4. **`gl/port/BudgetData.cpp`:** 12 rows in the grid with the amount column editable (M0 task 4). Save writes `budget1`–`budget12`.
5. **Wire the commands:** 10072, 10074.

### Exit criteria

- Spec §8 #9–#10 pass on Debug x64.
- `/stress /quiet` exits 0.

---

## Milestone 6 — Year End, Backup and Restore, Reindex

**Goal:** spec §8 #11–#12, plus D6.

### Tasks

1. **`Vouchers.cpp` `YearEnd`**, in one transaction:
   - corrected test (D2): the total goes to the Retained Earnings account;
   - it refuses unless exactly one account has type `R`, and returns the accounts it found;
   - `gl_tests` cases for zero, one and two `R` accounts, a check that the sum of `curr_bal` is unchanged, and a check that the periods roll into the previous-year columns.
2. **The Year End command (1068):** confirm with string 8111. On refusal, show a new message naming the type-R accounts and pointing to the Chart of Accounts.
3. **`gl/port/Backup.cpp`** (D5):
   - **Backup:** a Save As dialog with the default name `<db>-<yyyymmdd>.db`, then `backupTo`.
   - **Restore:** an Open dialog, then:
     1. check the chosen file in a separate `Code4`;
     2. confirm with string 1074;
     3. `cb.close()`;
     4. rename the current file to `.bak`, replacing an older `.bak`;
     5. copy the chosen file over it;
     6. `cb.open()`, then refresh the module bar.

     If any step fails after step 3, put the `.bak` back and reopen.
   - Wire the commands 1072 and 1074.
4. **Reindex (3047):** a message saying a migrated company database needs no reindexing.

### Exit criteria

- Spec §8 #11–#12 pass on Debug x64.
- `gl_tests` passes.
- Restoring a corrupt or foreign file is refused, and `gl.exe` keeps running on the original database.

---

## Milestone 7 — Acceptance walkthrough and sign-off

**Goal:** all 14 spec §8 criteria signed off.

### Tasks

1. Build Release x64 and confirm the hardening options, as in Spec #1 M7.
2. Write `gl/docs/superpowers/specs/2026-11-08-spec2-acceptance.md` in the shape of the Spec #1 sheet: run details, the automated checks, a fresh-machine install, a checklist per criterion against the fixture, known gaps, triage and sign-off.
3. Walk through it with a witness on the fresh machine.
4. Triage the failures: fix them in Spec #2, or file them for Spec #3–#6.

### Exit criteria

- The sign-off file exists, with all 14 criteria signed off or accepted with a recorded gap.

---

## Cross-cutting

- **Per-port checklist.** Use the Spec #1 plan's checklist for every ported file. Step 8, the `SetupWindow` / `CleanupWindow` lifecycle scan, matters most for voucher entry's four windows.
- **Per-milestone gates.** As in Spec #1:
  - `/W4 /WX` clean on Debug and Release x64;
  - the milestone's tests pass;
  - spec drift check;
  - an entry in `gl/docs/superpowers/plans/2026-05-10-progress.md`, under a "Spec #2" heading.
- **Write the "As built" notes** into each milestone section of this plan when it finishes, as Spec #1 M6 did. Write corrections into the spec ("*Corrected in S2-M3:* …").
- **What not to do:**
  - Don't port reports, Fixed Assets, AP or AR work. Their commands stay stubs.
  - Don't use transactions anywhere except D1's operations.
  - Don't extend the `querySet` grammar beyond what a Spec #2 call site needs.
  - Don't edit `gl/res/gl.rc` by hand. Change `prune_rc.py` and regenerate.

## Risks specific to the plan

| # | Severity | Risk | Mitigation |
|---|---|---|---|
| Q1 | High | Voucher entry is much bigger than anything in Spec #1, and the read task may reveal behaviour the spec doesn't describe. | M3 task 1 updates the spec before any code. If the screen turns out larger than L, split M3 into the core plus lists, then the dialog. |
| Q2 | Medium | The in-place grid edit is new UI code with focus and keyboard edge cases. | It has its own self-test (M0 task 5). Budget Data is the only user in Spec #2. |
| Q3 | Medium | Restore replaces the live database file. A bug there loses data. | Keep the `.bak` and roll back on any failure. Check the file in a separate `Code4` first. The M6 exit criteria include the refusal cases. |
| Q4 | Low | `gl_tests` builds on two platforms, and the Linux build depends on tools that were never in the repository. | M0 task 1 brings them in first. |

## How to use this plan

Run M0 and M1 in parallel, then M2 → M3 → M4, with M5 and M6 alongside, and finish with M7. Treat the gates as a checklist. If the plan and the spec conflict, stop, edit the spec, recheck the plan, then continue.
