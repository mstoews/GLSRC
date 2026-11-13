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
dbf, mdx, dbt = mf.dbf, mf.mdx, mf.dbt


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


def accounts_payable(d):
    """Spec #5 9: the AP tables and the port's AP_LINEX.

    Layouts and tags copy the real AP_* tables (checked 2026-11-13); the rows
    are made up, in period 7 (Oct 26), F mode (GL.INI [WITHHOLDING] TAX=F).
    Every AP_JSUM row is waiting ("1"), not transferred, so no GL balance
    moves. AP_LINEX holds the lines' roles and typed amounts for the vouchers
    the port would have written; invoice 3 and the migrated-style invoice 5
    have none, as the deployed vouchers, so LoadApLines classifies them.

      invoice 1  open        V001 INV-1001  501000 1,120.00 -> 1,000.00 + VAT 120.00; W/H 10.00; AP 1,110.00
      invoice 2  open        V001 INV-1002  501000 560.00 + 751000 224.00 -> 700.00 + VAT 84.00; W/H 7.00; AP 777.00
      invoice 3  posted      V002 INV-2001  731000-20 5,000.00, no VAT, W/H 2 % 100.00; AP 4,900.00; open item "O"
      invoice 4  cancelled   V003 INV-3001  751000 1,500.00 ("X", lines kept)
      invoice 5  "2" + "Cancelled": the original's cancel, migrated to "X" by EnsureApTables
      invoice 6  posted      V003 INV-3002  751000 1,200.00; paid by cheque 5001 (102101), printed ("V"), not cleared
      cash 1     open        V001 INV-1003  501000 336.00 -> 300.00 + VAT 36.00; W/H 3.00; bank 102101 333.00
    """
    vend_fields = [
        ("VENDORNO", "C", 6, 0), ("VDSHORT", "C", 12, 0), ("VDNAME", "C", 40, 0), ("ADDRESS1", "C", 30, 0),
        ("ADDRESS2", "C", 30, 0), ("ADDRESS3", "C", 30, 0), ("POSTAL", "C", 7, 0), ("PHONE", "C", 16, 0),
        ("FAX", "C", 16, 0), ("DEPART", "C", 4, 0), ("ACCOUNT", "C", 7, 0), ("VAT_DEPT", "C", 4, 0),
        ("VAT_ACCT", "C", 7, 0), ("AP_ACCT", "C", 7, 0), ("AP_DEPT", "C", 4, 0), ("DESCRIPT", "C", 40, 0),
        ("CONTACT", "C", 12, 0), ("TYPE", "C", 3, 0), ("MEDREP", "C", 6, 0), ("STATUS", "C", 3, 0),
        ("TERMS", "N", 3, 0), ("VAT", "N", 1, 0), ("FIVE", "N", 1, 0), ("THREE", "N", 1, 0),
        ("PERCENTAGE", "N", 2, 0), ("REBATE", "N", 2, 0), ("VIN", "C", 10, 0), ("DIS_DAYS", "N", 2, 0),
        ("DIS_PERCT", "N", 2, 0)]

    def vendor(no, short, name, typ, vat, wh, terms, contact):
        return (no, short, name, "2 Supplier Lane", None, "Vancouver BC", "V6B1A2", "(604) 555 - 0150", None,
                None, "221002", None, "148000", None, None, None, contact, typ, None, "ACT",
                terms, 0, 0, 0, vat, wh, "123-456-78", 0, 0)
    dbf(os.path.join(d, "AP_VEND.DBF"), vend_fields, [
        vendor("V001", "OFFICESUP", "Office Supplies Co.", "C", 12, 1, 30, "M. Reyes"),
        vendor("V002", "SANTOS", "J. Santos (consultant)", "I", 0, 2, 15, "J. Santos"),
        vendor("V003", "LANDLORD", "City Landlord Inc.", "C", 0, 0, 30, "Leasing"),
    ], mdx=True)
    mdx(os.path.join(d, "AP_VEND.MDX"), "AP_VEND", [("VENDORNO", "VENDORNO", "C", False, False, "")])

    def head_fields(cash):
        f = [("BATCHNO", "N", 6, 0), ("MODE", "C", 1, 0), ("TYPE", "C", 2, 0), ("ADJ", "C", 1, 0),
             ("PERIOD", "N", 2, 0), ("VENDORNO", "C", 6, 0), ("VDNAME", "C", 40, 0),
             ("INVOICENO", "C", 10 if cash else 8, 0)]
        if cash:
            f.append(("CHEQUENO", "C", 10, 0))
        f += [("ORDERNO", "C", 10, 0), ("DUEDATE", "D", 8, 0), ("INVDATE", "D", 8, 0), ("REFERENCE", "C", 40, 0),
              ("DESCRIPT", "C", 40, 0), ("DATE", "D", 8, 0), ("ACCOUNT", "C", 7, 0), ("DEPART", "C", 4, 0),
              ("PAYABLE", "N", 16, 2), ("REBATEAMT", "N", 16, 2), ("ENTRYNO", "N", 3, 0),
              ("VATPERCENT", "N", 2, 0), ("REBATE", "N", 2, 0), ("TERMS", "N", 3, 0), ("VATAMT", "N", 16, 2)]
        f += [("VATACCT", "C", 7, 0), ("VATDEPT", "C", 4, 0)] if cash else [("ADJAMT", "N", 16, 2)]
        return f

    def line_fields(cash):
        return [("BATCHNO", "N", 6, 0), ("ENTRYNO", "N", 3, 0), ("DESCRIPT", "C", 40, 0), ("MEDREP", "C", 5, 0),
                ("DRCODES" if cash else "DRCODE", "C", 5, 0), ("PRODUCT", "C", 7, 0), ("ACCOUNT", "C", 7, 0),
                ("DEPT", "C", 4, 0), ("DEBIT", "N", 14, 2), ("CREDIT", "N", 14, 2)]

    def head_tags(name):
        return [("ACCOUNT", "ACCOUNT+DEPART", "C", False, False, ""),
                ("BATCHNO", "BATCHNO", "N", False, False, ""),
                ("INVOICENO", "VENDORNO+INVOICENO", "C", False, False, ""),
                ("RECNO", "RECNO()", "N", False, False, "")]

    line_tags = [("BATCHNO", "BATCHNO", "N", False, False, ""), ("RECNO", "RECNO()", "N", False, False, "")]

    # (batchno, mode, period, vendor, name, invoice, due, invdate, reference, date, ap account,
    #  payable, rebateamt, lines, vat %, w/h %, terms, vatamt)
    def inv(no, mode, vend, name, invoice, due, invdate, ref, payable, wh, lines, vat_pct, wh_pct, terms, vat):
        return (no, mode, None, None, 7, vend, name, invoice, None, due, invdate, ref, None, invdate,
                "221002", None, payable, wh, lines, vat_pct, wh_pct, terms, vat, 0.0)
    dbf(os.path.join(d, "AP_BTCH.DBF"), head_fields(False), [
        inv(1, "1", "V001", "Office Supplies Co.", "INV-1001", "20261104", "20261005", "Paper and toner",
            1120.0, -10.0, 4, 12, 1, 30, 120.0),
        inv(2, "1", "V001", "Office Supplies Co.", "INV-1002", "20261111", "20261012", "Stock and office items",
            784.0, -7.0, 5, 12, 1, 30, 84.0),
        inv(3, "0", "V002", "J. Santos (consultant)", "INV-2001", "20261016", "20261001", "Consulting, September",
            5000.0, -100.0, 3, 0, 2, 15, 0.0),
        inv(4, "X", "V003", "City Landlord Inc.", "INV-3001", "20261031", "20261001", "Rent entered twice",
            1500.0, 0.0, 2, 0, 0, 30, 0.0),
        inv(5, "2", "V003", "City Landlord Inc.", "INV-3000", "20261031", "20261001", "Cancelled",
            0.0, 0.0, 2, 0, 0, 30, 0.0),
        inv(6, "0", "V003", "City Landlord Inc.", "INV-3002", "20261101", "20261002", "Rent, October",
            1200.0, 0.0, 2, 0, 0, 30, 0.0),
    ], mdx=True)
    mdx(os.path.join(d, "AP_BTCH.MDX"), "AP_BTCH", head_tags("AP_BTCH"))

    def line(no, entry, desc, acct, dept, dr, cr):
        return (no, entry, desc, None, None, None, acct, dept, dr, cr)
    dbf(os.path.join(d, "AP_BTCHD.DBF"), line_fields(False), [
        line(1, 1, "COST OF SALES", "501000", None, 1000.0, 0.0),
        line(1, 2, "PREPAID VAT", "148000", None, 120.0, 0.0),
        line(1, 3, "WITHHOLDING TAX PAYABLE", "260004", None, 0.0, 10.0),
        line(1, 4, "ACCOUNTS PAYABLE - TRADE", "221002", None, 0.0, 1110.0),
        line(2, 1, "COST OF SALES", "501000", None, 500.0, 0.0),
        line(2, 2, "OFFICE RENT", "751000", None, 200.0, 0.0),
        line(2, 3, "PREPAID VAT", "148000", None, 84.0, 0.0),
        line(2, 4, "WITHHOLDING TAX PAYABLE", "260004", None, 0.0, 7.0),
        line(2, 5, "ACCOUNTS PAYABLE - TRADE", "221002", None, 0.0, 777.0),
        line(3, 1, "SALARIES - GENERAL", "731000", "20", 5000.0, 0.0),
        line(3, 2, "WITHHOLDING TAX PAYABLE", "260004", None, 0.0, 100.0),
        line(3, 3, "ACCOUNTS PAYABLE - TRADE", "221002", None, 0.0, 4900.0),
        line(4, 1, "OFFICE RENT", "751000", None, 1500.0, 0.0),
        line(4, 2, "ACCOUNTS PAYABLE - TRADE", "221002", None, 0.0, 1500.0),
        line(5, 1, "Cancelled", "751000", None, 0.0, 0.0),
        line(5, 2, "Cancelled", "221002", None, 0.0, 0.0),
        line(6, 1, "OFFICE RENT", "751000", None, 1200.0, 0.0),
        line(6, 2, "ACCOUNTS PAYABLE - TRADE", "221002", None, 0.0, 1200.0),
    ], mdx=True)
    mdx(os.path.join(d, "AP_BTCHD.MDX"), "AP_BTCHD", line_tags)

    dbf(os.path.join(d, "AP_CASH.DBF"), head_fields(True), [
        (1, "1", None, None, 7, "V001", "Office Supplies Co.", "INV-1003", "000123", None, "20261020",
         "20261020", "Supplies paid at the counter", None, "20261020", "102101", None, 336.0, -3.0, 4, 12, 1, 30,
         36.0, "148000", None),
    ], mdx=True)
    mdx(os.path.join(d, "AP_CASH.MDX"), "AP_CASH", head_tags("AP_CASH"))
    dbf(os.path.join(d, "AP_CASHD.DBF"), line_fields(True), [
        line(1, 1, "COST OF SALES", "501000", None, 300.0, 0.0),
        line(1, 2, "PREPAID VAT", "148000", None, 36.0, 0.0),
        line(1, 3, "WITHHOLDING TAX PAYABLE", "260004", None, 0.0, 3.0),
        line(1, 4, "CHECKING ACCOUNT", "102101", None, 0.0, 333.0),
    ], mdx=True)
    mdx(os.path.join(d, "AP_CASHD.MDX"), "AP_CASHD", line_tags)

    # The port's line roles and typed amounts (Ap.h): E expense, V VAT, W W/H, C credit.
    linex = [("I", 1, 1, "E", 1120.0), ("I", 1, 2, "V", 0.0), ("I", 1, 3, "W", 0.0), ("I", 1, 4, "C", 0.0),
             ("I", 2, 1, "E", 560.0), ("I", 2, 2, "E", 224.0), ("I", 2, 3, "V", 0.0), ("I", 2, 4, "W", 0.0),
             ("I", 2, 5, "C", 0.0),
             ("I", 4, 1, "E", 1500.0), ("I", 4, 2, "C", 0.0),
             ("I", 6, 1, "E", 1200.0), ("I", 6, 2, "C", 0.0),
             ("C", 1, 1, "E", 336.0), ("C", 1, 2, "V", 0.0), ("C", 1, 3, "W", 0.0), ("C", 1, 4, "C", 0.0)]
    dbf(os.path.join(d, "AP_LINEX.DBF"),
        [("KIND", "C", 1, 0), ("BATCHNO", "N", 6, 0), ("ENTRYNO", "N", 3, 0), ("ROLE", "C", 1, 0),
         ("GROSS", "N", 16, 2)], linex, mdx=True)
    mdx(os.path.join(d, "AP_LINEX.MDX"), "AP_LINEX", [("BATCHNO", "BATCHNO", "N", False, False, "")])

    dbf(os.path.join(d, "AP_DIST.DBF"),
        [("ACCOUNT", "C", 7, 0), ("DEPT", "C", 4, 0), ("APACCOUNT", "C", 7, 0), ("APDEPART", "C", 4, 0),
         ("PERIOD", "N", 2, 0), ("DATE", "D", 8, 0), ("DUEDATE", "D", 8, 0), ("BATCHNO", "N", 6, 0),
         ("INVOICENO", "C", 8, 0), ("PAYMENTCTL", "C", 1, 0), ("VENDORNO", "C", 6, 0), ("CHEQUENO", "N", 10, 0),
         ("REFERENCE", "C", 40, 0), ("DESCRIPT", "C", 40, 0), ("AMOUNT", "N", 16, 2), ("REBATEAMT", "N", 16, 2),
         ("AMOUNTPAID", "N", 16, 2), ("PAYMENTREQ", "N", 10, 0), ("DATEPAID", "D", 8, 0), ("VATAMT", "N", 16, 2),
         ("ADJUSTMENT", "N", 16, 2), ("REBATE", "C", 2, 0)],
        [(None, None, "221002", None, 7, "20261001", "20261016", 3, "INV-2001", "O", "V002", 0,
          "Consulting, September", None, 5000.0, -100.0, 0.0, 0, None, 0.0, 0.0, None),
         ("102101", None, "221002", None, 7, "20261002", "20261101", 6, "INV-3002", "V", "V003", 5001,
          "Rent, October", None, 1200.0, 0.0, 1200.0, 0, "20261015", 0.0, 0.0, None)], mdx=True)
    mdx(os.path.join(d, "AP_DIST.MDX"), "AP_DIST", [
        ("ACCOUNT", "ACCOUNT", "C", False, False, ""),
        ("BATCHNO", "BATCHNO", "N", False, False, ""),
        ("CHEQUENO", "CHEQUENO", "N", False, False, ""),
        ("CHEQUES", "VENDORNO", "C", False, False, 'PAYMENTCTL="P"'),
        ("DEPT", "DEPT", "C", False, False, ""),
        ("INVOICENO", "VENDORNO+INVOICENO", "C", False, False, ""),
        ("PAYMENTREQ", "PAYMENTREQ", "N", False, False, ""),
        ("VENDORNO", "VENDORNO", "C", False, False, ""),
    ])

    dbf(os.path.join(d, "AP_REQST.DBF"),
        [("VENDORNO", "C", 6, 0), ("PAYMENTREQ", "N", 10, 0), ("REFERENCE", "C", 20, 0),
         ("AMOUNTPAID", "N", 16, 2), ("DATEPAID", "D", 8, 0), ("CHEQUE", "N", 10, 0), ("BANKACCT", "C", 7, 0),
         ("BANKDEPT", "C", 4, 0), ("CLEARED", "C", 1, 0)],
        [("V003", 0, "Rent, October", 1200.0, "20261015", 5001, "102101", None, None)], mdx=True)
    mdx(os.path.join(d, "AP_REQST.MDX"), "AP_REQST", [
        ("PAYMENTREQ", "PAYMENTREQ", "N", False, False, ""),
        ("VENDORNO", "VENDORNO", "C", False, False, ""),
    ])

    # AP's journal, all waiting: invoices 3 and 6, and invoice 6's payment.
    def jsum(acct, dept, date, batch, ref, cheque, desc, dr, cr):
        return (acct, dept, " 7", "1", date, "AP", batch, ref, cheque, desc, dr, cr)
    dbf(os.path.join(d, "AP_JSUM.DBF"),
        [("ACCOUNT", "C", 7, 0), ("DEPT", "C", 4, 0), ("PERIOD", "C", 4, 0), ("MODE", "C", 1, 0),
         ("DATE", "D", 8, 0), ("TYPE", "C", 3, 0), ("BATCHNO", "N", 6, 0), ("REFERENCE", "C", 30, 0),
         ("CHEQUE", "C", 7, 0), ("DESCRIPT", "C", 40, 0), ("DEBIT", "N", 16, 2), ("CREDIT", "N", 16, 2)],
        [jsum("731000", "20", "20261001", 3, "Consulting, September", None, "SALARIES - GENERAL", 5000.0, 0.0),
         jsum("260004", None, "20261001", 3, "Consulting, September", None, "WITHHOLDING TAX PAYABLE", 0.0, 100.0),
         jsum("221002", None, "20261001", 3, "Consulting, September", None, "ACCOUNTS PAYABLE - TRADE", 0.0, 4900.0),
         jsum("751000", None, "20261002", 6, "Rent, October", None, "OFFICE RENT", 1200.0, 0.0),
         jsum("221002", None, "20261002", 6, "Rent, October", None, "ACCOUNTS PAYABLE - TRADE", 0.0, 1200.0),
         jsum("221002", None, "20261015", 6, "Rent, October", "5001", "ACCOUNTS PAYABLE - TRADE", 1200.0, 0.0),
         jsum("102101", None, "20261015", 6, "Rent, October", "5001", "CHECKING ACCOUNT", 0.0, 1200.0)],
        mdx=True)
    mdx(os.path.join(d, "AP_JSUM.MDX"), "AP_JSUM", [
        ("ACCOUNT", "ACCOUNT+DEPT", "C", False, False, ""),
        ("APDIST", "BATCHNO", "N", False, False, 'TYPE="AP "'),
        ("ARDIST", "BATCHNO", "N", False, False, 'TYPE="AR "'),
        ("BATCHNO", "BATCHNO", "N", False, False, ""),
        ("DESCRIPT", "DESCRIPT", "C", False, False, ""),
    ])


def accounts_receivable(d):
    """Spec #6 9: the AR tables and the port's AR_LINEX and AR_RCPT.

    Layouts and tags copy the real AR_* tables (checked 2026-11-13; the tags in
    the real .MDX order); the rows are made up, in period 7 (Oct 26). VAT is
    included in the price. Every AR_JSUM row is waiting ("1"), not transferred,
    so no GL balance moves. AR_XFER is left to EnsureArTables, as AP_XFER.

      C001 hospital  VAT 10 %, terms 30, sales 410001-10    C002 doctor  VAT 10 %, terms 30, sales 410001-20
      C003 pharmacy  no VAT, terms 0, sales 410001-10       all: AR 111101, VAT 260007

      voucher 1  open       C001 S-1001  3 x 1,100.00 = 3,300.00 -> sales 3,000.00 + VAT 300.00
      voucher 2  open       C002 S-1002  2 x 550.00 = 1,100.00, and 220.00 typed -> sales 1,000.00 + 200.00,
                                         VAT 120.00; AR 1,320.00
      voucher 3  posted     C001 S-1003  1 x 2,200.00 -> 2,000.00 + VAT 200.00; owed in full ("O")
      voucher 4  posted     C002 S-1004  4 x 275.00 = 1,100.00 -> 1,000.00 + VAT 100.00; receipt 1 500.00 on
                                         10/20/26 ("R", 600.00 owed)
      voucher 5  posted     C003 S-1005  800.00 typed, no VAT; receipt 2 800.00 on 10/15/26 ("C", settled)
      voucher 6  cancelled  C001 S-1006  1 x 550.00 -> 500.00 + VAT 50.00 ("X", lines kept)
    """
    cust_fields = [
        ("VENDORNO", "C", 6, 0), ("VDSHORT", "C", 12, 0), ("VDNAME", "C", 40, 0), ("ADDRESS1", "C", 30, 0),
        ("ADDRESS2", "C", 30, 0), ("ADDRESS3", "C", 30, 0), ("POSTAL", "C", 8, 0), ("PHONE", "C", 16, 0),
        ("FAX", "C", 16, 0), ("S_ADD1", "C", 40, 0), ("S_ADD2", "C", 40, 0), ("S_ADD3", "C", 40, 0),
        ("S_POSTAL", "C", 8, 0), ("S_PHONE", "C", 16, 0), ("S_FAX", "C", 16, 0), ("DEPART", "C", 4, 0),
        ("ACCOUNT", "C", 7, 0), ("AR_DEPT", "C", 4, 0), ("AR_ACCT", "C", 7, 0), ("VAT_ACCT", "C", 7, 0),
        ("VAT_DEPT", "C", 4, 0), ("CONTACT", "C", 12, 0), ("TERMS", "N", 3, 0), ("TERMS_DAYS", "N", 3, 0),
        ("TERMS_PERC", "N", 2, 0), ("TERMS_TXT", "C", 20, 0), ("CREDIT", "N", 16, 2), ("VAT", "N", 3, 0),
        ("VIN", "C", 10, 0)]

    # addr: (address1, address2, address3, postal); ship: (s_add1, s_add2, s_add3, s_postal)
    def customer(no, short, name, addr, ship, sales_dept, vat_acct, contact, terms, vat, vin):
        return ((no, short, name) + addr + ("(604) 555 - 0170", None) + ship
                + (None, None, None, "111101", sales_dept, "410001", vat_acct, None, contact, terms,
                   None, None, None, None, vat, vin))
    dbf(os.path.join(d, "AR_CUST.DBF"), cust_fields, [
        customer("C001", "CITYHOSP", "City General Hospital", ("100 Hospital Road", None, "Vancouver BC", "V5Z1M9"),
                 ("Receiving, Pharmacy Wing", "100 Hospital Road", "Vancouver BC", "V5Z1M9"),
                 "10", "260007", "Purchasing", 30, 10, "200-123-45"),
        customer("C002", "REYES", "Reyes, Ana Dra.", ("12 Clinic Street", None, "Burnaby BC", "V5H2E1"),
                 (None, None, None, None), "20", "260007", "A. Reyes", 30, 10, None),
        customer("C003", "NORTHPHARM", "Northside Pharmacy", ("8 North Avenue", None, "Surrey BC", "V3T4W2"),
                 (None, None, None, None), "10", None, "Owner", 0, 0, None),
    ], mdx=True)
    mdx(os.path.join(d, "AR_CUST.MDX"), "AR_CUST", [("VENDORNO", "VENDORNO", "C", False, False, "")])

    btch_fields = [
        ("BATCHNO", "N", 6, 0), ("MODE", "C", 1, 0), ("TYPE", "C", 2, 0), ("ADJ", "C", 1, 0), ("PERIOD", "N", 2, 0),
        ("VENDORNO", "C", 6, 0), ("INVOICENO", "C", 8, 0), ("ORDERNO", "C", 10, 0), ("ORDERDATE", "D", 8, 0),
        ("VIN", "C", 10, 0), ("UNITS", "C", 7, 0), ("DUEDATE", "D", 8, 0), ("INVDATE", "D", 8, 0),
        ("REFERENCE", "C", 40, 0), ("DESCRIPT", "C", 10, 0), ("DATE", "D", 8, 0), ("ACCOUNT", "C", 7, 0),
        ("DEPART", "C", 4, 0), ("PAYABLE", "N", 16, 2), ("REBATEAMT", "N", 16, 2), ("ENTRYNO", "N", 3, 0),
        ("VATPERCENT", "N", 2, 0), ("REBATE", "N", 2, 0), ("TERMS", "N", 3, 0), ("VATAMT", "N", 16, 2),
        ("ADJAMT", "N", 16, 2), ("MEMOFIELD", "M", 10, 0)]
    memo = dbt(os.path.join(d, "AR_BTCH.DBT"), "AR_BTCH", [b"Deliver to the pharmacy wing."])

    def head(no, mode, cust, invoice, order, invdate, due, ref, total, lines, vat_pct, terms, vat, memo_ptr=None):
        return (no, mode, None, None, 7, cust, invoice, order, invdate, None, None, due, invdate, ref, "Sales",
                invdate, "111101", None, total, 0.0, lines, vat_pct, 0, terms, vat, 0.0, memo_ptr)
    dbf(os.path.join(d, "AR_BTCH.DBF"), btch_fields, [
        head(1, "1", "C001", "S-1001", "PO-5521", "20261022", "20261121", "Saline, 3 cases", 3300.0, 3, 10, 30,
             300.0, memo[0]),
        head(2, "1", "C002", "S-1002", None, "20261023", "20261122", "Clinic supplies", 1320.0, 4, 10, 30, 120.0),
        head(3, "0", "C001", "S-1003", "PO-5480", "20261005", "20261104", "Dressings", 2200.0, 3, 10, 30, 200.0),
        head(4, "0", "C002", "S-1004", None, "20261008", "20261107", "Vaccines, 4 boxes", 1100.0, 3, 10, 30, 100.0),
        head(5, "0", "C003", "S-1005", None, "20261002", "20261002", "Counter sale", 800.0, 2, 0, 0, 0.0),
        head(6, "X", "C001", "S-1006", None, "20261009", "20261108", "Entered twice", 550.0, 3, 10, 30, 50.0),
    ], version=0x8B, mdx=True)
    mdx(os.path.join(d, "AR_BTCH.MDX"), "AR_BTCH", [
        ("ACCOUNT", "ACCOUNT+DEPART", "C", False, False, ""),
        ("INVOICENO", "VENDORNO+INVOICENO", "C", False, False, ""),
        ("RECNO", "RECNO()", "N", False, False, ""),
        ("BATCHNO", "BATCHNO", "N", False, False, ""),
    ])

    rd, gen, vatp, ar = "SALES REVENUE - R&D", "SALES REVENUE - GENERAL", "VAT PAYABLE", "ACCOUNTS RECEIVABLE - TRADE"

    def line(no, entry, desc, acct, dept, qty, price, dr, cr):
        return (no, entry, desc, None, None, None, qty, price, acct, dept, dr, cr)
    dbf(os.path.join(d, "AR_BTCHD.DBF"),
        [("BATCHNO", "N", 6, 0), ("ENTRYNO", "N", 3, 0), ("DESCRIPT", "C", 40, 0), ("MEDREP", "C", 5, 0),
         ("PRODUCT", "C", 7, 0), ("DRCODE", "C", 5, 0), ("QTY", "N", 14, 2), ("PRICE", "N", 14, 2),
         ("ACCOUNT", "C", 7, 0), ("DEPT", "C", 4, 0), ("DEBIT", "N", 14, 2), ("CREDIT", "N", 14, 2)], [
        line(1, 1, rd, "410001", "10", 3.0, 1100.0, 0.0, 3000.0),
        line(1, 2, vatp, "260007", None, 0.0, 0.0, 0.0, 300.0),
        line(1, 3, ar, "111101", None, 0.0, 0.0, 3300.0, 0.0),
        line(2, 1, gen, "410001", "20", 2.0, 550.0, 0.0, 1000.0),
        line(2, 2, gen, "410001", "20", 0.0, 0.0, 0.0, 200.0),
        line(2, 3, vatp, "260007", None, 0.0, 0.0, 0.0, 120.0),
        line(2, 4, ar, "111101", None, 0.0, 0.0, 1320.0, 0.0),
        line(3, 1, rd, "410001", "10", 1.0, 2200.0, 0.0, 2000.0),
        line(3, 2, vatp, "260007", None, 0.0, 0.0, 0.0, 200.0),
        line(3, 3, ar, "111101", None, 0.0, 0.0, 2200.0, 0.0),
        line(4, 1, gen, "410001", "20", 4.0, 275.0, 0.0, 1000.0),
        line(4, 2, vatp, "260007", None, 0.0, 0.0, 0.0, 100.0),
        line(4, 3, ar, "111101", None, 0.0, 0.0, 1100.0, 0.0),
        line(5, 1, rd, "410001", "10", 0.0, 0.0, 0.0, 800.0),
        line(5, 2, ar, "111101", None, 0.0, 0.0, 800.0, 0.0),
        line(6, 1, rd, "410001", "10", 1.0, 550.0, 0.0, 500.0),
        line(6, 2, vatp, "260007", None, 0.0, 0.0, 0.0, 50.0),
        line(6, 3, ar, "111101", None, 0.0, 0.0, 550.0, 0.0),
    ], mdx=True)
    mdx(os.path.join(d, "AR_BTCHD.MDX"), "AR_BTCHD", [
        ("RECNO", "RECNO()", "N", False, False, ""),
        ("BATCHNO", "BATCHNO", "N", False, False, ""),
    ])

    # The port's line roles and typed amounts (Ar.h): S sales, V VAT, R the AR line.
    linex = [(1, 1, "S", 3300.0), (1, 2, "V", 0.0), (1, 3, "R", 0.0),
             (2, 1, "S", 1100.0), (2, 2, "S", 220.0), (2, 3, "V", 0.0), (2, 4, "R", 0.0),
             (3, 1, "S", 2200.0), (3, 2, "V", 0.0), (3, 3, "R", 0.0),
             (4, 1, "S", 1100.0), (4, 2, "V", 0.0), (4, 3, "R", 0.0),
             (5, 1, "S", 800.0), (5, 2, "R", 0.0),
             (6, 1, "S", 550.0), (6, 2, "V", 0.0), (6, 3, "R", 0.0)]
    dbf(os.path.join(d, "AR_LINEX.DBF"),
        [("BATCHNO", "N", 6, 0), ("ENTRYNO", "N", 3, 0), ("ROLE", "C", 1, 0), ("GROSS", "N", 16, 2)], linex,
        mdx=True)
    mdx(os.path.join(d, "AR_LINEX.MDX"), "AR_LINEX", [("BATCHNO", "BATCHNO", "N", False, False, "")])

    # (bank, ctl, date, due, batch, invoice, customer, reference, amount, vat, received, datepaid, remainder)
    def dist(bank, ctl, date, due, batch, invoice, cust, ref, amount, vat, paid, datepaid, remainder):
        return (bank, None, None, "111101", None, 7, date, due, batch, invoice, ctl, cust, None, ref, None,
                amount, 0.0, paid, 0, datepaid, vat, 0.0, None, remainder)
    dbf(os.path.join(d, "AR_DIST.DBF"),
        [("ACCOUNT", "C", 7, 0), ("DEPT", "C", 4, 0), ("MODE", "C", 1, 0), ("APACCOUNT", "C", 7, 0),
         ("APDEPART", "C", 4, 0), ("PERIOD", "N", 2, 0), ("DATE", "D", 8, 0), ("DUEDATE", "D", 8, 0),
         ("BATCHNO", "N", 6, 0), ("INVOICENO", "C", 8, 0), ("PAYMENTCTL", "C", 1, 0), ("VENDORNO", "C", 6, 0),
         ("CHEQUENO", "C", 10, 0), ("REFERENCE", "C", 20, 0), ("DESCRIPT", "C", 50, 0), ("AMOUNT", "N", 16, 2),
         ("REBATEAMT", "N", 16, 2), ("AMOUNTPAID", "N", 16, 2), ("PAYMENTREQ", "N", 10, 0), ("DATEPAID", "D", 8, 0),
         ("VATAMT", "N", 16, 2), ("ADJUSTMENT", "N", 16, 2), ("REBATE", "C", 2, 0), ("REMAINDER", "N", 16, 2)], [
        dist(None, "O", "20261005", "20261104", 3, "S-1003", "C001", "Dressings", 2200.0, 200.0, 0.0, None, 2200.0),
        dist("102101", "R", "20261008", "20261107", 4, "S-1004", "C002", "Vaccines, 4 boxes", 1100.0, 100.0,
             500.0, "20261020", 600.0),
        dist("102101", "C", "20261002", "20261002", 5, "S-1005", "C003", "Counter sale", 800.0, 0.0,
             800.0, "20261015", 0.0),
    ], mdx=True)
    mdx(os.path.join(d, "AR_DIST.MDX"), "AR_DIST", [
        ("PAYMENTREQ", "PAYMENTREQ", "N", False, False, ""),
        ("VENDORNO", "VENDORNO", "C", False, False, ""),
        ("BATCHNO", "BATCHNO", "N", False, False, ""),
        ("INVOICENO", "VENDORNO+INVOICENO", "C", False, False, ""),
        ("DEPT", "DEPT", "C", False, False, ""),
        ("ACCOUNT", "ACCOUNT", "C", False, False, ""),
    ])

    dbf(os.path.join(d, "AR_RCPT.DBF"),
        [("RCPTNO", "N", 6, 0), ("BATCHNO", "N", 6, 0), ("VENDORNO", "C", 6, 0), ("INVOICENO", "C", 8, 0),
         ("DATE", "D", 8, 0), ("PERIOD", "N", 2, 0), ("BANKACCT", "C", 7, 0), ("BANKDEPT", "C", 4, 0),
         ("AMOUNT", "N", 16, 2), ("DISCOUNT", "N", 16, 2), ("DISCVAT", "N", 16, 2), ("REFERENCE", "C", 30, 0)], [
        (1, 4, "C002", "S-1004", "20261020", 7, "102101", None, 500.0, 0.0, 0.0, "Cheque 88231"),
        (2, 5, "C003", "S-1005", "20261015", 7, "102101", None, 800.0, 0.0, 0.0, "Cash"),
    ], mdx=True)
    mdx(os.path.join(d, "AR_RCPT.MDX"), "AR_RCPT", [
        ("BATCHNO", "BATCHNO", "N", False, False, ""),
        ("RCPTNO", "RCPTNO", "N", False, False, ""),
    ])

    # AR's journal, all waiting: vouchers 3, 4 and 5, and the two receipts (CHEQUE: the receipt's number).
    def jsum(acct, dept, date, typ, batch, ref, rcpt, desc, dr, cr):
        return (acct, dept, " 7", "1", date, typ, batch, ref, rcpt, desc, dr, cr)
    bank = "CHECKING ACCOUNT"
    dbf(os.path.join(d, "AR_JSUM.DBF"),
        [("ACCOUNT", "C", 7, 0), ("DEPT", "C", 4, 0), ("PERIOD", "C", 4, 0), ("MODE", "C", 1, 0),
         ("DATE", "D", 8, 0), ("TYPE", "C", 3, 0), ("BATCHNO", "N", 6, 0), ("REFERENCE", "C", 30, 0),
         ("CHEQUE", "C", 7, 0), ("DESCRIPT", "C", 50, 0), ("DEBIT", "N", 16, 2), ("CREDIT", "N", 16, 2)], [
        jsum("410001", "10", "20261005", "AR", 3, "Dressings", None, rd, 0.0, 2000.0),
        jsum("260007", None, "20261005", "AR", 3, "Dressings", None, vatp, 0.0, 200.0),
        jsum("111101", None, "20261005", "AR", 3, "Dressings", None, ar, 2200.0, 0.0),
        jsum("410001", "20", "20261008", "AR", 4, "Vaccines, 4 boxes", None, gen, 0.0, 1000.0),
        jsum("260007", None, "20261008", "AR", 4, "Vaccines, 4 boxes", None, vatp, 0.0, 100.0),
        jsum("111101", None, "20261008", "AR", 4, "Vaccines, 4 boxes", None, ar, 1100.0, 0.0),
        jsum("410001", "10", "20261002", "AR", 5, "Counter sale", None, rd, 0.0, 800.0),
        jsum("111101", None, "20261002", "AR", 5, "Counter sale", None, ar, 800.0, 0.0),
        jsum("102101", None, "20261015", "RC", 5, "Cash", "2", bank, 800.0, 0.0),
        jsum("111101", None, "20261015", "RC", 5, "Cash", "2", ar, 0.0, 800.0),
        jsum("102101", None, "20261020", "RC", 4, "Cheque 88231", "1", bank, 500.0, 0.0),
        jsum("111101", None, "20261020", "RC", 4, "Cheque 88231", "1", ar, 0.0, 500.0),
    ], mdx=True)
    mdx(os.path.join(d, "AR_JSUM.MDX"), "AR_JSUM", [
        ("ARDIST", "BATCHNO", "N", False, False, 'TYPE="AR "'),
        ("APDIST", "BATCHNO", "N", False, False, 'TYPE="AP "'),
        ("DESCRIPT", "DESCRIPT", "C", False, False, ""),
        ("BATCHNO", "BATCHNO", "N", False, False, ""),
        ("ACCOUNT", "ACCOUNT+DEPT", "C", False, False, ""),
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
    accounts_payable(d)
    accounts_receivable(d)

    print(f"dev fixture written to {OUT}: {len(items)} accessm rows")


if __name__ == "__main__":
    main()
