# Spec #5 — Implementation Plan: Accounts Payable

**Spec reference:** `gl/docs/superpowers/specs/2026-11-11-owl-to-win32-sdk-port-spec5-design.md`
**Date:** 2026-11-13
**Plan status:** Draft

**Paths:** relative to the repository root. The legacy sources are `gl/src/ap_*.cpp` and `gl_apupd.cpp`, the table classes `include/db_apbtc.h`, `db_apdst.h`, `db_vend.h`, `db_jsum.h`, `db_cash.h` and `db_reqst.h`, the dialogs in `gl/gl_new.rc` (1045 only in `gl/gl.rc`), and the Crystal files `dbf/AP*.RPT`. New code goes in `gl/port/` (screens, and the portable `Ap.{h,cpp}`) and `gl/port/reports/` (the reports).

This plan carries out the Spec #5 design. If a milestone finds that the spec is wrong, **stop, fix the spec, then resume**.

**Read first, as in Specs #2–#4.** Each milestone that ports a screen starts by re-reading its part of the original and its dialog, and checking the spec's §2 against them. The spec's §2.6 lists what doesn't work; each fix is a decision (D1–D15), and each screen's translation notes say which ones apply.

**Open questions.** Q1 and Q2 are answered (spec §13): all of AP is ported, with the spec's tax rules (F mode, VAT-inclusive, W/H on the net). Q3–Q7 stay as the spec assumes until the finance team answers: invoices start open (Q3), one cheque per vendor and bank (Q4), the deployed test data cleared before go-live (Q5), cheques on plain paper (Q6), no partial payments or credit notes (Q7). Each milestone notes where an answer would change it.

**Where it runs.** Specs #1–#4 were written on Linux and run on Windows by the user. From Spec #5 the development machine is Windows 11 with Visual Studio 18 (v145), so each milestone is built, tested and driven in the app as it is written. The Linux checks (`tools/linux/`) are kept working: new portable files join `build_tests.sh`, and `wincheck_all.sh` is run when a Linux machine is available.

---

## Milestone summary

| # | Milestone | Size | Depends on | Critical path? |
|---|---|---|---|---|
| 0 | `Ap` core: tables, records, the arithmetic, modes; the fixture; tests | M | none | yes |
| 1 | Vendors (1041) | S | M0 | yes |
| 2 | Invoice voucher entry (1042) | M | M1 | yes |
| 3 | Post, Cancel, Recover; the AP → GL transfer (1028) | M | M2 | yes |
| 4 | Cash vouchers (1044): entry, post, cancel | M | M3 | no |
| 5 | Create Cheques (1045), Hold/Force; the payment's GL entry | M | M3 | yes |
| 6 | Print Cheques, Confirm Printed, Bank Reconciliation, Void, Purge | M | M5 | yes |
| 7 | Reports; the report list; menu labels; `/stress`, `/checkres`; README | M | M4, M6 | yes |
| 8 | Acceptance sheet, walkthrough, sign-off | S | all | yes |

S is less than a day, M is 1–3 days. **Parallel work:** M4 needs only M3, and can run beside M5 and M6.

---

## Milestone 0 — The portable core

> **As built (2026-11-13; built and run on Windows 11, VS 18, v145).**
> - **Read** (task 1): spec §2.1 and §2.3 hold. `GetAPExpense` (`ap_btche.cpp:817`) is as the spec summarizes it; `PAYABLE` is the gross, `VATAMT` the VAT, `REBATEAMT` the signed W/H (negative in F mode). Further findings:
>   - `AP_DIST.ACCOUNT`/`DEPT` is the **bank**, set when a cheque is made (blank after Post); `APACCOUNT`/`APDEPART` is the AP account.
>   - `AP_REQST.CLEARED` is "1" when cleared.
>   - The original's Cancel writes string 1042, "Cancelled", into the reference, and zeroes the payable and lines.
> - **The typed amounts** (spec change): the lines store each expense's net, and the gross can't be recovered from it. In F mode two grosses often give one net (at 12 %, 11.24 and 11.25 both give 10.04): a brute-force check (every amount up to 2,000.00, VAT 0–30 %) found millions of such pairs. So re-editing a line would move the invoice total by a cent. A port table, **`ap_linex`** (kind, voucher, line, role, gross), keeps each line's typed amount and role (in T mode the W/H line is a debit, like VAT). The design's §4.1 and D4 say so.
> - **`gl/port/Ap.{h,cpp}`** (task 2):
>   - `EnsureApTables` creates the eight AP tables with the deployed layouts and tags, and `ap_linex`, when missing. It then migrates D3 ("2" + "Cancelled" → "X") and returns the count.
>   - `LoadApAccounts` reads `GL_SETAT`: AP, VATR, VATP, WTXC, WTXI, and the `CA` banks.
>   - `Vendor` with `LoadVendors` / `FindVendor` / `SaveVendor`.
>   - `ApVoucher` (invoice or cash) and `ApLine`, with `LoadApVouchers`, `FindApVoucher`, `LoadApLines`, `NextApVoucherNumber` and `SaveApVoucher`. The last replaces a voucher's lines and their `ap_linex` rows in one transaction, writing the doctor and product codes to their own columns (D13).
>     - `LoadApLines` classifies a deployed voucher's lines (no `ap_linex` rows) by the header's VAT and W/H amounts. It computes an expense line's gross back with `GrossFromExpense`, in the mode `REBATEAMT`'s sign shows.
>   - `DuplicateInvoice` (D6) works across invoice and cash vouchers, skipping cancelled ones and the voucher being edited.
>   - The arithmetic is pure: `ParseTaxMode`, `SplitAmount`, `GrossFromExpense` and `ComputeVoucher`.
>   - Read-only for now: `LoadOpenItems`, `LoadCheques`, `LoadJournal`.
> - **Fixture** (task 3): `accounts_payable()` writes the eight tables and `AP_LINEX`:
>   - vendors V001–V003;
>   - invoices: 1 and 2 open, 3 posted with its open item, 4 cancelled ("X"), 5 the original's cancel ("2"), and 6 posted, paid by cheque 5001 and printed;
>   - one open cash voucher;
>   - AP's journal for invoices 3 and 6 and the payment, all waiting ("1").
>
>   Everything is in period 7, F mode, on accounts already in the chart. **Every General Ledger, voucher and fixed-asset table is row-for-row identical** after migration (compared table by table). `ACCESSM` and `ACCESSL` gain the Voucher List menu row: #42 added the item without regenerating the fixture. `sheet_queries.py` (named in task 3) isn't in the repository, so the table-by-table comparison replaced it.
> - **Tests** (task 4): `gl_tests/test_ap.cpp`, 9 cases:
>   - F and T splits, including the spec's 1,120.00 examples, the deployed sample and rounding;
>   - `ParseTaxMode` and the inverse;
>   - `ComputeVoucher`: two lines, a deletion, no tax, T mode, and 37 lines balancing in both modes;
>   - the fixture's vendors, vouchers, lines (from `ap_linex`, and classified), open items, cheque and journal;
>   - the `GL_SETAT` accounts;
>   - `DuplicateInvoice`;
>   - saving vendors and vouchers, without disturbing the others;
>   - the tables created in an empty company.
>
>   `gl_tests`: 123 cases.
> - **Checked:** Debug and Release `/W4 /WX` clean, `gl_tests` passes in both, and `/checkres` exits 0. `/stress` wasn't run: M0 has no screen.

**Goal:** AP's tables can be read and written, and an invoice's amounts computed, under test, on a fixture whose AP data is consistent with its ledger.

### Tasks

1. **Read** the table classes (`db_apbtc.h`, `db_apdst.h`, `db_vend.h`, `db_jsum.h`, `db_cash.h`, `db_reqst.h`), `GetAPExpense` and `UpdateBatchDetail` (`ap_btche.cpp`), and the deployed layouts and tags (spec §2.1, "Layouts worth knowing"). Check the field meanings the spec gives, in particular:
   - `PAYABLE` is the gross;
   - `REBATEAMT` is the signed W/H;
   - the modes and `PAYMENTCTL` codes.
2. **`gl/port/Ap.{h,cpp}`**, portable (no Win32), over `cb4sqlite`:
   - **`EnsureApTables(code)`:** creates `ap_vend`, `ap_btch`, `ap_btchd`, `ap_cash`, `ap_cashd`, `ap_dist`, `ap_reqst` and `ap_jsum` with the deployed layouts and tags when the company lacks them. It also migrates D3: a voucher with `MODE` "2" whose reference starts "Cancelled" becomes "X". It returns how many vouchers it migrated, and runs from every AP screen, the transfer and the reports, as Spec #4's `EnsureAssetTables`.
   - **Records, amounts in cents** (Spec #2 D12), deleted rows skipped:
     - `Vendor`;
     - `ApVoucher`, one type for invoice and cash vouchers, with a `kind`;
     - `ApLine`;
     - `OpenItem` (`AP_DIST`);
     - `Cheque` (`AP_REQST`);
     - `JournalRow` (`AP_JSUM`).

     Each has `Load…` / `Find…` / `Save…` functions as the screens of M1–M6 need them. M0 provides those for vendors and vouchers with their lines; the rest come with their milestones.
   - **Modes** as named constants (`ap_mode::kOpen` "1", `kPosted` "0", `kTransferred` "2", `kCancelled` "X"), and the payment states (`PAYMENTCTL` H, O, F, P, V, C).
   - **The arithmetic** (spec §4.1, D4), pure functions with no table access:
     - `SplitAmount(amount, vatRate, whRate, mode)` → expense, VAT and W/H, for F and T;
     - `ComputeVoucher(expenseLines, rates, mode, accounts)` → the VAT, W/H and AP (or bank) lines. The voucher balances to the cent.
   - **The accounts** from `GL_SETAT`: `AP`, `VATR`, `VATP`, `WTXC`, `WTXI`, and the banks (`TYPE` "CA").
   - **`DuplicateInvoice(code, vendor, invoiceNo, exceptVoucher)`** (D6), across invoice and cash vouchers not cancelled.
3. **The fixture** (spec §9) in `make_dev_fixture.py`, as `accounts_payable(d)`, with the deployed layouts and tags:
   - **Vendors** V001 (company, VAT 12 %, W/H 1 %, terms 30, AP 221002, VAT 148000), V002 (individual, VAT 0, W/H 2 %) and V003 (no VAT or W/H).
   - **Vouchers:**
     - two open invoice vouchers, one with two expense lines;
     - one posted and untransferred, with its `AP_DIST` row ("O") and `AP_JSUM` rows;
     - one cancelled ("X");
     - one migrated-style cancelled voucher ("2", reference "Cancelled …") for the D3 migration test;
     - one open cash voucher.
   - **Payments:** one paid, printed invoice ("V") with its cheque in `AP_REQST`, not cleared, and the payment's `AP_JSUM` rows.

   The expense lines use accounts already in the chart, and the AP data's journal is **untransferred**, so no GL balance moves. It must leave every Spec #1–#4 figure unchanged: check by running the sheets' queries (`sheet_queries.py`) before and after, as S4-M0 did.
4. **Tests**, `gl_tests/test_ap.cpp`:
   - **The arithmetic, F mode:** 1,120.00 at 12 % / 1 % gives 1,000.00 / 120.00 / 10.00, and AP 1,110.00. The deployed sample: 1,100.00 at 10 % / 1 % gives 1,000.00 / 100.00 / 10.00. No VAT, no W/H, both zero. A rounding case.
   - **The arithmetic, T mode:** 1,120.00 at 12 % / 1 % gives 991.15 / 118.94 / 9.91, and AP 1,120.00. A rate of 0 means none in both modes (§2.6 #5).
   - **The voucher's computed lines:** two expense lines recompute one VAT, one W/H and one AP line, balancing to the cent; deleting one recomputes.
   - **Tables:** missing tables are created; the D3 migration ("2" + "Cancelled" → "X", other "2" untouched).
   - **The fixture:** its vendors and vouchers load with the expected amounts.
   - **`DuplicateInvoice`:** refused for the same vendor and number; allowed for a cancelled voucher, another vendor, or the voucher itself.
   - **Accounts:** the `GL_SETAT` accounts and the two banks.
5. **Build lists:** add `Ap.cpp` and the test to `gl_tests.vcxproj` and `tools/linux/build_tests.sh`, and `Ap.{h,cpp}` to `gl.vcxproj` (`genproj.py`).

### Exit criteria

- `gl_tests` passes in Debug and Release.
- The Spec #1–#4 sheets' figures are unchanged on the new fixture.

---

## Milestone 1 — Vendors (1041)

> **As built (2026-11-13; built and run on Windows 11, VS 18, v145).**
> - **Read** (task 1): spec §2.2 holds. `SetData` seeks `GetField(ID_VENDORNO) == 0`, a boolean, so it never finds the vendor. `Ok` checks only the number and the names; it never checks the accounts, and keeps the last type when neither radio button is set. Further findings in dialog 1041:
>   - **The labels don't match the fields:** 4009 (`ID_VENDORCONTACT`) is labelled "TIN No."; 1039 (the VIN) "VAT No."; 5111, beside "VIN No.", is read by nothing. `ID_VENDORTEL` and `ID_VENDORSTATUS` aren't in the template. The deployed vendors use `CONTACT` (3 of 130) and never `VIN`.
>   - **Two default buttons:** Add and Delete are both 32785, so the dialog took Delete as its default, and **Enter asked to delete the vendor**.
>   - **`RADIOBUTTON … BS_AUTORADIOBUTTON`** is 0xD, `BS_SPLITBUTTON` on Win32: Individual and Company drew as split buttons that never show a check.
>   - String 6017, the original's "not found", says "Customer number not found."
> - **`prune_rc.py`** (task 2): 1041 added. A `RADIOBUTTON` with `BS_AUTORADIOBUTTON` is now written `AUTORADIOBUTTON` (only 1041 has one). `/checkres` found "W/H Tax percentage" 1 px too wide, with its field right after it: `PATCHES` renames it "W/H %", like the "VAT %" beside it (the patch can now replace text, from S5-M0's fixes).
> - **`gl/port/ApVendors.{h,cpp}`** (task 3), the Spec #2 / #4 master-data pattern:
>   - **Enter on the number**, or Set Data, shows the vendor (D12). A new number moves on to the short name.
>   - **Add** runs `Ap.h`'s new `CheckVendor`, then asks before updating an existing vendor (6037). It keeps the vendor shown after an update and clears after adding, as the original.
>     - The checks: string 8101 naming the field; a type is required; rates must be 0–99; the AP account must be in the chart; a VAT account must be in the chart, and is required with a VAT rate.
>   - **Delete** asks (6034), then calls `DeleteVendor`. That refuses while the vendor has an invoice or cash voucher (cancelled ones too), an open item or a cheque, naming it (D12).
>   - **New** fills in `GL_SETAT` AP and VATR (D12).
>   - **Search** opens the list (`GL.INI [AP_VEND]`; Vendor, Short Name, Name). Top / Bottom / Next / Previous stop at the ends.
>   - **The template's flaws, fixed in code:**
>     - 4009 is labelled "Contact"; 5111 and its label are hidden.
>     - Add is the default button (`DM_SETDEFID`), and Delete loses the default style.
>     - The pictures: New uses 4057's icon and Search 4054's, so no icon is added; `/checkres`'s `DialogCase` gained `borrowedPics` for that.
> - **Wire** (task 4): 4045 (`ID_ADDVENDOR`) in `ManilaGL.cpp`, out of `kApCommands`; `/checkres` loads 1041; `/stress` has an AP step, `StressAp`, which later screens join.
> - **Tests** (task 5): `CheckVendor` (each refusal; the deployed odd types "R" and "X" refused) and `DeleteVendor` (by invoice, by a cancelled voucher, by a cash voucher; not found; deleted, leaving the others). `gl_tests`: 125 cases.
> - **Checked:** Debug and Release `/W4 /WX` clean; `gl_tests` passes in both; `/checkres` 0; `/stress` 0 (68 s, every step). Driven in the app (a scratch copy with the fixture, typing at the screen):
>   - New shows 221002 / 148000;
>   - V001 and Enter: shown, Company checked;
>   - Delete V001: asks, then "invoice voucher 1 is theirs";
>   - V002 and Enter: Individual, W/H 2, terms 15; Next: V003;
>   - adding V004 without a name names the field; without a type, the type message; with AP 999999, the account message; then added and cleared;
>   - V004 and Enter shows what was saved; Delete removes it (no vouchers);
>   - changing V002's city asks 6037, and Yes saves only that.
>
>   Screenshots checked the labels, the radio buttons and the defaults. Found while driving: the screen's pictures and the default button, both fixed above.

**Goal:** vendors can be maintained.

### Tasks

1. **Read** `TAPVendorAdd` (`ap_vendi.cpp`, `ap_vendo.cpp`), its list, and dialog 1041.
2. **`prune_rc.py`:** add 1041; regenerate `gl.rc`. Check `/checkres`'s text fit.
3. **`gl/port/ApVendors.{h,cpp}`** on the Spec #2 master-data pattern:
   - **Add/Mod** asks before replacing a vendor. The accounts are checked (`ValidateAccount`).
   - **Defaults:** the AP account from `GL_SETAT AP`, the VAT account from `VATR` (D12).
   - **Set Data / Enter** on the number finds the vendor (D12: the original never found it).
   - **Delete** is refused while the vendor has vouchers or open items (D12), naming one.
   - **New, Top / Bottom / Next / Previous;** the vendor list (`TableList`, `GL.INI [AP_VEND]`).
4. **Wire** 4045 in `ManilaGL.cpp`; `/checkres` loads 1041; `/stress` opens it (a new AP step, which later screens join).
5. **Tests:** `Ap.h`'s vendor checks and `DeleteVendor`'s refusals.

### Exit criteria

- On the fixture: add V004; Enter finds V001; deleting V001 is refused, naming its voucher.

---

## Milestone 2 — Invoice voucher entry (1042)

> **As built (2026-11-13; built and run on Windows 11, VS 18, v145).**
> - **Read** (task 1): spec §2.3 and §2.6 hold. One Add posts the amount in 1030 (labelled "Total") on one expense account. Further findings in dialog 1042:
>   - **Account and Dept are read-only** (`tbES_STATIC`): the original filled them only from the account list.
>   - **The line fields:** 3037 "Description" is the line's description, and 3036 "Document" the voucher's reference. Debit and Credit (2030, 2031) were never read.
>   - **The rest:** Company and Individual are plain radio buttons the code checks. The original read GL_SETAT VATP as the default VAT account, then the vendor's.
> - **`prune_rc.py`** (task 2): 1042 added. `/checkres` fits it as it is.
> - **`Ap.h`** (task 3):
>   - **`SaveExpenseLine`** adds or replaces an expense line, then recomputes the VAT, W/H and credit lines from every expense line with the header's rates (D4). It writes the totals: `PAYABLE` the typed amounts, `VATAMT`, and `REBATEAMT` signed by the mode. A new voucher is numbered after the last, in mode open. It refuses any voucher that isn't open.
>   - **`DeleteExpenseLine`** removes a line and recomputes.
>   - **`DeleteApVoucher`** removes only the voucher made in this session, for D5's No.
>   - **Helpers:** `AddDays` (the due date) and `AccountName` (blank descriptions and the computed lines' text).
> - **`gl/port/ApInvoiceEntry.{h,cpp}`** (task 4), on Spec #2's Voucher Entry pattern:
>   - **The header:** Enter on the vendor fills in the name, accounts, rates, terms and type, and the due date as invoice date + terms. A typed due date is kept. The invoice date defaults to today.
>   - **Add** checks the vendor, invoice number, date, period, document and type; the AP, VAT and W/H accounts; the rates; the line's account; an amount above 0; and duplicates (D6, string 1039 naming the voucher). It then calls `SaveExpenseLine`.
>   - **The Distribution List** is Spec #2's `LineList`. A computed line can't be picked; a message says why.
>   - **Navigation:** New and Close ask 1056 for a voucher made in this session (D5). Top / Bottom / Next / Previous go through the open vouchers. The voucher list refuses a posted, transferred or cancelled voucher, naming its state.
>   - **Lists:** Vendors, Lists (accounts) and Voucher open their lists.
>   - **Template fixes in code:** Account and Dept made editable; "Total" relabelled "Amount"; Debit/Credit read-only; Print hidden until S5-M7; the radio buttons checked by code.
>   - **A cleared line leaves the amount blank.** With "0.00" in it, typing "1120" landed before the zeros and gave 11,200.00 (found while driving it).
> - **Wire** (task 5): 4044 (`ID_APBATCH`, the toolbar's AP voucher button too) in `ManilaGL.cpp`, out of `kApCommands`. `/checkres` loads 1042; `StressAp` opens it.
> - **Tests** (task 6), 3 cases. `gl_tests`: 128 cases.
>   - `AddDays`: month and year ends, a leap year, invalid dates.
>   - The spec's 1,120.00 invoice, line by line: a blank description gets the account's name. A second line, with its analysis codes in their own columns (D13). Replacing line 1, deleting it, and changing the rates all recompute. Posted vouchers are refused.
>   - `DeleteApVoucher` removes that open voucher only.
> - **Checked:** Debug and Release `/W4 /WX` clean; `gl_tests` passes in both; `/checkres` 0; `/stress` 0 (69 s). Driven in the app (a scratch copy with the fixture), checked in the database and the Distribution List:
>   - V001 and Enter: the vendor's accounts, 12 / 1, terms 30, Company, the due date 30 days on.
>   - 1,120.00 on 501000: Dr 1,000.00, VAT 120.00, Cr W/H 10.00, Cr AP 1,110.00.
>   - A second line, 560.00 on 751000: 1,680.00 payable, VAT 180.00, W/H 15.00, AP 1,665.00.
>   - Line 1 picked and changed to 2,240.00: replaced; 2,800.00 each way.
>   - The VAT line picked: the message. Line 1 deleted: recomputed from 560.00 alone (AP 555.00).
>   - V001 INV-1001 refused: "on invoice voucher 1".
>   - A new V003 voucher, then New and No: removed; voucher 7 untouched.
>
>   **How it was driven:** messages posted to the app's own windows. Keystrokes sent to the foreground window had reached another program when gl.exe wasn't in front, and are no longer used.

**Goal:** purchase invoices are entered with their VAT, W/H and AP lines computed.

### Tasks

1. **Read** `TAPInvoiceEntry` (`ap_btche.cpp`) and its lists, and dialog 1042. Note which controls the code reads that aren't in the template (§2.6 #11).
2. **`prune_rc.py`:** add 1042; regenerate.
3. **`gl/port/ApInvoiceEntry.{h,cpp}`** on Spec #2's Voucher Entry pattern (spec §4.2):
   - **The header:** vendor (Enter shows it and fills the rates and accounts), invoice number, dates (the due date is invoice date + terms, editable), period, rates, the AP and VAT accounts.
   - **Expense lines:** account, dept, description, med rep, product, doctor, amount. Each add, change or delete recomputes the computed lines (D4) through `Ap.h`.
   - **The Distribution List** beside it (Spec #2's `LineList`): double-click edits an expense line; computed lines aren't editable.
   - **Duplicates** refused (D6). **The analysis codes** go to their own columns (D13).
   - **New and Close** ask to keep a voucher made in this session; No removes only that voucher (D5).
   - **The mode** is `GL.INI [WITHHOLDING] TAX`, default "F" (spec §4.1).
4. **The vendor popup** from entry, if the template has one, uses `VATR` like Vendors (D12).
5. **Wire** 4044; `/checkres`, `/stress`.
6. **Tests:** `SaveApLine` / `DeleteApLine` with recompute; D5's removal; the due date.

### Exit criteria

- On the fixture, a V001 invoice of 1,120.00 shows Dr expense 1,000.00, Dr VAT 120.00, Cr W/H 10.00, Cr AP 1,110.00.
- A second line recomputes, and the voucher still balances; the same invoice number is refused.

---

## Milestone 3 — Post, Cancel, Recover; the transfer (1028)

> **As built (2026-11-13; built and run on Windows 11, VS 18, v145).**
> - **Read** (task 1): spec §2.4 and §2.6 #3, #8 hold. Further findings:
>   - `TAPPostBatch::HandleListBox` (a double-click) wrote `GL_DIST` rows and no `AP_JSUM` rows, unlike Ok.
>   - The original's Cancel wrote string 1042 over the reference and descriptions, and zeroed the payable and lines.
>   - `APUpdate` dated the GL voucher today and read rows with mode blank or "1".
>   - Dialog 1030's boxes are hand-toggled `BS_CHECKBOX`.
>   - **Settled:** where the transfer records its GL voucher. A port table, **`ap_xfer`** (the `ap_jsum` row's record number, the GL voucher, the period), as Spec #4's `gl_fadep`. A transferred row is never deleted, and `pack()` doesn't renumber, so the link holds. The design's §4.4 says so.
> - **`Ap.h`** (task 2), each in one transaction:
>   - **`PostApVouchers(kind, number)`:** an invoice gets its `ap_dist` row ("O", D11); a cash voucher gets none (D10). Both get `ap_jsum` rows (TYPE "AP" / "CA", the cash cheque, the period as " 7", the invoice date), then "0". It refuses a voucher out of balance by a cent, with no lines, or with an account not in the chart. Cash posting is in place for S5-M4.
>   - **`CancelApVoucher`:** "X", lines kept (D3).
>   - **`RecoverApVoucher` (D7):** refuses a voucher that is transferred, or whose open item is P/V/C. Otherwise it removes its own `ap_dist` and `ap_jsum` rows and sets "1".
>   - **`TransferAp`:**
>     - one GL voucher per period through Spec #2's `SaveLine`, netted by account and dept, dated the period's end (D2);
>     - closed or undated periods are reported and left waiting;
>     - rows go to "2" with `ap_xfer` links, and each voucher of either kind whose rows are all transferred goes to "2" (D10).
>   - **`UndoApTransfer`**, called by Spec #2's `CancelVoucher` after Spec #4's `UndoRun`: one call in `Vouchers.cpp`.
>   - `EnsureApTables` creates `ap_xfer`.
> - **Screens** (tasks 3–4):
>   - **`gl/port/ApPosting.{h,cpp}`:** Spec #2's voucher-list shape, for either kind. Post on 1047 (OK posts all, a double-click one), Cancel on 1043, and Recover on 1015 (refusing a transferred or paid voucher, saying what to do).
>   - **`gl/port/ApTransfer.{h,cpp}`** on 1030: the AP box checked, the AR box disabled (Spec #6), the waiting rows counted. OK reports each period's GL voucher, or why a period was left.
>   - `prune_rc.py`: 1047 and 1030 added. A menu edit puts **Recover Purchase Voucher** (`CM_AP_RECOVER`, 20504) after Recover General Ledger Voucher, behind its access.
> - **Wire** (task 5): 1048, 3064, `CM_AP_RECOVER` and 1028; 1048 and 3064 left `kApCommands`. `/checkres` loads 1047 and 1030; `StressAp` opens all four.
> - **Fixture:** regenerated for the new menu row (`ACCESSM` 108 rows); every other table identical.
> - **Tests** (task 6), 5 cases. `gl_tests`: 133 cases.
>   - Posting invoice 1 (the open item, 4 journal rows), all the rest, and the cash voucher.
>   - The refusals, with nothing written.
>   - Cancel; Recover (invoice 3 recovered and posted again; paid invoice 6 refused).
>   - The transfer: GL voucher 104, 7 lines, 6,536.00 each way, 11 rows, closed period 5 left waiting, the vouchers "2" with cash included. Cancelling 101 leaves AP alone; cancelling 104 puts everything back.
>   - Posting the transfer: 221002 moves by 4,900.00 (the unpaid invoice 3), 731000-20 by 5,000.00.
> - **Checked:** Debug and Release `/W4 /WX` clean; `gl_tests` passes in both; `/checkres` 0; `/stress` 0 (69 s). Driven in the app (a scratch copy, window messages only), checked in the database:
>   - cancel invoice 2 ("X");
>   - post all ("1 voucher(s) posted");
>   - Recover refuses paid invoice 6, then recovers invoice 1;
>   - Update AR and AP gives "Period 7: GL voucher 104, 5 line(s), 6,200.00 each way";
>   - Cancel Voucher on GL voucher 104 puts the 7 rows back to waiting and invoices 3 and 6 back to "0".
> - **Q3** is still the spec's assumption (posted invoices start open).

**Goal:** invoices post to AP's ledger, and AP's journal reaches the General Ledger.

### Tasks

1. **Read** `TAPPostBatch`, `TAPCancelBatch` (`ap_post.cpp`, `ap_cancb.cpp`), `TGLAPUpdate` (`gl_apupd.cpp`), dialogs 1047, 1043 and 1030, and Spec #2's `VoucherPosting.cpp`.
   - **Settle** where the transfer records the GL voucher of each `AP_JSUM` row (spec §4.4: `AP_JSUM` has no field for it). The likely answer is a port table, as Spec #4's `gl_fadep`. Record the answer in the spec before going on.
2. **`Ap.h`**, each in one transaction:
   - `PostApVoucher` / `PostAllAp` (spec §4.3): the `AP_DIST` row ("O", D11), the `AP_JSUM` rows (`TYPE` "AP", the voucher's period and invoice date), `MODE` "0". An unbalanced voucher is refused.
   - `CancelApVoucher`: `MODE` "X", lines kept.
   - `RecoverApVoucher` (D7): posted, untransferred, unpaid; removes exactly its own `AP_DIST` and `AP_JSUM` rows.
   - `TransferAp` (D2): one GL voucher per period (Spec #2 `SaveLine`, mode "1"), netted by account and dept, dated the period's end, described by string 1049. A closed period is reported and left. The rows and vouchers get "2".
   - **The Cancel hook:** Spec #2's `CancelVoucher` sets a transfer voucher's rows back to "1", as Spec #4's `UndoRun`. One call in `Vouchers.cpp`.
3. **Screens** on Spec #2's Post / Cancel list dialogs: Post (1048), Cancel (3064), and Recover (a new menu item, Transactions > Recover Voucher > Purchase Vouchers, by a `prune_rc.py` edit).
4. **`gl/port/ApTransfer.{h,cpp}`** on dialog 1030 (Maintenance > Update AR and AP): the AP half; the AR half stays a stub for Spec #6.
5. **Wire** 1048, 3064, the Recover item and 1028; `/checkres`, `/stress`.
6. **Tests:**
   - posting the fixture's open voucher (the `AP_DIST` row and journal), and a refused unbalanced one;
   - cancelling; recovering, and refusing a transferred or paid voucher;
   - the transfer: one GL voucher per period, its lines and totals, a closed period left, the rows marked;
   - cancelling the transfer's GL voucher puts the rows back;
   - posting it with Spec #2, then 221002 carrying the unpaid invoices.

### Exit criteria

- On the fixture: post, cancel and recover driven in the app; the transfer's GL voucher posts with Spec #2's Post Voucher.
- Q3 still assumed (open), or answered.

---

## Milestone 4 — Cash vouchers (1044)

> **As built (2026-11-13; built and run on Windows 11, VS 18, v145).**
> - **Read** (task 1): spec §5 holds. Further findings:
>   - `TAPCashEntry` is 1,573 lines in `ap_cshbt.cpp`, plus 635 in `ap_cshrb.cpp`, mostly one slip builder per tax case (`CreateVatSlip`, `CreateRebateSlip`, `CreateVat_RebateSlip`, …).
>   - **Dialog 1044 uses 1042's control IDs.** In place of the due date and terms it has a bank combo box (1031, filling 1087 / 1086) and the cheque number (6070).
>   - Its vendor field is read-only (picked from the list only), and the invoice number's mask allows 8 characters, though `AP_CASH.INVOICENO` holds 10.
>   - `gl_new.rc` has three copies of 1044; the first has no closing brace and `prune_rc.py` drops it.
> - **`prune_rc.py`** (task 2): 1044 added. `/checkres` fits it as it is.
> - **The screen** (task 3) is S5-M2's, given the voucher's kind, rather than a copy: `RunApCashEntry` in `ApInvoiceEntry.{h,cpp}` (D10).
>   - The bank combo box lists the `GL_SETAT` "CA" accounts, and picking one fills the credit account.
>   - A cash voucher needs a bank. The cheque and the VAT account go on its header; its due date is the invoice date.
>   - The vendor field is typed, and the vendor's AP account doesn't overwrite the bank.
>   - Every voucher operation, the lists and the navigation use the kind.
> - **`Ap.h`** (task 4): nothing new. S5-M3's `PostApVouchers` already posts cash (TYPE "CA" with the cheque, no `ap_dist` row), `CancelApVoucher` cancels it, and `TransferAp` marks it.
> - **Wire** (task 5): 1079 (`ID_APCASH`, and the toolbar's cash button), 1088 and 1085, out of `kApCommands`. `/checkres` loads 1044; `StressAp` opens it and the cash Post and Cancel.
> - **Tests** (task 6), 1 case. `gl_tests`: 134 cases.
>   - Entry with a bank, through `SaveExpenseLine`: the credit is the bank, 555.00; the cheque, the VAT account and a 10-character invoice number are kept; D6 across kinds.
>   - Post (4 "CA" rows with the cheque, no open item), the transfer marking it "2", and cancel.
> - **Checked:** Debug and Release `/W4 /WX` clean; `gl_tests` passes in both; `/checkres` 0; `/stress` 0 (74 s). Driven in the app (window messages only), checked in the database:
>   - V001, the CHCK bank picked (102101), cheque 000124, 560.00 on 501000: Dr 500.00, VAT 60.00, Cr W/H 5.00, Cr bank 555.00.
>   - Post Voucher > Cash Vouchers: "2 voucher(s) posted", each with its "CA" rows and cheque.
>   - The live window shows every label: `/screenshots` and the visible statics agree. An earlier capture missing four labels caught it mid-paint.

**Goal:** invoices paid at entry are entered, posted and transferred.

### Tasks

1. **Read** `TAPCashEntry` (`ap_cashp.cpp`, `ap_cshbt.cpp`, `ap_cshrb.cpp`) and dialog 1044.
2. **`prune_rc.py`:** add 1044; regenerate.
3. **`gl/port/ApCashEntry.{h,cpp}`:** as M2, with a bank (the `GL_SETAT` `CA` accounts) and a cheque number in place of the AP account; the credit line is the bank (spec §5, D10).
4. **`Ap.h`:** posting writes `AP_JSUM` (`TYPE` "CA", with the cheque number) and no `AP_DIST` row; cancel as invoices; the transfer marks cash vouchers "2" (D10).
5. **Wire** 1079, 1088, 1085; `/checkres`, `/stress`.
6. **Tests:** a cash voucher's lines; post; cancel; transferred with the invoices.

### Exit criteria

- On the fixture: the open cash voucher posts, and the transfer carries it.

---

## Milestone 5 — Create Cheques (1045)

> **As built (2026-11-13; built and run on Windows 11, VS 18, v145).**
> - **Read** (task 1): spec §2.5 and §6.1 hold. Further findings:
>   - `TAPCreatePayment::Ok` paid the full amount, W/H included, and wrote no cheque; Set Cheque Numbers (8074) later built `AP_REQST`.
>   - `CreateGLEntry` dated the payment's rows by the invoice, in the invoice's period.
>   - Force was skipped unless due; an empty list gave AR's message (4074).
>   - Dialog 1045 (only in `gl.rc`) holds a **`tbStatusBar`** (5013), a class win32fx didn't register, so the dialog couldn't load. Its status pane also sits partly below the dialog.
>   - Dialog 1066's two auto check boxes could both be ticked.
> - **win32fx:** `tbStatusBar` is registered (`src/controls/StatusPane.cpp`): a one-line pane drawing its text in a sunken border. It is in `genproj.py`'s win32fx list.
> - **`prune_rc.py`** (task 2): 1045 (from `gl.rc`) and 1066 added. "Payment Date" didn't fit its 33 units, with the field right after it: `PATCHES` makes it "Pay Date", like the "Due Date" above it.
> - **`Ap.h`** (task 3):
>   - `AmountOwed` (amount less the W/H withheld, less what's paid);
>   - `PaymentItems` (H/O/F due by a date, plus F whatever the date; or all);
>   - `SetPaymentControl` (H/O/F, refused once paid, 4010);
>   - `NextChequeNumber`, `OpenPeriodOf`;
>   - **`CreateCheques`** in one transaction (D8, D9): O items due and F items, one cheque per vendor, numbered from the starting number, on one bank. It writes `ap_reqst`; each item gets P, the cheque, the amount owed as paid, the payment date and the bank; the journal gets Dr AP / Cr bank, TYPE "AP", dated the payment date, in its open period, with the cheque number. It refuses a bad bank (4007), a payment date outside an open period, or a number already used.
> - **`gl/port/ApPayments.{h,cpp}`** (task 4):
>   - **The list** is filled at once (due by today) with Vendor, Ctl, Code, Invoice, Due Date, Amount, VAT, W/H and Owed. Set Date refreshes it and View All shows every item.
>   - **Double-click** opens Hold or Force: one box or the other; Accept sets H, F or O.
>   - **Create** asks (4009), reports each cheque, refreshes, and moves the starting number on.
>   - **The status pane** says what happened and is fitted to the bottom of the client area. Print (the cheque list) is hidden until S5-M7.
> - **Wire** (task 5): 1057 (`ID_GLSLIP`), out of `kApCommands`. `/checkres` loads 1045 and 1066; `StressAp` opens Create Cheques.
> - **Tests** (task 6), 1 case. `gl_tests`: 135 cases.
>   - What's due by a date, and View All.
>   - Force and Hold, with an item in a payment refused.
>   - The four refusals, with nothing written.
>   - Cheque 5002 for V001's due and forced invoices (1,887.00), with the items, the cheque and the journal checked.
>   - Recover refusing the paid invoice; nothing more due; the released invoice paid on the next run with its own cheque.
> - **Checked:** Debug and Release `/W4 /WX` clean; `gl_tests` passes in both; `/checkres` 0; `/stress` 0 (74 s). Driven in the app (window messages only), checked in the database:
>   - post the open invoices;
>   - Create Cheques: "3 invoice(s) due … 6,787.00 owed"; invoice 3 held through Hold or Force;
>   - the CHCK bank picked; Create gives "Cheque 5002 V001 1,887.00 (2 invoice(s)) … period 8";
>   - items 1 and 2 P on 5002 / 102101, item 3 H, and 4 journal rows dated the payment;
>   - View All shows every state, and the next number is 5003.
>
>   Screenshots found the Due Date column and the status pane clipped; both are fixed above.
> - **Q4** is still the spec's assumption: one cheque per vendor (and bank).

**Goal:** due invoices are paid by cheques, with the payment's GL entry.

### Tasks

1. **Read** `TAPCreatePayment`, `TAPHoldForce` (`ap_paymt.cpp`, `ap_payrq.cpp`), dialogs 1045 (from `gl.rc`) and 1066.
2. **`prune_rc.py`:** add 1045 (from `gl.rc`, as 1061 in Spec #2) and 1066.
3. **`Ap.h`:**
   - `DueItems(code, date)`: the O, F and H items due by the date, with the amount owed (amount − W/H);
   - `SetPaymentControl(code, item, H|O|F)`;
   - `CreateCheques(code, date, bank, firstNumber)` (spec §6.1, D8, D9), in one transaction: one cheque per vendor and bank, the items marked "P" with the cheque, and the payment's `AP_JSUM` rows (Dr AP / Cr bank, dated the payment date, in its period, with the cheque number).
4. **`gl/port/ApPayments.{h,cpp}`:** the selection list with Hold / Force / Open, the date, the bank, the first cheque number (default: the last + 1), Create; then the cheques made.
5. **Wire** 1057; `/checkres`, `/stress`.
6. **Tests:** what's due by a date; Hold keeps an item out and Force pays one not due; one cheque per vendor and bank; the journal rows and their period.

### Exit criteria

- On the fixture: cheques for the due invoices driven in the app.
- Q4 still assumed (per vendor and bank), or answered.

---

## Milestone 6 — Cheques, Bank Reconciliation, Void, Purge

> **As built (2026-11-13; built and run on Windows 11, VS 18, v145).**
> - **Read** (task 1): spec §2.5 holds. Further findings:
>   - **`AP_REQST.CLEARED` "0" means cleared** (`Tapreqst::bCleared`), and `AddCheque` writes "1" for a new cheque. So the deployed cheques, all "1", aren't cleared; the design said they were, and is corrected. `LoadCheques` reads "0" as cleared, and `CreateCheques` writes "1".
>   - Set Cheque Numbers (`TPrintCheques`, 1157) zapped `AP_REQST`, renumbered every "P" item, and marked them all "V" on Verify.
>   - Bank Reconciliation (`TAPPayment`, 1046) listed only "P" items; its 1051 cleared one and closed every table.
>   - Purge (`TAPPurgeBatch`, 1048) deleted the "P" items.
> - **`Ap.h`** (task 2):
>   - `ChequeState` (P / V / C, from the cheque's items) and `LoadChequesIn` (cheques in a state, with payee and stub);
>   - `AmountInWords`;
>   - `ConfirmPrinted(from, to)`;
>   - `RenumberCheque` (an unconfirmed cheque, to an unused number: the cheque, its items and its journal rows);
>   - `ClearCheque` (printed only: "C" on its items, CLEARED "0", the date in the reference);
>   - `VoidCheque`, one transaction: the items back to "O"; waiting payment rows deleted; transferred ones reversed with new rows dated the void, in its open period; the cheque row deleted;
>   - `PurgeCandidates` / `PurgePaid` (cleared items paid before a date, and their emptied cheques).
> - **The cheque** (task 3): `gl/port/reports/ApReports.{h,cpp}` `ApCheques`, on Spec #3's engine.
>   - One slip a page (the engine's slip mode starts each on a new page): number, date, payee and address, amount, bank.
>   - The amount in words on a full-width line under it.
>   - The stub (voucher, invoice, date, reference, amount, W/H, paid) and its total.
>   - `RunApCheques` runs it for 4091, which left `kApReports`; it joins `/stress`'s report builders.
> - **Screens** (task 4), in `ApPayments.{h,cpp}`:
>   - **Confirm Printed Cheques** (8074) is a dialog of the port's own, `IDD_AP_CONFIRM` in `port.rc`, since 1157 has room for one number. It lists the cheques waiting, confirms a range (defaulting to all), and renumbers the selected cheque.
>   - **Bank Reconciliation** (1046) lists the cheques not cleared, Made or Printed. Double-click opens 1051, "Clear or Void Cheque N": Clear, Void (a button added in code), Close, and the date.
>   - **Purge Paid Payables** (1048): a date control is added in code over 3031, and its label becomes "Cleared and paid before:". The list shows what goes; Purge asks, then deletes.
>   - `prune_rc.py`: 1046, 1051 and 1048 added. A menu edit renames 8074 "Confirm Printed Cheques…" (D9), and another puts "Purge Paid Payables…" (1066) on Maintenance after Year End.
> - **Wire** (task 5): 4091, 8074, 6085 and 1066; out of `kApCommands` / `kApReports`. `/checkres` loads 1046, 1051, 1048 and `IDD_AP_CONFIRM`; `StressAp` opens the three screens.
> - **Fixture:** regenerated for the menu (`ACCESSM` 109 rows); every other table identical.
> - **Tests** (task 6), 2 cases. `gl_tests`: 137 cases.
>   - `AmountInWords`.
>   - The whole cycle: the fixture's cheque printed; cheque 5002 made, in the report (slip, words, stub); Renumber refused for a used number, a printed cheque, a missing one, then 5002 → 5010 everywhere.
>   - Clear refused until confirmed, then cleared (CLEARED "0", the date), and Void refused for it.
>   - Void of 5001 before the transfer (2 rows removed, invoice 6 open again).
>   - Void of 5011 after the transfer (refused for a closed period, then 2 rows reversed, dated the void, in period 7).
>   - Purge: only 5010's two items and the cheque.
> - **Checked:** Debug and Release `/W4 /WX` clean; `gl_tests` passes in both; `/checkres` 0; `/stress` 0 (75 s). Driven in the app (window messages only), checked in the database:
>   - post, then Create Cheques (5002 V001 1,887.00; 5003 V002 4,900.00);
>   - Print Cheques' preview, 2 pages;
>   - Confirm 5002–5003 ("2 cheque(s) confirmed printed");
>   - Bank Reconciliation: 5002 cleared; 5001 voided (invoice 6 open, 2 rows removed);
>   - Purge before 12/31/2026: "2 invoice(s) and 1 cheque(s) purged", the uncleared cheques kept.
>
>   Screenshots found a blank third page (an explicit page break as well as the slip's), the amount in words clipped in the slip, and clipped columns; all fixed.
> - **Q6** is still the spec's assumption: plain paper.

**Goal:** the payment cycle closes.

### Tasks

1. **Read** `TPrintCheques`, `TAPPayment`, `TAPSetChequeNumber`, `TAPPurgeBatch` (`ap_reqst.cpp`, `ap_paymt.cpp`, `ap_purge.cpp`), dialogs 1157, 1046, 1051 and 1048, and `AP_CHEQ.RPT`.
2. **`Ap.h`:**
   - `ConfirmPrinted(code, from, to)` ("V") and `RenumberCheque(code, old, new)` (D9);
   - `ClearCheque(code, cheque, date)` ("C");
   - `VoidCheque` (spec §6.3): reverses the payment's journal (or writes reversing rows if transferred) and returns the items to "O";
   - `PurgePaid(code, before)`: cleared items and their cheques only (D8).
3. **The cheque** (4091) on Spec #3's engine: payee, amount in figures and words, date, and a stub with the invoices (spec §6.2; Q6 assumed plain paper).
4. **Screens** in `ApPayments`: Confirm Printed (8074, replacing Set Cheque Numbers), Bank Reconciliation (6085), Purge (1066, put on Maintenance by a `prune_rc.py` edit).
5. **Wire** 4091, 8074, 6085, 1066; `/checkres`, `/stress`.
6. **Tests:** confirm and renumber; clear; void (journal reversed, items open again, before and after a transfer); purge leaves everything not cleared; the amount in words.

### Exit criteria

- On the fixture: print a cheque to PDF, confirm it, clear one, void one, purge.

---

## Milestone 7 — Reports, report list, clean-up

> **As built (2026-11-13; built and run on Windows 11, VS 18, v145).**
> - **Reports** (task 2): `gl/port/reports/ApReports.{h,cpp}`, 22 builders over `Ap.h`'s records, in cents, plus M6's cheques.
>   - Shared layouts: vouchers with their lines (1078, 1081, 8077, 8078); the journal by account (8068 a line each; 8069, 8075, 8076 in detail); the aged layout (1062, 1064), its buckets from `DaysBetween` (new in `Ap.h`) as at the report's date.
>   - 1063 (AP_AGEDC) runs 1062. The tax reports 8067 and 8071 read the AP vouchers' own VAT and W/H lines, per invoice; 8072 reads `gl_btchd` on `GL_SETAT` VATR's account.
>   - Landscape: 1060, 1062, 1064, 1077, 4092, 8072, 8073, and the journal's detail.
> - **Wire** (task 3):
>   - `RunApReport(frame, id)` (`RunReport.{h,cpp}`) runs any AP report from a table of command and builder; `ManilaGL::OnCommand` tries it before the "not ported" message. `RunApCheques` went into it.
>   - `kApReports` is gone; 1082 and 1990 (adjustments) aren't ported, as the spec says.
>   - The report list has the 22 AP reports.
>   - `/stress` opens every AP report from the same table.
>   - Menu labels (`prune_rc.py`): `MENU_EDITS` `remove` and `rename` can name the item's text when two items share an ID. 4091's "Customer Sales Statements" copy is removed; 4092 becomes "Cheque Payment Detail"; 1060's "Vendor Summary" copy is removed and the other is "Vendor List"; 8069 is "AP Journal Detail", with 8068, 1049 and 1065 after it; the other AP report labels name their reports (and "Reqister" is spelt right).
>   - Found: 8075 and 8076 ran the cash and AP journals as their labels said; the spec said otherwise and is corrected.
> - **Fixture:** regenerated for the menu (`ACCESSM` 110 rows); every other table identical.
> - **Tests** (task 4), 3 cases. `gl_tests`: 140 cases.
>   - `DaysBetween` and `ApAsAtDate`.
>   - Every report on the fixture: a cell per column; the open vouchers and their balance; Voucher Status's total; the posting journal without AP lines; three vendors; what's owed (4,900.00) the same in 1061, 1058, 1077, 1062 and 1064, the aged buckets adding to it and moving with the date; the payment detail; the cash cheque summary; the VAT and W/H lines; the journal's totals; after posting, the empty pre-posting reports and 8075's cash rows.
>   - **The columns fit** (task 6): each report laid out on A4 with the engine's own fonts measured by GDI (`GdiMeasure`), before and after posting: no text is cut and no number prints "###". It found two (Voucher Status's total label; 8073's vendor and reference), both fixed (8073 is landscape). This replaces printing each to PDF by hand: the preview and the printer draw the same pages.
> - **Checked:** Debug and Release `/W4 /WX` clean; `gl_tests` passes in both; `/checkres` 0 in both; `/stress` 0. In a scratch copy of the app, each of the 22 menu commands (posted to the frame) opened its preview with the right title.
> - **Not checked:** the reports on the deployed company's data; printing to paper.

**Goal:** the AP reports print; the module is complete in the menus, the checks and the README.

### Tasks

1. **Read** each `.RPT` of spec §7 (titles, columns, grouping, selection, footers) with `tools/rptdump.py`.
2. **`gl/port/reports/ApReports.{h,cpp}`**, portable: the reports of spec §7 (D14), accounts from `GL_SETAT`, landscape where the columns need it.
3. **Wire:**
   - the reports leave `kApReports` (Spec #3 S3-M7), which then goes;
   - they join `ReportList`'s entries;
   - the mislabelled menu items are corrected by `prune_rc.py` edits (spec §7);
   - `/stress` opens every preview.
4. **Tests:** each report's rows and totals on the fixture; the aged report's buckets add to the open total.
5. **README:** an Accounts Payable section (the screens, the cycle: enter, post, transfer, pay, print, reconcile, purge; the reports).
6. **Print** each to PDF from the fixture and check the column widths, as in Specs #3 and #4.

### Exit criteria

- Debug and Release `/W4 /WX` clean.
- Every suite, `/checkres`, `/stress` and the self-test pass.
- The PDFs checked.

---

## Milestone 8 — Acceptance

1. **`gl/docs/superpowers/specs/2026-11-13-spec5-acceptance.md`**, in the Spec #4 sheet's shape, with spec §10's checks, the fixture's expected figures and the `sqlite3` queries that produce them.
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
  - Don't change the original tables' layouts: anything new goes in a table of the port's own, as Spec #4's `gl_fadep`.
  - Don't edit `gl/res/gl.rc` by hand: change `prune_rc.py` or `port.rc`.
  - Don't run anything against the real company's AP tables until Q5 is answered.
  - Don't let AR's journal (`AR_JSUM`) be touched by AP code (D15).

## Risks specific to the plan

| # | Severity | Risk | Mitigation |
|---|---|---|---|
| P1 | Medium | The finance team's answers to Q3–Q7 arrive after their milestones. | Each is one function in `Ap.cpp` with its test; each milestone's exit criteria record what is still assumed. |
| P2 | Medium | The transfer's Cancel hook changes Spec #2 behaviour. | One call, inside Cancel's transaction; a test cancels an ordinary voucher; the Spec #2 sheet's Cancel check is re-run in M8. |
| P3 | Medium | The templates lack controls the code reads (§2.6 #11), or don't fit Segoe UI. | Each screen's read task lists them; missing controls are added in code (as S4-M4) and `/checkres` checks the fit. |
| P4 | Low | The fixture's AP data shifts a Spec #1–#4 figure. | Its journal is untransferred; M0 task 3 compares the sheets' queries before and after. |

## How to use this plan

M0 → M1 → M2 → M3, then M4 beside M5 → M6, then M7 and M8. If the plan and the spec conflict, stop, edit the spec, recheck the plan, then continue.
