#!/usr/bin/env python3
"""Build gl/res/gl.rc, bitmaps/, icons/ and resource.h from the legacy
gl/gl_new.rc (primary: it is the one that matches the legacy sources) and
gl/gl.rc (bitmaps and strings that gl_new.rc lacks). Plan M4; spec 6.2.

The output is committed. Re-run this only to regenerate it, e.g. when a later
spec adds a dialog to DIALOGS. Hand edits to gl.rc made since are lost.

usage: python3 prune_rc.py <repo-root>
needs: a C preprocessor that understands -P -w -I, to resolve the symbolic IDs
       through gl/header (default x86_64-w64-mingw32-cpp; override with $CPP).
       On Windows, Visual Studio's LLVM tools have one:
         set CPP=C:\\Program Files\\Microsoft Visual Studio\\18\\Community\\VC\\Tools\\Llvm\\x64\\bin\\clang-cpp.exe
         python -I gl\\res\\tools\\prune_rc.py .

Besides the legacy resources, gl.rc #includes gl/port/PortIds.h (top) and
gl/res/port.rc (end): the port's own IDs and resources (spec #3 3), written
by hand. MENU_EDITS below changes the copied menus.
"""
import os, re, subprocess, sys

ROOT = sys.argv[1]
GL = os.path.join(ROOT, 'gl')
OUT = os.path.join(GL, 'res')
CPP = os.environ.get('CPP', 'x86_64-w64-mingw32-cpp')

# Spec #1 dialogs (spec 6.2 as corrected in M4), all taken from gl_new.rc,
# then Spec #2's as each milestone adds them (S2-M2: 1006, 1007, 1023, 1040;
# S2-M3: 1013; S2-M4: 1061, 1015, 1043; S2-M5: 1029, 1017). gl_new.rc's only complete copy of
# 1061 is missing (its second copy has no closing brace and no Close button),
# so a dialog gl_new.rc lacks is taken from gl.rc, and the note says so.
DIALOGS = ['1008', '1010', '1011', '1021', '1028', '1032', '1033', '1034', '1035',
           '2057', '1158', '1159', '1160',
           '1006', '1007', '1023', '1040', '1013',
           '1061', '1015', '1043', '1029', '1017',
           '1022',   # Spec #3 S3-M7: the report list (D9)
           '4002',   # Spec #4 S4-M1: Asset Types
           '4004']   # Spec #4 S4-M2: Fixed Assets Add/Modify
MENUS = ['MANILA', 'GLE', 'GLSUB']

# The port's changes to those menus (spec #3 3: D13 and the new items), made
# here so gl.rc stays generated. Each edit names a menu and the ID of an item
# in it (a number, or a symbol from the legacy headers or gl/port/PortIds.h),
# and must match exactly one item there, or the script stops:
#   (menu, 'rename', id, '"&New text"')         replaces the item's text
#   (menu, 'remove', id)                        drops the item
#   (menu, 'insert_after', id, '"&Text", ID')   adds MENUITEM "&Text", ID after it
#   (menu, 'set_id', id, 'NEW_ID', '"&Text"')   gives the item with that ID and text a new ID
# Items the port adds take IDs from gl/port/PortIds.h, which gl.rc #includes.
MENU_EDITS = [
    # Spec #3 D13 (S3-M3): Financial Statements runs F3, F4 and F5.
    ('MANILA', 'rename', '1018', '"&Cash flow statement..."'),
    ('MANILA', 'remove', '1025'),
    # Spec #3 D9 (S3-M7): Custom Report always opens the report list; File >
    # Print and the toolbar keep 3068, which prints the active preview first.
    ('MANILA', 'set_id', '3068', 'CM_REPORT_LIST', '"&Custom Report..."'),
    ('GLSUB', 'set_id', '3068', 'CM_REPORT_LIST', '"&Custom Report..."'),
]

STRIP = re.compile(r'\s+\b(LOADONCALL|PRELOAD|MOVEABLE|FIXED|DISCARDABLE|PURE|IMPURE)\b', re.I)
HEAD = re.compile(r'^(\S+)\s+(BITMAP|ICON|CURSOR|MENU|DIALOG|ACCELERATORS)\b(.*)$', re.I)


TRUNCATED = []


def parse(path):
    """-> list of dict(name, type, head, lines, src_line)."""
    lines = open(path, encoding='ascii').read().split('\n')
    lines = [l.rstrip('\r') for l in lines]
    res, i = [], 0
    while i < len(lines):
        l = lines[i]
        m = HEAD.match(l)
        st = re.match(r'^STRINGTABLE\b', l)
        if not (m or st):
            i += 1
            continue
        start = i
        block = [l]
        i += 1
        while i < len(lines) and not re.match(r'^\s*(\{|BEGIN\b)', lines[i]):
            block.append(lines[i]); i += 1
        depth = 0
        while i < len(lines):
            t = lines[i].strip()
            if depth > 0 and (HEAD.match(lines[i]) or re.match(r'^STRINGTABLE\b', lines[i])):
                TRUNCATED.append(f'{os.path.basename(path)}:{start + 1} {l.split()[0]} has no closing brace '
                                 f'(next resource starts at line {i + 1}); block dropped')
                block = None
                break
            block.append(lines[i])
            if t == '{' or t.upper() == 'BEGIN':
                depth += 1
            elif t == '}' or t.upper() == 'END':
                depth -= 1
            i += 1
            if depth == 0:
                break
        if block is None:
            continue
        res.append(dict(name=m.group(1) if m else None, type=(m.group(2) if m else 'STRINGTABLE').upper(),
                        lines=block, src_line=start + 1))
    return res


def hexbytes(block):
    out = bytearray()
    for l in block:
        for q in re.findall(r"'([0-9A-Fa-f ]*)'", l):
            out += bytes.fromhex(q)
    return bytes(out)


def resolve(symbols):
    """Expand candidate identifiers through the headers gl_new.rc includes."""
    probe = ''.join(f'#include "{h}"\n' for h in ('gl_manil.h', 'gl.h', 'ar_id.h', 'ar_clear.h'))
    symbols = list(symbols)
    probe += ''.join(f'@@{n}@@ {s}\n' for n, s in enumerate(symbols))
    r = subprocess.run([CPP, '-P', '-w', '-I', os.path.join(GL, 'header'), '-'], input=probe,
                       capture_output=True, text=True, check=True)
    vals = {}
    for l in r.stdout.splitlines():
        m = re.match(r'@@(\d+)@@\s+(.*)$', l)
        if m and m.group(2).strip() != symbols[int(m.group(1))]:
            vals[symbols[int(m.group(1))]] = m.group(2).strip()
    return vals


def num(v):
    try:
        return int(eval(v, {}, {}))
    except Exception:
        return None


new = parse(os.path.join(GL, 'gl_new.rc'))
old = parse(os.path.join(GL, 'gl.rc'))

# ---- pick resources -------------------------------------------------------
def first(items, typ, name):
    hits = [r for r in items if r['type'] == typ and r['name'] and r['name'].upper() == name.upper()]
    return hits

dialogs, notes = [], []
for d in DIALOGS:
    hits = first(new, 'DIALOG', d)
    src = 'gl_new.rc'
    if not hits:
        hits, src = first(old, 'DIALOG', d), 'gl.rc'
        if not hits:
            sys.exit(f'dialog {d} missing from gl_new.rc and gl.rc')
        notes.append(f'dialog {d}: not complete in gl_new.rc; taken from gl.rc line {hits[0]["src_line"]}')
    for h in hits:
        h['src'] = src
    if len(hits) > 1:
        same = all(h['lines'][1:] == hits[0]['lines'][1:] for h in hits)
        notes.append(f'dialog {d}: {len(hits)} copies in {src} (lines '
                     + ', '.join(str(h['src_line']) for h in hits) + f'); identical={same}; first used')
    dialogs.append(hits[0])
menus = []
for mname in MENUS:
    hits = first(new, 'MENU', mname)
    if len(hits) != 1:
        sys.exit(f'menu {mname}: {len(hits)} copies')
    menus.append(hits[0])
accels = [r for r in new if r['type'] == 'ACCELERATORS']
icons = [r for r in new if r['type'] == 'ICON']

# candidate symbols: every identifier outside strings in the chosen text
def idents(text):
    text = re.sub(r'"(?:[^"]|"")*"', '""', text)
    return set(re.findall(r'\b[A-Za-z_]\w*\b', text))

bitmaps_new = [r for r in new if r['type'] == 'BITMAP']
bitmaps_old = [r for r in old if r['type'] == 'BITMAP']
strings_new = [r for r in new if r['type'] == 'STRINGTABLE']
strings_old = [r for r in old if r['type'] == 'STRINGTABLE']
cands = set()
for r in dialogs + menus + accels + strings_new + strings_old:
    cands |= idents('\n'.join(r['lines']))
cands |= {r['name'] for r in bitmaps_new + bitmaps_old + icons}
vals = resolve(sorted(cands))

def key(name):
    v = vals.get(name)
    n = num(v) if v is not None else num(name)
    return n if n is not None else name.upper()

# ---- bitmaps: union keyed by resolved ID, gl_new.rc wins ------------------
os.makedirs(os.path.join(OUT, 'bitmaps'), exist_ok=True)
os.makedirs(os.path.join(OUT, 'icons'), exist_ok=True)
chosen, seen = [], {}
for src, items in (('gl_new.rc', bitmaps_new), ('gl.rc', bitmaps_old)):
    for r in items:
        k = key(r['name'])
        data = hexbytes(r['lines'])
        if k in seen:
            if seen[k][1] != data and seen[k][0] != src:
                notes.append(f"bitmap {r['name']} differs between files; gl_new.rc kept")
            elif seen[k][0] == src:
                notes.append(f"bitmap {r['name']} duplicated in {src}; first kept")
            continue
        seen[k] = (src, data)
        chosen.append((r['name'], data, src))

def check_bmp(name, data):
    if data[:2] != b'BM':
        sys.exit(f'bitmap {name}: no BM signature')
    size = int.from_bytes(data[2:6], 'little')
    if size != len(data):
        notes.append(f'bitmap {name}: header size {size} != {len(data)} bytes (file written as stored)')

bitmap_lines = []
for name, data, src in chosen:
    check_bmp(name, data)
    fn = f'{name}.bmp'
    open(os.path.join(OUT, 'bitmaps', fn), 'wb').write(data)
    bitmap_lines.append(f'{name} BITMAP "bitmaps/{fn}"' + ('' if src == 'gl_new.rc' else '   // only in gl.rc'))

icon_lines = []
for r in icons:
    data = hexbytes(r['lines'])
    if data[:4] != b'\x00\x00\x01\x00':
        sys.exit(f"icon {r['name']}: not an .ico")
    fn = f"{r['name']}.ico"
    open(os.path.join(OUT, 'icons', fn), 'wb').write(data)
    icon_lines.append(f'{r["name"]} ICON "icons/{fn}"')

# ---- strings: gl_new.rc, plus gl.rc entries whose ID gl_new.rc lacks ------
def entries(blocks):
    out = []
    for b in blocks:
        for l in b['lines']:
            m = re.match(r'^\s*([\w]+)\s*,\s*(".*")\s*$', l)
            if m:
                out.append((m.group(1), m.group(2)))
    return out

str_new, str_old = entries(strings_new), entries(strings_old)
ids, final_new = {}, []
for sid, s in str_new:
    k = key(sid)
    if k in ids:
        notes.append(f'string {sid}: duplicate in gl_new.rc; first kept')
        continue
    ids[k] = sid
    final_new.append((sid, s))
extra = []
for sid, s in str_old:
    k = key(sid)
    if k not in ids:
        ids[k] = sid
        extra.append((sid, s))
nstr_raw = sum(1 for b in strings_new for l in b['lines'] if re.match(r'^\s*\w+\s*,\s*"', l))
if nstr_raw != len(str_new):
    sys.exit('string parse mismatch')

# ---- emit -----------------------------------------------------------------
def clean(block):
    out = [STRIP.sub('', block[0]).rstrip()] + [l.rstrip() for l in block[1:]]
    return out

# A control line's ID: the first field, or the second when the first is its text.
CTL_ID = re.compile(r'^\s*[A-Z]+\s+(?:"(?:[^"]|"")*"\s*,\s*)?([\w]+|-1)\s*,')

def dedupe_controls(r):
    """Drop a dialog's controls whose ID repeats an earlier one's (rc.exe RC2182).
    GetDlgItem finds only one control per ID, so the rest are unreachable. A
    control written with a symbolic ID is kept over one with the same number
    written as a literal, so the symbol stays in resource.h."""
    lines = r['lines']
    # Controls only: skip the header (STYLE, CAPTION, FONT 8, "..." would match CTL_ID).
    body_start = next(i for i, l in enumerate(lines) if re.match(r'^\s*(\{|BEGIN\b)', l, re.I)) + 1
    by_id = {}
    for i, l in enumerate(lines[body_start:], body_start):
        m = CTL_ID.match(l)
        if m and m.group(1) not in ('-1', 'IDC_STATIC'):
            by_id.setdefault(key(m.group(1)), []).append((i, m.group(1)))
    drop = set()
    for k, hits in by_id.items():
        if len(hits) < 2:
            continue
        keep = next((h for h in hits if num(h[1]) is None), hits[0])
        for h in hits:
            if h is not keep:
                drop.add(h[0])
                notes.append(f"dialog {r['name']}: control ID {h[1]} (= {k}) also on {r['src']} line "
                             f"{r['src_line'] + keep[0]} as {keep[1]}; the copy on line {r['src_line'] + h[0]} dropped")
    return [l for i, l in enumerate(lines) if i not in drop]

MENU_ITEM = re.compile(r'^(\s*)MENUITEM\s+("(?:[^"]|"")*")\s*,\s*(\w+)(.*)$')

def apply_menu_edits(r):
    """MENU_EDITS for menu r, on its cleaned lines."""
    lines = list(r['lines'])
    for edit in MENU_EDITS:
        if edit[0].upper() != r['name'].upper():
            continue
        op, target = edit[1], edit[2]
        hits = [i for i, l in enumerate(lines)
                if (m := MENU_ITEM.match(l)) and key(m.group(3)) == key(target)
                and (op != 'set_id' or m.group(2) == edit[4])]
        if len(hits) != 1:
            sys.exit(f'menu edit {edit}: {len(hits)} items with ID {target} in {r["name"]}')
        i = hits[0]
        m = MENU_ITEM.match(lines[i])
        if op == 'rename':
            lines[i] = f'{m.group(1)}MENUITEM {edit[3]}, {m.group(3)}{m.group(4)}'
        elif op == 'remove':
            del lines[i]
        elif op == 'set_id':
            lines[i] = f'{m.group(1)}MENUITEM {m.group(2)}, {edit[3]}{m.group(4)}'
        elif op == 'insert_after':
            lines.insert(i + 1, f'{m.group(1)}MENUITEM {edit[3]}')
        else:
            sys.exit(f'menu edit {edit}: unknown operation {op}')
        notes.append(f'menu {r["name"]}: {op} {target}' + (f' -> {edit[3]}' if len(edit) > 3 else ''))
    return lines

used = set()
body = []
body += ['// ---- bitmaps (extracted from the hex blocks; see the M4 progress-log entry)'] + bitmap_lines + ['']
body += ['// ---- icons'] + icon_lines + ['']
for r in accels:
    body += clean(r['lines']) + ['']
for r in menus:
    body += [f'// ---- menu {r["name"]} (gl_new.rc line {r["src_line"]})'] + apply_menu_edits(
        {**r, 'lines': clean(r['lines'])}) + ['']
for r in dialogs:
    body += [f'// ---- dialog {r["name"]} ({r["src"]} line {r["src_line"]})'] + clean(dedupe_controls(r)) + ['']
body += ['// ---- strings (gl_new.rc)', 'STRINGTABLE', '{'] + [f' {i}, {s}' for i, s in final_new] + ['}', '']
if extra:
    body += ['// ---- strings only in gl.rc', 'STRINGTABLE', '{'] + [f' {i}, {s}' for i, s in extra] + ['}', '']
body += ["// ---- the port's own resources (spec #3 3), written by hand", '#include "port.rc"', '']

text = '\n'.join(body)
used = sorted(s for s in idents(text) if s in vals)
hdr = [
    '//',
    '// gl.rc - the Spec #1 resources of gl.exe (spec 6.2), generated in plan M4',
    '// from the legacy gl/gl_new.rc (and gl/gl.rc for bitmaps and strings that',
    '// gl_new.rc lacks). Templates, IDs, styles and strings are copied verbatim;',
    '// only the 16-bit load attributes are gone and the hex bitmaps/icons are now',
    '// files. UTF-16 LE with BOM (spec 5.3); edit as text, not in the VS',
    '// resource editor (spec risk 11).',
    '//',
    '#include <windows.h>',
    '#include "resource.h"',
    '#include "..\\port\\PortIds.h"   // the IDs the port adds (spec #3 3): port.rc, menu edits',
    '',
    '// The application manifest is added by the linker (gl.vcxproj).',
    '',
]
# UTF-16 LE with BOM (spec 5.3)
with open(os.path.join(OUT, 'gl.rc'), 'w', encoding='utf-16-le', newline='\r\n') as f:
    f.write('\ufeff' + '\n'.join(hdr) + '\n' + text)
rh = ['//', '// resource.h - the symbolic IDs gl.rc uses, with the values they have in the',
      '// legacy headers gl_new.rc included (gl_manil.h, gl.h, ar_id.h, ar_clear.h).',
      '// Generated in plan M4.', '//', '#pragma once', '']
rh += [f'#define {s:<24} {vals[s]}' for s in used]
open(os.path.join(OUT, 'resource.h'), 'w', encoding='ascii', newline='\r\n').write('\n'.join(rh) + '\n')

print(f'bitmaps {len(chosen)} ({sum(1 for c in chosen if c[2] == "gl.rc")} only in gl.rc), icons {len(icons)}, '
      f'menus {len(menus)}, dialogs {len(dialogs)}, strings {len(final_new)}+{len(extra)}, symbols {len(used)}')
for n in TRUNCATED + notes:
    print('note:', n)
