#ifndef __AP_CUSTL_H
#define __AP_CUSTL_H
#include "tlist_w.h"
#include "db_vend.h"
_CLASSDEF(TCustomerList)
class TCustomerList : public TWindow
{
 public:
 // The class name must be the same name as the dbf file.
 //
 PTListWindow List;
 Tapvendor *apvendor;
 TCustomerList(PTWindowsObject AParent, LPSTR AName,Tapvendor *ap):TWindow (AParent, AName)
  {
   apvendor = ap;
	List = new TListWindow(this,ID_DISTLIST,"",0,0,0,0,GetClassName());
	Attr.Style |= WS_THICKFRAME;
  }
 virtual void HandleListBox (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK];
 virtual void WMSize        (RTMessage Msg) = [WM_FIRST + WM_SIZE]; 
 virtual void SetupWindow();
 virtual void SetWndDetail();
 virtual LPSTR GetClassName(){ return "AP_VEND";}
};
#endif