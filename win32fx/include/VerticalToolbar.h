//
// VerticalToolbar.h - the module bar: the original TVToolBar (spec 3.5).
//
// Despite the name (kept from the plan), the original docks this bar at the
// bottom of the frame (TGlWindow::WMSize). Dialog 2057 holds the General
// Ledger / Payables / Receivable buttons and three read-only tbPic fields the
// frame writes status text into: 3222 company, 3223 menu help, 3224 period.
//
#pragma once

#include "Toolbar.h"

namespace win32fx {

class VerticalToolbar : public Toolbar {
public:
    VerticalToolbar() noexcept { SetLayout(2, 30, 25); }

    // SetDlgItemTextW on one of the bar's fields.
    void SetText(int id, const wchar_t* text);

    // Moves field `id` to x..x+width (the original resized 3222-3224 on every
    // WMSize so the help text took the space left over).
    void PlaceField(int id, int x, int width);
};

} // namespace win32fx
