#!/usr/bin/env python3
"""Build gl/res/gl.rc, bitmaps/, icons/ and resource.h from the legacy
gl/gl_new.rc (primary: it is the one that matches the legacy sources) and
gl/gl.rc (bitmaps and strings that gl_new.rc lacks). Plan M4; spec 6.2.

The output is committed. Re-run this only to regenerate it, e.g. when a later
spec adds a dialog to DIALOGS. Hand edits to gl.rc made since are lost.

usage: python3 prune_rc.py <repo-root>
needs: a C preprocessor that understands -P -w -I, to resolve the symbolic IDs
       through gl/header (default x86_64-w64-mingw32-cpp; override with $CPP).
"""
import os, re, subprocess, sys

ROOT = sys.argv[1]
GL = os.path.join(ROOT, 'gl')
OUT = os.path.join(GL, 'res')
CPP = os.environ.get('CPP', 'x86_64-w64-mingw32-cpp')

# Spec #1 dialogs (spec 6.2 as corrected in M4), all taken from gl_new.rc.
DIALOGS = ['1008', '1010', '1011', '1021', '1028', '1032', '1033', '1034', '1035',
           '2057', '1158', '1159', '1160']
MENUS = ['MANILA', 'GLE', 'GLSUB']
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
    if not hits:
        sys.exit(f'dialog {d} missing from gl_new.rc')
    if len(hits) > 1:
        same = all(h['lines'][1:] == hits[0]['lines'][1:] for h in hits)
        notes.append(f'dialog {d}: {len(hits)} copies in gl_new.rc (lines '
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

used = set()
body = []
body += ['// ---- bitmaps (extracted from the hex blocks; see the M4 progress-log entry)'] + bitmap_lines + ['']
body += ['// ---- icons'] + icon_lines + ['']
for r in accels:
    body += clean(r['lines']) + ['']
for r in menus:
    body += [f'// ---- menu {r["name"]} (gl_new.rc line {r["src_line"]})'] + clean(r['lines']) + ['']
for r in dialogs:
    body += [f'// ---- dialog {r["name"]} (gl_new.rc line {r["src_line"]})'] + clean(r['lines']) + ['']
body += ['// ---- strings (gl_new.rc)', 'STRINGTABLE', '{'] + [f' {i}, {s}' for i, s in final_new] + ['}', '']
if extra:
    body += ['// ---- strings only in gl.rc', 'STRINGTABLE', '{'] + [f' {i}, {s}' for i, s in extra] + ['}', '']

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
