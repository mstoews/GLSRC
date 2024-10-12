#ifndef _GL_POSTB_H
#define _GL_POSTB_H
#include "gl_dlg.h"
_CLASSDEF(TGLPostBatch)
class TGLPostBatch : public TMainDialog
{
 public:
 PTglbtch  glbtch;
 PTglbtchd glbtchd;
 PTglaccts glaccts;
 PTgldist  gldist;
 TSpread   *List;
 TGLPostBatch(PTWindowsObject AParent, int AName, PTModule AModule = NULL): TMainDialog(AParent,AName)
 {
	List = new TSpread(this,ID_DISTLIST,"",0,0,0,0);
 };
 virtual void SetupWindow();
 virtual void Cancel         (RTMessage Msg) = [ID_FIRST + IDCANCEL];
 virtual void Ok             (RTMessage Msg) = [ID_FIRST + IDOK];
 virtual void HandleListBox  (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK];
 void UpdateGL(Field4 *account, Field4 *dept);
 void PostGLSlip    (long lBatchno);
 void PostAdjustment(long lBatchno);
 void Redraw();
};
#endif