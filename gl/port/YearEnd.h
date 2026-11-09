//
// YearEnd.h - Maintenance > Year End Consolidation (ID_YEAREND, 1068):
// TGlWindow::GLYearEnd (gl/src/gl_postb.cpp). Asks string 8111, then runs
// YearEnd (Vouchers.h), which refuses unless exactly one account has type
// "R" (spec #2 D2). Plan S2-M6.
//
#pragma once

#include <Windows.h>

void RunYearEnd(HWND frame);
