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
    nested popup's items carry the nested popup's name (as Reset did).
    An item's ID is a number, or one of the port's symbols from
    gl/port/PortIds.h (Spec #3 S3-M7: Custom Report is CM_REPORT_LIST)."""
    with open(os.path.join(ROOT, "gl", "res", "gl.rc"), encoding="utf-16") as f:
        text = f.read()
    with open(os.path.join(ROOT, "gl", "port", "PortIds.h"), encoding="utf-8") as f:
        port_ids = {m.group(1): int(m.group(2))
                    for m in re.finditer(r"^#define\s+(\w+)\s+(\d+)", f.read(), re.M)}
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
        m = re.match(r'MENUITEM "(.*)", (\w+)', s)
        if m:
            ident = m.group(2)
            if not ident.isdigit() and ident not in port_ids:
                sys.exit(f"MANILA item {m.group(1)!r}: unknown ID {ident}")
            items.append((stack[-1], m.group(1).replace("&", ""),
                          int(ident) if ident.isdigit() else port_ids[ident]))
    return items


# ---- the finance set's activity (spec #3 9, plan S3-M2) ----------------------
#
# A year of a small company, so the trial balances and statements have
# figures in every section. Fiscal period p is the month April 2026 + (p - 1).
# New accounts only: the Spec #1/#2 accounts keep their rows, so the earlier
# checks don't move. Opening balances (OPENBAL, the start of the year) of the
# new balance-sheet accounts sum to zero; each journal balances, so every
# period does too. CURR_BAL = OPENBAL + sum of PERIOD1-12 for every account.
#
#   account  dept  type  opening    description
FINANCE_ACCOUNTS = [
    ("102201", None, "B", 20000.0, "BANK - OPERATING ACCOUNT"),
    ("151000", None, "B", 30000.0, "OFFICE EQUIPMENT"),
    ("161000", None, "B", -6000.0, "ACCUM. DEPRECIATION - OFFICE EQUIPMENT"),
    ("201000", None, "B", 0.0, "SHORT-TERM BANK LOAN"),
    ("301000", None, "B", -44000.0, "SHARE CAPITAL"),
    ("411000", None, "I", 0.0, "SALES - CASH"),
    ("501000", None, "I", 0.0, "COST OF SALES"),
    ("731000", "10", "I", 0.0, "SALARIES - R&D"),
    ("731000", "20", "I", 0.0, "SALARIES - GENERAL"),
    ("751000", None, "I", 0.0, "OFFICE RENT"),
    ("761000", None, "I", 0.0, "DEPRECIATION EXPENSE"),
    ("801000", None, "I", 0.0, "INTEREST INCOME"),
    ("911000", None, "I", 0.0, "INCOME TAX EXPENSE"),
    ("999000", None, "B", 0.0, "SUSPENSE"),   # outside every default layout range: "Unclassified"
]

# Prior-year and budget amounts per period (the same in each of the 12) on
# the income-statement accounts, for F3's comparisons.
#   (account, dept): (previous, budget)
FINANCE_COMPARE = {
    ("411000", None): (-9000.0, -10000.0),
    ("501000", None): (3600.0, 4000.0),
    ("731000", "10"): (1800.0, 2000.0),
    ("731000", "20"): (1400.0, 1500.0),
    ("751000", None): (1200.0, 1200.0),
    ("761000", None): (500.0, 600.0),
    ("801000", None): (-40.0, -50.0),
    ("911000", None): (700.0, 800.0),
}

# Posted journals: (voucher no., period, date, reference, [(account, dept, description, debit, credit)]).
# Each has gl_dist rows. Their voucher numbers have no gl_btch header (as
# for vouchers posted and archived long ago); Recover lists only gl_btch.
FINANCE_JOURNALS = [
    (1, 1, "20260415", "April cash sales",
     [("102201", None, "Cash sales", 10000.0, 0.0), ("411000", None, "Cash sales", 0.0, 10000.0)]),
    (2, 1, "20260420", "April purchases",
     [("501000", None, "Goods sold", 4000.0, 0.0), ("102201", None, "Supplier payment", 0.0, 4000.0)]),
    (3, 2, "20260531", "May salaries",
     [("731000", "10", "Salaries R&D", 2000.0, 0.0), ("731000", "20", "Salaries general", 1500.0, 0.0),
      ("102201", None, "Payroll", 0.0, 3500.0)]),
    (4, 3, "20260610", "Office equipment",
     [("151000", None, "Laptops", 5000.0, 0.0), ("102201", None, "Laptops", 0.0, 5000.0)]),
    (5, 3, "20260620", "Unexplained payment",
     [("999000", None, "To be classified", 100.0, 0.0), ("102201", None, "Unexplained", 0.0, 100.0)]),
    (6, 4, "20260701", "Bank loan drawdown",
     [("102201", None, "Loan proceeds", 8000.0, 0.0), ("201000", None, "Loan proceeds", 0.0, 8000.0)]),
    (7, 5, "20260805", "August rent",
     [("751000", None, "Rent", 1200.0, 0.0), ("102201", None, "Rent", 0.0, 1200.0)]),
    (8, 5, "20260830", "Interest received",
     [("102201", None, "Interest", 50.0, 0.0), ("801000", None, "Interest", 0.0, 50.0)]),
    (9, 7, "20261005", "October cash sales",
     [("102201", None, "Cash sales", 6000.0, 0.0), ("411000", None, "Cash sales", 0.0, 6000.0)]),
    (10, 7, "20261010", "October purchases",
     [("501000", None, "Goods sold", 2400.0, 0.0), ("102201", None, "Supplier payment", 0.0, 2400.0)]),
    (11, 7, "20261020", "Income tax instalment",
     [("911000", None, "Instalment", 900.0, 0.0), ("102201", None, "Instalment", 0.0, 900.0)]),
    (12, 7, "20261031", "October salaries",
     [("731000", "10", "Salaries R&D", 2000.0, 0.0), ("102201", None, "Payroll", 0.0, 2000.0)]),
]

# Movements with no gl_dist rows (F2 marks these accounts "*"): period 6's
# depreciation, as if entered by a route that wrote no distribution lines.
FINANCE_UNDISTRIBUTED = [
    (6, [("761000", None, 600.0, 0.0), ("161000", None, 0.0, 600.0)]),
]

# A gl_dist row that is deleted (dBase '*'), which every report must ignore.
FINANCE_DELETED_DIST = (7, 5, "20260805", "August rent", ("751000", None, "Rent - entered twice", 1200.0, 0.0))


def finance_periods():
    """{(account, dept): [12 period amounts]} from the journals and the undistributed movements."""
    out = {}
    def add(acc, dept, period, dr, cr):
        out.setdefault((acc, dept), [0.0] * 12)[period - 1] += dr - cr
    for _, period, _, _, lines in FINANCE_JOURNALS:
        for acc, dept, _, dr, cr in lines:
            add(acc, dept, period, dr, cr)
    for period, lines in FINANCE_UNDISTRIBUTED:
        for acc, dept, dr, cr in lines:
            add(acc, dept, period, dr, cr)
    return out


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
    # Voucher 100's two lines, then the finance journals' (S3-M2), then one
    # deleted row. PERIOD is " 7" style, as the real rows; TYPE blank for GL.
    dist_rows = [(acc, dp, mr, dr, pr, " 7", date, None, b, "Cash to checking", "000123", desc, dr_, cr)
                 for (b, m, ln, acc, dp, mr, dr, pr, desc, dr_, cr) in lines if b == 100]
    for b, period, jdate, ref, jlines in FINANCE_JOURNALS:
        for acc, dp, desc, dr_, cr in jlines:
            dist_rows.append((acc, dp, None, None, None, f"{period:2d}", jdate, None, b, ref, None, desc, dr_, cr))
    b, period, jdate, ref, (acc, dp, desc, dr_, cr) = FINANCE_DELETED_DIST
    dist_rows.append((acc, dp, None, None, None, f"{period:2d}", jdate, None, b, ref, None, desc, dr_, cr))
    dbf(os.path.join(d, "GL_DIST.DBF"),
        [("ACCOUNT", "C", 7, 0), ("DEPT", "C", 4, 0), ("MEDREP", "C", 5, 0), ("DRCODE", "C", 5, 0),
         ("PROD", "C", 7, 0), ("PERIOD", "C", 4, 0), ("DATE", "C", 10, 0), ("TYPE", "C", 3, 0),
         ("BATCHNO", "N", 6, 0), ("REFERENCE", "C", 30, 0), ("CHEQUE", "C", 7, 0), ("DESCRIPT", "C", 50, 0),
         ("DEBIT", "N", 16, 2), ("CREDIT", "N", 16, 2)],
        dist_rows, deleted=(len(dist_rows) - 1,), mdx=True)
    mdx(os.path.join(d, "GL_DIST.MDX"), "GL_DIST", dist_tags)
    # The archive the retired month close wrote; empty, as in the real company.
    dbf(os.path.join(d, "GL_DISTB.DBF"),
        [("ACCOUNT", "C", 7, 0), ("DEPT", "C", 4, 0), ("PERIOD", "C", 4, 0), ("DATE", "C", 10, 0),
         ("TYPE", "C", 3, 0), ("BATCHNO", "N", 6, 0), ("REFERENCE", "C", 30, 0), ("CHEQUE", "C", 7, 0),
         ("DESCRIPT", "C", 40, 0), ("DEBIT", "N", 16, 2), ("CREDIT", "N", 16, 2)], [], mdx=True)
    mdx(os.path.join(d, "GL_DISTB.MDX"), "GL_DISTB", dist_tags)


def fixed_assets(d):
    """Spec #4 9: GL_FA, GL_FATYP and the port's GL_FADEP.

    Layouts and tags copy the real GL_FA and GL_FATYP (checked 2026-11-11);
    GL_FADEP is the port's history table (spec #4 5.3). The register agrees
    with the Spec #3 accounts: type EQ's assets cost 35,000.00 (151000) and
    have 6,600.00 accumulated (161000), of which period 6's 600.00 (E001
    500.00, E002 100.00) is in GL_FADEP with no voucher (batchno 0), as it has
    none in GL_DIST either.
    """
    acct = lambda a, desc: (a, None, desc)
    eq = ("EQ", "Office equipment", None, None) + acct("151000", "OFFICE EQUIPMENT")         + acct("761000", "DEPRECIATION EXPENSE") + acct("161000", "ACCUM. DEPRECIATION - OFFICE EQUIPMENT")
    # XX names an account that isn't in the chart; the Types screen would refuse it.
    xx = ("XX", "Bad accounts (test)", None, None) + acct("999999", "MISSING")         + acct("999999", "MISSING") + acct("999999", "MISSING")
    dbf(os.path.join(d, "GL_FATYP.DBF"),
        [("FATYPE", "C", 4, 0), ("FADESC", "C", 30, 0), ("FAYEARS", "N", 2, 0), ("FAPERCENT", "N", 2, 0),
         ("ASSET_ACCT", "C", 7, 0), ("ASSET_DEPT", "C", 4, 0), ("ASSET_DESC", "C", 40, 0),
         ("EXPEN_ACCT", "C", 7, 0), ("EXPEN_DEPT", "C", 4, 0), ("EXPEN_DESC", "C", 40, 0),
         ("ACCUM_ACCT", "C", 7, 0), ("ACCUM_DEPT", "C", 4, 0), ("ACCUM_DESC", "C", 40, 0)],
        [eq, xx], mdx=True)
    mdx(os.path.join(d, "GL_FATYP.MDX"), "GL_FATYP", [("FATYPE", "fatype", "C", False, False, "")])

    # (code, method, reference, description, date sold, date purchased, months used, life, rate,
    #  cost, accumulated, monthly, salvage, type, mode)
    dbf(os.path.join(d, "GL_FA.DBF"),
        [("FACODE", "C", 10, 0), ("DEP_METH", "C", 10, 0), ("REFERENCE", "C", 40, 0), ("DESCRIPT", "C", 40, 0),
         ("DATE_SOLD", "D", 8, 0), ("DATE_PUR", "D", 8, 0), ("LIFELEFT", "N", 16, 0), ("LIFETIME", "N", 16, 0),
         ("PERCENT", "N", 2, 0), ("COST", "N", 16, 2), ("ACCUM_DEP", "N", 16, 2), ("CURR_DEP", "N", 16, 2),
         ("SALVAGE", "N", 16, 2), ("FATYPE", "C", 4, 0), ("MODE", "C", 1, 0)],
        [("E001", "Straight", "Head office", "Office furniture", None, "20250401", 13, 60, 0,
          30000.0, 6500.0, 500.0, 0.0, "EQ", None),
         ("E002", "Declining", "Sales team", "Laptops", None, "20260615", 1, 36, 24,
          5000.0, 100.0, 100.0, 0.0, "EQ", None),
         ("E004", "Straight", "Head office", "Old printer", "20260815", "20230401", 36, 36, 0,
          1000.0, 1000.0, 27.78, 0.0, "EQ", "D"),
         ("E005", "Straight", "Training room", "Projector", None, "20260501", 0, 24, 0,
          2000.0, 0.0, 83.33, 0.0, "XX", None)], mdx=True)
    mdx(os.path.join(d, "GL_FA.MDX"), "GL_FA", [
        ("FACODE", "facode", "C", False, False, ""),
        ("DEP_METH", "dep_meth", "C", False, False, ""),
    ])

    dbf(os.path.join(d, "GL_FADEP.DBF"),
        [("FACODE", "C", 10, 0), ("YEAR_END", "D", 8, 0), ("PERIOD", "N", 2, 0), ("BATCHNO", "N", 6, 0),
         ("AMOUNT", "N", 16, 2)],
        [("E001", "20270331", 6, 0, 500.0), ("E002", "20270331", 6, 0, 100.0)], mdx=True)
    mdx(os.path.join(d, "GL_FADEP.MDX"), "GL_FADEP", [
        ("FACODE", "facode", "C", False, False, ""),
        ("BATCHNO", "batchno", "N", False, False, ""),
    ])


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
    # CURR_BAL = OPENBAL + the periods (spec #3 2.5; until S3-M2 the period-7
    # activity of 101000 and 102101 was left out of their CURR_BAL).
    def acct(no, dept, desc, typ, bal, period=None):
        periods = [None] * 12
        if period:
            periods[period[0] - 1] = period[1]
        curr = bal + (period[1] if period else 0.0)
        return (no, dept, desc, desc, typ, None, curr) + tuple(periods) + (bal,) + (None,) * 11 + (bal,) + (None,) * 12

    # The finance set's accounts (FINANCE_ACCOUNTS): balance-sheet ones show
    # their opening balance as PREVIOUS12, like the rows above; income ones
    # carry FINANCE_COMPARE's prior-year and budget amounts.
    movements = finance_periods()
    def finance(no, dept, typ, opening, desc):
        periods = movements.get((no, dept), [0.0] * 12)
        curr = round(opening + sum(periods), 2)
        prev, budget = FINANCE_COMPARE.get((no, dept), (None, None))
        previous = (prev,) * 12 if prev is not None else (None,) * 11 + (opening,)
        budgets = (budget,) * 12 if budget is not None else (None,) * 12
        return ((no, dept, desc, desc, typ, None, curr) + tuple(p or None for p in periods) + (opening,)
                + previous + budgets)

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
    ] + [finance(*a) for a in FINANCE_ACCOUNTS], mdx=True)
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
    fixed_assets(d)

    print(f"dev fixture written to {OUT}: {len(items)} accessm rows")


if __name__ == "__main__":
    main()
