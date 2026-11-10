#pragma once
//
// Print.h - printing a report (spec #3 4.6; plan S3-M1 task 4).
//
// PrintReport shows the Print dialog (printer, page range, copies), which
// starts on the session printer in the report's orientation. The printer
// chosen there becomes the session printer. The report is laid out again
// with the chosen printer's DC, so the page range refers to those pages, and
// printed as one document named after the title. A small "Printing page n of
// m" window with Cancel stays up meanwhile (SetAbortProc); the owner is
// disabled. A GDI failure is shown in a message box.
//
#include "report/Report.h"

#include <Windows.h>

namespace rpt::win32 {

enum class PrintResult { Printed, Cancelled, Failed };

// pageCount: the pages the preview shows, for the dialog's page range (0: unknown).
PrintResult PrintReport(HWND owner, const Report& report, int pageCount = 0);

} // namespace rpt::win32
