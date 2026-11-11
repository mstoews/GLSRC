//
// FixedAssetTypes.h - Fixed Assets > Asset Types (ID_FATYPE 6036, dialog 4002;
// gl/src/gl_fa.cpp TGLTypes, TAssetTypesList; spec #4 6.1, plan S4-M1).
//
// A type names the GL accounts an asset's cost, depreciation expense and
// accumulated depreciation go to. The record operations are in Assets.h.
//
#pragma once

#include <Windows.h>

INT_PTR RunAssetTypes(HWND frame);

// gl.exe /stress: opens and closes the Fixed Assets screens `times` times and
// returns the change in the process's GDI + USER object count (0 if none),
// as StressMasterData does. S4-M1: Asset Types; the later milestones add
// their screens here.
int StressFixedAssets(int times);
