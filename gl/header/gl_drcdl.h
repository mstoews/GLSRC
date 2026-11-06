#ifndef __GL_DRCDL_H
#define __GL_DRCDL_H
#include "tlist_w.h"
#include "db_drcod.h"
_CLASSDEF(TGLDrCodeList)
class TGLDrCodeList : public TWindow
{
 public:
 // The class name must be the same name as the dbf file.
 //
 PTListWindow List;
 Tgldrcodes *gldrcodes;
 TGLDrCodeList(PTWindowsObject AParent, LPSTR AName,Tgldrcodes *gl):TWindow (AParent, AName)
  {
   gldrcodes = gl;
	List = new TListWindow(this,ID_DISTLIST,"",0,0,0,0,GetClassName());
	Attr.Style |= WS_THICKFRAME;
  }
 virtual void HandleListBox (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK];
 virtual void WMSize		    (RTMessage Msg) = [WM_FIRST + WM_SIZE];
 virtual void SetupWindow();
 virtual void SetWndDetail();
 virtual LPSTR GetClassName(){ return "GL_DRCOD";}
};
#endif