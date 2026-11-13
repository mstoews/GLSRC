# Spec #5 — OWL-to-Win32 SDK Port: Accounts Payable

**Status:** Reviewed (2026-11-13): Q1 and Q2 answered; the plan is `plans/2026-11-13-owl-to-win32-sdk-port-spec5-plan.md`
**Date:** 2026-11-11
**Scope:** Fifth spec of the migration: the Accounts Payable module.
- vendors;
- purchase invoice vouchers and cash (paid-at-entry) vouchers, with their posting and cancelling;
- the payment cycle: selecting invoices, cheques, the payments' GL entry, cheque printing, bank reconciliation, purge;
- the transfer of AP's journal to the General Ledger;
- the AP reports, on Spec #3's engine.

The original's cycle has broken links at almost every step (§2.6), and the company never used AP beyond a few test vouchers (§2.7). So, as for Fixed Assets (Spec #4), the spec fixes what it ports. Q1 asked first whether all of it is wanted: it is (§13).

**Depends on:** Specs #1–#4. Spec #2's GL vouchers carry AP's GL entries; Spec #3's `report` library prints the reports; the Spec #2 master-data and voucher-entry screens are the pattern. Everything they fixed still holds: the build, the error idioms, the Unicode strategy, the translation pattern and the per-port checklist.

**Numbering.** Seam 5 of Spec #1 §9 ("AP module port"). AR stays Spec #6. The "Update AR and AP" transfer (1028) is shared: this spec ports its AP half, and Spec #6 adds AR.

**Sources.** `gl/src/ap_*.cpp` (8,528 lines; built per `gl/gl.ide`), `gl_apupd.cpp`, the table classes in `include/db_ap*.h`, `db_vend.h`, `db_jsum.h`, `db_cash.h`, dialogs and strings in `gl/gl_new.rc` (1045 only in `gl/gl.rc`), and the deployed `dbf/AP_*`. Two surveys of the code (2026-11-11) are summarized in §2. File and line references are to the original.

---

## 1. Goals & Non-Goals

### 1.1 Goals (Spec #5 deliverables)

1. **Vendors** (4045, dialog 1041).
2. **Purchase invoice vouchers** (4044, dialog 1042): entry, with VAT and withholding tax computed from the vendor's rates (D4); **Post** (1048), **Cancel** (3064), and **Recover** of a posted voucher not yet transferred (D7).
3. **Cash invoice vouchers** (1079, dialog 1044): invoices paid when entered, with their Post (1088) and Cancel (1085).
4. **Payments:**
   - **Create Cheques** (1057, dialog 1045): select the invoices due, hold or force them, and create cheques, one per vendor and bank, numbered at once (D9). The payment's GL entry (Dr AP, Cr bank) is written then.
   - **Print Cheques** (4091) and confirming they printed.
   - **Bank Reconciliation** (6085): clear cheques.
   - **Purge** of cleared payments (1066).
5. **The AP → GL transfer** (1028, AP half): AP's posted journal becomes GL vouchers, one per period, which post the usual way (D2).
6. **Reports:** the AP reports that work or can be made to (D14), on Spec #3's engine, in the report list.
7. **Fixture and tests** as Specs #2–#4: a portable core (`ApCore.h` or similar) under `gl_tests`.

### 1.2 Non-Goals

- **Partial payments, credit notes, debit memos and adjustments.** The original has none that work: `ADJUSTMENT` is never written; 1082/1990's adjustment reports print nothing useful; `APPostAdj` is empty.
- **Printing on pre-printed cheque stock with exact positioning.** The original's `AP_CHEQ.RPT` was laid out for an Epson EX-1000 on LPT1. Cheques print as a report (amount in words, payee, stub); a form-exact layout is Q6.
- **Foreign currency**, purchase orders, three-way matching, recurring invoices, vendor statements.
- **`AP_REGIS`, `AP_CTRL`, `AP_SYS`, `AP_TAG`.** No working code uses them (§2.1). They stay in the database, untouched.
- **AR** (Spec #6), including the AR half of 1028.

---

## 2. Accounts Payable in the original

### 2.1 Tables

| Table | Rows (deployed) | Role |
|---|---|---|
| `AP_VEND` | 130 | Vendors. `ACCOUNT`/`DEPART` are the vendor's **AP control account**. `VAT_ACCT`/`VAT_DEPT` are its input-VAT account. `PERCENTAGE` is VAT %; `REBATE` is withholding (W/H) %; `TYPE` is "C" company or "I" individual; `TERMS` are days. `AP_ACCT`/`AP_DEPT`, `VAT`, `FIVE`, `THREE` are never used. |
| `AP_BTCH` / `AP_BTCHD` | 9 / 22 | Invoice voucher header and lines. The header carries vendor, invoice no., dates, period, AP account, `PAYABLE` (gross), `VATAMT`, `REBATEAMT` (signed W/H). `MODE` is "1" open, "0" posted, "2" **both** transferred-to-GL and cancelled. |
| `AP_CASH` / `AP_CASHD` | 1 / 4 | Cash voucher header and lines. The header's `ACCOUNT`/`DEPART` are the bank. `MODE` is "1" open, "0" posted, "2" cancelled. |
| `AP_DIST` | 9 | **The open-item ledger:** one row per posted invoice. It holds the AP account, amount, VAT, W/H, due date, and the payment fields: bank `ACCOUNT`/`DEPT`, `CHEQUENO`, `AMOUNTPAID`, `DATEPAID`. `PAYMENTCTL` is H held, O open, F forced, N selected, P paid (cheque made), V printed, C cleared. |
| `AP_REQST` | 3 | The cheques: vendor, amount, date, cheque no., bank, `CLEARED`. Rebuilt from scratch by every Set Cheque Numbers run. |
| `AP_JSUM` | 40 | **AP's journal, staged for the GL:** account, dept, period, date, debit, credit; `TYPE` "AP" (invoices and payments) or "CA" (cash vouchers); `MODE` "1" waiting, "2" transferred. |
| `GL_SETAT` | (GL) | Named accounts: `AP` 221002, `VATR` prepaid VAT 148000, `VATP` VAT payable 260007, `WTXC`/`WTXI` withholding 260004; and the banks (`TYPE` "CA"). |
| `AP_SETUP`, `AP_CTRL`, `AP_REGIS`, `AP_SYS`, `AP_TAG` | 2, 0, 2, 12, 0 | Not used by any built code. `AP_SETUP` belongs to the unbuilt `ap_setup.cpp`; `AP_CTRL` is zapped by every Post; `AP_REGIS` has no reference at all. |

**Layouts worth knowing** (checked against the deployed `.DBF`/`.MDX`, 2026-11-13):
- `AP_BTCHD` / `AP_CASHD` lines have no mode, period, date or reference: only voucher, line number, description, the three analysis codes, account, dept, debit and credit. The voucher's header carries the rest.
- The cash lines' doctor column is `DRCODES`; the invoice lines' is `DRCODE`.
- `INVOICENO` is 8 characters in `AP_BTCH` and `AP_DIST`, 10 in `AP_CASH`.
- `AP_JSUM.PERIOD` is text (C4), and `AP_JSUM` has no field for the GL voucher a transfer makes (§4.4).
- Tags: `AP_BTCH`/`AP_CASH` `batchno`, `invoiceno` (`VENDORNO+INVOICENO`), `account`; `AP_DIST` `batchno`, `invoiceno`, `vendorno`, `chequeno`, `paymentreq`, `cheques` (filtered on P); `AP_JSUM` `batchno`, `account`, `apdist`/`ardist` (filtered on the type); `AP_REQST` `paymentreq`, `vendorno`; `AP_VEND` `vendorno`.

### 2.2 Vendors (4045, dialog 1041)

- **Fields:** number, names, address, phone/fax, contact, VIN, VAT %, W/H %, company or individual, terms, discount days and %, AP account, VAT account, med rep.
- **Defaults:** the AP account from `GL_SETAT AP`, the VAT account from `VATR`. The add-vendor popup from invoice entry uses `VATP` instead, which is inconsistent.
- **Add/Mod** asks before updating. **Delete** deletes and packs, with no check for open invoices.
- **Set Data** seeks a boolean (a misplaced parenthesis), so it never finds the typed vendor.

### 2.3 Purchase invoice vouchers (4044, 1042)

**One Add posts one invoice amount on one expense account.** The header carries the vendor's VAT and W/H rates, editable. Then:

- **The amounts:** `GetAPExpense` splits the amount into expense, VAT, W/H and AP. It has two modes, chosen by `GL.INI [WITHHOLDING] TAX` (default "T"; the sample `gl.ini` says "F"):
  - **F:** VAT is included in the amount; W/H is credited, on the gross. For 1,120 at 12 % VAT and 1 % W/H: Dr expense 1,000, Dr VAT 120, Cr W/H 11.20, Cr AP 1,108.80.
  - **T:** the amount is grossed up, so W/H is *debited* and AP credited at the full amount. For 1,120 at 12 % and 1 %: Dr expense 991.15, Dr VAT 118.94, Dr W/H 9.91, Cr AP 1,120.
- **The lines:** the expense line is appended; the VAT, W/H and AP lines are replaced by account (`UpdateBatchDetail`).
- **The accounts:** the expense is typed; VAT is the vendor's (editable); W/H is `GL_SETAT WTXI`/`WTXC`, chosen by the company/individual radio; AP is the vendor's `ACCOUNT`.

### 2.4 Posting, cancelling, the transfer

- **Post (1048):** for each open voucher that balances (an exact `double` compare):
  - one `AP_DIST` row, `PAYMENTCTL` "H" (held);
  - one `AP_JSUM` row per line, `TYPE` "AP", `MODE` "1";
  - the voucher's `MODE` becomes "0".

  No GL table is touched.
- **Cancel (3064):** for an open voucher: "Cancelled" in the reference, the payable and lines zeroed, `MODE` "2".
- **Update AR and AP (1028):** every `AP_JSUM` row with `MODE` "1" (invoices, payments and cash) is netted by account into **one GL voucher**, in the **GL's current period**, whatever the rows' own periods; the rows get `MODE` "2", and every posted invoice voucher too. The GL voucher is then posted by Spec #2's Post Voucher.

### 2.5 Payments

1. **Create Cheques (1057):**
   - **Set Date** lists the held, open and forced invoices due by a date. Double-clicking one sets it Hold, Force or Open (dialog 1066).
   - **Create** takes the **open and forced** ones (Force is no different from Open) and marks them "N". It gives one cheque number per vendor run, sets the bank and pays the **full amount**.
   - It then writes two `AP_JSUM` rows per invoice (Dr AP, Cr bank), dated the *invoice* date, and marks the invoices "P".
2. **Set Cheque Numbers (8074):** zaps `AP_REQST`, renumbers every "P" invoice, one cheque per vendor (across banks), and rebuilds `AP_REQST`.
3. **Print Cheques (4091, `AP_CHEQ.RPT`):** every `AP_REQST` row, with the amount in words. **Verify** marks "P" as "V".
4. **Bank Reconciliation (6085):** lists "P" cheques only, so nothing shows once verified. Double-click clears a cheque ("C"), then breaks the screen (`cb.init()` closes its tables).
5. **Purge (1066, on no menu):** deletes the "P" rows, not the finished "V"/"C" ones.

### 2.6 What doesn't work

1. **The duplicate-invoice check** seeks on the wrong tag, so the same invoice can be entered twice.
2. **Modes:** "2" means both transferred and cancelled. Cancelled vouchers stay in the entry screen's navigation; adding to one silently "recovers" it.
3. **Recover (re-adding to a posted voucher):**
   - It deletes **AR**'s journal rows (the `Tgldist` class it uses opens `AR_JSUM`) and no AP rows.
   - It deletes no `AP_DIST` row (wrong tag).
   - Re-posting doubles the AP journal.
4. **Saving:**
   - "No" to "save the current transaction?" deletes the **whole** voucher, including lines saved before.
   - Modify and Delete of a line never recompute the VAT, W/H or AP lines, so the voucher goes out of balance.
5. **Arithmetic:**
   - **"No VAT" is a sentinel rate of 1:** in T mode it is used as a real 100 % rate. An invoice without VAT then books VAT equal to the expense and a huge negative W/H.
   - **A 100 % rate counts as "none".**
   - **Rounding:** amounts are computed to 3 decimals but stored with 2, and the balance check is an exact `double` compare, so rounding can block posting.
6. **The sample data matches neither formula** but an older, commented-out one (W/H on the expense).
7. **Swapped columns:**
   - The product and doctor codes are written to each other's columns (the class maps fields 5 and 6 the wrong way round). The product code is also cut to 5 characters.
   - Modify then swaps med rep and product as well.
8. **The transfer:**
   - It discards the AP vouchers' periods and nets everything into the GL's current period.
   - Payments are dated by the invoice date.
   - Cash vouchers posted to `AP_JSUM` are never marked transferred.
9. **Payments:**
   - Force means nothing.
   - Set Cheque Numbers renumbers cheques already printed and wipes `AP_REQST`.
   - One cheque can cover two banks.
   - Bank Reconciliation shows only unprinted cheques and breaks after one use.
   - Purge deletes the unfinished rows.
   - Create Cheques shows an AR message (4074) and a missing string (4079); its "Print" selects only forced invoices.
10. **Reports:**
    - Several open the wrong file (1082 names `apadjst.rpt`).
    - Several hard-code accounts (148000, 1491000, 2210000, 2220000).
    - Several select nothing useful (AP_REB's selection is commented out).
    - One double-counts day 20 (AP_AGEDR).
    - Several menu items are mislabelled: "Customer Sales Statements" prints cheques; "Summary of Cash Entries" and "Summary of AP Entries" are swapped.
11. **Other defects:**
    - Many controls the code reads aren't in the dialogs.
    - Strings 4079, 4081, 4084, 4086, 4110 and 4111 are missing.
    - Win16 16-bit integers are overwritten by 32-bit stores.
    - `ValidateGL` returns true when the account is *not* found.

### 2.7 The real data

Checked against the deployed company (`dbf/`):

- **Vendors are real:** 130 of them, most pointing at AP accounts 260008 (non-trade, 44) or 221002 (trade, 18), with VAT account 148000.
- **Everything else is test data from March 1995:**
  - 9 invoice vouchers, all period 2, with references like "SAMPLE INVOICE" and "Sample cheque printing";
  - 1 cash voucher ("TEST INVOICE NO. 1");
  - 3 cheques (29145–29147), all marked cleared.
- **The 40 `AP_JSUM` rows were transferred:** their effect is in the GL balances. For example, 102101 is −36,500.00, and 711000 clinical-trial samples is 16,000.00.
- **No AP-typed line is in `GL_DIST`.**

**So AP was set up and tested, with its vendor master kept, but not run in production.** That makes Q1 the first question. It also means the port isn't bound to the original's figures: as with Spec #4, fixing the cycle changes no real history. The test postings already in the GL balances stay as they are.

---

## 3. Screens and port files

| Original | ID | Port file | Notes |
|---|---|---|---|
| `TAPVendorAdd` | 4045, 1041 | `gl/port/ApVendors.{h,cpp}` | D12 |
| `TAPInvoiceEntry` | 4044, 1042 | `gl/port/ApInvoiceEntry.{h,cpp}` | D4–D6 |
| `TAPPostBatch`, `TAPCancelBatch` | 1048, 3064 (1047, 1043) | `gl/port/ApPosting.{h,cpp}` | Spec #2's Post/Cancel list dialogs; Recover (D7) |
| `TAPCashEntry`, cash post/cancel | 1079, 1088, 1085 (1044, 1047, 1043) | `gl/port/ApCashEntry.{h,cpp}`, `ApPosting` | D10 |
| `TAPCreatePayment`, `TAPHoldForce` | 1057 (1045, 1066) | `gl/port/ApPayments.{h,cpp}` | D8, D9 |
| `TPrintCheques`, `TAPPayment`, `TAPSetChequeNumber` | 8074, 6085 (1157, 1046, 1051) | `ApPayments` | D8, D9 |
| `TAPPurgeBatch` | 1066 (1048) | `ApPayments` | D8 |
| `TGLAPUpdate` (AP half) | 1028 (1030) | `gl/port/ApTransfer.{h,cpp}` | D2 |
| `ap_repts.cpp` | 1049 … 8078 | `gl/port/reports/ApReports.{h,cpp}` | D14 |
| — | — | `gl/port/Ap.{h,cpp}` | portable: tables, records, the arithmetic, posting, payments, the transfer; `gl_tests` checks it |

Dialogs come from `gl_new.rc` through `prune_rc.py`; 1045 comes from `gl.rc`, as 1061 did in Spec #2. Changes that a template can't carry are made in code, as in Specs #3 and #4.

---

## 4. The invoice voucher

### 4.1 Amounts (D4)

All amounts are in cents (`rpt::ToCents`), as Spec #2 D12.

- **For each expense line**, with `v` the VAT rate and `w` the W/H rate (whole percentages, 0 meaning none), on the line's amount `a` as typed:
  - **F mode** (VAT included, W/H withheld from the payment):
    - VAT = round(a × v ÷ (100 + v));
    - expense = a − VAT;
    - W/H = round(expense × w ÷ 100), on the net, as the deployed sample's postings show (Q2).
  - **T mode** (amounts grossed up):
    - expense = round(a × 100 ÷ (100 + v + w));
    - VAT = round(expense × v ÷ 100);
    - W/H = a − expense − VAT.
- **The voucher's lines are recomputed from its expense lines** whenever one is added, changed or deleted:
  - one VAT line on the voucher's VAT account (Dr, the sum);
  - one W/H line on `WTXC`/`WTXI` (F: Cr; T: Dr);
  - one AP line on the voucher's AP account (Cr): F: Σa − W/H; T: Σa.

  The voucher always balances to the cent; rounding stays in the VAT/W/H lines (D4).
- **The mode** is the Company Profile setting (`GL.INI [WITHHOLDING] TAX`), as in the original. Its default is "F" when the key is missing (the original's "T" default contradicts the deployed `gl.ini`; Q2).

### 4.2 The entry screen (1042)

The Spec #2 Voucher Entry pattern:
- **The voucher:** a header (vendor, invoice no., dates, period, rates, accounts), then expense lines entered in the line fields: account, dept, description, med rep, product, doctor, amount.
- **Lines:** the Distribution List beside it shows all the lines. Double-click an expense line to edit it; Delete removes one. Both recompute (§4.1).
- **Lookups:** Enter on the vendor shows the vendor; Enter on the voucher number shows the voucher.
- **New and Close:** for a voucher created in this session, they ask whether to keep it. No removes only that new voucher (D5).
- **Due date:** invoice date + terms, editable (the original overwrote what was typed).
- **Duplicates:** the same vendor and invoice number in any voucher that isn't cancelled is refused (D6).
- **Analysis codes:** med rep, product and doctor go to their own columns (D13).

### 4.3 Posting, cancelling, recovering (D2, D3, D7)

- **Post** lists the open vouchers. Post all, or double-click one. In one transaction, for each balanced voucher:
  - an `AP_DIST` row, `PAYMENTCTL` "O" (open). The original's "H" made every invoice wait for a manual release (Q3);
  - its `AP_JSUM` rows (`TYPE` "AP", the voucher's period and invoice date);
  - `MODE` "0".
- **Cancel** lists the open vouchers. It marks the voucher cancelled, `MODE` "X" (D3), keeping its lines for the record.
- **Recover** (new, on the Post list's sibling: Transactions > Recover Voucher > Purchase Vouchers) reopens a posted voucher whose journal isn't yet transferred and whose invoice isn't in a payment. It removes its `AP_DIST` row and its `AP_JSUM` rows, and sets `MODE` "1".

### 4.4 The transfer (1028, D2)

Maintenance > **Update AR and AP**:
- `AP_JSUM` rows with `MODE` "1" become **one GL voucher per period** (Spec #2 `SaveLine`, mode 1, numbered `NextVoucherNumber`). Its lines are netted by account and dept, it is dated the period's end, and its description is "Accounts Payable GL Update" (1049).
- A period that is closed in `gl_pds` is reported and left untransferred, rather than moved into another period.
- The rows get `MODE` "2", and the GL voucher's number is recorded with them. `AP_JSUM` has no field for it, so where it is kept is settled in plan S5-M3, as Spec #4 kept its run history in a table of its own (`gl_fadep`). Posted invoice and cash vouchers get `MODE` "2" (transferred).
- The GL vouchers post with Spec #2's Post Voucher; Cancel Voucher on one sets its rows back to "1" (as Spec #4's undo).

---

## 5. Cash vouchers (D10)

**1079 (dialog 1044):** an invoice paid by cheque or cash when it is entered.
- **Entry:** as §4, but the header names a **bank** (from `GL_SETAT` `CA` accounts) instead of an AP account, and a cheque number. The credit line is the bank, not AP (F: Σa − W/H; T: Σa).
- **Post** writes `AP_JSUM` (`TYPE` "CA", with the cheque number) and sets `MODE` "0". There is no `AP_DIST` row: nothing is owed.
- **Cancel** works as for invoices.

---

## 6. Payments (D8, D9)

### 6.1 Create Cheques (1057, 1045)

1. **Select:**
   - The screen lists the posted invoices due by a date: `PAYMENTCTL` O, F or H, with the amount still owed (amount − W/H).
   - Double-click toggles **Hold** (H: not paid until released), **Force** (F: paid even if not yet due) or **Open** (O).
2. **Create**, in one transaction:
   - takes the O invoices due and the F ones;
   - groups them by vendor and bank: one cheque each, numbered from the starting number (default: the last cheque + 1);
   - writes the cheque (`AP_REQST`: vendor, amount, date, number, bank, not cleared);
   - writes for each invoice: `CHEQUENO`, `AMOUNTPAID`, `DATEPAID`, bank, `PAYMENTCTL` "P";
   - writes the payment's `AP_JSUM` rows: Dr AP / Cr bank, **dated the payment date, in its period**, with the cheque number.
3. The list then shows the cheques made.

### 6.2 Cheques (4091, 8074)

- **Print Cheques** previews and prints the cheques not yet printed (Spec #3 engine): payee, amount in figures and words, date, and a stub with the invoices.
- **Confirm Printed** (8074, replacing Set Cheque Numbers) marks a range as printed ("V"). A spoiled cheque can be **renumbered** before it is confirmed: its new number goes on the cheque and its invoices (D9).

### 6.3 Bank Reconciliation (6085, 1046)

- **The list:** the printed cheques not yet cleared, by bank.
- **Clear:** double-click one, or select several, to clear it ("C", `AP_REQST.CLEARED`), with the clearing date.
- **Void a cheque (new):** for a cheque not cleared. It reverses the payment's `AP_JSUM` rows (or, if transferred, writes reversing rows) and returns its invoices to "O".

### 6.4 Purge (1066)

Removes the **cleared** invoices and their cheques older than a date. Nothing open or uncleared is touched. It is on the menu (Maintenance > Purge Paid Payables).

---

## 7. Reports (D14)

On Spec #3's engine; all in the report list. Accounts come from `GL_SETAT`, not literals.

| ID | Report | From | Notes |
|---|---|---|---|
| 1078 | Invoice Pre-Posting | `APBTCHSM.RPT` | open vouchers, lines, totals |
| 1081 | Cash Pre-Posting | `AP_CASHB.RPT` | |
| 8077 / 8078 | AP / Cash Voucher Detail | `AP_BTCHD` / `AP_CASHD` | for a period, every mode |
| 1065 | Voucher Status | `AP_BSTAT` | |
| 1049 | Invoice Posting Journal | `AP_POSTJ` | without the hard-coded account |
| 1060 | Vendor List | `AP_VEND` | in vendor order (the original grouped by address line 1) |
| 1061 | Open Payables Detail | `AP_DETL` | unpaid only (the original counted V/C rows as open) |
| 1062 | Aged Payables 30-60-90 | `AP_OVERD` (1063 merged) | as at a date |
| 1064 | Cash Requirement | `AP_AGEDR` | the day-20 double count fixed |
| 1077 | AP Register | `AP_BALA` | invoices with payments |
| 1058 | Prepayments | `AP_PREPY` | |
| 4092 | Payment Detail | `AP_REQSD` | per cheque |
| 8073 | Cash Cheque Summary | `AP_CHEQC` | |
| 8067 / 8072 | VAT: AP / GL | `AP_VATAP`, `AP_VATGL` | VAT account from `GL_SETAT VATR` |
| 8071 | Withholding Tax | `AP_REB` | W/H accounts from `GL_SETAT` (its selection was commented out) |
| 8068 / 8069 / 8075 / 8076 | AP Journal: summary, detail, cash, invoices | `AP_JSUM*` | the cash/invoice menu labels corrected |
| 4091 | Cheques | `AP_CHEQ` | §6.2 |

**Not ported:** 1082 and 1990 (adjustments: there are none) and the unreferenced `AP_CHECK`, `AP_CHEQA`, `AP_CHEQS`, `AP_SUMRY`, `AP_VAT`. **Menu labels** that named the wrong report are corrected by `prune_rc.py` edits, as in Spec #3 D13.

---

## 8. Deviations from the original (decision record)

| # | Decision | Why |
|---|---|---|
| D1 | **Port AP, fixing the cycle.** Screens keep their templates, IDs and strings where they work. | AP was never run in production (§2.7); as written, invoices can't be paid and reconciled reliably (§2.6). |
| D2 | **The transfer makes one GL voucher per period**, dated its end, from the rows' own periods. Closed periods are reported, not moved. Cash vouchers are marked transferred too. | §2.6 #8. The GL must show AP activity in the period it happened. |
| D3 | **Separate modes:** "1" open, "0" posted, "2" transferred, "X" cancelled. Migrated "2" rows whose reference is "Cancelled" become "X". | §2.6 #2. |
| D4 | **One set of VAT/W/H formulas in cents.** The voucher's VAT, W/H and AP lines are recomputed from its expense lines. Rates are whole percentages, 0 meaning none. | §2.6 #4–6. |
| D5 | **"No" to saving removes only a voucher created in this session**, as Spec #2's Voucher Entry. | §2.6 #4. |
| D6 | **The duplicate check works:** the same vendor and invoice number in any voucher not cancelled, invoice or cash. | §2.6 #1. |
| D7 | **Recover** reopens a posted, untransferred, unpaid voucher, removing exactly its own `AP_DIST` and `AP_JSUM` rows. | §2.6 #3. |
| D8 | **The payment states** H → O/F → P (cheque made, GL entry written) → V (printed) → C (cleared). Purge removes C. Void returns P/V to O. | §2.6 #9. |
| D9 | **Cheques are numbered when they are created**, one per vendor and bank. Confirm Printed replaces Set Cheque Numbers; a spoiled cheque is renumbered, not the whole run. | §2.6 #9. |
| D10 | **Cash vouchers** post with their cheque number and are marked transferred by the transfer. | §2.6 #8. |
| D11 | **Posted invoices start Open (O)**, not Held. | The original's H needed a manual release per invoice (Q3). |
| D12 | **Vendors:** Set Data finds the typed vendor. Delete is refused while the vendor has vouchers or open items. New vendors default the VAT account to `VATR` (input VAT, the deployed vendors' 148000). | §2.2. |
| D13 | **The analysis codes go to their own columns.** Deployed rows are left as they are (they're test data). | §2.6 #7. |
| D14 | **The reports in §7**, with accounts from `GL_SETAT` and their defects fixed; the adjustment reports and unreferenced files are dropped. | §2.6 #10. |
| D15 | **The `AR_JSUM` / `AP_JSUM` class confusion is gone:** each module reads its own journal. | §2.6 #3. |

---

## 9. Development fixture

`make_dev_fixture.py` gains the AP tables with the deployed layouts:

- **Vendors:**
  - V001: a company, VAT 12 %, W/H 1 %, terms 30, AP account 221002;
  - V002: an individual, VAT 0, W/H 2 %;
  - V003: no VAT or W/H.

  The fixture's chart gains the AP, VAT and W/H accounts the vendors need, if it lacks them. The `GL_SETAT` rows AP, VATR, WTXC and WTXI point at them, and the bank 102101 is a `CA` row.
- **Vouchers:**
  - two open invoice vouchers (one with two expense lines);
  - one posted and untransferred, with its `AP_DIST` and `AP_JSUM` rows;
  - one cancelled;
  - one open cash voucher.
- **Payments:** one paid, printed invoice, with its cheque, not cleared.

Its expected figures go in the tests and the acceptance sheet, as in Specs #3 and #4. Adding accounts changes the Spec #3 sheet's account counts, so the fixture's additions are made so the Spec #1–#4 figures don't move (new accounts with zero balances, or the sheets updated), and checked as in S4-M0.

---

## 10. Acceptance criteria — manual checks (outline)

1. **Vendors:** add, modify, find by Enter, Delete refused with vouchers.
2. **Invoice entry:**
   - V001 invoice of 1,120.00, F mode: Dr expense 1,000.00, Dr VAT 120.00, Cr W/H 10.00, Cr AP 1,110.00 (W/H on the net, Q2).
   - A second expense line recomputes the VAT, W/H and AP lines, and the voucher still balances. Delete recomputes too.
   - The same invoice number is refused.
3. **Post:** `AP_DIST` open, `AP_JSUM` rows, mode 0. **Cancel** marks "X". **Recover** reopens.
4. **Cash voucher:** entry with the bank credited, post, cancel.
5. **Transfer:** one GL voucher per period, posting with Spec #2. The Trial Balance moves by the vouchers' totals; AP account 221002 carries the unpaid invoices.
6. **Payments:**
   - Create Cheques for the due invoices: one cheque per vendor and bank, Dr AP / Cr bank in the payment's period.
   - Hold keeps an invoice out; Force pays one not yet due.
7. **Cheques:** print to PDF, confirm printed, renumber a spoiled one.
8. **Bank reconciliation:** clear one; void another, whose invoice returns to open and whose GL entry is reversed.
9. **Purge:** removes the cleared items only.
10. **Reports:** each previews and prints; the aged report's buckets add to the open total.
11. **Regression:** the Spec #1–#4 sheets pass; `/checkres` and `/stress` exit 0.

---

## 11. Risks

| # | Severity | Risk | Mitigation |
|---|---|---|---|
| R1 | High | AP isn't wanted, or is wanted only in part (Q1). | Asked before the plan; the milestones are ordered so vendors + invoices + transfer can stand alone. |
| R2 | High | The tax rules (VAT inclusive or not, W/H on gross or net, debited or credited) differ from what the finance team needs. | Q2 before the invoice milestone; one function (`Ap.cpp`) holds them, with tests per mode. |
| R3 | Medium | Cheque printing on real stock needs exact positions. | Q6; the report engine can position by twips if needed. |
| R4 | Medium | The module is large (about 8,500 original lines): the largest spec yet. | Milestones of one screen each; a portable core first; the Spec #2–#4 patterns reused. |
| R5 | Low | Migrated test data with the old modes and swapped columns. | D3's migration of "2"/Cancelled; the deployed vouchers are test data, and Q5 asks whether to clear them. |

---

## 12. Milestones (the plan: `plans/2026-11-13-owl-to-win32-sdk-port-spec5-plan.md`)

| Milestone | Content |
|---|---|
| S5-M0 | `Ap.{h,cpp}`: tables, records, the arithmetic (§4.1), modes and migration (D3); the fixture; tests |
| S5-M1 | Vendors (1041) |
| S5-M2 | Invoice voucher entry (1042) |
| S5-M3 | Post, Cancel, Recover; the AP → GL transfer (1028) |
| S5-M4 | Cash vouchers: entry, post, cancel |
| S5-M5 | Create Cheques, Hold/Force; the payment's GL entry |
| S5-M6 | Cheque printing, Confirm Printed, Bank Reconciliation, Void, Purge |
| S5-M7 | Reports; the report list; menu labels; `/stress`, `/checkres`; README |
| S5-M8 | Acceptance sheet, walkthrough, sign-off |

---

## 13. Still open

Questions for the finance team (and the user), with the spec's assumption until they answer.

**Answered (the user, 2026-11-13):**

- **Q1 Is AP wanted, and how much of it?** **All of §1.1.** (AP was set up and tested in 1995 but never run, §2.7. The other choices were vendors, invoices and the transfer only, or the vendor list only.)
- **Q2 VAT and withholding tax:** **as assumed**: amounts VAT-inclusive, W/H withheld from the payment (F mode), W/H computed on the net, rates from each vendor. The deployed sample confirms it: voucher 4, 1,100.00 at 10 % VAT and 1 % W/H, booked expense 1,000.00, VAT 100.00 and W/H 10.00. T mode stays, chosen by `GL.INI [WITHHOLDING] TAX` as in the original (§4.1). The rules are one function in `Ap.cpp`, with tests per mode, so a later change from the finance team touches only that.

**Still assumed** until the finance team answers:

- **Q3 New invoices: open or held** until someone releases them for payment? *Assumed: open (D11).*
- **Q4 One cheque per vendor per run, or per invoice?** *Assumed: per vendor and bank (D9).*
- **Q5 The deployed AP test data** (9 invoices, 1 cash voucher, 3 cheques from March 1995): clear it before go-live? Its effect is already in the GL balances. *Assumed: yes, as Spec #4 Q4.*
- **Q6 Cheque stock:** pre-printed forms (which layout?) or plain paper? *Assumed: plain paper with the report layout of §6.2.*
- **Q7 Partial payments and credit notes:** needed? *Assumed: no (§1.2).*
