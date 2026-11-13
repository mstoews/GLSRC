# gl.exe — the General Ledger port

The Win32 / MSVC x64 / Unicode port of the 1995 OWL General Ledger (spec: `gl/docs/superpowers/specs/2026-05-10-owl-to-win32-sdk-port-design.md`).
- **New code:** `gl/port/` (spec §2.4) and the pruned resources in `gl/res/`.
- **Legacy reference only:** `gl/src/`, `gl/header/`, `gl/gl.rc` and `gl/gl_new.rc` are not built.

## Build

Open `gl.sln` (repository root), choose `Debug|x64` or `Release|x64`, and build. Output goes to `x64\<Config>\`: `gl.exe`, `dbf2sqlite.exe`, `cb4sqlite_tests.exe`.

## First run: the dev fixture

`gl.exe` needs a company database. `gl/test/dev-fixture/` has a small made-up company with two users. From a command prompt in the repository root:

```
gl\test\dev-fixture\setup.cmd Debug
x64\Debug\gl.exe
```

`setup.cmd` migrates `gl\test\dev-fixture\dbf\` into `x64\Debug\dev.db` and copies `GLDR.INI` and `GL.INI` next to `gl.exe`. It replaces those three files if they already exist.

| User | Password | Level | Menu access |
|---|---|---|---|
| `ADMIN` | `ADMIN` | A | everything |
| `CLERK` | `clerk` | B | the Maintenance menu is grayed |

Company "Dev Fixture Company Ltd.", fiscal year April 2026 to March 2027, current period "Oct 26". The chart has 25 accounts. 340000 is the Retained Earnings account. 102101 and 101000 have period-7 activity (posted voucher 100, below), so the Chart of Accounts won't delete them. The deletable accounts are 144910 and 410001 depts 10 and 20.

For the Spec #2 voucher screens, the fixture also has:

| Voucher | State (`gl_btch.mode`) | Lines |
|---|---|---|
| 100 | posted (`0`), with its `gl_dist` rows | 102101 Dr 250, 101000 Cr 250 |
| 101 | open (`1`), balanced | 111101 Dr 1,150; 410001-10 Cr 600 and 410001-20 Cr 550, with medical reps, doctors and products |
| 102 | open (`1`), out of balance | 148000 Dr 100, 260007 Cr 90 |
| 103 | open adjustment (`3`) | 144910 Dr 75, 111101 Cr 75 |

- **Master data:** three medical reps (MR001–MR003), doctors (DR001–DR003) and products (P100, P200, P300).
- **Departments:** 10 and 20.
- **Archive:** `gl_distb` is empty.

For the Spec #3 reports (S3-M2), it also has a year of activity on 14 more accounts (bank 102201, equipment and depreciation, a loan, share capital, sales, cost of sales, salaries in depts 10 and 20, rent, interest, tax, and 999000 Suspense): twelve posted journals in periods 1–7 with their `gl_dist` lines, one period-6 movement with no lines (F2 marks it `*`), one deleted `gl_dist` line, and prior-year and budget amounts on the income accounts. `make_dev_fixture.py`'s `FINANCE_*` tables list them. Every account satisfies `curr_bal = openbal + period1..12`.
For Spec #5 (Accounts Payable, S5-M0) it also has three vendors (V001 a company with 12 % VAT and 1 % W/H, V002 an individual with 2 % W/H, V003 with neither), six invoice vouchers (1 and 2 open, 3 posted and unpaid, 4 cancelled, 5 cancelled the original's way, 6 posted and paid by cheque 5001, printed but not cleared), one open cash voucher, and AP's journal for them, not yet transferred to the GL, so no GL balance includes it. `make_dev_fixture.py`'s `accounts_payable()` lists the figures.
For Spec #6 (Accounts Receivable, S6-M0) it also has three customers (C001 a hospital with 10 % VAT and 30 days' terms, C002 a doctor with 10 % VAT, C003 a pharmacy with no VAT and cash terms), six sales vouchers (1 and 2 open, 3 posted and owed in full, 4 posted and partly received, 5 posted and settled, 6 cancelled), two receipts, and AR's journal for them, not yet transferred. `make_dev_fixture.py`'s `accounts_receivable()` lists the figures.

- **`GL.INI`:** `[PERIOD] BATCHNO=99`.

To regenerate the `.dbf` files after editing them, run `python3 gl/test/dev-fixture/make_dev_fixture.py`.

## A real company

Migrate the company's `.dbf` directory once (`dbf2sqlite_core/README.md`):

```
x64\Release\dbf2sqlite.exe --src D:\GL\DBF --out D:\GL\company.db
```

Then set `GLDR.INI` next to `gl.exe` to name the database file:

```
[GL_DIR]
DIR=D:\GL\company.db
```

## Settings files

`GLDR.INI` and `GL.INI` keep their original format and keys, with two changes:

- **Location.** They are read from the directory of `gl.exe`. The original read them from the Windows directory.
- **`[GL_DIR] DIR`.** It names the company **database file**, not the `.dbf` directory. A relative path is relative to `gl.exe`. `DRIVE` is ignored.

`gl.exe` refuses to start if `DIR` is any of these:
- a directory;
- a file that doesn't exist;
- a network path (UNC or mapped drive). SQLite isn't safe there until a later spec adds locking (spec risk 10).

| File | Section / key | Used for |
|---|---|---|
| `GLDR.INI` | `[GL_DIR] DIR` | the company database |
| `GL.INI` | `[ACCESS] PSW` | `T`: log in at startup. Anything else: no login, user `NOLOGIN`, no menu items grayed |
| `GL.INI` | `[WITHHOLDING] TAX` | Company Profile's "Withholding Tax Excluded" box (written on Save) |
| `GL.INI` | `[GLACCTS]` `LEFT` `TOP` `RIGHT` `BOTTOM` `FIELD1`–`FIELD3` `QUERY` | the account list's position (saved on close), its three columns (field numbers, default 1–3), and its row filter (default `.NOT. DELETED()`) |
| `GL.INI` | `[GLDEPT]`, `[GL_MEDRP]`, `[GL_DRCOD]`, `[GL_ITEMM]`, same keys | the same for the Department, Medical Rep, Doctor and Product lists |

## What runs (Specs #1 to #5)

- **Frame:**
  - the MANILA menu and the toolbar (hover over a button for its name);
  - a status bar along the bottom showing the company name, menu help and the current period;
  - right-click on the background for the GLSUB menu.
- **Login** (dialog 1158). **Maintenance > User Access** (1159). Its Menu button opens Menu Access (1160) with the menus of the level shown: double-click or Enter on a menu grays or enables it, and OK saves them as that level's menu code. Reset applies the level shown to the current session's menus.
- **File > Company Profile** (1021), with:
  - Periods (1028);
  - Accounts (1033) and its Cash button (1034, with the cash accounts list, 1011, beside it).
- **Accounts > Chart of Accounts** (1010), also GLSUB > Add Accounts:
  - **Add/Mod:** adds the typed account + dept, or updates it if it exists;
  - **Search:** shows the account list at the typed account;
  - **Set Data:** shows the typed account, or the next one after it;
  - **Delete:** removes an account with no period or previous-year balance.

  GLSUB > Delete Accounts opens the same dialog with Delete on the OK button.
- **The account list** beside it ("General Ledger Accounts List"):
  - double-click or Enter picks an account;
  - typing jumps to an account;
  - right-click edits the list query.

  Its position and query are kept in `GL.INI [GLACCTS]`. `QUERY` accepts `.NOT. DELETED()` (the default) and comparisons of a text field with a quoted value, joined with `.AND.`: for example `ACCOUNT = '4'` lists the accounts starting with 4.
- **Accounts > Department Codes** (1023, also GLSUB), **Medical Reps** (1006), **Doctor Codes** (1040) and **Product Codes** (1007):
  - Add/Mod adds the typed code, or updates it after asking.
  - Delete removes the typed code.
  - Medical Reps, Doctor Codes and Product Codes also have New and Top / Bottom / Next / Previous.
  - Each has a list beside it, like the account list. Search, or the Med Reps / Doctor / Products button, shows the list at the typed code, and picking a row shows that record. The lists keep their positions and queries in `GL.INI [GLDEPT]`, `[GL_MEDRP]`, `[GL_DRCOD]` and `[GL_ITEMM]`.
- **Transactions > General Ledger Voucher** (1013, also GLSUB and the toolbar's batch button) and **Transactions > Prior Period Entry** (the same dialog, for adjustment vouchers):
  - **Fields:** a voucher has a description, a date, a period (only open periods are offered), a cheque number and its lines.
  - **Add/Mod** (or Enter) saves the line shown into the voucher, creating the voucher with the number displayed if it's new. A line typed with an existing line number replaces that line.
  - **Delete** removes the line shown.
  - **New** starts the next voucher. For a voucher created in this session, New and Close first ask whether to keep it; No removes it.
  - **Top / Bottom / Next / Previous** move between vouchers not yet posted.
  - **Windows:** the Distribution List below the dialog shows the voucher's lines and totals; double-click a line to edit it. List shows the account list, Voucher the list of vouchers, and Show the floating tool box.
  - **Print** previews the voucher's slip in a window over the dialog; a voucher not yet saved gets a message.
  - The lists keep their positions in `GL.INI [GL_DISTLIST]`, `[GL_BTCH]` and `[GLACCTS]`.
- **Transactions > Post Voucher > General Ledger Voucher** (1061, also GLSUB and the toolbar's post button) lists the open and adjustment vouchers. **Post** posts them all; double-clicking one posts just that one. A voucher whose lines don't balance, or that uses an account not in the chart, is refused with a message and stays listed. Posting is all or nothing: if anything fails, nothing is posted.
- **Transactions > Recover General Ledger Voucher** (1015) lists the posted vouchers. Double-click one and confirm: its postings are reversed and it is reopened for editing.
- **Transactions > Cancel Voucher > General Ledger Voucher** (1043) lists the vouchers not yet posted. Double-click one and confirm to delete it.
- **Transactions > Voucher Reports > General Ledger Voucher Inquiry** (1029): type an account and dept, or pick one in the account list (Search shows and hides it), then Set Data. It lists the account's posted lines between its opening balance and its balance.
- **Accounts > Budget Data** (1017): the Chart of Accounts screen with a grid of the 12 months. Pick an account (Search, or Set Data on the account typed), edit the Budget column in the grid (Enter, F2 or a double-click edits a cell), then OK saves the 12 amounts.
- **Maintenance > Backup Databases** saves a copy of the company database to a file you choose, by default `<name>-<yyyymmdd>.db` beside it. **Restore Databases** replaces the company database with a backup you choose, after checking it is one and asking. The replaced file is kept as `<name>.db.bak`. If anything goes wrong, the original is put back.
- **Maintenance > Year End Consolidation** moves the income and expense balances into the Retained Earnings account and the period amounts into the previous-year columns. Since Spec #3 (D16) it also makes the closing balances the new opening balances, moves the year's posted lines to the archive (`gl_distb`), and closes the year's posted vouchers (mode 9), so Transaction Inquiry and Recover show only the new year. It runs only when exactly one account has type R; otherwise it names the type-R accounts so you can correct them in the Chart of Accounts. **Reindex Databases** explains that an SQLite company database needs no reindexing.
- **Fixed Assets > Asset Types** (4002): a type names the GL accounts for an asset's cost, its depreciation expense and its accumulated depreciation.
  - **Add** checks that each account (and dept) is in the chart, and modifies an existing type after asking.
  - **Delete** is refused while an asset uses the type.
  - Typing a type and pressing Enter shows it; **Types** shows the list.

- **Fixed Assets > Add/Modify** (4004): the asset register.
  - **Fields:** an asset has a code, type, description, reference, date purchased, cost, salvage value, life in months, months used and method (straight line, or declining balance at a yearly rate).
  - **Computed** as you type: the monthly depreciation, net book value and life in years.
  - **Accumulated depreciation** (and months used) can be typed until the monthly run has depreciated the asset. After that they are the run's.
  - **Lookup:** typing a code and pressing Enter shows the asset; Assets and Types show the lists.
  - **Disposed assets:** an asset that has been disposed of can't be changed.

- **Fixed Assets > Create Voucher** (4003): the month's depreciation, for the current period.
  - **Before OK**, the dialog says how many assets are due and the total. OK lists them, with the assets it will skip and why: disposed, fully depreciated, bought after the period, or a type that's missing or points at an account not in the chart.
  - **OK** writes one open voucher, dated the period's end, with two lines per asset (Dr depreciation expense, Cr accumulated depreciation). It moves each asset's accumulated depreciation and months used on.
  - **Post** the voucher with Transactions > Post Voucher. **To undo a run**, cancel its voucher (Transactions > Cancel Voucher): the assets go back.
  - **One run per period:** a second run is refused, naming the voucher.
  - **After year end**, set the new year's period dates (Company Profile > Periods) before the first run.

- **Fixed Assets > Disposal** (1001): marks an asset sold or scrapped.
  - **Enter** a code to show the asset, with its cost, accumulated depreciation and book value. **Type the sale price** to see the gain or loss.
  - **Dispose** records the date and keeps the asset in the register, marked disposed; the monthly run skips it from then on. Nothing is posted: the message lists the entry to make in Voucher Entry.
  - **Delete** removes an asset entered by mistake, if it has never been depreciated.

- **The monthly fixed-asset cycle:**
  1. Run Create Voucher for the period.
  2. Post its voucher with Transactions > Post Voucher.
  3. Print the Fixed Assets Listing: its check against the ledger should agree.

  To undo a run before posting, cancel its voucher. After disposing of an asset, record the entry Disposal gives in Voucher Entry, so the check agrees again.
- **Accounts > Purchase Vendors** (1041, Spec #5): the Accounts Payable vendors.
  - **Fields:** number, short name and name, address, phone and fax, contact, VAT number, VAT % and W/H %, company or individual, terms, discount, the vendor's AP account and VAT account, medical rep.
  - **New** fills in the default AP account (Company Profile > Account Setup, AP) and VAT account (VATR).
  - **Add** checks the required fields, the type (company or individual: it chooses the W/H account), the rates, and the accounts against the chart; it updates an existing vendor after asking.
  - **Delete** is refused while the vendor has vouchers, open items or cheques, naming one.
  - Typing a number and pressing Enter (or **Set Data**) shows the vendor; **Search** shows the list (`GL.INI [AP_VEND]`).
- **Transactions > Purchase Invoice Vouchers** (1042, Spec #5; also the toolbar's AP voucher button): a vendor's invoice.
  - **The header:** type the vendor and press Enter to fill in the name, the AP and VAT accounts, the rates, the terms and company or individual; the due date becomes the invoice date plus the terms (a due date you type is kept). Then the invoice number, the document (the voucher's reference) and the period.
  - **Each Add** saves an expense line: the amount as invoiced (VAT included), the account and dept, a description, and the medical rep, product and doctor. The voucher's VAT, withholding tax and AP lines are recomputed from all its expense lines, so it always balances. With `GL.INI [WITHHOLDING] TAX=F` (the default) VAT is taken out of the amount and W/H is withheld from the payment, on the amount before VAT.
  - **The Distribution List** shows every line. Double-click an expense line to change it (then Add) or Delete it; the computed lines can't be picked. Changing the rates and adding recomputes every line.
  - **The same vendor's invoice number** can't be entered twice (any voucher, invoice or cash, not cancelled).
  - **New and Close** ask whether to keep a voucher made in this session; No removes only that voucher.
  - Top / Bottom / Next / Previous go through the open vouchers; **Voucher** lists them all, **Vendors** the vendors, **Lists** the accounts.
- **Transactions > Cash Invoice Vouchers** (1044, also the toolbar's cash button): an invoice paid when it is entered. The same screen as purchase invoices, but the voucher names the **bank** it was paid from (the Bank Account box lists the cash accounts of Company Profile > Cash Accounts) and the cheque number; the bank is credited instead of AP. There is no due date. Post it with **Post Voucher > Cash Vouchers** and cancel it with **Cancel Voucher > Cash Invoice Vouchers**: posting writes the journal (with the cheque number) and no open item, since nothing is owed. Update AR and AP carries it to the General Ledger with the invoices.
- **Transactions > Post Voucher > Purchase Vouchers** (1047) lists the open invoice vouchers. **OK** posts them all; double-clicking one posts just that one. Posting writes the invoice's open item (ready to pay) and its journal for the General Ledger. A voucher out of balance by a cent, with no lines, or using an account not in the chart is refused with a message.
- **Transactions > Cancel Voucher > Purchase Vouchers** (1043) lists the open invoice vouchers. Double-click one and confirm: it is kept, marked cancelled.
- **Transactions > Recover Purchase Voucher** (1015) lists the posted invoice vouchers. Double-click one and confirm to reopen it for editing, removing its open item and journal. A voucher whose journal has gone to the General Ledger, or whose invoice is in a payment, can't be recovered.
- **Maintenance > Update AR and AP** (1030): AP's posted journal, then AR's, each becomes one open General Ledger voucher per period, dated the period's end, with its lines netted by account ("Accounts Payable GL Update", "Accounts Receivable GL Update"). Post them with Transactions > Post Voucher > General Ledger Voucher. A closed period's rows wait. To undo a transfer before posting, cancel its GL voucher: the rows wait again.
- **Receipts/Payments > Create Cheques** (1045): pays the invoices due.
  - The list shows the posted invoices due by the **Due Date**, and any forced, with what each still owes (the amount less the W/H withheld). **Set Date** refreshes it for another date; **View All** shows every open item, paid ones included, with its state (H held, O open, F forced, P cheque made, V printed, C cleared).
  - **Double-click an invoice** to hold it (it isn't paid until released), force it (paid even if not yet due) or open it again. An invoice already in a payment can't be changed.
  - **Create**, with the bank chosen and the **Starting Cheque No** (the next free one is filled in): one cheque per vendor for its open invoices due and its forced ones. Each invoice is marked paid with its cheque; the payment's GL entry (Dr AP, Cr the bank) goes in AP's journal, dated the **Pay Date**, in that date's period, which must be open. A cheque number already used is refused.
- **Receipts/Payments > Print Cheques** previews the cheques made and not yet printed, one a page: the cheque number and date, the payee and address, the amount in figures and words, the bank, and a stub of the invoices it pays. Print them from the preview, on plain paper.
- **Receipts/Payments > Confirm Printed Cheques** lists the cheques made and not yet confirmed. Enter the range that printed correctly and **Confirm**; they can't be renumbered after that. A spoiled cheque: select it, enter its new number and **Renumber** (the number moves to its invoices and journal), then print it again.
- **Receipts/Payments > Bank Reconciliation** lists the cheques not yet cleared. Double-click one: **Clear** it (printed cheques only) with the date it cleared, or **Void** it: its invoices are open again, and its payment is undone (its journal rows removed, or reversed with new rows dated the void if they have gone to the General Ledger).
- **Maintenance > Purge Paid Payables** deletes the cleared invoices paid before a date, and their cheques. Nothing open, unpaid or uncleared is touched.
- **The AP cycle:** enter the invoice (or cash voucher), post it, create the cheques, print them, confirm them, Update AR and AP, post the GL voucher; clear the cheques from the bank statement; purge the cleared ones now and then.
- **Accounts > Sales Customers** (1052, Spec #6): the Accounts Receivable customers.
  - **Fields:** number, short name and name, address and ship-to address, phone and fax; under **Options** (1063), the AR, sales and VAT accounts, the VAT rate, terms, contact and VIN.
  - **New** fills in the default accounts (Company Profile > Account Setup: AR, SALE, VATP). Options works before a new customer is first saved.
  - **Add** checks the names, the rate and the accounts against the chart; a wrong account opens Options on it. **Delete** is refused while the customer has sales vouchers, open items or receipts.
- **Transactions > Sales Invoice Vouchers** (1053): a customer's invoice.
  - **The header:** type the customer and press Enter to fill in the accounts, VAT rate, terms and due date; then the invoice number, the document and the period.
  - **Each Add** saves a product line: a quantity at a unit price (picking a product fills its price), or an amount typed, VAT included, on the customer's sales account or another. The VAT and AR lines are recomputed from all the product lines, so the voucher always balances.
  - **The Distribution List** shows every line; double-click a product line to change it or Delete it. **Memo** keeps a note on the voucher.
  - The same customer's invoice number can't be entered twice. **New and Close** ask whether to keep a voucher made in this session; No removes only that voucher.
- **Transactions > Post Voucher > Sales Voucher** (1061) posts the open sales vouchers (all with Post, or one by double-click): each gets its open item, owed in full, and its journal. **Cancel Voucher > Sales Invoice Voucher** (1054) marks one cancelled, keeping it. **Recover Sales Voucher** reopens a posted one whose journal is still waiting and which has no receipt.
- **Transactions > Clear Receipts** (1156): what customers pay.
  - The list shows the invoices with something owed (**View All**: every one). Double-click one to record a receipt (1155): the amount (what's owed is filled in), the date and the bank.
  - **In full**, **Partial** (the rest stays owed) or **Discount** (the rest is written off as an early-payment discount, split into sales and VAT at the invoice's rate). A short amount with no box ticked asks which.
  - Each receipt goes in AR's journal at once (Dr bank, Dr sales and VAT for a discount, Cr AR), dated the receipt, in its period, which must be open.
  - **Receipts** lists the selected invoice's receipts and reverses the latest (a receipt entered by mistake, a bounced cheque): removed if still waiting, reversed with new rows dated today if it has gone to the General Ledger.
- **Maintenance > Purge Received Receivables** (1062) deletes the settled invoices last received before a date, with their receipts (all with Purge, or one by double-click). Nothing owed is touched, and the vouchers and journal stay.
- **The AR cycle:** enter the invoice, post it, record the receipts as they come in, Update AR and AP, post the GL voucher; purge the settled ones now and then.
- **Reports**, the AP and AR ones (Reporting > Accounts Payable and Accounts Receivable, and some on Transactions and Receipts/Payments) among them: see [Reports](#reports). They are in the report list too.
- **Help > About.**
- **Everything else:** other General Ledger commands say they are not part of Spec #1.

## Reports

Spec #3 replaced the Crystal Reports runtime with the port's own report engine. Every General Ledger, fixed-asset, Accounts Payable and Accounts Receivable report opens in a **preview** window of its own; none goes straight to the printer.

### The reports

| Menu | Report | Options |
|---|---|---|
| Reporting > General Ledger > **Trial Balance** | F1: each account's balance at the end of a period, in Debit or Credit, with the totals | period; zero balances |
| Reporting > General Ledger > **Distribution List** | F2, the Distribution Trial Balance: for a period or a range, each account's opening balance, debits, credits and closing balance. A row marked `*` has movement without posted lines | periods; each account's posted lines beneath it (the original Distribution List, landscape); accounts with nothing |
| Reporting > Financial Statements > **Income statement** | F3: month and year to date | period; department; prior year; budget and variance; the accounts under each line; zero lines |
| Reporting > Financial Statements > **Balance sheet** | F4: end of the period and start of the year | period; accounts; zero lines |
| Reporting > Financial Statements > **Cash flow statement** | F5: month and year to date, from net income, reconciled to cash | period; accounts; zero lines |
| Reporting > General Ledger > **Trial Balance by Dept.** | the original listing (`GLDEPT.RPT`) at the current period | none |
| Accounts > **Chart of Accounts Report** (also the toolbar's inquiry button) | opening balance, accumulated this year, current balance | none |
| Accounts > **Budget Report** | the current period's amount, its budget and the difference | none |
| Reporting > General Ledger > **Voucher Slips**, **Voucher Detail**, **Voucher Summary** | every voucher, in any state. The detail shows a voucher's difference when its debits and credits differ | none |
| Reporting > General Ledger > **Preposting Distribution List**; Transactions > Voucher Reports > **General Ledger Pre-Posting Report** | the vouchers not yet posted | none |
| Reporting > General Ledger > **Medical Representatives**, **Product code summary**, **Doctor Codes** | the master-data listings | none |
| Voucher Entry's **Print** | the voucher's slip, in a window over the dialog | none |
| Fixed Assets > **Fixed assets listing** | the asset register by type at the current period: cost, salvage, monthly and accumulated depreciation, book value; disposed assets with their date sold. It ends with a check of the register against the ledger, account by account | none |
| Fixed Assets > **Fixed asset types** | each type with its asset, expense and accumulated-depreciation accounts | none |
| Transactions > Voucher Reports > **Invoice Pre-Posting Report**, **Cash Pre-Posting Report** | the open purchase (or cash) vouchers, each with its lines and totals | none |
| Reporting > Accounts Payable > **AP Voucher Detail**, **Cash Voucher Detail** | the current period's vouchers, every state, with their lines | none |
| Reporting > Accounts Payable > **Voucher Status** | a line per purchase voucher of the current period with its state; the total leaves out the cancelled | none |
| Reporting > Accounts Payable > **Invoice Posting Journal** | the posted purchase vouchers' expense, VAT and W/H lines, and what each charged to AP | none |
| Reporting > Accounts Payable > **Vendor List** | every vendor with address, accounts, terms and rates | none |
| Reporting > Accounts Payable > **Open Payables Detail** (also Accounts > Vendor Inquiry Report) | the unpaid invoices (held, open, forced) by vendor, with what each owes | none |
| Reporting > Accounts Payable > **Aged Payables 30-60-90** | the unpaid invoices by days past due, as at today: current (up to 30), 31-60, 61-90, over 90 | none |
| Reporting > Accounts Payable > **Cash Requirement** | the unpaid invoices by days to their due date: up to 10 (overdue included), 11-20, 21-30, over 30 | none |
| Reporting > Accounts Payable > **Accounts Payable Register** | every open item by vendor: amount, W/H, paid, date paid, cheque, balance, state | none |
| Receipts/Payments > **Pre-payment List** | the unpaid invoices with their order numbers | none |
| Receipts/Payments > **Print Cheques** | the cheques made and not yet printed, one a page | none |
| Receipts/Payments > **Cheque Payment Detail** | per cheque, the invoices it pays: amount, VAT, W/H, net payment | none |
| Reporting > Accounts Payable > **Cash Cheque Summary** | the current period's cash vouchers paid by cheque, less W/H | none |
| Reporting > Accounts Payable > **VAT: AP Vouchers**, **VAT: General Ledger** | the VAT lines of the period's posted AP vouchers; the General Ledger lines on the VAT account (Company Profile, VATR) | none |
| Reporting > Accounts Payable > **Withholding Tax** | the W/H lines of the period's posted AP vouchers, by W/H account | none |
| Reporting > Accounts Payable > **AP Journal Summary**, **AP Journal Detail**, **AP Journal: Cash**, **AP Journal: Invoices and Payments** | AP's journal for the General Ledger: what waits for Update AR and AP, by account; the current period's rows, all or by type | none |
| Transactions > Voucher Reports > **Sales Pre-Posting report** | the open sales vouchers, each with its lines and totals | none |
| Reporting > Accounts Receivable > **Customer List** | every customer with address, accounts, terms and VAT rate | none |
| Reporting > Accounts Receivable > **Customer Summary** | a line per customer: invoiced, received, discounts, owed | none |
| Reporting > Accounts Receivable > **Open Receivables Detail** (also Accounts > Customer Inquiry Report) | what's owed, by customer | none |
| Reporting > Accounts Receivable > **Accounts Receivable Register** | every open item by customer with its receipts: amount, received, discount, bank, owed, state | none |
| Reporting > Accounts Receivable > **Voucher Status**, **Sales Voucher Detail** | the current period's sales vouchers, a line each with its state (the total leaves out the cancelled), or with their lines | none |
| Reporting > Accounts Receivable > **Receipts Register** | the current period's receipts: invoice, bank, amount, discount | none |
| Reporting > Accounts Receivable > **Aged Receivables 30-60-90** | what's owed by days past due, as at today: current (up to 30, and not yet due), 31-60, 61-90, over 90 | none |
| Reporting > Accounts Receivable > **Collections Forecast** | what's owed by days to its due date: up to 10 (overdue included), 11-20, 21-30, over 30 | none |
| Reporting > Accounts Receivable > **VAT: General Ledger**, **VAT: Sales Vouchers** | the General Ledger lines on the output VAT account (Company Profile, VATP); the output VAT of the period's posted sales vouchers, less the VAT of receipts' discounts | none |
| Reporting > Accounts Receivable > **AR Journal Summary**, **AR Journal Detail** | AR's journal for the General Ledger: what waits for Update AR and AP, by account; the current period's rows (AR invoices, RC receipts) | none |
| Reporting > Accounts Receivable > **Customer Statements** | per customer owing, the address and the open items, one a page | none |

A report with options asks for them first and keeps them until the program exits. The reports read the books and change nothing.

**The financial statements' lines** come from a statement layout kept in the company database (`gl_fsline`, `gl_fsrange`), created with a default grouping by account number the first time a statement runs. Accounts on no line print as "Unclassified accounts", and the statements say which. **Accounts > Statement Layout** edits it, one statement at a time.
- **Editing lines:** pick a line in the grid and change its fields (kind, caption, sign, indent, a total's formula, an account line's ranges such as `410000-419999; 420000-429999`), then Apply. Add, Insert before and Delete change the lines themselves.
- **Checking and trying it out:** Check lists errors and warnings (an account on no line, or on two). Preview runs the statement from your changes before you save.
- **Saving:** Save refuses errors and asks about warnings. Reset to default puts the default layout back.

Line numbers are what totals refer to, so they are never renumbered. Statement Layout is greyed for users who can't open the Chart of Accounts. (The menu's "Statement of retained earnings" is now "Cash flow statement"; "Manufacturing statement" is gone.)

### The preview

Each preview has Print, First / Previous / Next / Last, the page number, Zoom (fit width, fit page, 100 %), Options (the report's options again, then a fresh copy) and Close. PgUp / PgDn, Home / End, the arrow keys, the mouse wheel and Ctrl+P work in it. Several previews can be open at once; the Window menu lists them.

### Printing

- **Print** in a preview (or Ctrl+P), **File > Print** and the toolbar's Print button print the active preview. The Print dialog chooses the printer and the pages; on Windows 11 it is the system's own print window.
- **With no preview active**, File > Print and the toolbar's Print button open the **report list** ("Print Custom Report"). **Reporting > Custom Report** (also on the right-click menu) always opens it. Choose a report and press Print (or double-click it, or press Enter): it runs as from its menu. The original's custom Crystal reports can't run, so the list holds the port's reports and can't be edited.
- **File > Printer Setup** chooses the printer, paper and margins. The choice lasts until the program exits; it doesn't change the Windows default printer. Orientation isn't offered: each report has its own.
- **To make a PDF**, choose *Microsoft Print to PDF* in the Print dialog; Windows then asks for the file name.

## Icons

The buttons and toolbar show icons from Microsoft's **Fluent UI System Icons** (MIT licence, `gl/res/icons/fluent/LICENSE`) in place of the original bitmaps (UI refresh spec §4.2). `gl/res/icons/fluent/map.csv` says which icon replaces which bitmap, and `sheet.png` shows each pair. A bitmap with no icon there is still shown, with its greys matched to the system's button colour. To change or add one, edit `map.csv` and follow `gl/res/tools/make_icons.py`'s instructions (it downloads the SVGs at a pinned commit and renders the `.ico` files); the outputs are committed.

## Checks

- **`gl.exe /checkres`** loads every resource the port uses, including the port's own dialogs (`gl/res/port.rc`), the report list (1022) and the Fixed Assets dialogs, and checks that each dialog's captions fit their controls (UI refresh spec §4.6). Add `/quiet` to get the failure count as the exit code instead of a message box.
- **`gl.exe /screenshots [dir]`** shows the frame and saves it and every dialog as PNG files in `dir` (default `screenshots`), named `<id>-<dpi>.png`, then exits. For before-and-after comparisons of the UI refresh. Add `/quiet` to skip the summary.
- **`gl.exe /stress`** opens and closes the Chart of Accounts 100 times, (Add, Delete and Budget Data in turn), then Department, Medical Rep, Doctor and Product Codes, Voucher Entry, Post, Recover, Cancel and Transaction Inquiry 25 times each, then every report's preview (the sample report, the finance set and the original reports) 25 times, paging through each at every zoom, then the Fixed Assets screens 25 times, on the company database, and reports the change in GDI and USER objects, in total and per step. With `/quiet` the total is the exit code, so 0 passes.
- **`gl.exe /samplereport`** opens the report engine's test report after startup: the chart of accounts grouped by type, with subtotals, a total and a wrapped note. Its options repeat the list (to make pages), drop zero balances, or turn the page to landscape. Use it to check the preview, Printer Setup and printing, e.g. to *Microsoft Print to PDF*.
- **`gl.exe /failpost <n>`** runs normally, except that posting fails after writing its nth line. Use it to check that a failed post leaves `glaccts`, `gl_dist`, `gl_btch` and `gl_btchd` unchanged (spec #2 §8 #6).
- **Debug builds** report CRT heap leaks at exit in the debugger's output window.
