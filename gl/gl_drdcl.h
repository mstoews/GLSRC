#ifndef __GL_MEDRL_H
#define __GL_MEDRL_H
#include "tlist_w.h"
#include "db_drcod.h"
_CLASSDEF(TGLDrCodeList)
class TGLDrCodeList : public TWindow
{
 public:
 // The class name must be the same name as the dbf file.
 //
 PTListWindow List;
 Tglmedreps *glmedrp;
 TGLDrCodeList(PTWindowsObject AParent, LPSTR AName,Tglmedreps *gl):TWindow (AParent, AName)
  {
   glmedrp = gl;
	List = new TListWindow(this,ID_DISTLIST,"",0,0,0,0,GetClassName());
	Attr.Style |= WS_THICKFRAME;
  }
 virtual void HandleListBox (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK];
 virtual void SetupWindow();
 virtual void SetWndDetail();
 virtual LPSTR GetClassName(){ return "GL_DRCOD";}
};
#endif