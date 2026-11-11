# Spec #4 — OWL-to-Win32 SDK Port: Fixed Assets

**Status:** Draft, for review
**Date:** 2026-11-11
**Scope:** Fourth spec of the migration. It ports the General Ledger's Fixed Assets menu: the asset register, asset types, disposal, the monthly depreciation voucher, and the two fixed-asset reports (1019, 1020) on Spec #3's report engine. Because the original's depreciation cycle can't run more than once per asset (§2.4), the spec also fixes that cycle (D2–D6).
**Depends on:** Specs #1, #2 and #3. Spec #2's voucher code (`Vouchers.h`: numbering, saving, posting, cancelling) carries the depreciation voucher; Spec #3's `report` library prints the reports. Everything they fixed still holds: the build, the error idioms, the Unicode strategy, the translation pattern and the per-port checklist.

**Numbering.** Spec #2 left "the rest of seam 4, mainly Fixed Assets" to Spec #4 (Spec #2 header). AP and AR stay Specs #5 and #6.

---

## 1. Goals & Non-Goals

### 1.1 Goals (Spec #4 deliverables)

1. **Asset Types** (6036, dialog 4002): add, modify, delete and browse the types. A type names the GL accounts for the asset cost, the depreciation expense and the accumulated depreciation.
2. **Fixed Assets Add/Modify** (6000, dialog 4004): the asset register. Code, type, description, reference, date purchased, cost, salvage value, life, months used, method, and the computed monthly depreciation, accumulated depreciation and net book value.
3. **Create Voucher** (6003, dialog 4003): the month's depreciation as one open GL voucher, which is posted the usual way (Spec #2). The run can be repeated every month (D2), never depreciates an asset below its salvage value (D4), and can be undone by cancelling its voucher (D5).
4. **Disposal** (6035, dialog 1001): marks an asset sold or scrapped, with its date, instead of deleting it (D7).
5. **Reports** on the Spec #3 engine:
   - **Fixed Assets Listing** (1019, `GL_FA.RPT`): the register, with a check against the ledger (D11);
   - **Fixed Asset Types** (1020, `GL_FATYP.RPT`).
6. **Tables:** `gl_fa` and `gl_fatyp` as migrated, created when a company doesn't have them, and one new table, `gl_fadep`, the depreciation history (§5).

### 1.2 Non-Goals

- **A disposal voucher.** Disposal records the sale; the entry that removes the cost and accumulated depreciation and books the gain or loss is made in Voucher Entry (D7, Q3).
- **Tax depreciation**, multiple books, revaluation, impairment, partial disposals, transfers between types or departments.
- **Depreciation methods beyond straight line and declining balance** (sum of the years' digits, units of production).
- **Mid-month and half-year conventions** beyond D8.
- **Back-dated runs:** depreciation is created for the current period only, as in the original.
- **Importing an asset register** from a spreadsheet.

---

## 2. Fixed assets in the original

All of it is in `gl/src/gl_fa.cpp` (1,055 lines); the reports are Crystal files opened by `gl_xls.cpp:467-482`. The class headers are `gl/header/gl_fa.h`, `gl_asstl.h` and `gl_fatyp.h`; the table classes' header `db_fa.h` isn't in the repository, so the field meanings below come from the code and the data.

### 2.1 Tables

**`GL_FA`** (one row per asset; tag `FACODE`):

| Field | Type | Meaning in the code |
|---|---|---|
| `FACODE` | C10 | asset code (key) |
| `FATYPE` | C4 | asset type → `GL_FATYP` |
| `DESCRIPT`, `REFERENCE` | C40, C40 | description; reference (in the data: the person who holds it) |
| `DATE_PUR` | D | date purchased |
| `DATE_SOLD` | D | never written |
| `DEP_METH` | C10 | "Straight" or "Declining" |
| `COST`, `SALVAGE` | N16.2 | cost; salvage value |
| `LIFETIME` | N16 | life in **months** |
| `LIFELEFT` | N16 | despite the name, **months used** (the dialog's "Months Used"; the voucher run adds 1) |
| `PERCENT` | N2 | declining rate, % a year |
| `CURR_DEP` | N16.2 | monthly depreciation |
| `ACCUM_DEP` | N16.2 | never written |
| `MODE` | C1 | "0" once a voucher has been created for the asset; blank before |

**`GL_FATYP`** (one row per type; tag `FATYPE`): `FATYPE` C4, `FADESC` C30, `FAYEARS` N2 and `FAPERCENT` N2 (never used), and three account triples `ASSET_ACCT`/`_DEPT`/`_DESC`, `EXPEN_…`, `ACCUM_…` (account C7, dept C4, description C40).

### 2.2 Screens

- **Add/Modify (4004).**
  - **Fields:** code, method (combo: Straight, Declining), date purchased (and a time control that is never set), description, reference, and the type. The type's three accounts are shown read-only.
  - **Amounts:** cost, salvage, accumulated depreciation, net book value, life in months, life in years (months ÷ 12), months used, declining rate, monthly depreciation.
  - **Buttons:** New, Top / Bottom / Next / Previous, Types and Assets (lists beside the dialog, like the account list), Add/Mod, Close. There is also a "Full Year" check box.
  - **Add/Mod** validates the three accounts (strings 6056–6058), salvage ≤ cost (9097) and months used ≤ life (9098), then appends or, after asking (8102), modifies.
  - **The monthly amount:** straight line is (cost − salvage) ÷ life. Declining is (cost − salvage) × rate ÷ 12, a fixed amount, so it is straight line at a rate. "Full Year" ticked sets months used to 12.
- **Asset Types (4002):** type, description, the three accounts; Add (validates the accounts; modifies after asking, 8108), Delete (asks, 8109), New and navigation, Types list.
- **Disposal (1001):** the asset's code, method, dates, description and reference, and a "Disposal Data" box labelled Sale Price, Life, Rmd, Salvage Amount, Gain/Loss on Sale. **Delete** asks (8107), then deletes and packs the current record.
- **Create Voucher (4003):** shows the current period; OK asks (8110), then writes the voucher (§2.3).

### 2.3 The depreciation voucher

`TGLCreateDeprSlip::Ok`:
1. The voucher number is the last `gl_btch` record's number + 1.
2. For each asset whose `MODE` isn't "0":
   - append two lines: Dr the type's expense account, Cr its accumulated-depreciation account, each for `CURR_DEP`, dated today, in the current period, with the asset's reference;
   - add 1 to months used and set `MODE` to "0".
3. If any lines were written, append a header: mode 1 (open), "Monthly Depreciation Voucher" (8118), today's date, the current period, and **debit and credit totals of 0**.

An asset whose type is missing gets message 8120 and no lines.

### 2.4 What doesn't work

1. **Each asset is depreciated once, ever.** The run sets `MODE` to "0" and skips assets in mode "0"; nothing sets it back. The second month's run creates nothing.
2. **No limit.** Months used can pass the life, and the amount never stops at the salvage value. The life check (9098) is only on data entry.
3. **Accumulated depreciation** isn't stored (`ACCUM_DEP` is never written). The screen shows monthly × months used. Net book value is shown as cost − salvage − that, which is not a net book value.
4. **The report disagrees with the screen.** `GL_FA.RPT` computes the monthly amount as cost ÷ life, ignoring salvage, and accumulated depreciation as that × months used.
5. **Disposal deletes the asset**, with no date sold and no gain or loss. Its dialog shows the cost under "Sale Price" and the monthly amount under "Gain/Loss on Sale", because it reuses the Add/Modify field IDs. It deletes whichever record is current: typing a code does nothing (`Search` is empty).
6. **Typing a code finds nothing** on any of the three screens; only navigation and the lists do.
7. **Declining** isn't declining balance (§2.2).
8. **Voucher header totals are 0**, the line numbers repeat the asset's position for both of its lines, and the number isn't the highest + 1 if records are out of order.
9. **Deleting a type** leaves its assets pointing at nothing (then 8120 at the run).
10. `CheckInput` (required fields) is never called and always returns false; an asset can be added with no code (the record is appended, then the next Add/Mod with a blank code seeks it).
11. `FAYEARS` and `FAPERCENT` aren't on any screen. The listing report's table path is hard-coded to `c:\gl\dbf\`; the types report's title reads "General Leger".

### 2.5 The real data

Checked against the deployed company (`dbf/`):

- **`GL_FATYP`** has 26 types: A–E (machinery, furniture, EDP, leasehold, transport) and the same by department (A10…D60). They point at 154000–158001 (cost), 164000–168001 (accumulated) and 757000 (expense, by department), all of which exist in `GLACCTS`.
- **`GL_FA`** has 5 records that look like tests: F001–F003 (a PC at 100,000.00, one purchased in "1919", a reference "Mr. Nakahoasdfasdfasdf…"), and TEST and TEST1.
- **The ledger has no fixed-asset activity:** every one of those accounts has a zero balance, and no `GL_BTCH`, `GL_DIST` or `GL_DISTB` row touches them or says "Depreciation".

**So the module was set up but never used in production.** The port isn't bound to the original's numbers, and fixing the cycle (D2–D6) changes no history. The test records need a decision before the first run (Q4): F001–F003 would otherwise each depreciate by 2,081.25 a month.

---

## 3. Screens and port files

| Original | ID | Port file | Notes |
|---|---|---|---|
| `TGLTypes` | 6036, 4002 | `gl/port/FixedAssetTypes.{h,cpp}` | D9 |
| `TGLFixedAssets` | 6000, 4004 | `gl/port/FixedAssets.{h,cpp}` | D8, D9, D10 |
| `TGLFixedDelete` | 6035, 1001 | `gl/port/FixedAssetDisposal.{h,cpp}` | D7 |
| `TGLCreateDeprSlip` | 6003, 4003 | `gl/port/DepreciationRun.{h,cpp}` (dialog) | D2–D6 |
| — | — | `gl/port/Assets.{h,cpp}` | portable: tables, the depreciation arithmetic, the run, disposal; `gl_tests` checks it |
| `TAssetList`, `TAssetTypesList` | — | `TableList` (Spec #2) | `GL.INI [GL_FA]`, `[GL_FATYP]` |
| `GL_FA.RPT`, `GL_FATYP.RPT` | 1019, 1020 | `gl/port/reports/AssetReports.{h,cpp}` | D11 |

The dialogs come from `gl_new.rc` through `prune_rc.py` (`DIALOGS`), unchanged except where a decision says otherwise. Changes that a template can't carry are made in code, as Spec #3 did for 1022.

The pattern is Spec #2's: a portable core (`Assets.h`) over `cb4sqlite`, tested on Linux and Windows, and thin Win32 screens.

---

## 4. Depreciation

### 4.1 The amounts

All amounts are in cents (`rpt::ToCents`), as Spec #2 D12.

- **Depreciable amount** = cost − salvage.
- **Straight line:** monthly = depreciable ÷ life (months), rounded to the cent.
- **Declining balance (D3):** each fiscal year's monthly amount = (net book value at the start of the fiscal year) × rate ÷ 12, rounded to the cent. For an asset bought during the year, the start-of-year value is its cost. It is recomputed at the first run of each fiscal year.
- **Never below salvage (D4):** a month's amount is the smaller of the monthly amount and what is left, which is (cost − salvage − accumulated). A straight-line asset's last month takes the rounding remainder, so its total is exactly the depreciable amount.
- **An asset is due** in a run when it isn't disposed, its type exists, and what is left is above 0.
- **Accumulated depreciation** is stored in `ACCUM_DEP` (D6). **Net book value** = cost − accumulated.

### 4.2 The run (Create Voucher, 4003)

The dialog shows the current period (Spec #2's lowest open period) and, once the assets are read, the number of assets due and the total. OK asks (8110), then in one transaction (`cb4sqlite::Transaction`):

1. Refuses if this period of this fiscal year already has a run whose voucher still exists (D5), naming the voucher.
2. Creates one open voucher (mode 1) with `NextVoucherNumber`. The description is "Monthly Depreciation Voucher" (8118), dated the period's end date (D10), in the current period.
3. For each asset due, in code order, adds two lines: Dr the type's expense account and dept, Cr its accumulated-depreciation account and dept, each for the month's amount. The reference is the asset code; the descriptions are the type's. Lines are numbered 1, 2, 3, ….
4. Sets the header's totals (`UpdateTotals`).
5. For each asset: adds the amount to `ACCUM_DEP` and 1 to months used, and appends a `gl_fadep` row (§5.3).

Nothing due: a message, and no voucher. An asset whose type is missing is listed in a message before OK and skipped (8120).

The voucher is then posted like any other (Transactions > Post Voucher). Posting checks that the accounts exist (Spec #2), so a type pointing at a deleted account is refused there.

### 4.3 Undoing a run (D5)

**Cancel Voucher** (Spec #2, 1043) on a depreciation voucher reverses the run in the same transaction. For each `gl_fadep` row with that voucher number, it subtracts the amount from `ACCUM_DEP`, subtracts 1 from months used, and deletes the row. The period can then be run again.

A posted depreciation voucher must first be recovered (Spec #2's Recover, which doesn't touch the assets), then cancelled.

---

## 5. Tables

### 5.1 `gl_fa` and `gl_fatyp`

Kept as migrated by `dbf2sqlite`, with the meanings of §2.1 except:

- **`MODE`:** "" means active and "D" means disposed (D7). The original's "0" is read as active: it only meant "a voucher was created once" (§2.4 #1).
- **`ACCUM_DEP`** is written by the run (D6). **On first use**, a blank `ACCUM_DEP` on an asset with months used > 0 is set to monthly × months used, capped at the depreciable amount. That is what the original's screen showed, and it is done once, by the first port screen or run that opens the table, with a note in the progress log.
- **`DATE_SOLD`** is written by Disposal.

A company database without these tables gets them, empty, the first time a Fixed Assets screen opens. They are created with the original layouts (`createTable`, `createTag`), as Spec #3 created `gl_fsline`.

### 5.2 Months used

The column stays `LIFELEFT`; screens and reports call it "Months used". The life check (9098) stays.

### 5.3 `gl_fadep` — the depreciation history (new)

One row per asset per run:

| Field | Type | |
|---|---|---|
| `FACODE` | C10 | the asset |
| `YEAR_END` | D | the fiscal year's last period end date, so periods of different years don't collide |
| `PERIOD` | N2 | 1–12 |
| `BATCHNO` | N6 | the voucher |
| `AMOUNT` | N16.2 | the month's depreciation |

Tags: `FACODE`, `BATCHNO`. It gives the guard of D5, the undo, and the declining method's start-of-year value. It also gives the listing a "depreciated this period" column.

Year end (Spec #2/#3) doesn't touch any of the three tables.

---

## 6. Screens

The screens follow the Spec #2 master-data pattern (Medical Reps, Product Codes): typed codes are looked up, there is a list beside the dialog, and changes are made in transactions.

### 6.1 Asset Types (4002)

- **Add** adds the typed type, or modifies it after asking (8108). Each of the three accounts and depts must exist in `glaccts` (6056–6058), and the type and description must be filled in (8101).
- **Delete** asks (8109), then is **refused if an asset uses the type** (D9).
- **New, Top / Bottom / Next / Previous**; **Types** shows the list. Typing a type and pressing Enter shows it.

### 6.2 Fixed Assets Add/Modify (4004)

- **Enter** on the code shows that asset. **Assets** and **Types** show the lists; picking a type fills in its accounts, shown read-only.
- **Add/Mod** checks the required fields (8101: code, type, description, cost, life), that the type exists, salvage ≤ cost (9097), months used ≤ life (9098), and a valid date. It then adds the asset, or modifies it after asking (8102).
- **Computed and shown**, not typed:
  - monthly depreciation (§4.1);
  - accumulated depreciation and net book value;
  - life in years.

  Accumulated is typed only for a new asset (D6): an asset brought in part-way through its life. It is fixed once a run has depreciated the asset.
- **Method** is Straight or Declining. The declining rate shows only for Declining, as in the original.
- **"Full Year"** is hidden (D8).
- **The time control** (1035) is hidden.
- **A disposed asset** shows its date sold, and its fields are read-only.

### 6.3 Disposal (1001)

- **Enter** on the code shows the asset, with its cost, accumulated depreciation and net book value. The original's mislabelled fields are relabelled or hidden in code (D7): Sale Price typed, Gain/Loss computed as sale price − net book value.
- **Date Disposed** defaults to today.
- **Dispose** (the original's Delete button) asks (8107), then sets `DATE_SOLD` and `MODE` "D". Nothing is posted (D7, Q3); a message reminds the user to record the entry in Voucher Entry, giving the amounts.
- **Delete** (a new button) removes an asset entered by mistake. Only an asset with no `gl_fadep` rows can be deleted.

### 6.4 Create Voucher (4003)

As §4.2. The caption's "Deprecation" becomes "Depreciation".

---

## 7. Reports

On the Spec #3 engine, wired in place of the stubs for 1019 and 1020. Both are in the report list (dialog 1022).

- **1019 General Ledger Fixed Asset Summary** (`GL_FA.RPT`), landscape, at the current period, grouped by type:
  - **Columns:** code; date purchased; date sold; reference and description; months used; life; cost; salvage; monthly; accumulated; net book value (added).
  - **Totals** per type and overall.
  - **The ledger check (D11):** at the end, for each cost and accumulated-depreciation account the types use, the register's total beside the account's GL balance at the current period, with a note when they differ. Types can share accounts (the real data's A10…A60 all use 154000), so the check is per account, not per type. Disposed assets are listed (with the date sold) but not in the check.
- **1020 General Ledger Fixed Asset Types** (`GL_FATYP.RPT`): type, description, and each of the three accounts with dept and description. The title is spelled "Ledger".

---

## 8. Deviations from the original (decision record)

| # | Decision | Why |
|---|---|---|
| D1 | **Port the module, fixing the cycle.** The screens and reports keep their layouts, IDs and strings; the depreciation run, disposal and the stored amounts are made to work. | The module was never used in production (§2.5), so there is no history to stay compatible with, and as written it can't run a second month (§2.4). |
| D2 | **A run depreciates every asset that is due, every month.** `MODE` "0" no longer blocks it. | §2.4 #1. |
| D3 | **Declining is declining balance**: the rate applies to the net book value at the start of each fiscal year. | The original's "declining" is straight line at a rate (§2.2). Q2. |
| D4 | **Depreciation stops at the salvage value**; a straight-line asset's last month takes the remainder. | §2.4 #2. |
| D5 | **One run per period, guarded by `gl_fadep`**; cancelling the voucher undoes the run. | Repeating a month or losing its voucher would otherwise go unnoticed. |
| D6 | **`ACCUM_DEP` is stored** and is the accumulated depreciation everywhere. Net book value = cost − accumulated. It can be typed for a new asset. | §2.4 #3–4. One figure for the screen, the report and the ledger check. |
| D7 | **Disposal marks the asset** with the date sold and keeps it. It computes the gain or loss for information but posts nothing. A separate Delete removes only never-depreciated assets. | §2.4 #5. A disposal entry needs proceeds and gain/loss accounts that no type has (Q3). |
| D8 | **"Full Year" is hidden.** The first month is depreciated in the first run after purchase, whatever the purchase date. | Its effect (months used := 12) has no clear meaning. Q6. |
| D9 | **Typed codes are looked up** on all three screens. **A type in use can't be deleted.** | §2.4 #6, #9. |
| D10 | **The voucher is dated the period's end date**, numbered with `NextVoucherNumber`, with its totals set and its lines numbered 1, 2, 3, …. | §2.4 #8. The depreciation belongs to the period, not the day it is run. Q5. |
| D11 | **The listing checks the register against the ledger**, per account. | The register and the GL can drift apart: a disposal entry not made, a voucher not posted. The real data already differs (F001–F003 against zero balances). |
| D12 | **The reports read the open company**, not `c:\gl\dbf\`, and fix the title spelling. | As Spec #3 D12. |

---

## 9. Development fixture

`make_dev_fixture.py` gains `GL_FA`, `GL_FATYP` and an empty `gl_fadep`. It uses the Spec #3 fixture's accounts, without changing them: equipment 151000 (35,000.00: 30,000.00 opening and 5,000.00 of laptops in period 3), accumulated depreciation 161000 (6,000.00 opening and 600.00 in period 6) and depreciation expense 761000. The register agrees with those balances, so the ledger check (D11) starts clean.

- **Types:**
  - EQ: 151000 / 761000 / 161000;
  - XX: its three accounts are 999999, which isn't in the chart, for the checks. (It is written straight into the fixture: the Types screen would refuse it.)
- **Assets** (type EQ unless noted):
  - **E001** office furniture: straight line, cost 30,000.00, salvage 0, life 60 months, so 500.00 a month. 13 months used, accumulated 6,500.00: 12 months before this year, plus period 6.
  - **E002** laptops, bought in June 2026 (period 3): declining balance at 24 %, cost 5,000.00, salvage 0. The start-of-year value is its cost, so it is 100.00 a month. 1 month used, accumulated 100.00 (period 6).
  - Period 6's 600.00 is E001's 500.00 plus E002's 100.00, so cost 35,000.00 and accumulated 6,600.00 match the ledger.
  - **E004:** disposed in period 5, cost 1,000.00, fully depreciated. It is not in the check.
  - **E005:** type XX, cost 2,000.00. It is skipped by the run and named.

The salvage floor (D4), rounding and the start of a new fiscal year are checked in `gl_tests` on assets the tests add, so the fixture stays consistent with the ledger. The expected figures go in the tests and the acceptance sheet, as in Spec #3.

---

## 10. Acceptance criteria — manual checks

On a fresh copy of the fixture, as `ADMIN`:

1. **Types:** add a type with a bad account: refused, naming the account. Add a good one; modify it; delete one in use: refused. Delete an unused one.
2. **Assets:**
   - add a straight-line asset: monthly = (cost − salvage) ÷ life, accumulated 0, net book value = cost;
   - salvage > cost and months used > life are refused;
   - typing a code shows the asset; the lists pick assets and types.
3. **Create Voucher in period 7:**
   - it creates one open voucher dated October 31, 2026, with four lines: E001 500.00 and E002 100.00, each Dr 761000 / Cr 161000. It is balanced at 600.00, with its totals set;
   - E004 (disposed) and E005 (type XX, named in a message) get none;
   - E001 is at 7,000.00 and 14 months used; E002 at 200.00 and 2.
   - (The salvage floor is covered by the tests.)
4. **A second run in period 7** is refused, naming the voucher. **Cancel** that voucher in Cancel Voucher: the assets return to their values before the run, and the run works again.
5. **Post** the voucher. Trial Balance: 761000 is 1,200.00 Dr and 161000 is 7,200.00 Cr. The Fixed Assets Listing's ledger check agrees: 151000 35,000.00 and 161000 7,200.00 on both sides; 999999 (type XX) is named as not in the chart.
6. **Disposal:**
   - E002 sold at 4,000.00: its net book value is 4,800.00 after the period-7 run, so it shows a loss of 800.00;
   - Dispose marks it with the date, and the next period's run skips it. The listing's check then differs for 151000 and 161000 by E002's cost and accumulated depreciation, until the disposal entry is made in Voucher Entry;
   - Delete is refused for E001 (depreciated) and works for a new, never-depreciated asset.
7. **Reports:** 1019 and 1020 preview and print; disposed assets show their date sold; both are in the report list.
8. **Declining:** E002's monthly amount this year is 5,000.00 × 24 % ÷ 12 = 100.00. On a copy, after a year end, the next year's first run gives (5,000.00 − the year's depreciation) × 24 % ÷ 12.
9. **Regression:** the Spec #1, #2 and #3 sheets still pass; `/checkres` and `/stress` (with the three new screens) exit 0.

---

## 11. Tests

`gl_tests` (Linux and Windows), on the fixture:

- **The arithmetic:**
  - straight line, with rounding and the last-month remainder;
  - declining across a fiscal year;
  - the salvage floor;
  - a migrated blank `ACCUM_DEP`.
- **The run:** the voucher's lines, totals, date, period and number; `gl_fadep` rows; the per-period refusal; a missing type skipped; nothing due.
- **Cancel** reverses the run; a cancelled period can be run again.
- **Disposal** marks the asset; a disposed asset isn't due; Delete is refused with history.
- **Types:** account validation, and deleting a type in use is refused.
- **Reports:** row counts, totals and the ledger check, with and without a difference.
- **Missing tables** are created.

---

## 12. Risks

| # | Severity | Risk | Mitigation |
|---|---|---|---|
| R1 | High | The first run on the real company depreciates the test records F001–F003 (6,243.75 a month) into the books. | Q4 before go-live. The run lists the assets due and their total before OK. |
| R2 | Medium | The finance team expects other conventions: methods, first-month rule, disposal entry. | Q2, Q3, Q5, Q6 settled before the plan's run milestone. All are in `Assets.cpp`, in one place each. |
| R3 | Medium | Cancel Voucher (Spec #2 code) gains a fixed-asset side effect. | The undo is in `Assets.cpp`, called in Cancel's transaction; tests cover cancelling an ordinary voucher too. |
| R4 | Low | The dialogs' reused field IDs (Disposal) need code-side relabelling. | Done in `OnInitDialog`; `/checkres` loads all four templates. |

---

## 13. Milestones (plan: `2026-11-11-owl-to-win32-sdk-port-spec4-plan.md`)

| Milestone | Content | After |
|---|---|---|
| S4-M0 | `Assets.{h,cpp}`: tables (create if missing, `gl_fadep`), reading and writing assets and types, the arithmetic (§4.1); the fixture; tests | — |
| S4-M1 | Asset Types (4002) | M0 |
| S4-M2 | Fixed Assets Add/Modify (4004) | M1 |
| S4-M3 | The run (4003), the Cancel hook (§4.3) | M2; Q2, Q5, Q6 settled |
| S4-M4 | Disposal (1001) | M2; Q3 settled |
| S4-M5 | Reports 1019, 1020; the report list; `/stress`, `/checkres`; README | M3, M4 |
| S4-M6 | Acceptance sheet, walkthrough, sign-off | M5 |

---

## 14. Still open

Questions for the finance team (and the user), with the spec's assumption until they answer. Q1 is settled.

- **Q1 Is Fixed Assets wanted?** It was never used in production (§2.5). If the company keeps its register elsewhere, Spec #4 could shrink to the screens and reports as they were, or be dropped. **Settled (2026-11-11, the user): wanted, with the cycle fixed (D1).**
- **Q2 Declining method:** true declining balance (D3), or the original's fixed rate on cost? *Assumed: D3.*
- **Q3 Disposal entry:** should Disposal post the entry, and to which proceeds and gain/loss accounts? That would mean two new accounts per type or company-wide. *Assumed: no (D7).*
- **Q4 The five records in the real `GL_FA`** (F001–F003, TEST, TEST1): delete them before go-live? *Assumed: yes, by the finance team, with Delete (§6.3), before the first run.*
- **Q5 Voucher date:** the period's end date (D10), or the day of the run as in the original? *Assumed: D10.*
- **Q6 First month:** depreciate from the month of purchase, the month after, or (the "Full Year" box) a full year in the first year? *Assumed: from the first run after purchase (D8).*
