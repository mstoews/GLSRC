# Spec #6 — OWL-to-Win32 SDK Port: Accounts Receivable

**Status:** Draft for review (2026-11-13). Q1–Q3 answered (§13). The plan: `plans/2026-11-13-owl-to-win32-sdk-port-spec6-plan.md`.
**Date:** 2026-11-13
**Scope:** Sixth spec of the migration: the Accounts Receivable module.
- customers;
- sales invoice vouchers, with their posting, cancelling and recovering;
- receipts: a customer's payment against an invoice, in full, in part, or with an early-payment discount;
- the purge of fully received invoices;
- the AR half of the transfer to the General Ledger;
- the AR reports, on Spec #3's engine.

As with Accounts Payable (Spec #5), the original's AR cycle doesn't work end to end (§2.6), and the company never ran it: its customer master is real, but every AR transaction table is empty (§2.7). So the spec fixes what it ports. The user chose all of it, receipts with partials and discounts, and no withholding (§13, Q1–Q3).

**Depends on:** Specs #1–#5. Spec #2's GL vouchers carry AR's GL entries; Spec #3's `report` library prints the reports; Spec #5's AP port is the pattern for nearly every screen, the portable core, the transfer and the reports. Everything they fixed still holds: the build, the error idioms, the Unicode strategy, the translation pattern and the per-port checklist.

**Numbering.** Seam 6 of Spec #1 §9. The "Update AR and AP" transfer (1028) is shared: Spec #5 ported its AP half, and this spec adds the AR half.

**Sources.** `gl/src/ar_*.cpp` (about 6,150 lines), `gl/src/gl_apupd.cpp` (`TGLAPUpdate::ARUpdate`), `gl/src/ap_repts.cpp` (the AR report handlers), the headers `gl/header/ar*.h`, the table classes in `include/db_ar*.h` and `db_cust.h`, dialogs and strings in `gl/gl_new.rc` (1062 only in `gl/gl.rc`), the deployed `dbf/AR_*` and the `.RPT` files. Two surveys of the code and data (2026-11-13) are summarized in §2. File and line references are to the original.

---

## 1. Goals & Non-Goals

### 1.1 Goals (Spec #6 deliverables)

1. **Customers** (4059, dialog 1052, with its options dialog 1063).
2. **Sales invoice vouchers** (5045, dialog 1053): entry by product line (quantity × price, or an amount), VAT included in the price (D4); **Post** (4049), **Cancel** (4053), and **Recover** of a posted voucher not yet transferred or received (D7).
3. **Receipts** (Clear Receipts, 4054, dialogs 1156 and 1155): record what a customer paid against an invoice, in full, in part, or in full with an early-payment discount; the receipt's GL entry is written at once (D8, D9). A receipt entered by mistake can be reversed (D10).
4. **Purge** of fully received invoices (4062, dialog 1062; D11).
5. **The AR → GL transfer** (1028, AR half): AR's journal becomes GL vouchers, one per period, which post the usual way (D2).
6. **Reports:** the AR reports that work or can be made to, rebuilt on AR's own tables (D14), on Spec #3's engine, in the report list.
7. **Fixture and tests** as Specs #2–#5: a portable core, `gl/port/Ar.{h,cpp}`, under `gl_tests`.

### 1.2 Non-Goals

- **Withholding tax on receipts** (Q3: as the original, none is booked). The receipt dialog's W/H box is hidden. Adding it later touches the receipt function only.
- **Credit notes, returns, debit memos and adjustments.** The original has none that work (`ADJUSTMENT` is only added to by a duplicate post, `ADJ`/`ADJAMT` are never written). A return is entered as a cancelled or recovered invoice, or as a GL voucher.
- **Printing the sales invoice itself** as a customer document. The original's `AR_SLIPS` print is commented out; the port prints the voucher on the reports (Q5).
- **Credit limits** (`AR_CUST.CREDIT` is never read or written; Q7), finance charges, dunning letters, foreign currency, sales orders.
- **`AR_REQST`, `AR_CTRL`, `AR_SETUP`, `AR_SYS`, `AR_TAG`.** No working code uses them (§2.1). They stay in the database, untouched.
- **The standalone `ar/` application** in the repo. It isn't part of `GL.EXE`.

---

## 2. Accounts Receivable in the original

### 2.0 The sources don't match any build

- **The AR sources can't compile as checked in.** The record classes they use (`Tarbtch`, `Tarbtchd`, `Tardist`) aren't defined anywhere `GL.EXE` builds from. `include/db_arbtc.h` and `db_ardst.h` declare AP-named classes on the `ar_*` tables. `db_arjsm.h`, included by `ar_payrq.cpp`, doesn't exist, and `db_cust.h`'s `Tarcust` lacks most of the fields the code uses.
- **Four files are binary junk, not source:** `gl_arupd.cpp`, `arp.cpp`, `ar_glslp.cpp`, `ar_index.cpp`.
- **The deployed `eiwa/GL.EXE` holds AR code the repo doesn't** (strings such as `TYPE = 'CH' .AND. MODE = '2'` and `gl_jsumh`).

So `gl/src` is a work-in-progress revision. This spec ports **what the source means**, fixed, not a build that can't be reproduced. That is the same footing as Spec #5, made firmer by §2.7: nothing was ever recorded.

### 2.1 Tables

| Table | Rows (deployed) | Role |
|---|---|---|
| `AR_CUST` | 142 | Customers, in `AP_VEND`'s layout (`VENDORNO`, `VDSHORT`, `VDNAME`, addresses), plus ship-to `S_*` fields. **The accounts are named the other way round from AP:** `ACCOUNT`/`DEPART` is the customer's **AR control account**, `AR_ACCT`/`AR_DEPT` its **sales** account. `VAT_ACCT`/`VAT_DEPT` is output VAT, `VAT` the rate (whole %), `TERMS` days, `VIN` the tax number. `TERMS_DAYS`, `TERMS_PERC`, `TERMS_TXT` and `CREDIT` are never used. **There is no `WITHTAX` field**, though the code reads and writes one. |
| `AR_BTCH` / `AR_BTCHD` | 0 / 0 | Sales invoice voucher header and lines, in AP's layout. The header adds `ORDERDATE`, `VIN`, `UNITS`, `MEMOFIELD`; the lines add `QTY` and `PRICE`. `MODE` is "1" open, "0" **both** posted and cancelled, "2" transferred. `REBATEAMT`, `ADJ`, `ADJAMT` and `TYPE` are never written. |
| `AR_DIST` | 0 | **The open-item ledger:** one row per posted invoice, in `AP_DIST`'s layout plus `REMAINDER`. `APACCOUNT`/`APDEPART` hold the AR control; `ACCOUNT`/`DEPT` are blank at posting and become the bank when a receipt is entered. `PAYMENTCTL`: O open, P received in full, R partly received, D received with a discount, C cleared (journalled). `AMOUNT`, `VATAMT`, `AMOUNTPAID`, `DATEPAID`, `REMAINDER`. |
| `AR_JSUM` | 0 | **AR's journal, staged for the GL**, in `AP_JSUM`'s layout. `TYPE` "AR"; `MODE` "1" waiting, "2" transferred. |
| `AR_REQST`, `AR_CTRL`, `AR_SETUP`, `AR_SYS`, `AR_TAG` | 0, 0, 2, 9, 0 | Unused. `AR_CTRL` is zapped by every Post and never filled. `AR_SETUP`'s two rows still say "Accounts Payable Control". `AR_SYS` lists the nine `AR_*` files. |
| `GL_SETAT` | (GL) | Named accounts: `AR` 111101, `SALE` 410001, `VATP` VAT payable 260007; and the banks (`TYPE` "CA"). |

### 2.2 Customers (4059, dialog 1052; options 1063)

- **Add/Mod** asks first (string 1093). A new customer gets blank accounts.
- **Options (1063)** prefill the sales account from `GL_SETAT SALE`, AR from `AR`, VAT from `VATP` and W/H from `WTXI`; they validate the AR, VAT and sales accounts, and save them with VAT %, terms, contact and VIN.
- **Defects:**
  - SetupWindow's conditions are crossed (`ar_vendo.cpp:117-128`): it tests `ACCOUNT` to fill the sales field and `AR_ACCT` to fill the AR field.
  - It reads and writes a `WITHTAX` field that doesn't exist.
  - Delete deletes and packs with no check for invoices, and says "Vendor code not found".

### 2.3 Sales invoice vouchers (5045, dialog 1053)

**One Add is one product line**, with med rep, product and doctor.
- **The amount:** quantity × price, or the amount field when the price is 0.
- **The split**, with v the customer's VAT rate (editable on the voucher): `SALES = floor(AR ÷ (1 + v/100) · 1000) ÷ 1000`, `VAT = AR − SALES`. VAT is always included in the price. There is no withholding on invoices and no T mode.
- **The lines:** Cr sales (the customer's `AR_ACCT`), one per product line; Cr output VAT (the customer's `VAT_ACCT`), cumulative; Dr AR control (the customer's `ACCOUNT`), cumulative.
- **The header:** `PAYABLE` is the invoice total, `VATAMT` its VAT.
- **The duplicate check** seeks `AR_DIST`, so it only catches **posted** invoices.
- A zero quantity and amount offers "Enter Sample Entry?", which writes a zero sales line.
- The screen is **modeless**, unlike AP's.

### 2.4 Posting, cancelling, the transfer

- **Post (4049, dialog 1061)**, for each open voucher that balances (an exact `double` compare):
  - one `AR_DIST` row, `PAYMENTCTL` "O", `AMOUNT` = `REMAINDER` = the total, with its VAT; if the invoice is already there, `ADJUSTMENT` grows by the total instead;
  - one `AR_JSUM` row per line (`TYPE` "AR", `MODE` "1", the voucher's period and date);
  - `MODE` "0".
- **Cancel (4053)** uses AP's dialog 1043 ("Cancel Accounts Payable Journal"). It sets the reference to "Cancelled", zeroes the total and the lines but not `VATAMT`, and writes **`MODE` "0"**, the same as posted.
- **The transfer (1028, `TGLAPUpdate::ARUpdate`)**, if its AR box is ticked, after AP:
  - zaps `GL_SUM`, nets every waiting `AR_JSUM` row by account and dept into it (with `GL_SUM.TYPE` hard-coded "AP"), and marks the rows "2";
  - writes one GL voucher, "Accounts Receivable GL Update", **in the GL's current period, dated today**, whatever the rows' periods;
  - turns every `AR_BTCH` with `MODE` "0" into "2", so **cancelled vouchers become "transferred"** too.

### 2.5 Receipts (4054, Clear Receipts: dialogs 1156 and 1155)

1. **The list (1156)** shows the invoices not cleared. Double-click one to open **1155**: the bank (from `GL_SETAT` cash names), the date, the amount (default: what's left), and the boxes Discount, Partial and W/H, with the customer's accounts for display.
2. **OK on 1155** marks the invoice:
   - P received in full; R partly received; D received with a discount;
   - a short amount with no box ticked asks: Yes is a discount, No a partial;
   - it writes the bank, `DATEPAID`, `AMOUNTPAID` and `MODE` "0". The W/H box is ignored.
3. **Create** on the list then writes the journal for every marked invoice:
   - **P:** Dr bank / Cr AR control, the amount received; the invoice becomes C.
   - **R:** `REMAINDER` is reduced, `AMOUNTPAID` is zeroed, and **then** the journal is written from `AMOUNTPAID`: **0.00**. The invoice becomes C when nothing is left.
   - **D:** Dr bank (received), Dr sales discount (the shortfall ÷ (1 + v)), Dr VAT (the rest of the shortfall), Cr AR **the original `AMOUNT`**.
   - All rows are dated by the **invoice's** date and period.

### 2.6 What doesn't work

1. **Partial receipts journal 0.00** (`ar_payrq.cpp:628-662`), so the GL never sees them, while `REMAINDER` goes down.
2. **The discount's sales account is a session global** (`szSales`), set by the last 1155 OK: Create in a new session books it to a blank account, or to another customer's.
3. **A discount credits the original `AMOUNT`**, ignoring earlier partials, and the over-payment check uses `AMOUNT`, not what's left.
4. **Receipts are dated by the invoice**, in the invoice's period, not when the money came in.
5. **Out-of-balance vouchers:**
   - an amount entered with price 0 balances on the first line, but later lines compute sales from quantity × price, 0;
   - the AR line takes the sales dept on later lines, the AR dept on the first;
   - Modify and Delete of a line never recompute the VAT or AR lines;
   - 3-decimal arithmetic, 2-decimal storage and an exact compare can block posting.
6. **The period** is overwritten with the GL's current period on every line after the first.
7. **Modes:** "0" means both posted and cancelled, and the transfer makes both "2". Adding a line to a posted or cancelled voucher silently reopens it ("1"), and re-posting doubles `AR_JSUM`.
8. **Recover is dead and would crash:** `RecoverPostedInvoice` uses an object that is never constructed, with AP's tag.
9. **The duplicate check** only sees posted invoices.
10. **"No" to "save the current transaction?"** deletes the whole voucher, as in AP.
11. **`WITHTAX`/`WITHDEPT` don't exist in `AR_CUST`.**
12. **The product and doctor codes are swapped**, as in AP.
13. **The transfer** discards the rows' periods, dates the voucher today, and marks cancelled vouchers transferred.
14. **Copy-paste from AP:** the entry screen checks that `AP_VEND`, `AP_JSUM` and `AP_DIST` exist; Cancel uses AP's dialog; Clear Receipts reports "no invoices" with an AP string (4074); `AR_SETUP` and the posting code still say "AP Control" (`TYPE` "APC").
15. **The receipt dialog differs between the two `.rc` files.** In `gl.rc`, the one `GL.IDE` compiled, 1155 has no amount field (so OK always says the amount can't be zero), and its invoice and W/H IDs aren't the ones the code uses. `gl_new.rc`'s matches the code. The purge dialog 1062 is only in `gl.rc`.
16. **Missing strings:** 4076, 5013, 4113.
17. **Reports** (§7): most of the AR `.RPT` files are AP reports renamed, still reading `AP_JSUM`, `AP_DIST` or `AP_VEND` (the AR journal reports print AP's journal); two (`ar_bstat`, `ar_batch`) aren't in `dbf/`, two more (`ar_recei`, `ar_cust`) exist nowhere; 4061 and 4068 print the same aged report; "Customer Summary" (4058) has no handler; 4074 and 4075's labels are swapped.

### 2.7 The real data

Checked against the deployed company (`dbf/`):

- **Customers are real:** 142 of them, 113 doctors, plus hospitals, pharmacies and distributors (Zuellig Pharma, United Laboratories). Their AR control is 111199 "Trade AR – all others" (89) or 111102 "Trade AR – Luprolex" (53), their sales account 410099 or 410002, output VAT 260007; 140 have VAT 10 % and terms 30, two have neither. All depts are blank. One customer (R0006) has stray bytes in `VAT_DEPT`.
- **Every transaction table is empty:** `AR_BTCH`, `AR_BTCHD`, `AR_DIST`, `AR_JSUM`, with headers dated March 1995, when AP was tested. One memo block was once allocated in `AR_BTCH.DBT`.
- **Nothing AR ever reached the GL:** 111101, 111102, 111199, 410001, 410002, 410099 and 260007 are zero in every period and in the opening and prior-year columns.

**So AR was set up, with its customer master, and never run.** Fixing the cycle changes no history at all.

---

## 3. Screens and port files

| Original | ID | Port file | Notes |
|---|---|---|---|
| `TARVendorAdd`, options | 4059 (1052, 1063) | `gl/port/ArCustomers.{h,cpp}` | D12 |
| `TARBatchEntry` | 5045 (1053) | `gl/port/ArInvoiceEntry.{h,cpp}` | D4–D6; modal, as AP's |
| `TARPostBatch`, `TARCancelBatch` | 4049, 4053 (1061, 1054) | `gl/port/ArPosting.{h,cpp}` | Spec #5's post/cancel list dialogs; AR's own cancel dialog 1054, not AP's 1043; Recover (D7) |
| `TARCreateSlip`, `TARClear` | 4054 (1156, 1155) | `gl/port/ArReceipts.{h,cpp}` | D8–D10 |
| `TARPurgeBatch` | 4062 (1062) | `ArReceipts` | D11 |
| `TGLAPUpdate::ARUpdate` | 1028 (1030) | `gl/port/ApTransfer.{h,cpp}`, extended to both journals | D2 |
| `ap_repts.cpp` (AR handlers) | 4056 … 8137 | `gl/port/reports/ArReports.{h,cpp}` | D14 |
| — | — | `gl/port/Ar.{h,cpp}` | portable: tables, records, the arithmetic, posting, receipts, the transfer; `gl_tests` checks it |

Dialogs come from `gl_new.rc` through `prune_rc.py`; 1062 comes from `gl.rc`, as 1045 did in Spec #5. 1155 is `gl_new.rc`'s (§2.6 #15). Changes a template can't carry are made in code, as before.

**Shared with AP.** Where Spec #5's code already does the job, AR reuses it rather than copying it: the account and bank lookups (`ApAccount`, `LoadApAccounts`' banks), `AddDays`/`DaysBetween`, the post/cancel/recover list dialogs, the transfer's netting and its `*_XFER` link table, and the report layouts. The module's names follow the original's screens; only the shared helpers move to a common header if they need to.

---

## 4. The sales invoice voucher

### 4.1 Amounts (D4)

All amounts are in cents, as Spec #2 D12 and Spec #5 D4.

- **Each product line's amount** `a`: quantity × price, rounded to the cent, or the amount typed when there is no price. Quantity and price are kept in `AR_BTCHD.QTY`/`PRICE`.
- **The split**, with v the voucher's VAT rate (whole %, 0 meaning none), per line: VAT = round(a × v ÷ (100 + v)); sales = a − VAT. Each line's sales credit is on its own account (the customer's sales account by default, editable per line).
- **The voucher's lines are recomputed from its product lines** whenever one is added, changed or deleted:
  - one Cr sales line per product line;
  - one Cr VAT line on the voucher's VAT account, the sum of the lines' VAT;
  - one Dr AR line on the voucher's AR account, Σa.

  The voucher always balances to the cent.
- **The typed amounts are kept** in a port table, **`AR_LINEX`** (voucher, line, role, gross), as Spec #5's `AP_LINEX`: a line's gross can't be recovered from its net, and its role can't be told from its account.

### 4.2 The entry screen (1053)

The Spec #5 invoice entry pattern, **modal** (the original was modeless; every other port screen is modal, and a modeless one could post a voucher while it is open):
- **The voucher:** a header (customer, invoice no., order no. and date, dates, period, VAT rate, accounts), then product lines: quantity, price or amount, account, dept, description, med rep, product, doctor.
- **Lines:** the list beside it shows all the lines. Double-click a product line to edit it; Delete removes one. Both recompute (§4.1).
- **Lookups:** Enter on the customer fills in the name, the accounts, the VAT rate and the terms; the due date becomes the invoice date plus the terms (a typed one is kept).
- **New and Close:** for a voucher created in this session, they ask whether to keep it. No removes only that new voucher (D5).
- **Duplicates:** the same customer and invoice number in any voucher that isn't cancelled is refused (D6).
- **The period** is the one chosen, on every line (§2.6 #6).
- **No "Sample Entry":** a line needs an amount.

### 4.3 Posting, cancelling, recovering (D3, D7)

- **Post** (Transactions > Post Voucher > Sales) lists the open vouchers. Post all, or double-click one. In one transaction, for each balanced voucher:
  - an `AR_DIST` row, `PAYMENTCTL` "O", `AMOUNT` = `REMAINDER` = the total, with its VAT, AR account, invoice and due dates;
  - its `AR_JSUM` rows (`TYPE` "AR", the voucher's period and invoice date);
  - `MODE` "0".
- **Cancel** lists the open vouchers and marks one cancelled, `MODE` "X" (D3), keeping its lines for the record.
- **Recover** (new: Transactions > Recover Sales Voucher, beside the purchase one) reopens a posted voucher whose journal isn't yet transferred and which has no receipt. It removes its `AR_DIST` row and its `AR_JSUM` rows, and sets `MODE` "1".

### 4.4 The transfer (1028, AR half; D2)

Maintenance > **Update AR and AP** transfers both journals, as Spec #5 §4.4 does for AP:
- `AR_JSUM` rows with `MODE` "1" become **one GL voucher per period**, its lines netted by account and dept, dated the period's end, described "Accounts Receivable GL Update". A closed period, or one with no end date, is reported and left waiting.
- The rows get `MODE` "2" and an **`AR_XFER`** row each (the `AR_JSUM` row, the GL voucher, the period), as `AP_XFER`. Posted vouchers whose rows are all transferred get `MODE` "2"; cancelled ones keep "X" (§2.6 #13).
- AP is transferred first, then AR, each into its own vouchers. The screen's status line counts both journals' waiting rows, and its message lists both.
- Cancelling an AR transfer voucher in Spec #2's Cancel Voucher sets its rows back to "1", as for AP.

---

## 5. Receipts (D8–D10)

### 5.1 Clear Receipts (4054, 1156 and 1155)

- **The list (1156)** shows the invoices with something still owed (O and R), with customer, invoice, due date, amount, received and owed. **View All** adds the received ones.
- **Double-click one: the receipt (1155).** The bank (the `GL_SETAT` cash accounts), the date (in an open period), the amount received (default: what's owed), and:
  - **in full** when the amount is what's owed;
  - **partial** when it is less, and Discount isn't ticked: the invoice stays owed for the rest;
  - **with a discount** when it is less and Discount is ticked: the rest is written off as an early-payment discount, and the invoice is settled.

  More than what's owed is refused. The W/H box is hidden (Q3).
- **OK records the receipt and its GL entry at once**, in one transaction (D9). The list's separate Create step goes: there is nothing left to create.

### 5.2 The receipt's entry

Dated the receipt date, in its period, `TYPE` "RC", with the invoice's voucher number and the bank:
- Dr bank, the amount received;
- for a discount d (what was owed less the amount received): d is split as a sale (§4.1): Dr the voucher's **VAT account** round(d × v ÷ (100 + v)), and Dr the customer's **sales account** the rest of d (so a discount of a whole invoice reverses its split exactly), so output VAT falls with the price, as the original intended (§2.5); the sales account is the invoice's, not a session global (§2.6 #2);
- Cr the invoice's AR account, the amount received plus d.

The invoice's `AR_DIST` row: `AMOUNTPAID` grows by the amount received, `REMAINDER` falls by the amount plus any discount, `DATEPAID` and the bank are the latest receipt's. `PAYMENTCTL` becomes **R** while something is owed, **C** when nothing is (D8).

### 5.3 The receipts table (D8)

A port table, **`AR_RCPT`**, keeps each receipt: the invoice (voucher number), the customer, date, period, bank, amount, discount, the discount's VAT, and a reference, numbered across all invoices (`RCPTNO`). The receipt's `AR_JSUM` rows carry that number in `CHEQUE`, so reversing it finds exactly its rows. `AR_DIST` holds only the latest receipt's fields, so without it a second partial would overwrite the first, and a receipt couldn't be reversed. The deployed table has no rows to migrate.

### 5.4 Reversing a receipt (D10)

On the list, an invoice's receipts can be shown (**Receipts**) and the latest reversed, for a receipt entered by mistake or a bounced cheque:
- if its journal is still waiting, its rows are removed;
- if it has gone to the General Ledger, new rows reverse it, dated the reversal, in that date's open period;
- the invoice's `AMOUNTPAID`, `REMAINDER` and `PAYMENTCTL` go back; the `AR_RCPT` row is deleted.

Only the latest receipt of an invoice can be reversed, so the invoice's fields can always be put back.

---

## 6. Purge (4062, dialog 1062; D11)

Maintenance > **Purge Received Receivables** (the original's "Purge Cleared Accounts", on Transactions) deletes the fully received invoices (C) whose last receipt is before a date, with their receipts. Open and partly received invoices are never touched. As Spec #5's purge, the list shows what goes, and Purge asks first.

---

## 7. Reports (D14)

All on Spec #3's engine, with accounts from `GL_SETAT` and the customer records, on AR's own tables.

| ID | Report | From | Notes |
|---|---|---|---|
| 4063 | Sales Pre-Posting | `ARBTCHSM` | the open vouchers with their lines and totals |
| 4073 | Sales Voucher Detail | `AR_BTCHD` | the current period's vouchers, every state; on no menu, added |
| 4070 | Voucher Status | `ar_bstat` (eiwa) | a line per voucher of the current period with its state |
| 4056 | Customer List | `AR_VEND` | the original's "Customer Detail" |
| 4058 | Customer Summary | — (new) | a line per customer: invoiced, received, owed. The menu item had no handler |
| 4057 | Open Receivables Detail | `AR_DETL` | what's owed (O, R) by customer; also Accounts > Customer Inquiry Report |
| 4060 | AR Register | `AR_BALA` | every open item with its receipts |
| 4068 | Aged Receivables 30-60-90 | `AR_AGEDC` | as at a date, by days past due; current and not yet due in the first column |
| 4061 | Collections Forecast | `AR_AGEDR` | owed by days to the due date (≤10, 11–20, 21–30, over 30), without the day-20 double count; 4061 printed the aged report again |
| 4076 | Receipts Register | — (new) | the receipts of the current period: invoice, bank, amount, discount |
| 4071 | VAT: Sales | `AR_VATAP` | the output VAT of the period's posted vouchers and discounts, on AR's own lines |
| 4072 | VAT: General Ledger | `AR_VATGL` | the GL lines on `VATP`'s account (the original named 1491000) |
| 4074 / 4075 | AR Journal: Summary / Detail | `AR_JSUM`, `AR_JSUMD` | on `AR_JSUM`, not AP's journal; the labels corrected |
| — | Customer Statements | `AR_STAT` | new on the menu: per customer, the open items with the address, one a page |

**Not ported:** 8137 (`ar_cust.rpt` exists nowhere and nothing calls it) and the unreferenced AP copies (`AR_ADJBT`, `AR_ADJST`, `AR_CASHB`, `AR_CASHD`, `AR_CHEQA`, `AR_CHEQC`, `AR_JSUMA`, `AR_JSUMC`, `AR_POSTJ`, `AR_PREPY`, `AR_REB`, `AR_REQSD`, `AR_REQST`, `AR_SLIPS`). **Menu labels** that named the wrong report are corrected by `prune_rc.py` edits; 4073, 4071, 4072, 4076 and the statements join the menu.

---

## 8. Deviations from the original (decision record)

| # | Decision | Why |
|---|---|---|
| D1 | **Port AR, fixing the cycle.** Screens keep their templates, IDs and strings where they work. | AR was never run (§2.7); as written, receipts don't reach the GL and vouchers go out of balance (§2.6). |
| D2 | **The transfer makes one GL voucher per period**, dated its end, from the rows' own periods, linked in `AR_XFER`; closed periods are reported, not moved. Cancelled vouchers keep "X". | §2.6 #13; as Spec #5 D2. |
| D3 | **Separate modes:** "1" open, "0" posted, "2" transferred, "X" cancelled. | §2.6 #7. |
| D4 | **One VAT formula in cents.** The voucher's VAT and AR lines are recomputed from its product lines, whose typed amounts are kept in `AR_LINEX`. Rates are whole percentages, 0 meaning none. | §2.6 #5; as Spec #5 D4. |
| D5 | **"No" to saving removes only a voucher created in this session.** | §2.6 #10. |
| D6 | **The duplicate check covers every voucher not cancelled**, not only posted ones. | §2.6 #9. |
| D7 | **Recover** reopens a posted, untransferred voucher with no receipt, removing exactly its own `AR_DIST` and `AR_JSUM` rows. | §2.6 #8. |
| D8 | **The receipt states** O → R (partly received) → C (settled), with each receipt kept in `AR_RCPT`. P and D are no longer states: a full receipt and a discount both settle the invoice. Purge removes C. | §2.6 #1, #3; the original's P/D/R were marks waiting for Create. |
| D9 | **A receipt is journalled when it is entered**, dated by the receipt in its own period: Dr bank, Dr sales and VAT for a discount, Cr AR. The Create step goes. | §2.6 #1–#4. |
| D10 | **A receipt can be reversed**: removed if waiting, reversed with new rows if transferred, the latest one per invoice. | A receipt entered by mistake or a bounced cheque otherwise needs a hand-made GL voucher and leaves `AR_DIST` wrong. As Spec #5's Void. |
| D11 | **Purge** deletes settled invoices whose last receipt is before a date, and their receipts; it moves to Maintenance beside AP's. | §2.4; as Spec #5 D8. |
| D12 | **Customers:** the options' fields fill from the right fields; Set Data finds the typed customer; Delete is refused while the customer has vouchers or open items; new customers default to `GL_SETAT` `AR`, `SALE` and `VATP`; no W/H field. | §2.2, §2.6 #11. |
| D13 | **The analysis codes go to their own columns.** | §2.6 #12; as Spec #5 D13. |
| D14 | **The reports in §7**, on AR's own tables, with accounts from `GL_SETAT`; two new ones (Customer Summary, Receipts Register) fill menu items that had none, and the customer statement goes on the menu. | §2.6 #17. |
| D15 | **No withholding** on invoices or receipts (Q3). | As the original; `AR_CUST` has no W/H field. |
| D16 | **The entry screen is modal.** | Every port screen is; a modeless one could have its voucher posted while open. |

---

## 9. Development fixture

`make_dev_fixture.py` gains the AR tables with the deployed layouts (`accounts_receivable()`), in period 7, using accounts the fixture already has (AR 111101, sales 410001-10 and 410001-20, VAT payable 260007, the bank 102101), all with their current balances untouched:

- **Customers:**
  - C001: a hospital, VAT 10 %, terms 30;
  - C002: a doctor, VAT 10 %, terms 30, sales dept 20;
  - C003: no VAT, terms 0.
- **Vouchers:**
  - two open (one with two product lines, quantity × price);
  - one posted, owed in full;
  - one posted and partly received, with its receipt;
  - one posted and settled, with its receipt (purgeable);
  - one cancelled.
- **The journal:** the posted vouchers' and receipts' rows waiting, not transferred, so no GL balance moves.

`AR_LINEX` and `AR_RCPT` rows are written for them. The Spec #1–#5 figures don't move: no account balance changes, and the Spec #5 sheet's `ACCESSM` count changes only if the menu does (checked as in S4-M0 and S5-M0).

---

## 10. Acceptance criteria — manual checks (outline)

1. **Customers:** add, modify, find by Enter, options filled from the right fields; Delete refused with vouchers.
2. **Invoice entry:**
   - C001 invoice, 3 × 1,100.00 at 10 % VAT: Cr sales 3,000.00, Cr VAT 300.00, Dr AR 3,300.00.
   - A second line recomputes the VAT and AR lines; Delete recomputes too; the voucher still balances.
   - The same invoice number is refused, posted or not.
3. **Post:** `AR_DIST` open, `AR_JSUM` rows, mode 0. **Cancel** marks "X". **Recover** reopens.
4. **Transfer:** one GL voucher per period for AR beside AP's, posting with Spec #2; the Trial Balance moves by the vouchers' totals; 111101 carries what's owed.
5. **Receipts:**
   - in full: Dr bank / Cr AR, dated the receipt, in its period; the invoice is settled;
   - in part: the amount received is journalled, the rest stays owed; a second partial adds to the first;
   - with a discount: Dr bank, Dr sales and Dr VAT for the discount, Cr AR the whole remainder; settled;
   - more than owed is refused.
6. **Reverse:** a waiting receipt removed; a transferred one reversed, dated the reversal; the invoice owed again.
7. **Purge:** removes the settled invoices only.
8. **Reports:** each previews and prints; the aged report's buckets add to the open total; the statements show each customer's open items.
9. **Regression:** the Spec #1–#5 sheets pass; `/checkres` and `/stress` exit 0.

---

## 11. Risks

| # | Severity | Risk | Mitigation |
|---|---|---|---|
| R1 | Medium | The source doesn't match any build (§2.0), so the port can't be checked against the original's behaviour. | AR never ran (§2.7): there is no behaviour to match. The spec ports the source's intent, and the tests fix the figures. |
| R2 | Medium | The receipt rules (discount with VAT reversal, partials) differ from what the finance team does. | Q2 answered by the user; one function (`Ar.cpp`) holds them, with tests. |
| R3 | Medium | Withholding on receipts is needed after all (hospitals and companies may withhold). | Q3: left out as the original; adding it touches the receipt function and the dialog's hidden box. |
| R4 | Low | Sharing Spec #5's code couples AR to AP. | Only helpers and the transfer are shared; each module keeps its own tables and core. |
| R5 | Low | Real customers with stray data (R0006's `VAT_DEPT`). | The customer screen validates on save; the reports trim. |

---

## 12. Milestones (the plan comes after review)

| Milestone | Content |
|---|---|
| S6-M0 | `Ar.{h,cpp}`: tables, records, the arithmetic (§4.1), modes (D3), `AR_LINEX`, `AR_RCPT`, `AR_XFER`; the fixture; tests |
| S6-M1 | Customers (1052, 1063) |
| S6-M2 | Sales invoice entry (1053) |
| S6-M3 | Post, Cancel, Recover; the AR half of the transfer (1030) |
| S6-M4 | Receipts (1156, 1155): full, partial, discount; reversing |
| S6-M5 | Purge (1062) |
| S6-M6 | Reports; the report list; menu labels; `/stress`, `/checkres`; README |
| S6-M7 | Acceptance sheet, walkthrough, sign-off |

---

## 13. Still open

Questions for the finance team (and the user), with the spec's assumption until they answer.

**Answered (the user, 2026-11-13):**

- **Q1 How much of AR?** **All of §1.1.** (The other choices were invoices and the transfer only, or the customer master only.)
- **Q2 Receipts:** **in full, in part, and with an early-payment discount**, as the original intended, fixed (§5). The other choices were full and partial only, or full only.
- **Q3 Withholding on receipts:** **none, as the original** (D15). It stays a question for the finance team (R3).

**Still assumed** until the finance team answers:

- **Q4 Credit notes and returns:** needed? *Assumed: no (§1.2).*
- **Q5 A printed sales invoice** for the customer? *Assumed: no; the voucher prints on the reports (§1.2).*
- **Q6 Statements:** how often, and with which items (open only, or the period's activity)? *Assumed: open items, on demand (§7).*
- **Q7 Credit limits** (`AR_CUST.CREDIT`): enforced at entry? *Assumed: no.*
- **Q8 The deployed customers' accounts** (111199/111102 AR, 410099/410002 sales, which `GL_SETAT` doesn't name): keep as they are? *Assumed: yes; new customers default to `GL_SETAT` (D12).*
