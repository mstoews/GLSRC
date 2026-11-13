# Spec #6 acceptance sign-off

This sheet records plan Milestone 7 (`gl/docs/superpowers/plans/2026-11-13-owl-to-win32-sdk-port-spec6-plan.md`, "Milestone 7"). It covers the manual checks in spec §10 (`2026-11-13-owl-to-win32-sdk-port-spec6-design.md`).

Fill in every blank (`____`) and set each **Result** to `PASS`, `FAIL` or `PASS with known gap`. A `FAIL` needs a triage line in [Gaps and triage](#gaps-and-triage): either fixed in Spec #6, or filed against a later spec.

**Which copy of `dev.db`:** criteria #1–#7 run in order on one fresh copy (`gl\test\dev-fixture\setup.cmd Release`); each step builds on the one before. #8 runs on the same copy after #7. Keep an untouched copy so the walkthrough can be repeated.

**The figures** below come from `gl_tests` ("The Spec #6 acceptance walkthrough's figures" in `gl_tests/test_ar.cpp`), which makes the same entries in the same order through the functions the screens call, and checks every `sqlite3` result quoted here. If a figure on screen differs, the test says which one is right.

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
| `x64\<Config>\gl_tests.exe` (163 test cases pass; `[ar]` 22 of them) | ____ | ____ | |
| `x64\<Config>\report_tests.exe` (17 test cases pass) | ____ | ____ | |
| `gl.exe /checkres /quiet` (now also 1052, 1063, 1053, 1060 with its menu, 1054, 1156, 1155 and 1062) | ____ | ____ | |
| `gl.exe /stress /quiet` (now also every AR screen and the 15 AR report previews) | ____ | ____ | |
| `trivial_dialog.exe /selftest /quiet` | ____ | ____ | |
| Debug `gl.exe` under the debugger: open and close each AR screen, then exit. The Output window shows no "Detected memory leaks!" | ____ | n/a | |

### P3. Fresh-machine install

As in the Spec #3 sheet (P3), with the current `dev.db`. The fixture now includes the AR tables (`AR_CUST`, `AR_BTCH`, `AR_BTCHD`, `AR_DIST`, `AR_JSUM`, `AR_LINEX`, `AR_RCPT`).

| Check | Result | Notes |
|---|---|---|
| `gl.exe` starts with no Visual C++ runtime installed; login as `ADMIN` / `ADMIN` works | ____ | |
| `sqlite3 dev.db "select count(*) from ar_cust where _deleted=0"` returns 3 | ____ | |

## The fixture

The Spec #3 company, current period 7 ("Oct 26", ending October 31, 2026); period 6 is closed, period 8 open. Accounts (Company Profile > Account Setup): AR 111101, sales 410001 (no dept: the chart has 410001-10 and 410001-20), output VAT 260007 (VATP); cash accounts CHCK 102101 (checking) and CA 101000 (petty cash). Prices include VAT.

- **Customers:** C001 City General Hospital (CITYHOSP, VAT 10 %, terms 30, sales 410001-10); C002 Reyes, Ana Dra. (VAT 10 %, terms 30, sales 410001-20); C003 Northside Pharmacy (no VAT, cash, sales 410001-10). All AR 111101, VAT 260007.
- **Sales vouchers:**

  | No. | State | Customer | Invoice | Total | Due | Note |
  |---|---|---|---|---|---|---|
  | 1 | open | C001 | S-1001 | 3,300.00 | 11/21/26 | 3 × 1,100.00: sales 3,000.00 + VAT 300.00 |
  | 2 | open | C002 | S-1002 | 1,320.00 | 11/22/26 | 2 × 550.00 and 220.00 typed |
  | 3 | posted | C001 | S-1003 | 2,200.00 | 11/04/26 | owed in full |
  | 4 | posted | C002 | S-1004 | 1,100.00 | 11/07/26 | receipt 1, 500.00 on 10/20/26: 600.00 owed |
  | 5 | posted | C003 | S-1005 | 800.00 | 10/02/26 | receipt 2, 800.00 on 10/15/26: settled |
  | 6 | cancelled | C001 | S-1006 | 550.00 | | |

- **AR's journal:** 12 rows waiting for the General Ledger (vouchers 3, 4 and 5, and receipts 1 and 2). No GL balance includes them yet. AP's journal has 7 rows waiting (the Spec #5 fixture).
- **The General Ledger:** voucher 100 is posted, 101–103 are open; the next voucher is 104. Before the walkthrough 111101 is 8,200.00 Dr, 102101 12,750.00 Dr, 260007 450.00 Cr, 410001-10 and 410001-20 0.00.

## The criteria (spec §10)

Log in as `ADMIN`.

### 1. Customers

- [ ] **Accounts > Sales Customers**, **New**. Type number `C004`, short name `EASTCLINIC`, name "East Clinic", address "5 East Road"; **Add**. It is refused: the default sales account (410001, no dept) isn't in the chart, "Set it under Options.", and **Options** opens with AR 111101, sales 410001 and VAT 260007 filled in.
- [ ] In Options, sales dept `20`, VAT 10, terms 15; **OK**. **Add**: the customer is saved and the fields clear.
- [ ] Type `C004` and press **Enter**: the customer shows. **Options**, terms 45, **OK**: "The current customer has already been added. Do you want to modify the entry?" **Yes**. `C004` + **Enter**, **Options**: terms 45.
- [ ] Type `C001`, **Delete**, **Yes**: "Customer C001 can't be deleted: sales voucher 1 is theirs."
- [ ] Type `C004`, **Delete**, **Yes**: deleted. `sqlite3 dev.db "select count(*) from ar_cust where _deleted=0"` returns `3`.
- [ ] Type `C999`, **Find**: "Customer number not found."

**Result:** ____ **Notes:** ____

### 2. Invoice entry

- [ ] **Transactions > Sales Invoice Vouchers**: the voucher number is 7. Type customer `C001` + **Enter**: CITYHOSP, AR 111101, sales 410001-10, VAT 260007, VAT 10, terms 30 fill in. Invoice `S-2001`, document "Saline, 3 cases", invoice date 10/20/26: due date 11/19/26. Period 7.
- [ ] Quantity 3, unit price 1,100.00; **Add**. The Distribution List shows Cr 410001-10 3,000.00, Cr 260007 300.00, Dr 111101 3,300.00 (spec §10). The invoice total shows 3,300.00.
- [ ] A second line: amount 220.00, no price; **Add**. The lines are recomputed: Cr 410001-10 3,000.00 and 200.00, Cr 260007 320.00, Dr 111101 3,520.00.
- [ ] Double-click the 200.00 line, **Delete**, **Yes**: back to the three lines of the first step.
- [ ] `sqlite3 dev.db "select payable, vatamt, duedate from ar_btch where batchno=7"` returns `3300.0|300.0|20261119`.
- [ ] **New**, **Yes** to keep voucher 7. Customer `C001`, invoice `S-1001`, document "x", amount 100.00; **Add**: "Invoice number has been used previously. Customer C001's invoice S-1001 is on sales voucher 1." Invoice `S-1003`: the same, on sales voucher 3 (posted). Nothing is saved.
- [ ] **Memo** on voucher 7 (**Top**, then **Next** to 7): type a note, **File > Save**. **Close**; voucher 7 is still open.

**Result:** ____ **Notes:** ____

### 3. Post, Cancel, Recover

- [ ] **Transactions > Cancel Voucher > Sales Invoice Voucher**: double-click voucher 2 and confirm. `sqlite3 dev.db "select trim(mode) from ar_btch where batchno=2"` returns `X`.
- [ ] **Transactions > Post Voucher > Sales Voucher** lists vouchers 1 and 7. **Post**, **Yes**: "2 sales voucher(s) posted."
- [ ] **Transactions > Recover Sales Voucher**: double-click voucher 7 and confirm. It reopens.
- [ ] **Post Voucher > Sales Voucher**: double-click voucher 7, **Yes**.
- [ ] `sqlite3 dev.db "select batchno, trim(paymentctl), remainder from ar_dist where _deleted=0 order by batchno"` returns `1|O|3300.0`, `3|O|2200.0`, `4|R|600.0`, `5|C|0.0`, `7|O|3300.0`.
- [ ] `sqlite3 dev.db "select trim(mode), count(*) from ar_jsum where _deleted=0 and batchno=7 and trim(type)='AR' group by 1"` returns `1|3`.
- [ ] **Recover Sales Voucher** on voucher 4: "A receipt has been entered against voucher 4's invoice: it can't be recovered."

**Result:** ____ **Notes:** ____

### 4. Transfer to the General Ledger

- [ ] **Maintenance > Update AR and AP**: both boxes ticked; "Waiting for the General Ledger: 7 AP and 18 AR journal row(s)." **OK**: "Accounts Payable: Period 7: GL voucher 104, ...", "Accounts Receivable: Period 7: GL voucher 105, 5 line(s), 10,700.00 each way."
- [ ] **Transactions > Post Voucher > General Ledger Voucher**: double-click voucher 105 and confirm.
- [ ] `sqlite3 dev.db "select date, period, debit, credit, trim(mode), trim(reference) from gl_btch where batchno=105"` returns `20261031|7|10700.0|10700.0|0|Accounts Receivable GL Update`.
- [ ] `sqlite3 dev.db "select trim(mode), count(*) from ar_jsum where _deleted=0 group by 1"` returns `2|18`.
- [ ] `sqlite3 dev.db "select batchno, trim(mode) from ar_btch where _deleted=0 order by batchno"` returns `1|2`, `2|X`, `3|2`, `4|2`, `5|2`, `6|X`, `7|2`: the cancelled stay cancelled.
- [ ] **Reporting > General Ledger > Trial Balance**, period 7:

  | Account | Before | After |
  |---|---|---|
  | 111101 | 8,200.00 Dr | 17,600.00 Dr (+ the five invoices, 10,700.00, less receipts 1 and 2, 1,300.00) |
  | 102101 | 12,750.00 Dr | 14,050.00 Dr (+ receipts 1 and 2) |
  | 260007 | 450.00 Cr | 1,350.00 Cr |
  | 410001-10 | 0.00 | 8,800.00 Cr |
  | 410001-20 | 0.00 | 1,000.00 Cr |

- [ ] **Recover Sales Voucher** on voucher 7: refused, its journal has been transferred. **Update AR and AP** again: "There is nothing to transfer." for both.

**Result:** ____ **Notes:** ____

### 5. Receipts

On **Transactions > Clear Receipts**, double-click the invoice, choose the bank **CHCK - Checking Account** (102101), type the date and amount, then **Clear**.

- [ ] Invoice S-1001 (voucher 1): 3,300.00 on 10/28/26: settled. The status line: "Receipt 3 recorded for invoice S-1001: settled."
- [ ] Invoice S-2001 (voucher 7): 1,000.00 on 10/29/26 with **Partial** ticked; then 500.00 on 10/30/26, no box ticked: "Total entered is less than the amount due. Do you want to record the payment as a discount?" **No**. `sqlite3 dev.db "select amountpaid, remainder, trim(paymentctl) from ar_dist where batchno=7"` returns `1500.0|1800.0|R`.
- [ ] Invoice S-1003 (voucher 3): 2,090.00 on 10/29/26 with **Discount** ticked: settled, "with a discount of 110.00". `sqlite3 dev.db "select trim(account), trim(dept), debit, credit from ar_jsum where _deleted=0 and trim(type)='RC' and trim(cheque)='6' order by _rowid"` returns `102101||2090.0|0.0`, `410001|10|100.0|0.0`, `260007||10.0|0.0`, `111101||0.0|2200.0`.
- [ ] Invoice S-1004 (voucher 4): 700.00: "Total entered is greater than the amount due." **Close**.

**Result:** ____ **Notes:** ____

### 6. Reversing receipts

Pick the invoice's row in the list, then **Receipts**: the list of its receipts asks to reverse the latest. **The reversal is dated today**, in today's period. The figures below are for a date in period 8 (11/01/26–11/30/26), as `gl_tests` runs it (11/14/26). Note today's date: ____. If it is in period 7, the second transfer makes one voucher of 4 lines, 6,000.00; if it is in no open period, the reversal of a transferred receipt is refused ("today isn't in an open period").

- [ ] Invoice S-2001: receipts 4 (1,000.00) and 5 (500.00); reverse receipt 5 (waiting): its rows are removed. `sqlite3 dev.db "select amountpaid, remainder, trim(paymentctl), datepaid from ar_dist where batchno=7"` returns `1000.0|2300.0|R|20261029`.
- [ ] **View All**, invoice S-1004: receipt 1 (500.00, transferred in #4); reverse it: new rows reverse it, dated today, and the invoice is owed 1,100.00 again (O). `sqlite3 dev.db "select trim(account), debit, credit, trim(period), trim(mode) from ar_jsum where _deleted=0 and batchno=4 and trim(type)='RC' order by _rowid"` returns `102101|500.0|0.0|7|2`, `111101|0.0|500.0|7|2`, `102101|0.0|500.0|8|1`, `111101|500.0|0.0|8|1`.
- [ ] `sqlite3 dev.db "select rcptno, batchno, amount, discount from ar_rcpt where _deleted=0 order by rcptno"` returns `2|5|800.0|0.0`, `3|1|3300.0|0.0`, `4|7|1000.0|0.0`, `6|3|2090.0|110.0`.
- [ ] **Update AR and AP**: "Accounts Receivable: Period 7: GL voucher 106, 4 line(s), 6,500.00 each way. Period 8: GL voucher 107, 2 line(s), 500.00 each way." Post both (**Post Voucher > General Ledger Voucher**, double-click each).
- [ ] **Trial Balance**, period 8 (the reversal's period): 111101 11,600.00 Dr, the fixture's 8,200.00 plus the 3,400.00 owed (#8); 102101 19,940.00 Dr.

**Result:** ____ **Notes:** ____

### 7. Purge

- [ ] **Maintenance > Purge Received Receivables**, "Received before:" 10/28/26: voucher 5 only. 10/29/26: vouchers 1 and 5.
- [ ] **Purge**, **Yes**: "2 invoice(s) and 2 receipt(s) purged."
- [ ] `sqlite3 dev.db "select batchno, trim(paymentctl), remainder from ar_dist where _deleted=0 order by batchno"` returns `3|C|0.0`, `4|O|1100.0`, `7|R|2300.0`. Vouchers 1 and 5 and their journal rows are still there.

**Result:** ____ **Notes:** ____

### 8. Reports

Each previews; print at least the landscape ones to PDF and check that no column is cut (`gl_tests` checks it on A4). Owed after #7: voucher 4, 1,100.00, and voucher 7, 2,300.00: 3,400.00.

- [ ] **Reporting > Accounts Receivable:** Customer List (3), **Customer Summary** (invoiced 6,600.00 = received 3,090.00 + discounts 110.00 + owed 3,400.00), Open Receivables Detail (3,400.00), Accounts Receivable Register, Voucher Status, Sales Voucher Detail, Receipts Register (receipts 4 and 6), Aged Receivables 30-60-90, Collections Forecast, VAT: General Ledger, VAT: Sales Vouchers, AR Journal Summary ("No journal rows." after #6's transfer), AR Journal Detail, Customer Statements (two pages: C001, C002).
- [ ] **Aged Receivables 30-60-90** and **Collections Forecast**: the total is 3,400.00, and the four buckets add up to it. The buckets depend on the day it is run ("As at"): note them. ____
- [ ] **Transactions > Voucher Reports > Sales Pre-Posting report**: "No vouchers."
- [ ] **File > Print** with no preview open: the report list holds 55 reports, the AR ones from 41 to 55; each runs from there.

**Result:** ____ **Notes:** ____

### 9. Regression and resources

- [ ] The Spec #1–#5 sheets' criteria still pass, on a fresh copy of `dev.db`. (The Spec #5 sheet's #5 now shows "7 AP and 12 AR journal row(s)" and makes AR's GL voucher too.)
- [ ] The menus: Reporting > Accounts Receivable names each report it runs, with no duplicate; Maintenance has Purge Received Receivables beside Purge Paid Payables; Transactions has Recover Sales Voucher and no Purge Cleared Accounts.
- [ ] `/checkres /quiet` and `/stress /quiet` exit 0 (P2).
- [ ] No leaks in Debug (P2).

**Result:** ____ **Notes:** ____

## Known gaps accepted before the walkthrough

| # | Gap | Why accepted | Follow-up |
|---|---|---|---|
| G1 | The finance team hasn't answered Q4–Q8 (spec §13). The build assumes no credit notes or returns, no printed sales invoice, statements of open items on demand, no credit-limit check, and that the deployed customers keep their accounts. Q3 (no withholding on receipts) is the user's answer, still to confirm with them. | AR was never run (spec §2.7); each answer touches one function. | Settle them with the finance team. |
| G2 | The fixture's `GL_SETAT` SALE account (410001) has no dept, so a new customer needs one typed (#1). The deployed company's SALE is its own. | Fixture data, not code. | Check SALE on the deployed company before go-live. |
| G3 | A new receipt's bank defaults to the first cash account (petty cash), unless the invoice has an earlier receipt. | The original listed the cash accounts the same way, with none chosen. | If the finance team wants the checking account by default, a one-line change. |
| G4 | High DPI and real printers aren't tested. | As in the Spec #3 sheet (G1, G3). | |
| G5 | The Linux ASan/UBSan build hasn't been run for Spec #6. | No Linux machine this round. | Run `tools/linux/build_tests.sh --run`. |
| G6 | **Nothing has run against the deployed company's AR tables.** Its customer master is real; its transaction tables are empty (spec §2.7). | The plan forbids it before go-live is agreed. | A go-live checklist step: back up, then open Sales Customers on a copy first. |

## Gaps and triage

Every `FAIL` above gets a line here.

| # | Criterion | What happened | Decision (fix in Spec #6 / file for Spec #N) | Fixed in commit / issue |
|---|---|---|---|---|
| | | | | |

## Sign-off

| Criterion | Result |
|---|---|
| 1. Customers | ____ |
| 2. Invoice entry | ____ |
| 3. Post, Cancel, Recover | ____ |
| 4. Transfer to the General Ledger | ____ |
| 5. Receipts | ____ |
| 6. Reversing receipts | ____ |
| 7. Purge | ____ |
| 8. Reports | ____ |
| 9. Regression and resources | ____ |

All criteria pass, or pass with the known gaps recorded above:

- Tester: ____ Date: ____
- Witness: ____ Date: ____
