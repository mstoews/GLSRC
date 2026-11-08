#!/usr/bin/env python3
"""
Writes the gl.exe dev fixture (plan M5 task 2): a small company directory of
dBase IV tables that dbf2sqlite turns into a database gl.exe can log in to.
Deterministic: re-running produces identical bytes.

    python3 gl/test/dev-fixture/make_dev_fixture.py

Writes gl/test/dev-fixture/dbf/. Bootstrap steps: gl/README.md.

The field layouts and index tags copy the real company directory's tables
(checked 2026-11-07): ACCESS.PSW (the layout of the real ACCESS.DBF), GL_PROF,
GL_PDS, GL_SETAT, GLACCTS, GLDEPT. The real directory has no ACCESS.PSW,
ACCESSL or ACCESSM (its GL.INI has [ACCESS] PSW=F, so it never logged in);
ACCESSM here is built from the MANILA menu in gl/res/gl.rc, the way the
commented-out TGLAdministration::Reset code in gl_acces.cpp built it. The
rows are made up.

Users (name / password / level):
    ADMIN / ADMIN / A   nothing grayed
    CLERK / clerk / B   the Maintenance menu grayed
"""

import importlib.util
import os
import re
import shutil
import sys

sys.dont_write_bytecode = True   # importing make_fixtures must not leave __pycache__ in the repo

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.normpath(os.path.join(HERE, "..", "..", ".."))
OUT = os.path.join(HERE, "dbf")

# The dBase / .mdx writers of the dbf2sqlite fixtures.
_spec = importlib.util.spec_from_file_location(
    "make_fixtures", os.path.join(ROOT, "dbf2sqlite_core", "test", "make_fixtures.py"))
mf = importlib.util.module_from_spec(_spec)
_spec.loader.exec_module(mf)
dbf, mdx = mf.dbf, mf.mdx


def manila_items():
    """[(popup, text, id)] for every command in MANILA, in menu order. A
    nested popup's items carry the nested popup's name (as Reset did)."""
    with open(os.path.join(ROOT, "gl", "res", "gl.rc"), encoding="utf-16") as f:
        text = f.read()
    start = text.index("\nMANILA MENU")
    lines = text[start:].splitlines()[2:]
    items, stack, depth = [], [], 0
    for line in lines:
        s = line.strip()
        m = re.match(r'POPUP "(.*)"', s)
        if m:
            stack.append(m.group(1).replace("&", ""))
            continue
        if s == "{":
            depth += 1
            continue
        if s == "}":
            depth -= 1
            if depth == 0:
                break
            stack.pop()
            continue
        m = re.match(r'MENUITEM "(.*)", (\d+)', s)
        if m:
            items.append((stack[-1], m.group(1).replace("&", ""), int(m.group(2))))
    return items


def vouchers(d):
    """Spec #2 7: the voucher tables and the master data voucher entry reads.

    Layouts and tags copy the real GL_BTCH, GL_BTCHD, GL_DIST, GL_DISTB,
    GL_MEDRP, GL_DRCOD and GL_ITEMM (checked 2026-11-09); the rows are made up.
    gl_btch.mode: "1" open, "0" posted, "3" adjustment (Tglbtch::Open /
    Closed / Adjustment, gl_btche.cpp). Value formats follow the real rows:
    gl_btchd.period is the period number right-aligned in C(2) (" 7"), and
    gl_dist copies it and the date into wider fields, left-aligned.
    """
    # Master data: medical reps, doctors (the same layout, keyed by repcode /
    # drcode), products.
    def party(code, short, name, territory):
        return (code, short, name, "1 Clinic Road", None, "Vancouver BC", "V6B1A1", "(604) 555 - 0199",
                territory, None, None, None, "A", None, "ACT", 30, 0.0, 0.0)
    party_fields = lambda key: [
        (key, "C", 5, 0), ("RPSHORT", "C", 12, 0), ("RPNAME", "C", 40, 0), ("ADDRESS1", "C", 30, 0),
        ("ADDRESS2", "C", 30, 0), ("ADDRESS3", "C", 30, 0), ("POSTAL", "C", 7, 0), ("PHONE", "C", 16, 0),
        ("TERRITORY", "C", 10, 0), ("FAX", "C", 16, 0), ("DESCRIPT", "C", 40, 0), ("CONTACT", "C", 12, 0),
        ("TYPE", "C", 1, 0), ("REF", "C", 6, 0), ("STATUS", "C", 3, 0), ("TERMS", "N", 3, 0),
        ("BALANCE", "N", 16, 2), ("CREDITLMT", "N", 16, 2)]
    dbf(os.path.join(d, "GL_MEDRP.DBF"), party_fields("REPCODE"), [
        party("MR001", "TANAKA", "Hana Tanaka", "NORTH"),
        party("MR002", "LEE", "Jordan Lee", "SOUTH"),
        party("MR003", "PATEL", "Sam Patel", "EAST"),
    ], mdx=True)
    mdx(os.path.join(d, "GL_MEDRP.MDX"), "GL_MEDRP", [
        ("RPNAME", "rpname", "C", False, False, ""),
        ("REPCODE", "repcode", "C", False, False, ""),
    ])
    dbf(os.path.join(d, "GL_DRCOD.DBF"), party_fields("DRCODE"), [
        party("DR001", "MORGAN", "Dr. Alex Morgan", "NORTH"),
        party("DR002", "CHEN", "Dr. Robin Chen", "SOUTH"),
        party("DR003", "SINGH", "Dr. Kim Singh", "EAST"),
    ], mdx=True)
    mdx(os.path.join(d, "GL_DRCOD.MDX"), "GL_DRCOD", [
        ("DRCODE", "drcode", "C", False, False, ""),
        ("RPNAME", "rpname", "C", False, False, ""),
    ])
    n3 = lambda name, w: (name, "N", w, 3)
    dbf(os.path.join(d, "GL_ITEMM.DBF"),
        [("FPCODE", "C", 7, 0), ("FPNAME", "C", 10, 0), ("FFULLN", "C", 40, 0), ("FPTYPE", "C", 2, 0),
         ("FELEMENT", "C", 4, 0), ("FCLASS", "C", 4, 0), ("FSKBN", "C", 3, 0), ("FSHOP1", "C", 4, 0),
         ("FSHOP2", "C", 4, 0), ("FPLACE", "C", 4, 0), ("FUNIT", "C", 3, 0), n3("FLOTSIZE", 11),
         n3("FONHAND", 11), n3("FLMHAND", 11), n3("FLMPRICE", 14), n3("FAVPRICE", 14), n3("FSTPRICE", 14)],
        [("P100", "ANALGESIC", "Analgesic tablets 100mg", "FG", None, "A1", None, None, None, None, "BOX",
          10.0, 120.0, 100.0, 4.5, 4.25, 4.0),
         ("P200", "SYRUP", "Cough syrup 200ml", "FG", None, "B2", None, None, None, None, "BTL",
          24.0, 60.0, 48.0, 6.75, 6.5, 6.0),
         ("P300", "OINTMENT", "Topical ointment 30g", "FG", None, "C3", None, None, None, None, "TUB",
          12.0, 36.0, 30.0, 3.2, 3.1, 3.0)], mdx=True)
    mdx(os.path.join(d, "GL_ITEMM.MDX"), "GL_ITEMM", [
        ("FFULLN", "ffulln", "C", False, False, ""),
        ("FPCODE", "fpcode", "C", False, False, ""),
    ])

    # Vouchers. 100 is posted (its gl_dist rows are 101000's and 102101's
    # period-7 activity in GLACCTS), 101 is open and balanced, 102 open and out
    # of balance, 103 an open adjustment.
    date = "20261015"
    hdr = [("BATCHNO", "N", 6, 0), ("MODE", "C", 1, 0), ("REFERENCE", "C", 40, 0), ("PERIOD", "N", 2, 0),
           ("DATE", "D", 8, 0), ("DEBIT", "N", 14, 2), ("CREDIT", "N", 14, 2), ("ENTRYNO", "N", 3, 0),
           ("CHEQUE", "C", 7, 0)]
    dbf(os.path.join(d, "GL_BTCH.DBF"), hdr, [
        (100, "0", "Cash to checking", 7, date, 250.0, 250.0, 2, "000123"),
        (101, "1", "October sales", 7, date, 1150.0, 1150.0, 3, None),
        (102, "1", "VAT, out of balance", 7, date, 100.0, 90.0, 2, None),
        (103, "3", "Opening balance correction", 7, date, 75.0, 75.0, 2, None),
    ], mdx=True)
    mdx(os.path.join(d, "GL_BTCH.MDX"), "GL_BTCH", [
        ("REFERENCE", "reference", "C", False, False, ""),
        ("RECNO", "recno()", "N", False, False, ""),
        ("BATCHNO", "batchno", "N", False, False, ""),
    ])

    # (batchno, mode, line, account, dept, medrep, drcode, prod, descript, debit, credit).
    # AppendDetail never sets a line's mode, so open lines have a blank one;
    # posting sets "0" (PostGLSlip), which Transaction Inquiry selects on.
    lines = [
        (100, "0", 1, "102101", None, None, None, None, "Transfer from petty cash", 250.0, 0.0),
        (100, "0", 2, "101000", None, None, None, None, "Transfer to checking", 0.0, 250.0),
        (101, None, 1, "111101", None, None, None, None, "Invoices October", 1150.0, 0.0),
        (101, None, 2, "410001", "10", "MR001", "DR001", "P100", "Analgesic sales", 0.0, 600.0),
        (101, None, 3, "410001", "20", "MR002", "DR002", "P200", "Syrup sales", 0.0, 550.0),
        (102, None, 1, "148000", None, None, None, None, "Prepaid VAT", 100.0, 0.0),
        (102, None, 2, "260007", None, None, None, None, "VAT payable", 0.0, 90.0),
        (103, None, 1, "144910", None, None, None, None, "Correct transit", 75.0, 0.0),
        (103, None, 2, "111101", None, None, None, None, "Correct receivables", 0.0, 75.0),
    ]
    dbf(os.path.join(d, "GL_BTCHD.DBF"),
        [("BATCHNO", "N", 6, 0), ("MODE", "C", 1, 0), ("JOURNAL", "N", 6, 0), ("PERIOD", "C", 2, 0),
         ("SOURCECODE", "C", 6, 0), ("DATE", "D", 8, 0), ("REFERENCE", "C", 40, 0), ("DESCRIPT", "C", 40, 0),
         ("CHEQUE", "C", 7, 0), ("ACCOUNT", "C", 7, 0), ("DEPT", "C", 4, 0), ("MEDREP", "C", 5, 0),
         ("DRCODE", "C", 5, 0), ("PROD", "C", 7, 0), ("DEBIT", "N", 14, 2), ("CREDIT", "N", 14, 2)],
        [(b, m, ln, " 7", None, date, None, desc, None, acc, dp, mr, dr, pr, dr_, cr)
         for (b, m, ln, acc, dp, mr, dr, pr, desc, dr_, cr) in lines], mdx=True)
    mdx(os.path.join(d, "GL_BTCHD.MDX"), "GL_BTCHD", [
        ("JOURNAL", "journal", "N", False, False, ""),
        ("DEPT", "dept", "C", False, False, ""),
        ("ACCOUNT", "account", "C", False, False, ""),
        ("BATCHNO", "batchno", "N", False, False, ""),
        ("ACCTDEPT", "account+dept", "C", False, False, ""),
    ])

    dist_tags = [
        ("ARDIST", "batchno", "N", False, False, "type = 'AR '"),
        ("APDIST", "batchno", "N", False, False, "type = 'AP '"),
        ("DESCRIPT", "descript", "C", False, False, ""),
        ("BATCHNO", "batchno", "N", False, False, ""),
        ("ACCOUNT", "account+dept", "C", False, False, ""),
    ]
    dbf(os.path.join(d, "GL_DIST.DBF"),
        [("ACCOUNT", "C", 7, 0), ("DEPT", "C", 4, 0), ("MEDREP", "C", 5, 0), ("DRCODE", "C", 5, 0),
         ("PROD", "C", 7, 0), ("PERIOD", "C", 4, 0), ("DATE", "C", 10, 0), ("TYPE", "C", 3, 0),
         ("BATCHNO", "N", 6, 0), ("REFERENCE", "C", 30, 0), ("CHEQUE", "C", 7, 0), ("DESCRIPT", "C", 50, 0),
         ("DEBIT", "N", 16, 2), ("CREDIT", "N", 16, 2)],
        [(acc, dp, mr, dr, pr, " 7", date, None, b, "Cash to checking", "000123", desc, dr_, cr)
         for (b, m, ln, acc, dp, mr, dr, pr, desc, dr_, cr) in lines if b == 100], mdx=True)
    mdx(os.path.join(d, "GL_DIST.MDX"), "GL_DIST", dist_tags)
    # The archive the retired month close wrote; empty, as in the real company.
    dbf(os.path.join(d, "GL_DISTB.DBF"),
        [("ACCOUNT", "C", 7, 0), ("DEPT", "C", 4, 0), ("PERIOD", "C", 4, 0), ("DATE", "C", 10, 0),
         ("TYPE", "C", 3, 0), ("BATCHNO", "N", 6, 0), ("REFERENCE", "C", 30, 0), ("CHEQUE", "C", 7, 0),
         ("DESCRIPT", "C", 40, 0), ("DEBIT", "N", 16, 2), ("CREDIT", "N", 16, 2)], [], mdx=True)
    mdx(os.path.join(d, "GL_DISTB.MDX"), "GL_DISTB", dist_tags)


def main():
    shutil.rmtree(OUT, ignore_errors=True)
    os.makedirs(OUT)
    d = OUT

    # ---- access ----------------------------------------------------------
    dbf(os.path.join(d, "ACCESS.PSW"),
        [("USERNAME", "C", 10, 0), ("DESCRIPT", "C", 30, 0), ("DATE", "D", 8, 0),
         ("PASSWORD", "C", 10, 0), ("LEVEL", "C", 1, 0)],
        [("ADMIN", "System Administrator", "20261107", "ADMIN", "A"),
         ("CLERK", "Data entry", "20261107", "clerk", "B")], mdx=True)
    mdx(os.path.join(d, "ACCESS.MDX"), "ACCESS", [("U", "USERNAME", "C", False, False, "")])

    items = manila_items()
    dbf(os.path.join(d, "ACCESSM.DBF"),
        [("COUNT", "N", 5, 0), ("POPUP", "C", 40, 0), ("TEXT", "C", 60, 0), ("ID", "N", 6, 0)],
        [(i + 1, popup, text, cmd) for i, (popup, text, cmd) in enumerate(items)], mdx=True)
    mdx(os.path.join(d, "ACCESSM.MDX"), "ACCESSM", [("MENU", "STR(COUNT,5)", "C", False, False, "")])

    admin = "0" * len(items)
    clerk = "".join("1" if popup == "Maintenance" else "0" for popup, _, _ in items)
    dbf(os.path.join(d, "ACCESSL.DBF"), [("LEVEL", "C", 1, 0), ("MENUCODE", "C", len(items), 0)],
        [("A", admin), ("B", clerk)], mdx=True)
    mdx(os.path.join(d, "ACCESSL.MDX"), "ACCESSL", [("L", "LEVEL", "C", False, False, "")])

    # ---- company profile and periods ---------------------------------------
    dbf(os.path.join(d, "GL_PROF.DBF"),
        [("COMPANY", "C", 40, 0), ("ADDRESS1", "C", 50, 0), ("ADDRESS2", "C", 50, 0),
         ("ADDRESS3", "C", 50, 0), ("POSTAL", "C", 9, 0), ("TELEPHONE", "C", 16, 0),
         ("FAX", "C", 16, 0), ("CONTACT", "C", 20, 0)],
        [("Dev Fixture Company Ltd.", "1 Main Street", "Suite 100", "Vancouver BC", "V6B 1A1",
          "(604) 555 - 0100", "(604) 555 - 0101", "A. Accountant")], mdx=False)

    months = ["Apr 26", "May 26", "Jun 26", "July 26", "Aug 26", "Sep 26", "Oct 26", "Nov 26", "Dec 26",
              "Jan 27", "Feb 27", "Mar 27"]
    ends = ["30", "31", "30", "31", "31", "30", "31", "30", "31", "31", "28", "31"]
    pds = []
    for i in range(12):
        y, mo = (2026, 4 + i) if i < 9 else (2027, i - 8)
        status = "1" if i < 6 else "0"   # Apr-Sep closed: the current period is Oct 26
        pds.append(("Dev Fixture Company Ltd." if i == 0 else None, i + 1, f"{y}{mo:02d}01",
                    f"{y}{mo:02d}{ends[i]}", months[i], status))
    dbf(os.path.join(d, "GL_PDS.DBF"),
        [("COMPANY_NM", "C", 40, 0), ("PERIOD", "N", 2, 0), ("PERIODS", "D", 8, 0), ("PERIODE", "D", 8, 0),
         ("PD_DESC", "C", 10, 0), ("STATUS", "C", 1, 0)], pds, mdx=True)
    mdx(os.path.join(d, "GL_PDS.MDX"), "GL_PDS", [("PERIOD", "period", "N", False, False, "")])

    # ---- control and cash accounts -----------------------------------------
    dbf(os.path.join(d, "GL_SETAT.DBF"),
        [("SHORT", "C", 4, 0), ("NAME", "C", 30, 0), ("ACCOUNT", "C", 7, 0), ("DEPT", "C", 4, 0),
         ("TYPE", "C", 2, 0)],
        [("RE", "Retained Earnings", "340000", None, "GL"),
         ("SALE", "Sales Revenue", "410001", None, "GL"),
         ("VATR", "Prepaid VAT", "148000", None, "GL"),
         ("VATP", "VAT Payable", "260007", None, "GL"),
         ("WTXC", "Company Withholding Tax", "260004", None, "GL"),
         ("WTXI", "Ind. Withholding Tax", "260004", None, "GL"),
         ("AP", "Accounts Payable", "221002", None, "GL"),
         ("AR", "Accounts Receivable", "111101", None, "GL"),
         ("CHCK", "Checking Account", "102101", None, "CA"),
         ("CA", "Petty Cash", "101000", None, "CA")], mdx=True)
    mdx(os.path.join(d, "GL_SETAT.MDX"), "GL_SETAT", [
        ("NAME", "NAME", "C", False, False, 'TYPE="CA"'),
        ("CASH", "ACCOUNT+DEPT", "C", False, False, 'TYPE="CA"'),
        ("GL_ACCTS", "SHORT", "C", False, False, 'TYPE="GL"'),
    ])

    # ---- chart of accounts ---------------------------------------------------
    n = lambda name: (name, "N", 16, 2)
    glaccts = [("ACCOUNT", "C", 7, 0), ("DEPT", "C", 4, 0), ("DESCRIPT", "C", 45, 0), ("FDESCRIPT", "C", 45, 0),
               ("TYPE", "C", 1, 0), ("CATEGORY", "C", 5, 0), n("CURR_BAL")]
    glaccts += [n(f"PERIOD{i}") for i in range(1, 13)] + [n("OPENBAL")]
    glaccts += [n(f"PREVIOUS{i}") for i in range(1, 13)] + [n(f"BUDGET{i}") for i in range(1, 13)]

    # The opening balance is OPENBAL and PREVIOUS12 (the Chart of Accounts shows PREVIOUS12);
    # `period` puts activity in one period, which the Chart of Accounts won't delete.
    def acct(no, dept, desc, typ, bal, period=None):
        periods = [None] * 12
        if period:
            periods[period[0] - 1] = period[1]
        return (no, dept, desc, desc, typ, None, bal) + tuple(periods) + (bal,) + (None,) * 11 + (bal,) + (None,) * 12

    dbf(os.path.join(d, "GLACCTS.DBF"), glaccts, [
        acct("101000", None, "PETTY CASH AND CASH IN HAND", "B", 500.0, period=(7, -250.0)),
        acct("102101", None, "CHECKING ACCOUNT", "B", 12500.0, period=(7, 250.0)),
        acct("111101", None, "ACCOUNTS RECEIVABLE - TRADE", "B", 8200.0),
        acct("144910", None, "TRANSIT ACCOUNT", "B", 0.0),
        acct("148000", None, "PREPAID VAT", "B", 310.0),
        acct("221002", None, "ACCOUNTS PAYABLE - TRADE", "B", -4100.0),
        acct("260004", None, "WITHHOLDING TAX PAYABLE", "B", -120.0),
        acct("260007", None, "VAT PAYABLE", "B", -450.0),
        acct("340000", None, "RETAINED EARNINGS", "R", -16840.0),
        acct("410001", "10", "SALES REVENUE - R&D", "I", 0.0),
        acct("410001", "20", "SALES REVENUE - GENERAL", "I", 0.0),
    ], mdx=True)
    mdx(os.path.join(d, "GLACCTS.MDX"), "GLACCTS", [
        ("TYPE", "type", "C", False, False, ""),
        ("DEPT", "dept", "C", False, False, ""),
        ("ACCOUNTS", "account+dept", "C", False, False, ""),
        ("ACCOUNT", "account+dept", "C", False, False, ""),
    ])

    dbf(os.path.join(d, "GLDEPT.DBF"), [("DEPART", "C", 4, 0), ("DESCRIPT", "C", 40, 0)],
        [("10", "Research & Development"), ("20", "General - Common")], mdx=True)
    mdx(os.path.join(d, "GLDEPT.MDX"), "GLDEPT", [("DEPART", "depart", "C", False, False, "")])

    vouchers(d)

    print(f"dev fixture written to {OUT}: {len(items)} accessm rows")


if __name__ == "__main__":
    main()
