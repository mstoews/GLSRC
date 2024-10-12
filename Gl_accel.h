#ifndef __GL_ACCEL_H
#define __GL_ACCEL_H
#include "tlist_w.h"
#include "db_acces.h"
_CLASSDEF(TAccessList)
class TAccessList : public TWindow
{
 public:
 // The class name must be the same name as the dbf file.
 //
 TListWindow* List;
 TAccess* Access;
 TAccessList(PTWindowsObject AParent, LPSTR AName,TAccess* ap):TWindow (AParent, AName)
  {
	Access = ap;
	List = new TListWindow(this,ID_DISTLIST,"",0,0,0,0,GetClassName());
	Attr.Style |= WS_THICKFRAME;
  }
 virtual void WMSize        (RTMessage Msg) = [WM_FIRST + WM_SIZE];
 virtual void HandleListBox (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK];
 virtual void SetupWindow();
 virtual void SetWndDetail();
 virtual LPSTR GetClassName(){ return "ACCESS";}
};

_CLASSDEF(TAccessLevel)
class TAccessLevel : public TWindow
{
 public:
 // The class name must be the same name as the dbf file.
 //
 TListWindow* List;
 TAccessl* Accessl;
 TAccessLevel(PTWindowsObject AParent, LPSTR AName,TAccessl* ap):TWindow (AParent, AName)
  {
	Accessl = ap;
	List = new TListWindow(this,ID_DISTLIST,"",0,0,0,0,GetClassName());
	Attr.Style |= WS_THICKFRAME;
  }
 virtual void WMSize        (RTMessage Msg) = [WM_FIRST + WM_SIZE];
 virtual void HandleListBox (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK];
 virtual void SetupWindow();
 virtual void SetWndDetail();
 virtual LPSTR GetClassName(){ return "ACCESSL";}
};

#endif