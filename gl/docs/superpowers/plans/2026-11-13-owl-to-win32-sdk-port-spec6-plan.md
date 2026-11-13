# Spec #6 — Implementation Plan: Accounts Receivable

**Spec reference:** `gl/docs/superpowers/specs/2026-11-13-owl-to-win32-sdk-port-spec6-design.md`
**Date:** 2026-11-13
**Plan status:** In progress (S6-M0 built)

**Paths:** relative to the repository root. The legacy sources are `gl/src/ar_*.cpp` and `gl/src/gl_apupd.cpp` (`TGLAPUpdate::ARUpdate`), the AR report handlers in `gl/src/ap_repts.cpp`, the headers `gl/header/ar*.h`, the table classes `include/db_ar*.h` and `db_cust.h`, the dialogs in `gl/gl_new.rc` (1062 only in `gl/gl.rc`), and the Crystal files `dbf/AR*.RPT`. New code goes in `gl/port/` (the screens, and the portable `Ar.{h,cpp}`) and `gl/port/reports/` (the reports).

This plan carries out the Spec #6 design. If a milestone finds that the spec is wrong, **stop, fix the spec, then resume**.

**Read first, as in Specs #2–#5.** Each milestone that ports a screen starts by re-reading its part of the original and its dialog, and checking the spec's §2 against them. The AR sources don't compile as checked in (spec §2.0), so "the original" means what the source says, not a build. The spec's §2.6 lists what doesn't work; each fix is a decision (D1–D16), and each screen's translation notes name the ones it applies.

**Open questions.** Q1–Q3 are answered (spec §13): all of AR; receipts in full, in part and with a discount; no withholding. Q4–Q8 stay as the spec assumes until the finance team answers: no credit notes (Q4), no printed sales invoice (Q5), statements of open items on demand (Q6), no credit limits (Q7), the deployed customers' accounts kept (Q8). Each milestone notes where an answer would change it.

**Where it runs.** As Spec #5: Windows 11 with Visual Studio 18 (v145), each milestone built, tested and driven in the app as it is written. New portable files join `tools/linux/build_tests.sh`.

**Shared with AP.** `Ar.h` includes `Ap.h` and uses its account type (`ApAccount`), its date helpers (`AddDays`, `DaysBetween`, `OpenPeriodOf`), `AccountName` and its journal row (`JournalRow`: `AR_JSUM` has `AP_JSUM`'s layout). AR keeps its own tables, records and functions; nothing in `Ap.cpp` reads an `ar_*` table except the transfer (M3).

---

## Milestone summary

| # | Milestone | Size | Depends on | Critical path? |
|---|---|---|---|---|
| 0 | `Ar` core: tables, records, the arithmetic, modes; the fixture; tests | M | none | yes |
| 1 | Customers (1052, 1063) | S | M0 | yes |
| 2 | Sales invoice entry (1053) | M | M1 | yes |
| 3 | Post, Cancel, Recover; the AR half of the transfer (1028) | M | M2 | yes |
| 4 | Receipts (1156, 1155): full, partial, discount; reversing | M | M3 | yes |
| 5 | Purge Received Receivables (1062) | S | M4 | no |
| 6 | Reports; the report list; menu labels; `/stress`, `/checkres`; README | M | M4 | yes |
| 7 | Acceptance sheet, walkthrough, sign-off | S | all | yes |

S is less than a day, M is 1–3 days. **Parallel work:** M5 needs only M4, and can run beside M6.

---

## Milestone 0 — The portable core

> **As built (2026-11-13; built and run on Windows 11, VS 18, v145).**
> - **Read** (task 1): spec §2.1 holds against `dbf/AR_*.DBF` and `.MDX`. `TARInvoiceEntry::ModifyCurrentSlip` (`ar_btche.cpp:950-1060`) is as §2.3 says. Further findings:
>   - The real tags are not in alphabetical order (`AR_BTCH`: account, invoiceno, recno, batchno; `AR_DIST` starts with paymentreq; `AR_JSUM` with ardist). `EnsureArTables` and the fixture create them in the real order, since the first is the default.
>   - `AR_CUST.VAT_DEPT` is C4 in the DBF. A migration of the real company shows it as 6 wide only because dbf2sqlite widens the column for R0006's stray bytes.
>   - The original never wrote `QTY` or `PRICE`; the port does (§4.1).
> - **`gl/port/Ar.{h,cpp}`** (task 2):
>   - `EnsureArTables` creates the five deployed tables, with their layouts and tags, and the port's `ar_linex`, `ar_rcpt` and `ar_xfer`, when missing. Nothing is migrated: no deployed voucher exists (§2.7).
>   - `LoadArAccounts` reads `GL_SETAT` `AR`, `SALE`, `VATP` and the `CA` banks.
>   - `Customer` with `LoadCustomers` / `FindCustomer` / `SaveCustomer`. A replaced customer keeps `CREDIT` and the `TERMS_*` fields the port doesn't use.
>   - `ArVoucher` and `ArLine` (quantity in hundredths, price in cents), with `LoadArVouchers`, `FindArVoucher`, `LoadArLines`, `NextArVoucherNumber` and `SaveArVoucher`. The last replaces a voucher's lines and `ar_linex` rows in one transaction, with the memo, and the analysis codes in their own columns (D13).
>   - `DuplicateArInvoice` (D6): any voucher not cancelled, posted or not.
>   - The arithmetic is pure: `LineAmount`, `SplitSale` and `ComputeSale`.
>   - Read-only for now: `LoadArOpenItems`, `LoadReceipts`, `LoadArJournal` (Spec #5's `JournalRow`).
>   - `ar_journal` names the journal types: "AR" for invoices, "RC" for receipts. A receipt's rows carry its `RCPTNO` in `CHEQUE`.
> - **Spec change:** a discount's split (§5.2) now uses `SplitSale`, so its VAT is round(d × v ÷ (100 + v)) and its sales part the rest. The design's round(d × 100 ÷ (100 + v)) for the sales part could differ by a cent on a half-cent tie, and then a discount of a whole invoice wouldn't reverse it exactly. §5.3 now names `RCPTNO` and its link to the journal.
> - **Fixture** (task 3): `accounts_receivable()` writes the five tables, `AR_LINEX` and `AR_RCPT` (layouts and tags as the real ones; `AR_BTCH` with its memo file):
>   - customers C001–C003;
>   - vouchers 1 and 2 open (2 has a quantity × price line and a typed one), 3 posted and owed, 4 posted with receipt 1 (500.00, "R"), 5 posted with receipt 2 (800.00, "C"), 6 cancelled;
>   - 12 journal rows for 3, 4, 5 and the two receipts, all waiting.
>
>   **All 29 tables the fixture had are row-for-row identical** after migration (compared table by table), `_cb_*` metadata included. The new tables' fields and tags match the real company's.
> - **Tests** (task 4): `gl_tests/test_ar.cpp`, 7 cases:
>   - `LineAmount` and `SplitSale`, including the spec's 3,300.00, a half-cent tie, a negative amount, and sales + VAT = amount over thousands of amounts and rates;
>   - `ComputeSale`: two lines, a deletion, separate sales accounts, no VAT, nothing, and 37 lines balancing;
>   - the tables made in an empty company, and a second `EnsureArTables` changing nothing;
>   - the fixture's customers and `GL_SETAT` accounts;
>   - the fixture's vouchers, each one's lines equal to what `ComputeSale` makes of its product lines; the open items agreeing with the vouchers and receipts; the journal balanced, each invoice's AR debit its total, each receipt's credit its amount;
>   - `DuplicateArInvoice`;
>   - saving a customer (keeping `CREDIT` and `TERMS_TXT`) and vouchers, without touching the others.
>
>   `gl_tests`: 148 cases (`[ar]` 7).
> - **Checked:** Debug and Release `/W4 /WX` clean; `gl_tests` (148), `cb4sqlite_tests` (77) and `report_tests` (17) pass; `/checkres` 0; `/stress` 0 (126 s) on the new fixture.

**Goal:** AR's tables can be read and written, and a sales invoice's amounts computed, under test, on a fixture whose AR data is consistent with itself and moves no GL balance.

### Tasks

1. **Read** the table classes (`db_arbtc.h`, `db_ardst.h`, `db_cust.h`), `TARBatchEntry`'s line arithmetic and `UpdateBatchDetail` (`ar_btche.cpp`), and the deployed layouts and tags (`dbf/AR_*.DBF` and `.MDX`). Check the field meanings in spec §2.1, in particular:
   - `AR_CUST.ACCOUNT`/`DEPART` is the AR control and `AR_ACCT`/`AR_DEPT` the sales account;
   - `AR_BTCH` has no customer name and no VAT account field; `DESCRIPT` is 10 characters; `MEMOFIELD` is a memo;
   - `AR_DIST.CHEQUENO` is text (AP's is numeric), and `REMAINDER` is AR's own;
   - `AR_JSUM` has `AP_JSUM`'s layout with a 50-character `DESCRIPT`.
2. **`gl/port/Ar.{h,cpp}`**, portable (no Win32), over `cb4sqlite`:
   - **`EnsureArTables(code)`:** creates `ar_cust`, `ar_btch`, `ar_btchd`, `ar_dist` and `ar_jsum` with the deployed layouts and tags when the company lacks them, and the port's own `ar_linex` (spec §4.1), `ar_rcpt` (§5.3) and `ar_xfer` (§4.4). Run by every AR screen, the transfer and the reports. `AR_REQST`, `AR_CTRL`, `AR_SETUP`, `AR_SYS` and `AR_TAG` are left alone (§1.2).
   - **Records, amounts in cents** (Spec #2 D12), deleted rows skipped:
     - `Customer` (`AR_CUST`);
     - `ArVoucher` and `ArLine` (with quantity and price);
     - `ArOpenItem` (`AR_DIST`);
     - `Receipt` (`AR_RCPT`);
     - `AR_JSUM` as Spec #5's `JournalRow`.

     With `Load…` / `Find…` / `Save…` for customers and vouchers with their lines; the rest are read-only until their milestones.
   - **Modes** (D3: `ar_mode` "1", "0", "2", "X") and the receipt states (D8: `ar_receipt` O, R, C).
   - **The arithmetic** (spec §4.1, D4), pure functions:
     - `LineAmount(qty, price, typed)`: quantity × price to the cent, or the amount typed when there is no price;
     - `SplitSale(amount, vatRate)` → sales and VAT;
     - `ComputeSale(productLines, vatRate, accounts)` → the sales, VAT and AR lines. The voucher balances to the cent.
   - **The accounts** from `GL_SETAT`: `AR`, `SALE`, `VATP`, and the banks (`TYPE` "CA").
   - **`DuplicateArInvoice(code, customer, invoiceNo, except)`** (D6): any voucher not cancelled.
3. **The fixture** (spec §9) in `make_dev_fixture.py`, as `accounts_receivable(d)`, with the deployed layouts and tags, in period 7:
   - **Customers** C001 (a hospital, VAT 10 %, terms 30, sales 410001-10), C002 (a doctor, VAT 10 %, terms 30, sales 410001-20) and C003 (no VAT, terms 0); AR 111101, VAT 260007.
   - **Vouchers:** two open (one with two product lines); one posted and owed in full; one posted and partly received; one posted and settled; one cancelled.
   - **Receipts** in `AR_RCPT` for the partly received and the settled invoice; `AR_LINEX` for every voucher.
   - **The journal:** the posted vouchers' and the receipts' rows, all waiting, so no GL balance moves.

   It must leave every Spec #1–#5 figure unchanged: migrate the fixture before and after and compare every non-AR table row for row, as S5-M0.
4. **Tests**, `gl_tests/test_ar.cpp`:
   - **The arithmetic:** 3 × 1,100.00 at 10 % gives sales 3,000.00, VAT 300.00, AR 3,300.00 (spec §10); no VAT; a fractional quantity; rounding; a rate of 0.
   - **The voucher's computed lines:** two product lines on different sales accounts recompute one VAT and one AR line, balancing; deleting one recomputes; many lines balance.
   - **Tables:** created in an empty company; `EnsureArTables` twice changes nothing.
   - **The fixture:** its customers, vouchers, lines (quantities, prices, roles), open items, receipts and journal load with the expected amounts; the journal balances; the open items agree with the vouchers and receipts.
   - **Accounts:** `AR`, `SALE`, `VATP` and the banks.
   - **`DuplicateArInvoice`:** refused for the same customer and number, posted or not; allowed for a cancelled voucher, another customer, or the voucher itself.
   - **Saving** a customer and a voucher, without disturbing the others.
5. **Build lists:** `Ar.cpp` and the test in `gl_tests.vcxproj` and `tools/linux/build_tests.sh`; `Ar.{h,cpp}` in `gl.vcxproj` (`genproj.py`).

### Exit criteria

- `gl_tests` passes in Debug and Release.
- The Spec #1–#5 tables are unchanged on the new fixture.

---

## Milestone 1 — Customers (1052, 1063)

**Goal:** customers can be maintained.

### Tasks

1. **Read** `TARVendorAdd` and its options (`ar_vendi.cpp`, `ar_vendo.cpp`), dialogs 1052 and 1063, and Spec #5's `ApVendors.cpp`.
2. **`prune_rc.py`:** add 1052 and 1063; regenerate `gl.rc`; check `/checkres`'s text fit.
3. **`gl/port/ArCustomers.{h,cpp}`** on Spec #5's vendor screen:
   - **Enter / Set Data** on the number finds the customer (D12).
   - **Add/Mod** runs `CheckCustomer` (required fields; VAT rate 0–99; the AR, sales and VAT accounts in the chart; a VAT account with a rate), then asks before replacing (string 1093).
   - **Options (1063):** the AR, sales and VAT accounts, VAT %, terms, contact and VIN, each filled from its own field (§2.2); no W/H field (D15).
   - **New** fills `GL_SETAT` `AR`, `SALE` and `VATP` (D12).
   - **Delete** asks, then `DeleteCustomer` refuses while the customer has a voucher, an open item or a receipt, naming it.
   - **Search, Top / Bottom / Next / Previous;** the list (`GL.INI [AR_CUST]`).
4. **Wire** 4059 in `ManilaGL.cpp`; `/checkres` loads 1052 and 1063; `/stress` gains an AR step.
5. **Tests:** `CheckCustomer`'s refusals; `DeleteCustomer`'s refusals and its delete.

### Exit criteria

- On the fixture: add C004; Enter finds C001; deleting C001 is refused, naming its voucher.
- **Known:** the fixture's `GL_SETAT SALE` is 410001 with no dept, and the chart has 410001 only with depts 10 and 20, so a new customer's default sales account needs a dept. Q8 still assumed.

---

## Milestone 2 — Sales invoice entry (1053)

**Goal:** sales invoices can be entered, product line by product line, always balanced.

### Tasks

1. **Read** `TARBatchEntry` (`ar_btche.cpp`), dialog 1053, and Spec #5's `ApInvoiceEntry.cpp`.
2. **`prune_rc.py`:** add 1053; `/checkres`.
3. **`Ar.h`:** `SaveSaleLine` / `DeleteSaleLine` (add, replace or delete a product line, then recompute the VAT and AR lines and the header's totals, D4); `DeleteArVoucher` (D5). Each refuses a voucher that isn't open.
4. **`gl/port/ArInvoiceEntry.{h,cpp}`**, modal (D16), on the AP entry pattern (spec §4.2): the customer lookup fills the name, accounts, VAT rate and terms, and the due date; quantity × price or an amount; the lines list with double-click to edit and Delete; the duplicate check (D6); New and Close ask to keep a voucher made in this session (D5); the analysis codes to their own columns (D13).
5. **Wire** 5045; `/checkres`; `/stress`.
6. **Tests:** adding, replacing and deleting product lines (the totals and balance each time); a refused line on a posted voucher; `DeleteArVoucher` touching only its voucher; the spec §10 example.

### Exit criteria

- On the fixture: C001, 3 × 1,100.00 gives Cr sales 3,000.00, Cr VAT 300.00, Dr AR 3,300.00; a second line and its delete recompute; the same invoice number is refused.

---

## Milestone 3 — Post, Cancel, Recover; the transfer (1028)

**Goal:** sales invoices post to AR's ledger, and AR's journal reaches the General Ledger.

### Tasks

1. **Read** `TARPostBatch`, `TARCancelBatch` (`ar_post.cpp`, `ar_cancb.cpp`), `TGLAPUpdate::ARUpdate`, dialogs 1061, 1054 and 1030, and Spec #5's `ApPosting.cpp` and `ApTransfer.cpp`.
2. **`Ar.h`**, each in one transaction:
   - `PostArVouchers` (spec §4.3): the `AR_DIST` row ("O", `AMOUNT` = `REMAINDER`), the `AR_JSUM` rows (`TYPE` "AR"), `MODE` "0". Unbalanced, lineless or unknown-account vouchers are refused.
   - `CancelArVoucher`: "X", lines kept.
   - `RecoverArVoucher` (D7): posted, untransferred, with no receipt.
   - `TransferAr` (D2): one GL voucher per period, "Accounts Receivable GL Update", linked in `ar_xfer`; posted vouchers whose rows are all transferred get "2".
   - `UndoArTransfer`, called beside `UndoApTransfer` from Spec #2's Cancel Voucher.
3. **Screens** on Spec #5's list dialogs: Post (1061), Cancel (1054, AR's own), and Transactions > Recover Voucher > Sales Vouchers (a `prune_rc.py` menu edit).
4. **`ApTransfer.cpp`:** the AR half, after AP; the status line counts both journals' waiting rows, and the message lists both.
5. **Wire** 4049, 4053 and the Recover item; `/checkres`, `/stress`.
6. **Tests:** posting the fixture's open vouchers; refusals; cancel; recover and its refusals (transferred, received); the transfer (one voucher per period, lines, totals, a closed period left, the rows marked, cancelled vouchers kept "X"); cancelling the transfer's GL voucher puts the rows back; posting it with Spec #2, then 111101 carrying what's owed; the Spec #5 transfer test still passing with AR data present.

### Exit criteria

- On the fixture: post, cancel and recover driven in the app; one Update AR and AP makes AP's and AR's GL vouchers, which post with Spec #2's Post Voucher.

---

## Milestone 4 — Receipts (1156, 1155)

**Goal:** what customers pay is recorded and journalled, in full, in part or with a discount, and a mistaken receipt can be reversed.

### Tasks

1. **Read** `TARCreateSlip` and `TARClear` (`ar_payrq.cpp`, `ar_clear.cpp`), dialogs 1156 and 1155 (`gl_new.rc`'s, spec §2.6 #15).
2. **`prune_rc.py`:** add 1156 and 1155 from `gl_new.rc`; `/checkres`.
3. **`Ar.h`**, each in one transaction:
   - `ReceiptItems(code, all)`: the list (O and R; with `all`, C too).
   - `RecordReceipt(code, batchno, date, bank, amount, discount, reference)` (D8, D9): refuses more than owed, a closed period, an unknown bank; writes the `AR_RCPT` row, the journal (Dr bank; for a discount, Dr sales and Dr VAT by `SplitSale`; Cr AR), and the open item (`AMOUNTPAID`, `REMAINDER`, `DATEPAID`, the bank, R or C).
   - `ReverseReceipt(code, batchno, date)` (D10): the latest receipt only; waiting rows removed, transferred ones reversed in `date`'s open period; the open item put back.
4. **`gl/port/ArReceipts.{h,cpp}`:** the list (View All, Receipts), and 1155 with its W/H box hidden (Q3). No Create step (D9).
5. **Wire** 4054; `/checkres`; `/stress`.
6. **Tests:** in full; two partials; a discount after a partial (the discount's sales and VAT, AR credited the whole remainder); more than owed refused; a closed period refused; reversing a waiting and a transferred receipt; only the latest reversible.

### Exit criteria

- On the fixture: each kind of receipt driven in the app; the journal and the open items as the tests expect. Q3 still assumed.

---

## Milestone 5 — Purge Received Receivables (1062)

**Goal:** settled invoices can be removed.

### Tasks

1. **Read** `TARPurgeBatch` and dialog 1062 (`gl/gl.rc` only).
2. **`prune_rc.py`:** 1062 from `gl.rc`, as 1045 in Spec #5; the menu item moves to Maintenance beside AP's purge (D11).
3. **`Ar.h`:** `ArPurgeCandidates` / `PurgeReceived(code, before)`: C items whose last receipt is before the date, with their receipts.
4. **Screen** in `ArReceipts`: the list of what goes, and Purge asks first.
5. **Tests:** only C items go, with their receipts; O and R are never touched.

### Exit criteria

- On the fixture: the settled invoice purges; the others stay.

---

## Milestone 6 — Reports, report list, clean-up

**Goal:** the AR reports of spec §7 preview and print, from the report list and the menu.

### Tasks

1. **Read** the AR handlers in `ap_repts.cpp` and each `.RPT` named in spec §7 (`AR_DETL`, `AR_BALA`, `AR_AGEDC`, `AR_AGEDR`, `AR_VATAP`, `AR_VATGL`, `AR_JSUM`, `AR_JSUMD`, `AR_STAT`, `AR_VEND`, `ARBTCHSM`, `AR_BTCHD`, and the eiwa `ar_bstat`).
2. **`gl/port/reports/ArReports.{h,cpp}`:** the 15 reports of §7 on Spec #3's engine, on AR's own tables (D14).
3. **`RunReport.cpp`:** an `RunArReport` table, as `RunApReport`; `StressReports` loops it.
4. **`ReportList.cpp`:** the AR entries.
5. **`prune_rc.py` menu edits:** the labels that named the wrong report; 4073, 4071, 4072, 4076 and the statements on the menu; 4074 / 4075 swapped back; 8137 off.
6. **`/stress`, `/checkres`;** README.
7. **Tests:** each report's rows and totals on the fixture; the aged buckets add to the open total; every column fits A4 (`GdiMeasure`).

### Exit criteria

- Every AR report previews in the app; `/stress` and `/checkres` exit 0.

---

## Milestone 7 — Acceptance

1. **`gl/docs/superpowers/specs/2026-11-13-spec6-acceptance.md`**, in the Spec #5 sheet's shape, with spec §10's checks, the fixture's figures and the `sqlite3` queries that produce them; a `gl_tests` case that makes the same entries and checks every figure.
2. Release build check, the walkthrough with a witness, triage, sign-off.

---

## Cross-cutting

- **Per-port checklist** from the Spec #1 plan for every ported file. Each screen's header lists its translation notes, naming the decisions it applies.
- **Per-milestone gates:**
  - `/W4 /WX` clean on Debug and Release x64;
  - the milestone's tests pass;
  - `/checkres` and `/stress` exit 0;
  - a spec drift check;
  - an entry in `gl/docs/superpowers/plans/2026-05-10-progress.md`;
  - the milestone's "As built" note here.
- **What not to do:**
  - Don't post to the GL directly: the transfer writes open GL vouchers, and Spec #2's posting posts them.
  - Don't change the original tables' layouts: anything new goes in a port table (`ar_linex`, `ar_rcpt`, `ar_xfer`).
  - Don't edit `gl/res/gl.rc` by hand: change `prune_rc.py` or `port.rc`.
  - Don't let AP code touch `ar_*` tables, or AR code `ap_*` tables, except the shared transfer screen.
  - Don't run anything against the real company's AR tables before go-live is agreed: its customer master is real.

## Risks specific to the plan

| # | Severity | Risk | Mitigation |
|---|---|---|---|
| P1 | Medium | The finance team's answers to Q4–Q8 arrive after their milestones. | Each is one function in `Ar.cpp` with its test; each milestone's exit criteria record what is still assumed. |
| P2 | Medium | Extending the shared transfer breaks Spec #5's. | AP first, then AR, each in its own function; the Spec #5 transfer and acceptance tests run unchanged with AR data in the fixture. |
| P3 | Medium | The receipt dialog differs between the two `.rc` files (spec §2.6 #15). | `gl_new.rc`'s 1155 is used; M4's read task checks every ID the code uses. |
| P4 | Low | The fixture's AR data shifts a Spec #1–#5 figure. | Its journal is untransferred and its accounts' balances untouched; M0 task 3 compares every other table. |

## How to use this plan

M0 → M1 → M2 → M3 → M4, then M5 beside M6, then M7. If the plan and the spec conflict, stop, edit the spec, recheck the plan, then continue.
