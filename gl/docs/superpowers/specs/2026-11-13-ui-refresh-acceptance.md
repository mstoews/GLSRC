# UI refresh acceptance sign-off

The record of milestone U5 of the UI refresh (`2026-11-13-ui-refresh-design.md`, §9 and §10). It checks the look-only changes of U1–U4: the font, the icons, the toolbar and status bar, and high-DPI scaling. It also checks that nothing a user does has changed.

Fill in every blank (`____`) and set each **Result** to `PASS`, `FAIL` or `PASS with known gap`. A `FAIL` needs a triage line in [Gaps and triage](#gaps-and-triage): either fixed in the UI refresh, or filed for later (the redesign, or the spec that ports the screen).

Run everything on one copy of the dev fixture's `dev.db` (`gl\test\dev-fixture\setup.cmd`), logged in as `ADMIN`. Keep the screenshot folders (P3) outside the repository.

## Run details

| Item | Value |
|---|---|
| Commit tested (`git rev-parse --short HEAD`, on `ui/refresh` or `main` after it merges) | ____ |
| Build machine | ____ |
| Windows edition and build (`winver`) | ____ (target: Windows 11 23H2 x64; Windows 10 22H2 x64 if 11 is unavailable) |
| Monitor 1: resolution and scaling | ____ (the single-monitor checks change its scaling: 100%, 125%, 150%, 200%) |
| Monitor 2: resolution and scaling | ____ (target: 150% while monitor 1 is at 100%; for criterion 7 only) |
| Tester | ____ |
| Date | ____ |

## Before the walkthrough

### P1. Builds

| Check | Result | Notes |
|---|---|---|
| `gl.sln` `Debug|x64` builds with `/W4 /WX`, no warnings | ____ | |
| `gl.sln` `Release|x64` builds with `/W4 /WX`, no warnings | ____ | |
| `gl.exe` is about 0.8 MB larger than `main` before the refresh (the icons; spec §10 U2 note) | ____ | |

### P2. Automated checks

From `cmd`, run each with `start /wait ... /quiet`, then `echo %errorlevel%`. Each should print 0. Run the `/checkres` rows at the scaling named, signing out and in after changing it.

| Check | Debug | Release | Notes |
|---|---|---|---|
| `x64\<Config>\cb4sqlite_tests.exe` (75 test cases pass) | ____ | ____ | |
| `x64\<Config>\gl_tests.exe` (114 test cases pass) | ____ | ____ | |
| `gl.exe /checkres /quiet` at 100%: dialogs, text fit (§4.6), the 31 icons | ____ | ____ | |
| `gl.exe /checkres /quiet` at 150% | ____ | ____ | |
| `gl.exe /stress /quiet` (GDI + USER growth: the buttons now hold icons, and the grids their own fonts) | ____ | ____ | |
| `trivial_dialog.exe /selftest /quiet` (covers `fx::Grid` editing, whose fonts changed in U4) | ____ | ____ | |

Without `/quiet`, `/checkres` lists each failure. Copy any `text too wide` or `text wraps` lines into [Gaps and triage](#gaps-and-triage); each one is fixed by a `PATCHES` entry in `gl/res/tools/prune_rc.py` (spec §4.1).

### P3. Screenshots

`gl.exe /screenshots <dir>` saves the frame and the 23 top-level dialogs as `<id>-<dpi>.png` (spec §4.6). Take each set at 100% and 150%.

| Set | Built from | 100% | 150% | Notes |
|---|---|---|---|---|
| `before` | `ui/u0` (the original fonts and pictures) | ____ | ____ | |
| `after` | the commit under test | ____ | ____ | |

## The criteria

### 1. Font (U1, D1)

Compare `after` with `before`, dialog by dialog, at 100% and at 150%. Expected: the same layout in Segoe UI, about 15% larger. No text cut off or wrapped onto a hidden line. No controls overlapping. Every dialog fits on the screen.

| Dialog | 100% | 150% | Notes |
|---|---|---|---|
| 1010 Chart of Accounts | ____ | ____ | |
| 1017 Budget Data | ____ | ____ | |
| 1158 Login | ____ | ____ | |
| 1159 User Access Update | ____ | ____ | |
| 1160 Menu Access | ____ | ____ | |
| 1021 Company Profile | ____ | ____ | |
| 1028 Set Period Dates | ____ | ____ | |
| 1033 Account Setup | ____ | ____ | |
| 1034 Cash Accounts, with its list 1011 beside it | ____ | ____ | |
| 1035 List Window Parameters | ____ | ____ | |
| 1023 Department Codes | ____ | ____ | |
| 1006 Medical Reps | ____ | ____ | |
| 1040 Doctor Codes | ____ | ____ | |
| 1007 Product Codes | ____ | ____ | |
| 1013 Voucher Entry (Segoe UI 8) | ____ | ____ | |
| 1061 Post, 1015 Recover, 1043 Cancel | ____ | ____ | |
| 1029 Transaction Inquiry | ____ | ____ | |
| 1008 About | ____ | ____ | |
| 1022 Report list (File > Print with no preview, Reporting > Custom Report) | ____ | ____ | |
| 4002 Asset Types, 4004 Fixed Assets Add/Modify, 4003 Create Voucher, 1001 Disposal | ____ | ____ | |
| Port dialogs: Sample Report options, Trial balance options (F1 and F2), Financial statement options (F3–F5) | ____ | ____ | |
| Statement Layout (Accounts > Statement Layout) | ____ | ____ | |

**Result:** ____ **Notes:** ____

### 2. Icons (U2, D2)

1. Review `gl/res/icons/fluent/sheet.png`: does each icon say what its button does? (spec §11: the user's review is U2's exit.)
2. In the running program, at 100% and 150%:
   - Chart of Accounts: Add/Mod, Search, Set Data, Delete and Close show their icons beside the text, sharp, with no grey box around them.
   - Voucher Entry: New, Top, Bottom, Next, Previous, Delete, Voucher list, Add, List, Print, Close; and the tool box (Show) shows its ten icon-only buttons.
   - Menu Access: each row shows a plus or a minus, and double-clicking a row (or Enter) switches it.
   - A disabled button with an icon looks disabled.

| Check | Result | Notes |
|---|---|---|
| Icon sheet reviewed; changes asked for: ____ | ____ | |
| Dialog buttons | ____ | |
| Voucher Entry tool box | ____ | |
| Menu Access plus / minus | ____ | |
| Disabled buttons | ____ | |

**Result:** ____ **Notes:** ____

### 3. Toolbar (U3, D3)

Hover over each button for its tooltip, then click it. Close what it opens before the next one.

| Button (tooltip) | Expected | Result |
|---|---|---|
| New | "not part of Spec #1" | ____ |
| Open | "not part of Spec #1" | ____ |
| Save | "not part of Spec #1" | ____ |
| Print | with a report preview active, prints it; without one, the reporting "not yet ported" message | ____ |
| General Ledger Voucher | Voucher Entry | ____ |
| Purchase Invoice Vouchers | the AP "not yet ported" message | ____ |
| Cash Invoice Vouchers | the AP "not yet ported" message | ____ |
| Sales Invoice Voucher | the AR "not yet ported" message | ____ |
| Chart of Accounts Report | the reporting "not yet ported" message | ____ |
| Post General Ledger Voucher | Post General Ledger Voucher (1061) | ____ |
| Chart of Accounts | the Chart of Accounts (1010) | ____ |
| Exit | the program closes | ____ |

Also check: there are separators after Print, Sales Invoice Voucher and Chart of Accounts. The buttons are flat, and highlight on hover.

**Result:** ____ **Notes:** ____

### 4. Status bar (U3, D4)

| Check | Expected | Result |
|---|---|---|
| Left part | The company name | ____ |
| Right part | The current period, "Oct 26" on the fixture | ____ |
| Middle part | Moving through the menus shows each item's help; a menu title shows "Select a menu item…"; closing the menu shows "Ready..." | ____ |
| Period update | Company Profile > Periods: change the current period, then OK. The right part shows the new one. Change it back. | ____ |
| Resize | Narrow and widen the frame: the company and period parts keep their widths, and the help part takes the rest. The size grip works. | ____ |

**Result:** ____ **Notes:** ____

### 5. Nothing a user does has changed

| Check | Expected | Result |
|---|---|---|
| Chart of Accounts: Tab through the fields | The same order as on `main` before the refresh | ____ |
| Chart of Accounts: Alt+letter shortcuts (`&Account`, `&Search`, `&Close`…) | Each still works | ____ |
| Voucher Entry: Show, then Hide | The dialog narrows by its column of buttons while the tool box shows, and comes back to its full width after | ____ |
| Voucher Entry: the tool box's buttons | Each does what the dialog's own button does | ____ |
| Account list window (it opens with the Chart of Accounts) | Columns as before, the description column filling the rest | ____ |
| F1 and F2 with their options, in the preview | As in the S3-M2 checks (progress log) | ____ |

**Result:** ____ **Notes:** ____

### 6. High DPI: one monitor (U4, spec §5.3)

Sign out and in after each scaling change. At each, open the frame and the screens below. Look for:
- text cut off, or overlapping controls;
- blurred icons;
- a toolbar or status bar whose height doesn't fit its contents;
- grid columns too narrow for their headings.

| Screens | 100% | 125% | 150% | 200% |
|---|---|---|---|---|
| Frame: toolbar, status bar, a right-click on the background (GLSUB) | ____ | ____ | ____ | ____ |
| Chart of Accounts and Budget Data | ____ | ____ | ____ | ____ |
| Company Profile, Period Dates, Account Setup, Cash Accounts | ____ | ____ | ____ | ____ |
| Login, User Access, Menu Access | ____ | ____ | ____ | ____ |
| Department, Medical Rep, Doctor and Product Codes | ____ | ____ | ____ | ____ |
| Voucher Entry, with the tool box shown | ____ | ____ | ____ | ____ |
| Post, Recover, Cancel; Transaction Inquiry | ____ | ____ | ____ | ____ |
| Fixed Assets: Asset Types, Add/Modify, Create Voucher, Disposal | ____ | ____ | ____ | ____ |
| Report list; Statement Layout; Transactions > Voucher List | ____ | ____ | ____ | ____ |
| Account list window | ____ | ____ | ____ | ____ |
| F1 and F2: options dialogs; the preview at "Zoom: 100%" and "Zoom: whole page" | ____ | ____ | ____ | ____ |
| `/checkres` text fit (0 failures) | ____ | ____ | ____ | ____ |

**Result:** ____ **Notes:** ____

### 7. High DPI: two monitors (U4, spec §5.2)

Monitor 1 at 100%, monitor 2 at 150%. Drag each from monitor 1 to monitor 2, check, then drag it back and check again.

| Window | Expected after each move | 1 → 2 | 2 → 1 |
|---|---|---|---|
| The frame, with an F1 and an F2 preview open | Toolbar icons and height, status bar font and parts, and each preview's button strip follow the new scaling | ____ | ____ |
| Chart of Accounts (modal), then its account list (a window of its own) | The dialog, its text and its button icons rescale; the list's font and columns rescale | ____ | ____ |
| Menu Access (modal) | As above, and the plus and minus rescale; the grid's columns too | ____ | ____ |
| Voucher Entry with its tool box showing (move each) | Each rescales on its own monitor. Then Hide and Show: the dialog's width is still right | ____ | ____ |

**Result:** ____ **Notes:** ____

## Known gaps accepted before the walkthrough

| # | Gap | Why accepted | Follow-up |
|---|---|---|---|
| G1 | The screens not yet ported (AP, AR, fixed assets) are not checked. They get Segoe UI automatically through `prune_rc.py`, but their pictures may be old bitmaps without icons. | Spec §1.2. | Their specs (#4–#6) add `map.csv` rows for their buttons. |
| G2 | The print Cancel window isn't rescaled if it is dragged to another monitor while printing. | It exists only while printing (spec §10 U4 note). | None. |
| G3 | The frame icon "GL", the About bitmap and the "G/L" logos are the original 16-colour artwork, blurred at 150% and up. | The company's own artwork (spec §1.2, §11 Q2). | Spec §11 Q2, or the redesign. |
| G4 | No dark mode. | Spec §1.2. | The redesign. |
| G5 | The icons are one colour (#212121), as in Windows 11, not full colour as spec §1.1 said. | Spec §10 U2 note. | None. |

## Gaps and triage

Add one row for each `FAIL`, each `/checkres` text-fit line, and anything unexpected.

| # | Criterion | What happened | Decision (fix in the UI refresh / file for later) | Fixed in commit |
|---|---|---|---|---|
| | | | | |

## Sign-off

| Criterion | Result |
|---|---|
| P1–P3 | ____ |
| 1. Font | ____ |
| 2. Icons | ____ |
| 3. Toolbar | ____ |
| 4. Status bar | ____ |
| 5. Nothing a user does has changed | ____ |
| 6. High DPI: one monitor | ____ |
| 7. High DPI: two monitors | ____ |

All criteria pass, or pass with the known gaps recorded above. When criteria 6 and 7 pass, known gap G1 of the Spec #1 and #2 sheets (high DPI not tested) is closed for the screens ported so far:

- Tester: ____ Date: ____
