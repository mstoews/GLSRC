//
// FixedAssets.h - Fixed Assets > Add/Modify (ID_FAADD 6000, dialog 4004;
// gl/src/gl_fa.cpp TGLFixedAssets, TAssetList; spec #4 6.2, plan S4-M2).
//
// The asset register: an asset's code, type (whose GL accounts are shown),
// description, reference, date purchased, cost, salvage value, life and
// months used, method and declining rate; and, computed, its monthly
// depreciation, accumulated depreciation and net book value. The record
// operations and the arithmetic are in Assets.h.
//
#pragma once

#include <Windows.h>

// `closeAtOnce`: open and close at once, for /stress.
INT_PTR RunFixedAssets(HWND frame, bool closeAtOnce = false);

// gl.exe /stress: opens and closes each Fixed Assets screen `times` times and
// returns the change in the process's GDI + USER object count (0 if none),
// as StressMasterData does; -1 if a screen couldn't open.
int StressFixedAssets(int times);
