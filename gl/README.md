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

Company "Dev Fixture Company Ltd.", fiscal year April 2026 to March 2027, current period "Oct 26". To regenerate the `.dbf` files after editing them, run `python3 gl/test/dev-fixture/make_dev_fixture.py`.

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

## What runs (plan M5)

- **Frame:**
  - the MANILA menu and the top toolbar;
  - a module bar along the bottom whose fields show the company name, menu help and the current period;
  - right-click on the background for the GLSUB menu.
- **Login** (dialog 1158). **Maintenance > User Access** (1159). The Menu button there needs the grid control, which Spec #1 doesn't have.
- **File > Company Profile** (1021), with:
  - Periods (1028);
  - Accounts (1033) and its Cash button (1034, without the grid list beside it).
- **Help > About.**
- **Everything else:**
  - Accounts Payable, Accounts Receivable and report commands show a "not yet ported" message naming the spec that brings them.
  - Other General Ledger commands say they are not part of Spec #1.
  - The Chart of Accounts comes in plan M6.

Checks: `gl.exe /checkres` loads every Spec #1 resource. Add `/quiet` to get the failure count as the exit code instead of a message box.
