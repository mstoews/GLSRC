#ifndef __AP_CUSTL_H
#define __AP_CUSTL_H
#include "tlist_w.h"
#include "db_vend.h"
_CLASSDEF(TCustomerList)
class TCustomerList : public TWindow
{
 public:
 Code4 *cb;
 long iRows;
 TListWindow* List;
 Tapvendor apvendor;
 string    strKey;
 TCustomerList(PTWindowsObject AParent, LPSTR AName):TWindow (AParent, AName)
  {
	List = new TListWindow(this,ID_DISTLIST,"",0,0,0,0,GetClassName());
	Attr.Style |= WS_THICKFRAME;
	cb = &((TAcctModule *)GetApplication())->cb;
  }
 ~TCustomerList()
 {
	delete List;
	apvendor.close();
 }
 virtual void HandleListBox (RTMessage Msg) = [WM_FIRST + SSM_DBLCLK];
 virtual void WMSize        (RTMessage Msg) = [WM_FIRST + WM_SIZE]; 
 virtual void SetupWindow();
 virtual void SetWndDetail();
 virtual LPSTR GetClassName(){ return "AP_VEND";}
};
#endif