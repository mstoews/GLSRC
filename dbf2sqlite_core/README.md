# dbf2sqlite — dBase IV company directory → cb4sqlite database

One-way migration (spec §4.5). The `dbf2sqlite_core` static library has all the logic; `dbf2sqlite.exe` is a thin CLI over it, and the Layer-2 tests in `cb4sqlite_tests/test_dbf2sqlite.cpp` drive the library directly.

```
dbf2sqlite --src D:\GL\DBF --out D:\GL\company.db ^
           --headers-out include --headers-check include --headers-check gl\header --headers-check common
```

| Option | Meaning |
|---|---|
| `--src <dir>` | company directory: `.dbf` tables and their production `.mdx` indexes |
| `--out <file>` | database to create; refused if it exists, unless `--force` |
| `--table <name>` | only this table (repeatable). Default: **every dBase table in `--src`**, detected by header, so `ACCESS.PSW` is included |
| `--force` | overwrite `--out`; skip, with a warning, fields and tags that can't be represented |
| `--codepage <n>` | source code page. Default: each file's language-driver byte, else 1252. Japanese installs: `932` (Windows only) |
| `--headers-out <dir>` | after migrating, write `db_<table>.h` for tables no existing code opens |
| `--headers-check <dir>` | existing headers and sources to scan for `Data4(cb, "<table>")` / `Data4::open(cb, "<table>")` (repeatable) |

Exit codes: `0` ok, `1` bad arguments, `2` source read error, `3` destination write error, `4` schema/index translation error, `5` row-count verification mismatch.

## What it does

1. **Tables.** Each table becomes `cb4sqlite::createTable(...)`, which records the dBase type, width and decimals in `_cb_fields`. Rows go in record order inside one transaction, with `_rowid` = dBase record number. Records marked deleted keep `_deleted = 1`, because CodeBase showed them until `pack()`.
2. **Values.**
   - `C`: converted to UTF-8 and right-padded.
   - `N`/`F`: INTEGER or REAL.
   - `D`: `YYYYMMDD`.
   - `L`: 0/1.
   - Blank and unreadable values become NULL; unreadable ones get a warning (at most 5 per field, then a total).
3. **Widening.** If a character value is longer in UTF-8 than its declared byte width, the field is widened and reported. Text is never truncated.
4. **Tags.** Tags come from the production `.mdx` (`S4MDX`: the app's CodeBase build used dBase IV indexes, not `.ndx`/`.cdx`). Each tag's dBase expression becomes a `key_expr`:

   | dBase | SQL |
   |---|---|
   | `ACCOUNT+DEPT` | `"ACCOUNT" \|\| "DEPT"` |
   | `UPPER(x)` | `upper(x)` |
   | `DTOS(d)` | `ifnull(d, '        ')` |
   | `STR(n,len,dec)` | `printf('%len.decf', ifnull(n, 0))` |

   Not representable (exit 4, or skipped with `--force`): FOR clauses, descending tags, other functions, alias references. UNIQUE tags migrate as normal tags; any duplicate keys are an error (or a warning with `--force`).
5. **Verify.** Every table is reopened through the shim and its row count compared.
6. **Atomic.** Everything is built in `<out>.tmp` and renamed into place only on success.

## Header generation

A table is *covered* if any `.h`/`.hpp`/`.cpp` in the check directories, or in the output directory, opens it. File names can't be used to decide this: `glaccts` lives in `db_accts.h`, and `access.psw` in `db_acces.h`. Covered tables are skipped. For the others, `db_<table>.h` is written: a `T<table> : public Data4` with one `Field4*` per field, a destructor that frees them, and no timestamps, so output is deterministic. Existing files are never overwritten. `test/golden/db_accessm.h` is the committed expected output, and it is also compiled and run by the tests.

## Fixtures

`test/make_fixtures.py` writes `test/fixtures/{company,bad,corrupt_dbf,corrupt_mdx}`; the output is byte-for-byte reproducible. **No real GL data was available when they were made:** the `.mdx` files follow the layout this reader implements (from CodeBase's `S4MDX` structures in `include/d4data.h`). Run the tool once on a real company directory before relying on it (spec §4.8).
