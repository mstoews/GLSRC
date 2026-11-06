#!/usr/bin/env python3
"""
Writes the dbf2sqlite test fixtures (plan M2): small, hand-built dBase IV
tables and production indexes. Deterministic: re-running produces identical
bytes. Run from anywhere:

    python3 dbf2sqlite_core/test/make_fixtures.py

Layout written:
  fixtures/company/      a valid company: the Spec #1 tables
  fixtures/bad/          tables the shim can't fully represent (exit 4 / --force)
  fixtures/corrupt_dbf/  truncated .dbf (exit 2)
  fixtures/corrupt_mdx/  .mdx whose tag table is garbage (exit 2)

The .mdx files follow the layout in dbf2sqlite_core/src/Source.h (CodeBase
S4MDX structures; tag slots at offset 544). That layout has since been checked
against a real company directory (2026-11-06: 45 tables, every .mdx read).
AR_JSUM and AR_BTCH copy the real tables' field layouts and tag definitions
(FOR clauses, RECNO() keys, a dBase IV memo); their rows are made up.
"""

import os
import shutil
import struct

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(HERE, "fixtures")
DATE = bytes([126, 11, 6])  # 2026-11-06 (year - 1900)
CP1252 = 0x57              # dBase language driver id: ANSI 1252


def dbf(path, fields, rows, *, ldid=CP1252, deleted=(), version=0x03, mdx=False):
    """fields: [(name, type, len, dec)]; rows: [tuple]; values are str / int /
    float / None, or bytes for a raw, already-formatted field."""
    header = bytearray(32)
    header[0] = version
    header[1:4] = DATE
    struct.pack_into("<I", header, 4, len(rows))
    struct.pack_into("<H", header, 8, 32 + 32 * len(fields) + 1)
    struct.pack_into("<H", header, 10, 1 + sum(f[2] for f in fields))
    header[28] = 1 if mdx else 0
    header[29] = ldid
    out = bytearray(header)
    for name, typ, length, dec in fields:
        d = bytearray(32)
        d[0:len(name)] = name.encode("ascii")
        d[11] = ord(typ)
        d[16] = length
        d[17] = dec
        out += d
    out += b"\x0d"
    for i, row in enumerate(rows):
        out += b"*" if i in deleted else b" "
        for (name, typ, length, dec), v in zip(fields, row):
            if isinstance(v, bytes):
                cell = v.ljust(length)
            elif v is None:
                cell = b" " * length
            elif typ == "C":
                cell = v.encode("cp1252").ljust(length)
            elif typ in "NF":
                cell = f"{v:.{dec}f}".rjust(length).encode("ascii")
            elif typ == "D":
                cell = v.encode("ascii")
            elif typ == "L":
                cell = v.encode("ascii")
            else:
                cell = str(v).encode("ascii").rjust(length)
            assert len(cell) == length, (path, name, cell)
            out += cell
    out += b"\x1a"
    with open(path, "wb") as f:
        f.write(out)


def dbt(path, dataname, memos):
    """dBase IV memo file, 512-byte blocks. memos: [bytes]; returns the
    10-byte block pointers to store in the .dbf, in the same order."""
    block = 512
    body = bytearray()
    pointers = []
    nxt = 1
    for m in memos:
        pointers.append(str(nxt).rjust(10).encode("ascii"))
        rec = b"\xff\xff\x08\x00" + struct.pack("<I", 8 + len(m)) + m
        rec += b"\x00" * (-len(rec) % block)
        body += rec
        nxt += len(rec) // block
    header = bytearray(block)
    struct.pack_into("<I", header, 0, nxt)               # next free block
    n = dataname.encode("ascii")[:8]
    header[8:8 + len(n)] = n
    struct.pack_into("<H", header, 20, block)
    with open(path, "wb") as f:
        f.write(header + body)
    return pointers


def mdx(path, dataname, tags):
    """tags: [(name, expression, keytype, unique, descending, filter)]"""
    buf = bytearray(2048 + 1024 * len(tags))
    buf[0] = 2
    buf[1:4] = DATE
    name = dataname.encode("ascii")[:12]
    buf[4:4 + len(name)] = name
    struct.pack_into("<H", buf, 20, 2)       # block_chunks: 1024-byte blocks
    struct.pack_into("<H", buf, 22, 1024)    # block_rw
    buf[24] = 1                              # production index
    buf[25] = 48                             # slots
    struct.pack_into("<H", buf, 26, 32)      # slot size
    struct.pack_into("<H", buf, 28, len(tags))
    struct.pack_into("<I", buf, 32, len(buf) // 512)
    for i, (tname, expr, keytype, unique, desc, filt) in enumerate(tags):
        slot = 544 + i * 32
        hdr = 2048 + 1024 * i
        struct.pack_into("<I", buf, slot, hdr // 512)
        n = tname.encode("ascii")
        buf[slot + 4:slot + 4 + len(n)] = n
        buf[slot + 20] = ord(keytype)
        struct.pack_into("<I", buf, hdr, hdr // 512 + 1)       # root page (not read)
        buf[hdr + 8] = 0x10 | (0x08 if desc else 0) | (0x40 if unique else 0)
        buf[hdr + 9] = ord(keytype)
        struct.pack_into("<H", buf, hdr + 12, 16)              # key length (not read)
        struct.pack_into("<H", buf, hdr + 22, 0x4000 if unique else 0)
        e = expr.encode("ascii")
        buf[hdr + 24:hdr + 24 + len(e)] = e
        if filt:
            fb = filt.encode("ascii")
            buf[hdr + 0x2FA:hdr + 0x2FA + len(fb)] = fb
    with open(path, "wb") as f:
        f.write(buf)


def company(d):
    n = lambda name: ("" + name, "N", 14, 2)
    glaccts = [("ACCOUNT", "C", 7, 0), ("DEPT", "C", 4, 0), ("DESCRIPT", "C", 30, 0),
               ("FDESCRIPT", "C", 30, 0), ("TYPE", "C", 1, 0), ("CURR_BAL", "N", 14, 2),
               ("CATEGORY", "C", 4, 0)]
    glaccts += [n(f"BUDGET{i}") for i in range(1, 13)]
    glaccts += [n(f"PERIOD{i}") for i in range(1, 13)]
    glaccts += [n("OPENBAL")]
    glaccts += [n(f"PREVIOUS{i}") for i in range(1, 13)]
    glaccts += [("UPDATE", "D", 8, 0), ("USER", "C", 10, 0), ("TIME", "C", 24, 0)]

    def acct(no, dept, desc, typ, bal, period3=None):
        row = [no, dept, desc, desc, typ, bal, "CA"]
        row += [None] * 12                                   # budgets blank
        row += [None, None, period3] + [None] * 9            # periods
        row += [bal]                                          # openbal
        row += [None] * 12                                    # previous
        row += ["20261106", "MSTOEWS", "Fri Nov 06 11:30:00 2026"]
        return tuple(row)

    dbf(os.path.join(d, "GLACCTS.DBF"), glaccts, [
        acct("3000", "01", "Sales revenue", "R", -5000.0),
        acct("1000", "01", "Caisse générale", "A", 1500.75, -20.5),
        acct("2000", "01", "Accounts payable", "L", -120.0),
        acct("1000", "02", "Petty cash", "A", 75.25),
        acct("9999", "99", "Obsolete account", "E", 0.0),   # deleted, not packed
    ], deleted={4}, mdx=True)
    mdx(os.path.join(d, "GLACCTS.MDX"), "GLACCTS", [
        ("ACCOUNT", "ACCOUNT+DEPT", "C", False, False, ""),
        ("ACCOUNTS", "ACCOUNT+DEPT", "C", False, False, ""),
        ("DEPT", "DEPT", "C", False, False, ""),
        ("TYPE", "TYPE", "C", False, False, ""),
        ("DESCRIP", "UPPER(DESCRIPT)", "C", False, False, ""),
    ])

    dbf(os.path.join(d, "GLDEPT.DBF"),
        [("DEPART", "C", 4, 0), ("DESCRIPT", "C", 30, 0), ("ACTIVE", "L", 1, 0)],
        [("02", "Branch office", "F"), ("01", "Head office", "T"), ("99", "Unassigned", "?")], mdx=True)
    mdx(os.path.join(d, "GLDEPT.MDX"), "GLDEPT", [("DEPART", "DEPART", "C", True, False, "")])

    dbf(os.path.join(d, "ACCESS.PSW"),
        [("USERNAME", "C", 10, 0), ("DESCRIPT", "C", 30, 0), ("DATE", "D", 8, 0),
         ("PASSWORD", "C", 10, 0), ("LEVEL", "C", 2, 0)],
        [("MSTOEWS", "Administrator", "19950301", "secret", "9"),
         ("CLERK", "Data entry", None, "pw", "1")], mdx=True)
    mdx(os.path.join(d, "ACCESS.MDX"), "ACCESS", [("U", "USERNAME", "C", False, False, "")])

    dbf(os.path.join(d, "ACCESSL.DBF"), [("LEVEL", "C", 2, 0), ("MENUCODE", "C", 6, 0)],
        [("9", "ALL"), ("1", "GLE")], mdx=True)
    mdx(os.path.join(d, "ACCESSL.MDX"), "ACCESSL", [("L", "LEVEL", "C", False, False, "")])

    dbf(os.path.join(d, "ACCESSM.DBF"),
        [("COUNT", "N", 5, 0), ("POPUP", "C", 40, 0), ("TEXT", "C", 40, 0), ("ID", "N", 6, 0)],
        [(2, "&File", "&Open company...", 1001), (10, "&File", "E&xit", 1099),
         (1, "&File", "&New company...", 1000)], mdx=True)
    mdx(os.path.join(d, "ACCESSM.MDX"), "ACCESSM", [("MENU", "STR(COUNT,5)", "C", False, False, "")])

    dbf(os.path.join(d, "GL_PROF.DBF"),
        [("COMPANY", "C", 40, 0), ("ADDRESS1", "C", 30, 0), ("ADDRESS2", "C", 30, 0),
         ("ADDRESS3", "C", 30, 0), ("POSTAL", "C", 10, 0), ("TELEPHONE", "C", 20, 0),
         ("FAX", "C", 20, 0), ("CONTACT", "C", 30, 0)],
        [("Sample Company Ltd.", "1 Main Street", "Suite 100", "Vancouver", "V6B 1A1",
          "604-555-0100", "604-555-0101", "M. Stoews")], ldid=0)

    # AR_JSUM: the real journal-summary layout. ARDIST/APDIST are the real FOR
    # tags; RECNO and LIVEAR add a RECNO() key and a compound condition.
    jsum = [("ACCOUNT", "C", 7, 0), ("DEPT", "C", 4, 0), ("PERIOD", "C", 4, 0), ("MODE", "C", 1, 0),
            ("DATE", "D", 8, 0), ("TYPE", "C", 3, 0), ("BATCHNO", "N", 6, 0), ("REFERENCE", "C", 30, 0),
            ("CHEQUE", "C", 7, 0), ("DESCRIPT", "C", 50, 0), ("DEBIT", "N", 16, 2), ("CREDIT", "N", 16, 2)]
    def j(acct, dept, typ, batch, desc, debit):
        return (acct, dept, "11", "1", "20261106", typ, batch, "REF", None, desc, debit, None)
    dbf(os.path.join(d, "AR_JSUM.DBF"), jsum, [
        j("1000", "01", "AR ", 5, "Invoice 5", 100.0),
        j("2000", "01", "AP ", 3, "Bill 3", 40.0),
        j("1000", "02", "AR ", 2, "Invoice 2", 25.5),
        j("3000", "01", "GL ", 1, "Journal 1", 10.0),
        j("1000", "01", "AR ", 7, "Invoice 7 (deleted)", 99.0),
        j("4000", "01", "   ", 4, "No type", 1.0),
    ], deleted={4}, mdx=True)
    mdx(os.path.join(d, "AR_JSUM.MDX"), "AR_JSUM", [
        ("ARDIST", "batchno", "N", False, False, "type = 'AR '"),
        ("APDIST", "batchno", "N", False, False, "type = 'AP '"),
        ("DESCRIPT", "descript", "C", False, False, ""),
        ("BATCHNO", "batchno", "N", False, False, ""),
        ("ACCOUNT", "account+dept", "C", False, False, ""),
        ("RECNO", "RECNO()", "N", False, False, ""),
        ("LIVEAR", "account+dept", "C", False, False, "TYPE='AR' .AND. .NOT. DELETED()"),
    ])

    # AR_BTCH: the real invoice-batch layout, with its dBase IV memo field.
    btch = [("BATCHNO", "N", 6, 0), ("MODE", "C", 1, 0), ("TYPE", "C", 2, 0), ("ADJ", "C", 1, 0),
            ("PERIOD", "N", 2, 0), ("VENDORNO", "C", 6, 0), ("INVOICENO", "C", 8, 0), ("ORDERNO", "C", 10, 0),
            ("ORDERDATE", "D", 8, 0), ("VIN", "C", 10, 0), ("UNITS", "C", 7, 0), ("DUEDATE", "D", 8, 0),
            ("INVDATE", "D", 8, 0), ("REFERENCE", "C", 40, 0), ("DESCRIPT", "C", 10, 0), ("DATE", "D", 8, 0),
            ("ACCOUNT", "C", 7, 0), ("DEPART", "C", 4, 0), ("PAYABLE", "N", 16, 2), ("REBATEAMT", "N", 16, 2),
            ("ENTRYNO", "N", 3, 0), ("VATPERCENT", "N", 2, 0), ("REBATE", "N", 2, 0), ("TERMS", "N", 3, 0),
            ("VATAMT", "N", 16, 2), ("ADJAMT", "N", 16, 2), ("MEMOFIELD", "M", 10, 0)]
    long_memo = ("".join(f"Line {i:03d} of a memo longer than one 512-byte block.\r\n" for i in range(20)))
    ptr = dbt(os.path.join(d, "AR_BTCH.DBT"), "AR_BTCH",
              ["Net 30 days.\r\nCafé on the corner".encode("cp1252"), long_memo.encode("ascii")])
    def b(batch, invoice, memo):
        return (batch, "1", "IN", "N", 11, "C100", invoice, None, None, None, None, "20261206", "20261106",
                "Invoice " + invoice, "Sales", "20261106", "1100", "01", 250.0, None, 1, 5, None, 30,
                12.5, None, memo)
    dbf(os.path.join(d, "AR_BTCH.DBF"), btch,
        [b(1, "INV001", ptr[0]), b(2, "INV002", None), b(3, "INV003", ptr[1])], version=0x8B, mdx=True)
    mdx(os.path.join(d, "AR_BTCH.MDX"), "AR_BTCH", [
        ("BATCHNO", "batchno", "N", False, False, ""),
        ("RECNO", "RECNO()", "N", False, False, ""),
    ])

    with open(os.path.join(d, "README.TXT"), "w", newline="\n") as f:
        f.write("Not a dBase file; the migrator must ignore it.\n")


def bad(d):
    dbf(os.path.join(d, "MEMOTAB.DBF"), [("ID", "C", 4, 0), ("NOTES", "M", 10, 0)],
        [("A1", b"         1")], version=0x8B)
    dbf(os.path.join(d, "DESCTAG.DBF"), [("NAME", "C", 10, 0)], [("b",), ("a",)], mdx=True)
    mdx(os.path.join(d, "DESCTAG.MDX"), "DESCTAG", [("NAME", "NAME", "C", False, True, "")])
    dbf(os.path.join(d, "FILTTAG.DBF"), [("NAME", "C", 10, 0), ("ACTIVE", "L", 1, 0)],
        [("a", "T"), ("b", "F")], mdx=True)
    mdx(os.path.join(d, "FILTTAG.MDX"), "FILTTAG", [("ACT", "NAME", "C", False, False, "SOUNDEX(NAME) = 'A000'")])
    dbf(os.path.join(d, "DUPUNIQ.DBF"), [("NAME", "C", 10, 0)], [("same",), ("same",), ("other",)], mdx=True)
    mdx(os.path.join(d, "DUPUNIQ.MDX"), "DUPUNIQ", [("NAME", "NAME", "C", True, False, "")])
    dbf(os.path.join(d, "BADEXPR.DBF"), [("NAME", "C", 10, 0)], [("abc",)], mdx=True)
    mdx(os.path.join(d, "BADEXPR.MDX"), "BADEXPR", [("PFX", "SUBSTR(NAME,1,3)", "C", False, False, "")])
    dbf(os.path.join(d, "WIDEN.DBF"), [("NAME", "C", 4, 0)], [("Café",), ("Tea",)])
    dbf(os.path.join(d, "BADVALS.DBF"), [("AMOUNT", "N", 8, 2), ("WHEN", "D", 8, 0)],
        [(b"   12a.0", b"2026XX01"), (1.5, "20261106")])
    with open(os.path.join(d, "NOTDBF.PSW"), "wb") as f:
        f.write(b"plain text, not a table\n" * 4)


def corrupt_dbf(d):
    dbf(os.path.join(d, "BROKEN.DBF"), [("NAME", "C", 10, 0)], [("a",), ("b",), ("c",)])
    with open(os.path.join(d, "BROKEN.DBF"), "r+b") as f:
        f.truncate(32 + 32 + 1 + 11)        # header + 1 record; header says 3
    # looksLikeDbf() rejects it, so name it explicitly with --table BROKEN


def corrupt_mdx(d):
    dbf(os.path.join(d, "T.DBF"), [("NAME", "C", 10, 0)], [("a",)], mdx=True)
    mdx(os.path.join(d, "T.MDX"), "T", [("NAME", "NAME", "C", False, False, "")])
    with open(os.path.join(d, "T.MDX"), "r+b") as f:
        f.seek(544 + 4)
        f.write(b"\x01\x02\x03\x04")         # non-printable tag name


def main():
    for name, fn in [("company", company), ("bad", bad), ("corrupt_dbf", corrupt_dbf),
                     ("corrupt_mdx", corrupt_mdx)]:
        d = os.path.join(OUT, name)
        shutil.rmtree(d, ignore_errors=True)
        os.makedirs(d)
        fn(d)
    print("fixtures written to", OUT)


if __name__ == "__main__":
    main()
