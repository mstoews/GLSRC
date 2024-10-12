#ifndef __GL_BTCAN_H
#define __GL_BTCAN_H
#include "gl_dlg.h"
_CLASSDEF(TGLCancelBatch)
class TGLCancelBatch : public TMainDialog
{
 public:
 Tglbtch *glbtch;
 Tglbtchd* glbtchd;
 TGLCancelBatch(PTWindowsObject AParent, int AName, PTModule AModule = NULL): TMainDialog(AParent,AName){};
 virtual void SetupWindow();
 virtual void Cancel         (RTMessage Msg) = [ID_FIRST + IDCANCEL];
 virtual void SelectSlip     (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK];
};
#endif

