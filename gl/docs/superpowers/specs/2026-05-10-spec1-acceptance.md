# Spec #1 acceptance sign-off

The record of plan Milestone 7 (`gl/docs/superpowers/plans/2026-05-10-owl-to-win32-sdk-port-spec1-plan.md`, "Milestone 7"). It covers the 9 manual checks in spec §6.7 (`2026-05-10-owl-to-win32-sdk-port-design.md`).

Fill in every blank (`____`) and set each **Result** to `PASS`, `FAIL` or `PASS with known gap`. A `FAIL` needs a triage line in [Gaps and triage](#gaps-and-triage): either fixed in Spec #1, or filed against a later spec.

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
2. Confirm the hardening options from `common.props` are active. Turn on verbose output (**Tools > Options > Projects and Solutions > Build and Run > MSBuild project build output verbosity: Detailed**) and look for these options:
   - compiler: `/sdl`, `/guard:cf`;
   - linker: `/DYNAMICBASE`, `/NXCOMPAT`, `/GUARD:CF`.

   Or check the binary from a Developer Command Prompt with `dumpbin /headers x64\Release\gl.exe`. Under "DLL characteristics" it should list "Dynamic base", "NX compatible" and "Control Flow Guard".

| Check | Result | Notes |
|---|---|---|
| Release build, no warnings | ____ | |
| `/sdl` and `/guard:cf` (compiler) | ____ | |
| `/DYNAMICBASE`, `/NXCOMPAT`, `/GUARD:CF` (linker) | ____ | |

### P2. Automated checks (dev machine)

Use `setup.cmd` for each configuration first (`gl\test\dev-fixture\setup.cmd Debug`, then `Release`). From `cmd`, run each check with `start /wait ... /quiet`, then `echo %errorlevel%`. Each one should print 0.

| Check | Debug | Release | Notes |
|---|---|---|---|
| `x64\<Config>\cb4sqlite_tests.exe` (all test cases pass) | ____ | ____ | |
| `gl.exe /checkres /quiet` (resource failures) | ____ | ____ | |
| `gl.exe /stress /quiet` (GDI + USER growth over 100 opens) | ____ | ____ | |
| `trivial_dialog.exe /selftest /quiet` (failures) | ____ | ____ | see #8, #9 |
| Debug `gl.exe` under the debugger: open and close the Chart of Accounts, exit. The Output window shows no "Detected memory leaks!" | ____ | n/a | |

### P3. Fresh-machine install (plan M7 task 2)

On the test machine, make a folder with these files and nothing else:

- `gl.exe` and `dbf2sqlite.exe` from `x64\Release\`;
- `GL.INI` from `gl\test\dev-fixture\`;
- a `GLDR.INI` containing only:

  ```
  [GL_DIR]
  DIR=dev.db
  ```

- `dev.db`, either migrated there with `dbf2sqlite.exe --src <copy of gl\test\dev-fixture\dbf> --out dev.db`, or copied from the dev machine's `x64\Release\dev.db` after `setup.cmd Release`.

Keep an untouched copy of `dev.db` so the walkthrough can be repeated. For the external checks in #3–#5, use a `sqlite3.exe` on the test machine (the command-line shell from sqlite.org) or copy `dev.db` back to the dev machine.

| Check | Result | Notes |
|---|---|---|
| `gl.exe` starts with no Visual C++ runtime installed (static CRT) | ____ | |
| `dbf2sqlite.exe` migration exits 0 (if used) | ____ | |
| Login as `ADMIN` / `ADMIN` works | ____ | |

## The 9 criteria (spec §6.7)

Run these on the test machine, logged in as `ADMIN`, against the dev fixture. The fixture has 11 accounts. 340000 is the only Retained Earnings account. Every account with a non-zero balance counts as having activity (its previous-year balance), and 102101 also has period activity. The accounts that can be deleted are 144910, 410001 dept 10 and 410001 dept 20.

### 1. Launch and frame

- [ ] (a) The menu matches the original MANILA menu in labels and order.
- [ ] (b) The toolbar is along the top, and the "manila" module bar (2057) is along the bottom.
- [ ] (c) The module bar's status line shows "Dev Fixture Company Ltd." and the current period "Oct 26".
- [ ] Right-click on the frame background opens the GLSUB menu.
- [ ] Log in as `CLERK` / `clerk` (level B): the Maintenance menu is grayed.

**Result:** ____ **Notes:** ____

### 2. Chart of Accounts navigation

- [ ] **Accounts > Chart of Accounts** opens dialog 1010, "General Ledger Account Maintenance - Add". "General Ledger Accounts List" appears beside it without taking the focus.
- [ ] The list shows the 11 accounts in account + dept order (410001/10 before 410001/20).
- [ ] Double-click a row, and separately select one and press Enter: the dialog shows that account's account, dept, description, balances and type. The list hides.
- [ ] Type `22` in Account and click **Search**: the list appears with 221002 highlighted.
- [ ] Type `2210` and click **Set Data**: the dialog shows 221002. Type `2500` and click **Set Data**: the dialog shows the next account after it, 260004.
- [ ] GLSUB > Add Accounts opens the same dialog.

**Result:** ____ **Notes:** ____

### 3. Add

- [ ] Enter account `150000`, description `TEST ASSET`, no type, then click **Add/Mod**. You get "Choose a account type before appending" (4071), and nothing is added.
- [ ] Choose **Balance Sheet** and click **Add/Mod**. The account appears in the list.
- [ ] External check: `sqlite3 dev.db "select account, dept, descript, type, curr_bal, openbal from glaccts where account like '150000%'"` returns one row, type `B`.
- [ ] Enter account `350000`, description `SECOND RE`, choose **Retained earnings**, then click **Add/Mod**. You get "Only 1 Retained Earnings account may be entered." (16011), and nothing is added.
- [ ] Leave Account blank and click **Add/Mod**. You get "The field ... must be completed before continuing." (4072).

**Result:** ____ **Notes:** ____

### 4. Modify

- [ ] Record the row first: `sqlite3 dev.db "select * from glaccts where account like '150000%'"`.
- [ ] Pick 150000 from the list and change its description to `TEST ASSET EDITED` and its opening balance to `100.00`. Click **Add/Mod**.
- [ ] Run the query again. Only `descript`, `openbal` / `previous12` and `curr_bal` changed. `type` is still `B`.

**Result:** ____ **Notes:** ____

### 5. Delete

- [ ] With 150000 in the dialog, click **Delete**. You get "Are you sure you wish to delete this account?" (1021). Answer **No**: the account is still there.
- [ ] Click **Delete** again and answer **Yes**. The account is gone from the list, and `sqlite3 dev.db "select count(*) from glaccts where account like '150000%'"` returns 0.
- [ ] Pick 102101 and click **Delete**, then **Yes**. You get "Account number must be empty to be deleted." (1027), and the account is still there.
- [ ] GLSUB > Delete Accounts opens the same dialog. Its caption is "General Ledger Accounts Maintenance - Delete" and the OK button says **Delete**. Delete 144910 there: the account is gone.
- [ ] Type an account that doesn't exist (`999999`) and click **Delete**. You get "Accounts number not found." (1026).

**Result:** ____ **Notes:** ____

### 6. Persistence

- [ ] Before closing, add account `150001` (`PERSIST TEST`, Balance Sheet) and change 101000's description to `PETTY CASH EDITED`.
- [ ] Exit `gl.exe` and start it again.
- [ ] 150001 is there. 101000 shows the edited description. 150000 and 144910 are still gone.

**Result:** ____ **Notes:** ____

### 7. Account list

- [ ] The list window's columns are account, dept and description (strings 6068, 6054, 6044). Its rows are in "accounts" tag order.
- [ ] Selecting a row fills the dialog (covered by #2) and hides the list.
- [ ] Click a row, then type `41`: the selection jumps to the first 410001 row.
- [ ] Right-click the list: List Window Parameters (1035) opens with the query `.NOT. DELETED()`. **Cancel** leaves the list unchanged.
- [ ] Move and resize the list, then close the dialog and open it again. The list comes back where you left it (`GL.INI [GLACCTS]` LEFT/TOP/RIGHT/BOTTOM).
- [ ] Close the list with its X, then click **Search**. The list comes back.

**Result:** ____ **Notes:** ____

### 8. `tb*` input validation

The Spec #1 dialogs use `tbPic` and `tbFloat`. No Spec #1 dialog contains a `tbInteger`, so test it in `trivial_dialog.exe`, whose `/selftest` also covers all three controls.

| Control | Where | Type | Expected | Result |
|---|---|---|---|---|
| `tbPic` `XXXXXXX` | 1010, Account | `ABCDEFGHIJ` | stops at 7 characters | ____ |
| `tbPic` `(NNN) NNN - NNNN` | 1021 (File > Company Profile), phone | `5551234567` | `(555) 123 - 4567`, literals inserted | ____ |
| `tbPic` `(NNN) NNN - NNNN` | 1021, phone | `-` or `!` | rejected | ____ |
| `tbFloat` `.99` | 1010, Current balance | `12.345` | `12.34` (the third decimal is rejected) | ____ |
| `tbFloat` | 1010, Current balance | `1.2.3` | the second `.` is rejected | ____ |
| `tbFloat` | 1010, Current balance | `abc` | rejected | ____ |
| `tbInteger` `9999` | `trivial_dialog.exe` | `12345` | `1234` | ____ |
| `tbInteger` | `trivial_dialog.exe` | `1.5` | `15` (`.` is rejected) | ____ |

If you have the original 16-bit build or notes on its behaviour, compare against them in **Notes**.

**Result:** ____ **Notes:** ____

### 9. Framework reusability

- [ ] `win32fx/examples/trivial_dialog/trivial_dialog.sln` is not part of `gl.sln`, and it links `x64\<Config>\win32fx.lib`. Build `gl.sln`, then build `trivial_dialog.sln` for the same configuration. No `win32fx/` source changes are needed.
- [ ] `trivial_dialog.exe` (under `win32fx\examples\trivial_dialog\x64\<Config>\`) shows its modal dialog. Its `OnCommand` handler responds, and the dialog closes.
- [ ] `trivial_dialog.exe /selftest /quiet` exits 0 (P2).

Known deviation: the spec says "a 30-line example program in `win32fx/examples/trivial_dialog.cpp`". The example is a project folder, `win32fx/examples/trivial_dialog/`, and it is longer because it carries the plan's risk-P2 dispatch self-test.

**Result:** ____ **Notes:** ____

## Known gaps accepted before the walkthrough

| # | Gap | Why accepted | Follow-up |
|---|---|---|---|
| G1 | High DPI is not tested. The walkthrough runs at 96 DPI. | Plan risk P5. The manifest still declares per-monitor v2. | The first hi-DPI bug becomes a task in the AP/AR specs (#5/#6). |
| G2 | The account list can only run the query `.NOT. DELETED()`. Any other `GL.INI [GLACCTS] QUERY` shows a message and falls back to that default. | `Relate4set::querySet` in cb4sqlite supports only that expression in Spec #1. | A later spec adds an expression evaluator. |
| G3 | The account list is a ListView, not the original `tbSpread` grid. | Spec #1 has no grid control (§6.7 #7 correction). | The grid arrives with the spec that ports `tbSpread`. |
| G4 | The User Access Menu button and the Cash Accounts grid list (1034) are not available. | They need the grid control. | As G3. |
| G5 | No Spec #1 dialog contains a `tbInteger`, so #8 tests it in `trivial_dialog.exe`. | No Spec #1 screen uses one. | None needed. |

## Gaps and triage

Add one row for each `FAIL` or unexpected behaviour found during the walkthrough.

| # | Criterion | What happened | Decision (fix in Spec #1 / file for Spec #N) | Fixed in commit / issue |
|---|---|---|---|---|
| | | | | |

## Sign-off

| Criterion | Result |
|---|---|
| 1. Launch and frame | ____ |
| 2. Chart of Accounts navigation | ____ |
| 3. Add | ____ |
| 4. Modify | ____ |
| 5. Delete | ____ |
| 6. Persistence | ____ |
| 7. Account list | ____ |
| 8. `tb*` validation | ____ |
| 9. Framework reusability | ____ |

All 9 criteria pass, or pass with the known gaps recorded above:

- Tester: ____ Date: ____
- Witness: ____ Date: ____
