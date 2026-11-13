# UI Refresh — A Better Look for the Ported Screens

**Status:** Draft
**Date:** 2026-11-13
**Scope:** A small, look-only pass over the Win32 port. The screens stay where they are and work as they do; they get a modern font, modern icons, a standard toolbar and status bar, and correct scaling on high-DPI displays.
**Depends on:** Spec #1 (`2026-05-10-owl-to-win32-sdk-port-design.md`) for the resource strategy (§6), the custom controls (§6.3) and the manifest (§7.1). It doesn't depend on Spec #2 or #3, and can run between or after their milestones (§10).

**Why so small.** The application will be redesigned from the ground up later. This pass is for the years the port is in use before that. Its rule is: **nothing that changes how the program is used, and nothing the redesign would throw away at great cost.** The pieces most likely to carry over into the redesign are the icon set (§4.2) and the text-fit and screenshot checks (§4.6).

---

## 1. Goals & Non-Goals

### 1.1 Goals

1. **Font** (§4.1): every dialog in Segoe UI, Windows' own dialog font, instead of MS Sans Serif and the Windows 3.1 "Helv".
2. **Icons** (§4.2): modern full-colour icons, sharp at any scaling, in place of the 4-bit bitmaps on the ported screens' buttons and toolbars.
3. **Toolbar** (§4.3): the main toolbar becomes a standard Windows toolbar, with flat buttons and tooltips, without the stretched background bitmap.
4. **Status bar** (§4.4): the bottom bar becomes a standard Windows status bar, with the same three fields.
5. **Edit fields** (§4.5): every `tb*` field gets the themed Windows border.
6. **High DPI** (§5): the port looks right at 100%, 125%, 150% and 200% scaling, and when a window moves between monitors. This closes known gap G1 of the Spec #1 and #2 acceptance sheets, for the screens ported so far.
7. **Checks** (§4.6): `/checkres` reports text that doesn't fit its control. A new `/screenshots` mode saves an image of every dialog, so each change can be compared before and after.

### 1.2 Non-Goals

- **Anything a user would notice in how they work.** No change to dialog layouts, beyond the sizing fixes in §4.1. No change to tab order, keyboard shortcuts, menus, wording, or what any button does.
- **New controls.** `tbDate` stays a masked edit (no date picker), and `tbSpread` stays the current list view grid.
- **Colours with meaning.** The grids' red and blue-on-green marks, for example in Menu Access and the voucher lists, stay as they are.
- **Dark mode.** Windows' standard controls barely support it.
- **The report preview and printing.** Spec #3 already built them with system fonts and DPI scaling. They are only tested here (§5.3).
- **Screens not yet ported.** Mostly AP and AR. They get the font automatically, because `prune_rc.py` applies it to every dialog it emits (§4.1), but nothing is done to them by hand.
- **Redrawing the company's own artwork:** the frame icon "GL", the About box bitmap and the "G/L" logos (1111, 3076). See §11 Q2.

---

## 2. The UI today

| Area | What it is now | Where |
|---|---|---|
| Dialog font | 23 dialogs in `gl/res/gl.rc` use the original fonts: 17 MS Sans Serif 8; the module bar 2057 MS Sans Serif 10; Voucher Entry 1013 MS Sans Serif 7; 1011, 1158 and 1159 "Helv" 8. The Spec #3 dialogs in `port.rc` copied MS Sans Serif 8. | `gl.rc` comes from `gl/res/tools/prune_rc.py`, which copies each template's `FONT` line as it is. |
| Button pictures | 83 bitmaps, almost all 4-bit, 11–32 px, with a grey background and often a bevel drawn into the picture. Ported screens give about 35 of them to buttons, through `MainDialog::SetBitmap(id)` → `SBM_SETPICT` with the bitmap of the same ID. | `gl/res/bitmaps/`, `win32fx/src/controls/SuperBtn.cpp` |
| `tbSuperBtn` | A superclass of the system BUTTON, so already themed. The picture is drawn with `BM_SETIMAGE`. The grey background of the bitmap shows as a box on the themed face. | `SuperBtn.cpp` |
| Main toolbar | Dialog 1032, a modeless child dialog of `tbSuperBtn`s, moved into 30 × 25 px slots by `DialogBar::PlaceButton`. The background is the 10 × 30 bitmap `STATUSBAR`, stretched. 12 visible buttons; 8 spreadsheet buttons placed but never shown. No tooltips. | `win32fx/src/Toolbar.cpp`, `gl/port/ManilaGL.cpp` (`OnCreate`, `TopToolbar::MapCommand`) |
| Bottom bar | Dialog 2057 on the same stretched bitmap. It holds three static fields: company (`ID_STATUSHELP`), menu help (`ID_MENUHELP`) and period (`ID_PERIODFIELD`). `ManilaGL::OnSize` places them in pixels. | `win32fx/src/VerticalToolbar.cpp`, `ManilaGL.cpp` |
| Edit borders | `tb*` edits with `ES_3D` get `WS_EX_CLIENTEDGE`; the others get `WS_BORDER`, a flat black line. | `win32fx/src/controls/EditBase.cpp` |
| Voucher Entry tool box | A floating palette of ten picture-only `tbSuperBtn`s, 30 × 25 px. | `gl/port/VoucherLists.cpp` (`ToolBox`) |
| DPI | The manifest declares per-monitor v2. Only the report preview and printing scale. The rest has pixel constants: toolbar slots, `TOOLBAR_HEIGHT` 30, the status field widths, the tool box. Nothing handles `WM_DPICHANGED`. Grid columns are already in character widths, so they follow the font. | §5.1 |

---

## 3. Design principles

1. **Change shared code, not screens.** The custom controls are registered under their original class names (Spec #1 §6.3), and `gl.rc` is generated. A change in `SuperBtn.cpp`, `EditBase.cpp` or `prune_rc.py` reaches every screen at once, including the AP and AR screens still to come. A port file is touched only where it has pixel constants (§5.1).
2. **Generated files stay generated.** Font and layout fixes to `gl.rc` go into `prune_rc.py`, never into `gl.rc` by hand. Its header says hand edits are lost.
3. **Measure, then fix.** Each change is checked by the text-fit check and by before-and-after screenshots (§4.6), not by eye alone.
4. **One branch and one commit per milestone,** as in the port specs.

---

## 4. Changes

### 4.1 Font

**D1. Segoe UI, one point larger than the original.** MS Sans Serif 8 and Segoe UI 9 look about the same size; Segoe UI 9 is what Windows itself uses in dialogs. The map:

| Original | New |
|---|---|
| `FONT 7, "MS Sans Serif"` (Voucher Entry 1013) | `FONT 8, "Segoe UI"` |
| `FONT 8, "MS Sans Serif"`, `FONT 8, "Helv"`, `FONT 8, "Helv "` | `FONT 9, "Segoe UI"` |
| `FONT 10, "MS Sans Serif"` (module bar 2057) | `FONT 11, "Segoe UI"`; the bar goes in U3 anyway (§4.4) |

- **Where:** `prune_rc.py`'s `clean()` rewrites the `FONT` line through a table, then `gl.rc` is regenerated. `port.rc` (Spec #3's options dialogs, and any later ones) is edited by hand. New `port.rc` dialogs use `FONT 9, "Segoe UI"` from then on.
- **The effect on size:** dialog units follow the font, so every dialog grows by about 15% at the same scaling, keeping its proportions. Nothing is moved by hand.
- **Rejected:** `DS_SHELLFONT` with "MS Shell Dlg 2", which is Tahoma on current Windows, not Segoe UI. Also rejected: replacing the font in each template in memory at load time. It needs DLGTEMPLATE re-alignment code, and gains nothing over doing it at generation time.
- **Text that no longer fits** (§4.6 finds it): `prune_rc.py` gains a small `PATCHES` table, keyed by dialog and control ID, that widens a control (or, rarely, the dialog), with a comment saying why. Each patch is a few dialog units. If a dialog needs more than a handful, record it and widen the dialog instead.

### 4.2 Icons

**D2. Fluent UI System Icons, MIT-licensed, "Regular" style.** Microsoft publishes this set as SVG. It matches Windows 11 and comes in every size needed. Reasons for this choice:
- Drawing a new set by hand costs far more than the rest of this spec together.
- The redesign can keep using the same set.
- Its licence only asks that the notice be kept: `gl/res/icons/fluent/LICENSE`.

Rejected: cleaning up the old bitmaps (making the grey transparent, removing the drawn bevels). They would still be 4-bit and 16-colour, and would still blur when scaled. It stays as the fallback for buttons without a new icon (below).

- **Mapping:** `gl/res/icons/fluent/map.csv` lists `bitmap ID, Fluent icon name, why`. For example, `4053, save_24_regular, the toolbar's Save`. It covers the ~35 bitmaps used by ported screens and the 12 toolbar buttons. U2's first task builds the exact list from the `SetBitmap`/`PlaceButton`/`SBM_SETPICT` calls.
- **Build:** `gl/res/tools/make_icons.py` renders each SVG at 16, 20, 24, 32, 40 and 48 px and writes one 32-bit multi-size `.ico` per entry to `gl/res/icons/fluent/P<id>.ico`. It uses ImageMagick or `rsvg-convert` to render and Pillow to write the `.ico`. The outputs are committed, so the Windows build needs nothing new. `port.rc` declares them as `P<id> ICON "icons/fluent/P<id>.ico"`. The downloaded SVGs stay out of the repo; the script records the release tag it used.
- **Loading:** no port file changes. `SBM_SETPICT` with `SUPERBTN_PICT_BITMAP` and an ID first looks for an icon named `P<id>`. If found, it loads it with `LoadIconWithScaleDown` at the button's DPI, 16 px at 96 DPI for text buttons and 20 px for picture-only buttons. If not, it loads the bitmap as now, with `LR_LOADMAP3DCOLORS` so its greys become the system button colours. Every button looks consistent, even the unmapped ones.
- **Grid cell images:** `PLUS` and `MINUS` (Menu Access) are mapped the same way through `Grid::SetCellImage`'s image list.

### 4.3 Toolbar

**D3. A standard toolbar (`ToolbarWindow32`) replaces dialog 1032.** It uses `TBSTYLE_FLAT | TBSTYLE_TOOLTIPS | CCS_NODIVIDER`. Its icons are 24 px at 96 DPI, scaled, from the same `P<id>` icons, in an image list rebuilt on DPI change.

- **The same 12 buttons in the same order, with the same commands.** `TopToolbar::MapCommand` is kept. The original's extra 0.3-slot gaps become separators: after Print, after AR voucher and after Account Add.
- The 8 spreadsheet buttons are dropped. They are never shown (Spec #1 has no spreadsheet), and dialog 1032 stays in `gl.rc` for `/checkres`.
- **Tooltips:** a short name per button (New, Open, Save, Print, GL Voucher, AP Voucher, Cash, AR Voucher, Account Inquiry, Post Vouchers, Chart of Accounts, Exit), kept in a `port.rc` string table. The menu-help strings are sentences; they keep going to the status bar.
- **win32fx:** a new `fx::CommandBar` (`Toolbar.h`), a thin wrapper that creates the toolbar, adds buttons and separators, maps commands and handles DPI. `Toolbar` and `DialogBar` are removed if nothing else uses them; today only `ManilaGL` does.

### 4.4 Status bar

**D4. `msctls_statusbar32` replaces dialog 2057,** through the existing `fx::StatusBar`, with a size grip.

- **Three parts:** company on the left (200 px at 96 DPI), menu help in the middle (the rest), period on the right (100 px). This is the original's layout.
- `ManilaGL` keeps calling a `SetText(ID_…)` that maps the old field IDs to parts. `RefreshStatus` and the menu-help code don't change.
- The `STATUSBAR` background bitmap is no longer used. `VerticalToolbar` is removed if unused.

### 4.5 Edit fields

**D5. Every `tb*` edit gets `WS_EX_CLIENTEDGE`,** with or without `ES_3D`. Under common controls v6 this draws the themed single-line border, in place of `WS_BORDER`'s flat black line. One line in `EditBase.cpp`; `styles.md` notes it.

### 4.6 Checks

**D6. Two automated checks, both in `ResourceCheck.cpp`, run on Windows.**

- **Text fit (in `/checkres`):** for every dialog it already creates, it measures each static, button, check box, radio button and group box caption in the dialog's font with `DrawText(DT_CALCRECT)`, allowing for the check box or radio glyph. It reports any text wider than its control as `dialog 1010, control 2021 "&Retained earnings": text 78 px, control 72 px`. It runs at the current system DPI. U1's exit is no reports at 100% and 150%.
- **Screenshots (`gl.exe /screenshots <dir>`):** it creates each dialog on its own, the way `/checkres` does, captures it with `PrintWindow`, saves `<id>.png` through WIC, and saves the frame, with its toolbar and status bar, as `frame.png`. The fields are empty, which is enough for a check of the look and sizes. One set is taken before U1 ("before") and one after each milestone, at 100% and 150%. They are not committed. The acceptance sheet (§9) points to where the user keeps them.

---

## 5. High DPI

### 5.1 Pixel sites

Each of these becomes DPI-aware: a 96-DPI value scaled with `MulDiv(v, GetDpiForWindow(h), 96)`, recalculated on `WM_DPICHANGED`.

| Site | Today | After |
|---|---|---|
| Main toolbar | `PlaceButton` 10/30/25 px, `TOOLBAR_HEIGHT` 30 | `fx::CommandBar` measures itself (`TB_GETIDEALSIZE`/`TB_AUTOSIZE`) |
| Bottom bar | `OnSize` 8 × 25 px, 100 px, 102 px | Status bar part widths, scaled |
| `MdiFrame` docking | Heights given in pixels | Asks each docked bar its height after a DPI change |
| Voucher Entry tool box | 30 × 25 px buttons, `dlg.right - 110` | Scaled; buttons sized to the icon plus padding |
| Voucher Entry Show/Hide | Narrows by the button's measured width + 10 | +10 scaled; the rest is already measured |
| Company Profile list placement | Measured from the dialog | No change |
| Grids | Columns in `CharWidth()` units | Recompute widths when the font changes (`WM_SETFONT` after a DPI change): `Grid` keeps each column's width in characters and reapplies it |

### 5.2 Moving between monitors

- **The frame** handles `WM_DPICHANGED`: it applies the suggested rectangle, rebuilds the toolbar's image list and the status bar's parts, and lays out again.
- **Dialogs created from templates** are rescaled by Windows under per-monitor v2, fonts included. Pictures set by `SBM_SETPICT` are not: `tbSuperBtn` handles `WM_DPICHANGED_AFTERPARENT` by reloading its icon at the new size.
- **Fonts set in code** (the report preview's buttons, the print abort box) are already created per DPI. The preview recreates them on `WM_DPICHANGED`; check that it does, and add it if not.

### 5.3 Test matrix

All on Windows 10 or 11, Release build:
- one monitor at 100%, 125%, 150% and 200%;
- two monitors at 100% and 150%, moving the frame, an open modal dialog and the report preview between them.

The screens are the ones in the Spec #1 and #2 acceptance sheets, plus the Spec #3 options dialogs and preview. G1 moves from "not tested" to "tested for the ported screens" in both sheets.

---

## 6. Files

| File | Change |
|---|---|
| `gl/res/tools/prune_rc.py` | Font map, `PATCHES`; regenerated `gl.rc` |
| `gl/res/port.rc` | Segoe UI; `P<id>` icons; toolbar tooltip strings |
| `gl/res/tools/make_icons.py`, `gl/res/icons/fluent/` | New: script, `map.csv`, `.ico` files, LICENSE |
| `win32fx/src/controls/SuperBtn.cpp` | `P<id>` lookup, DPI-sized icons, `LR_LOADMAP3DCOLORS` fallback, `WM_DPICHANGED_AFTERPARENT` |
| `win32fx/src/controls/EditBase.cpp`, `styles.md` | D5 |
| `win32fx/src/controls/Grid.cpp` | Column widths in characters reapplied on font change; cell images through the icon lookup |
| `win32fx/include/Toolbar.h`, `src/Toolbar.cpp` | `fx::CommandBar`; `DialogBar`/`Toolbar` removed if unused |
| `win32fx/src/StatusBar.cpp`, `VerticalToolbar.*` | Parts by field ID, DPI; `VerticalToolbar` removed if unused |
| `win32fx/src/MdiFrame.cpp` | `WM_DPICHANGED`; docked bar heights |
| `gl/port/ManilaGL.cpp` | Toolbar and status bar creation; `OnSize` |
| `gl/port/VoucherLists.cpp`, `VoucherEntry.cpp` | §5.1 pixel sites |
| `gl/port/ResourceCheck.cpp` | Text fit; `/screenshots` |
| `gl/README.md`, `win32fx/README.md` | The new look, the checks, the icon set |

No database, report, or behaviour code changes. `gl_tests`, `cb4sqlite_tests` and `report_tests` must pass unchanged.

---

## 7. Decision record

| # | Decision | Why |
|---|---|---|
| D1 | Segoe UI, original size + 1, applied in `prune_rc.py` | Windows' dialog font; one place reaches every generated dialog, now and for AP/AR |
| D2 | Fluent UI System Icons (MIT), built into `.ico` files; bitmaps as fallback | Modern, free, every size; no drawing; reusable by the redesign |
| D3 | Standard toolbar, same buttons and commands, tooltips added | Flat, themed, DPI-aware; nothing moves for the user |
| D4 | Standard status bar, same three fields | As D3 |
| D5 | Themed border on every `tb*` edit | One line; removes the flat black borders |
| D6 | Text-fit check and `/screenshots` | Font changes are judged by measurement, and before/after is repeatable |

---

## 8. Risks

| Risk | Mitigation |
|---|---|
| Dialogs ~15% larger no longer fit a 1366 × 768 laptop at 125% | The largest dialogs are about 340 × 180 DLU, which at the new size is still well under 768 px tall. Checked in U1's screenshots at 150% |
| Segoe UI glyphs wider than the original's, so labels clip | D6's text-fit check finds every case; `PATCHES` fixes them |
| A Fluent icon doesn't say what the old picture said | `map.csv` records each choice; the user reviews the icon sheet (U2 exit) before it ships |
| Icon licence | MIT; LICENSE kept beside the files; noted in `gl/README.md` |
| The work is lost in the redesign | Kept small: about one Spec #3 milestone in total. The icons and checks carry over |
| Spec #3 adds dialogs to `port.rc` while this runs | From U1 on, new dialogs use Segoe UI 9 and pass the text-fit check |

---

## 9. Acceptance

A short sheet, `2026-11-13-ui-refresh-acceptance.md`, in the shape of the Spec #1 and #2 sheets:
- **Automated:** `/checkres` passes with no text-fit reports at 100% and 150%. The three Linux test suites pass. `/W4 /WX` builds are clean in Debug and Release.
- **Visual:** for each dialog and the frame, the before and after screenshots at 100% and 150%, with a tick for "nothing clipped, nothing overlaps, icons sharp".
- **Behaviour:** a short walkthrough. Every toolbar button sends its old command. The status bar shows the company, the menu help and the period. Voucher Entry's Show/Hide and tool box work. Login's and Menu Access's grids look right.
- **DPI:** the §5.3 matrix. G1 updated in the Spec #1 and #2 sheets.

---

## 10. Milestones

Each is one branch and one commit (`ui/u0` … `ui/u5`), with a progress-log entry under a "UI refresh" heading listing what still needs Windows.

| | Milestone | Done when |
|---|---|---|
| U0 | **Checks first:** text fit in `/checkres`, `/screenshots`. Take the "before" set. | The before set exists at 100% and 150%; the text-fit report on the old font is recorded (it should be nearly empty) |
| U1 | **Font and edit borders** (D1, D5). `prune_rc.py` font map and `PATCHES`; `port.rc` by hand. | No text-fit reports at 100% and 150%; after set taken |
| U2 | **Icons** (D2). Map, script, `.ico` files, `SuperBtn` and `Grid` lookup. | An icon sheet (all mapped icons beside the bitmaps they replace) reviewed by the user; after set taken |
| U3 | **Toolbar and status bar** (D3, D4). | Behaviour walkthrough for the bars; after set taken |
| U4 | **High DPI** (§5). | §5.3 matrix passes |
| U5 | **Acceptance sheet**, README updates. | Sheet signed off |

On Linux, each milestone runs `tools/linux/wincheck_all.sh`, `tools/linux/build_tests.sh --run` and the windres compile of `gl.rc` and `port.rc`. U1 and U2 add a check that `prune_rc.py` and `make_icons.py` produce the committed files. Everything visual is checked on Windows.

---

## 11. Still open

1. **When.** *Settled 2026-11-13:* U0 now, from `port/s3-m2`; Spec #3 (S3-M3 on) resumes later. Still to decide: whether U1–U5 follow U0 at once or after Spec #3.
2. **The company's own artwork.** The frame icon "GL" (32 × 32, 16 colours) blurs at 150% and up. Redrawing it is design work outside this spec. A cheap middle way would be a Fluent "book" icon with the letters GL. Keep the original, or that?
3. **Fluent's release.** Which release tag to pin. The download needs your OK when U2 starts.
