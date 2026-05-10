# OWL → Win32 SDK Port — Spec #1 (Foundation + Data Layer + Chart of Accounts)

**Status:** Draft for review — 2026-05-10
**Author:** Brainstormed with Claude
**Scope:** Spec #1 of a multi-spec migration of the 1994 Tegara OWL accounting suite (GL/AP/AR) to a strict Win32 SDK application.

---

## 0. Context

`/home/mst/Projects/GLSRC/` is a 1994-era Borland C++ accounting suite (Tegara Corporation General Ledger / Accounts Payable / Accounts Receivable). Layout:

```
GLSRC/
├── gl/        (~140 .cpp files — General Ledger module)
├── ap/        (AP.IDE + APSRC.LZH — Accounts Payable, source archived)
├── ar/        (AR.IDE + ARSRC.LZH — Accounts Receivable, source archived)
├── common/    (gl_acct.cpp, gldbf.cpp, gl_prof.cpp, gl_pds.cpp,
│               gl_setat.cpp, utildlg.{rc,h,prj} — shared across modules)
├── include/   (CodeBase 4 headers d4all.{h,hpp}, d4data, d4declar, d4defs,
│               e4defs/error/expr, f4flag, r4*, s4*, o4opt, p4port;
│               Toolbox tbowl.h, toolbox.h, ctrl4*.h, tlist_w.h;
│               Crystal Reports crpe.h; per-module table schemas db_*.h)
├── owl/       (16-bit .lib files: owl100wl.lib, crpe.lib, ctl3d.lib,
│               dbf.lib, tclass1.lib, tools.lib, vtssdll.lib — discarded)
├── db/        (.LZH archives of dBase schema definitions)
├── scs/       (separate "scs" app — out of scope)
└── eiwa/      (likely localization / customer build — out of scope for #1)
```

The application stack to remove:
- **OWL** (Borland ObjectWindows) — `TApplication`, `TMDIFrame`, `TWindow`, `TDialog`, `TToolBar`, response-table macros.
- **CodeBase 4** (Sequiter) — `Code4`, `Data4`, `Field4`, `Date4`, `Relate4set`, dBase-format `.dbf`/`.ndx`/`.cdx` files.
- **Crystal Reports Print Engine** (`crpe.h` / `crpe.lib`) — replaced with stubs in #1; future spec for a real solution.
- **CTL3D** — Win10/11 has 3D look natively; deleted.
- **BWCC** (Borland Windows Custom Controls) — deleted.
- **Toolbox / TBOWL / VTSS / Sequiter ctrl4** — provider of `tbPic`, `tbDate`, `tbFloat`, `tbInteger`, `tbSuperBtn` controls; reimplemented (see §3.6).
- Custom `string` class from `cstring.h` — replaced by `std::wstring`.

Spec #1 is the first of seven implementation specs identified in brainstorming:

| # | Spec | Status |
|---|---|---|
| 1 | Foundation + data-layer shim + one validating GL screen | this spec |
| 2 | Reporting strategy (Crystal Reports replacement) | future |
| 3 | GL module port (rest of GL) | future |
| 4 | AP module port | future |
| 5 | AR module port | future |
| 6 | Resource (.rc) modernization for unported screens | future |
| 7 | Optional: localization, multi-user, server DB | future |

---

## 1. Goals & Non-Goals

### 1.1 Goals (this spec)

1. Produce a Visual Studio (MSVC) solution that compiles to a **64-bit Unicode** `gl.exe`.
2. The exe launches into an MDI frame with menu, top toolbar, vertical toolbar, and status bar — visually equivalent to the OWL original's outer chrome (company name + period in the status bar).
3. The login flow (`gl/gl_acces.cpp`) and company-profile flow (`common/gl_prof.cpp` + `common/gl_pds.cpp`) work end-to-end against a SQLite-backed company file.
4. The **Chart of Accounts** screen (`gl/gl_accts.cpp` + `gl/gl_acctl.cpp`) is fully ported: open, browse, add, edit, delete, persist — round-tripping through the SQLite shim.
5. Five custom controls used by the chart-of-accounts dialogs (`tbPic`, `tbDate`, `tbFloat`, `tbInteger`, `tbSuperBtn`) are implemented as registered Win32 window classes in the shared framework.
6. A `dbf2sqlite` migration tool exists that converts a directory of `.dbf` files (with `.ndx`/`.cdx` indexes) to a single SQLite `.db` file. Required tables for #1: `accessm`, `access.psw`, the company-profile table, and `glaccts`. Other tables may be migrated as empty schemas.
7. The mini-framework (`win32fx`) and shim (`cb4sqlite`) are reusable as static libraries by the future module specs (#3-#5).

### 1.2 Non-goals

- AP and AR modules. Their `.LZH` source archives stay archived.
- Reporting (`*_rept.cpp`, Crystal Reports calls, `gl_xls.cpp` Excel export) — handlers exist but show "not yet ported" message boxes; no engine.
- Help system (`gl.hbk`, `WinExec("winhelp...")`) — replaced with a no-op stub.
- Backup / restore (`savegl.pif`, `restore.pif`) — stubbed.
- BWCC, CTL3D — deleted entirely.
- The original 16-bit `gl.exe` and the `.ide` (Borland IDE project) files — kept on disk for reference; not used by build.
- Full migration of every `.dbf` table — only those Spec #1 needs (above) round-trip; others may be created with schema only.
- Performance optimization, multi-threading.
- Localization beyond English resources.

### 1.3 Accepted risks

- Most of `gl/gl.rc` (~190 KB) and `gl/gl_new.rc` (~233 KB) is excluded from the build until later specs. The pruned `gl.rc` is the new build input; the original lives in git history.
- Toolbox provides more than five custom controls (e.g., grid controls used by AP/AR batch-entry screens). Those are deferred until AP/AR work demands them.
- The Crystal Reports stub means menu items in the reporting menus all show the same "not yet ported" dialog — the menu structure stays, the engine doesn't.

---

## 2. Architecture & Project Layout

A three-project Visual Studio solution `gl.sln` lives at the **repo root** (`/home/mst/Projects/GLSRC/`), alongside the existing module directories:

```
GLSRC/
├── gl.sln
├── win32fx/              (static lib — reusable Win32 mini-framework)
│   ├── win32fx.vcxproj
│   ├── Window.{h,cpp}              Window base class + WndProc dispatcher
│   ├── Dialog.{h,cpp}               Dialog base class
│   ├── MsgMap.h                    BEGIN_MSG_MAP / ON_MSG / END_MSG_MAP
│   ├── MdiFrame.{h,cpp}            MDI frame helper
│   ├── Toolbar.{h,cpp}             Top + vertical toolbars
│   ├── StatusBar.{h,cpp}           Status bar wrapper
│   ├── HandleGuard.h               RAII for HMENU/HFONT/HBITMAP/...
│   ├── StringUtil.{h,cpp}          LoadResString/Format/Utf*/Trim
│   └── controls/
│       ├── PicEdit.cpp             tbPic     (input-mask edit)
│       ├── DateEdit.cpp            tbDate    (wraps SysDateTimePick32)
│       ├── FloatEdit.cpp           tbFloat   (validated decimal)
│       ├── IntEdit.cpp             tbInteger (validated integer)
│       └── SuperBtn.cpp            tbSuperBtn (themed/owner-draw button)
│
├── cb4sqlite/            (static lib — CodeBase → SQLite shim)
│   ├── cb4sqlite.vcxproj
│   ├── d4all.hpp                   Drop-in umbrella header (matches original name)
│   ├── Code4.{h,cpp}               Database connection (replaces global cb)
│   ├── Data4.{h,cpp}               Table/cursor (replaces Data4)
│   ├── Field4.{h,cpp}              Field accessor
│   ├── Date4.{h,cpp}               Date helper
│   ├── Relate4set.{h,cpp}          Minimal relational query
│   ├── Schema.{h,cpp}              Maps .dbf field layout → SQLite columns
│   ├── sqlite/
│   │   ├── sqlite3.c               Vendored amalgamation
│   │   └── sqlite3.h
│   └── tools/
│       └── dbf2sqlite/             Console exe (separate vcxproj)
│           ├── dbf2sqlite.vcxproj
│           ├── main.cpp
│           ├── DbfReader.{h,cpp}
│           └── IndexReader.{h,cpp}
│
├── cb4sqlite_tests/      (test exe)
│   └── cb4sqlite_tests.vcxproj
│
└── gl/                   (the EXE — incrementally replaces files in the original gl/)
    ├── gl.vcxproj
    ├── App.cpp                     WinMain, hInstance globals, startup, RegisterClasses
    ├── App.h                       extern globals (hInst, HGLMenu, cb, szApp)
    ├── ManilaGL.{cpp,h}            Top-level frame (replaces TManilaGL/TGlWindow)
    ├── Login.{cpp,h}               Ported gl_acces.cpp
    ├── CompanyProfile.{cpp,h}      Ported common/gl_prof.cpp + gl_pds.cpp
    ├── ChartOfAccounts.{cpp,h}     Ported gl_accts.cpp
    ├── AccountList.{cpp,h}         Ported gl_acctl.cpp
    ├── stubs/
    │   ├── ReportingStubs.cpp      *_rept.cpp menu handlers → "not yet ported"
    │   ├── ApStubs.cpp             AP menu items → "not yet ported"
    │   └── ArStubs.cpp             AR menu items → "not yet ported"
    └── res/
        ├── gl.rc                   Pruned to needed resources
        ├── utildlg.rc              Included from common/utildlg.rc (pruned)
        ├── resource.h              Centralized IDs
        ├── manifest.xml            v6 common controls + per-monitor DPI v2
        └── bitmaps/                Extracted from original hex-embedded bitmaps
```

Notes:
- We **add files** to the existing `gl/` directory rather than create a parallel `gl-new/`. Each ported source replaces (in build, not by deleting) its OWL counterpart. The OWL files stay on disk for reference; only files listed in `gl.vcxproj` are compiled.
- `common/` is consumed for Spec #1 too: `gl_prof.cpp` (company profile), `gl_pds.cpp` (period setup), `utildlg.rc` (utility dialogs).
- `include/` headers (CodeBase / Toolbox / Crystal) are **not on the new include path** — the modern code does not include them. They stay on disk for reference.
- `owl/*.lib` are 16-bit Borland libs — incompatible with MSVC x64 and unused.

### 2.1 Dependencies

| Project | Links | External |
|---|---|---|
| `win32fx` | — | kernel32, user32, gdi32, comctl32 |
| `cb4sqlite` | — | (none — SQLite vendored) |
| `cb4sqlite_tests` | cb4sqlite | (test framework, see §7.3) |
| `dbf2sqlite` | cb4sqlite | (DBF reader vendored) |
| `gl.exe` | win32fx, cb4sqlite | comctl32, comdlg32 |

All static linking. No DLLs. No NuGet, no submodules, no system-installed libraries beyond the Win32 SDK and the MSVC redistributable (which static CRT eliminates).

### 2.2 Manifest

`manifest.xml` declares:
- v6 common controls (themed widgets — replaces what CTL3D provided).
- `perMonitorV2` DPI awareness from day one.
- Long-path awareness (`longPathAware = true`).
- Compatibility GUIDs for Windows 10 / 11.

---

## 3. The Mini-Framework (`win32fx`)

Approach B from brainstorming: smallest in-house Win32 framework that lets ported screens read like OWL originals without dragging in MFC/WTL/Qt.

### 3.1 `Window` base class

```cpp
class Window {
public:
    virtual ~Window();

    HWND  hwnd() const { return hwnd_; }
    bool  Create(HWND parent, LPCWSTR title, DWORD style, DWORD exStyle,
                 int x, int y, int w, int h, HMENU menu = nullptr);

    virtual bool OnCreate(CREATESTRUCT*)                         { return false; }
    virtual bool OnCommand(int id, int notify, HWND ctl)         { return false; }
    virtual bool OnSize(int cx, int cy, UINT type)               { return false; }
    virtual bool OnDestroy()                                     { return false; }
    virtual bool OnMessage(UINT msg, WPARAM, LPARAM, LRESULT&)   { return false; }

protected:
    HWND hwnd_ = nullptr;

private:
    static LRESULT CALLBACK Thunk(HWND, UINT, WPARAM, LPARAM);
    LRESULT Dispatch(UINT msg, WPARAM, LPARAM);
};
```

Standard SDK idiom: on `WM_NCCREATE`, `lpCreateParams` carries the `Window*`; `SetWindowLongPtrW(GWLP_USERDATA)` stashes it; subsequent messages are forwarded to the instance's `Dispatch`. ~30 lines.

### 3.2 `Dialog` base class

Parallel pattern for modal/modeless dialogs. `DoModal` calls `DialogBoxParamW`; the thunk pulls `this` out of `DWLP_USER` after `WM_INITDIALOG`.

```cpp
class Dialog {
public:
    virtual ~Dialog();

    INT_PTR DoModal(HINSTANCE, LPCWSTR templateName, HWND parent);
    bool    CreateModeless(HINSTANCE, LPCWSTR templateName, HWND parent);

    virtual bool OnInitDialog()                                  { return true; }
    virtual bool OnCommand(int id, int notify, HWND ctl)         { return false; }
    virtual bool OnMessage(UINT, WPARAM, LPARAM, INT_PTR&)       { return false; }

protected:
    HWND hwnd_ = nullptr;
    HWND Item(int id) const { return GetDlgItem(hwnd_, id); }

private:
    static INT_PTR CALLBACK Thunk(HWND, UINT, WPARAM, LPARAM);
};
```

### 3.3 Message map macro

Virtuals cover ~95% of messages; for the rest:

```cpp
#define BEGIN_MSG_MAP(cls)                                            \
    bool cls::OnMessage(UINT m, WPARAM wp, LPARAM lp, LRESULT& out) {

#define ON_MSG(msg, handler)                                          \
    if (m == msg) { out = handler(wp, lp); return true; }

#define END_MSG_MAP() return false; }
```

Typical use:
```cpp
BEGIN_MSG_MAP(ChartOfAccountsDlg)
    ON_MSG(WM_NOTIFY,         OnNotify)
    ON_MSG(WM_CTLCOLORSTATIC, OnCtlColor)
END_MSG_MAP()
```

No template tricks; debuggable; eight lines of macro total.

### 3.4 RAII handle wrappers

```cpp
template <typename H, BOOL (WINAPI *Deleter)(H)>
class HandleGuard {
    H h_ = nullptr;
public:
    HandleGuard() = default;
    explicit HandleGuard(H h) : h_(h) {}
    ~HandleGuard() { if (h_) Deleter(h_); }
    HandleGuard(HandleGuard&&) noexcept;
    HandleGuard& operator=(HandleGuard&&) noexcept;
    HandleGuard(const HandleGuard&) = delete;
    H get() const { return h_; }
    H release() { H t = h_; h_ = nullptr; return t; }
    operator H() const { return h_; }
};

using MenuGuard = HandleGuard<HMENU, DestroyMenu>;
using GdiFont   = HandleGuard<HFONT, (BOOL(WINAPI*)(HFONT))DeleteObject>;
// ... HBITMAP, HBRUSH, HPEN, HRGN, HICON
```

### 3.5 `MdiFrame`, `Toolbar`, `VerticalToolbar`, `StatusBar`

`MdiFrame : public Window` creates the frame plus a `MDICLIENT` child via `CreateWindowExW(0, L"MDICLIENT", ..., (LPVOID)&clientCreateStruct)`. Implements `WM_SIZE` to lay out toolbar / MDI client / status bar (porting the geometry from the original `TGlWindow::WMSize` at `gl/gl.cpp:263`). Provides `CreateMdiChild(...)` helper.

`Toolbar`, `VerticalToolbar`, `StatusBar` are separate `Window` subclasses created from resources (or programmatically). They own bitmaps via `GdiBitmap`.

### 3.6 Custom controls

Each control registers its window class with the **same name as the original Toolbox class**, so `.rc` `CONTROL` lines work unchanged:

| Original | Replacement | Strategy |
|---|---|---|
| `tbPic` | `PicEdit` | Subclassed `EDIT` + `WM_CHAR` filter using a mask string ("XXXXX", "(999) 999 - 9999"). Mask parser ~100 lines. |
| `tbDate` | `DateEdit` | Wraps `SysDateTimePick32`; translates between original field text and `SYSTEMTIME`. ~80 lines. |
| `tbFloat` | `FloatEdit` | Subclassed `EDIT` + `WM_CHAR` filter (digits, single decimal, optional sign). Stores precision/scale from mask ("9999999999999.99"). ~120 lines. |
| `tbInteger` | `IntEdit` | Subclassed `EDIT` + `WM_CHAR` filter (digits + sign). ~60 lines. |
| `tbSuperBtn` | `SuperBtn` | First pass: themed `BUTTON` (`BS_PUSHBUTTON`) registered as `tbSuperBtn`. If the visual gap is unacceptable, escalate to owner-draw with 3D border + bitmap + mnemonic. ~50–150 lines. |

All five classes register at startup from `App.cpp` via `Register*Class(hInst)` calls before any dialog is loaded.

This is the single biggest architectural lever in the entire port: keeping the class names lets us avoid rewriting ~190 KB of dialog templates.

### 3.7 Globals

The original uses globals heavily (`hInst`, `HGLMenu`, `cb`, `szApp`, etc.). We preserve that pragma — wrapping in singletons is not worth the abstraction cost for a port. They live in `App.cpp` as definitions, with `extern` declarations in `App.h`.

### 3.8 Estimated framework size

~1200 lines hand-written, no STL beyond `std::wstring`/`std::vector`/`std::optional`. Compiles in seconds.

---

## 4. `cb4sqlite` (CodeBase → SQLite Shim)

### 4.1 Design principle

Mirror the CodeBase 4 API surface only as much as ported `.cpp` files demand. Spec #1 implements only what Chart of Accounts + login + profile use. Each unimplemented method throws `cb4sqlite::Error("not implemented in spec #1: <name>")` — no silent corruption; clear failure on first touch.

### 4.2 API surface (Spec #1)

`cb4sqlite/d4all.hpp` is a drop-in umbrella header so existing `#include "d4all.hpp"` lines in ported files continue to work.

```cpp
namespace cb {

class Error : public std::runtime_error { using std::runtime_error::runtime_error; };

class Code4 {
public:
    int exclusive    = 0;
    int readOnly     = 0;
    int readLock     = 0;
    int lockAttempts = -1;

    bool open(const std::filesystem::path& dbFile);
    void closeAll();
    void initUndo();          // no-op (kept for API parity)
    sqlite3* handle() { return db_; }   // shim-internal

private:
    sqlite3* db_ = nullptr;
    std::vector<class Data4*> openTables_;
};

class Data4 {
public:
    Data4(Code4& cb, const wchar_t* tableName);
    Data4(Code4& cb, const char*   tableName);   // ANSI overload — less call-site churn
    ~Data4();

    int  top();
    int  bottom();
    int  skip(int n);
    int  seek(const wchar_t* key);
    int  seek(double key);
    int  append();
    int  flush();
    int  recCount();
    int  recNo();
    bool eof() const;
    int  selectTag(const wchar_t* tagName);
    void close();

    sqlite3_stmt* row() { return cursor_; }       // shim-internal

private:
    Code4& cb_;
    std::wstring                  table_;
    std::vector<class ColumnMeta> cols_;
    sqlite3_stmt*                 cursor_       = nullptr;
    int                           currentRowid_ = 0;
};

class Field4 {
public:
    Field4() = default;                                // permits init() pattern
    Field4(Data4& d, int fieldNum1Based);              // CodeBase fields are 1-based
    Field4(Data4& d, const wchar_t* name);

    void init(Data4& d, int fieldNum1Based);

    std::wstring str() const;                          // padded as dBase did
    double       num() const;
    int          assign(const wchar_t* value);
    int          assign(double value);
    bool         isValid() const;

private:
    Data4* data_   = nullptr;
    int    colIdx_ = -1;
    bool   valid_  = false;
};

class Date4 {
public:
    void today();
    std::wstring str() const;                          // CodeBase format: "YYYYMMDD"
private:
    int y_ = 0, m_ = 0, d_ = 0;
};

class Relate4set {
public:
    explicit Relate4set(Data4* primary);
    int top();
    int next();
    int eof();
    // additional surface added only as ported screens demand
};

} // namespace cb
```

Out of scope for #1: `Tag4`, `Index4`, full `Relate4set` join semantics, memo fields, BLOB I/O, locking primitives, transactions visible at the API level. Each becomes a `throw Error("...not implemented in spec #1...")` method.

### 4.3 Storage model

**One SQLite file per company.** The path comes from `GLDR.INI` exactly as the original derives the company directory — but it points at a `.db` file instead of a directory of `.dbf` files. (E.g., `[GL_DIR] DIR=C:\GL\COMPANY1\company.db`.)

**Schema** mirrors the .dbf field layouts directly. Each `.dbf` becomes a SQLite table with:
- `_rowid INTEGER PRIMARY KEY` — replaces the implicit dBase record number used by `recNo()`.
- `_deleted INTEGER NOT NULL DEFAULT 0` — soft-delete flag; navigation skips rows with `_deleted = 1`. Mirrors dBase semantics where deleted rows still exist until `pack`.
- One column per dBase field, using the original field name. Type mapping:

| dBase type | SQLite type | Notes |
|---|---|---|
| Character (n) | `TEXT` | Re-padded to width n on read; trimmed/padded on write. |
| Numeric (n.d), d > 0 | `REAL` | |
| Numeric (n.0) | `INTEGER` | |
| Date | `TEXT` | Stored `YYYYMMDD` to match `Date4::str()`. |
| Logical | `INTEGER` | 0 / 1. |
| Memo | `TEXT` | Deferred for #1 (throws on access). |

**Indexes:** for every `.ndx` / `.cdx` index in the migration source, `CREATE INDEX <table>_<tag> ON <table>(<col>)`. A metadata table holds the tag-to-column mapping so `Data4::selectTag()` can look up the right `ORDER BY` clause:

```sql
CREATE TABLE _cb_tags (
    table_name  TEXT NOT NULL,
    tag_name    TEXT NOT NULL,
    order_by    TEXT NOT NULL,         -- e.g. "ACCT_NO ASC"
    PRIMARY KEY (table_name, tag_name)
);
```

**Padding:** dBase character fields are fixed-width and space-padded. The original code does string compares assuming the padding. `Field4::str()` re-pads to the column width on read so existing comparison logic in ported code works without edits. `Field4::assign(const wchar_t*)` trims and re-pads on write.

### 4.4 Cursor model

Each `Data4` wraps one `sqlite3_stmt*` representing the current cursor.
- `top()` re-prepares with `ORDER BY <active tag's order_by>` and steps once.
- `bottom()` re-prepares with reversed order.
- `skip(n)` steps forward n times (`n` negative steps backward — implemented by re-preparing with reversed order from current position; less efficient but rare).
- `seek(key)` re-prepares with `WHERE <tag column> >= ?` and binds the key.
- `eof()` reflects the stmt being past the last row.

This is fundamentally different from the .dbf model (random-access via index file pointers), but the **observable behavior** through the `Data4`/`Field4` API is identical for the call sites we need.

Edge case: the original code occasionally does a `top()` on one table inside a loop over another (nested cursors). Each `Data4` has its own `sqlite3_stmt*`, so nested cursors work. If a `Field4` reference is held across a `top()` reset of its parent `Data4`, it sees new data — same semantics as the original.

### 4.5 Migration tool: `dbf2sqlite`

Console exe under `cb4sqlite/tools/dbf2sqlite/`.
- **Inputs:** path to a directory of `.dbf` (and optional `.ndx`/`.cdx`) files.
- **Output:** a SQLite `.db` with corresponding tables and indexes.

Implementation:
- DBF parser ~200 lines: read the .dbf header (32-byte main header + 32-byte field descriptors), iterate records, write one INSERT per row inside a single transaction. We vendor a BSD-licensed DBF reader rather than write from scratch.
- `.ndx` / `.cdx` index files: parse only the header to find the indexed expression's column name (full B-tree parse not needed). Issue `CREATE INDEX <table>_<tag> ON <table>(<col>)` and a `_cb_tags` row.
- `include/db_*.h` headers contain pre-existing field-layout knowledge (e.g., `db_accts.h` describes the chart-of-accounts table). The migration tool can optionally consume these as a verification source: parse them and confirm the live `.dbf` matches the documented schema. Mismatches log a warning.

For Spec #1, `dbf2sqlite` must handle at minimum: `accessm`, `access.psw`, the company-profile table (likely `gl_prof.dbf` or similar — confirm during implementation by reading `db.def` in `gl/`), and `glaccts`. Other tables may be migrated as schema-only (table created from .dbf header, zero rows imported) so future ports have a place to read from.

### 4.6 Out of scope for #1

- Locking primitives.
- Transactions visible at the API level (internal migration uses transactions, but no `Code4::beginTrans`).
- Memo fields.
- Custom dBase expression evaluator (chart-of-accounts uses simple field-name indexes, no expressions; the few we hit in #1 we handle ad hoc).
- Write-back to `.dbf` — once migrated, we stay on SQLite.

---

## 5. Unicode & String Strategy

### 5.1 Three-tier rule

**Tier 1 — Win32 boundary:** explicit `W` APIs and `LPCWSTR`. **No `TCHAR`, no `_T()`, no `LPTSTR`.** We're committed to Unicode permanently; the macros add no value. String literals at the boundary are `L"..."`. Calls become `MessageBoxW`, `LoadStringW`, `CreateWindowExW`, `GetPrivateProfileStringW`, etc.

**Tier 2 — Internal:** `std::wstring` replaces the custom `string` class from `cstring.h`. The `string* s = new string(...) / s->c_str() / delete s` pattern visible in `gl/gl.cpp` disappears (RAII).

```cpp
// Before (gl/gl.cpp:170):
string* s = new string(szApp);
MessageBox(HWindow, GetString(...), s->c_str(), MB_OK);
delete s;

// After:
std::wstring s = szApp;
MessageBoxW(hwnd_, fx::LoadResString(IDS_X).c_str(), s.c_str(), MB_OK);
```

**Tier 3 — Fixed buffers:** where the original has `char szApp[20]`, port to `WCHAR szApp[20]`. Don't gratuitously heap-allocate. `MAX_PATH + 1` for path buffers; `\\?\` long-path prefix only on actual overflow.

### 5.2 Helpers (`win32fx/StringUtil.h`)

```cpp
namespace fx {
    std::wstring LoadResString(UINT id);                       // wraps LoadStringW
    std::wstring Format(LPCWSTR fmt, ...);                     // wraps _vsnwprintf_s
    std::wstring Utf8ToWide(std::string_view utf8);            // for SQLite text I/O
    std::string  WideToUtf8(std::wstring_view wide);
    std::wstring Trim(std::wstring_view s);                    // replaces StrTrim
}
```

The original `GetString(hInstance, ID)` (`gl/gl.cpp:112`) — which `GlobalAlloc`s an 85-byte buffer, calls `LoadString`, leaks the global handle on the way out — is replaced everywhere by `fx::LoadResString(ID)`.

`StrTrim` (used at `gl/gl.cpp:237`) becomes `fx::Trim`.

### 5.3 SQLite I/O

SQLite's native text type is UTF-8. The shim does the conversion at its boundary:
- `Field4::str()` reads `sqlite3_column_text` (UTF-8) → `Utf8ToWide` → returns `std::wstring`.
- `Field4::assign(const wchar_t*)` → `WideToUtf8` → `sqlite3_bind_text` with `SQLITE_TRANSIENT`.
- Table and column names are ASCII (matching dBase 8.3 conventions); SQL statements bind via `sqlite3_prepare_v2`'s narrow API. Only field values cross the UTF-8/UTF-16 boundary.

### 5.4 Resources

`gl.rc` (and any included `.rc` like `utildlg.rc`) becomes UTF-16 LE with BOM (modern `rc.exe` default). `STRINGTABLE` entries — used heavily for menu help text — load via `fx::LoadResString`.

Verbatim English strings; no localization framework, no `.po`, no `.resx`. Resource-based strings are already the foundation if localization becomes a future goal.

### 5.5 INI files

The original reads `GL.INI` and `GLDR.INI` via `GetPrivateProfileString` / `GetPrivateProfileInt`. We keep this pattern for #1 — they remain in the Win32 SDK as `GetPrivateProfileStringW` / `GetPrivateProfileIntW`. Microsoft deprecates them in favor of registry/structured config, but they work, they're zero-dependency, and the existing INI files keep working as-is. Migration to JSON or registry is a future cleanup, not a Spec #1 concern.

### 5.6 Things explicitly not changing

- Numeric formatting in business logic (currency, percentages) — keep whatever the original uses (likely `sprintf` of `Field4::num()` results). We're not introducing `std::format` mid-port.
- Locale awareness — original is implicitly en-US. SQLite default `BINARY` collation matches the original's `strcmp` semantics. We don't introduce locale-aware comparisons.

### 5.7 Conversion churn estimate

Per ported `.cpp`, roughly:

| Pattern | Frequency | Translation |
|---|---|---|
| `LPSTR` / `char *` parameter | every function | `LPCWSTR` (in) or `LPWSTR` (out) |
| `"..."` literal | many | `L"..."` |
| `MessageBox(...)` etc. | dozens | `*W` variants |
| `string` (custom) | dozens | `std::wstring` |
| `char szBuf[N]` | common | `WCHAR szBuf[N]` |
| `strcpy`/`strcmp`/`strlen` | common | `wcscpy_s`/`wcscmp`/`wcslen` |
| `_chdir`, `getcwd` | a few | `SetCurrentDirectoryW`, `GetCurrentDirectoryW` |

Mechanical, compiler-error-driven, file-at-a-time work.

---

## 6. Resource Strategy & The Chart of Accounts Target

### 6.1 .rc modernization in place

Most of `gl/gl.rc` ports as-is. Borland Resource Workshop output uses a few non-portable constructs that we scrub globally:

| Original | Action |
|---|---|
| `LOADONCALL MOVEABLE DISCARDABLE` on every resource | delete (16-bit segment-loader hints) |
| Embedded BITMAP hex blobs (`'42 4D ...'`) | extract → external `.bmp` files; replace with `BITMAP "bitmaps/<id>.bmp"` |
| Magic style numbers (`40962`, `32784`, `9216`, `29696`) on tb* controls | leave as-is — our reimplemented controls interpret them |
| `DIALOG` headers with implicit features | upgrade to `DIALOGEX` only when needed |

What does **not** change:
- Dialog coordinates (dialog units auto-scale via the DPI manifest).
- Control IDs.
- Menu structure (`MENU` blocks).
- String tables.

What gets pruned for Spec #1:
- Dialogs not used by login / company profile / chart-of-accounts (~95% of `gl.rc`).
- All of `gl_new.rc` (defer to a later spec).

Kept dialogs (initial list — confirm and refine during implementation):
- 1008 — About box (per `gl/gl.cpp:195`)
- 1032 — main window top toolbar template (per `gl/gl.cpp:223` `new TToolBar(this, 1032)` — global commands, used across all MDI screens)
- 2057 — main window vertical toolbar template (per `gl/gl.cpp:226` `new TVToolBar(this, 2057)`)
- 1033 — General Ledger Account Setup
- 1034 — General Ledger Cash Accounts (referenced by chart-of-accounts)
- Login + company profile dialogs (IDs confirmed by reading `gl/gl_acces.cpp` and `common/gl_prof.cpp` during implementation)
- Menus: `manila`, `gle`, `GLSUB`
- STRINGTABLE entries referenced by the above

### 6.2 Why we can keep `tbPic` / `tbDate` / `tbFloat` / `tbInteger` / `tbSuperBtn` class names

Win32's dialog manager creates controls by passing the class name string from the `CONTROL` line to `CreateWindowExW`. As long as a window class with that exact name has been registered before the dialog is created, the dialog manager doesn't care who registered it.

`win32fx/controls/PicEdit.cpp`:
```cpp
void RegisterPicEditClass(HINSTANCE hInst) {
    WNDCLASSEXW wc{ sizeof(wc) };
    wc.lpszClassName = L"tbPic";        // exact original Toolbox name
    wc.lpfnWndProc   = PicEditProc;
    wc.cbWndExtra    = sizeof(PicEdit*);
    wc.hCursor       = LoadCursorW(nullptr, IDC_IBEAM);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    RegisterClassExW(&wc);
}
```

Called once from `App.cpp` startup. All existing `CONTROL "...", id, "tbPic", ...` lines work unchanged.

### 6.3 Bitmap modernization

The original `.rc` embeds dozens of bitmaps as hex literals (toolbar icons, mostly 11×11 or 16×16, 4-bit). Plan:
1. Throwaway extraction script: find every `<id> BITMAP ... { '...' }` block, decode hex into `gl/res/bitmaps/<id>.bmp`.
2. Rewrite `.rc` references as `<id> BITMAP "bitmaps/<id>.bmp"`.
3. Bitmaps render but look dated. Acceptable for #1; PNG / image-list upgrade is a future spec.

### 6.4 Chart of Accounts: source files & ports

| Original | Target | Notes |
|---|---|---|
| `gl/gl_accts.cpp` (554 lines) | `gl/ChartOfAccounts.cpp` | `TGLAcctsAdd`, `TGLAcctsDelete` → `ChartOfAccountsDlg`, `ChartOfAccountsDeleteDlg`. Dialog 1033 (modal — "General Ledger Account Setup"). Navigation buttons (Top/Bottom/Next/Prev) are part of dialog 1033's own controls. |
| `gl/gl_accts.h` | `gl/ChartOfAccounts.h` | Class declarations. |
| `gl/gl_acctl.cpp` (488 lines) | `gl/AccountList.cpp` | `TAccountList` → `AccountListDlg`. Owner-draw listbox, navigation hotkeys. Dialog 1034. |
| `gl/gl_acctl.h` | `gl/AccountList.h` | |
| Headers: `gl_acct.h`, `gl_main.h`, `glclass.h` | merged into `gl/App.h` and per-screen headers | |

Database touched: `glaccts` table + indexes (likely `ACCT_NO`, `DESCRIP` — confirmed during implementation by reading `gl/db.def` in the source tree).

### 6.5 Translation pattern (concrete example)

OWL (`gl/gl_accts.h`):
```cpp
class TGLAcctsAdd : public TMainDialog {
    BOOL bAcctList;
    PTglaccts glaccts;
    virtual void SetupWindow();
    virtual void Search   (RTMessage Msg) = [ID_FIRST + ID_GLINQ];
    virtual void Ok       (RTMessage Msg) = [ID_FIRST + IDOK];
    virtual void Delete   (RTMessage Msg) = [ID_FIRST + ID_DELETE];
};
```

Win32 SDK (`gl/ChartOfAccounts.h` + `.cpp`):
```cpp
class ChartOfAccountsDlg : public fx::Dialog {
    bool        bAcctList_ = false;
    Glaccts*    glaccts_   = nullptr;        // wraps cb::Data4

    bool OnInitDialog() override;
    bool OnCommand(int id, int notify, HWND ctl) override;

private:
    void OnSearch();
    void OnOk();
    void OnDelete();
};

bool ChartOfAccountsDlg::OnCommand(int id, int notify, HWND) {
    switch (id) {
        case ID_GLINQ:  OnSearch(); return true;
        case IDOK:      OnOk();     return true;
        case IDCANCEL:  EndDialog(hwnd_, IDCANCEL); return true;
        case ID_DELETE: OnDelete(); return true;
        // ...
    }
    return false;
}
```

The OWL `[ID_FIRST + X]` annotations become `case X:` lines. Method bodies port near-verbatim with the §4 (DB) and §5 (Unicode) transforms.

### 6.6 Acceptance criteria (concrete manual checklist)

These are the eight things a human runs through to declare Spec #1 done:

1. Launch `gl.exe` → MDI frame appears with menu, top toolbar, vertical toolbar, status bar showing company name + period (loaded from SQLite-backed company profile).
2. From menu: GL → Chart of Accounts opens dialog 1033, with `tbPic` fields populated by stepping through `glaccts` records in tag order. Top / Bottom / Next / Previous navigation works (mechanism — dialog buttons, main toolbar commands, or menu hotkeys — preserved as in original; confirmed during implementation).
3. **Add:** enter account number + description + type, click Add/Mod → row appears in `glaccts` SQLite table (verify with `sqlite3 company.db "select * from glaccts"`).
4. **Modify:** navigate to existing account, edit fields, Add/Mod → SQLite row updated.
5. **Delete:** navigate, click Delete → `_deleted` flag set; subsequent navigation skips the deleted record.
6. **Persistence:** close dialog and reopen — changes survive.
7. **Account list dialog (1034):** opens, lists accounts, selecting one populates the parent dialog.
8. **tb\* validation works:** `tbInteger` rejects letters, `tbFloat` allows one decimal, `tbPic` enforces the mask (e.g., 3-character department code).

If all eight pass, Spec #1 is done.

---

## 7. Build, Errors, Tests, Risks

### 7.1 Build system

- Hand-written `.vcxproj` files committed to git (not CMake — Windows-only, MSVC-only target).
- Configurations: `Debug|x64`, `Release|x64`. No 32-bit, no ARM64 in #1.
- Shared `.props` file with compiler settings:
  - `/std:c++20`
  - `/W4 /WX` (treat warnings as errors)
  - `/permissive-`
  - `UNICODE`, `_UNICODE` defined
  - `WIN32_LEAN_AND_MEAN`, `NOMINMAX`
  - `/sdl`, `/guard:cf` on Release
  - Static CRT (`/MT`, `/MTd`) — single-exe distribution
- Linker:
  - Subsystem `WINDOWS` for `gl.exe`, `CONSOLE` for `dbf2sqlite`
  - `/DYNAMICBASE`, `/NXCOMPAT`
- SQLite vendored as the [amalgamation](https://www.sqlite.org/amalgamation.html), compiled with:
  - `SQLITE_THREADSAFE=0` (single-threaded)
  - `SQLITE_OMIT_LOAD_EXTENSION=1`
  - `SQLITE_DEFAULT_FOREIGN_KEYS=1`
- Output:
  ```
  build/x64/Debug/gl.exe
  build/x64/Debug/dbf2sqlite.exe
  build/x64/Debug/cb4sqlite_tests.exe
  build/x64/Release/...
  ```

### 7.2 Error handling

The original mixes three idioms (return codes, `MessageBox` + `exit(1)`, silent fall-through). We translate idiom-for-idiom — no unification. That's a future refactor.

- **Win32 API failures:** `MessageBoxW` with `FormatMessageW` text, return failure to caller. `Window::Dispatch` and `Dialog::Thunk` wrap their handler invocations in `try { ... } catch (...) { LogAndShow(); ... }` so a stray exception in a handler can't unwind through `user32`.
- **Shim errors:** `cb::Error` (derives `std::runtime_error`) carrying `sqlite3_errmsg`. Ported screens catch at handler boundaries and show a `MessageBoxW`. Internal shim code uses `CHECK_SQLITE(rc)` to keep call sites tidy.
- **Programmer errors:** `assert()` in debug; `FX_VERIFY(cond, msg)` in release for "should never happen but if it does, log + bail with MessageBox" cases.

We do **not** introduce `std::expected`, `Result<T>`, or per-call try/catch. We do **not** add error handling for paths the original didn't.

### 7.3 Testing

Three layers:

**Layer 1 — `cb4sqlite` unit tests** (`cb4sqlite_tests.exe`). Required for #1. Catch2 single-header is the simplest fit; if we want zero external deps, a hand-rolled `TEST(name) { ... }` macro is ~50 lines. Coverage:
- Open / close.
- Append a row, read it back, round-trip TEXT / REAL / INTEGER / DATE.
- Tag selection orders correctly.
- `seek()`: hit, miss, partial match.
- `top()` / `bottom()` / `skip(n)` / `skip(-n)` / `eof()`.
- Soft-delete: delete, navigate skips, `recCount()` excludes deleted.
- Padding: assigning a 5-char string to a 10-char field reads back as 10 chars space-padded.

**Layer 2 — `dbf2sqlite` integration test.** Required for #1. Vendor a small fixture (5-row `.dbf` matching the chart-of-accounts schema). Migration runs, `cb4sqlite` reads back, asserts known field values.

**Layer 3 — manual walkthrough.** Required for #1. The eight criteria from §6.6. Documented and signed off by the implementer.

What we **don't** test in #1:
- `win32fx` itself (correctness validated by the chart-of-accounts dialog working end-to-end). Adding unit tests for `Window`/`Dialog` requires a fake message pump and is more code than the framework being tested.
- UI automation (UIA, AutoIt, WinAppDriver) — heavyweight, defer to AP/AR work.

### 7.4 Risks & mitigations

| # | Risk | Likelihood | Mitigation |
|---|---|---|---|
| 1 | tb* control reimplementation has subtle differences (e.g., `tbFloat` cursor behavior, `tbPic` mask edge cases) that break ported screens non-obviously | High | Implement minimum behavior; test against chart-of-accounts manually; fix discovered gaps. Don't replicate every original behavior preemptively. |
| 2 | Magic style number bits (`40962`, `9216`, `29696`) on tb* controls encode behavior we haven't decoded | Medium | Treat unknown bits as no-ops first; decode only what the screen demonstrably needs. Document discovered bits in `controls/styles.md`. |
| 3 | `Relate4set` semantics in `gl_acctl.cpp` are richer than expected | Medium | If complex relational queries appear, escalate per-site: implement either as nested `Data4` cursors (equivalent semantics) or a real SQL JOIN. |
| 4 | `.cdx` index expressions ("UPPER(NAME)+ACCT") need expression evaluation | Medium | Support only what chart-of-accounts uses (uppercase, concatenation). Defer general expression evaluator. |
| 5 | DPI scaling: original's pixel-based `MoveWindow` calls misalign on hi-DPI | High on hi-DPI hardware | Per-monitor v2 manifest from day one (dialog units auto-scale). Hand-coded pixel math in `WMSize` ports gets `MulDiv(value, dpi, 96)`. |
| 6 | Static CRT + SQLite amalgamation produces ~1.5–2 MB exe vs original ~600 KB | Low (cosmetic) | Accept. Static linking is worth more than file size in 2026. |
| 7 | Borland RC dialect has constructs `rc.exe` rejects we haven't seen yet | Medium | First step of resource port: feed pruned `gl.rc` to `rc.exe`, fix errors. Allocate a half-day buffer. |
| 8 | `tbSuperBtn` owner-draw painting non-trivial to match | Low–Medium | First pass: themed `BUTTON` registered as `tbSuperBtn`. Escalate to owner-draw only if visual gap is unacceptable. |
| 9 | Hidden global state in OWL not yet surveyed (e.g., `extern PTWindowsObject GetObjectPtr` at `gl/gl.cpp:60`) | Medium | Surface via grep as files are ported; each becomes either deleted (OWL machinery) or replaced. |
| 10 | `.dbf` field naming or indexing differs between the documented `db_*.h` schemas and the live data | Low–Medium | `dbf2sqlite` cross-checks live header against `db_*.h` and warns on mismatch. |

### 7.5 What changes when we move to Spec #3+ (AP / AR)

So we know we're not designing ourselves into a corner:

- **Adds:** new screens as `fx::Dialog` subclasses; new tb* controls only if AP/AR demand ones we haven't built (probably a grid control); more `cb4sqlite` surface (`Tag4` for index rebuilds, locking primitives if multi-user becomes a goal); more `dbf2sqlite` table coverage.
- **No expected change to:** the build system, framework core, Unicode strategy, `Window`/`Dialog` base classes, project layout. Spec #1 is intentionally where those decisions get made because they're hardest to revisit.

### 7.6 Out of scope for the entire migration

- Web/cloud version.
- Multi-user concurrent access (the original was single-user).
- Touch / tablet UI.
- Internationalization beyond what English string resources naturally allow.
- Replacing SQLite with a server database (Postgres, SQL Server). The shim's API is narrow enough that this swap is achievable later, but it's a deliberate non-goal now.
- Automated UI tests.
- Performance work (modern hardware vs. 1994 → speed is not a concern).

---

## 8. Open questions to resolve during implementation

These are deliberately deferred to the implementation plan rather than guessed at now:

1. **Exact dialog IDs** for login and company-profile flows in `gl/gl_acces.cpp` and `common/gl_prof.cpp` — read during the implementation pass and added to the kept-resources list in §6.1.
2. **Exact tag names** on the `glaccts` table — read from `gl/db.def`.
3. **Whether `tbSuperBtn` themed-button approximation is visually acceptable** — decided after first integration with the Chart of Accounts dialog. If not acceptable, owner-draw escalation is a known follow-up within #1.
4. **Whether to use Catch2 or a hand-rolled test macro** for `cb4sqlite_tests` — decided at first test write. Default to hand-rolled (zero deps); switch to Catch2 only if test ergonomics suffer.
5. **Specific BSD-licensed DBF reader to vendor** — research during `dbf2sqlite` implementation. Candidates: `libdbf` (LGPL — would need shim), `dbfread`-style C ports, or write our own (~200 lines).

---

## Appendix A — Files known-to-be-touched by Spec #1

**New files (all created):**
```
gl.sln
win32fx/...                     (entire directory, ~15 files)
cb4sqlite/...                   (entire directory + sqlite/ + tools/)
cb4sqlite_tests/...
gl/App.{cpp,h}
gl/ManilaGL.{cpp,h}
gl/Login.{cpp,h}
gl/CompanyProfile.{cpp,h}
gl/ChartOfAccounts.{cpp,h}
gl/AccountList.{cpp,h}
gl/stubs/ReportingStubs.cpp
gl/stubs/ApStubs.cpp
gl/stubs/ArStubs.cpp
gl/res/manifest.xml
gl/res/bitmaps/...              (extracted from existing gl.rc)
gl/gl.vcxproj
docs/superpowers/specs/2026-05-10-owl-to-win32-sdk-port-design.md
```

**Files modified:**
```
gl/gl.rc                        (pruned + scrubbed — original lives in git history)
common/utildlg.rc               (pruned + scrubbed; included into gl/res/)
```

**Files preserved on disk but not in build:**
```
gl/*.cpp                        (OWL originals — reference only)
gl/*.h                          (OWL originals — reference only)
gl/gl_new.rc                    (deferred to later spec)
gl/gl.exe                       (original 16-bit binary — reference)
gl/GLSRC.LZH, gl/GLEXE.LZH      (original archives)
include/                        (third-party headers — reference only; not on new include path)
owl/*.lib                       (16-bit libs — incompatible, unused)
ap/, ar/                        (archived module sources — future specs)
```

---

## Appendix B — Decisions locked in by brainstorming

For provenance: the four user-confirmed answers that drove this spec.

| # | Question | Decision |
|---|---|---|
| 1 | Primary goal | Strict Win32 SDK port (raw Win32 C API, not MFC/Qt/WTL) |
| 2 | Toolchain | Visual Studio (MSVC), 64-bit Unicode |
| 3 | Data layer | Replace CodeBase with SQLite + compatibility shim |
| 4 | Spec #1 acceptance | Shell + DB shim + one full GL screen (chart of accounts) |
| 5 | SDK code structure | Approach B — light C++ helpers (in-house mini-framework) |

---

*End of Spec #1 design.*
