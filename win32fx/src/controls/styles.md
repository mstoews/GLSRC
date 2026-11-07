# Toolbox style bits: what the ported screens depend on

Plan M4 task 7 creates this file. M5 and M6 fill it in (spec §6.1). Add a row when a ported screen is shown to depend on a style bit of a `tb*` control. "Shown" means the screen behaves wrongly without it, not just that the bit is set in `gl/res/gl.rc`.

What the bits *mean*, and how win32fx translates them, is in [`win32fx/README.md`](../../README.md#custom-controls). The values were decoded in M3 from `include/toolbox.h`. This file records which screens rely on which bits.

## Style words used by the Spec #1 dialogs

Each word is one control's style word without its `WS_*` bits. The number is written exactly as it appears in the template.

| Value | Hex | Control | Decoded | Dialogs |
|---|---|---|---|---|
| 1024 | 0x0400 | `tbPic` | `ES_3D` | 1010, 1021, 1028, 1033, 1034 |
| 1152 | 0x0480 | `tbPic` | `ES_3D \| ES_STATIC` | 2057 |
| 1024 | 0x0400 | `tbDate` | `ES_3D` (no format bits: read as MM/DD/YY) | 1159 |
| 25664 | 0x6440 | `tbDate` | `DS_MMDDYY \| ES_3D \| DS_SPIN` | 1028 |
| 9216 | 0x2400 | `tbFloat` | `ES_3D \| FS_SEPARATOR` | 1010 |
| 16 | 0x0010 | `tbSuperBtn` | `SBS_TEXTRIGHT_PICTLEFT` | 1021 |
| 32784 | 0x8010 | `tbSuperBtn` | `SBS_TEXTRIGHT_PICTLEFT \| SBS_NOPOINTERFOCUS` | 1010, 1028, 1033, 1034, 1035, 1158, 1159, 1160 |
| 32785 | 0x8011 | `tbSuperBtn` | `SBS_DEFPUSHBUTTON \| SBS_TEXTRIGHT_PICTLEFT \| SBS_NOPOINTERFOCUS` | 1158 |
| 40962 | 0xA002 | `tbSuperBtn` | `SBS_TEXTTOP_PICTBOT \| SBS_TEXTBITMAP \| SBS_NOPOINTERFOCUS` | 1032 |
| 49168 | 0xC010 | `tbSuperBtn` | `SBS_TEXTRIGHT_PICTLEFT \| SBS_TEXTTEXT \| SBS_NOPOINTERFOCUS` | 1010 |

`tbSpread` (1011, 1160) is not listed: win32fx doesn't register it.

Two bits here that win32fx ignores (README, "Not implemented") are worth checking first when a port misbehaves:
- `DS_SPIN`, on every date of Set Period Dates (1028).
- `SBS_NOPOINTERFOCUS`, on nearly every button.

## Dependencies found while porting

| Screen (port file) | Control ID | Bit | What breaks without it | Found in |
|---|---|---|---|---|
| | | | | |
