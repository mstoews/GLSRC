# Spec #1 — OWL-to-Win32 SDK Port: Foundation, Data Layer, Chart of Accounts

**Status:** Draft
**Date:** 2026-05-10
**Scope:** First spec of a multi-spec migration of the 1994 Tegara Corporation General Ledger / AP / AR accounting application from Borland OWL (Win16-era) to a strict Win32 SDK, MSVC x64, Unicode application.

---

## 1. Goals & Non-Goals

### 1.1 Goals (Spec #1 deliverables)

A working `gl.exe` that demonstrates the migration foundation end-to-end:

1. Compilable MSVC x64 Unicode `gl.exe`.
2. MDI frame chrome equivalent to the original (menu, vertical toolbar, status bar with company name + accounting period).
3. Login flow against the new SQLite-backed access table.
4. Company profile load on the new SQLite-backed company table.
5. **Full Chart of Accounts CRUD round-trip** end-to-end:
   - Add, modify, delete an account.
   - Navigate (top / bottom / next / prev) by ACCT_NO and DESCRIP indexes.
   - Persistence across process restarts.
6. Custom controls reimplemented at the binary-compatible class-name level: `tbPic`, `tbDate`, `tbFloat`, `tbInteger`, `tbSuperBtn`.
7. `dbf2sqlite` migration utility supporting the tables required by Spec #1: `accessm`, `access.psw`, the company-profile table, `glaccts`.
8. Mini-framework (`win32fx`) reusable as-is by AP and AR module ports in later specs.

### 1.2 Non-Goals

The following are **explicitly out of scope** for Spec #1:

- AP and AR modules (Specs #5 and #6).
- Reporting (Crystal Reports Print Engine replacement; Spec #3). Where the original launches a report, this port shows a "not yet ported" placeholder dialog.
- Help system (`gl.hbk`).
- Backup / restore.
- BWCC and CTL3D theming (deleted, not replaced).
- Full `.rc` port — only the resources Spec #1 needs are included in the build.
- TBOWL grid controls (used by AP/AR batch entry) — deferred until those modules demand them.
- Multi-user concurrency and any record/file locking — Spec #1 is single-user.
- Backup / restore.

### 1.3 Accepted Risks

- Most of `gl.rc` and all of `gl_new.rc` are excluded from the build until later specs. The pruned `gl.rc` for Spec #1 lives alongside the original in version control; the original stays for reference.
- TBOWL grid controls are not implemented; any dialog that references them will not be in the Spec #1 build.

---

## 2. Architecture & Project Layout

### 2.1 Solution structure

The repository root (historically `GLSRC/`; in this checkout the git root `gl-win/`) has this top-level layout. The legacy GL sources were moved in commit `1606b78` from `gl/` into `gl/src/` (`.cpp`) and `gl/header/` (`.h`/`.hpp`); they are kept **as reference only** and are never compiled by the new solution.

```
<repo root>/
├── include/                ← shared headers (db_accts.h, other db_*.h, CodeBase/Toolbox/CRPE headers)
├── common/                 ← shared sources across modules (gl_acct, gldbf, gl_prof, gl_pds, utildlg …)
├── gl/
│   ├── src/                ← legacy GL .cpp files (reference only)
│   ├── header/             ← legacy GL .h/.hpp files (reference only; includes db_acces.h)
│   ├── lzh/                ← original LZH source/exe archives
│   ├── gl.rc, gl_new.rc    ← legacy Borland resources (reference only)
│   └── docs/               ← this spec and its plan
├── ap/, ar/                ← Accounts Payable / Receivable (source still in .LZH archives)
└── owl/, db/, scs/, eiwa/, src/  ← 16-bit libs and archives (out of scope)
```

Spec #1 adds five new top-level project directories alongside the existing module directories. The Visual Studio solution `gl.sln` and the shared property sheet `common.props` live at the repo root:

```
<repo root>/
├── gl.sln                  ← NEW — solution file
├── common.props            ← NEW — shared compiler/linker settings (spec §7.1), imported by every project
├── include/                ← existing — added to every project's IncludePath
├── gl/                     ← existing — Spec #1 adds gl.vcxproj + new port sources (§2.4) next to src/ and header/
├── ap/                     ← existing — untouched in Spec #1
├── ar/                     ← existing — untouched in Spec #1
├── win32fx/                ← NEW (static lib) — Mini-framework
├── cb4sqlite/              ← NEW (static lib) — CodeBase compatibility shim over SQLite
├── cb4sqlite_tests/        ← NEW (console EXE) — Layer-1 + Layer-2 test harness (Catch2)
├── dbf2sqlite_core/        ← NEW (static lib) — Migration core (DBF parse, schema gen, header gen)
└── dbf2sqlite/             ← NEW (console EXE) — Thin CLI wrapper around dbf2sqlite_core
```

**Include path:** every project adds `$(SolutionDir)include` and then `$(SolutionDir)gl\header` to `IncludePath`, in that order. The schema headers live in those two directories — `db_accts.h` and the other shared `db_*.h` in `include/`, `db_acces.h` in `gl/header/` — and future `db_*.h` from the M2 generator are written to `include/`. Ports built in Spec #1 `#include "db_accts.h"` exactly as the original code does. Because `include/` comes first, the copies of `glh.h`, `gl_pds.hpp` and `gl_prof.hpp` in `include/` win over the duplicates in `gl/header/`.

**Legacy-header warnings:** the hand-written schema headers trip `/W4` warnings (e.g. C4100, the unused `LPSTR name` constructor parameter). Under `/WX` those would fail the build, so ports wrap legacy-header includes in `cb4sqlite/include/legacy_headers_begin.h` / `legacy_headers_end.h` (`#pragma warning(push)` + targeted disables / `pop`) instead of editing the headers.

Static linking only. No DLLs. Single-exe distribution.

**Build artifact layout:** all projects emit to `$(SolutionDir)x64\$(Configuration)\` (i.e., `<repo root>/x64/Debug/` or `<repo root>/x64/Release/`; git-ignored). Static libs land alongside EXEs; `gl.exe` references them by relative path during link.

### 2.2 `win32fx/` (static lib)

```
win32fx/
├── Window.h/.cpp           — HWND-owning base class, message dispatch
├── Dialog.h/.cpp           — Modal/modeless base class
├── MsgMap.h                — BEGIN_MSG_MAP / ON_MSG / END_MSG_MAP macros
├── HandleGuard.h           — RAII for HMENU/HFONT/HBITMAP/HBRUSH/HPEN/HRGN/HICON
├── StringUtil.h/.cpp       — LoadResString, Format, Utf8ToWide, WideToUtf8, Trim
├── MdiFrame.h/.cpp         — MDI frame helper (menu, accelerator, MDICLIENT)
├── Toolbar.h/.cpp          — Top toolbar
├── VerticalToolbar.h/.cpp  — Left vertical toolbar (manila tabs equivalent)
├── StatusBar.h/.cpp        — Bottom status bar
└── controls/
    ├── PicEdit.h/.cpp       — tbPic   (mask-validated edit)
    ├── DateEdit.h/.cpp      — tbDate  (date edit)
    ├── FloatEdit.h/.cpp     — tbFloat (one-decimal numeric edit)
    ├── IntEdit.h/.cpp       — tbInteger (integer-only edit)
    └── SuperBtn.h/.cpp      — tbSuperBtn (themed/owner-draw button)
```

### 2.3 `cb4sqlite/` (static lib)

```
cb4sqlite/
├── include/
│   ├── cb4sqlite.h         — Umbrella header (replaces d4all.hpp in ports)
│   ├── cb4sqlite_compat.h  — _CLASSDEF no-op, r4*/e4* return codes, field-type constants
│   ├── Schema.h            — FieldDef/TagDef/TableInfo, createTable/createTag (shared with dbf2sqlite_core)
│   ├── legacy_headers_begin.h / legacy_headers_end.h — warning push/pop around legacy schema headers
│   ├── Code4.h             — Database session
│   ├── Data4.h             — Cursor (per-table sqlite3_stmt*)
│   ├── Field4.h            — Field accessor
│   ├── Date4.h             — Date helpers
│   ├── Relate4set.h        — Multi-cursor join helper
│   └── Errors.h            — cb4sqlite::Error : std::runtime_error
├── src/                    — Code4/Data4/Field4/Field4_win32/Date4/Schema/Relate4set .cpp + Internal.h/.cpp
├── README.md               — semantics, method coverage, compile-only stubs
├── test/parse_smoke.cpp    — M0 compile-only smoke test (built by cb4sqlite_tests)
└── sqlite3.c / sqlite3.h   — Vendored amalgamation
```

The migration tool is **not** under `cb4sqlite/`: it is split into `dbf2sqlite_core/` (static lib — DBF reader vendored at `dbf2sqlite_core/vendor/shapelib/`, index-expression translation, header generator; test fixtures at `dbf2sqlite_core/test/fixtures/`) and `dbf2sqlite/` (thin console EXE). See §4.5 and plan M2.

### 2.4 `gl/` (the EXE)

New port code goes in `gl/port/` and new resources in `gl/res/`, kept apart from the legacy reference files in `gl/src/`, `gl/header/` and the legacy `gl/gl.rc` / `gl/gl_new.rc`:

```
gl/
├── gl.vcxproj               — NEW — Windows EXE project
├── port/
│   ├── App.cpp              — WinMain, globals, init/shutdown
│   ├── ManilaGL.h/.cpp      — MDI frame subclass; menu/toolbar wiring
│   ├── Login.h/.cpp         — gl/src/gl_acces.cpp port
│   ├── CompanyProfile.h/.cpp — gl/src/gl_profi.cpp port
│   ├── ChartOfAccounts.h/.cpp — gl/src/gl_accts.cpp port (TGLAcctsAdd/Delete)
│   ├── AccountList.h/.cpp   — gl/src/gl_acctl.cpp port (TAccountList)
│   └── stubs/
│       ├── ReportingStubs.cpp — "not yet ported" message boxes
│       ├── ApStubs.cpp
│       └── ArStubs.cpp
├── res/
│   ├── gl.rc                — Pruned copy of gl/gl.rc for Spec #1 (see §6)
│   ├── manifest.xml         — v6 common controls + per-monitor DPI v2
│   └── bitmaps/             — Extracted from original hex blobs
├── src/, header/            — legacy, reference only (not in gl.vcxproj)
└── gl.rc, gl_new.rc         — legacy, reference only
```

### 2.5 Removed dependencies

The following are **deleted** in Spec #1; ports will not include their headers:

- OWL — `tapplication.h`, `tmdiframe.h`, `twindow.h`, `tdialog.h`, `ttoolbar.h`, etc.
- CodeBase — `d4all.hpp` (replaced by `cb4sqlite/`)
- Crystal Reports — `crpe.h` (stubbed)
- BWCC — `bwcc.h`
- CTL3D — `ctl3d.h`
- TBOWL / Toolbox DLLs — `tbowl.h`
- Custom string class — `cstring.h` (replaced by `std::wstring`)
- DOS / 16-bit headers — `dir.h`, `direct.h`
- `DBF.DLL` — replaced by static `cb4sqlite`

---

## 3. The Mini-Framework (`win32fx`)

### 3.1 Window base class

`Window` owns an HWND and dispatches messages via a `SetWindowLongPtr(GWLP_USERDATA)` thunk to virtual handlers:

- `OnCreate(LPCREATESTRUCT)`
- `OnCommand(WORD id, WORD code, HWND hwndCtl)`
- `OnSize(UINT type, int cx, int cy)`
- `OnDestroy()`
- `OnMessage(UINT msg, WPARAM, LPARAM)` — fall-through for everything else

The thunk pattern: subclass calls `Create(...)` which calls `CreateWindowExW` with `this` in `lpParam`; the WndProc on `WM_NCCREATE` stores `this` via `SetWindowLongPtr(GWLP_USERDATA)` and routes subsequent messages to the instance.

### 3.2 Dialog base class

`Dialog` parallels `Window` for modal (`DialogBoxParamW`) and modeless (`CreateDialogParamW`) flows. Same virtual handlers; `OnInitDialog` instead of `OnCreate`.

### 3.3 Message map macros

For the ~5% of cases virtuals don't cover (`WM_NOTIFY`, `WM_CTLCOLOR*`, custom messages):

```cpp
BEGIN_MSG_MAP(MyDialog)
    ON_MSG(WM_NOTIFY,    OnNotify)
    ON_MSG(WM_CTLCOLORBTN, OnCtlColor)
END_MSG_MAP()
```

### 3.4 RAII handle wrapper

`HandleGuard<H, Deleter>` template instantiated for `HMENU`, `HFONT`, `HBITMAP`, `HBRUSH`, `HPEN`, `HRGN`, `HICON`. Replaces the OWL `delete window` pattern.

### 3.5 MDI scaffolding

`MdiFrame`, `Toolbar`, `VerticalToolbar`, `StatusBar` are independent `Window` subclasses. `MdiFrame` owns the MDICLIENT child and the menu merging logic (the original `HGLMenu` global is preserved as `extern HMENU HGLMenu`).

### 3.6 Custom-control registration — the critical lever

`win32fx::RegisterCustomControls()` registers WNDCLASS entries with the **original class names**:

```cpp
WNDCLASSEXW wc = {};
wc.cbSize        = sizeof(wc);
wc.style         = CS_DBLCLKS | CS_GLOBALCLASS;
wc.lpfnWndProc   = &PicEditWndProc;
wc.cbClsExtra    = 0;
wc.cbWndExtra    = sizeof(void*);          // for instance-thunk pointer
wc.hInstance     = hInst;
wc.hIcon         = nullptr;
wc.hCursor       = LoadCursorW(nullptr, IDC_IBEAM);
wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
wc.lpszMenuName  = nullptr;
wc.lpszClassName = L"tbPic";
wc.hIconSm       = nullptr;
RegisterClassExW(&wc);
// Repeat for tbDate, tbFloat, tbInteger, tbSuperBtn
// (tbSuperBtn uses IDC_ARROW and a null background — see SuperBtn.cpp)
```

This means the original `.rc` dialog templates can call `CreateWindowExW(L"tbPic", ...)` unchanged — **~190 KB of resource layout is reused without editing**.

### 3.7 Globals preserved

The original module-level globals (`hInst`, `HGLMenu`, `cb`, `szApp`) are kept as `extern` declarations in `App.cpp`. Not wrapped in singletons or namespaces. Spec #1 prefers compatibility over modernization.

### 3.8 Estimated size

~1200 lines for the framework (including controls). Confirms the "light helpers" decision — heavier than raw Win32, much lighter than MFC/WTL.

---

## 4. `cb4sqlite` Compatibility Shim

### 4.1 Principle

Mirror the `Code4` / `Data4` / `Field4` API surface **only as much as ported `.cpp` files need**. Spec #1 implements the subset used by login + company profile + Chart of Accounts. Unimplemented methods throw:

```cpp
throw cb4sqlite::Error("cb4sqlite: not implemented in spec #1: <method>");
```

This is intentional: the shim grows organically as later specs port more screens. We do not pre-implement the entire CodeBase API.

### 4.2 API surface required by Spec #1

The shim mirrors the subset of CodeBase 4 that the Spec #1 sources call. **The original CodeBase headers are in `include/` (`d4data.hpp`, `d4defs.h`, `e4expr.h`) and are the reference for signatures, return codes and behaviour.** An earlier sketch here was written before those headers were consulted and was replaced in M1. See §4.7 for what changed.

**Implemented surface** (authoritative: `cb4sqlite/include/*.h`; semantics and test coverage: `cb4sqlite/README.md`):

| Class | Methods |
|---|---|
| `Code4` | `open(path, create=false)` *(new)*, `close()` *(new)*, `isOpen()`, `closeAll()`, `init()`, `initUndo()`, `handle()`, `errorCode` |
| `Data4` | ctor `(Code4*` or `Code4&, const char* table)`; `top`, `bottom`, `skip(n=1)`, `go(rec)`, `seek(const char*)`, `seek(double)`, `eof`, `bof`, `recNo`, `recCount`; `SetIndex(tag)`, `select()`; `appendStart`, `append`, `appendBlank`, `blank`, `flush`, `deleteRec`, `recall`, `deleted`, `pack`, `zap`; `numFields`, `fieldNumber`, `isValid`, `close`, `unlock` |
| `Field4` | ctors `()`, `(Data4&, int fieldNo)`, `(Data4&, const char* name)`, `init(...)`; `str`, `ptr`, `num`, `len`, `decimals`, `type`, `name`, `number`, `isValid`; `assign(const char*)`, `assignDouble(d, newLen=-1, nDec=-1)`, `assignLong(l, newLen=-1, zerosInFront=0)`, `trim`; Windows: `SetField(HWND, UINT)`, `AssignField(HWND, UINT)` |
| `Date4` | `Date4()`, `Date4(const char*)`; `today()` (non-static, as CodeBase), `assign`, `str`, `ptr`, `len`, `year`, `month`, `day` |
| `Relate4set` | ctor `(Data4*` or `Data4&)`; `querySet(expr=nullptr)`, `top`, `bottom`, `skip(n=1)`, `eof`, `unlock`, `free` |
| `Schema.h` | `createTable`, `createTag`, `loadTableInfo`, `ensureMetaTables`, `normaliseTableName`, `quoteIdent` (shared with `dbf2sqlite_core`) |
| free functions | `Str4ptr(const char*)`, `Str4long(const char*)` (in CodeBase `Str4ptr` was a class; every call site is `double(Str4ptr(x))`) |
| constants | `r4success 0, r4found 1, r4after 2, r4eof 3, r4bof 4, r4entry 5`; `e4*` error codes; field types `r4str 'C', r4num 'N', r4float 'F', r4date 'D', r4log 'L', r4memo 'M'`, with values copied from `d4defs.h` / `e4expr.h` |
| global | `extern cb4sqlite::Code4 cb;` at global scope, *defined* in `gl/port/App.cpp` (M5) |

`SetField` / `AssignField` aren't CodeBase methods. They come from the application's own `Field : Field4` class (`gl/src/gl_field.cpp`), and the shim puts them on `Field4` so call sites like `glaccts->accounts->SetField(HWindow, ID)` compile unchanged.

Methods not listed here don't exist in the shim: a call site that needs one fails to compile and is ported, or the shim grows (§4.1). Listed methods that hit an unsupported case (an unknown `querySet` expression, a ranged `zap`) throw `cb4sqlite::Error("… not implemented in spec #1 …")`.

**Per-table subclasses — hand-written authoritative.** The original code uses subclasses like `Tglaccts : public Data4` with named `Field4*` members (`accounts`, `dept`, `descript`, `budget1..budget12`, `previous1..previous12`, `period1..period12`, `openbal`, `curr_bal`, `category`, `type`, `fdescript`). Those subclasses live in `include/db_*.h` headers and are **hand-written, authoritative, and kept as-is**. The verified shape of `Tglaccts` (44 fields, plus convenience methods like `Open_bal()`, `dPeriod1..12()`, `dBudget1..12()`, `dPrevious1..12()`, `dPeriod(account, dept, period)`, `UpdateAccount(seek, period, debit, credit)`) is the ground truth for Spec #1.

**Field constructor pattern.** `db_accts.h` and `db_dept.h` use the name-based `Field4` ctor (`new Field4(*(Data4*)this, "account")`); `db_acces.h` uses 1-based field numbers. The shim supports both; the generator (M2) emits the name form.

**Methods declared on per-table subclasses but defined elsewhere.** Some convenience methods on `Tglaccts` (e.g., `dPeriod(...)`, `UpdateAccount(...)`) are declared in `db_accts.h` but defined in module-specific .cpp files (`ap_post.cpp`). Spec #1 does **not** invoke them, so the Spec #1 build does not link them. The headers compile cleanly without those .cpp files.

**Generator role (M2 of plan) — incremental.** The `dbf2sqlite` tool can also emit `db_<table>.h` headers for tables that **no existing code opens**. Policy (revised in M2):

- A table is **covered** if any `.h`/`.hpp`/`.cpp` in the `--headers-check` directories (normally `include/`, `gl/header/`, `common/`), or in the output directory, contains `Data4(cb, "<table>")` or `Data4::open(cb, "<table>")`. A file-name check can't work: the 8.3-era names don't follow the tables (`glaccts` → `db_accts.h`, `access.psw` and `accessl` → `db_acces.h`, `gl_prof` → `common/gl_prof.cpp`). Covered tables are left untouched; the hand-written code is authoritative.
- An uncovered table gets `db_<table>.h` (non-identifier characters become `_`): a `T<table> : public Data4` with one `Field4*` per field, opened by name, plus a destructor that deletes them. It compiles cleanly at `/W4` without the legacy-header guards. Hand edits are welcome.
- An existing file is never overwritten, even if it doesn't open the table.
- Output is deterministic, with no timestamps. Generated headers are committed, not regenerated at build time.

This policy preserves hand-tuned helpers (`dPeriod`, `Open_bal`, etc.) while filling in coverage for tables that don't yet have headers.

### 4.3 Storage model

- One SQLite file per company. Path comes from `GLDR.INI` exactly as before, but `.db` instead of a directory of `.dbf` files.
- Each `.dbf` becomes one SQLite table:
  - `_rowid INTEGER PRIMARY KEY` replaces dBase recno.
  - `_deleted INTEGER NOT NULL DEFAULT 0` is the soft-delete flag.
  - Columns named per dBase fields; types map as below.
- Type mapping:
  - `Character` → `TEXT` (values stored as-stored, **right-padded with spaces to declared column width on write**, preserved on read so byte-for-byte compares match the original dBase wire form). Spec #1 commits to right-pad-on-write; alternative interpretations (trim-on-write) break `seek` compatibility.
  - `Numeric`   → `REAL` or `INTEGER` (decided per field at migration time)
  - `Date`      → `TEXT('YYYYMMDD')`
  - `Logical`   → `INTEGER` (0/1)
  - `Memo`      → `TEXT`, the whole memo, read from the `.dbt` at migration (no separate memo file afterwards). `_cb_fields.len` keeps the 10-byte dBase pointer width, which doesn't limit the text.
  - Blank numeric, date, logical and memo values are stored as NULL.
- Field metadata: `_cb_fields(table_name, field_no, field_name, type, len, dec)` records each field's dBase type, declared width and decimals in record order. `Field4::len()/type()/decimals()`, padding and number formatting come from it.
- Indexes: each `.mdx` tag becomes a row in `_cb_tags(table_name, tag_name, key_expr, filter)`. `key_expr` is an SQL expression that reproduces the dBase key; for `ACCOUNT+DEPT` it is `account || dept`, which is byte-identical to the dBase key because character columns are stored padded. `createTag` also creates `CREATE INDEX <table>__<tag> ON <table> ((key_expr), _rowid)`. (This replaced an earlier `order_by_sql` column: a single key expression is what both ordering and `seek` need.) `filter` is NULL, or the SQL form of the tag's FOR clause: only matching records are in the tag, so `top`/`bottom`/`skip`/`seek` through it skip the rest, while `go()` and record order still see every record. Filtered tags get a partial index (`… WHERE (filter)`), and `Data4` adds the same term to its queries so SQLite uses it. `RECNO()` keys become `_rowid`. Databases written before the `filter` column existed (M2) still open, and `ensureMetaTables` adds the column.
- **`seek` rule (CodeBase partial-key match).** `Data4::seek(const char* key)` finds the first record whose tag key is `>= key`. It returns `r4success` if that key *starts with* `key`, `r4after` otherwise (positioned on it), and `r4eof` if there is none. Example: key `account || dept`, stored `"100    01"`; `seek("100")`, `seek("100    ")` and `seek("100    01")` all return `r4success`. The earlier "pad to the leading column, then compare for equality" rule is a special case of this, and the M1 tests cover both.

### 4.4 Cursor model

Each navigation runs one keyset query: `SELECT … WHERE ((key_expr), _rowid) > (?current_key, ?current_rowid) ORDER BY (key_expr), _rowid LIMIT 1 OFFSET n-1` (reversed for backward moves). The `Data4` keeps the current record's key value between calls. Statements are prepared per call. That costs some raw throughput, but every query uses the `(key_expr, _rowid)` index, and it's far below interactive thresholds for the Chart of Accounts dataset.

### 4.5 `dbf2sqlite` migration tool

Static library `dbf2sqlite_core/` (all logic) plus the thin console EXE `dbf2sqlite/`. Usage and details: `dbf2sqlite_core/README.md`. Responsibilities:

1. **Read `.dbf` files** via the vendored shapelib reader (`dbf2sqlite_core/vendor/shapelib/`, MIT licence option; §10 Q3). The open goes through a custom file hook, so tables without a `.dbf` extension (`ACCESS.PSW`) work.
2. **Create each table** with `cb4sqlite::createTable` (§4.3: `_cb_fields` plus typed columns). Insert the rows in record order in one transaction, with `_rowid` = record number and the deletion flag kept in `_deleted`.
3. **Convert text** from the source code page to UTF-8: the language-driver byte, `--codepage`, or 1252 by default. Character fields that grow past their byte width are **widened and reported**, never truncated.
4. **Read the index tags** from each table's **production `.mdx`**: the app's CodeBase was built with `S4MDX` (`include/d4all.h`), so dBase IV indexes, not `.ndx`/`.cdx`. Each tag's dBase key expression is translated to a `key_expr` and created with `cb4sqlite::createTag`. Supported: field names, `+` (character or numeric), `UPPER()`, `DTOS()`, `STR()`, `RECNO()`. FOR clauses are translated too: comparisons (with dBase `SET EXACT OFF` prefix matching for text), `.AND.`/`.OR.`/`.NOT.`, literals, `DELETED()`. Descending tags, alias references and other functions exit 4 (`--force` skips them). Memo fields are read from the table's `.dbt` (dBase III or IV layout); a missing `.dbt` exits 4. UNIQUE tags migrate as plain tags; any duplicate keys are an error (or a warning with `--force`).
5. **Verify** by reopening every table through `cb4sqlite::Code4`/`Data4` and comparing record counts.
6. **Write atomically:** the database is built in `<out>.tmp` and renamed into place only if everything succeeded.

The Spec #1 screens touch these tables: `glaccts`, `gldept`, `access.psw`, `accessl`, `accessm`, `gl_adm`, `gl_prof`, `gl_pds`, `gl_setat`. By default the tool migrates **every** dBase table in the directory rather than a fixed list: the migration is one-shot per company, and a partial database would be missing tables later specs need.

**Command-line interface:**

```
dbf2sqlite.exe --src <dbf-dir>  --out <company.db>  [--table <name>]...  [--force]
               [--codepage <n>]  [--headers-out <dir> [--headers-check <dir>]...]

  --src <path>          directory containing the .dbf and .mdx files
  --out <path>          destination SQLite file (must not exist unless --force)
  --table <name>        restrict migration to one table (repeatable). If omitted,
                        migrates every dBase table in --src (detected by header).
  --force               overwrite the destination; skip untranslatable fields/tags with a warning
  --codepage <n>        source code page (default: language-driver byte, else 1252)
  --headers-out <dir>   generate db_<table>.h for tables without a hand-written class (§4.2)
  --headers-check <dir> existing headers/sources to scan for coverage (repeatable)
```

Exit codes: `0` success, `1` invalid arguments, `2` source read error, `3` destination write error, `4` schema/index translation error, `5` row-count verification mismatch.

**Test fixtures:** `dbf2sqlite_core/test/make_fixtures.py` writes `dbf2sqlite_core/test/fixtures/` (deterministically): `company/` (the Spec #1 tables, incl. cp1252 text, a deleted record, blank values, `ACCESS.PSW`), `AR_JSUM` and `AR_BTCH` with the real tables' layouts and tags (FOR clauses, `RECNO()`, a dBase IV memo); `bad/` (memo without `.dbt`, descending / untranslatable-FOR / bad-expression / duplicate-unique tags, widening, unreadable values), and two corrupt cases. They are hand-built, not taken from any production database; see §4.8 for what that leaves unverified.

### 4.6 Semantics & out of scope for Spec #1

**Write semantics (revised in M1, see §4.7):** CodeBase's record-buffer model is kept. `Field4::assign*()` edits the in-memory record. The record is written by `flush()`, or automatically before the cursor moves (`top`/`skip`/`seek`/`go`/`SetIndex`/`appendStart`/`close`), which is when CodeBase wrote a changed record. Each write is one autocommitted `UPDATE`/`INSERT`. The Spec #1 screens depend on this: `seek(); assign…; flush()` to modify, and `appendStart(); assign…; append()` to add.

**Out of scope for Spec #1:**

- Record / file locking — single-user mode only. The `.db` file is assumed to live on the local disk where `gl.exe` runs (UNC paths are unsupported in Spec #1; see risk #10).
- Transactions visible at the API (no `Code4::begin()`/`commit()`/`rollback()`).
- Memo files: memos are migrated into a TEXT column (§4.3) and read/written whole through `Field4memo`; CodeBase's separate memo-file API isn't modelled.
- Custom dBase expression evaluator (only literal expressions used by Spec #1 tables are supported).
- Write-back to `.dbf` (the migration is intentionally one-way).

### 4.7 M1 decisions: CodeBase semantics over the original sketch

Found during M1 (2026-11-06) by reading the CodeBase headers in `include/` alongside the Spec #1 call sites. Each item replaces what the earlier sketch / plan said:

| Topic | Earlier sketch / plan | Implemented (CodeBase behaviour) | Why it matters |
|---|---|---|---|
| Return codes | `r4eof = -22` | `r4success 0, r4found 1, r4after 2, r4eof 3, r4bof 4, r4entry 5` (`d4defs.h`) | ported code compares against these |
| `append()` | inserts an all-NULL row | `appendStart()` → assign → `append()` inserts the buffer; `appendStart()` keeps the current buffer (`blank()` clears) | `gl_acces.cpp`, `gl_profi.cpp`, `gl_accts.cpp` all use this sequence |
| Writes | each `assign` commits immediately; `flush()` no-op | record buffer, written on `flush()` or the next move (§4.6) | `seek; assign; flush` modify pattern |
| `seek` | pad to leading column, compare for equality | partial-key match on the full tag key; `r4after` / `r4eof` on a miss (§4.3) | `gl_accts.cpp` seeks `account + dept`; `gl_profi.cpp` seeks a trimmed key |
| Deleted records | skipped by navigation, excluded from `recCount` | visible and counted; `.NOT. DELETED()` filters; `pack()` removes | the legacy code always calls `pack()` after `deleteRec()`, and filters with `.NOT. DELETED()` |
| Tag metadata | `_cb_tags.order_by_sql` | `_cb_tags.key_expr`, plus a new `_cb_fields` table | seek and ordering need the key value; padding/formatting need widths |
| `Field4::assign(double)` | provided | removed; `assignDouble` / `assignLong` as in CodeBase | `assign(0)` would have been ambiguous |
| `seek(long recNo)` | numeric record seek | `seek(double)` numeric *key* seek, as CodeBase; `go(recNo)` positions by record | — |
| `Date4` | `static today()`, `std::wstring str()` | `void today()`, `const char* str()` | `Date4 d; d.today(); f.assign(d.str());` in `gl.cpp` |
| `Code4::closeAll()` | closes the connection | closes every `Data4`, keeps the connection; new `Code4::close()` closes it | the legacy `closeAll(); init(); reopen` pattern |
| Journal mode | WAL | `DELETE` | one file, so the legacy file-copy backup stays consistent |
| `Relate4set` query | — | also accepts `!DELETED()`; anything else throws | `gl_acctl.cpp` reads the query from `GLDR.INI` (plan risk P4) |

**One behaviour verified only against CodeBase's documented API, not by running the original:** `appendStart()` keeping the previous record's buffer. `gl_accts.cpp` zeroes only numeric fields after `appendStart()` and doesn't assign `category`, so new accounts inherit `category` from the record the cursor was on. That is faithful to CodeBase but may be a latent bug in the original; check it during the M6 Chart of Accounts walkthrough.

**Encoding risk for M2:** dBase widths are bytes in the original code page. Japanese installs (`eiwa/`) store Shift-JIS, where a kanji is 2 bytes; in UTF-8 it is 3. A migrated value can therefore exceed its declared width. The shim truncates without splitting a character, so `dbf2sqlite` must widen character fields (or report the overflow) when converting non-ASCII data.

### 4.8 M2 decisions and what remains unverified

Found during M2 (2026-11-06):

| Topic | Earlier plan / spec | Implemented | Why |
|---|---|---|---|
| Index format | `.ndx` / `.cdx` | dBase IV production `.mdx` only | `include/d4all.h` defines `S4MDX`; that CodeBase build couldn't read `.ndx`/`.cdx` at all |
| Default tables | the four Spec #1 tables | every dBase table in `--src`, detected by header (so `ACCESS.PSW` is found) | one-shot migration; Spec #1 alone touches nine tables (§4.5) |
| Header skip rule | `include/db_<table>.h` exists | scan for `Data4(cb, "<table>")` / `Data4::open(cb, "<table>")` (§4.2) | 8.3 file names don't match table names |
| Fixture location | `dbf2sqlite_core/test/fixtures/<table>/` | `…/fixtures/{company,bad,corrupt_dbf,corrupt_mdx}/`, generated by `make_fixtures.py` | a company directory is what the tool consumes |
| Output | written in place | `<out>.tmp`, renamed on success | a failed run never leaves a half-migrated company |
| Deleted records | — | migrated with `_deleted = 1` | CodeBase showed them until `pack()` (§4.7) |

**Real company data (validated 2026-11-06).** The `.mdx` reader follows the CodeBase `S4MDX` structures in `include/d4data.h` (`I4HEADER`, `T4DESC`, `T4HEADER`), plus one offset from CodeBase's documented layout: tag slots start at byte 544. A real company directory (kept local, not committed) confirmed it: all 45 tables and every `.mdx` tag read correctly. That first run also failed on things the fixtures hadn't covered, all fixed in M2b:

| Found in the real data | Count | Now |
|---|---|---|
| Tags keyed on `RECNO()` / `recno()` | 8 tables | key `_rowid` |
| Tags with FOR clauses (`batchno FOR type = 'AR '`, `TYPE="CA"`, …) | 17 tags | `_cb_tags.filter` + partial index (§4.3) |
| Memo field `AR_BTCH.MEMOFIELD` with a dBase IV `.dbt` | 1 | TEXT column, `Field4memo` |

Data-quality warnings in that directory are left to the owner: `***********` (dBase numeric overflow) and stray text in a few numeric fields are stored as NULL, and six tables declare DOS code page 437 (language driver 0x01). If those installs actually used another code page, migrate with `--codepage`.

**Shim gap found for M5:** `gl_profi.cpp` uses the generated classes in `common/` (`GL_PROF`, `GL_PDS`, `GL_SETAT`, `GLACCTS`), which call `Data4::open(Code4&, name)` and hold `Field4` *values* initialised with `Field4::init`. The shim's `Data4` has no default constructor or `open()` yet. Add both when porting `gl_profi.cpp`.

---

## 5. Unicode & String Strategy

### 5.1 Three-tier rule

- **Tier 1 — Win32 boundary.** Explicit W APIs and `LPCWSTR`. **No `TCHAR`, no `_T()` macros, no `LPTSTR`.** This is a Unicode-only build.
- **Tier 2 — Internal strings.** `std::wstring` replaces the custom `string` class from `cstring.h`. The `new string(...) / delete` heap-allocation pattern in original `gl.cpp` goes away in favor of stack/value `std::wstring`.
- **Tier 3 — Fixed buffers.** `WCHAR szApp[20]` etc. preserved as arrays where the original used `char[]`. Don't gratuitously heap-allocate small fixed-size buffers.

### 5.2 Helpers in `win32fx::StringUtil`

- `std::wstring LoadResString(UINT id)`
- `std::wstring Format(LPCWSTR fmt, ...)` — `swprintf_s` wrapper
- `std::wstring Utf8ToWide(std::string_view)` — for the SQLite UTF-8 boundary
- `std::string  WideToUtf8(std::wstring_view)`
- `std::wstring Trim(std::wstring_view)` — replaces the original `StrTrim`

### 5.3 Resources

- Resource files saved as **UTF-16 LE with BOM**.
- All strings verbatim English. No localization framework introduced in Spec #1.

### 5.4 INI files

Keep `GetPrivateProfileStringW` / `GetPrivateProfileIntW` for Spec #1. Microsoft has deprecated them, but they still work, have zero dependencies, and existing `GLDR.INI` files work as-is.

### 5.5 Out of scope

- Locale-aware comparisons.
- `std::format` conversion (sprintf-style preserved).
- Numeric / currency reformatting (original sprintf format strings preserved).

---

## 6. Resource Strategy & Chart of Accounts Target

### 6.1 Modernize in place — don't rewrite

- Strip `LOADONCALL MOVEABLE DISCARDABLE` (16-bit segment hints; modern `rc.exe` ignores them but we remove them for cleanliness).
- Extract embedded BITMAP hex blobs to external `.bmp` files via a one-time throwaway extraction script.
- Magic style numbers (`40962`, `32784`, `9216`, `29696`) on `tb*` controls are **passed through verbatim** to `CreateWindowExW` as the `dwStyle` word — Spec #1 does not branch on individual bits. Document discovered bits in `controls/styles.md` only when a screen demonstrably depends on one.
- Dialog coordinates (dialog units), control IDs, menu structure, and `STRINGTABLE` entries are **untouched**.

### 6.2 Resource pruning for Spec #1

The pruned `.rc` for Spec #1 — `gl/res/gl.rc` (one file) — is built by **merging** entries from the originals (`gl.rc` and `gl_new.rc`) since some Spec #1 dialogs only exist in `gl_new.rc`:

| Resource | ID | Source file | Notes |
|----------|-----|-------------|-------|
| Main toolbar dialog | 1001 | `gl.rc` | |
| About-box dialog | 1003 | `gl.rc` | |
| Chart of Accounts inner toolbar | 1032 | `gl.rc` | |
| Chart of Accounts main dialog | 1033 | `gl.rc` | |
| Account List dialog | 1034 | `gl.rc` | |
| Company Profile dialog | 1021 | `gl.rc` | `gl_new.rc` also has 1021 with different size — `gl.rc` is canonical for #1 |
| Period Dates dialog | 1028 | `gl.rc` | |
| Login dialog | 1158 | `gl_new.rc` | Not present in `gl.rc` — must come from `gl_new.rc` |
| Administration dialog | 1159 | `gl_new.rc` | **Note:** appears at lines 4300 and 4320 of `gl_new.rc` — duplicate. Use the entry at line 4300; document the choice. (§10 Q1.1) |
| Access Menus dialog | 1160 | `gl_new.rc` | |
| `MANILA` menu | — | `gl.rc` | Top-level frame menu. AP/AR menu items kept (route to stubs); see Q4 below. |
| `GLE` menu | — | `gl.rc` | Merged onto MANILA when a GL MDI child has focus. |
| `GLSUB` menu | — | `gl.rc` | Sub-menu activated for inner GL dialogs. |
| `STRINGTABLE` | — | both | Only the entries referenced by the dialogs/menus above are included. |

**Excluded from the Spec #1 build** (still on disk for reference): the AP / AR menu definitions, the `MEMO` and `SPREAD` menus, all dialogs not listed above. AP/AR menu *items* on MANILA remain (so the user sees the same File menu) and are wired to "not yet ported" stubs.

Original `gl.rc` and `gl_new.rc` files remain in the repo unchanged for reference and for sourcing additional resources in later specs.

### 6.3 The single biggest lever

Custom controls registering with their **original class names** means the dialog manager creates them via `CreateWindowExW(L"tbPic", ...)` and our implementation picks up. **Dialog templates are not edited.**

### 6.4 Bitmap modernization

- One-time script extracts hex-embedded bitmaps to `bitmaps/<id>.bmp`.
- `.rc` declarations rewritten as `<id> BITMAP "bitmaps/<id>.bmp"`.
- Visuals look dated (4-bit, 11px) — acceptable for Spec #1; a future spec replaces them with PNG image lists.

### 6.5 Chart of Accounts port targets

| Source file       | Header(s)             | Becomes                                     | Notes                                |
|-------------------|-----------------------|---------------------------------------------|--------------------------------------|
| `gl/src/gl_accts.cpp` | `gl_accts.h`, `gl_acct.h`, `glclass.h`, `db_dept.h` (missing — §10 Q9) | `gl/port/ChartOfAccounts.h/.cpp` | TGLAcctsAdd, TGLAcctsDelete; dialogs 1033 + 1032; also opens `GLDEPT` via `Tgldept` |
| `gl/src/gl_acctl.cpp` | `gl_acctl.h`, `gl_main.h`              | `gl/port/AccountList.h/.cpp`       | TAccountList; dialog 1034            |

**DB:** `glaccts` table. The only CodeBase tag the Chart of Accounts ports actually use is **`"accounts"`** (verified by `grep SetIndex gl_accts.cpp gl_acctl.cpp`). `gl_acctl.cpp::TListWindow::SetFields` accepts a tag name from the caller (`szIndex`), but every Spec #1 caller of `TAccountList` passes the default; no DESCRIP-tag switch is exercised. Other tags on `glaccts` (if present in the original `.cdx`) are migrated for completeness by `dbf2sqlite` but are not required by Spec #1 acceptance.

### 6.6 Translation pattern (OWL → Win32)

OWL's response-table macros become explicit `OnCommand` cases. Example:

```cpp
// Original (OWL)
class TGLAcctsAdd : public TDialog {
    virtual void Search(RTMessage) = [ID_FIRST + ID_GLINQ];
};

// Ported (Win32 SDK + win32fx)
class ChartOfAccounts : public fx::Dialog {
    bool OnCommand(WORD id, WORD code, HWND ctl) override {
        switch (id) {
            case ID_GLINQ: OnSearch(); return true;
            // ...
        }
        return false;
    }
    void OnSearch();
};
```

Method bodies port near-verbatim with the §4 (DB) and §5 (string) transforms applied.

### 6.7 Acceptance criteria — 9 manual checks

1. **Launch / chrome:** `gl.exe` launches and shows the MDI frame with: (a) the menu identical in label/order to the original GLE menu, (b) the vertical "manila" toolbar present on the left, (c) the status bar showing company name (from company-profile table) and current accounting period.
2. **Chart of Accounts navigation:** GL → Chart of Accounts opens dialog 1033; top / bottom / next / prev navigate `glaccts` rows in the `"accounts"` tag order (matches original; no other tags exercised by these ports).
3. **Add:** Enter required fields (ACCT_NO, DESCRIP at minimum), click Add/Mod → row appears in SQLite (verify externally with `sqlite3 <co>.db "select * from glaccts"`).
4. **Modify:** Navigate to a row, edit fields, click Add/Mod → row updated in SQLite (compare row before/after).
5. **Delete:** Click Delete → `_deleted` flag set in SQLite; navigation skips deleted rows.
6. **Persistence:** Close `gl.exe`, reopen → all changes from steps 3-5 survive across process restart.
7. **Account list:** Dialog 1034 lists accounts ordered by `ACCT_NO`. Selecting a row and confirming returns to the parent dialog with the parent's `ACCT_NO` field populated from the selection (other fields refresh from the DB on parent navigation).
8. **`tb*` validation:** `tbInteger` rejects non-digit input, `tbFloat` enforces at most one decimal point, `tbPic` enforces its mask character-by-character (matches original behavior on the same input strings).
9. **Framework reusability:** The (future) AP project can compile against `win32fx.lib`, declare a trivial `fx::Dialog` subclass with one `OnCommand` handler, and run it as a modal dialog without modifying any `win32fx/` source. Demonstrated by a 30-line example program in `win32fx/examples/trivial_dialog.cpp`.

---

## 7. Build, Errors, Tests, Risks

### 7.1 Build configuration

- **Tooling:** Hand-written `.vcxproj` files (not CMake — Windows-only, MSVC-only).
- **Configurations:** `Debug|x64` and `Release|x64` only.
- **Language:** C++20.
- **Warnings:** `/W4 /WX /permissive-`.
- **Defines:** `UNICODE`, `_UNICODE`, `WIN32_LEAN_AND_MEAN`, `NOMINMAX`.
- **CRT:** Static (`/MT`) for single-exe distribution.
- **SQLite:** Amalgamation vendored with `SQLITE_THREADSAFE=0`, `SQLITE_OMIT_LOAD_EXTENSION=1`, `SQLITE_DEFAULT_FOREIGN_KEYS=1`.
- **Subsystem:** `WINDOWS` for `gl.exe`, `CONSOLE` for `dbf2sqlite.exe`.
- **Release hardening:** `/DYNAMICBASE`, `/NXCOMPAT`, `/guard:cf`, `/sdl`.
- **Manifest:** v6 common controls + per-monitor DPI v2 from day one.

### 7.2 Error handling

The original code has **three error idioms** mixed together: return codes, `MessageBox` + `exit()`, and silent fall-through. Spec #1 does **per-idiom translation** rather than unifying them:

- **Single dispatch-boundary catch.** `win32fx::Window::WndProc` and `win32fx::Dialog::DlgProc` each wrap their entire body in one `try { ... } catch (const std::exception&) { ... } catch (...) { ... }`. Handler bodies do **not** add their own try/catch except where they intend to recover and continue.
- `cb4sqlite` shim throws `cb4sqlite::Error` (derives `std::runtime_error`); ported screens propagate, and the dispatch-boundary catch logs + shows a `MessageBox` + returns to the message loop.
- **`FX_VERIFY(cond, msg)` semantics:** active in both Debug and Release. On failure: in Debug, calls `__debugbreak()` after writing a line to `OutputDebugStringW`; in Release, writes to `OutputDebugStringW` then calls `FatalAppExitW(0, msg)`. It is for programmer-error invariants only — never for user input or recoverable I/O failures.
- `assert()` remains active in Debug (`NDEBUG` is **not** defined for Debug builds) and is a no-op in Release. Use for cheap invariants where `FX_VERIFY` would be overkill.
- **Explicitly avoided:** `Result<T>` / `std::expected` / per-call try-catch around every dispatched call — they would fight the original's calling style and inflate the diff with no real safety gain.

### 7.3 Test layers

| Layer | Required for #1 | Description |
|-------|-----------------|-------------|
| 1     | Yes             | `cb4sqlite_tests.exe` — **Catch2 v3 single-header**, vendored at `cb4sqlite_tests/vendor/catch2/`. Covers: open/close, append + round-trip, tag ordering, seek, navigation, soft-delete, padding-preserved compare. |
| 2     | Yes             | `dbf2sqlite` integration test — runs the migration over the 5-row fixture at `dbf2sqlite_core/test/fixtures/glaccts/`, verifies result with `cb4sqlite` reads. Wired into the same `cb4sqlite_tests.exe` binary as a separate test case. |
| 3     | Yes             | Manual walkthrough of the 9 acceptance criteria from §6.7, signed off in writing. |
| 4     | **No**          | UI automation (UIA / AutoIt / WinAppDriver) — heavyweight, deferred to AP/AR specs. |
| 5     | **No**          | `win32fx` unit tests — proven indirectly by Chart of Accounts working and by the §6.7 #9 reusability example. Adding direct tests deferred. |

### 7.4 Top risks

| # | Severity            | Risk                                                                                     | Mitigation                                                                                            |
|---|---------------------|------------------------------------------------------------------------------------------|-------------------------------------------------------------------------------------------------------|
| 1 | High                | `tb*` reimplementations differ subtly from originals.                                    | Implement minimum behavior; test against Chart of Accounts manually; fix discovered gaps as they appear. |
| 2 | Medium              | Magic style number bits encode unknown behavior.                                          | Treat unknown bits as no-ops; decode only what the screen demonstrably needs.                          |
| 3 | Medium              | `Relate4set` complexity in `gl_acctl`.                                                   | Decide per call site: nested `Data4` cursors vs. real SQL `JOIN`.                                     |
| 4 | Medium              | `.cdx` index expressions like `UPPER(NAME)+ACCT` need an evaluator.                       | Support only what Chart of Accounts uses in Spec #1; defer general expression evaluator.               |
| 5 | High on hi-DPI      | Pixel-based `MoveWindow` calls misalign on per-monitor DPI displays.                      | Per-monitor v2 manifest from day one; `MulDiv` adjustments at hand-coded pixel sites.                  |
| 6 | Low (cosmetic)      | Static CRT + SQLite produces ~1.5–2 MB exe vs. original ~600 KB.                          | Accept.                                                                                                |
| 7 | Medium              | Borland `.rc` dialect rejection by Microsoft `rc.exe`.                                    | Half-day buffer in the schedule for "feed pruned `gl.rc` to `rc.exe`, fix errors."                     |
| 8 | Low–Medium          | `tbSuperBtn` owner-draw visual matching.                                                 | Themed `BUTTON` first; escalate to owner-draw only if visual gap is unacceptable.                      |
| 9 | Medium              | Hidden global state (e.g., `extern PTWindowsObject GetObjectPtr`).                       | Surface via `grep` as ports happen; delete or replace per case.                                        |
| 10| Medium              | SQLite `.db` over UNC / SMB share — known corruption risk under single-user SQLite when the shared file lock is unreliable. Original installs sometimes placed company directories on a network drive. | Spec #1 supports local-disk paths only. If `GLDR.INI` resolves to a UNC path, `Code4::open` returns an error and the login screen shows a "Spec #1: company DB must be on local disk" message. Re-enabling UNC waits on a future spec that adds proper locking. |
| 11| Medium              | Resource encoding round-trip — re-saving the pruned `gl.rc` in Visual Studio's resource editor can rewrite layout (control order, embedded comment loss) and silently break dialog visuals; Borland Resource Workshop output is also non-canonical. | Edit `gl.rc` only as plain UTF-16 LE text in Spec #1; do **not** open it in the VS resource editor. Add a CI / pre-commit check that rejects layout-changing rewrites. |

### 7.5 Spec #2+ deltas (informational)

What is **expected to change** in later specs:

- New screens added as `fx::Dialog` subclasses.
- New `tb*` controls **only if** AP grid screens demand them.
- More `cb4sqlite` API surface (e.g., `Tag4` rebuilds, locking when multi-user is reintroduced).
- More tables supported by `dbf2sqlite` — and therefore **a new `dbf2sqlite.exe` re-release per spec that adds tables**, since the tool's supported-table set is hardcoded per §4.5. Document the version bump in each future spec.
- UNC path support in `cb4sqlite` (paired with locking work).

What is **not expected to change**:

- Build configuration.
- Mini-framework core (`Window` / `Dialog` / `MsgMap` / `HandleGuard`).
- Unicode strategy.
- `Window` / `Dialog` base class shape.

### 7.6 Out of scope for the entire migration

- Web / cloud port.
- Multi-user concurrency.
- Touch UI.
- i18n beyond English resources.
- Replacing SQLite with a server database (the shim is narrow enough to revisit, but not in this migration).
- Automated UI tests.
- Performance work beyond what acceptance criteria force.

---

## 8. Codebase orientation

- **Origin:** 1994 Tegara Corporation General Ledger / AP / AR (Borland C++ OWL, Win16-era).
- **Size:** ~140 `.cpp` files, MDI architecture, three logical modules (GL, AP, AR).
- **Resources:** `gl.rc` (~190 KB) + `gl_new.rc` (~233 KB), Borland Resource Workshop output, hex-embedded bitmaps, custom controls registered at runtime by `ToolBoxLoadDlls`.
- **Stack to remove:** OWL, CodeBase 4, Crystal Reports Print Engine, CTL3D, BWCC, TBOWL/Toolbox DLLs, custom `string` class.
- **Spec #1 source files:** `gl/src/gl.cpp`, `gl/header/gl_main.h`, `gl/src/gl_acces.cpp`, `gl/src/gl_profi.cpp`, `gl/src/gl_accts.cpp` / `gl/header/gl_accts.h`, `gl/src/gl_acctl.cpp` / `gl/header/gl_acctl.h`.
- **Layout:** legacy GL sources live in `gl/src/` (`.cpp`) and `gl/header/` (`.h`) since commit `1606b78`; shared headers in `include/` at the repo root. See §2.1.

---

## 9. Sub-project decomposition (context for future specs)

Spec #1 covers items 1 + 2 of the seven seams identified during decomposition:

1. Foundation / scaffolding (Win32 shell, MDI frame, resource strategy, build, framework). — **Spec #1**
2. Data layer (CodeBase → SQLite shim + `dbf2sqlite` migration tool). — **Spec #1**
3. Reporting (Crystal Reports Print Engine replacement — biggest unknown, deferred).
4. GL module port (remaining screens after Chart of Accounts).
5. AP module port.
6. AR module port.
7. Resource (`.rc`) modernization for unported screens.

---

## 10. Open Questions & Resolutions

This section started as open questions during brainstorming. Most were resolved during a follow-up read of the source. Remaining items are flagged at the end.

### Resolved (locked into spec body)

1. **Login & company-profile dialog IDs.** Verified by `grep` of source:
   - `TGLAccess` (login) → dialog `1158` in `gl_new.rc` only.
   - `TGLAdministration` (admin) → dialog `1159` in `gl_new.rc` only (also at line 4320 — likely a duplicate; pick line 4300 and document).
   - `TAccessMenus` → dialog `1160` in `gl_new.rc` only.
   - `TGLProfile` (company profile) → dialog `1021` (canonical version in `gl.rc`; `gl_new.rc` also has 1021 at different dimensions — choose `gl.rc`'s version).
   See §6.2 table.

   1.1. **`gl_new.rc:1159` duplicate disposition.** Two definitions of dialog 1159 exist (lines 4300 and 4320). Pick one for the pruned build. **Decision: use the entry at line 4300** (first occurrence; cross-check fields match the `TGLAdministration` member references in `gl_acces.cpp`).

2. **`glaccts` index tag names.** Verified by `grep SetIndex gl_accts.cpp gl_acctl.cpp`. The chart-of-accounts ports use exactly one tag: **`"accounts"`**. `dbf2sqlite` will migrate any other tags present on the source `.cdx` (so the SQLite database stays a faithful representation), but Spec #1 acceptance does not depend on them. See §6.5.

3. **DBF reader library final choice.** **Decision (revised 2026-11-06): shapelib `dbfopen.c` + `safileio.c` v1.6.3**, vendored under `dbf2sqlite_core/vendor/shapelib/`, used under its MIT licence option. The original choice, `libdbf`, was dropped during M0 vendoring: the GitHub repo the plan cited does not exist, and the real libdbf (SourceForge) is LGPL, not BSD, which is awkward for a statically linked single-exe distribution. shapelib reads `.dbf` only; `.ndx`/`.cdx` index-expression parsing stays our own code in `dbf2sqlite_core` (§4.5 item 4), as before. See `VENDORED.md`.

4. **AP / AR menu items in the pruned `gl.rc`.** **Decision: keep MANILA menu items routing to AP/AR; AP and AR sub-menu definitions are excluded from the build.** Items 2210 (Accounts Payable...) and 1002 (Accounts Receivable...) on the MANILA File menu route to "not yet ported" stubs in `ApStubs.cpp` / `ArStubs.cpp`. The standalone `AP` and `AR` `MENU` resource definitions (lines 1457 and 1559 of `gl.rc`) are pruned.

5. **Menu-merge behavior in Spec #1.** Menu merging IS exercised — when Chart of Accounts (or any GL MDI child) has focus, the `GLE` and `GLSUB` menus merge onto the MANILA frame menu. The merge is non-trivial in Spec #1; not identity-only. `MdiFrame::OnMdiChildActivate` must implement the merge correctly. Document the merge points in `win32fx/MdiFrame.cpp`.

### Resolved during plan-writing

7. **Per-monitor DPI test display.** **Decision: 96-DPI only for Spec #1 acceptance.** The per-monitor v2 manifest still ships (cheap insurance against future hi-DPI hardware), but the §6.7 manual walkthrough runs on 96-DPI hardware only. High-DPI is documented as untested and caught in a later spec.

8. **`db_accts.h` location.** **Resolved.** The header lives at `<repo root>/include/db_accts.h` (an identical copy is in `common/`). `db_acces.h` lives at `gl/header/db_acces.h`. Spec §2.1 reflects the real top-level layout, and the build's `IncludePath` adds `$(SolutionDir)include;$(SolutionDir)gl\header`. The hand-written header is authoritative. Verified `Tglaccts` shape: 44 fields (accounts/dept/descript/fdescript/type/curr_bal/category + budget1..12 + period1..12 + openbal + previous1..12), plus `dPeriod()`/`UpdateAccount()` member methods (defined in `ap_post.cpp` — not link-required for Spec #1 since unused). M2's generator emits `db_<table>.h` only for tables lacking a hand-written header.

### Still open

6. **`tb*` control behavioral envelope.** Required-to-match behaviors for Spec #1:
   - **Class registration** at original names (locked, §3.6).
   - **Window styles**: pass-through verbatim (locked, §6.1).
   - **`WM_GETDLGCODE`**: tbPic/tbDate/tbInteger/tbFloat must return `DLGC_WANTCHARS | DLGC_WANTARROWS` so character validation runs on every keystroke. tbSuperBtn returns `DLGC_BUTTON | DLGC_DEFPUSHBUTTON` as appropriate.
   - **`WM_NOTIFY` codes**: not used by the original tb* controls (they communicate via `WM_COMMAND` / `EN_*` notifications). Match by emitting the same `EN_CHANGE` / `EN_KILLFOCUS` notifications on edit-derived controls.
   - **Paint / erase**: themed default; escalate to owner-draw only if visual gap.

   Open question: is the above sufficient, or are there concealed message-flow dependencies (e.g., subclass thunks that swallow specific WM_*)? Decide by porting and observing.

9. **`db_dept.h` / `Tgldept`.** **Resolved 2026-11-06.** It was missing from `include/`; the user recovered it in an archive unpacked into `ar/`, and it is now `include/db_dept.h` (copied unchanged from `ar/DB_DEPT.H`): `class Tgldept : public Data4` on table `gldept`, tag `depart`, fields `depart` and `descript`. That matches its use in `gl/src/gl_accts.cpp:12,233`, and it compiles against the M0 shim. Nine more missing headers came from the same source (`db_arbtc`, `db_ardst`, `db_btch`, `db_btchd`, `db_cust`, `db_dist`, `db_itemm`, `db_medrp`, `db_srce`). **Caution:** the `ar/` tree is an *older* snapshot. Its copies of headers already in `include/` differ (e.g. `ar/DB_ACCTS.H` lacks `category` and uses 1-based field indexes), so the `include/` versions remain authoritative and only missing headers were taken from `ar/`. Still missing (non-Spec #1): `db_acct.h`, `db_alloc.h`, `db_arjsm.h`, `db_distd.h`, `db_fa.h`, `db_glopn.h`, `db_index.h`, `db_xacct.h`.

