# Spec #2 — OWL-to-Win32 SDK Port: the General Ledger Voucher Cycle

**Status:** Draft
**Date:** 2026-11-08
**Scope:** Second spec of the migration. It ports the General Ledger's transaction screens (voucher entry, posting, recovery, cancellation, inquiry, budget data, year end), the master data they need, and the grid control they share.
**Depends on:** Spec #1 (`2026-05-10-owl-to-win32-sdk-port-design.md`), which is signed off in `2026-05-10-spec1-acceptance.md`. Everything Spec #1 fixed still holds: the build (§7.1), the error idioms (§7.2), the Unicode strategy (§5), the translation pattern (§6.6) and the per-port checklist in the Spec #1 plan.

**Numbering.** Spec #1 §9 lists seven seams. Spec #1 covered seams 1 and 2, which leaves the number 2 free; this spec takes it for the first part of seam 4 ("GL module port"). The rest of seam 4, mainly Fixed Assets, becomes Spec #4. Spec #3 (reporting), #5 (AP) and #6 (AR) keep their meanings.

---

## 1. Goals & Non-Goals

### 1.1 Goals (Spec #2 deliverables)

1. **Voucher entry** (dialog 1013) with its voucher list, line list, account list and tool box, plus **Prior Period Entry** (the same dialog in prior-period mode).
2. **Posting** (1061), **Recover** (1015) and **Cancel** (1043) of General Ledger vouchers. Posting, recovery and year end each run in one database transaction (D1).
3. **Transaction Inquiry** (1029) and **Budget Data** (1017).
4. **Master data:** Department Codes (1023), Medical Reps (1006), Product Codes (1007) and Doctor Codes (1040).
5. **Year End Consolidation** (1068), with the Retained Earnings defect corrected (D2).
6. **Backup** and **Restore** of the company database (D5).
7. **`fx::Grid`**, a reusable `win32fx` grid registered as the `tbSpread` window class, so dialog templates that contain one load unchanged. Spec #1's grid-dependent gaps close with it:
   - the User Access Menu button (1159 → 1160);
   - the Cash Accounts list (1011, `TAPCashAcctsList`) beside 1034.
8. **`cb4sqlite` additions:** a `querySet` expression subset (§5.1), transactions (§5.2), and an online backup call (§5.3).

### 1.2 Non-Goals

- **Reports** (Spec #3). This includes the voucher reports, trial balances, financial statements, the Chart of Accounts and budget reports, the custom report and printer setup. They stay "not yet ported" stubs.
- **Fixed Assets** (Spec #4), including Create Voucher (6003), which will call this spec's voucher code.
- **AP and AR vouchers**, their post and cancel commands, Update AR and AP (1028) and the cheque and receipt screens (Specs #5 and #6).
- **The spreadsheet** (`gl_xls.cpp`, the GLE menu, File > Open).
- **Reindex Databases** (3047). It rebuilt `.mdx` files, and a migrated company has none. The menu item shows a message saying the database needs no reindexing (D6).
- **Maintain External Vouchers** (1051). Its handler only opens dialog 1058, whose purpose is unknown. It stays a stub (D7).
- **Month close** (1069), which the deployed original had already retired (D4).
- Multi-user locking, UNC paths and hi-DPI testing, as in Spec #1.

---

## 2. The voucher cycle in the original

This section is the behaviour to preserve. It was read from `gl/src/gl_btche.cpp`, `gl_postb.cpp`, `gl_postr.cpp`, `gl_btcan.cpp`, `gl_delrq.cpp`, `gl_trans.cpp`, `gl_budgt.cpp` and `ap_repts.cpp` (`SetPeriods`). Where the port has to pin down a detail these sources leave open, the milestone that ports the screen records it in §11, the way Spec #1 recorded its M1–M6 corrections.

### 2.1 Tables

| Table | Header | Role |
|---|---|---|
| `gl_btch` | `include/db_btch.h` (`Tglbtch`) | voucher headers: `batchno`, `period`, `mode`, debit and credit totals, user and time |
| `gl_btchd` | `include/db_btchd.h` (`Tglbtchd`) | voucher lines: `batchno`, line number, `account`, `dept`, `period`, `date`, `reference`, `cheque`, `medrep`, `product`, `drcode`, `descript`, `debit`, `credit`, `mode` |
| `gl_dist` | **wrong today**, see §5.4 | posted lines, one per `gl_btchd` line |
| `glaccts` | `include/db_accts.h` | `period1`–`period12`, `curr_bal`, `openbal`, `previous1`–`previous12`, `budget1`–`budget12` |
| `gl_pds` | `GL_PDS` (Spec #1) | the 12 periods; `status` `"0"` is open, `"1"` is closed |
| `gldept` | `include/db_dept.h` | departments |
| `gl_medrp`, `gl_itemm`, `gl_drcod` | `db_medrp.h`, `db_itemm.h` (`Tprods`), `db_drcod.h` | medical reps, products, doctors |

`mode` is `"1"` while a voucher is open and `"0"` once it is posted. A voucher is either a normal voucher or an adjustment; `Tglbtch::Open()` and `Tglbtch::Adjustment()` tell them apart (`gl_btche.cpp:276–300`).

### 2.2 Periods

The port keeps the original's period control. Periods are opened and closed in **Company Profile > Periods** (1028, ported in Spec #1), which sets `gl_pds.status`. Voucher entry's period combo lists only open periods (`SetPeriods`, `ap_repts.cpp:227`). The module bar shows the lowest open period (`GetCurrentPeriod`, `gl/port/Company.cpp`).

### 2.3 Voucher entry (1013)

- **Windows.** The dialog opens with three owned windows beside it, plus the floating tool box `TGLToolBox` (top, bottom, next, previous, new, voucher list, OK, delete, cancel):
  - the account list (`TAccountList`, already ported as `AccountList`);
  - the voucher list (`TGLViewBatchList`);
  - the line list (`TGLDistList`).
- **Numbering.** A new voucher's number is the last `gl_btch.batchno` plus 1. If the table is empty, it is `GL.INI [PERIOD] BATCHNO` plus 1.
- **Fields.** Each line takes:
  - account and dept, both checked against `glaccts` and `gldept` (`CheckGLAccount`, `CheckGLDept`);
  - period and date, reference and cheque;
  - medical rep, product and doctor, from combo boxes filled from their tables, each with a blank entry;
  - description, debit and credit.
- **Saving.** Add appends a header (`AppendHeader`) and lines (`AppendDetail`), or replaces them when the voucher already exists (`ReplaceHeader`, `ReplaceDetail`).
- **Editing.** Picking a line in the line list loads it into the fields for editing (`TGLDistList::SelectCell`). Delete removes a line or a voucher.
- **Prior Period Entry** (`gl_prior.cpp`) opens the same dialog. It refuses to modify anything that isn't a prior-period adjustment ("Only prior period adjustment entries may be modified in this program.").

### 2.4 Posting (1061)

**OK** walks every voucher in `gl_btch`:

- **Open, balanced voucher.** For each of its lines, posting:
  - adds debit minus credit to `glaccts.period<N>`, where N is the line's period;
  - adds the same amount to `glaccts.curr_bal`;
  - sets the line and header `mode` to `"0"`;
  - appends a `gl_dist` row copying the line, including `medrep`, `prod` and `drcode`.
- **Adjustment voucher** (`PostAdjustment`): it adjusts the opening balance instead.
- **Out of balance** (debits ≠ credits): the voucher is refused with string 8116, and the rest still post.
- **Afterwards:** if every voucher posted, it shows string 8115 and closes. Otherwise it redraws the list.
- **Picking a row** posts just that voucher (`SSGetData` on column 1, `gl_postb.cpp:328`).

### 2.5 Recover (1015) and Cancel (1043)

- **Recover** reverses a posted voucher. It subtracts each line back out of `period<N>` (`SetPeriodAmount`) and deletes the voucher's `gl_dist` rows, then `pack`s.
- **Cancel** deletes an open voucher's header and lines (`gl_btcan.cpp`, `gl_delrq.cpp`).

The port pins down two things during S2-M4:

- whether Recover also reverses `curr_bal`;
- what Recover leaves in `gl_btch` / `gl_btchd`: reopened (`mode` `"1"`) or removed.

### 2.6 Transaction Inquiry (1029) and Budget Data (1017)

- **Transaction Inquiry** lists the lines of one account and dept, using the query `ACCOUNT = '<account>' .AND. DEPT = '<dept>' .AND. MODE = '0'` (`gl_trans.cpp:193`).
- **Budget Data** shows `budget1`–`budget12` of one account in a 12-row grid whose amount column is editable. On save it reads the column back with `SSGetData` (`gl_budgt.cpp:39`).

### 2.7 Year end (1068)

After confirming with string 8111, `GLYearEnd` (`gl_postb.cpp`) does three things:

1. For every account whose type isn't `B`, it adds `curr_bal` to a running total and sets `curr_bal` to zero.
2. For every account, it copies `period1`–`period12` into `previous1`–`previous12`, then sets the periods to zero.
3. It adds the total to `curr_bal` of every account whose type **isn't** `R`. This is the defect D2 corrects.

---

## 3. Screens and port files

New code goes in `gl/port/`, following Spec #1's split: portable logic separate from the Win32 dialogs, so the Linux harness can test it.

| Command (ID) | Dialog | Port file(s) | Original |
|---|---|---|---|
| General Ledger Voucher (10076), toolbar `ID_GLBATCH`; `ID_BATCHENT` (1007) | 1013 | `VoucherEntry.h/.cpp`, `VoucherLists.h/.cpp` | `gl_btche.cpp` |
| Prior Period Entry (8081) | 1013 | `VoucherEntry.cpp` (a mode) | `gl_prior.cpp` |
| Post Voucher > General Ledger (10121), toolbar `ID_GLPOST` | 1061 | `PostVouchers.h/.cpp` | `gl_postb.cpp` |
| Recover General Ledger Voucher (10122) | 1015 | `RecoverVoucher.h/.cpp` | `gl_postr.cpp` |
| Cancel Voucher > General Ledger (3054) | 1043 | `CancelVoucher.h/.cpp` | `gl_btcan.cpp`, `gl_delrq.cpp` |
| General Ledger Voucher Inquiry (10072) | 1029 | `TransactionInquiry.h/.cpp` | `gl_trans.cpp` |
| Budget Data (10074) | 1017 | `BudgetData.h/.cpp` | `gl_budgt.cpp` |
| Department Codes (10061) | 1023 | `MasterData.h/.cpp` | `gl_deptc.cpp` |
| Medical Reps (6042) | 1006 | `MasterData.cpp` | `gl_medrp.cpp` |
| Product Codes (6005) | 1007 | `MasterData.cpp` | `gl_prods.cpp` |
| Doctor Codes (6040) | 1040 | `MasterData.cpp` | `gl_drcod.cpp` |
| Year End Consolidation (1068) | message box | `YearEnd.h/.cpp` | `gl_postb.cpp` |
| Backup (1072), Restore (1074) | common file dialogs | `Backup.h/.cpp` | `gl.cpp` |

The portable core is **`Vouchers.h/.cpp`**, built like `Accounts.cpp`: no Win32. It holds append and replace, post, post-adjustment, recover, cancel and year end as functions over the table classes. Each dialog collects input and calls them.

**Resources.** `gl/res/tools/prune_rc.py` gains the dialogs 1006, 1007, 1013, 1015, 1017, 1023, 1029, 1040, 1043 and 1061, plus whatever bitmaps the tool box and buttons use. `/checkres` covers them. The message strings are already in `gl/res/gl.rc`, because Spec #1 kept the whole string table.

**Captions (D3).** Three captions in the original were copied from other dialogs. The port corrects them:

- 1061 "Post Accounts Receivable Voucher" becomes "Post General Ledger Voucher";
- 1043 "Cancel Accounts Payable Journal" becomes "Cancel General Ledger Voucher";
- 1017 "General Ledger Account Maintenance - Add" becomes "General Ledger Budget Data".

*Added in S2-M2:* list headers and prompts copied from other screens, corrected the same way:

- the Department Code list's headers "Acct" / "Dept" / "Description", over its two fields, become "Dept" / "Description";
- the Medical Reps list's third header, string 8127 ("The invoice has already been cleared…"), becomes 6044 "Description", as on the Doctor list;
- the Product list's headers "Code" / "Description" / "Description" become "Code" / "Name" / "Description";
- the Product Codes update prompt 8102 ("The current asset has already been added…") becomes 8122 ("Record has already been added…");
- the Doctor Codes delete prompt 8113 ("…delete the current Medical Representative?") becomes 6052 ("Delete the record?").

Any further wrong caption found during porting is added to this list.

---

## 4. `fx::Grid` (the `tbSpread` replacement)

### 4.1 What the GL screens use

Every Spec #2 screen except Budget Data uses the grid for display only:

- `SSSetData`, `SSSetCellType`, `SSSetColWidth` and `SSSetMaxRows`;
- `SSSetBool`, for grid options;
- `SSSetColor`, for row colours;
- `SSGetData` on column 1 of the picked row, to get its key.

Budget Data is the only screen that reads back edited cells.

*Read in S2-M0* (every `SS*` call in `gl_acctl.cpp`, `gl_btche.cpp`, `gl_postb.cpp`, `gl_postr.cpp`, `gl_btcan.cpp`, `gl_delrq.cpp`, `gl_trans.cpp`, `gl_budgt.cpp`, `gl_acces.cpp`, and `gl_profi.cpp`'s 1011 list):

- **Notifications.** `SSM_DBLCLK` is the only grid message any GL screen handles: `SelectCell` / `HandleListBox` / `Select` in every header under `gl/header/` that declares one, `HandleDoubleClick` in `gl_postr.cpp`. Its `SS_CELLCOORD` gives the row (and column). The port's `GRN_PICK` replaces it; Enter also picks.
- **Colours.** One convention throughout:
  - green on blue, `RGB(192,220,192)` / `RGBCOLOR_BLUE`, marks the row picked or matched (`gl_acctl` Seek, `gl_postb` pick, `gl_btche` line and voucher lists) or a grayed menu (`gl_acces`);
  - white or `RGB(250,250,250)` on black is the normal colour, reset with `SS_ALLCOLS, SS_ALLROWS` before each mark;
  - the voucher line list's total row writes "Voucher Out Of Balance" in white on red in column 2 when debits and credits differ, and colours its two total cells green on blue (`gl_btche.cpp:813–827`).
  
  `fx::Grid::SetColor` covers all three cases (all cells, one row, one cell).
- **Two-state button cells.** `gl_acces.cpp` (1160) uses `SSSetTypeButton` with the `PLUS` / `MINUS` bitmaps for each menu's on/off state. The port shows them with `SetImageList` + `SetCellImage`.
- **Editable cells.** `gl_btche` and `gl_budgt` use `SSSetTypeEdit` and `SSSetTypeFloat` cells. Only `gl_budgt` reads edited values back. `gl_postr` and `gl_delrq` set `SSB_ARROWSEXITEDITMODE`, `SS_ENTERACTION_NEXT` and lock columns 5 and 6 of 4-column grids, which looks copied from another screen; they never read a cell other than column 1.
- **Not mapped:**
  - `SSSetFont` with "HELV" at `GetFontSize()`: the grid uses the dialog's font;
  - `SSGetColor` (`gl_btche.cpp:192`, saving the default colours): S2-M3 adds a getter if voucher entry needs it;
  - `SSGetRowHeightInPixels` (`gl_btche`, `gl_acctl`, to fill the window with blank rows): S2-M3 likewise;
  - `SSSaveToFile` (`gl_budgt`, a debugging aid);
  - `SSBuildDependencies` / `SSReCalc` (formulas; none are used).

### 4.2 Design

- **Control.** `fx::Grid` is a window class registered as **`tbSpread`**, alongside the `tb*` controls (Spec #1 §3.6), so the dialogs 1015, 1017, 1029, 1043 and 1061 load unchanged. Internally it is a report-mode ListView child that fills the control.
- **Owned windows.** The same class serves windows that the original created in code: the voucher list, the line list and the account list.
- **API.** Rows and columns are 1-based, as in `SS*`:
  - `SetColumns({header, width, align})`, `SetMaxRows`, `Clear`;
  - `SetData(col, row, text)`, `GetData(col, row)`;
  - `SetColor(col, row, back, fore)`, done with `NM_CUSTOMDRAW`. *As built:* the draft's `SetRowColor` became `SetColor` with `Grid::kAll` for either argument, because `gl_btche` colours single cells (§4.1).
  - *Added in S2-M0:* `SetStretchColumn` (the column that takes the remaining width), `SetImageList` / `SetCellImage` (1160's PLUS / MINUS cells), `ActiveRow` / `ActiveCol` / `SetActiveCell`, `SetRedraw`, and `Grid::Create` for grids made in code.
- **Notifications.** A pick (double-click or Enter) and a selection change go to the parent as `WM_COMMAND` with notification codes defined in `CustomControls.h`. This replaces OWL's `SelectCell` and `HandleListBox` messages. *As built:* `GRN_PICK`, `GRN_SELCHANGE`, `GRN_CELLCHANGED`, `GRN_KEYDOWN` and `GRN_RCLICK`, numbered as the `SSM_*` messages they replace. The last two exist for `AccountList`, which handles Esc and right-click. Inside a dialog the grid asks for Enter, so Enter picks a row instead of pressing the default button.
- **Editable columns** (Budget Data). `SetColumnEditable(col, EditKind::Float)`. Enter or F2 opens a `tbFloat` over the cell. Enter or losing focus commits the value, and Esc cancels. AP's grid screens will extend the same mechanism (Spec #1 §7.5). *As built:* `SetColumnEditable(col, floatMask, tbStyle)`, since a float column is the only kind Spec #2 needs; a double-click also starts editing; Tab commits; Up and Down commit and edit the cell above or below (`SSB_ARROWSEXITEDITMODE`).
- **`AccountList`** moves onto `fx::Grid`. Its behaviour stays as Spec #1 §6.7 #7 describes.

---

## 5. `cb4sqlite` additions

### 5.1 `querySet` expression subset

Spec #1 accepted only `.NOT. DELETED()` (and an empty query). Spec #2 accepts this grammar, case-insensitive for keywords and field names:

```
query  := clause { ".AND." clause }
clause := ".NOT." "DELETED()"
        | FIELD op 'literal'          (single or double quotes)
op     := "=" | "<>" | "#" | "!="
```

**Comparison** follows dBase with `SET EXACT OFF`:

- The field value is compared at its full stored width, with trailing blanks. So `ACCOUNT = '100    '` matches account "100", and `ACCOUNT = '10 '` doesn't.
- `=` on strings is a **left match** over the length of the right-hand literal. So `ACCOUNT = '4'` matches "410001", and `ACCOUNT = ''` matches every record. A literal longer than the field never matches.
- `<>`, `#` and `!=` are its negation.
- Values compare case-sensitively, byte by byte. Only character fields can be compared with a text literal; a numeric, date or logical field is an error, as it was a type mismatch in dBase.
- A NULL character value reads as blanks, as dBase saw an empty field.

*Corrected in S2-M1:* the draft said these rules would be "verified against `include/e4expr.h` and the CodeBase 4 documentation". The tree has neither CodeBase's source nor its documentation. `include/e4expr.h` only defines operator codes (`E4EQUAL` 26, `E4NOT_EQUAL` 30, …), and no header has an EXACT setting, nor does any GL source set one. The reference is therefore the rule `dbf2sqlite` already applies to the real tags' FOR conditions (`dbf2sqlite_core/src/KeyExpr.cpp:24–27`, `dbf2sqlite_core/README.md`): `type = 'AR '` becomes `substr("TYPE", 1, length('AR ')) = 'AR '`. The query translation uses the same SQL, so a tag's FOR condition and a query on the same expression select the same rows. Confirming it against real CodeBase behaviour needs the original binary or documentation (§12).

**Unsupported text.** A query outside the grammar still throws, now `Error("… not implemented: expression …")` (the "in spec #1" wording is gone from `notImplemented`). `AccountList` keeps its fallback message.

**Translation** (`cb4sqlite/src/QueryExpr.cpp`). Each literal becomes a named parameter (`:q1`, `:q2`, …) bound at each `top`/`bottom`/`skip`, never pasted into the SQL. Unknown fields throw `Error("… has no field X …")`. If a `querySet` call fails, the previous filter stays in place.

This covers `gl_trans.cpp` and `gl_acctl.cpp` (`ACCOUNT <> ' '`), so it also closes Spec #1 gap G2.

### 5.2 Transactions

- `Code4::begin()`, `commit()` and `rollback()`, plus an RAII `cb4sqlite::Transaction` that rolls back unless committed. These are additions; CodeBase had none.
- While a transaction is open, the record-buffer write semantics (Spec #1 §4.6) stay the same, but all the writes commit together.
- After a rollback, the in-memory record buffers of open `Data4`s may hold values that were never stored. The rule is that the caller re-reads: it closes the tables, or does `closeAll` and reopens them, as the original did after posting.
- Nesting is not supported. `begin()` inside a transaction throws.
- *As built (S2-M1):*
  - `begin()` first flushes every open `Data4`'s changed record, so earlier edits aren't swept into the transaction. It then runs `BEGIN IMMEDIATE`.
  - `commit()` also flushes the changed records first, as moving off them would have.
  - `rollback()` discards changed-but-unwritten records, so a later move doesn't write them back.
  - `close()` with a transaction open rolls it back.
  - `inTransaction()` reports the state. There is no global `using` for `Transaction`, so write `cb4sqlite::Transaction`.

### 5.3 Backup

`Code4::backupTo(path)` copies the open database with SQLite's online backup API (`sqlite3_backup_*`). Restore is handled by the application (D5).

*As built (S2-M1):*
- It flushes changed records first.
- It refuses while a transaction is open.
- It writes `<path>.part`, then moves it over `path` (replacing an existing file), so a failure leaves neither file behind.
- Paths are UTF-8, and a `std::wstring_view` overload is provided on Windows.

### 5.4 Table headers

`include/db_dist.h` came from the older `ar/` snapshot (Spec #1 §10 Q9). Its `Tgldist` opens **`ar_jsum`** and has no `medrep`, `drcode` or `prod` fields, but GL posting writes exactly those to `gl_dist`. Spec #2:

- adds a correct `Tgldist` for `gl_dist`, matching the real table's 14 fields: `account`, `dept`, `medrep`, `drcode`, `prod`, `period`, `date`, `type`, `batchno`, `reference`, `cheque`, `descript`, `debit`, `credit`;
- puts it in `gl/port/`, generated with `dbf2sqlite --headers-out` from the real table and then hand-checked;
- leaves `include/db_dist.h` untouched as legacy reference, because AR code may still include it;
- checks every other header in §2.1 against the real `.dbf` field lists in S2-M1, and records any difference here.

*As built (S2-M1).* `gl/port/db_gl_dist.h` (`Tgldist`) and `gl/port/db_gl_distb.h` (`Tgldistb`) were generated from the real tables and then hand-checked. Their default tag is **`batchno`**, string 8140, which `gl_postr.cpp` selects. The generator picked the first tag, `ardist`, which has `FOR TYPE = 'AR '` and would hide every GL line.

The other headers, compared with the real `.dbf` layouts. `gl_tests/test_tables.cpp` pins each finding against the fixture, which copies the real layouts.

| Header | Finding | Consequence for S2-M2/M3 |
|---|---|---|
| `include/db_btch.h` (`Tglbtch`) | Opens `descript`, which `gl_btch` doesn't have; the real table has `reference` instead. `gl_btche.cpp` `AppendHeader` assigns `descript` too. | Port with a GL header for `gl_btch`, mapping the voucher description to `reference`, the real column. |
| `include/db_btchd.h` (`Tglbtchd`) | Lacks `medrep`, `drcode` and `prod`, which the real `gl_btchd` has and voucher entry and posting use. The original code calls the product field `product`; the real column is **`prod`**. | Needs a correct GL header, generated like `Tgldist`. |
| `include/db_drcod.h` (`Tgldrcodes`) | Opens fields **by number**, and from field 9 on the numbers don't match the real order: the header's 9 "fax" is the real `territory`, 10 "contact" is `fax`, and so on to 17 "territory", which is `balance`. `creditlmt` (18) is never opened. | Don't use it. Open by name, generated or hand-written. |
| `include/db_medrp.h` (`Tglmedreps`) | Every field it opens exists; `territory` and `type` aren't opened. | Usable. |
| `include/db_itemm.h` (`Tprods`) | Every field it opens exists; `felement` and `fshop2` aren't opened. Its `Man()` selects a tag **`MAN`** that the real `GL_ITEMM.MDX` doesn't have; the real tags are `ffulln` and `fpcode`. | Usable; don't call `Man()`. |
| `include/db_dept.h` (`Tgldept`) | Matches. | none |

The format of the real values, which the fixture copies:
- `gl_btchd.period` is C(2), holding the period number right-aligned (`" 7"`).
- `gl_dist` copies the period and date into wider character fields, left-aligned (`" 7  "`, `"20261015  "`).
- A voucher line's `mode` is blank until posting sets `"0"`. `AppendDetail` never sets it; only the header gets `"1"` (open) or `"3"` (adjustment).

### 5.5 `dbf2sqlite`

No change. It already migrates every dBase table in `--src`, so the per-spec re-release that Spec #1 §7.5 expected isn't needed.

---

## 6. Deviations from the original (decision record)

| # | Decision | Reason |
|---|---|---|
| D1 | Posting, posting a single picked voucher, recovery and year end each run in **one transaction** (§5.2). On any error, nothing is written and the error is shown at the dispatch boundary (Spec #1 §7.2). | The original wrote row by row, so a crash halfway left the books half-posted. With SQLite this protection costs almost nothing, and it is the only change here that protects data. |
| D2 | Year end adds the total to the **Retained Earnings** account (`type == "R"`), not to every other account. It **refuses** unless exactly one account has type `R`, and names the accounts it found. | The original's `strcmp(type, "R") != 0` contradicts its own purpose (§2.7). The real data has two type-R accounts, 340000 Retained Earnings and 101000 Petty Cash, which is clearly a data error, so the corrected test would credit both. Every `previous1`–`previous12` in the real data is zero, so the original's year end never ran on this company and nothing depends on its behaviour. The Spec #1 Chart of Accounts can correct 101000's type. |
| D3 | Wrong dialog captions are corrected (§3). | They name other modules and would mislead users. |
| D4 | Month close (1069, `gl_closm.cpp`) is **not ported**. | The deployed original had already retired it: no menu item or button sends 1069, its period-advance code is commented out, `CopyINI` is empty, and it would write an uninitialized buffer to `GL.INI [PERIOD] DATE`. Periods are closed in Company Profile > Periods (§2.2). The real `GL_DISTB`, its archive table, has no rows. |
| D5 | Backup writes the database to a file chosen in a Save As dialog, using `Code4::backupTo`. The default name is `<database name>-<yyyymmdd>.db`. Restore picks a file, checks that it opens as a company database (`glaccts` and `_cb_fields` exist), confirms with string 1074, then closes the database. It saves the current file as `<name>.bak` and copies the chosen file over it. Then it reopens the database and refreshes the module bar. | The original ran external DOS programs, `savegl.pif` and `restore.pif`. With one `.db` file, a copy does the same job. |
| D6 | Reindex shows a message instead of rebuilding. | There are no `.mdx` files after migration, and SQLite maintains its own indexes. |
| D7 | Maintain External Vouchers stays a stub. | Its handler only opens dialog 1058, and nothing shows what that dialog was for. |
| D8 | Budget Data edits in the grid (§4.2) rather than in 12 separate fields. | It keeps the original layout, and AP needs grid editing anyway. |
| D9 | **Menu Access (1160)** starts from the edited level's own menu code (its `accessl` record, or the first record if it has none, as login resolves it), and only OK saves. Editing no longer changes the session. **Reset** (1159, string 8152) applies the selected level's code to the session and re-grays the frame menu. | The original edited the global `szMsg`, the logged-in user's string, so 1160 showed the session's menus rather than the selected level's. Its `Menus()` then saved that string even after Close, and Reset wrote `szMsg` into the level, the reverse of what 8152 says. |
| D10 | The master-data screens (S2-M2) stop losing data. Department Codes' Delete removes the department typed, not whatever record was current. Medical Reps and Doctor Codes no longer write `balance` from control 6039, which neither template has. Medical Reps reads territory from its own control, 1076, not 6038. Product Codes no longer writes `flotsize` from the missing control 6007, and refuses a blank code on Add/Mod (string 4072), as its Delete did. | Each one silently changed or lost saved data on every save: balances and lot sizes reset to 0, territories saved blank, and the wrong department deleted. |
| D11 | **Voucher Entry (S2-M3)** saves what it shows. The voucher description goes to `gl_btch.reference`, not the missing `descript`. A new voucher gets the number displayed, not 1 when `gl_btch` is empty. Totals come from the voucher's own lines. Add/Mod replaces the line with the typed number, or appends one. Editing keeps the voucher's mode. Picking a line selects its medical rep, product and doctor. The account check requires that account and dept, not a prefix. IDOK is Add/Mod instead of closing the dialog without the save question. | Each of these silently saved something other than what was on the screen: a lost description, a voucher renumbered to 1, totals copied from another voucher, an edit dropped, an adjustment turned into a normal voucher, codes taken from the previous line, or a line posted to the wrong account. |

---

## 7. Development fixture

`gl/test/dev-fixture/make_dev_fixture.py` already has `gl_pds` (periods 1–6 closed, so the current period is "Oct 26") and `gldept`. It grows to add:

- **`gl_btch` / `gl_btchd`:**
  - one open, balanced voucher with three lines that use a medical rep, a product and a doctor;
  - one open voucher that is out of balance;
  - one open adjustment voucher;
  - one posted voucher with its `gl_dist` rows, matching 102101's period-7 activity.
- **`gl_medrp`, `gl_itemm`, `gl_drcod`:** a few rows each.
- **`gldept`:** departments 10 and 20, which the 410001 accounts use, if they aren't there already.
- **`GL.INI`:** `[PERIOD] BATCHNO` for the empty-table case.

The fixture's 101000 keeps type `B`, so its year end has exactly one Retained Earnings account.

*As built (S2-M1):*
- **Vouchers:**
  - 100 is posted: 102101 Dr 250 / 101000 Cr 250 in period 7, with its two `gl_dist` rows. 101000 now has −250 in `period7`, so the books' period columns agree with `gl_dist`.
  - 101 is open and balanced: 111101 Dr 1,150 / 410001-10 Cr 600 / 410001-20 Cr 550, the 410001 lines with reps MR001/MR002, doctors DR001/DR002 and products P100/P200.
  - 102 is open and out of balance: 100 / 90.
  - 103 is an open adjustment (`mode "3"`): 144910 / 111101, 75.
- **Master data:** three rows each in `gl_medrp`, `gl_drcod` and `gl_itemm`.
- **Archive:** `gl_distb` is empty, as in the real company.
- **Departments** 10 and 20 were already there.
- **`GL.INI`:** `[PERIOD] BATCHNO=99`.

---

## 8. Acceptance criteria — manual checks

These follow the style of Spec #1 §6.7 and are signed off in `2026-11-08-spec2-acceptance.md`.

1. **Master data.** Department Codes, Medical Reps, Product Codes and Doctor Codes each add, modify and delete a record. Each change can be checked with `sqlite3`.
2. **Voucher entry.**
   - Create a voucher with three lines using the combo boxes. Its header and lines appear in `gl_btch` / `gl_btchd` with `mode = '1'` and the next voucher number.
   - The period combo lists only open periods.
   - An unknown account or dept is refused.
3. **Voucher edit.** Reopen a voucher from the voucher list, change a line picked in the line list, delete another line and save. The database matches.
4. **Prior Period Entry** opens the voucher dialog in prior-period mode and refuses to modify a normal voucher.
5. **Post all.**
   - Every balanced open voucher posts: each `glaccts.period<N>` and `curr_bal` changes by the voucher's lines, `gl_dist` gains one row per line, and `mode` becomes `'0'`.
   - The out-of-balance voucher is refused with string 8116 and stays open.
   - The adjustment voucher adjusts opening balances.
6. **Atomic posting.** A forced failure during posting leaves `glaccts`, `gl_dist`, `gl_btch` and `gl_btchd` exactly as they were. The check build has a fault-injection switch, `gl.exe /failpost <n>`, which throws after the nth line.
7. **Recover** reverses a posted voucher: the period columns and `gl_dist` return to their values before posting.
8. **Cancel** removes an open voucher's header and lines.
9. **Transaction Inquiry** lists the posted lines of the account and dept entered.
10. **Budget Data** edits the 12 amounts in the grid, saves them, and shows the saved values when reopened.
11. **Year end.**
    - It refuses while two accounts have type `R`, and names them.
    - With one: income and expense balances move into the Retained Earnings account, the period columns roll into the previous-year columns, and the books still balance (the sum of `curr_bal` is unchanged).
12. **Backup and Restore.** Back up, then add an account, then restore. The account is gone, the `.bak` file exists, and `gl.exe` keeps running on the restored database.
13. **Grid.**
    - The account list behaves as Spec #1 §6.7 #7 describes.
    - The User Access Menu button (1160) and the Cash Accounts list (1011) work.
    - A `GL.INI [GLACCTS] QUERY` of `ACCOUNT = '4'` lists only the 4xxxxx accounts.
14. **Regression and resources.**
    - Spec #1 §6.7 #1–#9 still pass.
    - `/checkres /quiet` exits 0.
    - `/stress /quiet` also opens and closes voucher entry, and exits 0.
    - No leaks in Debug.

---

## 9. Tests

| Layer | Required | What |
|---|---|---|
| `cb4sqlite_tests` | Yes | the `querySet` grammar: each operator, `.AND.`, left-match `=`, trailing blanks, rejected expressions; transactions (commit, rollback, RAII, nested `begin` throws); `backupTo` |
| `gl_tests` (new, Catch2; `Accounts.cpp`, `Vouchers.cpp`) | Yes | append and replace, post (balanced, out of balance, adjustment), single-voucher post, recover, cancel, year end (zero, one or two `R` accounts), each against a copy of the fixture database. It runs on Windows and also builds on Linux under ASan/UBSan, replacing the uncommitted M6 harness. |
| `trivial_dialog /selftest` | Yes | `fx::Grid`: columns, data, pick notification, an editable float cell |
| Manual | Yes | §8, signed off |

---

## 10. Risks

| # | Risk | Mitigation |
|---|---|---|
| R1 | `gl_btche.cpp` is the largest screen ported so far (1,387 lines): four windows, a tool box and several modes. The OWL lifecycle (Spec #1 risk P6) bites harder with four windows. | Port the portable logic first (`Vouchers.cpp` plus the harness). Port the windows one at a time. `/stress` covers voucher entry. |
| R2 | The dBase string comparison rules (§5.1) differ from SQL's. A wrong rule silently lists the wrong rows. | Tests pin each rule. Verify against `e4expr.h` before implementing. |
| R3 | Accounting arithmetic in `double`, as in the original, which compares debit and credit totals with `==`. | Keep `double` for fidelity. Compare totals after rounding to the field's decimals (2), and record the comparison in §11 once verified. |
| R4 | Recover's exact effects (§2.5) aren't fully pinned down. | S2-M4 reads the whole of `gl_postr.cpp` and records the result before porting. |
| R5 | Rollback leaves stale record buffers (§5.2). | The re-read rule, plus a harness test that a failed post followed by a retry gives correct totals. |

---

## 11. Milestones (plan to be written)

| # | Milestone | Depends on |
|---|---|---|
| S2-M0 | `fx::Grid` as `tbSpread`, read-only, then the editable float column. Move `AccountList` onto it. User Access Menu button (1160); Cash Accounts list (1011). | none |
| S2-M1 | `cb4sqlite`: `querySet` subset, transactions, `backupTo`, tests. Correct `Tgldist` and check the other headers. Fixture. | none |
| S2-M2 | Master data (1023, 1006, 1007, 1040). Prune the dialogs into `gl.rc`. | M0 |
| S2-M3 | `Vouchers.cpp` and harness. Voucher entry (1013) and Prior Period Entry. | M0, M1, M2 |
| S2-M4 | Post, Recover, Cancel | M3 |
| S2-M5 | Transaction Inquiry, Budget Data | M0, M1 |
| S2-M6 | Year end, Backup and Restore, Reindex message | M1 |
| S2-M7 | Acceptance walkthrough and sign-off | all |

Corrections found while porting are recorded under each milestone here, as Spec #1 did ("*Corrected in M6:* …").

---

## 12. Still open

1. **Recover's effect on `curr_bal` and on the voucher tables** (§2.5, R4). Settle in S2-M4.
2. ~~**The adjustment-voucher rules** (`PostAdjustment`), in particular which balance fields it changes and how adjustments differ from prior-period entries.~~ *Partly settled in S2-M3:* they are the same thing. Prior Period Entry (`TGLPrior`) is Voucher Entry writing mode `"3"`, which posting treats as an adjustment (`Tglbtch::Adjustment`). Which balance fields `PostAdjustment` changes is still to settle in S2-M4.
3. ~~**How voucher entry chooses the date** and whether it checks the date against the period.~~ *Settled in S2-M3:* the user enters it in the `tbDate` control. Add only requires one (string 8090) and never compares it with the period. The port keeps that.
4. ~~**What the `SSSetBool` options change** in each grid. Settle in S2-M0 and map each one to a ListView style, or record that it is ignored.~~ *Resolved in S2-M0:*

   | Option | Where | Port |
   |---|---|---|
   | `SSB_REDRAW` | all | `Grid::SetRedraw` (`WM_SETREDRAW`) |
   | `SSB_PROTECT` TRUE | post, recover, cancel, delete, budget, 1160, line list | the default: cells are read-only unless `SetColumnEditable` |
   | `SSB_SHOWROWHEADERS` FALSE | all that set it | the default: a ListView has no row headers |
   | `SSB_ALLOWUSERSELBLOCK` | TRUE in inquiry, 1160, 1011; FALSE elsewhere | ignored: one row is selected at a time (`LVS_SINGLESEL`) |
   | `SSB_MOVEACTIVEONFOCUS` FALSE | all | the default: focus doesn't move the active row |
   | `SSB_SCROLLBAREXTMODE` TRUE, `SSB_HORZSCROLLBAR` FALSE | inquiry, recover, 1160, 1011 / post, cancel, delete | the default: scroll bars only when needed; the stretch column avoids a horizontal one |
   | `SSB_ARROWSEXITEDITMODE` TRUE | recover, delete | Up and Down end an edit |
   | `SSB_AUTOSIZE`, `SSB_ALLOWUSERRESIZE` FALSE, `SSB_NOBEEP`, `SSB_ALLOWUSERFORMULAS` | 1160, 1011, line list / budget | ignored. The port's header lets the user resize columns. |
5. **The dBase comparison rules against real CodeBase 4** (§5.1). They follow `dbf2sqlite`'s FOR-condition translation, not CodeBase's own source or documentation, which aren't in the tree. Confirm them if either turns up, or with the original 16-bit build on the real data, for example by comparing the account list's rows under a `GL.INI [GLACCTS] QUERY`.
