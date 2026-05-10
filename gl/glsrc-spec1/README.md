# GLSRC Spec #1 — M0 Bootstrap Scaffold

This is the **M0 staging tree** for the OWL-to-Win32 SDK port (Spec #1).

It exists in `/tmp/glsrc-spec1/` because the sandbox can't write to `GLSRC/` directly. **Move or copy this entire tree into your real `GLSRC/` directory** before opening it in Visual Studio.

## What's in this tree

```
glsrc-spec1/
├── README.md                            ← this file
├── gl.sln                               ← VS solution (M0: cb4sqlite + cb4sqlite_tests only)
├── include/                             ← shared schema headers (also lives in your real GLSRC/include/)
│   ├── db_accts.h                       ← Tglaccts (44 fields) — pasted from your repo
│   └── db_acces.h                       ← TAccess / TAccessl — copied from /work
├── cb4sqlite/                           ← shim library (M0 stubs; M1 fills in real impls)
│   ├── cb4sqlite.vcxproj                ← static lib project, all spec §7.1 settings applied
│   ├── include/
│   │   ├── cb4sqlite.h                  ← umbrella header
│   │   ├── cb4sqlite_compat.h           ← _CLASSDEF no-op + r4eof constant
│   │   ├── Code4.h                      ← session — declarations only
│   │   ├── Data4.h                      ← cursor — declarations only
│   │   ├── Field4.h                     ← column accessor — declarations only
│   │   ├── Date4.h                      ← date helpers — declarations only
│   │   ├── Relate4set.h                 ← query helper — declarations only
│   │   └── Errors.h                     ← cb4sqlite::Error
│   ├── src/
│   │   └── stub_impl.cpp                ← every method throws "not implemented" (M0)
│   └── test/
│       └── parse_smoke.cpp              ← M0 EXIT CRITERION — must compile clean
└── cb4sqlite_tests/                     ← test harness (M0: just builds parse_smoke.cpp)
    └── cb4sqlite_tests.vcxproj
```

Other project directories (`win32fx/`, `dbf2sqlite_core/`, `dbf2sqlite/`, `gl/`) are present as empty placeholders for future milestones. They are **not** in `gl.sln` yet.

## What you do next (in order)

### 1. Move scaffold into your repo

```bash
# From your machine (NOT the sandbox):
rsync -av --ignore-existing /path/to/glsrc-spec1/  /home/mst/Projects/GLSRC/
# --ignore-existing protects your hand-written include/db_accts.h, include/db_acces.h
```

`include/db_accts.h` and `include/db_acces.h` in this scaffold are **copies of your originals** (so the staging tree is self-contained for parse-checking). Your real headers should win — that's why `--ignore-existing`.

### 2. Confirm the existing `include/` tree (Plan M0 task 1)

In `GLSRC/include/`, check that all `db_*.h` referenced from `gl/*.cpp`/`gl/*.h` are present:

```bash
cd /home/mst/Projects/GLSRC/gl
grep -hE '^#include "db_' *.cpp *.h | sort -u
# Each entry must resolve to a file in ../include/
```

Expected: `db_accts.h`, `db_acces.h`, plus any other `db_*.h` siblings I haven't seen. **Tell me if anything's still missing.**

### 3. Survey other shared headers (Plan M0 task 2)

```bash
grep -hE '^#include "' *.cpp *.h | sort -u | grep -v '^#include "[a-z]' | head -50
```

Note which fall into:

- Schema (keep, in `include/`): `db_*.h`
- CodeBase API (replace via cb4sqlite): `d4all.hpp`, anything starting with `d4`, `cb`, `f4`
- OWL (replace via win32fx): `tapplic*`, `tdialog*`, `tmdiframe*`, `twindow*`, `ttoolbar*`
- Custom string class (retire): `cstring.h`
- TBOWL custom controls (delete, win32fx replaces): `tbowl.h`, `tlist_w.h`
- Crystal Reports (stub): `crpe.h`

### 4. Vendor third-party code (Plan M0 task 3)

Drop these into the scaffold (all small, no submodules):

- **SQLite amalgamation** → `cb4sqlite/sqlite3.c` + `cb4sqlite/sqlite3.h`
  - https://www.sqlite.org/download.html → "sqlite-amalgamation-*.zip"
- **libdbf** → `dbf2sqlite_core/vendor/libdbf/` (BSD-licensed DBF reader)
  - https://github.com/hpjansson/libdbf (or alternative `shapelib`'s `dbfopen.c` — see Spec §10 Q3)
- **Catch2 v3 single-header** → `cb4sqlite_tests/vendor/catch2/catch_amalgamated.{hpp,cpp}`
  - https://github.com/catchorg/Catch2/releases → `catch_amalgamated.*`

### 5. Open `gl.sln` in Visual Studio 2022 (Plan M0 task 5/6)

Confirm:

- Both projects (`cb4sqlite`, `cb4sqlite_tests`) load.
- `cb4sqlite.vcxproj` is the **template** for all other projects' settings — clone it for win32fx / dbf2sqlite_core / dbf2sqlite / gl in M1+. Settings already applied: C++20, /W4 /WX, /permissive-, UNICODE/_UNICODE/WIN32_LEAN_AND_MEAN/NOMINMAX, static CRT (/MT), per-config IncludePath, Release hardening (/sdl /guard:cf /DYNAMICBASE /NXCOMPAT).

### 6. Build the M0 acceptance target

```
> Build > Build Solution   (Ctrl+Shift+B)
```

Expected:

- `cb4sqlite.lib` builds (from `src/stub_impl.cpp`).
- `cb4sqlite_tests.exe` builds (from `test/parse_smoke.cpp`).
- **`parse_smoke.cpp` compiles clean** — this is the M0 exit criterion.

If `parse_smoke.cpp` fails to compile:

- A schema header (`db_accts.h` or `db_acces.h`) references something the cb4sqlite stubs don't declare. Tell me what compiler error appears and I'll add to the stubs.

### 7. Mark M0 done, hand off to M1

When `parse_smoke.cpp` builds clean, M0 is done. Reply "M0 done" or paste the build log and I'll start M1 (real cb4sqlite implementation).

## Things this scaffold does NOT include (intentionally deferred)

- `win32fx`, `dbf2sqlite_core`, `dbf2sqlite`, `gl` `.vcxproj` files — added in their respective milestones (M3, M2, M2, M5).
- SQLite, libdbf, Catch2 amalgamations — too big / source-of-truth lives upstream.
- Any pruned `.rc` resources — that's M4.
- Any `gl_*.cpp` ports — M5/M6.

## Departures from spec / plan

None. This scaffold matches Spec §2.1 (project layout) and Plan M0 (six-project skeleton, IncludePath, settings).

## Plan progress

- [x] Plan M0 task 4 — cb4sqlite stub headers (`include/`)
- [x] Plan M0 task 4 (extra) — stub_impl.cpp so the lib actually links
- [x] Plan M0 task 5 (partial) — `cb4sqlite.vcxproj` + `cb4sqlite_tests.vcxproj` + `gl.sln` (2 of 6 projects)
- [x] Plan M0 task 6 — project settings encoded in `cb4sqlite.vcxproj` (clone-and-edit for the other libs)
- [x] Plan M0 task 7 — `parse_smoke.cpp` written
- [ ] Plan M0 task 1 — verify `include/` tree (user-side)
- [ ] Plan M0 task 2 — survey shared headers (user-side)
- [ ] Plan M0 task 3 — vendor third-party code (user-side)
- [ ] M0 exit criterion — `parse_smoke.cpp` actually compiles in VS (user-side)
