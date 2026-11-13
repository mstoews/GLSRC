# Spec #5 acceptance sign-off

This sheet records plan Milestone 8 (`gl/docs/superpowers/plans/2026-11-13-owl-to-win32-sdk-port-spec5-plan.md`, "Milestone 8"). It covers the 11 manual checks in spec §10 (`2026-11-11-owl-to-win32-sdk-port-spec5-design.md`).

Fill in every blank (`____`) and set each **Result** to `PASS`, `FAIL` or `PASS with known gap`. A `FAIL` needs a triage line in [Gaps and triage](#gaps-and-triage): either fixed in Spec #5, or filed against a later spec.

**Which copy of `dev.db`:** criteria #1–#9 run in order on one fresh copy (`gl\test\dev-fixture\setup.cmd Release`); each step builds on the one before. #10 runs on the same copy after #9. Keep an untouched copy so the walkthrough can be repeated.

**The figures** below come from `gl_tests` ("The Spec #5 acceptance walkthrough's figures" in `gl_tests/test_ap.cpp`), which makes the same entries in the same order through the functions the screens call, and checks every `sqlite3` result quoted here. If a figure on screen differs, the test says which one is right.

Dates are typed as `mm/dd/yy`. Run the `sqlite3` queries from `x64\Release`, against `dev.db`, with the program closed or between screens.

## Run details

| Item | Value |
|---|---|
| Commit tested (`git rev-parse --short HEAD`) | ____ |
| Build machine | ____ |
| Test machine (fresh install, not the dev box) | ____ |
| Windows edition and build (`winver`) | ____ (target: Windows 11 23H2 or later, x64) |
| Display scaling | ____ (target: 100% / 96 DPI) |
| Printers installed | ____ (at least *Microsoft Print to PDF*) |
| Tester | ____ |
| Witness | ____ |
| Date | ____ |

## Before the walkthrough

### P1. Release build and hardening

As in the Spec #3 sheet (P1): build `gl.sln` `Release|x64` with `/W4 /WX`, then confirm the hardening options from `common.props`, or check them with `dumpbin /headers x64\Release\gl.exe`.

| Check | Result | Notes |
|---|---|---|
| Release build, no warnings | ____ | |
| `/sdl`, `/guard:cf`; `/DYNAMICBASE`, `/NXCOMPAT`, `/GUARD:CF` | ____ | |

### P2. Automated checks (dev machine)

Run `gl\test\dev-fixture\setup.cmd` for each configuration first. From `cmd`, run each check with `start /wait ... /quiet` where it takes `/quiet`, then `echo %errorlevel%`. Each should print 0.

| Check | Debug | Release | Notes |
|---|---|---|---|
| `x64\<Config>\cb4sqlite_tests.exe` (77 test cases pass) | ____ | ____ | |
| `x64\<Config>\gl_tests.exe` (141 test cases pass; `[ap]` 27 of them) | ____ | ____ | |
| `x64\<Config>\report_tests.exe` (17 test cases pass) | ____ | ____ | |
| `gl.exe /checkres /quiet` (now also 1041, 1042, 1044, 1047, 1043, 1015, 1030, 1045, 1066, 1046, 1051, 1048 and `IDD_AP_CONFIRM`) | ____ | ____ | |
| `gl.exe /stress /quiet` (now also every AP screen and the 23 AP report previews) | ____ | ____ | |
| `trivial_dialog.exe /selftest /quiet` | ____ | ____ | |
| Debug `gl.exe` under the debugger: open and close each AP screen, then exit. The Output window shows no "Detected memory leaks!" | ____ | n/a | |

### P3. Fresh-machine install

As in the Spec #3 sheet (P3), with the current `dev.db`. The fixture now includes the AP tables (`AP_VEND`, `AP_BTCH`, `AP_BTCHD`, `AP_CASH`, `AP_CASHD`, `AP_DIST`, `AP_REQST`, `AP_JSUM`, `AP_LINEX`).

| Check | Result | Notes |
|---|---|---|
| `gl.exe` starts with no Visual C++ runtime installed; login as `ADMIN` / `ADMIN` works | ____ | |
| `sqlite3 dev.db "select count(*) from ap_vend where _deleted=0"` returns 3 | ____ | |

## The fixture

The Spec #3 company, current period 7 ("Oct 26", ending October 31, 2026); period 6 is closed. Accounts: AP 221002, input VAT 148000, W/H payable 260004, the bank 102101 (checking). AP runs in F mode (`GL.INI [WITHHOLDING] TAX=F`): amounts include VAT, and W/H is withheld from the payment, on the amount before VAT.

- **Vendors:** V001 Office Supplies Co. (company, VAT 12 %, W/H 1 %, terms 30); V002 J. Santos (consultant) (individual, W/H 2 %, terms 15); V003 City Landlord Inc. (no VAT or W/H, terms 30).
- **Invoice vouchers:**

  | No. | State | Vendor | Invoice | Amount | Due | Note |
  |---|---|---|---|---|---|---|
  | 1 | open | V001 | INV-1001 | 1,120.00 | 11/04/26 | 1,000.00 + VAT 120.00; W/H 10.00; AP 1,110.00 |
  | 2 | open | V001 | INV-1002 | 784.00 | 11/11/26 | two lines; AP 777.00 |
  | 3 | posted | V002 | INV-2001 | 5,000.00 | 10/16/26 | W/H 100.00; owes 4,900.00 |
  | 4 | cancelled | V003 | INV-3001 | 1,500.00 | | |
  | 5 | cancelled | V003 | INV-3000 | | | the original's cancel, migrated |
  | 6 | posted | V003 | INV-3002 | 1,200.00 | 11/01/26 | paid by cheque 5001, printed, not cleared |

- **Cash voucher 1:** open, V001 INV-1003, 336.00, bank 102101, cheque 000123.
- **AP's journal:** 7 rows waiting for the General Ledger (invoices 3 and 6, and cheque 5001's payment). No GL balance includes them yet.
- **The General Ledger:** voucher 100 is posted, 101–103 are open; the next voucher is 104. Before the walkthrough 221002 is 4,100.00 Cr, 102101 12,750.00 Dr, 148000 310.00 Dr, 260004 120.00 Cr, 501000 6,400.00 Dr, 751000 1,200.00 Dr and 731000-20 1,500.00 Dr.

## The 11 criteria (spec §10)

Log in as `ADMIN`.

### 1. Vendors

- [ ] **Accounts > Purchase Vendors**, **New**: the AP account (221002) and VAT account (148000) fill in. Type number `V004`, short name `STATIONERY`, name "Stationery Mart", Company, VAT % 12, W/H % 1, terms 15; **Add**. The fields clear.
- [ ] Type `V004` and press **Enter**: the vendor shows. Change the terms to 30; **Add**, **Yes** to modify. `V004` + **Enter** shows terms 30.
- [ ] Type `V001`, **Delete**, **Yes**: "Vendor V001 can't be deleted: invoice voucher 1 is theirs."
- [ ] Type `V004`, **Delete**, **Yes**: deleted. **Search** lists V001–V003.
- [ ] Type `V999` + **Enter**: "There is no vendor V999."

**Result:** ____ **Notes:** ____

### 2. Invoice entry

- [ ] **Transactions > Purchase Invoice Vouchers**, **New** (the voucher is numbered 7 at its first **Add**). Type vendor `V001` + **Enter**: the name, accounts, rates (12 / 1) and terms 30 fill in. Invoice `INV-1004`, document "Toner", invoice date 10/20/26: the due date becomes 11/19/26. Period 7.
- [ ] Line: amount 1,120.00, account 501000; **Add**. The Distribution List shows Dr 501000 1,000.00, Dr 148000 120.00, Cr 260004 10.00, Cr 221002 1,110.00 (W/H on the net, Q2).
- [ ] A second line: amount 560.00, account 751000; **Add**. The lines are recomputed: Dr 501000 1,000.00, Dr 751000 500.00, Dr 148000 180.00, Cr 260004 15.00, Cr 221002 1,665.00. Debits and credits are both 1,680.00.
- [ ] Double-click the 751000 line, **Delete**: back to the four lines of the first step (Cr 221002 1,110.00).
- [ ] `sqlite3 dev.db "select payable, vatamt, rebateamt, duedate from ap_btch where batchno=7"` returns `1120.0|120.0|-10.0|20261119`.
- [ ] **New**, **Yes** to keep voucher 7. Vendor `V001`, invoice `INV-1001`, a line of 100.00 on 501000; **Add**: "Invoice was entered previously. Vendor V001's invoice INV-1001 is on invoice voucher 1." Nothing is saved.
- [ ] **Close**. Voucher 7 is still open, with its four lines.

**Result:** ____ **Notes:** ____

### 3. Post, Cancel, Recover

- [ ] **Transactions > Cancel Voucher > Purchase Vouchers**: double-click voucher 2 and confirm. `sqlite3 dev.db "select trim(mode) from ap_btch where batchno=2"` returns `X`.
- [ ] **Transactions > Post Voucher > Purchase Vouchers** lists vouchers 1 and 7. **OK**: "2 voucher(s) posted."
- [ ] **Transactions > Recover Purchase Voucher**: double-click voucher 7 and confirm. It reopens: voucher 7 shows in Purchase Invoice Vouchers again, and its open item and journal are gone.
- [ ] **Post Voucher > Purchase Vouchers**: double-click voucher 7 and confirm: "1 voucher(s) posted."
- [ ] `sqlite3 dev.db "select batchno, trim(paymentctl) from ap_dist where _deleted=0 order by batchno"` returns `1|O`, `3|O`, `6|V`, `7|O`.
- [ ] `sqlite3 dev.db "select trim(mode), count(*) from ap_jsum where _deleted=0 and batchno=7 and trim(type)='AP' group by 1"` returns `1|4`: four rows waiting for the General Ledger.
- [ ] **Recover Purchase Voucher** on voucher 6: "Voucher 6's invoice is in a payment: it can't be recovered."

**Result:** ____ **Notes:** ____

### 4. Cash voucher

- [ ] **Transactions > Cash Invoice Vouchers**, **New** (it becomes voucher 2). Vendor `V003` + **Enter**, invoice `INV-3003`, document "Parking", invoice date 10/21/26, period 7, Bank Account 102101, cheque `000124`. Line: amount 250.00, account 751000; **Add**. The lines are Dr 751000 250.00, Cr 102101 250.00 (V003 has no VAT or W/H). **Close**, **Yes** to keep it.
- [ ] **Transactions > Cancel Voucher > Cash Invoice Vouchers**: double-click voucher 1 and confirm.
- [ ] **Post Voucher > Cash Vouchers** lists voucher 2 only. **OK**: "1 voucher(s) posted."
- [ ] `sqlite3 dev.db "select batchno, trim(mode) from ap_cash where _deleted=0 order by batchno"` returns `1|X`, `2|0`.
- [ ] `sqlite3 dev.db "select trim(account), debit, credit, trim(cheque) from ap_jsum where _deleted=0 and trim(type)='CA' order by _rowid"` returns `751000|250.0|0.0|000124`, `102101|0.0|250.0|000124`. No open item is made: nothing is owed.

**Result:** ____ **Notes:** ____

### 5. Transfer to the General Ledger

- [ ] **Maintenance > Update AR and AP**: "17 AP journal row(s) waiting for the General Ledger." **OK**: "Period 7: GL voucher 104, 7 line(s), 8,690.00 each way."
- [ ] **Transactions > Post Voucher > General Ledger Voucher**: double-click voucher 104 and confirm.
- [ ] `sqlite3 dev.db "select date, period, debit, credit, trim(mode), trim(reference) from gl_btch where batchno=104"` returns `20261031|7|8690.0|8690.0|0|Accounts Payable GL Update`.
- [ ] `sqlite3 dev.db "select trim(mode), count(*) from ap_jsum where _deleted=0 group by 1"` returns `2|17`: every row transferred.
- [ ] **Reporting > General Ledger > Trial Balance**, period 7. Each account moves by the vouchers:

  | Account | Before | After |
  |---|---|---|
  | 221002 | 4,100.00 Cr | 11,220.00 Cr (+ invoices 1, 3 and 7: 1,110.00 + 4,900.00 + 1,110.00; invoice 6's 1,200.00 came and went) |
  | 102101 | 12,750.00 Dr | 11,300.00 Dr (− cheque 5001's 1,200.00 and cash voucher 2's 250.00) |
  | 148000 | 310.00 Dr | 550.00 Dr |
  | 260004 | 120.00 Cr | 240.00 Cr |
  | 501000 | 6,400.00 Dr | 8,400.00 Dr |
  | 751000 | 1,200.00 Dr | 2,650.00 Dr |
  | 731000-20 | 1,500.00 Dr | 6,500.00 Dr |

- [ ] **Recover Purchase Voucher** on voucher 3: refused, its journal has been transferred.
- [ ] **Update AR and AP** again: "There is nothing to transfer."

**Result:** ____ **Notes:** ____

### 6. Payments

- [ ] **Receipts/Payments > Create Cheques**, Due Date 11/05/26, **Set Date**: invoices 1 (due 11/04) and 3 (due 10/16) are listed, "2 invoice(s) due by 11/05/2026, or forced: 6,010.00 owed."
- [ ] Double-click invoice 3: in Hold / Force tick **Hold**, **OK**. **View All**, double-click invoice 7 (due 11/19): tick **Force**, **OK**. Double-click invoice 6: "Only unrequested invoices may be modified." (it is in a payment). **Set Date** again: invoices 1, 3 (held) and 7 (forced).
- [ ] Pay Date 10/30/26, bank 102101, Starting Cheque No 5002 (filled in); **Create**: "Cheque 5002  V001 Office Supplies Co.  2,220.00  (2 invoice(s))", then "1 cheque(s), 2,220.00, in period 7." Invoice 3 is held, so V002 gets no cheque.
- [ ] `sqlite3 dev.db "select cheque, trim(vendorno), amountpaid, datepaid, trim(cleared) from ap_reqst where _deleted=0 order by cheque"` returns `5001|V003|1200.0|20261015|` and `5002|V001|2220.0|20261030|1`.
- [ ] `sqlite3 dev.db "select batchno, trim(paymentctl), chequeno from ap_dist where _deleted=0 order by batchno"` returns `1|P|5002`, `3|H|0`, `6|V|5001`, `7|P|5002`.
- [ ] AP's journal has the payment: Dr 221002 / Cr 102101, 1,110.00 for each invoice, dated 10/30/26, period 7, cheque 5002 (Reporting > Accounts Payable > **AP Journal Summary** shows 221002 2,220.00 Dr and 102101 2,220.00 Cr).

**Result:** ____ **Notes:** ____

### 7. Cheques

- [ ] **Receipts/Payments > Print Cheques**: one page, cheque 5002 dated 10/30/2026 to Office Supplies Co., 2,220.00, "Two thousand two hundred twenty and 00/100", bank 102101, and a stub of invoices 1 and 7. **Print** to *Microsoft Print to PDF*: the PDF matches the preview.
- [ ] Say it was spoiled. **Receipts/Payments > Confirm Printed Cheques** lists 5002. Select it, new number 5001, **Renumber**: "Cheque number 5001 has already been used." New number 5003, **Renumber**: "Cheque 5002 is now cheque 5003. Print it again."
- [ ] **Print Cheques** again: one page, cheque 5003.
- [ ] **Confirm Printed Cheques**, from 5003 to 5003, **Confirm**, **Yes**: "1 cheque(s) confirmed printed." Renumber is now refused for it.

**Result:** ____ **Notes:** ____

### 8. Bank reconciliation

- [ ] **Receipts/Payments > Bank Reconciliation** lists 5001 and 5003. Double-click 5003: "Clear or Void Cheque 5003". Date 10/31/26, **Clear**. It leaves the list.
- [ ] Double-click 5001, date 10/31/26, **Void**, **Yes**: "Cheque 5001 voided: 1 invoice(s) open again; 0 journal row(s) removed, 2 reversed." (Its payment went to the General Ledger in #5, so it is reversed, not removed.)
- [ ] `sqlite3 dev.db "select trim(account), debit, credit, date, trim(mode) from ap_jsum where _deleted=0 and trim(cheque)='5001' order by _rowid"` returns the payment and its reversal: `221002|1200.0|0.0|20261015|2`, `102101|0.0|1200.0|20261015|2`, `221002|0.0|1200.0|20261031|1`, `102101|1200.0|0.0|20261031|1`.
- [ ] `sqlite3 dev.db "select cheque, trim(cleared), trim(reference) from ap_reqst where _deleted=0 order by cheque"` returns `5003|0|Cleared 20261031` ("0" is cleared, as in the original).
- [ ] **Maintenance > Update AR and AP**: "Period 7: GL voucher 105, 2 line(s), 1,020.00 each way" (5003's payment, 2,220.00, less 5001's reversal, 1,200.00). Post voucher 105.
- [ ] **Trial Balance**, period 7: 221002 10,200.00 Cr (invoices 3 and 6 unpaid again), 102101 10,280.00 Dr.

**Result:** ____ **Notes:** ____

### 9. Purge

- [ ] **Maintenance > Purge Paid Payables**, "Cleared and paid before:" 10/30/26: "No cleared invoice was paid before that date." (5003's were paid on the 30th, not before.)
- [ ] Date 12/31/26: "2 cleared invoice(s) paid before 12/31/2026." **Purge**, **Yes**: "2 invoice(s) and 1 cheque(s) purged."
- [ ] `sqlite3 dev.db "select batchno, trim(paymentctl) from ap_dist where _deleted=0 order by batchno"` returns `3|H`, `6|O`: the held and the reopened invoices stay. `sqlite3 dev.db "select count(*) from ap_reqst where _deleted=0"` returns `0`.

**Result:** ____ **Notes:** ____

### 10. Reports

Each previews; print at least the landscape ones to PDF and check that no column is cut (`gl_tests` checks it on the fixture, on A4). Owed after #9: invoice 3, 4,900.00 (held), and invoice 6, 1,200.00 (open): 6,100.00.

- [ ] **Reporting > Accounts Payable:** Vendor List (3 vendors), Open Payables Detail (6,100.00), Accounts Payable Register, AP Journal Detail, AP Journal Summary (nothing waiting: "No journal rows."), Invoice Posting Journal (vouchers 1, 3, 6, 7), Voucher Status (period 7), Aged Payables 30-60-90, Cash Requirement, VAT: General Ledger, VAT: AP Vouchers, Withholding Tax, Cash Cheque Summary (cash voucher 2, 250.00), AP Voucher Detail, Cash Voucher Detail, AP Journal: Cash, AP Journal: Invoices and Payments.
- [ ] **Aged Payables 30-60-90** and **Cash Requirement**: the total is 6,100.00, and the four buckets add up to it. The buckets depend on the day it is run ("As at"): note them. ____
- [ ] **Transactions > Voucher Reports > Invoice Pre-Posting Report** and **Cash Pre-Posting Report**: "No vouchers."
- [ ] **Receipts/Payments:** Cheque Payment Detail ("No payments." after the purge), Pre-payment List (6,100.00), Print Cheques ("No cheques are waiting to be printed.").
- [ ] **File > Print** with no preview open: the report list holds 40 reports, the AP ones from 19 to 40; each runs from there.

**Result:** ____ **Notes:** ____

### 11. Regression and resources

- [ ] The Spec #1–#4 sheets' criteria still pass, on a fresh copy of `dev.db`. The report list now holds 40 reports (Spec #3 sheet #11, Spec #4 sheet #7).
- [ ] The menus: Reporting > Accounts Payable has no duplicate item; Receipts/Payments has one Print Cheques (4091) and no "Customer Sales Statements".
- [ ] `/checkres /quiet` and `/stress /quiet` exit 0 (P2).
- [ ] No leaks in Debug (P2).

**Result:** ____ **Notes:** ____

## Known gaps accepted before the walkthrough

| # | Gap | Why accepted | Follow-up |
|---|---|---|---|
| G1 | The finance team hasn't answered Q3–Q7 (spec §13). The build assumes new invoices open (not held), one cheque per vendor and bank, plain-paper cheques, no partial payments or credit notes, and that the deployed AP test data is cleared before go-live. | AP was never run in production (spec §2.7); each answer touches one function. | Settle them with the finance team. |
| G2 | **Before go-live, clear the deployed AP test data** (9 invoices, 1 cash voucher, 3 cheques from March 1995, Q5). | Its effect is already in the GL balances; transferring it again would count it twice. | A step in the go-live checklist. |
| G3 | Cheques print on plain paper with the report layout (Q6); pre-printed stock isn't supported. | Spec R3: positions can be set in twips if needed. | When the cheque stock is known. |
| G4 | High DPI and real printers aren't tested. | As in the Spec #3 sheet (G1, G3). | |
| G5 | The Linux ASan/UBSan build hasn't been run for Spec #5. | No Linux machine this round. | Run `tools/linux/build_tests.sh --run`. |
| G6 | The AP reports haven't been run on the deployed company's data. | Its AP data is the test data of G2. | After go-live. |

## Gaps and triage

Add one row for each `FAIL` or unexpected behaviour found during the walkthrough.

| # | Criterion | What happened | Decision (fix in Spec #5 / file for Spec #N) | Fixed in commit / issue |
|---|---|---|---|---|
| | | | | |

## Sign-off

| Criterion | Result |
|---|---|
| 1. Vendors | ____ |
| 2. Invoice entry | ____ |
| 3. Post, Cancel, Recover | ____ |
| 4. Cash voucher | ____ |
| 5. Transfer to the General Ledger | ____ |
| 6. Payments | ____ |
| 7. Cheques | ____ |
| 8. Bank reconciliation | ____ |
| 9. Purge | ____ |
| 10. Reports | ____ |
| 11. Regression and resources | ____ |

All criteria pass, or pass with the known gaps recorded above:

- Tester: ____ Date: ____
- Witness: ____ Date: ____
