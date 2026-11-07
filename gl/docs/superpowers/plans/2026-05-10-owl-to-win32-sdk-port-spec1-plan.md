# Spec #1 — Implementation Plan: OWL-to-Win32 SDK Port

**Spec reference:** `docs/superpowers/specs/2026-05-10-owl-to-win32-sdk-port-design.md`
**Date:** 2026-05-10
**Plan status:** Draft

**Paths:** all paths are relative to the repo root (historically `GLSRC/`). Legacy GL sources are in `gl/src/` and `gl/header/` (moved in commit `1606b78`); shared headers in `include/`. See spec §2.1.

This plan operationalizes the locked design in Spec #1. It assumes the design is final; if a milestone reveals a contradiction with the spec, **stop, fix the spec, then resume the plan** — do not silently diverge.

---

## Milestone summary

| # | Milestone                              | Approx. size | Depends on | Critical-path? |
|---|----------------------------------------|--------------|-----------:|----------------|
| 0 | Bootstrap (vendoring, solution skeleton, include path) | S | —          | yes |
| 1 | `cb4sqlite` shim core + Layer-1 tests              | M | M0         | yes |
| 2 | `dbf2sqlite_core` lib + `dbf2sqlite` CLI + header generator | M | M1 (light dep) | yes |
| 3 | `win32fx` mini-framework + custom controls (no menu-merge impl) | L | M0   | parallel with M1/M2 |
| 4 | Resource pruning (`gl/res/gl.rc` + bitmaps)         | S | M3 (control names registered) | yes |
| 5 | `gl.exe` shell: WinMain, MdiFrame, Login, CompanyProfile, stubs | M | M2, M3, M4 | yes |
| 6 | Chart of Accounts + Account List ports              | L | M2, M3, M5 | yes |
| 7 | Acceptance walkthrough + sign-off                   | S | M6         | yes |

S = small (< 1 day), M = medium (1-3 days), L = large (3-5 days). Sizes are wall-clock estimates assuming one engineer.

**Parallelism opportunity:** M3 (`win32fx`) is independent of M1/M2 and can run in parallel from after M0. M4 (resources) only needs M3's *custom-control class-name list* (a documentation artifact decided early in M3), not working control implementations — so M4 can begin once that list is committed, even if M3's controls are still being implemented.

---

## Milestone 0 — Bootstrap

**Goal:** All third-party code vendored. Visual Studio solution opens. Empty build of every project succeeds. `include/` and `gl/header/` wired into `IncludePath` so existing `db_*.h` headers compile against the cb4sqlite shim.

### Tasks

1. **Confirm existing `include/` tree is intact** at `<repo root>/include/`. Spot-check files referenced by `gl/header/gl_acctl.h` and friends:
   - `db_accts.h` (verified: 44 `Field4*` members for `Tglaccts`, plus convenience `dPeriod*`/`dBudget*`/`dPrevious*` doubles, `dPeriod()`, `UpdateAccount()`).
   - Any other `db_*.h` referenced by `gl/src/*.cpp` / `gl/header/*.h` `#include` lines. Run from the repo root:
     ```bash
     grep -rhoiE '^\s*#\s*include\s*"db_[^"]+"' gl/src gl/header | sort -u
     ```
     to enumerate the dependency surface. Each listed header must exist in `include/` or `gl/header/`. Headers needed by Spec #1 screens that are missing are recorded in spec §10 (see Q9, `db_dept.h`); missing headers for later modules are only logged.
2. **Survey other shared `include/` headers** that ports may need: `tlist_w.h` (referenced by `gl_acctl.h:3`), `gl_lista.h` (referenced by `glclass.h:4`), `cstring.h` (custom string class to be retired), `tbowl.h` (custom controls — to be deleted), and any `*.h` `#include`d by `gl_accts.cpp`/`gl_acctl.cpp`. Note which are CodeBase-API-internal (replace via `cb4sqlite`), which are OWL-dependent (replace via `win32fx`), which are `.dbf` schema (keep).
3. **Vendor third-party code.** Drop into the repo, no submodules:
   - SQLite amalgamation (`sqlite3.c`, `sqlite3.h`) → `cb4sqlite/`.
   - shapelib DBF reader (`dbfopen.c`, `safileio.c`, `shapefil*.h`; MIT option) → `dbf2sqlite_core/vendor/shapelib/` (spec §10 Q3).
   - Catch2 v3 amalgamation → `cb4sqlite_tests/vendor/catch2/`. Catch2 supplies `main()`; `parse_smoke.cpp` has none.
4. **Provide M0 stub `cb4sqlite/include/` headers** declaring `Code4`, `Data4`, `Field4`, `Date4`, `Relate4set` types **without bodies** — empty class declarations with the methods listed in spec §4.2 marked `= delete` or empty. Just enough that `db_accts.h`, `db_acces.h`, etc. parse cleanly. Real implementations land in M1.
5. **Create `gl.sln` and the six `.vcxproj` files** at `<repo root>/gl.sln`. Project list per spec §2.1:
   - `win32fx/win32fx.vcxproj` — static lib.
   - `cb4sqlite/cb4sqlite.vcxproj` — static lib.
   - `cb4sqlite_tests/cb4sqlite_tests.vcxproj` — console exe (links `cb4sqlite.lib` and `dbf2sqlite_core.lib`).
   - `dbf2sqlite_core/dbf2sqlite_core.vcxproj` — static lib (DBF parsing, schema generation, header generation).
   - `dbf2sqlite/dbf2sqlite.vcxproj` — console exe (thin CLI wrapper around `dbf2sqlite_core.lib`).
   - `gl/gl.vcxproj` — Windows exe (new sources under `gl/port/`; `gl/src/` is never added to the project).
6. **Apply project settings from spec §7.1** to every project, via the shared `<repo root>/common.props` property sheet:
   - C++20, `/W4 /WX /permissive-`, defines `UNICODE`, `_UNICODE`, `WIN32_LEAN_AND_MEAN`, `NOMINMAX`.
   - **`IncludePath` adds `$(SolutionDir)include;$(SolutionDir)gl\header`** (in that order) so `#include "db_accts.h"` and `#include "db_acces.h"` work unchanged.
   - Configurations `Debug|x64` and `Release|x64` only.
   - Static CRT (`/MT`).
   - Output dir: `$(SolutionDir)x64\$(Configuration)\`.
   - SQLite `.c` compiles with `SQLITE_THREADSAFE=0`, `SQLITE_OMIT_LOAD_EXTENSION=1`, `SQLITE_DEFAULT_FOREIGN_KEYS=1`.
   - Release adds `/DYNAMICBASE`, `/NXCOMPAT`, `/guard:cf`, `/sdl`.
7. **Smoke-compile `db_accts.h`** standalone in a tiny `.cpp` (placed at `cb4sqlite/test/parse_smoke.cpp`) that does `#include "db_accts.h"` against the M0 stub headers. Verify the 44 `Field4*` initializations parse — full link is not required at this stage. Legacy headers are included between `legacy_headers_begin.h` / `legacy_headers_end.h` so their `/W4` warnings (e.g. C4100) don't break `/WX` (spec §2.1).

### Exit criteria

- `gl.sln` opens cleanly in Visual Studio.
- Empty (one-`.cpp`-stub-per-project) Debug|x64 build succeeds with no warnings.
- `cb4sqlite/test/parse_smoke.cpp` (which `#include`s `db_accts.h` and `db_acces.h`) compiles. (Member type names resolve against the M0 stub headers; full link not required.)
- `IncludePath` resolution verified by every project compiling a one-line `#include "db_accts.h"` test stub.

---

## Milestone 1 — `cb4sqlite` shim core

**Goal:** All five shim classes work against a hand-built SQLite database, demonstrated by Catch2 unit tests.

> **As built (2026-11-06):** the tasks below are the original plan. Reading the CodeBase headers in `include/` and the Spec #1 call sites showed several of them contradict real CodeBase behaviour: return-code values, the append sequence, the write model, seek matching, deleted-record visibility, and tag metadata. The implementation follows CodeBase, and spec §4.7 lists every deviation. `cb4sqlite/README.md` has the resulting semantics and the method-coverage table required by the exit criteria.

### Tasks

1. **Implement `Code4`** (open / closeAll / initUndo / handle).
   - `open(path)` opens (or creates) the SQLite file with `sqlite3_open_v2`, sets PRAGMAs (`foreign_keys=ON`, `journal_mode=WAL` for now — revisit if WAL on local disk causes test issues, switch to `DELETE`).
   - `closeAll()` closes the connection and all owned statements.
   - `initUndo()` is a no-op in Spec #1 (CodeBase undo log not modeled).
   - **Note on the `cb` global:** the spec declares `extern Code4 cb` for use across ports. The actual *definition* of `cb` lives in `App.cpp`, which is built in M5. Until M5 lands, M1 unit tests must instantiate their own local `Code4` instance — never assume `cb` is initialized. Document this in `cb4sqlite/README.md`.
2. **Implement `Data4`** (cursor over one table).
   - One owned `sqlite3_stmt*`. `top()`/`bottom()`/`skip(n)` re-prepare the statement with `ORDER BY <tag>` and (`LIMIT 1 OFFSET ...` for `skip`-from-position; or use a `WHERE rowid > ?` cursor for forward iteration to keep skip linear).
   - `seek(const char*)` matches the leading column of the active index against the key. **Right-pad the key with spaces to the leading column's declared width before compare** (per spec §4.3 padding rule). M1 unit test: store a `"100    "` row, call `seek("100")`, expect a hit.
   - `seek(long)` is implemented but Spec #1 ports use only the string overload.
   - `append()` inserts a row with all columns NULL (matches CodeBase semantics: caller then assigns fields and `flush()`es). Returns the new `_rowid`.
   - `flush()` is a no-op in the success path (per spec §4.6); exists for compat compile.
   - `recCount()` runs `SELECT COUNT(*) FROM <t> WHERE _deleted=0`.
   - `recNo()` returns the current `_rowid`.
   - `eof()` is true after `top()`/`skip(n)` walked off the end.
   - `numFields()` returns column count from the prepared statement (`sqlite3_column_count` on a `SELECT *` template).
   - `SetIndex(name)` sets the active tag — looks up `_cb_tags(table_name=<t>, tag_name=<name>)` and stores the resulting `order_by_sql`.
   - `go(rowid)` re-prepares the cursor at the given `_rowid`.
3. **Implement `Field4`** (column accessor).
   - Constructed from a `Data4` plus 1-based index OR column name. Looks up the SQLite column index lazily on first access.
   - `str()` returns `const char*` into a per-Field buffer. Lifetime: until next nav. Document this on the class header.
   - `num()` returns `sqlite3_column_double`.
   - `assign(const char*)` / `assign(double)` / `assignDouble(double)` mark the row dirty; the dirty value is committed on next `flush()` or implicit on next nav.
   - `SetField(HWND, UINT)` reads the field value and `SendDlgItemMessageW(WM_SETTEXT)` to the child control.
   - `AssignField(HWND, UINT)` reads `WM_GETTEXT` from the child and assigns into the field.
   - `len()` and `name()` return cached metadata from `sqlite3_column_*`.
4. **Implement `Date4`** (today + str).
   - `today()` constructs from system time, format `YYYYMMDD`. `str()` returns the buffer.
5. **Implement `Relate4set`** (querySet + iteration).
   - `querySet(expr)` parses the literal predicate. Supported in Spec #1: `.NOT. DELETED()` (the only literal seen in the source) → translates to `_deleted = 0`. Anything else: throw `not_implemented`.
   - `top()` / `skip(long)` / `eof()` iterate. Returns `r4eof` constant (declare `constexpr int r4eof = -22;` to match CodeBase).
   - `unlock()` and `free()` are no-ops in Spec #1.
6. **Define `cb4sqlite::Error`** as `: std::runtime_error`. Construction stamps the offending method name into the message.
7. **Layer-1 unit tests** in `cb4sqlite_tests/`. Catch2 v3.
   - Open / close round-trip.
   - Append + read back via every `Field4` accessor type.
   - Tag ordering: build `_cb_tags` row, verify `top()/skip()` order.
   - `seek` exact / not-found.
   - Soft-delete. *As built:* deleted records stay visible and counted, as in CodeBase; `.NOT. DELETED()` filters them and `pack()` removes them (spec §4.7).
   - Padding-preserved compare (TEXT trailing spaces preserved).
   - Stretch goal: thread-safety smoke (single-thread only — confirm `SQLITE_THREADSAFE=0` doesn't crash in the cb code path).

### Exit criteria

- All Catch2 unit tests in `cb4sqlite_tests.exe` pass on Debug|x64.
- **Every method in spec §4.2** is either covered by a behavioral Catch2 assertion **or** explicitly listed in `cb4sqlite/README.md` as "compile-only stub for compat" (e.g., `Data4::flush`, `Relate4set::unlock`, `Code4::initUndo`). The list of stubs is the answer to "what isn't tested and why."
- Right-pad seek-key behavior is verified by the dedicated test in task 2.
- Compiling a tiny program that exercises every Spec #1 method (per spec §4.2) succeeds with `/W4 /WX`.

---

## Milestone 2 — `dbf2sqlite_core` lib + `dbf2sqlite` CLI + header generator

**Goal:** Migrate a sample chart-of-accounts `.dbf` to SQLite, emit `include/db_<table>.h` headers for tables that lack a hand-written one, verify by reading back through the shim. Migration core lives in a static lib so the test harness can drive it directly without shelling out.

> **As built (2026-11-06):** the tasks below are the original plan. Spec §4.5 and §4.8 describe what was built and why it differs:
> - `.mdx` indexes, since the app was built with `S4MDX`.
> - Every table in the directory migrated by default.
> - The header skip rule detects `Data4(cb, "<table>")` instead of matching file names.
> - Fixtures are generated into `test/fixtures/{company,bad,…}`.
> - Output is written atomically via `<out>.tmp`.
> - **M2b, after the first real-data run:** FOR clauses (tag filters with partial indexes), `RECNO()` keys and dBase IV memo fields (`Field4memo`) are supported, so the real company directory migrates with exit 0.
>
> Usage is in `dbf2sqlite_core/README.md`.

### Tasks

1. **Bring up the shapelib DBF reader** (`DBFOpen` / `DBFGetFieldInfo` / `DBFReadStringAttribute`) in `dbf2sqlite_core/`. Confirm it reads the bundled fixture.
2. **Implement migration core in `dbf2sqlite_core.lib`:**
   - Public API surface in `dbf2sqlite_core/include/Migrator.h`:
     ```cpp
     namespace dbf2sqlite {
         struct Options { /* src dir, out path, table filter, force, ... */ };
         struct Result  { /* table_name → row_count, exit_code */ };
         Result migrate(const Options&);
         Result generateHeaders(const Options&);  // header generator
     }
     ```
   - Parse DBF header → emit `CREATE TABLE` (column types per spec §4.3, **right-pad CHAR values to declared width on insert**).
   - Bulk-insert records inside one transaction.
   - Read `.ndx` / `.cdx` files; parse the indexed expression. Spec #1 supports literal expressions only — `.NOT. DELETED()` and bare column references. Anything else: `--force` overrides with a warning, otherwise abort with exit 4. **`--force` does NOT override the header skip-policy** — hand-written headers are always preserved.
   - **Create tables and tags through `cb4sqlite::createTable` / `createTag`** (`cb4sqlite/include/Schema.h`, built in M1). They write `_cb_fields` (dBase type, width, decimals per field) and `_cb_tags` (`tag_name = .ndx file basename or CDX tag id`, `key_expr`), and create the `(key_expr, _rowid)` index, so navigation is index-backed.
   - **Translate each dBase index expression to `key_expr`:** field references → column names, `+` → `||` (character columns are stored padded, so the concatenation matches the dBase key byte for byte), `UPPER(x)` → `upper(x)`. Anything else: `--force` skips the tag with a warning, otherwise exit 4.
   - **Encoding:** convert character data from the source code page to UTF-8. Where a converted value exceeds the field's byte width (Shift-JIS → UTF-8 for `eiwa/` data), widen the field in `_cb_fields` and report it; never truncate silently (spec §4.7).
3. **Implement header generator in `dbf2sqlite_core` (`include/db_<table>.h`):**
   - **Skip-policy enforcement:** before emission, the generator checks every directory on the schema include path (`include/` and `gl/header/`) — `std::filesystem::exists(dir / ("db_" + table + ".h"))`. If true → log `[skip] include/db_<table>.h (hand-written)` and return success for that table. **Skip-policy is not overridable by `--force`** (which only relaxes the index-expression rule). This is enforced at runtime, not by a hardcoded list.
   - For new headers, emit a `Data4`-derived class declaration.
   - Class name: `T<table>` lowercase (matches existing convention: `Tglaccts`, `TAccess`).
   - Each column → one `Field4*` member named lowercase per the dBase column name.
   - Constructor takes `Code4*`, calls `Data4(cb, "<table>")`, checks `isValid()`, calls `SetIndex(<primary tag>)`, then `new Field4(*(Data4*)this, "<colname>")` per column (string-name form, matching existing `db_accts.h` style).
   - Field-by-string-name avoids the brittle 1-based-index assumption; the shim's column-name lookup happens once on first access (cached by Field4).
4. **CLI in `dbf2sqlite/main.cpp`** — thin wrapper that parses `--src`, `--out`, `--table`, `--force` (per spec §4.5), constructs `dbf2sqlite::Options`, calls `migrate()` then `generateHeaders()`, returns the exit code from `Result`. ~50 lines, no logic of its own.
5. **Layer-2 integration tests in `cb4sqlite_tests.exe`** — `cb4sqlite_tests` links `dbf2sqlite_core.lib` and calls `dbf2sqlite::migrate()` directly from a Catch2 `SECTION`. Run over `dbf2sqlite_core/test/fixtures/glaccts/` (5-row hand-built fixture), then read every row back through `cb4sqlite::Code4` + `Tglaccts`. No process-spawning involved.
6. **Run end-to-end migration on all four Spec #1 tables** as part of the test suite: `glaccts`, `accessm`, `access.psw`, the company-profile table. Use small synthetic fixtures committed under `dbf2sqlite_core/test/fixtures/<table>/`. Assert row counts, an arbitrary field round-trip, and tag ordering for each.

### Exit criteria

- `dbf2sqlite.exe` migrates the 5-row fixture without errors.
- Generator's emission is byte-for-byte stable (deterministic output) — re-running produces no diff.
- All four Spec #1 tables (`glaccts`, `accessm`, `access.psw`, company-profile) round-trip through migration → cb4sqlite read with row-count and field-value verification.
- Generator skips `include/db_accts.h` and `gl/header/db_acces.h` (existing hand-written headers).
- Generator is exercised end-to-end on at least one table that **lacks** a hand-written header (likely the company-profile table, depending on what's in `include/`); the generated header compiles standalone against `cb4sqlite`.
- An index `<table>__<tag>` exists for each migrated tag (verify via `.indexes <table>` in the `sqlite3` shell), and `_cb_fields` / `_cb_tags` rows match the source `.dbf` / index files.

---

## Milestone 3 — `win32fx` mini-framework + custom controls

**Goal:** Framework is complete enough to host login + chart-of-accounts dialogs.

> **As built (2026-11-06):** the tasks below are the original plan; spec §2.2 and §3 describe what was built, and `win32fx/README.md` the details. Where it differs:
> - **Task 7, `tbFloat`:** the number of decimals comes from the control's mask (the template text, e.g. `9999999999999.99` gives 2), not a fixed one decimal. The legacy resources mostly use 2.
> - **Task 7, `tbDate`:** the control keeps the displayed `MM/DD/YY` text, because the legacy code reads it with `GetField` and parses it with `Date4`. `WM_SETTEXT` also takes `YYYYMMDD`, and `DateGetYmd()` returns the `YYYYMMDD` value for cb4sqlite.
> - **Task 6:** `Toolbar` and `VerticalToolbar` are hosted dialog bars built from templates 1032 and 2057, like the originals. `VerticalToolbar` docks at the bottom of the frame. `StatusBar` is a common-controls wrapper. `MdiChild` was added, because the activation hook needs MDI children.
> - **Task 9:** `gl/port/App.cpp` defines the globals. win32fx never references them; it takes an `HINSTANCE` where it needs one.
> - **Exit criteria:**
>   - The two-instance test is `trivial_dialog.exe /selftest`. It covers two dialogs and two MDI children of one class, plus the custom controls.
>   - The smoke `gl.exe` loads the MANILA menu from a stub `gl/res/gl.rc`, which M4 replaces.

### Tasks

1. **`Window` base class** with thunk dispatch via `SetWindowLongPtr(GWLP_USERDATA)` set in `WM_NCCREATE`.
   - Static WndProc trampoline; per-instance pointer stored on first message.
   - Virtual handlers: `OnCreate`, `OnCommand`, `OnSize`, `OnDestroy`, `OnMessage`. Default impls return `false` (not handled).
2. **`Dialog` base class** parallel to `Window` for `DialogBoxParamW` / `CreateDialogParamW`.
   - Thunk on `WM_INITDIALOG`. Virtual `OnInitDialog`.
3. **Message-map macros** (`BEGIN_MSG_MAP` / `ON_MSG` / `END_MSG_MAP`) for `WM_NOTIFY` and `WM_CTLCOLOR*` cases that virtuals don't cover.
4. **`HandleGuard<H, Deleter>` template** instantiations: `HMENU`, `HFONT`, `HBITMAP`, `HBRUSH`, `HPEN`, `HRGN`, `HICON`.
5. **`StringUtil`**: `LoadResString(UINT)`, `Format(LPCWSTR, ...)`, `Utf8ToWide`, `WideToUtf8`, `Trim`. UTF-8 boundary helpers exercised by `cb4sqlite` integration in M5/M6.
6. **MDI scaffolding**: `MdiFrame`, `Toolbar`, `VerticalToolbar`, `StatusBar`. `MdiFrame` defines a virtual `OnMdiChildActivate(HWND child)` hook — but the **menu-merge implementation is deferred to M5**, where actual MDI children exist to test against. M3 ships the empty hook only. Document this on the `MdiFrame.h` header so M5 knows where to drop in the merge logic.
7. **Custom controls** in `win32fx/controls/`:
   - `RegisterCustomControls()` registers `tbPic`, `tbDate`, `tbFloat`, `tbInteger`, `tbSuperBtn` with the original class names per spec §3.6.
   - `tbPic` — mask-validated edit. Handle `WM_CHAR` to validate against the mask string before passing to `EDIT`.
   - `tbDate` — date edit. Accept `MM/DD/YYYY`, normalize to `YYYYMMDD` for cb4sqlite.
   - `tbFloat` — one-decimal numeric edit.
   - `tbInteger` — integer-only edit.
   - `tbSuperBtn` — themed `BUTTON` first; fall back to owner-draw if Spec #1 visual gap is unacceptable.
   - All controls return `DLGC_WANTCHARS | DLGC_WANTARROWS` from `WM_GETDLGCODE` (per spec §10 Q6).
8. **Reusability example:** `win32fx/examples/trivial_dialog/` — a **standalone** Visual Studio project (its own `.vcxproj`, **not** part of `gl.sln`) that consumes the released `win32fx.lib` from `$(SolutionDir)x64\$(Configuration)\`. Declares a `fx::Dialog` subclass, runs it modal. Builds against `win32fx.lib` exactly as the future AP / AR projects will — proving the framework is consumable by an outside consumer, not just by gl-sln-internal code. Satisfies acceptance §6.7 #9.
9. **Globals declared** in App.cpp (per spec §3.7): `extern HINSTANCE hInst;`, `extern HMENU HGLMenu;`, `extern Code4 cb;`, `extern WCHAR szApp[20];`. The framework references these as `extern` only — never owns or initializes them.

### Exit criteria

- `win32fx.lib` compiles `/W4 /WX`.
- The `trivial_dialog/` standalone solution builds and runs **against the released `win32fx.lib`** (acceptance §6.7 #9 — proves reusability, not just internal use).
- Manual smoke: a debug `gl.exe` stub that just creates an `MdiFrame` window with the MANILA menu and a status bar runs without crashing.
- Two-instance dispatch test (per plan risk P2) passes: open two instances of the same dialog class concurrently, verify `OnCommand` lands on the correct `this` pointer for each.

---

## Milestone 4 — Resource pruning

**Goal:** `gl/res/gl.rc` plus extracted bitmaps compile cleanly through `rc.exe`.

> **As built (2026-11-07):** spec §6.2 has the corrected resource table. Where M4 differs from the tasks below:
> - **Tasks 1, 2, 4:** one script, `gl/res/tools/prune_rc.py`, does the extraction, merging and rewriting. It is kept in the repo so later specs can add dialogs. The source is **`gl_new.rc`, not `gl.rc`**: the legacy code matches `gl_new.rc` (Company Profile 1021, Chart of Accounts 1010, toolbar 1032), and `gl.rc` adds only one bitmap and one string. The symbolic IDs resolve through the legacy headers into `gl/res/resource.h`. `gl.rc` stays UTF-16 LE with BOM (spec §5.3).
> - **Task 2, dialog set:** the draft table had wrong IDs. The Chart of Accounts is 1010, not 1033; 1033 is Account Setup, and 1034 is Cash Accounts. About is 1008, and 1001 is Fixed Assets Disposal. `TAccountList` has no template. 1159 uses the second, complete copy.
> - **Task 2, other resources:** all bitmaps (83) and all strings (408) are kept. MANILA comes from `gl_new.rc`, so its AP/AR module items 2210/1002 are gone (spec §10 Q4, revised).
> - **Task 3:** the manifest was already added in M3.
> - **Task 5:** every bitmap the toolbar (`SetPic`), `SetBitmap` and `LoadBitmap` calls in the Spec #1 sources name is present. There are two exceptions, both already missing in the originals:
>   - 1208 (`ID_SEARCH` in `gl_acces.cpp`) is in neither file.
>   - 1000/1002 are no longer controls in `gl_new.rc`'s 2057.
> - **Task 6:** `windres` compiles `gl/res/gl.rc` cleanly on Linux. `rc.exe` runs in the Windows build.
> - **Task 7:** the file is `win32fx/src/controls/styles.md`. It already lists the style words that the Spec #1 dialogs use, decoded.
> - **Exit criteria:** the debug program is `gl.exe /checkres [/quiet]`. It creates all 13 dialogs, with 1010 standing in for 1033 as the Chart of Accounts, and loads the menus, bitmaps, icons, accelerators and sample strings. 1011 and 1160 contain `tbSpread`. They are reported as present but can't be created, because win32fx doesn't register `tbSpread`.

### Tasks

1. **Throwaway bitmap-extraction script** — Python or PowerShell. Reads `gl/gl.rc` and `gl/gl_new.rc`, writes each hex-embedded `BITMAP` block to `gl/res/bitmaps/<id>.bmp`.
2. **Build the merged pruned `.rc`** per spec §6.2 table. Cherry-pick from both source files. Strip `LOADONCALL MOVEABLE DISCARDABLE`.
3. **Add manifest** `gl/res/manifest.xml` — v6 common controls + per-monitor DPI v2.
4. **Rewrite `BITMAP`** declarations to reference external `.bmp` files.
5. **Verify vertical-toolbar bitmaps** referenced by the `MANILA` frame's manila-tab toolbar (the original "vertical toolbar" buttons) are in the pruned `.rc`. Cross-check against `gl/header/gl_main.h` / `gl/header/gl_toolb.h` (and `gl/src/gl_toolb.cpp`) for the bitmap IDs used; ensure each lands in `gl/res/bitmaps/`.
6. **Smoke compile** `gl/res/gl.rc` standalone via `rc.exe` with `gl/res` as include path. Half-day buffer in the schedule for Borland-dialect rejection (spec risk #7).
7. **`win32fx/controls/styles.md`** — create the empty file in M4 with placeholder structure. **Populated incrementally during M5 and M6** as `tb*` style bits force discovery; not a M4 deliverable beyond file creation.

### Exit criteria

- `rc.exe` compiles `gl/res/gl.rc` to `.res` with no errors.
- A debug program that calls `LoadDialogW` for **each** of: `1033` (chart of accounts), `1158` (login), `1021` (company profile), returns a valid HWND for all three.

---

## Milestone 5 — `gl.exe` shell, login, company profile, stubs

**Goal:** `gl.exe` launches, user can log in, status bar shows company name and accounting period, AP/AR/Reporting menu items show "not yet ported."

> **As built (2026-11-07):** `gl/README.md` describes what runs. Spec §2.4, §4.3, §5.4, §6.7 #1 and §10 Q5 were corrected to match the original code. Where M5 differs from the tasks below:
> - **Task 1, startup order.** The original created the frame first. It then posted `ID_SYSACCESS` for the login, and only when `GL.INI [ACCESS] PSW=T`; the real company's `GL.INI` has `PSW=F`, so it never logged in. Company Profile opened only from the File menu. The port does the same: `GLDR.INI` → open `cb` → load menus → frame → login if `PSW=T`.
>   - The `GL` accelerator table is not loaded. The original never loaded it, and it maps Tab to Print.
>   - A second `gl.exe` brings the first to the front, as `hPrevInstance` did.
> - **Task 2.** The fixture is `gl/test/dev-fixture/`:
>   - `make_dev_fixture.py` writes `dbf/`, the real tables' layouts and tags with made-up rows.
>   - `setup.cmd <Config>` migrates them to `x64\<Config>\dev.db` and copies `GLDR.INI` and `GL.INI` beside `gl.exe`.
>   - Users: ADMIN/ADMIN sees everything; CLERK/clerk has the Maintenance menu grayed.
>
>   No bypass flag was needed. The real company directory has no `ACCESS.PSW`, `ACCESSL` or `ACCESSM`, so only the fixture can test the login.
> - **Task 3.**
>   - **Status bar:** the original's status line is the module bar 2057 (fields 3222 company, 3223 menu help, 3224 period). `HStatusBar` was never created. The port uses 2057 and no common-controls status bar.
>   - **Menu merge:** the original swapped MANILA for GLE, with `WM_MDISETMENU`, only while the spreadsheet child (`gl_xls.cpp`) was active. GLSUB was the right-click popup. `ManilaGL` does the same for any child that calls `UseGleMenu`. See spec §10 Q5, revised.
>   - **Stubs:** commands are routed by the legacy response tables: report handlers go to `ReportingStubs`, the `gl_man.h` AP/AR handlers to `ApStubs`/`ArStubs`, and the remaining GL commands get a general message. The lists are in `ManilaGL.cpp`.
> - **Task 4.**
>   - `gl_acces.cpp`'s `TGLAdministration` (1159, Maintenance > User Access) is ported too.
>   - `TAccessMenus` (1160) is not: it is a `tbSpread` grid, and its button says so.
>   - `db_acces.h` is used unchanged. Its constructors leak their `Field4`s, which the port's subclasses delete.
> - **Task 5.**
>   - The Company Profile children are ported too: Period Dates (1028), Account Setup (1033) and Cash Accounts (1034, without its `tbSpread` list 1011).
>   - `TGLProfile::GetCalc` is left out on purpose. On a right-click it zapped every table listed in the `clear` table.
>   - `gl_prof.hpp`/`gl_pds.hpp`/`gl_setat.hpp` don't compile as C++ (`open(Code4&)` has no return type), so `gl/port/Tables.h` re-declares them.
> - **Task 7.** The INI files are read from the directory of `gl.exe`, not the Windows directory. `[GL_DIR] DIR` names the `.db` file, and `DRIVE` is ignored. UNC paths and mapped network drives are refused before the database is opened (`gl/port/Ini.cpp`).
> - **Translation base.** `gl/port/MainDialog` ports the `TMainDialog` helpers (`GetField`, `SetField`, `GetDate`, `SetBitmap`, `Cancel`) that M6 reuses. `gl/port/LegacyIds.h` includes the legacy `gl.h`/`gl_manil.h`, so the original ID names stay.

### Tasks

1. **`App.cpp`** — `WinMain`, `_tWinMain`. Sequence (order matters): (a) register custom controls via `win32fx::RegisterCustomControls()` — **must happen before any `LoadDialogW` or `DialogBoxParamW`**, since dialog templates reference `tbPic`/etc. by class name; (b) define and `open()` the global `cb` (`Code4`); (c) load `HGLMenu` via `LoadMenuW(hInst, L"MANILA")`, plus `GLE` and `GLSUB` menus, into respective `extern HMENU` globals; (d) load `GLDR.INI`; (e) drive login → profile → MDI frame.
2. **Seed an initial test database** to make first-run actually loginable. Two-step task:
   - Provide a small dev-mode fixture in `gl/test/dev-fixture/` containing minimal `accessm.dbf` + `access.psw` rows (1 admin user) plus a stub company-profile row.
   - Document the bootstrap procedure in `gl/README.md`: run `dbf2sqlite.exe --src gl\test\dev-fixture --out <co>.db`, point `GLDR.INI` at the result, then `gl.exe` can log in.
   - Alternative: a `--dev-bypass-login` command-line flag on `gl.exe` that skips authentication for development. Default off; never built into Release. Implement only if the fixture path proves friction.
3. **`ManilaGL.h/.cpp`** — MDI frame subclass.
   - Loads MANILA menu, attaches toolbars and status bar.
   - **Implements menu-merge** in `OnMdiChildActivate(HWND child)` (the hook M3 created): when the active child has an `HMENU` to merge, it replaces popups on the frame menu (per the original `MANILA` ↔ `GLE` ↔ `GLSUB` pattern). Spec #1 only exercises the `MANILA + GLE` case (Chart of Accounts) and `MANILA + GLE + GLSUB` (Account List inside Chart of Accounts).
   - Status bar reads company name from company-profile table and current period from `cb.ini`-equivalent → renders both.
   - Menu IDs 2210 (AP) and 1002 (AR) and any reporting items route to `ApStubs::Show()`/`ArStubs::Show()`/`ReportingStubs::Show()` respectively (a `MessageBoxW` saying "not yet ported in Spec #1").
4. **`Login.h/.cpp`** — port of `gl_acces.cpp`'s `TGLAccess`.
   - Translates the OWL response-table macros (`= [ID_FIRST + IDOK]` etc.) into `OnCommand` cases per spec §6.6.
   - Uses `cb4sqlite::TAccess` (existing `db_acces.h` is structurally compatible — confirm column order matches what M2 produces for tables without hand-written headers, even though `db_acces.h` itself is hand-written).
   - Dialog ID 1158.
5. **`CompanyProfile.h/.cpp`** — port of `gl_profi.cpp`'s `TGLProfile`. Dialog ID 1021. Status-bar display reads the company name from this dialog's underlying table.
6. **Stubs** in `gl/stubs/` — three `.cpp` files (`ApStubs.cpp`, `ArStubs.cpp`, `ReportingStubs.cpp`), one `Show()` function each. **Stubs depend on nothing in `cb4sqlite` or `win32fx` beyond `MessageBoxW`** — confirm by inspection. They exist solely to keep MANILA menu items wired without dragging in unported code.
7. **`GLDR.INI` handling** — `GetPrivateProfileStringW` per spec §5.4. Reject UNC paths per risk #10 with the `MessageBoxW` "Spec #1: company DB must be on local disk."

### Exit criteria

- `gl.exe` launches → MANILA frame with menu, vertical toolbar, status bar.
- Login dialog accepts the seeded admin user from the dev-fixture-migrated `accessm`/`access.psw` tables; rejects an incorrect password.
- **Status bar shows company name read from the migrated company-profile table** (not a hardcoded string) — verified by editing the company-profile row and observing the change after relaunch.
- Status bar shows current period. Acceptance §6.7 #1 passes.
- AP/AR/Reporting menu items each show their stub MessageBox.
- Menu merge: with no MDI child active, MANILA menu is the frame menu unchanged. (Full merge behavior verified in M6 once a child exists to merge from.)

---

## Milestone 6 — Chart of Accounts + Account List

**Goal:** Full Chart of Accounts CRUD round-trip, navigation, account-list popup. Acceptance §6.7 #2-#8.

> **As built (2026-11-07).** The tasks below were drafted before the original code was read; where they differ, the original wins:
> - **Dialog 1010** has no top/bottom/next/prev buttons and no `ID_ADD`/`ID_MODIFY`/`ID_VIEW`. Its commands are Add/Mod (`IDOK`), Search (`ID_GLINQ`), Set Data (`ID_SETDATA`), Delete (`ID_DELETE`), Close, the three type radios and Show Foreign. Navigation is through the account list and Set Data (spec §6.7 #2, corrected).
> - **Delete** is `deleteRec()` + `pack()`, so the row leaves the table (spec §6.7 #5, corrected).
> - **The account list** is `TAccountList` (defined in `gl_btche.cpp`): a captioned window owned by the dialog, holding a `tbSpread` grid, not dialog 1034. The port's grid is a ListView (`gl/port/AccountList.cpp`).
> - **No menu merge** (spec §10 Q5, resolved). The Chart of Accounts is modal and the list is an owned popup; neither is an MDI child, so the frame menu stays MANILA. The exit check below is replaced.
> - **Code:** `gl/port/Accounts.h/.cpp` holds the table classes and record operations without Win32 (checked on Linux). `gl/port/ChartOfAccounts.cpp` is the dialog: `TGLAcctsAdd` and `TGLAcctsDelete`, wired to `ID_ACCTADD` (Accounts > Chart of Accounts, GLSUB > Add Accounts) and `ID_ACCTDEL` (GLSUB > Delete Accounts).
> - **Stress and leaks:** `gl.exe /stress [/quiet]` opens and closes the dialog 100 times and returns the GDI + USER object growth. Debug builds report CRT leaks at exit.

### Tasks

1. **`ChartOfAccounts.h/.cpp`** — port of `gl_accts.cpp` (`TGLAcctsAdd`, `TGLAcctsDelete`).
   - Ctor opens `Tglaccts`. **Do not** repeat `SetIndex("accounts")` — `Tglaccts`'s own ctor (in `db_accts.h`) already calls it. The original `gl_accts.cpp:52, 119, 232, 260` calls to `SetIndex("accounts")` were defensive after navigation that could have left a different tag active; in the port, these calls remain only at points where another part of the system *might* have changed the tag (currently: nowhere, since Spec #1 has no other glaccts users). Document this as `// SetIndex("accounts") — already active per Tglaccts ctor; retained from original for defensive symmetry`.
   - `OnCommand` handles `IDOK`, `ID_ADD`, `ID_MODIFY`, `IDCANCEL`, `ID_DELETE`, `ID_NEXT`, `ID_BOTTOM`, `ID_PREVIOUS`, `ID_TOP`, `ID_UPDATEWINDOW`, `ID_CLEAR`, `ID_RESET`, `ID_VIEW`, `ID_SEARCH`, `ID_FOREIGN`.
   - Dialog IDs: 1033 (main) + 1032 (inner toolbar).
2. **`AccountList.h/.cpp`** — port of `gl_acctl.cpp` (`TAccountList` + `TListWindow` helpers).
   - `TListWindow::SetFields` ported as-is; takes `(const char* table, const char* indexTag, int fieldCount)`.
   - `TListWindow::Redraw` uses `Relate4set::querySet(".NOT. DELETED()")` (the only literal predicate observed).
   - `HandleListBox` → `OnNotify` for `SSM_DBLCLK`.
   - Dialog ID 1034.
3. **Wire to GLE/GLSUB menus** — `GLE` activated when Chart of Accounts has focus; `GLSUB` when an inner dialog is up. Verify menu merge in `MdiFrame::OnMdiChildActivate` does the right thing.
4. **Smoke test each acceptance criterion** as code lands:
   - #2 navigation
   - #3 add (verify with `sqlite3 <co>.db "select * from glaccts"`)
   - #4 modify
   - #5 delete (`_deleted` flag)
   - #6 persistence
   - #7 account list selection
   - #8 `tb*` validation

### Exit criteria

- Acceptance §6.7 #2-#8 pass on Debug|x64.
- **Leak check via `_CrtDumpMemoryLeaks`** (Debug, at exit) over the full sequence: open dialog → add a row → modify a row → delete a row → Search / pick rows in the account list → Set Data → close dialog → reopen → close. No reported leaks.
- `gl.exe /stress /quiet` exits 0 (no GDI/USER object growth over 100 open/close cycles).
- ~~Menu merge verified: opening Chart of Accounts merges `GLE` onto `MANILA`; opening the Account List child within Chart of Accounts merges `GLSUB`; closing each restores the prior menu.~~ *Replaced (spec §10 Q5):* the frame menu stays MANILA while the Chart of Accounts and its account list are open, and GLSUB stays the right-click popup.

---

## Milestone 7 — Acceptance walkthrough + sign-off

**Goal:** Recorded sign-off on all 9 acceptance criteria.

### Tasks

1. **Build `gl.exe` Release|x64.** Confirm `/DYNAMICBASE /NXCOMPAT /guard:cf /sdl` are active in the linker output.
2. **Smoke-test on a fresh Windows install** (not the dev machine). Pin to **Windows 11 23H2 x64** at 96 DPI (or Windows 10 22H2 x64 if Windows 11 is unavailable). Copy `gl.exe`, `dbf2sqlite.exe`, an empty `GLDR.INI`, the migrated `<company>.db`, and confirm launch + Chart of Accounts CRUD.
3. **Walk through each §6.7 criterion** with a witness; record pass/fail and any deviation in a sign-off file at `docs/superpowers/specs/2026-05-10-spec1-acceptance.md`.
4. **Triage gaps** — anything failing acceptance is either fixed in Spec #1 (if scope-internal) or filed as a Spec #2+ task.

### Exit criteria

- `docs/superpowers/specs/2026-05-10-spec1-acceptance.md` exists, all 9 criteria signed off (or accepted-with-known-gap recorded for the hi-DPI exclusion).

---

## Cross-cutting checklists

### Per-port translation discipline (applies in M5 and M6)

For every OWL `.cpp` file ported, in order:

1. `_CLASSDEF(...)` → delete (no OWL pointer typedefs).
2. `class T... : public TMainDialog` / `TDialog` → `class ... : public fx::Dialog`.
3. Response-table macros (`virtual void X(RTMessage) = [ID_FIRST + N];`) → `OnCommand` cases per spec §6.6.
4. `new string(...)` / matching `delete` → `std::wstring` value.
5. `LPSTR` parameters that hold ANSI text from CodeBase → `const char*` on the cb4sqlite boundary; `LPCWSTR` everywhere else.
6. `MessageBox(...)` → `MessageBoxW(...)`.
7. `GetField(ID)` / `SendDlgItemMessage(.., WM_SETTEXT, ...)` → unchanged signature, W variant.
8. **Lifecycle scan** (per risk P6): if the original class overrides `SetupWindow` or `CleanupWindow`, verify the ported `WM_INITDIALOG` / `WM_DESTROY` handlers preserve the original ordering — `SetupWindow` runs after `CreateWindow` but before show; `CleanupWindow` runs *after* `WM_DESTROY`. Pay extra attention to `Field4*` member lifetimes that may be acquired in `SetupWindow` / released in `CleanupWindow`.
9. **Stress smoke-test** the ported screen by open / close 100 times in a row. No crashes, no leaks, no GDI-handle exhaustion.
10. After mechanical translation, run the screen and exercise it against the manual checks in §6.7.

### Per-milestone gates

Before declaring a milestone done:

- [ ] `/W4 /WX` clean on Debug|x64 and Release|x64.
- [ ] Tests for the milestone's layer pass.
- [ ] Spec drift check: re-read the relevant spec section. If reality diverged, edit the spec, then proceed.
- [ ] Update `docs/superpowers/plans/2026-05-10-progress.md` (running log).

### What NOT to do

- Don't add features beyond the acceptance criteria.
- Don't generalize the cb4sqlite API beyond what current call sites need.
- Don't introduce CMake, vcpkg, conan, or any package manager.
- Don't open `gl/res/gl.rc` in the VS resource editor (risk #11).
- Don't replace `MessageBoxW` with a "modern" custom dialog framework.
- Don't backport Spec #2+ work (AP/AR/Reporting) into Spec #1 stubs beyond the "not yet ported" message.

---

## Risks specific to the plan (vs. design risks in spec §7.4)

| # | Severity | Risk | Mitigation |
|---|----------|------|------------|
| P1 | Low | A hand-written `db_*.h` references a field name that doesn't actually appear in the migrated SQLite schema (e.g., a typo or an obsolete column). | M1 unit tests open every Field4 by name on a real glaccts row and surface the mismatch as a test failure with the offending column name. Trivially fixable. |
| P2 | Medium | M3 thunk dispatch has a subtle bug where messages route to the wrong instance (e.g., during `WM_NCCREATE` race or for `MDIChild` reparenting). | Add an explicit unit-style test in `win32fx/examples/trivial_dialog.cpp` that opens two instances of the same dialog class concurrently and verifies `OnCommand` lands in the right `this`. |
| P3 | Low | Original code uses 1-based-index `Field4` ctors at offset arithmetic sites (`Field4(d, i+7)`) — generator's name-based emission would lose this. | Existing hand-written `db_accts.h` already uses string-name ctors throughout — no risk for tables with hand-written headers. Generator's name-based emission is consistent. Index-form ctors remain supported for compat. |
| P4 | Medium | M6 `Relate4set::querySet` literal-only support breaks if `gl_acctl.cpp::TListWindow::SetupWindow` reads a custom `QUERY` from `GLDR.INI` (it does — line 304 `GetPrivateProfileString(... "QUERY" ...)`). | If the read returns `""`, code already defaults to `.NOT. DELETED()` (line 309). Spec #1 ships with an empty `[ACCOUNTLIST] QUERY=` entry — never customize. Document this constraint in the user-facing release notes. |
| P5 | Low | M7 walkthrough on a 96-DPI box hides a hi-DPI bug that surfaces in AP/AR Spec ports. | Accept; manifest still ships per-monitor v2; first hi-DPI bug becomes a Spec #5/#6 task. |
| P6 | Medium | OWL `SetupWindow`/`CleanupWindow` lifecycle does not 1:1 map to `WM_INITDIALOG`/`WM_DESTROY`. `SetupWindow` runs after `CreateWindow` but before show; `CleanupWindow` runs *after* `WM_DESTROY`. Mechanical translation may produce dialogs that initialize too late or destruct too early, causing initialization-order or use-after-free bugs invisible until acceptance. | Per-port checklist (cross-cutting section) gets a new step: "before declaring a port done, scan for `SetupWindow`/`CleanupWindow` overrides and verify the WM_INITDIALOG/WM_DESTROY mapping doesn't break ordering against the original behavior (in particular for nested dialogs and Field4 lifetime)." Add a smoke test: open + close Chart of Accounts 100 times in a row; no crashes, no leaks. |
| P7 | Resolved | Earlier draft had `cb4sqlite_tests.exe` depending on `dbf2sqlite/` source — a layering inversion, since `dbf2sqlite` was a console exe. | Resolved during this self-review by splitting `dbf2sqlite` into `dbf2sqlite_core.lib` + a thin `dbf2sqlite.exe` CLI. Tests now link the lib directly. Spec §2.1 and plan M0/M2 updated. |

---

## Out of plan (intentionally)

These are real concerns that are explicitly NOT this plan's job:

- Crystal Reports replacement → Spec #3.
- AP / AR module ports → Specs #5 / #6.
- TBOWL grid controls → unblocked when AP/AR demand them.
- Locking, multi-user, UNC paths → future spec paired with cb4sqlite locking surface.
- Hi-DPI per-monitor v2 actual testing → first AP/AR spec on hi-DPI hardware.
- Performance work → none planned; if profiling reveals a hot path, file a follow-up spec.
- Replacing the static-CRT bloat → accept (spec risk #6).
- UI automation harness → not in Spec #1.

---

## How to use this plan

Work through the milestones in dependency order: M0 → (M1, M3 in parallel) → M2 → M4 → M5 → M6 → M7. Each task above is a unit of work for `TaskCreate`/`TaskUpdate`. Treat the per-milestone gates as a checklist that has to be green before moving on.

If the plan and the spec conflict, **stop**, edit the spec, re-validate the plan, and continue. Don't paper over.
