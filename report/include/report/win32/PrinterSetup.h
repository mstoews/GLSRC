#pragma once
//
// PrinterSetup.h - the session's printer and Printer Setup (spec #3 4.6,
// D10; plan S3-M1 task 2).
//
// The session printer is the Windows default printer until the user picks
// another in Printer Setup (PageSetupDlgW) or in a Print dialog; the choice
// lasts until the program exits and never changes the Windows default.
// Printer Setup also sets the paper and the margins (default 0.75 inch).
// It doesn't offer orientation: each report has its own (spec #3 4.6,
// "default orientation per report"), and a preview lays out in it.
//
// With no printer installed, layouts use A4 and the screen for measuring,
// and Printer Setup and Print say there is no printer.
//
// Open previews register with AddPrinterListener; when the session printer
// or its paper or margins change, each gets kPrinterChanged (posted) and
// lays its report out again.
//
#include "report/Layout.h"

#include <Windows.h>

namespace rpt::win32 {

inline constexpr UINT kPrinterChanged = WM_APP + 0x3A1;

// Printer Setup (command 2001). True after OK.
bool ShowPrinterSetup(HWND owner);

// Copies of the session's DEVMODE and DEVNAMES with the orientation set, for
// PrintDlgW; both null without a printer. The caller frees them (GlobalFree).
struct PrinterHandles {
    HGLOBAL devMode = nullptr;
    HGLOBAL devNames = nullptr;
};
PrinterHandles CopyPrinter(Orientation o);

// Makes these (from a Print dialog) the session printer and tells the
// listeners. Takes ownership of both handles.
void SetSessionPrinter(HGLOBAL devMode, HGLOBAL devNames);

// An information context for the session printer in orientation `o`, or
// null without a printer. For GdiMeasure, which deletes it.
HDC CreatePrinterIC(Orientation o);

// The paper and margins for a layout in orientation `o`: the session
// printer's paper, or A4 without one; the session margins, widened to the
// printer's unprintable edges.
PageSetup CurrentPageSetup(Orientation o);

// The same for a given printer DC (the one a Print dialog returned).
PageSetup PageSetupFromDC(HDC printer);

// The paper's top-left corner in the device units of a printer DC: minus the
// unprintable offset, since a printer DC's origin is the printable area's corner.
POINT PaperOrigin(HDC printer);

void AddPrinterListener(HWND hwnd);
void RemovePrinterListener(HWND hwnd);

} // namespace rpt::win32
