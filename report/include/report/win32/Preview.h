#pragma once
//
// Preview.h - the report preview (spec #3 4.5; plan S3-M1 task 3).
//
// An MDI child per report run, titled with the report's title. Under a
// button strip it shows one page at a time on a grey desk:
//
//   [Print...]  [|<] [<]  3 of 12  [>] [>|]  [Fit width]  [Options...]  [Close]
//
//   - Zoom cycles Fit width (the default) -> Fit page -> 100 %.
//   - Keys: PgUp / PgDn the previous / next page, Home / End the first / last,
//     the arrows scroll, Ctrl+P prints.
//   - The wheel scrolls; when the page fits the window it turns pages.
//   - Options... asks the caller for a rebuilt report (its options dialog);
//     without a rebuild function the button isn't there.
//
// The pages are laid out for the session printer (PrinterSetup.h) in the
// report's orientation, and laid out again when that printer changes. Print
// lays out again for the printer chosen in the Print dialog (Print.h).
//
// The window owns itself: it is deleted when its MDI child is destroyed.
//
#include "report/Report.h"

#include "MdiFrame.h"   // win32fx

#include <functional>
#include <optional>

namespace rpt::win32 {

// Shows the report's options again and builds it: nullopt when cancelled
// (or when building failed and the error was shown).
using Rebuild = std::function<std::optional<Report>(HWND owner)>;

// Opens a preview of `report` in `frame`. Null if the window couldn't be created.
HWND OpenPreview(win32fx::MdiFrame& frame, Report report, Rebuild rebuild = {});

// True if `hwnd` is a preview window.
bool IsPreview(HWND hwnd) noexcept;

// If the frame's active MDI child is a preview, prints it (as its Print...
// button does) and returns true. File > Print and the toolbar Print button
// (spec #3 D9).
bool PrintActivePreview(win32fx::MdiFrame& frame);

// For tests and /stress: the page shown (0-based) and the page count of a
// preview window; -1 if `hwnd` isn't one.
int PreviewPage(HWND hwnd) noexcept;
int PreviewPageCount(HWND hwnd) noexcept;
// Presses the Zoom button: Fit width -> Fit page -> 100 % -> Fit width. False if not a preview.
bool CyclePreviewZoom(HWND hwnd);

} // namespace rpt::win32
