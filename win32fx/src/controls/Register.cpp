//
// Register.cpp - RegisterCustomControls (spec 3.6).
//

#include "ControlsInternal.h"

#include <CommCtrl.h>

#pragma comment(lib, "comctl32.lib")

bool win32fx::RegisterCustomControls(HINSTANCE inst)
{
    INITCOMMONCONTROLSEX icc{};
    icc.dwSize = sizeof icc;
    icc.dwICC = ICC_STANDARD_CLASSES | ICC_BAR_CLASSES | ICC_WIN95_CLASSES;
    InitCommonControlsEx(&icc);

    bool ok = detail::RegisterPicEdit(inst);
    ok = detail::RegisterDateEdit(inst) && ok;
    ok = detail::RegisterFloatEdit(inst) && ok;
    ok = detail::RegisterIntEdit(inst) && ok;
    ok = detail::RegisterSuperBtn(inst) && ok;
    ok = detail::RegisterGrid(inst) && ok;
    ok = detail::RegisterStatusPane(inst) && ok;
    return ok;
}
