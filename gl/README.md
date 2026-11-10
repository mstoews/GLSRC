# gl.exe — the General Ledger port

The Win32 / MSVC x64 / Unicode port of the 1995 OWL General Ledger (spec: `gl/docs/superpowers/specs/2026-05-10-owl-to-win32-sdk-port-design.md`).
- **New code:** `gl/port/` (spec §2.4) and the pruned resources in `gl/res/`.
- **Legacy reference only:** `gl/src/`, `gl/header/`, `gl/gl.rc` and `gl/gl_new.rc` are not built.

## Build

Open `gl.sln` (repository root), choose `Debug|x64` or `Release|x64`, and build. Output goes to `x64\<Config>\`: `gl.exe`, `dbf2sqlite.exe`, `cb4sqlite_tests.exe`.

## First run: the dev fixture

`gl.exe` needs a company database. `gl/test/dev-fixture/` has a small made-up company with two users. From a command prompt in the repository root:

```
gl\test\dev-fixture\setup.cmd Debug
x64\Debug\gl.exe
```

`setup.cmd` migrates `gl\test\dev-fixture\dbf\` into `x64\Debug\dev.db` and copies `GLDR.INI` and `GL.INI` next to `gl.exe`. It replaces those three files if they already exist.

| User | Password | Level | Menu access |
|---|---|---|---|
| `ADMIN` | `ADMIN` | A | everything |
| `CLERK` | `clerk` | B | the Maintenance menu is grayed |

Company "Dev Fixture Company Ltd.", fiscal year April 2026 to March 2027, current period "Oct 26". The chart has 11 accounts. 340000 is the Retained Earnings account. 102101 and 101000 have period-7 activity (posted voucher 100, below), so the Chart of Accounts won't delete them. The deletable accounts are 144910 and 410001 depts 10 and 20.

For the Spec #2 voucher screens, the fixture also has:

| Voucher | State (`gl_btch.mode`) | Lines |
|---|---|---|
| 100 | posted (`0`), with its `gl_dist` rows | 102101 Dr 250, 101000 Cr 250 |
| 101 | open (`1`), balanced | 111101 Dr 1,150; 410001-10 Cr 600 and 410001-20 Cr 550, with medical reps, doctors and products |
| 102 | open (`1`), out of balance | 148000 Dr 100, 260007 Cr 90 |
| 103 | open adjustment (`3`) | 144910 Dr 75, 111101 Cr 75 |

- **Master data:** three medical reps (MR001–MR003), doctors (DR001–DR003) and products (P100, P200, P300).
- **Departments:** 10 and 20.
- **Archive:** `gl_distb` is empty.
- **`GL.INI`:** `[PERIOD] BATCHNO=99`.

To regenerate the `.dbf` files after editing them, run `python3 gl/test/dev-fixture/make_dev_fixture.py`.

## A real company

Migrate the company's `.dbf` directory once (`dbf2sqlite_core/README.md`):

```
x64\Release\dbf2sqlite.exe --src D:\GL\DBF --out D:\GL\company.db
```

Then set `GLDR.INI` next to `gl.exe` to name the database file:

```
[GL_DIR]
DIR=D:\GL\company.db
```

## Settings files

`GLDR.INI` and `GL.INI` keep their original format and keys, with two changes:

- **Location.** They are read from the directory of `gl.exe`. The original read them from the Windows directory.
- **`[GL_DIR] DIR`.** It names the company **database file**, not the `.dbf` directory. A relative path is relative to `gl.exe`. `DRIVE` is ignored.

`gl.exe` refuses to start if `DIR` is any of these:
- a directory;
- a file that doesn't exist;
- a network path (UNC or mapped drive). SQLite isn't safe there until a later spec adds locking (spec risk 10).

| File | Section / key | Used for |
|---|---|---|
| `GLDR.INI` | `[GL_DIR] DIR` | the company database |
| `GL.INI` | `[ACCESS] PSW` | `T`: log in at startup. Anything else: no login, user `NOLOGIN`, no menu items grayed |
| `GL.INI` | `[WITHHOLDING] TAX` | Company Profile's "Withholding Tax Excluded" box (written on Save) |
| `GL.INI` | `[GLACCTS]` `LEFT` `TOP` `RIGHT` `BOTTOM` `FIELD1`–`FIELD3` `QUERY` | the account list's position (saved on close), its three columns (field numbers, default 1–3), and its row filter (default `.NOT. DELETED()`) |
| `GL.INI` | `[GLDEPT]`, `[GL_MEDRP]`, `[GL_DRCOD]`, `[GL_ITEMM]`, same keys | the same for the Department, Medical Rep, Doctor and Product lists |

## What runs (Specs #1 and #2, and Spec #3 so far)

- **Frame:**
  - the MANILA menu and the top toolbar;
  - a module bar along the bottom whose fields show the company name, menu help and the current period;
  - right-click on the background for the GLSUB menu.
- **Login** (dialog 1158). **Maintenance > User Access** (1159). Its Menu button opens Menu Access (1160) with the menus of the level shown: double-click or Enter on a menu grays or enables it, and OK saves them as that level's menu code. Reset applies the level shown to the current session's menus.
- **File > Company Profile** (1021), with:
  - Periods (1028);
  - Accounts (1033) and its Cash button (1034, with the cash accounts list, 1011, beside it).
- **Accounts > Chart of Accounts** (1010), also GLSUB > Add Accounts:
  - **Add/Mod:** adds the typed account + dept, or updates it if it exists;
  - **Search:** shows the account list at the typed account;
  - **Set Data:** shows the typed account, or the next one after it;
  - **Delete:** removes an account with no period or previous-year balance.

  GLSUB > Delete Accounts opens the same dialog with Delete on the OK button.
- **The account list** beside it ("General Ledger Accounts List"):
  - double-click or Enter picks an account;
  - typing jumps to an account;
  - right-click edits the list query.

  Its position and query are kept in `GL.INI [GLACCTS]`. `QUERY` accepts `.NOT. DELETED()` (the default) and comparisons of a text field with a quoted value, joined with `.AND.`: for example `ACCOUNT = '4'` lists the accounts starting with 4.
- **Accounts > Department Codes** (1023, also GLSUB), **Medical Reps** (1006), **Doctor Codes** (1040) and **Product Codes** (1007):
  - Add/Mod adds the typed code, or updates it after asking.
  - Delete removes the typed code.
  - Medical Reps, Doctor Codes and Product Codes also have New and Top / Bottom / Next / Previous.
  - Each has a list beside it, like the account list. Search, or the Med Reps / Doctor / Products button, shows the list at the typed code, and picking a row shows that record. The lists keep their positions and queries in `GL.INI [GLDEPT]`, `[GL_MEDRP]`, `[GL_DRCOD]` and `[GL_ITEMM]`.
- **Transactions > General Ledger Voucher** (1013, also GLSUB and the toolbar's batch button) and **Transactions > Prior Period Entry** (the same dialog, for adjustment vouchers):
  - **Fields:** a voucher has a description, a date, a period (only open periods are offered), a cheque number and its lines.
  - **Add/Mod** (or Enter) saves the line shown into the voucher, creating the voucher with the number displayed if it's new. A line typed with an existing line number replaces that line.
  - **Delete** removes the line shown.
  - **New** starts the next voucher. For a voucher created in this session, New and Close first ask whether to keep it; No removes it.
  - **Top / Bottom / Next / Previous** move between vouchers not yet posted.
  - **Windows:** the Distribution List below the dialog shows the voucher's lines and totals; double-click a line to edit it. List shows the account list, Voucher the list of vouchers, and Show the floating tool box.
  - **Print** is the Spec #3 "not yet ported" message.
  - The lists keep their positions in `GL.INI [GL_DISTLIST]`, `[GL_BTCH]` and `[GLACCTS]`.
- **Transactions > Post Voucher > General Ledger Voucher** (1061, also GLSUB and the toolbar's post button) lists the open and adjustment vouchers. **Post** posts them all; double-clicking one posts just that one. A voucher whose lines don't balance, or that uses an account not in the chart, is refused with a message and stays listed. Posting is all or nothing: if anything fails, nothing is posted.
- **Transactions > Recover General Ledger Voucher** (1015) lists the posted vouchers. Double-click one and confirm: its postings are reversed and it is reopened for editing.
- **Transactions > Cancel Voucher > General Ledger Voucher** (1043) lists the vouchers not yet posted. Double-click one and confirm to delete it.
- **Transactions > Voucher Reports > General Ledger Voucher Inquiry** (1029): type an account and dept, or pick one in the account list (Search shows and hides it), then Set Data. It lists the account's posted lines between its opening balance and its balance.
- **Accounts > Budget Data** (1017): the Chart of Accounts screen with a grid of the 12 months. Pick an account (Search, or Set Data on the account typed), edit the Budget column in the grid (Enter, F2 or a double-click edits a cell), then OK saves the 12 amounts.
- **Maintenance > Backup Databases** saves a copy of the company database to a file you choose, by default `<name>-<yyyymmdd>.db` beside it. **Restore Databases** replaces the company database with a backup you choose, after checking it is one and asking. The replaced file is kept as `<name>.db.bak`. If anything goes wrong, the original is put back.
- **Maintenance > Year End Consolidation** moves the income and expense balances into the Retained Earnings account and the period amounts into the previous-year columns. It runs only when exactly one account has type R; otherwise it names the type-R accounts so you can correct them in the Chart of Accounts. **Reindex Databases** explains that an SQLite company database needs no reindexing.
- **File > Printer Setup** chooses the printer, paper and margins for reports. The choice lasts until the program exits; it doesn't change the Windows default printer. Orientation isn't offered there: each report has its own.
- **Report preview** (Spec #3, S3-M1): every report opens in a window of its own, with Print, First / Previous / Next / Last, the page number, Zoom (fit width, fit page, 100 %), Options (the report's options again) and Close. PgUp / PgDn, Home / End, the arrow keys, the mouse wheel and Ctrl+P work in it. **File > Print** and the toolbar Print button print the active preview. On Windows 11 the Print dialog is the system's own print window.
  No original report is ported yet (they come from S3-M2 on); `gl.exe /samplereport` opens a test report (see Checks).
- **Help > About.**
- **Everything else:**
  - Accounts Payable, Accounts Receivable and report commands show a "not yet ported" message naming the spec that brings them.
  - Other General Ledger commands say they are not part of Spec #1.

## Checks

- **`gl.exe /checkres`** loads every resource the port uses, including the port's own dialogs (`gl/res/port.rc`). Add `/quiet` to get the failure count as the exit code instead of a message box.
- **`gl.exe /stress`** opens and closes the Chart of Accounts 100 times, (Add, Delete and Budget Data in turn), then Department, Medical Rep, Doctor and Product Codes, Voucher Entry, Post, Recover, Cancel and Transaction Inquiry 25 times each, then the sample report's preview 25 times (paging through it at each zoom), on the company database, and reports the change in GDI and USER objects, in total and per step. With `/quiet` the total is the exit code, so 0 passes.
- **`gl.exe /samplereport`** opens the report engine's test report after startup: the chart of accounts grouped by type, with subtotals, a total and a wrapped note. Its options repeat the list (to make pages), drop zero balances, or turn the page to landscape. Use it to check the preview, Printer Setup and printing, e.g. to *Microsoft Print to PDF*.
- **`gl.exe /failpost <n>`** runs normally, except that posting fails after writing its nth line. Use it to check that a failed post leaves `glaccts`, `gl_dist`, `gl_btch` and `gl_btchd` unchanged (spec #2 §8 #6).
- **Debug builds** report CRT heap leaks at exit in the debugger's output window.
