# Vendored third-party code

Third-party sources are copied into the repo unmodified; there are no submodules or package managers (plan: "What NOT to do"). To upgrade one, replace its files from the upstream URL, update the version and hashes below, rebuild, and run `cb4sqlite_tests`.

All three are compiled as part of their projects with `/W3`, without `/WX` (see the `.vcxproj` comments). Our own code stays at `/W4 /WX`.

| Library | Version | Licence | Location | Used by |
|---|---|---|---|---|
| SQLite amalgamation | 3.46.1 | Public domain | `cb4sqlite/sqlite3.c`, `cb4sqlite/sqlite3.h` | `cb4sqlite.lib` (spec §4) |
| Catch2 amalgamation | 3.7.1 | BSL-1.0 (`cb4sqlite_tests/vendor/catch2/LICENSE.txt`) | `cb4sqlite_tests/vendor/catch2/catch_amalgamated.{hpp,cpp}` | `cb4sqlite_tests.exe` (provides `main()`) |
| Fluent UI System Icons (27 icons, rendered to `.ico`) | commit `64e5aa7` (2026-10-28) | MIT (`gl/res/icons/fluent/LICENSE`) | `gl/res/icons/fluent/*.ico` | `gl.exe` resources (UI refresh spec §4.2) |
| shapelib (DBF reader only) | 1.6.3 | `MIT OR LGPL-2.0-or-later`; **used under MIT** (`dbf2sqlite_core/vendor/shapelib/LICENSE-MIT`) | `dbf2sqlite_core/vendor/shapelib/{dbfopen.c,safileio.c,shapefil.h,shapefil_private.h}` | `dbf2sqlite_core.lib` (spec §4.5, §10 Q3) |

## Sources (fetched 2026-11-06)

- SQLite: `https://www.sqlite.org/2024/sqlite-amalgamation-3460100.zip` (zip SHA-256 `77823cb110929c2bcb0f5d48e4833b5c59a8a6e40cdea3936b99e199dbbe5784`). Only `sqlite3.c` and `sqlite3.h` are kept; `shell.c` and `sqlite3ext.h` are not needed. Built with `SQLITE_THREADSAFE=0`, `SQLITE_OMIT_LOAD_EXTENSION=1`, `SQLITE_DEFAULT_FOREIGN_KEYS=1` (spec §7.1).
- Catch2: `https://raw.githubusercontent.com/catchorg/Catch2/v3.7.1/extras/catch_amalgamated.{hpp,cpp}` and `.../v3.7.1/LICENSE.txt`.
- shapelib: `https://raw.githubusercontent.com/OSGeo/shapelib/v1.6.3/{dbfopen.c,safileio.c,shapefil.h,shapefil_private.h,LICENSE-MIT}`. The `.shp`/`.shx` parts of shapelib are not vendored.

- Fluent UI System Icons (fetched 2026-11-13): `https://raw.githubusercontent.com/microsoft/fluentui-system-icons/64e5aa757d1b70c24a283256ddcb8269d0f58773/assets/<Icon>/SVG/ic_fluent_<icon>_<size>_regular.svg` and `.../LICENSE`, by `gl/res/tools/make_icons.py fetch`. Not kept as SVG: `make_icons.py build` renders them into the committed `.ico` files, so their hashes are the `.ico` files' own (`git log`).

## SHA-256 of vendored files

```
6c35bc5f7f85eac9c49928bacbb02bb694b547aabf69197e058cca245ad80e83  cb4sqlite/sqlite3.c
89b62c671c5964e137409ce034941b7b05a3af2c9875aba41f47f9483d0c2515  cb4sqlite/sqlite3.h
91d47e2ddfb2949df22ff582d6c292554184da743e11d3811dcdd06b5b0a159b  cb4sqlite_tests/vendor/catch2/catch_amalgamated.hpp
450fc5f213a87668f0f48c91a2afa4f4226186fef453d36e6fa3bc184ccdb9ba  cb4sqlite_tests/vendor/catch2/catch_amalgamated.cpp
2a6563a6019570e76b834ff5067d74c2edbb06896b047bc8f43a83970e73a8f2  dbf2sqlite_core/vendor/shapelib/dbfopen.c
1ee4e7c539c102ad553a71916c25ff71a9e4ffa610199e421beb40b0be991220  dbf2sqlite_core/vendor/shapelib/safileio.c
1e33647a421d627a095324c4e302d4390f781a38709d5154c065a48e1674437f  dbf2sqlite_core/vendor/shapelib/shapefil.h
834ec1311fa8d3616ea4138dd23aa40c96ef7878d891515319e07bda63152b36  dbf2sqlite_core/vendor/shapelib/shapefil_private.h
3f46082162242563bf4f43b091871cb19064bc5a398c6d2b024f6b64c6710516  dbf2sqlite_core/vendor/shapelib/LICENSE-MIT
```
