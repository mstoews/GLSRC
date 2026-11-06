
#ifndef __GL_ASSTL_H
#define __GL_ASSTL_H
#include "tlist_w.h"
_CLASSDEF(TAssetList)
class TAssetList : public TWindow
{
 public:
 // The class name must be the same name as the dbf file.
 //
 PTListWindow List;
 Tglfa *glfa;
 TAssetList(PTWindowsObject AParent, LPSTR AName,Tglfa *gl):TWindow (AParent, AName)
  {
   glfa = gl;
	List = new TListWindow(this,ID_DISTLIST,"",0,0,0,0,GetClassName());
	Attr.Style |= WS_THICKFRAME;
  }
 virtual void WMSize        (RTMessage Msg) = [WM_FIRST + WM_SIZE];
 virtual void HandleListBox (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK];
 virtual void SetupWindow();
 virtual void SetWndDetail();
 virtual LPSTR GetClassName(){ return "GL_FA";}
};
#endif