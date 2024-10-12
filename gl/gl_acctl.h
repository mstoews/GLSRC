#ifndef __GL_ACCTL_H
#define __GL_ACCTL_H
#include "tlist_w.h"
#include "db_accts.h"
_CLASSDEF(TAccountList)
class TAccountList : public TWindow
{
 public:
 // The class name must be the same name as the dbf file.
 //
 PTListWindow List;
 Tglaccts *glaccts;
 TAccountList(PTWindowsObject AParent, LPSTR AName,Tglaccts *gl):TWindow (AParent, AName)
  {
   glaccts = gl;
	List = new TListWindow(this,ID_DISTLIST,"",0,0,0,0,GetClassName());
	Attr.Style |= WS_THICKFRAME;
  }
 virtual void HandleListBox (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK];
 virtual void WMSize        (RTMessage Msg) = [WM_FIRST + WM_SIZE];
 virtual void SetupWindow();
 virtual void SetWndDetail();
 virtual LPSTR GetClassName(){ return "GLACCTS";} 
};
#endif