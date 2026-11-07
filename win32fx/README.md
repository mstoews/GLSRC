# win32fx — the mini-framework (spec §3)

A static library: light C++ helpers over the Win32 SDK that let ports of the OWL screens keep their shape, plus the Toolbox controls the legacy dialog templates use. Include `win32fx.h` (namespace `win32fx`, alias `fx`). The library owns no globals; `hInst`, `HGLMenu`, `cb` and `szApp` live in `gl/port/App.cpp` (spec §3.7).

| Header | What |
|---|---|
| `Window.h` | HWND-owning base class. `this` travels in `lpParam` and is stored in `GWLP_USERDATA` on `WM_NCCREATE`. Virtuals: `OnCreate`, `OnCommand`, `OnSize`, `OnDestroy`, `OnMessage(msg, wp, lp, result&)`; `OnFinalMessage` after `WM_NCDESTROY` (the place for `delete this`). |
| `Dialog.h` | Modal (`DoModal`) and modeless (`CreateModeless`) dialogs; `this` in `DWLP_USER` from `WM_INITDIALOG`. `OnInitDialog` plus the same virtuals. Unhandled `IDOK`/`IDCANCEL` close the dialog, as OWL's `TDialog` did. Results go through `DWLP_MSGRESULT` except for the messages that return directly (`WM_CTLCOLOR*`, …). |
| `MsgMap.h` | `BEGIN_MSG_MAP` / `ON_MSG` / `ON_MSG_RANGE` / `END_MSG_MAP`, checked before the virtuals. |
| `HandleGuard.h` | Move-only RAII: `MenuGuard`, `FontGuard`, `BitmapGuard`, `BrushGuard`, `PenGuard`, `RgnGuard`, `IconGuard`, `MemDcGuard`. |
| `StringUtil.h` | `LoadResString`, `Format`, `Utf8ToWide`, `WideToUtf8`, `Trim`, `GetText`, `GetDlgItemString`. |
| `Verify.h` | `FX_VERIFY(cond, msg)`: live in Debug and Release (spec §7.2). |
| `MessageLoop.h` | `RunMessageLoop(frame, mdiClient, accel)`: MDI accelerators, the accelerator table, `IsDialogMessageW` for every live modeless `Dialog`. |
| `MdiFrame.h` | `MdiFrame` (owns MDICLIENT, lays it out between docked bars, handles the OWL `CM_TILECHILDREN`…`CM_EXIT` commands) and `MdiChild` (`CreateMDIWindowW`, `DefMDIChildProcW`). `OnMdiChildActivate(child)` is the menu-merge hook. The GL frame's swap of MANILA and GLE is in `gl/port/ManilaGL.cpp`; the merge points are in `MdiFrame.cpp`. |
| `Toolbar.h` | `DialogBar`: a modeless child dialog from a template whose button clicks go to the parent (`TToolBar::Mssg`); `PlaceButton` is the original `SetPic`. `Toolbar` adds the stretched background bitmap. |
| `VerticalToolbar.h` | The original `TVToolBar` (dialog 2057): the module buttons and the status fields 3222–3224. It docks at the *bottom*, as the original did. |
| `StatusBar.h` | A wrapper over the common-controls status bar. |
| `controls/CustomControls.h` | `RegisterCustomControls(hInst)` and the Toolbox calls/messages (below). |

**Dispatch boundary (spec §7.2):** `Window::WndProc` and `Dialog::DlgProc` each wrap the whole dispatch in one `try`/`catch`. An exception (e.g. a `cb4sqlite::Error`) is written to `OutputDebugStringW` and shown in a `MessageBox`, and the message loop carries on. From `WM_CREATE` it fails the creation; from `OnInitDialog` it closes the dialog.

**MDI children** get `DefMDIChildProcW` for the messages it must see (`WM_SIZE`, `WM_SETFOCUS`, `WM_SYSCOMMAND`, …) even when a handler handled them. A subclass overriding `MdiChild::OnMessage` or `Toolbar::OnMessage` must call the base first.

## Custom controls

`RegisterCustomControls` registers `tbPic`, `tbDate`, `tbFloat`, `tbInteger` and `tbSuperBtn` (`CS_GLOBALCLASS`), so `gl.rc`'s templates create them unchanged. Each one superclasses the system `EDIT` or `BUTTON` control. Dialog code therefore keeps using `WM_SETTEXT`/`WM_GETTEXT`, `EN_CHANGE`/`EN_KILLFOCUS` and `BN_CLICKED`, which is how the legacy code talks to them. Each control keeps its state in window bytes after the base class's own, so `cbWndExtra` is the base class's plus a pointer.

**The template text is the mask** ("XXXXXXX", "(999) 999 - 9999", "9999999999999.99"), as with Toolbox; the control starts empty. **Toolbox style bits are translated** before `EDIT` sees them, because several share values with unrelated `EDIT` styles: `ES_3D` (0x0400) is `ES_OEMCONVERT`, and `FS_SEPARATOR` (0x2000) is `ES_NUMBER`.

| Toolbox | Becomes |
|---|---|
| `ES_3D` 0x0400 | `WS_EX_CLIENTEDGE` (otherwise `WS_BORDER`) |
| `ES_STATIC` 0x0080 | `ES_READONLY` |
| `ES_LEFTALIGN` 0x0200 | numbers left-aligned (default right) |
| `ES_UPPERCASE` / `ES_LOWERCASE` | the same `EDIT` styles (tbPic only) |
| `SBS_DEFPUSHBUTTON` 0x0001 | `BS_DEFPUSHBUTTON` |

| Control | Accepts | Text in / out |
|---|---|---|
| `tbPic` | per mask position: `9` digit, `A` letter, `N` letter or digit, `X`/`?` anything, `U`/`L` letter made upper/lower case; other mask characters are literals, inserted as the user types. Empty mask: anything. | `WM_SETTEXT` shown as given (database values are never refused). |
| `tbDate` | digits; `/` inserted (`-` and `.` count as `/`). Format from the style: `DS_MMDDYY` (also used when there are no format bits; every legacy use fills those with `MM/DD/YY` text), `DS_DDMMYY`, `DS_YYMMDD`; `DS_CENTURY` four-digit year. | The text is the displayed date (legacy code parses it with `Date4`). `WM_SETTEXT` also takes `YYYYMMDD`; `DateGetYmd(ctl)` returns `YYYYMMDD` or "" if it isn't a real date (00–49 → 20xx). |
| `tbFloat` | `-`, digits and one `.` within the mask's digits (`9999999999999.99`: 13 + 2; no mask: 13 + 2). | `WM_SETTEXT` and leaving the field reformat to the mask's decimals, with `1,234.50` separators under `FS_SEPARATOR`. `FloatGetValue` / `FloatSetValue`. |
| `tbInteger` | `-` and the mask's digits (`9999`: 4; no mask: 10). | `IntGetValue` / `IntSetValue`. |
| `tbSuperBtn` | a themed push button. `SBM_SETPICT` loads a bitmap/icon resource (or takes a handle). Buttons styled `SBS_TEXTBITMAP`/`SBS_TEXTICON`, or whose text is "tbSuperBtn" (the toolbars), show only the picture. | `BN_CLICKED`. |

All four edits return `DLGC_WANTCHARS | DLGC_WANTARROWS` from `WM_GETDLGCODE` (spec §10 Q6). A rejected character beeps. Control characters (Backspace, Ctrl+C/V/X) always reach `EDIT`, so the user can always delete. Paste goes through the same check as typing.

**Toolbox API kept** (same names, same message numbers as `include/toolbox.h`): `EPM_SETMASK`, `EPM_GETMASK`, `EPM_SETCALLBACK` (`gl_accts.cpp` uses these on `ID_CURRBAL`), `FloatGetValue`, `FloatSetValue`, `IntGetValue`, `IntSetValue` and their `…DlgItem…` macros, and `SBM_SETPICT`. The other `SBM_*` messages are accepted and ignored.

**Not implemented** (nothing in Spec #1 needs them): `SBS_NOPOINTERFOCUS` (a click focuses the button), the button frame/shadow/colour styles, `IS_SPIN`/`DS_SPIN` arrows, `IntSetRange`/`FloatSetRange`, `DS_DDMONYY` (unused by the resources; read as MM/DD/YY), and the `EN_INVALIDDATA` notifications (no legacy code handles them). `tbSpread` and `tbStatusBar` (the TBOWL grid and status bar) are not registered. Spec #1 dialogs don't use them, and the grid is out of plan.

The input rules are plain C++ in `src/controls/EditRules.cpp` (no Win32). `src/controls/styles.md` lists the style words the Spec #1 dialogs use, decoded. It also records which ported screens turn out to depend on a particular bit.

## Reusability example

`examples/trivial_dialog/` is a standalone solution: it isn't in `gl.sln`, and it links the built `x64\<Config>\win32fx.lib` the way AP and AR will (acceptance §6.7 #9). Build `gl.sln` first, then `trivial_dialog.sln` for the same configuration.

- `trivial_dialog.exe` shows a dialog with all five controls.
- `trivial_dialog.exe /selftest` runs the two-instance dispatch test (plan risk P2) and drives the controls with `WM_SETTEXT`/`WM_CHAR`; it reports in a message box. Some rejected characters beep on purpose.
- Add `/quiet` to skip the message box: the exit code is the number of failures. From `cmd`, run `start /wait trivial_dialog.exe /selftest /quiet` and then `echo %errorlevel%`.
