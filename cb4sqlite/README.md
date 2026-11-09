# cb4sqlite — CodeBase 4 shim over SQLite

Static library that lets the ported 1994 screens keep their CodeBase calls (`Code4`, `Data4`, `Field4`, `Date4`, `Relate4set`) while the data lives in one SQLite file per company. Spec: `gl/docs/superpowers/specs/2026-05-10-owl-to-win32-sdk-port-design.md` §4.

Ported code includes `cb4sqlite.h` where it used to include `d4all.hpp`. Hand-written schema headers (`db_*.h`) are included unchanged, between `legacy_headers_begin.h` and `legacy_headers_end.h`.

## Ground truth

The semantics follow the **original CodeBase headers in `include/`** (`d4data.hpp`, `d4defs.h`, `e4expr.h`) and the way the Spec #1 sources (`gl.cpp`, `gl_acces.cpp`, `gl_profi.cpp`, `gl_accts.cpp`, `gl_acctl.cpp`) call them. Where the spec's original sketch disagreed with CodeBase, CodeBase won and the spec was updated (§4.7).

## Storage (spec §4.3)

| Object | Layout |
|---|---|
| table `T` | `_rowid INTEGER PRIMARY KEY`, `_deleted INTEGER NOT NULL DEFAULT 0`, one column per dBase field |
| `_cb_fields` | `(table_name, field_no, field_name, type, len, dec)`: dBase type and width of each field, in record order |
| `_cb_tags` | `(table_name, tag_name, key_expr, filter)`: one row per index tag; `key_expr` is SQL producing the dBase key, e.g. `account \|\| dept`; `filter` is NULL or the SQL form of the tag's FOR clause |
| index `T__tag` | on `((key_expr), _rowid)`, created by `createTag`; partial (`WHERE (filter)`) for a filtered tag |

Column storage: `C` → TEXT right-padded to width; `N`/`F` → INTEGER (0 decimals) or REAL, blank → NULL; `D` → TEXT `YYYYMMDD`, blank → NULL; `L` → INTEGER 0/1, blank → NULL. Table names are lower-case, with any directory and `.dbf` removed (`access.psw` stays `access.psw`).

`createTable()` / `createTag()` (in `Schema.h`) build all of this; the tests and `dbf2sqlite_core` use them. Memo fields (`M`) are TEXT columns holding the whole memo. A database written before `filter` existed still opens; `ensureMetaTables()` adds the column.

## Semantics

- **Record buffer.** Each `Data4` holds the current record as fixed-width dBase text images, one per field. `Field4::str()` returns them, e.g. `"100    "` or `"      250.50"`. The pointer is valid until the cursor moves or the field is assigned, the same contract as CodeBase.
- **Writes.** `assign*()` edits the buffer. The change is written by `flush()`, or automatically before any navigation, `appendStart()`, `SetIndex()` or `close()`.
- **Append.** `appendStart()` keeps the current buffer contents; call `blank()` to clear them. `append()` inserts the buffer and positions on the new record. Moving between `appendStart()` and `append()` abandons the new record; nothing is written. `append()` without `appendStart()` throws.
- **Order.** Navigation follows the selected tag's key, then `_rowid`. With no tag selected, or after `SetIndex()` with an unknown name (which returns `e4tagName`), it follows record order. `SetIndex()` keeps the current record.
- **Filtered tags.** A tag with a FOR condition holds only the records that satisfy it: `top`, `bottom`, `skip` and `seek` through it never land on others, and a record whose fields stop matching drops out once written. `go()` and record order still reach every record. A `Relate4set` filter is combined with the tag's own.
- **Memos.** A memo field's image is its whole text, unpadded and never truncated (`""` when empty, stored as NULL). `Field4memo` is a `Field4` whose `len()` is the text length; a plain `Field4` on the same field reports the dBase width, 10.
- **Seek.** `seek(key)` is a partial (prefix) match on the full tag key. It returns `r4success` if the found key starts with `key`, `r4after` if positioned on the next key, `r4eof` if past the end. In record order it seeks on the first tag. `seek(double)` does the same for numeric keys.
- **eof / bof.** `skip()` past the end gives `r4eof`, a blank buffer, and `recNo() == recCount()+1`. `skip()` back past the start gives `r4bof`, positioned on the first record. `skip(-1)` from eof goes to the last record.
- **Deleted records** stay visible to navigation and are counted by `recCount()`, as in CodeBase without `SET DELETED`. `deleteRec()`/`recall()` set and clear the flag immediately, and `pack()` removes deleted rows and goes to `top()`. A `Relate4set` with `".NOT. DELETED()"` iterates without them.
- **Return codes** are CodeBase's real values: `r4success 0, r4found 1, r4after 2, r4eof 3, r4bof 4, r4entry 5`, and negative `e4*` codes.
- **Errors.** Expected outcomes (miss, eof, missing table or field) are return codes or `isValid() == false`. SQLite failures and misuse throw `cb4sqlite::Error`, which ported screens leave to the win32fx dispatch-boundary catch (spec §7.2). A `Data4` on a missing table is `!isValid()`: navigation returns `e4data` and its `Field4`s are invalid.
- **Text** is UTF-8 and widths are in bytes; truncation never splits a UTF-8 sequence.
- **`cb` global.** `Code4.h` declares `extern cb4sqlite::Code4 cb;` at global scope so ported code keeps writing `&cb`. It is **defined in `gl/port/App.cpp`** (plan M5), not in this library. Tests never use it; they create a local `Code4`.
- **Session.** `Code4::open(path, create=false)` opens the company file (`journal_mode=DELETE`, `foreign_keys=ON`). `closeAll()` closes every `Data4` but keeps the connection, so the legacy `cb.closeAll(); cb.init(); …reopen…` pattern works. `close()` closes the connection.

## Method coverage (plan M1 exit criterion)

Every method is either exercised by a behavioural Catch2 assertion (file in `cb4sqlite_tests/`) or listed as a compile-only stub.

| Class | Method | Covered by |
|---|---|---|
| Code4 | `open` (UTF-8 path), `close`, `isOpen`, `closeAll`, `init`, `errorCode` | `test_code4_schema.cpp` |
| Code4 | `open(std::wstring_view)` | Windows only; compile-checked, not yet exercised |
| Data4 | `top`, `bottom`, `skip`, `eof`, `bof`, `recNo`, `recCount`, `go`, `numFields`, `fieldNumber`, `isValid`, `close` | `test_data4.cpp` |
| Data4 | `SetIndex`, `select` | `test_data4.cpp` (tags) |
| Data4 | `seek(const char*)`, `seek(double)` | `test_data4.cpp` (seek), incl. the spec §4.3 padding example |
| Data4 | `appendStart`, `append`, `appendBlank`, `blank`, `flush` | `test_data4.cpp` (write, append) |
| Data4 | `deleteRec`, `recall`, `deleted`, `pack`, `zap` | `test_data4.cpp` (delete) |
| Field4 | `init`, `isValid`, `str`, `ptr`, `num`, `len`, `decimals`, `type`, `name`, `number` | `test_field4.cpp` |
| Field4 | `assign`, `assignDouble`, `assignLong`, `trim` | `test_field4.cpp` |
| Field4 | `SetField`, `AssignField` | `test_field4_win32.cpp` (Windows only) |
| Field4memo | constructors, `len`, `free` (+ inherited `str`/`assign`) | `test_field4.cpp` (memo), `test_dbf2sqlite.cpp` (via `db_arbtc.h`) |
| Date4 | `today`, `assign`, `str`, `ptr`, `len`, `year`, `month`, `day` | `test_field4.cpp` (Date4) |
| Relate4set | `querySet`, `top`, `bottom`, `skip`, `eof`, `free` | `test_relate4set.cpp` |
| Schema | `normaliseTableName`, `quoteIdent`, `createTable`, `createTag`, `loadTableInfo`, `ensureMetaTables` | `test_code4_schema.cpp`; tag filters and the pre-filter layout in `test_data4.cpp` |
| — | `Str4ptr`, `Str4long` | `m0_smoke_test.cpp` |
| — | `db_accts.h`, `db_acces.h`, `db_dept.h` unchanged | `test_legacy_headers.cpp` |

**Compile-only stubs** (exist so call sites compile; no behaviour to test):

| Method | Why |
|---|---|
| `Code4::initUndo()` | CodeBase undo log not modelled; returns 0 |
| `Data4::unlock()`, `Relate4set::unlock()` | no locking in Spec #1 (single user); return 0 |
| `Data4::appendStart(int memo)` argument | CodeBase's flag for keeping memo contents; memos live in the record buffer, so it's ignored |
| `Field4::assignDouble(…, int newLen, …)` / `assignLong(…, int newLen, …)` | `newLen` resizes `Str4` strings in CodeBase; a field's width is fixed |

**Not implemented** (throw `cb4sqlite::Error("… not implemented …")`): `Relate4set::querySet` outside the Spec #2 grammar (see `Relate4set.h`: `.NOT. DELETED()`, `FIELD = 'text'` / `<>` / `#` / `!=` on character fields, joined by `.AND.`, with dBase `SET EXACT OFF` left matching); `Data4::zap` with a record range.

**Added in Spec #2** (not CodeBase): transactions, through `Code4::begin` / `commit` / `rollback` / `inTransaction` and the RAII `cb4sqlite::Transaction`, and `Code4::backupTo`. See `Code4.h` and Spec #2 §5.2–5.3.

**Not provided at all** (port the call site instead): CodeBase `Str4*` string classes (`Str4large`, `Str4ten` …; use `std::string`/`std::wstring`, spec §5), `Data4::UpdateUserTime` (an app helper defined in legacy `gl.cpp`), locking, `Expr4`.

## Building the tests elsewhere

The library and tests are portable C++20 apart from `Field4_win32.cpp` and `test_field4_win32.cpp`, which compile to nothing off Windows. The tests also run under g++ with AddressSanitizer and UBSan; that is how they were first verified. `tools/linux/build_tests.sh --run` builds and runs them that way.
