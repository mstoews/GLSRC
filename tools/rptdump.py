#!/usr/bin/env python3
"""Dump the readable records of Crystal Reports 3/4 .RPT files (spec #3 2.2).

Usage: python3 -I tools/rptdump.py dbf/GLTB.RPT [more.RPT ...]

An .RPT file is a run of tag-length-value records: a 2-byte little-endian
tag, a 2-byte length, then NUL-terminated text. Known tags:

  0x1019 TITLE     report title (PEGetReportTitle)
  0x100f SELECT    selection formula (record, then group)
  0x100d FNAME     formula name; with no FTEXT after it, a reference to it
  0x100e FTEXT     formula text
  0x1008 FIELDREF  database field reference (table.FIELD), in file order:
                   page header, details, group sections, report footer
  0x1018 TEXT      text object on the page
  0x2004 / 0x2005 / 0x2006 / 0x2011   .dbf file / directory / DLL / alias
  0x2008 FIELD     field in a table definition
  0x200e / 0x200f  link from-table, from-field(s), to-table

Layout positions and fonts are binary and are not decoded. Font names are
skipped. Unknown tags print as hex.
"""
import struct
import sys

NAMES = {0x1019: "TITLE", 0x100F: "SELECT", 0x100D: "FNAME", 0x100E: "FTEXT",
         0x1008: "FIELDREF", 0x1018: "TEXT", 0x2004: "DBF", 0x2005: "DIR",
         0x2006: "DLL", 0x2011: "ALIAS", 0x2008: "FIELD", 0x200E: "LINKTABLE",
         0x200F: "LINKFIELD", 0x2016: "MDX", 0x2017: "TAG"}
FONTS = {b"Courier New", b"Arial", b"Times New Roman", b"MS Sans Serif"}


def records(data):
    """Yield (offset, tag, text) for each record that looks like text."""
    o = 0
    while o < len(data) - 4:
        tag, ln = struct.unpack_from("<HH", data, o)
        end = o + 4 + ln
        if (2 <= ln < 2000 and end <= len(data) and data[end - 1] == 0
                and data[o + 1] in (0x10, 0x20, 0x30)):
            s = data[o + 4:end - 1]
            if s and all(32 <= c < 127 or c in (9, 10, 13) for c in s):
                yield o, tag, s
                o = end
                continue
        o += 1


def main(paths):
    for p in paths:
        with open(p, "rb") as f:
            data = f.read()
        print(f"##### {p} ({len(data)} bytes)")
        for o, tag, s in records(data):
            if s in FONTS:
                continue
            text = s.decode("latin1").replace("\r\n", "\n").replace("\n", "\n      ")
            print(f"{o:05x} {NAMES.get(tag, f'{tag:04x}')}: {text}")


if __name__ == "__main__":
    if len(sys.argv) < 2:
        sys.exit(__doc__)
    main(sys.argv[1:])
