# Spec #6 — Implementation Plan: Accounts Receivable

**Spec reference:** `gl/docs/superpowers/specs/2026-11-13-owl-to-win32-sdk-port-spec6-design.md`
**Date:** 2026-11-13
**Plan status:** In progress (S6-M0 to M5 built)

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

> **As built (2026-11-13; built and run on Windows 11, VS 18, v145).**
> - **Read** (task 1): spec §2.2 holds, with more found (now in §2.2):
>   - **`TARVendorAdd::SetupWindow` opened its table on `ap_vend`**, so the screen maintained the vendors.
>   - Options needed the customer on file (string 4119), and Add wrote a new customer with blank accounts.
>   - Add blanked the contact, which is on Options.
>   - Options' account check showed string 4017 ("Select a menu item...").
>   - Its VAT dept went to control 1083, which 1063 doesn't have.
>
>   Find (Set Data) did work, unlike AP's. 1063's labels match its fields ("AR Acct" 1087/1086 is `ACCOUNT`, "Sales Acct" 8079/8080 is `AR_ACCT`); only `SetupWindow`'s tests were crossed.
> - **`Ar.h`:** `CheckCustomer` (required names; VAT 0–99; the AR and sales accounts in the chart; a VAT account in the chart, and required with a rate) and `DeleteCustomer` (refused while the customer has a sales voucher, cancelled ones too, an open item or a receipt, naming it).
> - **`prune_rc.py`** (task 2): 1052 and 1063 added. `/checkres` found both "Postal Code/Zip Code" labels wrapping to 45 px in 15. They are now "Postal Code", 15 units wider, as on 1006 and 1040.
> - **`gl/port/ArCustomers.{h,cpp}`** (task 3), on the vendor screen's pattern:
>   - The screen keeps the customer it shows. **Options edits it whether it's on file or not**, and its OK checks the accounts. For a customer on file, it saves its fields at once after asking (string 1093), as the original, leaving the main screen's edits for Add.
>   - **New, and a new number with Enter, fill `GL_SETAT` `AR`, `SALE` and `VATP`** (D12).
>   - **Add** runs `CheckCustomer`. A refusal on an account or the rate names it, adds "Set it under Options." and opens Options on that field. Replacing asks first (1093). A modified customer stays shown; an added one is cleared.
>   - **Delete** asks (6094), then `DeleteCustomer`. **Find** shows the customer, or string 6017.
>   - Enter on the number shows the customer; for a new number it moves on to the short name.
>   - List shows `GL.INI [AR_CUST]`; Top / Bottom / Next / Previous stop at the ends.
>   - Options hides the W/H account and its label (D15).
> - **Wire** (task 4): 4059 in `ManilaGL.cpp`, out of the AR not-ported list; `/checkres` loads 1052 and 1063; `/stress` has an AR step, `StressAr`.
> - **Tests** (task 5): `CheckCustomer` (each refusal; the fixture's `SALE` default refused until it has a dept) and `DeleteCustomer` (by voucher, open item, receipt; not found; deleted). `gl_tests`: 150 cases (`[ar]` 9).
> - **Checked:** Debug and Release `/W4 /WX` clean; `gl_tests` passes in both; `/checkres` 0; `/stress` 0 (123 s). Driven in the app, on a scratch copy with the fixture, by messages to its own windows:
>   - C001 and Enter: shown, with the ship-to address; Options shows its own AR, sales (410001-10), VAT, contact and VIN;
>   - Delete C001: asks, then "sales voucher 1 is theirs";
>   - Top, Next, Next to the end (stops at C003), Previous, Bottom;
>   - a new C004 without a short name names the field; with the names, the default sales account (410001, no dept) is refused and Options opens with `111101`, `410001` and `260007` filled; dept 20, VAT 10, terms 15 and a contact, then Add saves it all;
>   - C004's Options, terms 45: asks 1093, saved at once; then a new city with Add: asks, saved, terms kept;
>   - Delete C004: deleted and cleared; Find C999: string 6017.
>
>   Screenshots checked the screen and Options (the W/H field hidden).
> - **Still assumed:** Q8 (the deployed customers' accounts kept).

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

> **As built (2026-11-14; built and run on Windows 11, VS 18, v145).**
> - **Read** (task 1): spec §2.3 holds. More found (now in §2.3):
>   - `prune_rc.py` takes 1053 from `gl_new.rc`, which has **Memo** (1100) and Print. Memo worked in the original: it edits `MEMOFIELD` in dialog 1060, with the menu `MEMO`.
>   - Picking a product fills its average price (`FAVPRICE`); each combo's code shows beside it (5015, 1008, 1003).
>   - A line's description is the product's name, or else the sales account's (`GetSalesDescription`); 1053 has no description field.
> - **`Ar.h`** (task 3): `SaleEntry`, `SaveSaleLine`, `DeleteSaleLine` (recompute the VAT and AR lines and the header's `PAYABLE` / `VATAMT` from all the product lines; refused, 0 / false, on a voucher that isn't open), `DeleteArVoucher` (D5), and `SaveArMemo`.
> - **`prune_rc.py`** (task 2): 1053 and 1060, and the menu `MEMO`.
>   - `/checkres` found "Terms" too wide (32 px in 30): 2 units wider.
>   - 1060 wouldn't load: rc stores `MENU "MEMO"` with its quotes (error 1814). `prune_rc.py` now writes a dialog's `MENU` unquoted.
>   - The group box " Vendor " is " Customer ".
> - **`gl/port/ArInvoiceEntry.{h,cpp}`** (task 4), modal (D16), on `ApInvoiceEntry`'s pattern:
>   - **Enter on the customer** fills the short name, the AR, sales and VAT accounts, the VAT rate, terms and VIN, and the due date (invoice date + terms; a typed one is kept). With no customer, Add or Enter shows the customer list (Cust.).
>   - **A product line** is a quantity at a unit price, or an amount typed with no price. Its account starts as the customer's sales account and can be changed per line (spec §4.1).
>   - **Add** checks the invoice number, dates, period, document, the AR, VAT and line accounts (strings 6091, 6090, 6089), the rate, and duplicates (D6, naming the voucher). No "Sample Entry": a line needs an amount.
>   - **The Distribution List** shows all the lines. Double-clicking a product line fills it back for changing; a computed line says it's computed. **Delete** asks (6098) and recomputes.
>   - **New and Close** ask to keep a voucher made in this session (1056); No removes only that voucher (D5).
>   - Top / Bottom / Next / Previous go over the open vouchers only; picking another kind in the voucher list says why it can't be changed.
>   - **Memo** (TARMemo on 1060): Save keeps the text, Quit or Esc don't, and Enter makes a new line (the original's Enter saved). It needs the voucher on file (6088).
>   - Print is hidden until the reports (S6-M6).
> - **Wire** (task 5): 5045 in `ManilaGL.cpp`, out of the AR not-ported list; `/checkres` loads 1053 and 1060; `StressAr` opens the entry screen too.
> - **Tests** (task 6): three cases. Spec §10's 3 × 1,100.00; a second, typed line; replacing and deleting lines (totals and balance each time); posted and cancelled vouchers refused; `DeleteArVoucher` touching only its voucher; the memo. `gl_tests`: 152 cases (`[ar]` 11).
> - **Checked:** Debug and Release `/W4 /WX` clean; `gl_tests` passes in both; `/checkres` 0; `/stress` 0 (123 s). Driven in the app, on a scratch copy with the fixture, by messages to its own windows:
>   - C001 and Enter: CITYHOSP, 111101, 410001-10, 260007, 10 %, 30 days, due 12/14/26;
>   - S-2001, 3 × 1,100.00: voucher 7, Cr sales 3,000.00, Cr VAT 300.00, Dr AR 3,300.00 (the exit criterion);
>   - P100 picked: price 4.25, with MR001 and DR001; 10 of them: 38.64 + 3.86, total 3,342.50; then 220.00 typed: 3,562.50;
>   - line 2 picked from the list (its quantity, price, codes) and changed to 20: 3,605.00; line 1 deleted: 305.00, balanced;
>   - the VAT line picked: "computed"; New: asks to keep;
>   - S-2001 (open voucher 7) and S-1003 (posted 3) refused; S-1006 (cancelled 6) allowed as voucher 8; Close, No: only 8 removed;
>   - reopened: Top 1, Next 2, Next 7, Next stays at 7, Previous 2; Close without asking;
>   - Memo with no voucher: 6088; voucher 1's memo saved, then a change quit unsaved.
>
>   Screenshots checked the screen, the Distribution List and the memo.

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

> **As built (2026-11-14; built and run on Windows 11, VS 18, v145).**
> - **Read** (task 1): spec §2.4 holds. `gl_new.rc`'s 1061 is cut short (no Close, no closing brace); `prune_rc.py` drops a truncated block, so gl.rc's is used. 1061 was already generated for Spec #2's GL Post, which uses the same template.
> - **`Ap.cpp` refactor:** the transfer's per-period core is `TransferJournal` (no transaction of its own, the journal and link tables, description and reference as parameters), and the undo's is `UndoJournalTransfer`. `TransferAp` and `UndoApTransfer` wrap them, with AP's voucher marking, so the Spec #5 tests run unchanged.
> - **`Ar.h`** (task 2), each in one transaction:
>   - `PostArVouchers`: the refusals are AP's (no lines, out of balance by a cent, an account not in the chart). It writes the `AR_DIST` row ("O", `AMOUNT` = `REMAINDER`), the `AR_JSUM` rows ("AR", "1", in the voucher's period, dated its invoice date) and `MODE` "0".
>   - `CancelArVoucher` sets "X" and keeps the lines.
>   - `RecoverArVoucher` refuses "transferred" (any of its rows "2") and "received" (an `AR_RCPT` row, or the item not "O"). It removes the `AR_DIST` row and the "AR" rows only.
>   - `TransferAr` runs `TransferJournal` on `ar_jsum` / `ar_xfer`, "Accounts Receivable GL Update". A posted voucher is "2" when all its "AR" rows are transferred; receipts' rows don't count, and cancelled vouchers keep "X".
>   - `UndoArTransfer` is called beside `UndoApTransfer` from Spec #2's `CancelVoucher`.
> - **`gl/port/ArPosting.{h,cpp}`** (task 3): Post (1061: pick one, or Post for all, then close), Cancel (1054, AR's own) and Recover (1015, "Recover Sales Voucher"), on `ApPosting`'s list shape.
>   - The Recover menu item is "Recover Sal&es Voucher..." (`CM_AR_RECOVER` 20505, a `prune_rc.py` menu edit). Its access is Recover General Ledger Voucher's, as AP's.
> - **`ApTransfer.cpp`** (task 4): the AR box is enabled, and both boxes start checked. The status line counts both journals' waiting rows. OK transfers AP, then AR, each in its own transaction; the message lists both, and if AR's fails, it says AP's stands.
> - **Wire** (task 5): 4049 and 4053 in `ManilaGL.cpp`, out of the AR not-ported list; `CM_AR_RECOVER`; `/checkres` loads 1054; `StressAr` opens all three lists.
> - **Tests** (task 6): four cases.
>   - Post: one voucher, then all with three refusals.
>   - Cancel and Recover, with every refusal; Recover's rows only, and posting again.
>   - The transfer: one voucher of five netted lines, 4,100.00 each way, 12 rows. Vouchers 3–5 go "2", 6 keeps "X", 1 and 2 stay open, and AP's rows are untouched. A second transfer gives its own voucher. Cancelling the first puts its rows back, and AP's transfer still runs.
>   - Posting it with Spec #2: 111101 up by 2,800.00, which is what's owed.
>
>   `gl_tests`: 156 cases (`[ar]` 15).
> - **Checked:** Debug and Release `/W4 /WX` clean; `gl_tests` passes in both; `/checkres` 0; `/stress` 0 (129 s). Driven in the app, on a scratch copy with the fixture, by messages to its own windows:
>   - Post, voucher 1 picked: its open item ("O", 3,300.00, VAT 300.00, 111101, period 7, due 11/21) and three "AR" rows;
>   - Cancel, voucher 2: "X", its four lines kept;
>   - Recover: voucher 4 refused ("a receipt has been entered"); voucher 1 recovered (its item and rows gone, "1"); posted again with Post (all): "1 sales voucher(s) posted.", closed;
>   - Update AR and AP: "7 AP and 15 AR journal row(s)" waiting; GL vouchers 104 (AP, 6,200.00) and 105 (AR, 7,400.00), dated 10/31, period 7; every row "2"; vouchers 1, 3, 4, 5 "2", 2 and 6 "X";
>   - Post Voucher > General Ledger (all): 104 and 105 posted (the fixture's 102 refused, out of balance by design). 111101 rose 6,100.00 from AR (3,300.00 + 2,200.00 + 600.00) plus the fixture's own 1,075.00.
>
>   Screenshots checked the three lists and the transfer.

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

> **As built (2026-11-14; built and run on Windows 11, VS 18, v145).**
> - **Read** (task 1): spec §2.5 holds.
>   - The list's Set View (`ID_SETDATA`) opened `TSetData`, a customer filter with no template in either `.rc`: hidden.
>   - Reset only zeroed the marks: in the port it puts the receipt screen back as it opened.
>   - The original asked about a short amount with no box ticked (8132), and the port keeps that question (now in §5.1).
> - **`prune_rc.py`** (task 2): 1156 and 1155, `gl_new.rc`'s. `/checkres` found nothing too wide.
> - **`Ar.h`** (task 3):
>   - `ReceiptItems(all)` and `FindReceiptItem`: each item carries the customer's short name, and the voucher's VAT rate, first sales account and VAT account (the discount's accounts).
>   - `RecordReceipt`, in one transaction. Refused: not found, settled, zero, too much, a discount on the whole amount, no open period for the date, a bank not in the chart. It writes the `AR_RCPT` row, the "RC" rows (`CHEQUE` = the receipt's number, dated the receipt, in its period) and the open item (R / C).
>   - `NextReceiptNumber`: the highest in `AR_RCPT` or on an "RC" row, so a reversed transferred receipt's number isn't reused.
>   - `ReverseReceipt` reverses the latest receipt only. It removes waiting rows, or adds reversing rows dated the reversal in its open period. It puts the open item back: the previous receipt's date and bank, "O" if none is left.
> - **`gl/port/ArReceipts.{h,cpp}`** (task 4):
>   - **The list (1156)**, titled "Clear Receipts": the original's ten columns, with a status line giving the count and what's owed. View All toggles to View Owed. **Create** is **Receipts**: it lists the picked invoice's receipts and offers to reverse the latest, dated today.
>   - **The receipt (1155):** the amounts, today's date, the banks (preselecting the last receipt's), and the invoice's own sales, VAT and AR accounts, read-only. Discount and Partial toggle one at a time; the W/H box, its account and label are hidden (Q3). Clear records the receipt at once (D9).
> - **Wire** (task 5): 4054 in `ManilaGL.cpp`, out of the AR not-ported list; `/checkres` loads 1156 and 1155; `StressAr` opens the list.
> - **Tests** (task 6): three cases.
>   - The list, owed and all.
>   - Every refusal; in full; two partials; then 190.00 with a discount (110.00: 100.00 sales, 10.00 VAT, AR credited the whole 300.00 remainder).
>   - Reversing waiting receipts back to "O", and a transferred one by new rows (a closed date refused), its number not reused.
>
>   `gl_tests`: 159 cases (`[ar]` 18).
> - **Checked:** Debug and Release `/W4 /WX` clean; `gl_tests` passes in both; `/checkres` 0; `/stress` 0 (125 s). Driven in the app, on a scratch copy with the fixture, by messages to its own windows:
>   - the list: S-1003 and S-1004, "2 invoice(s); 2,800.00 owed.";
>   - S-1003: 1,000.00, no box, asked, No: a partial (R, 1,200.00 owed; Dr bank, Cr 111101, period 8, dated today);
>   - then 1,100.00 with Discount: settled, 100.00 written off (Dr 410001-10 90.91, Dr 260007 9.09, Cr 111101 1,200.00);
>   - S-1004's 600.00 in full: settled;
>   - View All: three rows; a settled one picked: 8127;
>   - Receipts on S-1004: both receipts listed, the latest reversed (its rows gone, R, 600.00 owed again);
>   - 700.00 on 600.00 owed: 8133.
>
>   Screenshots checked the list and the receipt (the W/H box hidden).
> - **Still assumed:** Q3 (no withholding on receipts).

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

> **As built (2026-11-14; built and run on Windows 11, VS 18, v145).**
> - **Read** (task 1): `TARPurgeBatch` listed every "C" item. Ok deleted them all, whatever their date; double-click deleted one after asking (6028). Both closed every table. Dialog 1062 is only in `gl.rc` (a "Purge Voucher Number" static and a "&Post" button).
> - **`prune_rc.py`** (task 2):
>   - 1062 from `gl.rc`, automatically: `gl_new.rc` has none.
>   - The menu edits remove "Purge C&leared Accounts..." from Transactions and add "Purge Recei&ved Receivables..." (4062) after AP's purge on Maintenance (D11).
>   - A new `STRING_TEXT` table rewrites 4062's status-bar help, which said "Delete all AR transactions that have already been cleared".
> - **`Ar.h`** (task 3): `ArPurgeCandidates(before)`: the "C" items whose `DATEPAID` (the last receipt) is before the date. `PurgeReceived(before, only)`, in one transaction, deletes them, or the one picked, with their `AR_RCPT` rows. Open and partly received items are never touched, and the vouchers and journal stay.
> - **Screen** (task 4), `RunArPurge` in `ArReceipts.cpp`, on AP's purge pattern:
>   - "Purge Received Receivables", with a date control (`IDC_AR_PURGEDATE`, today) over the hidden 3031, labelled "Received before:".
>   - The list: voucher, customer / reference, invoice, last received, amount.
>   - "&Purge" (the template's "&Post") asks, then purges all; double-click asks 6028 and purges one.
> - **Wire:** 4062 in `ManilaGL.cpp`, out of the AR not-ported list; `/checkres` loads 1062; `StressAr` opens it.
> - **Tests** (task 5): candidates by date (on the date is not before it); settling 3 and a partial on 4; purging one, then all; 4 (R) never touched; vouchers and journal kept. `gl_tests`: 160 cases (`[ar]` 19).
> - **Checked:** Debug and Release `/W4 /WX` clean; `gl_tests` passes in both; `/checkres` 0; `/stress` 0 (129 s). Driven in the app, on a scratch copy with the fixture, by messages to its own windows:
>   - the list on today: voucher 5 (C003, S-1005, received 10/15/2026, 800.00);
>   - before 10/10: "No settled invoice was last received before that date.";
>   - before 10/16: Purge asked (answered No); double-click asked 6028, Yes: "1 invoice(s) and 1 receipt(s) purged."; its open item and receipt 2 gone, voucher 5 and its four journal rows kept; 3 (O) and 4 (R) untouched.
>
>   A screenshot checked the screen.

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
