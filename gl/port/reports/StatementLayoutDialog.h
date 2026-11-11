//
// StatementLayoutDialog.h - Accounts > Statement Layout (CM_STATEMENT_LAYOUT,
// dialog IDD_STMT_LAYOUT; spec #3 6.4, 6.5; plan S3-M4).
//
// Edits a copy of a statement's layout (StatementLayout.h) in memory:
//
//   - a statement picker and a read-only grid of its lines (no., kind,
//     caption indented, sign, formula, ranges);
//   - the selected line's fields under the grid, with Add (at the end),
//     Insert before (the selected line), Delete and Apply (to the selected
//     line; Enter in a field applies too). A line number left blank is
//     chosen: 10 after the last line, or halfway to the previous line;
//   - Check lists spec 6.5's errors and warnings; Preview runs the statement
//     from the copy, in a preview window over the dialog; Reset to default
//     (after asking) writes the default at once; Save refuses errors, asks on
//     warnings, and writes the copy in one transaction; Close and changing
//     the statement ask about unsaved changes.
//
// Line numbers are what totals' formulas refer to, so nothing renumbers them.
//
#pragma once

#include <Windows.h>

// `closeAtOnce`: open and close at once, for /stress.
INT_PTR RunStatementLayout(HWND frame, bool closeAtOnce = false);

// gl.exe /stress: opens and closes the screen `times` times; the change in
// GDI + USER objects (0 if none), or -1 if it couldn't open.
int StressStatementLayout(int times);
