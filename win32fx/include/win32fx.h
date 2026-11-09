//
// win32fx.h - umbrella header for the mini-framework (spec 3).
//
//     #include "win32fx.h"
//     class Login : public fx::Dialog { ... };
//
// The library uses namespace win32fx; `fx` is a short alias. It owns no
// globals: hInst, HGLMenu, cb and szApp stay in gl/port/App.cpp (spec 3.7)
// and are passed in where needed. Link comctl32.lib (pulled in with
// #pragma comment) and ship a manifest for common controls v6.
//
#pragma once

#include "Dialog.h"
#include "HandleGuard.h"
#include "MdiFrame.h"
#include "MessageLoop.h"
#include "MsgMap.h"
#include "StatusBar.h"
#include "StringUtil.h"
#include "Toolbar.h"
#include "Verify.h"
#include "VerticalToolbar.h"
#include "Window.h"
#include "controls/CustomControls.h"
#include "controls/Grid.h"

namespace fx = win32fx;
