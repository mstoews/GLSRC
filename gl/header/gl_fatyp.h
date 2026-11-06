#ifndef __GL_TYPEL_H
#define __GL_TYPEL_H
#include "tlist_w.h"
_CLASSDEF(TAssetTypesList)
class TAssetTypesList : public TWindow
{
 public:
 // The class name must be the same name as the dbf file.
 //
 PTListWindow List;
 Tfatype  *fatype;
 TAssetTypesList(PTWindowsObject AParent, LPSTR AName,Tfatype *gl):TWindow (AParent, AName)
  {
   fatype = gl;
	List = new TListWindow(this,ID_DISTLIST,"",0,0,0,0,GetClassName());
	Attr.Style |= WS_THICKFRAME;
  }
 virtual void HandleListBox (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK];
 virtual void WMSize        (RTMessage Msg) = [WM_FIRST + WM_SIZE];
 virtual void SetupWindow();
 virtual void SetWndDetail();
 virtual LPSTR GetClassName(){ return "GL_FATYP";}
};
#endif