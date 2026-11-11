//
// FixedAssetTypes.h - Fixed Assets > Asset Types (ID_FATYPE 6036, dialog 4002;
// gl/src/gl_fa.cpp TGLTypes, TAssetTypesList; spec #4 6.1, plan S4-M1).
//
// A type names the GL accounts an asset's cost, depreciation expense and
// accumulated depreciation go to. The record operations are in Assets.h.
//
#pragma once

#include <Windows.h>

// `closeAtOnce`: open and close at once, for /stress (StressFixedAssets in
// FixedAssets.h).
INT_PTR RunAssetTypes(HWND frame, bool closeAtOnce = false);
